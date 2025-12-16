
# Arduino Radar mit Servo, HC-SR04 und OLED

## Überblick

Dieses Projekt realisiert ein einfaches **Ultraschall-Radar** mit einem rotierenden Servo, einem **HC-SR04 Ultraschallsensor** und einem **128x64 OLED-Display (SSD1306)**.

Der Sensor wird über einen Servo von 0–180° geschwenkt. Entdeckte Objekte werden als sogenannte *Blips* auf einem Radar-ähnlichen Display dargestellt und blenden nach einer gewissen Zeit automatisch aus.

Das Radar kann über einen **Taster** gestartet und gestoppt werden.

---

## Features

- Servo-Sweep von 0–180°
- Gefilterte Distanzmessung (Median aus 5 Messungen)
- Radar-Darstellung mit Sweep-Linie
- Zeitbasiertes Ausfaden alter Messpunkte (Blips)
- Start/Stop-Funktion per Taster (entprellt)
- Ruhiger Bildaufbau (~30 FPS)
- Klare READY-Anzeige im Stopp-Zustand

---

## Benötigte Hardware

- Arduino Uno / Nano / kompatibel
- HC-SR04 Ultraschallsensor
- Servo (z. B. SG90)
- OLED Display 128x64, I2C (SSD1306, Adresse 0x3C)
- Taster (Push Button)
- Widerstände sind nicht notwendig (INPUT_PULLUP wird genutzt)
- Breadboard
- Jumperkabel
- Optional: externe 5V Versorgung für Servo (empfohlen)

---

## Verkabelung

### HC-SR04

| HC-SR04 Pin | Arduino Pin |
|------------|-------------|
| VCC        | 5V          |
| GND        | GND         |
| TRIG       | D9          |
| ECHO       | D10         |

---

### Servo

| Servo Kabel | Arduino Pin |
|------------|-------------|
| Rot (+)    | 5V (oder extern 5V) |
| Braun/Schwarz (GND) | GND |
| Gelb/Orange (Signal) | D11 |

**Hinweis:** Bei instabilem Verhalten des Servos unbedingt eine externe 5V-Quelle verwenden und GND verbinden.

---

### OLED Display (I2C)

| OLED Pin | Arduino Pin |
|---------|-------------|
| GND     | GND         |
| VCC     | 5V          |
| SCL     | A5 (Uno)    |
| SDA     | A4 (Uno)    |

Adresse: `0x3C`

---

### Taster (Start / Stop)

| Taster Pin | Arduino Pin |
|-----------|-------------|
| Pin 1     | D4          |
| Pin 2     | GND         |

Der interne Pullup-Widerstand des Arduino wird verwendet.

---

## Benötigte Bibliotheken

Über den Arduino Library Manager installieren:

- Servo
- Adafruit GFX Library
- Adafruit SSD1306

Zusätzlich erforderlich:
- Wire (Standardbibliothek)

---

## Bedienung

1. Arduino starten
2. OLED zeigt **RADAR READY**
3. Taster drücken → Radar startet
4. Servo sweeped von 0–180°
5. Objekte werden als Blips angezeigt
6. Taster erneut drücken → Radar stoppt, Servo zentriert sich

---

## Funktionsweise (Kurzfassung)

- Der Servo bewegt den Ultraschallsensor schrittweise
- Pro Winkel wird eine gefilterte Distanz gemessen
- Jeder gültige Treffer erzeugt einen Blip
- Blips werden zeitlich verwaltet und ausgeblendet
- Das OLED wird mit fester Bildrate aktualisiert

---

## Wichtige Parameter im Code

```cpp
const int maxDistance = 50;      // Maximale Reichweite in cm
const unsigned long fadeTime = 3000; // Ausblendzeit der Blips
const unsigned long debounceDelayMs = 35; // Taster-Entprellung
```

---

## Hinweise & Tipps

- Ultraschall reagiert empfindlich auf schräge oder weiche Oberflächen
- Kleine Winkel-Schritte (2°) sorgen für ein ruhigeres Bild
- Für stabile Messungen ausreichend Abstand zwischen Sensor und Servo-Horn lassen
- OLED-Updates bewusst begrenzt, um Flackern zu vermeiden

---

## Lizenz

Dieses Projekt ist frei nutzbar zu Lern- und Bastelzwecken.
Keine Gewähr für Schäden an Hardware.

Viel Spaß beim Basteln!
