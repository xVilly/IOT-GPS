#include <SoftwareSerial.h>
#include <TinyGPS++.h>

static const int RXPin = 5, TXPin = 4;
static const uint32_t GPSBaud = 9600;

TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);

void setup() {
  Serial.begin(9600);
  ss.begin(GPSBaud);
  Serial.println("Oczekiwanie na dane GPS...");
}

void loop() {
  while (ss.available() > 0) {
    char c = ss.read();
    gps.encode(c);
  }

  // Co sekundę pokazuj dane, niezależnie czy się zmieniły
  static unsigned long last = 0;
  if (millis() - last > 1000) {
    last = millis();

    Serial.println(F("=== GPS STATUS ==="));

    // Lokalizacja
    if (gps.location.isValid()) {
      Serial.print(F("Lokalizacja: "));
      Serial.print(gps.location.lat(), 6);
      Serial.print(F(", "));
      Serial.println(gps.location.lng(), 6);
    } else {
      Serial.println(F("Lokalizacja: NIEZNANA"));
    }

    // Satelity
    if (gps.satellites.isValid()) {
      Serial.print(F("Satelity: "));
      Serial.println(gps.satellites.value());
    } else {
      Serial.println(F("Satelity: brak danych"));
    }

    // HDOP
    if (gps.hdop.isValid()) {
      Serial.print(F("HDOP: "));
      Serial.println(gps.hdop.hdop());
    } else {
      Serial.println(F("HDOP: brak danych"));
    }

    // Czas
    if (gps.time.isValid()) {
      Serial.print(F("Czas UTC: "));
      if (gps.time.hour() < 10) Serial.print('0');
      Serial.print(gps.time.hour());
      Serial.print(':');
      if (gps.time.minute() < 10) Serial.print('0');
      Serial.print(gps.time.minute());
      Serial.print(':');
      if (gps.time.second() < 10) Serial.print('0');
      Serial.println(gps.time.second());
    } else {
      Serial.println(F("Czas: brak danych"));
    }

    Serial.println(); // pusty wiersz
  }
}