#include <Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define TRIG_PIN 9
#define ECHO_PIN 10
#define SERVO_PIN 11

// Button: Pin -> GND, wir nutzen INPUT_PULLUP
#define BUTTON_PIN 4

Servo servo;

const int maxDistance = 50;               // cm
const int centerX = SCREEN_WIDTH / 2;
const int centerY = SCREEN_HEIGHT - 1;    // unten mittig
const int maxRadius = 50;                 // Pixel

struct Blip {
  float angle;                 // Grad 0..180
  int distance;                // cm
  unsigned long timestamp;     // ms
  bool active;
};

Blip blips[20];
int blipCount = 0;

unsigned long lastUpdate = 0;
const unsigned long fadeTime = 3000;      // ms

// -------------------------
// Start/Stop Logik
// -------------------------
bool running = false;
bool lastButtonReading = HIGH;
bool buttonStableState = HIGH;
unsigned long lastDebounceMs = 0;
const unsigned long debounceDelayMs = 35;

// -------------------------
// HC-SR04 Messung
// -------------------------
float measureDistanceOnce() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000UL);
  if (duration == 0) return (float)maxDistance;

  float d = (float)duration * 0.034f / 2.0f;   // cm
  if (d > maxDistance) d = (float)maxDistance;
  if (d < 0) d = 0;
  return d;
}

float measureDistanceFiltered() {
  float v[5];
  for (int i = 0; i < 5; i++) {
    v[i] = measureDistanceOnce();
    delay(5);
  }
  // sort
  for (int i = 0; i < 5; i++) {
    for (int j = i + 1; j < 5; j++) {
      if (v[j] < v[i]) {
        float t = v[i];
        v[i] = v[j];
        v[j] = t;
      }
    }
  }
  return v[2]; // median
}

// -------------------------
// Blips
// -------------------------
void clearBlips() {
  blipCount = 0;
  for (int i = 0; i < 20; i++) {
    blips[i].active = false;
    blips[i].angle = 0;
    blips[i].distance = 0;
    blips[i].timestamp = 0;
  }
}

void addBlip(float angleDeg, int distanceCm) {
  if (distanceCm >= maxDistance) return;
  if (distanceCm < 0) return;

  if (blipCount < 20) {
    blips[blipCount].angle = angleDeg;
    blips[blipCount].distance = distanceCm;
    blips[blipCount].timestamp = millis();
    blips[blipCount].active = true;
    blipCount++;
  } else {
    for (int i = 0; i < 19; i++) blips[i] = blips[i + 1];
    blips[19].angle = angleDeg;
    blips[19].distance = distanceCm;
    blips[19].timestamp = millis();
    blips[19].active = true;
  }
}

void updateBlips() {
  unsigned long now = millis();
  for (int i = 0; i < blipCount; i++) {
    if (blips[i].active && (now - blips[i].timestamp > fadeTime)) {
      blips[i].active = false;
      for (int j = i; j < blipCount - 1; j++) blips[j] = blips[j + 1];
      blipCount--;
      i--;
    }
  }
}

void drawBlips() {
  unsigned long now = millis();
  for (int i = 0; i < blipCount; i++) {
    if (!blips[i].active) continue;

    float age = (now - blips[i].timestamp) / (float)fadeTime;
    if (age < 0) age = 0;
    if (age > 1) age = 1;

    float ang = radians(blips[i].angle);
    float r = (blips[i].distance * (float)maxRadius) / (float)maxDistance;

    int x = (int)lroundf(centerX + r * cosf(ang));
    int y = (int)lroundf(centerY - r * sinf(ang));

    if (age < 0.5f) {
      display.drawLine(x - 2, y - 2, x + 2, y + 2, SSD1306_WHITE);
      display.drawLine(x - 2, y + 2, x + 2, y - 2, SSD1306_WHITE);
    } else {
      display.drawPixel(x, y, SSD1306_WHITE);
    }
  }
}

void drawRadar() {
  display.drawCircle(centerX, centerY, maxRadius / 2, SSD1306_WHITE);
  display.drawCircle(centerX, centerY, maxRadius, SSD1306_WHITE);
}

// -------------------------
// UI Screens
// -------------------------
void drawReadyScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.println("RADAR READY");
  display.setCursor(0, 28);
  display.println("Servo: 0 Grad");
  display.setCursor(0, 46);
  display.println("Button druecken");
  display.display();
}

// -------------------------
// Button (debounced) -> true on press event
// -------------------------
bool buttonPressedEvent() {
  bool reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonReading) {
    lastDebounceMs = millis();
    lastButtonReading = reading;
  }

  if ((millis() - lastDebounceMs) > debounceDelayMs) {
    if (reading != buttonStableState) {
      buttonStableState = reading;
      if (buttonStableState == LOW) return true; // Pullup
    }
  }
  return false;
}

// -------------------------
// Setup / Loop
// -------------------------
void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  servo.attach(SERVO_PIN);
  servo.write(90);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for (;;) {}
  }

  clearBlips();
  drawReadyScreen();
}

void loop() {
  // Start/Stop über Button
  if (buttonPressedEvent()) {
    running = !running;

    if (!running) {
      servo.write(90);
      clearBlips();
      drawReadyScreen();
      delay(150);
      return;
    } else {
      clearBlips();
      lastUpdate = 0;
      delay(150);
    }
  }

  if (!running) return;

  static int angle = 0;
  static bool forward = true;
  static unsigned long lastServoMove = 0;
  static unsigned long lastMeasurement = 0;

  unsigned long now = millis();

  // Servo bewegen
  if (now - lastServoMove >= 25) {
    servo.write(angle);
    delay(15); // settle

    if (forward) {
      angle += 2;
      if (angle >= 180) { angle = 180; forward = false; }
    } else {
      angle -= 2;
      if (angle <= 0) { angle = 0; forward = true; }
    }

    lastServoMove = now;
  }

  // Ultraschall messen (gefiltert)
  if (now - lastMeasurement >= 100) {
    float distance = measureDistanceFiltered();
    int d = (int)lroundf(distance);
    if (d < maxDistance) addBlip((float)angle, d);
    lastMeasurement = now;
  }

  // Rendern
  if (now - lastUpdate >= 33) {
    display.clearDisplay();

    updateBlips();
    drawRadar();

    // Sweep line
    float ang = radians((float)angle);
    int sweepX = (int)lroundf(centerX + maxRadius * cosf(ang));
    int sweepY = (int)lroundf(centerY - maxRadius * sinf(ang));
    display.drawLine(centerX, centerY, sweepX, sweepY, SSD1306_WHITE);

    drawBlips();

    display.display();
    lastUpdate = now;
  }
}
