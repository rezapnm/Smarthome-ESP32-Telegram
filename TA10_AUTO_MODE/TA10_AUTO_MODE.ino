/*
 * task     : 1. Rapihkan tiap tiap fungsi, pindahkan tab baru atau buat void baru------------------------------------------------- DONE                            
 *            2. Tambahkan indikator saat sistem terkoneksi dengan wifi (Gunakan led RGB)-------------------------------------------DONE
 *            3. Cek kondisi terakhir mode saat tombol start ditekan.---------------------------------------------------------------DONE
 *            4. Selesaikan fungsi monitoring saat mode otomatis--------------------------------------------------------------------DONE
 *            5. Atasi delay saat transisi dari mode manual ke mode otomatis--------------------------------------------------------  2
 *            6. Atasi pengiriman pesan double saat transisi dari mode otomatis ke mode manual--------------------------------------DONE
 *            7. GUNAKAN RTC untuk simpan semua kondisi, agar sistem tidak mengulang saat di restart--------------------------------  3
 *            8. Update nama variable, sesuai dengan yang terletak di pcb-----------------------------------------------------------DONE
 *            9. Menambah button front lamp pada mode manual------------------------------------------------------------------------DONE
 * 
 * negative : 1. Belum ada mode deep sleep------------------------------------------------------------------------------------------TINGGAL IMPLEMENTASI DARI PROGRAM YANG LAIN                                
 *            2. Cari referensi tentang realtime clock untuk mengatur jadwal deepsleep----------------------------------------------DONE
 *            3. Sistem harus bisa reconnect wifi terus menerus saat jaringan putus, dan beri indikator saat jaringan hilang.-------DONE
 *            4. saat user baru masuk sistem, user lain masih menerima pesan state terakhir-----------------------------------------  1
 *            5. Setelah memanggil realtime_state pada automatic mode, selalu terkirim message dan keyboard dari mode otomatis------DONE
 */

#include <DHT.h>
#include <DHT_U.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

//===== KONFIGURASI TELEGRAM DAN WIFI =====
#define BOTtoken "1259079245:AAG0L1KezEMraQs6WKHIcR3CQglVgysu014"
//char ssid[] = "kimi_connection";
//char password[] = "covid-19";
char ssid[] = "Loh";
char password[] = "purnamaa";
//=========================================

//===== PIN INDIKATOR =====
#define led_wifi_connect 4
#define led_wifi_disconnect 12
#define buzzer 15
#define led_manual 14
#define led_auto 21
//=========================

//===== PIN INPUT =====
#define DHTPIN 22
#define LDR_1 2
#define LDR_2 0
//=====================

//===== PIN OUTPUT =====
#define LAMPU_1 23
#define LAMPU_2 18
#define KIPAS 5
//======================

float humidity, temperature;
String dHTdata, userON, userOFF, lastMode;
int Bot_mtbs = 0.0001;
bool Start = false;
bool intoAuto = false;
bool automatic = false;
bool manual = false;
long Bot_lasttime;

DHT dht(DHTPIN, DHT11);
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

void reconnect()
{
  digitalWrite(led_wifi_disconnect,HIGH);
  digitalWrite(led_wifi_connect,LOW);
  Serial.println("WiFi Terputus !");
  while (WiFi.status() != WL_CONNECTED) 
  {
    digitalWrite(buzzer,HIGH);
    delay(500);
    digitalWrite(buzzer,LOW);
    Serial.print(".");
  }
  digitalWrite(buzzer, HIGH);
  delay(2000);
  digitalWrite(buzzer,LOW);
  digitalWrite(led_wifi_disconnect,LOW);
  digitalWrite(led_wifi_connect,HIGH);
  Serial.println("WiFi Tersambung Kembali");
}

void handleNewMessages(int numNewMessages)
{
  for(int i=0; i<numNewMessages; i++)                 /// karena numNewMessages = 1, maka pengulangan (handleMessage) hanya sekali !
  {
    String chat_id = String(bot.messages[i].chat_id);
    String chat_id_reza = "958206912";
    String chat_id_jessica = "1069252451";
    String text = bot.messages[i].text;
    String from_name = bot.messages[i].from_name;
    bool main_lamp_status, front_lamp_status, fan_status;
    if (from_name == "" ) from_name = "System";
//---------------------------------------------------------------------------------------
    if (text == "/start")
    {
      if (lastMode == "manual")
      {
        manual = true;
      }
      if (lastMode == "automatic")
      {
        intoAuto = true;
      }
    }
//---------------------------------------------------------------------------------------
    if (text == "/MANUAL_MODE")
    {
      digitalWrite(led_auto,LOW);
      digitalWrite(led_manual,HIGH);
             Serial.println("Keluar auto mode");
             bot.sendMessage(chat_id, "Keluar auto mode");
             //bot.sendMessage(chat_id_reza, "Keluar auto mode");
             automatic = false;
             manual = true;
    }
//---------------------------------------------------------------------------------------
    if (text == "/AUTOMATIC_MODE" || intoAuto == true)
    {
      lastMode = "automatic";
      automatic = true;
      Serial.println(from_name + " : Auto Mode");
      String welcome = "Automatic Mode is On \nActivated by : " + from_name;
      String autoFeedback = "[[\"/MANUAL_MODE\"],";
             autoFeedback += "[\"/REALTIME_STATE\"]]";
      bot.sendMessageWithReplyKeyboard(chat_id_reza, welcome,"", autoFeedback, true);           
      bot.sendMessageWithReplyKeyboard(chat_id_jessica, welcome,"", autoFeedback, true);
      intoAuto = false;
    }
//---------------------------------------------------------------------------------------
    if (manual == true)
    {
      lastMode = "manual";
      Serial.println(from_name+" : MANUAL_MODE");
      digitalWrite(led_manual, HIGH);
      String welcome = "Manual Mode is On \n";
             welcome += "Activated by : " + from_name;

      String manualFeedback = "[[\"/MAIN_LAMP_ON\", \"/MAIN_LAMP_OFF\"],";
             manualFeedback += "[\"/FRONT_LAMP_ON\", \"/FRONT_LAMP_OFF\"],";
             manualFeedback += "[\"/FAN_ON\", \"/FAN_OFF\"],";
             manualFeedback += "[\"/TEMPERATURE\", \"/REALTIME_STATE\"],";
             manualFeedback += "[\"/AUTOMATIC_MODE\"]]";
      bot.sendMessageWithReplyKeyboard(chat_id_reza, welcome, "", manualFeedback, true);
      bot.sendMessageWithReplyKeyboard(chat_id_jessica, welcome, "", manualFeedback, true);
      manual = false;
    }
//---------------------------------------------------------------------------------------
    if(text == "/MAIN_LAMP_ON")
    {
      main_lamp_status = digitalRead(LAMPU_1);
      Serial.println(from_name + " : MAIN LAMP ON ");
      if (main_lamp_status == 0)
      {
        userON = from_name;
        digitalWrite(LAMPU_1,HIGH);
        digitalWrite(buzzer,HIGH);
        delay(200);
        digitalWrite(buzzer,LOW);
        String main_lamp_on = "Main lamp is On";
        bot.sendMessage(chat_id, main_lamp_on);
       }
       else if (main_lamp_status == 1)
       {
         String main_lamp_already_on = "Main lamp already On \nUser : " + userON;
         bot.sendMessage(chat_id, main_lamp_already_on);
       }
    }
//---------------------------------------------------------------------------------------
    if (text == "/MAIN_LAMP_OFF")
    {
      main_lamp_status = digitalRead(LAMPU_1);
      Serial.println(from_name + " : MAIN LAMP OFF ");
      if (main_lamp_status == 1)
      {
         userOFF = from_name;
         digitalWrite(LAMPU_1,LOW);
         digitalWrite(buzzer,HIGH);
         delay(200);
         digitalWrite(buzzer,LOW);
         String main_lamp_off = "Main Lamp is Off";
         bot.sendMessage(chat_id, main_lamp_off);
       }
       else if (main_lamp_status == 0)
       {
         String main_lamp_already_off = "Main lamp already Off \nUser : " + userOFF;
         bot.sendMessage(chat_id, main_lamp_already_off);
       }
    }
//---------------------------------------------------------------------------------------
    if(text == "/FRONT_LAMP_ON")
    {
      front_lamp_status = digitalRead(LAMPU_2);
      Serial.println(from_name + " : FRONT LAMP ON");
      if(front_lamp_status == 0){
        userON = from_name;
        digitalWrite(LAMPU_2,HIGH);
        digitalWrite(buzzer,HIGH);
        delay(200);
        digitalWrite(buzzer,LOW);
        String front_lamp_on = "Front Lamp is On";
        bot.sendMessage(chat_id, front_lamp_on);
      }
      else if(front_lamp_status == 1){
        String front_lamp_already_on = "Front lamp already On \nUser : " + userON;
        bot.sendMessage(chat_id, front_lamp_already_on);
      }
    }
//---------------------------------------------------------------------------------------
    if(text == "/FRONT_LAMP_OFF")
    {
      front_lamp_status = digitalRead(LAMPU_2);
      Serial.println(from_name + " : FRONT LAMP OFF");
      if(front_lamp_status == 1){
        userOFF = from_name;
        digitalWrite(LAMPU_2,LOW);
        digitalWrite(buzzer,HIGH);
        delay(200);
        digitalWrite(buzzer,LOW);
        String front_lamp_off = "Front Lamp is Off";
        bot.sendMessage(chat_id, front_lamp_off);
      }
      else if(front_lamp_status == 0){
        String front_lamp_already_off = "Front lamp already Off \nUser : " + userOFF;
        bot.sendMessage(chat_id, front_lamp_already_off);
      }
    }
//---------------------------------------------------------------------------------------
    if(text == "/FAN_ON")
    {
      fan_status = digitalRead(KIPAS);
      Serial.println(from_name + " : FAN ON");
      if(fan_status == 0)
      {
        userON = from_name;
        digitalWrite(KIPAS,HIGH);
        digitalWrite(buzzer,HIGH);
        delay(200);
        digitalWrite(buzzer,LOW);
        String fan_on = "Fan is On";
        bot.sendMessage(chat_id, fan_on);
      }
      else if(fan_status == 1){
        String fan_already_on = "Fan already On \nUser : " + userON;
        bot.sendMessage(chat_id,fan_already_on);
      }
    }
//---------------------------------------------------------------------------------------
    if(text == "/FAN_OFF")
    {
      fan_status = digitalRead(KIPAS);
      Serial.println(from_name + " : FAN_OFF");
      if(fan_status == 1)
      {
        userOFF = from_name;
        digitalWrite(KIPAS,LOW);
        digitalWrite(buzzer,HIGH);
        delay(200);
        digitalWrite(buzzer,LOW);
        String fan_off = "Fan is Off";
        bot.sendMessage(chat_id, fan_off);
      }
      else if(fan_status == 0)
      {
        String fan_already_off = "Fan already Off \nUser : " + userON;
        bot.sendMessage(chat_id, fan_already_off);
      }
    }
//---------------------------------------------------------------------------------------
    if (text == "/TEMPERATURE")
    {
      humidity = dht.readHumidity();
      temperature = dht.readTemperature();
      Serial.println(from_name + " : TEMPERATURE");
      dHTdata = String() + "Temperature : " + temperature + " Celcius\n" + "Humidity : " + humidity + "%\n";
      bot.sendMessage(chat_id,dHTdata);
    }
//---------------------------------------------------------------------------------------
    if (text == "/REALTIME_STATE")
    {
      //manual = false;
      //automatic = true;
      main_lamp_status = digitalRead(LAMPU_1);
      front_lamp_status = digitalRead(LAMPU_2);
      fan_status = digitalRead(KIPAS);
      Serial.println(from_name + " : STATUS");
      humidity = dht.readHumidity();
      temperature = dht.readTemperature();
      String kondisi_main, kondisi_front, kondisi_kipas;
      if (main_lamp_status == 1)
      {
        kondisi_main = "On";
      }
      if (main_lamp_status == 0)
      {
        kondisi_main = "Off";
      }
      if (front_lamp_status == 1)
      {
        kondisi_front = "On";
      }
      if(front_lamp_status == 0)
      {
        kondisi_front = "Off";
      }
      if (fan_status == 1)
      {
        kondisi_kipas = "On";
      }
      if (fan_status == 0)
      {
        kondisi_kipas = "Off";
      }
      String systems_status = "*** REALTIME STATE ***\n";
             systems_status += "Main Lamp\t: " + kondisi_main + "\n";
             systems_status += "Front Lamp\t: " + kondisi_front + "\n";
             systems_status += "Fan\t:" + kondisi_kipas + "\n";
             systems_status += String() + "Temperature\t: " + temperature + " Celcius\n";
             systems_status += String() + "Humidity\t: " + humidity + "%";  
      bot.sendMessage(chat_id, systems_status);
    }

  }
}
void setup() {
  // put your setup code here, to run once:
Serial.begin(115200);
pinMode(led_wifi_connect,OUTPUT);
pinMode(led_wifi_disconnect,OUTPUT);
pinMode(buzzer,OUTPUT);
pinMode(led_manual,OUTPUT);
pinMode(led_auto,OUTPUT);
pinMode(LAMPU_1,OUTPUT);
pinMode(LAMPU_2,OUTPUT);
pinMode(KIPAS,OUTPUT);
pinMode(LDR_1,INPUT);
pinMode(LDR_2,INPUT);
//DHT BELUM MASUK PIN MODE

manual = true;
Serial.print("Connecting Wifi: ");
Serial.println(ssid);

WiFi.mode(WIFI_STA);
WiFi.begin(ssid, password);

while (WiFi.status() != WL_CONNECTED) 
{
    digitalWrite(led_wifi_connect,LOW);
    digitalWrite(led_wifi_disconnect,HIGH);
    Serial.print(".");
    digitalWrite(buzzer,HIGH);
    delay(500);
    digitalWrite(buzzer,LOW);
}
handleNewMessages(1);
Serial.println("");
Serial.println("WiFi connected");
Serial.print("IP address: ");
Serial.println(WiFi.localIP());
digitalWrite(led_wifi_connect,HIGH);
digitalWrite(led_wifi_disconnect,LOW);
dht.begin();
}

void loop() {
  // put your main code here, to run repeatedly:

  if (WiFi.status() != WL_CONNECTED) 
  {
      digitalWrite(led_wifi_connect, LOW);
      digitalWrite(led_wifi_disconnect,HIGH);
      Serial.print("Re-connecting Wifi: ");
      Serial.println(ssid);
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid, password);
      while (WiFi.status() != WL_CONNECTED) 
     {
        digitalWrite(buzzer,HIGH);
        delay(500);
        digitalWrite(buzzer,LOW);
        Serial.print(".");
     }
      digitalWrite(buzzer, HIGH);
      delay(2000);
      digitalWrite(buzzer,LOW);
      digitalWrite(led_wifi_connect, HIGH);
      digitalWrite(led_wifi_disconnect,LOW);
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
  }

while(WiFi.status() == WL_CONNECTED)
{
  if (millis() > Bot_lasttime + Bot_mtbs)  
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
     while(numNewMessages) 
     {
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
     }
     
     if(WiFi.status() == WL_CONNECTED && automatic == true)
     {
      //handleNewMessages(numNewMessages);
      
//        Serial.print("Indicator On");
//        digitalWrite(led_auto,HIGH);
//        digitalWrite(led_manual,LOW);
//        digitalWrite(LAMPU_1,HIGH);
//        delay(500);
//        digitalWrite(LAMPU_1,LOW);
//        delay(500);

//        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
//        handleNewMessages(numNewMessages);
        Serial.println("\n====== AUTOMATIC MODE ======");
//        Serial.print("LDR 1 = ");
//        Serial.print(nilaiLDR_1);
//        Serial.print("\tLDR 2 = ");
//        Serial.print(nilaiLDR_2);
        Serial.print("\tTEMPERATURE = ");
        Serial.println(temperature);
        if (temperature >= 27){
          digitalWrite(KIPAS, HIGH);
        }
        else {
          digitalWrite(KIPAS, LOW);
        }
     }
     //manual = "true";
     Bot_lasttime = millis();
  }
  }
}
