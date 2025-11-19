#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// WiFi
const char *ssid = "Sup";
const char *password = "12345678";

// MQTT
const char *mqtt_broker = "public.mqtthq.com";
const char *topic = "Data_dht";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

// Sleep time
#define uS_TO_S_FACTOR 1000000ULL
#define TIME_TO_SLEEP 600    // 600 sec = 10 minutes

void setupWifi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
}

void setup() {
  Serial.begin(9600);
  dht.begin();

  // Wake reason
  esp_sleep_wakeup_cause_t wake_reason = esp_sleep_get_wakeup_cause();
  Serial.print("Wake up reason: ");
  Serial.println(wake_reason);

  setupWifi();

  client.setServer(mqtt_broker, mqtt_port);

  while (!client.connected()) {
    String client_id = "esp32-client-" + String(WiFi.macAddress());
    if (client.connect(client_id.c_str())) {
      Serial.println("Connected to MQTT");
    } else {
      Serial.println("MQTT connection failed");
      delay(2000);
    }
  }

  // Read sensor
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT11!");
  } else {
    Serial.println("Temperature: " + String(temperature));
    Serial.println("Humidity: " + String(humidity));

    // JSON payload
    String payload = "{\"temperature\": ";
    payload += temperature;
    payload += ", \"humidity\": ";
    payload += humidity;
    payload += "}";

    client.publish(topic, payload.c_str());
    Serial.println("Data published!");
  }

  // Deep sleep
  Serial.println("Entering deep sleep...");
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}

void loop() {
  // Will never reach here in deep sleep mode
}
