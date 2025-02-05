#include <WebServer.h>
#include "HardwareSerial.h"
#include <WiFi.h>
#include <esp32cam.h>
 #include "FirebaseESP32.h"
 //kết nối wifi
const char* ssid = "Galaxy A32 5G";
const char* password = "88888888";
// const char* ssid = "Tuan";
// const char* password = "tuan@12345";
String a;
String receive;
String data = ""; 
//kết nối firebase:
#define FIREBASE_AUTH "AIzaSyAtHo_l-k8ehqn5DM6o8dsOouAsIZmHBno" //lấy api key
#define FIREBASE_HOST "https://gymapp-e8630-default-rtdb.asia-southeast1.firebasedatabase.app/" //lấy url firebase
FirebaseData firebaseData;
FirebaseJson json;
FirebaseAuth auth;
FirebaseConfig config;

WebServer server(80);
static auto loRes = esp32cam::Resolution::find(320, 240);
static auto midRes = esp32cam::Resolution::find(350, 530);
static auto hiRes = esp32cam::Resolution::find(800, 600);
void serveJpg()
{
  auto frame = esp32cam::capture();
  if (frame == nullptr) {
    Serial.println("CAPTURE FAIL");
    server.send(503, "", "");
    return;
  }
  Serial.printf("CAPTURE OK %dx%d %db\n", frame->getWidth(), frame->getHeight(),
                static_cast<int>(frame->size()));
 
  server.setContentLength(frame->size());
  server.send(200, "image/jpeg");
  WiFiClient client = server.client();
  frame->writeTo(client);
}
 
void handleJpgLo()
{
  if (!esp32cam::Camera.changeResolution(loRes)) {
    Serial.println("SET-LO-RES FAIL");
  }
  serveJpg();
}
 
void handleJpgHi()
{
  if (!esp32cam::Camera.changeResolution(hiRes)) {
    Serial.println("SET-HI-RES FAIL");
  }
  serveJpg();
}
 
void handleJpgMid()
{
  if (!esp32cam::Camera.changeResolution(midRes)) {
    Serial.println("SET-MID-RES FAIL");
  }
  serveJpg();
}
 
 
void  setup(){
  Serial.begin(115200);
  Serial.println();
  {
    //Cam web server 
    using namespace esp32cam;
    Config cfg;
    cfg.setPins(pins::AiThinker);
    cfg.setResolution(hiRes);
    cfg.setBufferCount(2);
    cfg.setJpeg(80);
 
    bool ok = Camera.begin(cfg);
    Serial.println(ok ? "CAMERA OK" : "CAMERA FAIL");
  }
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.print("http://");
  Serial.println(WiFi.localIP());
  Serial.println("  /cam-lo.jpg");
  Serial.println("  /cam-hi.jpg");
  Serial.println("  /cam-mid.jpg");
 
  //server.on("/cam-lo.jpg", handleJpgLo);
  server.on("/cam-hi.jpg", handleJpgHi);
  //server.on("/cam-mid.jpg", handleJpgMid);
 
  server.begin();
  //end cam web server
  //khởi tạo kết nối firebase
  config.api_key = FIREBASE_AUTH ;

  /* Assign the RTDB URL (required) */
  config.database_url = FIREBASE_HOST;
  Firebase.begin(&config, &auth);
  //kiểm tra kết nối
  if (Firebase.signUp(&config, &auth, "", "")){
  Serial.println("ok");
  //signupOK = true;
  }
  else{
    Serial.printf("Error");
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  //giữ kết nối
  Firebase.reconnectWiFi(true);
}
 void loop() {

    server.handleClient();
 //gui data
    Firebase.getString(firebaseData, "/communicate/state");
    String send=firebaseData.stringData();
    Serial.write(send.c_str());
    if (send !="wait")
    {
      Firebase.setString(firebaseData,"/communicate/state","wait");
    }
    if (Serial.available() > 0) // Nếu có lệnh gửi đến
    {
      Firebase.setString(firebaseData,"/communicate/ok","true");
      char c = Serial.read(); //Đọc từng kí tự
      //String b = Serial.readString();
      //Firebase.setString(firebaseData,"/communicate/test",b);
      //Firebase.setString(firebaseData,"/communicate/char",String(c));
      // Kiểm tra kết thúc câu lệnh    
      a = String(c);
      Firebase.setString(firebaseData,"/communicate/test",a);
      data = data + a;
      Firebase.setString(firebaseData,"/communicate/user",data);     
    }
    else{
      Firebase.setString(firebaseData,"/communicate/ok","false");
    }

}
