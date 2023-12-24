#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define OLED_Address 0x3C

MAX30105 particleSensor;
Adafruit_SSD1306 oled(128, 32, &Wire, OLED_Address);

const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;

float beatsPerMinute;
int beatAvg;

bool sensorEnabled = false;

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println("Initializing...");
  oled.begin(SSD1306_SWITCHCAPVCC, OLED_Address);

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    sensorEnabled = false;
    while (1)
      ;
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");
}



void loop() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    if (command.equals("off")) {
      sensorEnabled = false;
      digitalWrite(LED_BUILTIN, LOW);
      particleSensor.shutDown();
      oled.clearDisplay();
      oled.setTextSize(1);
      oled.setTextColor(SSD1306_WHITE);
      oled.setCursor(0, 0);
      oled.println("Sensor is off");
      oled.display();
    } else if (command.equals("on")) {
      sensorEnabled = true;
      digitalWrite(LED_BUILTIN, HIGH);
      particleSensor.setup();
      particleSensor.setPulseAmplitudeRed(0x0A);
      particleSensor.setPulseAmplitudeGreen(0);
    }
  }

  if (sensorEnabled == true) {
    long irValue = particleSensor.getIR();

    if (checkForBeat(irValue)) {
      long delta = millis() - lastBeat;
      lastBeat = millis();

      beatsPerMinute = 60 / (delta / 1000.0);

      if (beatsPerMinute < 255 && beatsPerMinute > 20) {
        rates[rateSpot++] = static_cast<byte>(beatsPerMinute);
        rateSpot %= RATE_SIZE;

        beatAvg = 0;
        for (byte x = 0; x < RATE_SIZE; x++)
          beatAvg += rates[x];
        beatAvg /= RATE_SIZE;
      }
    }

    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(SSD1306_WHITE);
    oled.setCursor(0, 0);

    if (irValue < 50000) {
      oled.println("No Finger");
    } else {
      oled.println("Heart Rate: ");
      oled.print(beatAvg);
      oled.print(" BPM");
    }
    oled.display();
    Serial.println(beatAvg);
  }
}
