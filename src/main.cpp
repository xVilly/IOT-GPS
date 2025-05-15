#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

static const int RXPin = 5, TXPin = 4;
static const uint32_t GPSBaud = 9600;

const char* ssid = "iPhone (Michał)";
const char* password = "raptor123";
const char* mqtt_server = "broker.hivemq.com";

TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Łączenie z WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi połączone");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Łączenie z MQTT...");
    if (client.connect("GPSClient")) {
      Serial.println("połączono");
    } else {
      Serial.print("błąd, rc=");
      Serial.print(client.state());
      Serial.println(" próbuję ponownie za 5s");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  ss.begin(GPSBaud);
  Serial.println("Oczekiwanie na dane GPS...");

  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  while (ss.available() > 0) {
    char c = ss.read();
    gps.encode(c);
  }

  static unsigned long last = 0;
  if (millis() - last > 1000) {
    last = millis();

    Serial.println(F("=== GPS STATUS ==="));

    float lat = gps.location.isValid() ? gps.location.lat() : 0.0;
    float lng = gps.location.isValid() ? gps.location.lng() : 0.0;
    int sats = gps.satellites.isValid() ? gps.satellites.value() : 0;
    int hdop = gps.hdop.isValid() ? gps.hdop.hdop() : 0;
    char czas[9] = "00:00:00";
    if (gps.time.isValid()) {
      snprintf(czas, sizeof(czas), "%02d:%02d:%02d", gps.time.hour(), gps.time.minute(), gps.time.second());
    }

    // Serial print jak wcześniej...
    if (gps.location.isValid()) {
      Serial.print(F("Lokalizacja: "));
      Serial.print(lat, 6);
      Serial.print(F(", "));
      Serial.println(lng, 6);
    } else {
      Serial.println(F("Lokalizacja: NIEZNANA"));
    }
    if (gps.satellites.isValid()) {
      Serial.print(F("Satelity: "));
      Serial.println(sats);
    } else {
      Serial.println(F("Satelity: brak danych"));
    }
    if (gps.hdop.isValid()) {
      Serial.print(F("HDOP: "));
      Serial.println(hdop);
    } else {
      Serial.println(F("HDOP: brak danych"));
    }
    if (gps.time.isValid()) {
      Serial.print(F("Czas UTC: "));
      Serial.println(czas);
    } else {
      Serial.println(F("Czas: brak danych"));
    }
    Serial.println();

    // Wysyłka MQTT jako JSON
    char payload[256];
    snprintf(payload, sizeof(payload),
      "{\"lat\":%.6f,\"lng\":%.6f,\"sats\":%d,\"hdop\":%d,\"czas\":\"%s\"}",
      lat, lng, sats, hdop, czas
    );
    client.publish("gps/status", payload);
  }
}