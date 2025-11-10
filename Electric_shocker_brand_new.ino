#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "esp_wifi.h"

#define SHOCKER_PIN 12 //Shocker

const char *ssid = "Lâm Tiến Dũng";
const char *password = "12345678";

AsyncWebServer server(80);

bool shockerOn = false;
int failedAttempts = 0;
const String correctPassword = "1234";

String shockerPage() {
  String html = R"rawliteral(
  <!DOCTYPE HTML>
  <html>
  <head>
    <meta charset="UTF-8">
    <title>Electric Shocker Control</title>
    <style>
      body {
        background: radial-gradient(circle at center, #ff4b5c, #1a1a1a);
        font-family: Arial, sans-serif;
        text-align: center;
        margin: 0;
        padding: 0;
        color: white;
      }
      h1 {
        margin-top: 30px;
        font-size: 2em;
        text-shadow: 0 0 10px red;
      }
      .warning {
        background: rgba(255,0,0,0.6);
        padding: 15px;
        margin: 20px auto;
        border-radius: 10px;
        width: 80%;
        font-weight: bold;
        font-size: 1.2em;
        box-shadow: 0 0 15px #ff0000;
      }
      form {
        background: rgba(0, 0, 0, 0.4);
        padding: 20px;
        margin: 30px auto;
        border-radius: 15px;
        width: 300px;
      }
      input[type=password] {
        width: 80%;
        padding: 10px;
        border-radius: 5px;
        border: none;
        margin: 10px 0;
      }
      input[type=submit], .btn-off {
        background: #ffcc70;
        color: #222;
        padding: 10px 20px;
        border: none;
        border-radius: 25px;
        cursor: pointer;
        font-size: 1em;
        transition: transform 0.2s;
      }
      input[type=submit]:hover, .btn-off:hover {
        transform: scale(1.05);
      }
      .status {
        margin-top: 20px;
        font-size: 1.3em;
      }
    </style>
  </head>
  <body>
    <h1> Electric Shocker Login Control </h1>
    <div class="warning">DANGER: Wrong password 3 times will ACTIVATE the SHOCKER!</div>
    <form action="/login" method="POST">
      <input type="password" name="password" placeholder="Enter Password" required><br>
      <input type="submit" value="Login">
    </form>
    <div class="status">Shocker is currently: 
      <strong style="color:%STATE_COLOR%">%STATE_TEXT%</strong>
    </div>
    <br>
    <form action="/shockerOff" method="POST">
      <input type="submit" class="btn-off" value="Turn OFF Shocker">
    </form>
  </body>
  </html>
  )rawliteral";

  html.replace("%STATE_TEXT%", shockerOn ? "ACTIVE" : "OFF");
  html.replace("%STATE_COLOR%", shockerOn ? "#FF0000" : "#00FF99");

  return html;
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  pinMode(SHOCKER_PIN, OUTPUT);
  digitalWrite(SHOCKER_PIN, LOW);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  esp_wifi_set_ps(WIFI_PS_NONE);  //stable pin state

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());

  //Main page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", shockerPage());
  });

  //Handle login
  server.on("/login", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("password", true)) {
      String pass = request->getParam("password", true)->value();
      if (pass == correctPassword) {
        failedAttempts = 0;  // reset attempts
      } else {
        failedAttempts++;
        if (failedAttempts >= 3) {
          shockerOn = true;
          digitalWrite(SHOCKER_PIN, HIGH);
        }
      }
    }
    request->redirect("/");
  });

  //Turn off shocker
  server.on("/shockerOff", HTTP_POST, [](AsyncWebServerRequest *request) {
    shockerOn = false;
    failedAttempts = 0;
    digitalWrite(SHOCKER_PIN, LOW);
    request->redirect("/");
  });

  server.begin();
}

//Server handling automatically
void loop() {
}
