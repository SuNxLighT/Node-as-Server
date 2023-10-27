#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>

ESP8266WebServer server(80);
DHT dht14(D4, DHT11);

const int ledPin = D6;
bool isLedOn = false;

void init_wifi(String ssid, String password) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected to " + ssid);
  Serial.println("IP address: " + WiFi.localIP().toString());
}

void setup(void) {
  Serial.begin(115200);
  init_wifi("SUN", "1212312121");
  pinMode(ledPin, OUTPUT);
  dht14.begin();

  server.on("/", HTTP_GET, []() {
    float temperature = dht14.readTemperature();
    float humidity = dht14.readHumidity();

    String ledStatus = isLedOn ? "On" : "Off";
    String html = "<html><body style='display: flex; flex-direction: column; align-items: center; justify-content: center; height: 100vh;'>";
    html += "<div style='text-align: center; background-color: #f5f5f5; border: 5px solid #ccc; padding: 20px; border-radius: 10px; font-family: Arial, sans-serif;'>";
    html += "<p style='font-size: 20px;'>Temperature: <span id='temperature'>" + String(temperature, 2) + " C</span></p>";
    html += "<p style='font-size: 20px;'>Humidity: <span id='humidity'>" + String(humidity, 2) + " %</span></p>";
    html += "<p style='font-size: 20px;'>LED Status: <span id='ledStatus'>" + ledStatus + "</span></p>";
    html += "<button style='font-size: 20px; margin-top: 20px;' onclick='toggleLed()'>Toggle LED</button>";
    html += "</div></body><script>function toggleLed() {fetch('/ToggleLed').then(response => response.json()).then(data => {document.getElementById('ledStatus').textContent = data.status;});}</script></html>";

    html += "<script>setInterval(() => fetch('/TempHumi').then(response => response.json()).then(data => { document.getElementById('temperature').textContent = data.temperature.toFixed(2) + ' C'; document.getElementById('humidity').textContent = data.humidity.toFixed(2) + ' %'; }), 500);</script>";
    server.send(200, "text/html", html);
  });

  server.on("/ToggleLed", HTTP_GET, []() {
    isLedOn = !isLedOn;
    digitalWrite(ledPin, isLedOn ? HIGH : LOW);
    String ledStatus = isLedOn ? "On" : "Off";
    String responseJson = "{\"status\":\"" + ledStatus + "\"}";
    server.send(200, "application/json", responseJson);
  });

  server.on("/TempHumi", HTTP_GET, []() {
    float temperature = dht14.readTemperature();
    float humidity = dht14.readHumidity();
    String TempHum = "{\"temperature\":" + String(temperature, 2) + ",\"humidity\":" + String(humidity, 2) + "}";
    server.send(200, "application/json", TempHum);
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
}