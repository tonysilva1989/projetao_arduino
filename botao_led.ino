/*
  
                                     :::::::::                                    
                                   :::::::::::::                                  
                                 :::::::::::::::::                                
                                ::::::::::::###::::                               
                               :::::::::::######::::                              
                              :::::::::::########::::                             
                              ::::::::: ########:::::                             
                              ::::::::   #####:::::::                             
                              ::::::       #:::::::::                             
                              :::::        ::::::::::                             
                               ::::      :::::::::::                              
                                :::     :::::::::::                               
                                 :::::::::::::::::                                
                                   :::::::::::::                               ###
                                     :::::::::                                 ###
                                                                               ###
                                                                               ###
     :::         :::         ::             :::#:   ##         ##          ##  ###
   :::::::     :::::::::   ::::::  :::     ::###########    ########    ##########
  :::    ::   :::    :::   ::       :::   ::#   ###   ###  ###    ###  ###     ###
  :::::::::   ::      ::    ::::     ::: ::##   ###   ###  ##########  ###     ###
  ::          ::      ::      ::::    :::::##   ###   ###  ###         ###     ###
  :::   :::   :::    :::   ::  :::     :::###   ###   ###   ###   ###   ###   ####
   :::::::     :::::: ::    :::::      :: ###   ###   ###    #######     #########
                                      ::                                          
                                     ::                                           
                                    :::                                           
                                   :::                                            
  Projetão 2016.1: EasyMed - Botão com LED
  (versão sem os LEDs implementados e sem a conexão com o servidor)
*/

#include <ESP8266WiFi.h>
#include <Ticker.h>
//Definições de valores fixos e variáveis globais do código

                                      // Valores relativos ao botao
#define LONGPRESS_LEN 25              // Numero minimo de loops para o toque ser
                                      // considerado longo
#define DELAY 20                      // Delay per loop in ms

#define EV_NONE 0                     // Botao nao foi apertado
#define EV_SHORTPRESS 1               // Toque curto (menos que o minimo)
#define EV_LONGPRESS 2                // Toque longo

boolean button_was_pressed;           // Estado anterior do botao
int button_pressed_counter;           // Contador de loops do botão pressionado

                                      // Valores relativos a maquina de estados
#define SLEEP 0                       // Estado de economia
#define APP 1                         // Abertura do APP no celular
#define CONNECT 2                     // Conexao com a rede
#define SEND 3                        // Envio do pedido


int lastState = -1;                   // Estado anterior da máquina de estados
                                      // (iniciado em -1 para evitar que o bo-
                                      // tão conecte sozinho)
int currentState = SLEEP;             // Estado atual (inicia em SLEEP)

                                      // Valores relativos ao AP de setup
const char WiFiAPPSK[] = "easymed0";  // Senha da WiFi de setup

String nwID = "";                     // SSID da rede WiFi
String nwPW = "";                     // Senha da rede WiFi

WiFiServer server(80);                // Define a criação de um servidor WiFi
                                      // na porta 80;

boolean WiFiConnected;                // Define se o botão está conectado a uma
                                      // rede WiFi.

                                      // Valores relativos aos LEDs
int ledCFG = D5;                      // LED do estado de configuração
int ledCON = D6;                      // LED do estado de conexão
int ledSND = D7;                      // LED do estado de envio
int ledAPP = D8;                      // LED do estado de abertura do APP

boolean statusCFG = false;            // Status do LED de configuração
boolean statusCON = false;            // Status do LED de conexão
boolean statusSND = false;            // Status do LED de envio
boolean statusAPP = false;            // Status do LED de abertura do APP

Ticker blinker;                       // Pisca o LED

                                      // Valores relativos ao servidor
const char* host = "easymed-projetao.herokuapp.com";
                                      // Endereço do servidor
const int httpPort = 80;              // Porta
WiFiClient client;

void blinkCFG() {
  /*
    Função que serve para piscar o LED do status de configuração.
  */
  digitalWrite(ledCFG, statusCFG);
  statusCFG = !statusCFG;
}
void blinkCON() {
  /*
    Função que serve para piscar o LED do status de conexão.
  */
  digitalWrite(ledCON, statusCON);
  statusCON = !statusCON;
}
void blinkSND() {
  /*
    Função que serve para piscar o LED do status de envio do pedido.
  */
  digitalWrite(ledSND, statusSND);
  statusSND = !statusSND;
}
void blinkAPP() {
  /*
    Função que serve para piscar o LED do status de abertura do aplicativo.
  */
  digitalWrite(ledAPP, statusAPP);
  statusAPP = !statusAPP;
}

boolean lightsOff(){
  /*
    Função que serve apagar todos os leds (modo de espera / antes de iniciar um
    novo estado).
    
    Entradas: nenhuma
    Saída:    true (retorna apenas para encerrar a função)
  */
  digitalWrite(ledCFG, LOW);
  digitalWrite(ledCON, LOW);
  digitalWrite(ledSND, LOW);
  digitalWrite(ledAPP, LOW);
  return true;
}

boolean connectToServer(){
  /*
    Função que estabelece a conexão com o servidor.
    
    Entradas: nenhuma
    Saída:    true (retorna apenas para encerrar a função)
  */
  
  boolean isConnected = false;
  
  lightsOff();
  blinker.attach(0.2, blinkCON);
   
  while(!isConnected){
    if (!client.connect(host, httpPort)) {
      Serial.println("connection failed");
      continue;
    }
    
    isConnected = true;
  }
  
  blinker.detach();
  digitalWrite(ledCON, HIGH);
  
  return true;
}

boolean sendRequest(){
  /*
    Função que envia o pedido para o servidor e recebe a confrmação de que o
    pedido foi recebido.
    Nota: Protótipo, apenas envia uma solicitação GET comum ao servidor web.
    Entradas: nenhuma (no protótipo)
    Saída:    true (retorna apenas para encerrar a função)
  */
  boolean x = false;
  
  lightsOff();
  blinker.attach(0.2, blinkSND);

//curl -H "Content-Type: application/json" -X POST /P '{"nome_medicamento":"xyz","qtd_medicamento":"1","pagamento":"xyz","status":"xyz"}' https://easymed-projetao.herokuapp.com/pedidos
//{"nome_medicamento"=>"xyz", "qtd_medicamento"=>"1", "pagamento"=>"xyz", "status"=>"xyz", "pedido"=>{"nome_medicamento"=>"xyz", "qtd_medicamento"=>"1", "pagamento"=>"xyz", "status"=>"xyz"}}
  
  while(!x){
    // We now create a URI for the request
    String url = "/pedidos";
    
    String json = "{\"nome_medicamento\": \"xyz\",\"qtd_medicamento\": \"1\",\"pagamento\": \"xyz\",\"status\": \"xyz\"}";
    Serial.print("Requesting URL: ");
    Serial.println(url);
    
    // This will send the request to the server
    client.print("POST "); 
    client.print(url);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(host);
    client.println("User-Agent: ESP2688");
    client.println("Accept: application/json");
    client.print("Content-Lenght: ");
    client.println(json.length());
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();
    client.println(json);
    
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        continue;
      }
    }

    
    
    // Read all the lines of the reply from server and print them to Serial
    while(client.available()){
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
    
    Serial.println();
    Serial.println("closing connection");
    
    x = true;

  }
  blinker.detach();
  digitalWrite(ledSND, HIGH);
  return true;
}

boolean openApp(){
  /*
    Função que envia ao servidor o pedido de que o aplicativo seja aberto no aparelho do cliente.
    Nota: protótipo, sequer envia pedidos ao servidor (apenas aguarda 1500 milisegundos e acende a luz).
    Entradas: nenhuma (no protótipo)
    Saída:    true (retorna apenas para encerrar a função)
  */
  lightsOff();
  blinker.attach(0.2, blinkAPP);
  delay(1500);
  blinker.detach();
  digitalWrite(ledAPP, HIGH);
  return true;
}

int nextState(int currentState, int lastPress){
  /*
    Função que calcula o próximo estado da máquina baseado no estado atual e no
    botão pressionado (no botão com LED, no tipo de toque no botão: longo ou
    curto). Dado o estado atual (definido como uma constante pelo define) e o
    botão pressionado (ou tipo de toque no botão), um switch determina qual o
    próximo estado que a máquina deve entrar.
    
    Entradas:   int CurrentState (o estado atual)
                int lastPress (último botão / último toque)
    Saída:      int next (o próximo estado da máquina)
  */
  int next = 0;
  switch (currentState){
    case SLEEP:
    if (lastPress == EV_LONGPRESS){
    next = APP;
    }else if (lastPress == EV_SHORTPRESS){
    next = CONNECT;
    }else{
    next = SLEEP;
    }
    break;
    case APP:
    if (lastPress == EV_SHORTPRESS){
    next = SLEEP;
    }else{
    next = APP;
    }
    break;
    case CONNECT:
    if (lastPress == EV_LONGPRESS){
    next = SLEEP;
    }else if (lastPress == EV_SHORTPRESS){
    next = SEND;
    }else{
    next = CONNECT;
    }
    break;
    case SEND:
    if (lastPress == EV_NONE){
    next = SEND;
    }else{
    next = SLEEP;
    }
    break;
    default:
    next = SLEEP;
    break;
  }
  return next;
}

int handle_button(){
  /*
    Função que determina se o toque no botão foi longo (EV_LONGPRESS), curto
    (EV_SHORTPRESS) ou nenhum (EV_NONE). Entrada lida negada (!digitalRead) de-
    vido ao resistor interno da placa. Retirado de: http://goo.gl/lDTjTE
    
    Entradas: nenhuma
    Saída:    int event (tipo de toque, definido como constante acima)
  */
  int event;
  int button_now_pressed = !digitalRead(D3);

  if (!button_now_pressed && button_was_pressed) {
    if (button_pressed_counter < LONGPRESS_LEN)
      event = EV_SHORTPRESS;
    else
      event = EV_LONGPRESS;
  }
  else
    event = EV_NONE;

  if (button_now_pressed)
    ++button_pressed_counter;
  else
    button_pressed_counter = 0;

  button_was_pressed = button_now_pressed;
  return event;
}

boolean getWiFiConfig(){
  /*
    Função que, usando um AP de configuração, envia uma página solicitando SSID
    e senha de uma rede WiFi. Ajusta as variáveis de SSID e senha da rede na
    qual o botão deve se conectar para enviar os pedidos. Essencialmente um pe-
    queno webServer, com apenas duas páginas.
    
    Entradas: nenhuma
    Saída:    true (retorna para avisar que recebeu informação de uma rede)
  */

  boolean gotWiFiInfo = false;
  
  digitalWrite(ledCFG, HIGH);
  
  while (gotWiFiInfo == false){
    
    WiFiClient client = server.available();
    
    if (!client) {
      continue;
    }
    
    String req = client.readStringUntil('\0');
    
    String s = "HTTP/1.1 200 OK\r\n";
    s += "Content-Type: text/html\r\n\r\n";
  
    if(req[0] == 'G'){          // GET - envia página de configuração
      
      s += "<!DOCTYPE HTML>\r\n<html>\r\n<form action=\"http://192.168.4.1\" method=\"post\">  <input name=\"nwID\" value=\"SSID\">  <input name=\"nwPW\" value=\"SENHA\">  <button>Enviar</button></form>";
      
    } else if (req[0] == 'P'){  //POST - Envia confirmação de configuração
    
      s += "<!DOCTYPE HTML>\r\n<html>\r\nObrigado!";
  
      nwID = (req.substring(req.indexOf("nwID=")+5, req.indexOf("&nwPW")));
      nwPW = (req.substring(req.indexOf("&nwPW=")+6));
      gotWiFiInfo = true;
    }
    
    s += "</html>\n";
  
    client.print(s);
    delay(1);
    Serial.println("Client disconnected");
  
  }
  
  digitalWrite(ledCFG, LOW);
  
  return true;
}


boolean connectToSSID(){
  /*
    Função que encerra o AP de setup e conecta com o SSID fornecido na etapa de
    configuração.
    
    Entradas: nenhuma
    Saída:    true (avisa que conectou ao SSID informado)
  */
  
  WiFi.mode(WIFI_STA);
  char ID[nwID.length()+1];
  nwID.toCharArray(ID, nwID.length()+1);
  char PW[nwPW.length()+1];
  nwPW.toCharArray(PW, nwPW.length()+1);
  Serial.println(ID);
  WiFi.begin(ID, PW);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    statusCFG = !statusCFG;
    //Serial.print(".");
    digitalWrite(D5, statusCFG);
  }
  Serial.println("WiFi connected");  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  digitalWrite(D5, LOW);
  return true;
}

void setupWiFi()
{
  /*
    Função que configura o AP de setup. Gera um nome "único" usando o endereço
    MAC da placa, e usa a senha padrão definida acima.
    
    Entradas: nenhuma
    Saída:    nenhuma
  */
  
  WiFi.mode(WIFI_AP);

  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  macID.toUpperCase();
  String AP_NameString = "EasyMed " + macID;

  char AP_NameChar[AP_NameString.length() + 1];
  AP_NameString.toCharArray(AP_NameChar, AP_NameString.length() + 1);
  WiFi.softAP(AP_NameChar, WiFiAPPSK);
}



void setup(){
  pinMode(D3, INPUT);
  //digitalWrite(D3, HIGH); // pull-up
  
  pinMode(ledCFG, OUTPUT);
  pinMode(ledCON, OUTPUT);
  pinMode(ledSND, OUTPUT);
  pinMode(ledAPP, OUTPUT);
  
  Serial.begin(9600);
  button_was_pressed = false;
  button_pressed_counter = 0;
  
  setupWiFi();
  server.begin();
  
  WiFiConnected = false;
  
  
}

void loop(){
  if (WiFiConnected == false){
    getWiFiConfig();
    WiFiConnected = connectToSSID();
  }else{
    if (lastState != currentState){
      switch (currentState){
        case SLEEP:
        lightsOff();
        Serial.println("Modo de espera.");
        break;
        case APP:
        Serial.println("Abrindo aplicativo...");
        //ledBlink(D6, 10);
        Serial.println("App aberto no celular!");
        break;
        case CONNECT:
        Serial.println("Conectando...");
        connectToServer();
        Serial.println("Conectado a rede!");
        break;
        case SEND:
        Serial.println("Enviando pedido...");
        sendRequest();
        Serial.println("Pedido enviado!");
        break;
        default:
        Serial.println("Erro de variável...");
        break;
      }
    }
    
    int lastPress = handle_button();

    lastState = currentState;
    currentState = nextState(currentState, lastPress);
    
    delay(DELAY);
  }
}

