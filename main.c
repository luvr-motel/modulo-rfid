#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>

#define RST_PIN 9
#define SS_PIN 10

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
byte bloco = 4; // bloco onde vamos salvar dados (setor 1)

// =======================
// CONFIGURAÇÃO DO WIFI
// =======================
String WIFI_SSID = "varoto";     // Coloque aqui o nome do seu Wi-Fi
String WIFI_SENHA = "12345678";     // Coloque aqui a senha do Wi-Fi

// Conexão ESP-01
#define ESP_RX 2
#define ESP_TX 3
SoftwareSerial esp(ESP_RX, ESP_TX);

// =======================
// AUXILIARES
// =======================
void inicializarChavePadrao() {
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
}

void aguardarTag() {
  Serial.println("Aproxime uma TAG...");
  while (!rfid.PICC_IsNewCardPresent());
  while (!rfid.PICC_ReadCardSerial());
}

// Envia comando AT e imprime resposta
void sendAT(String cmd, int wait = 2000) {
  esp.println(cmd);
  delay(wait);
  while (esp.available()) {
    Serial.write(esp.read());
  }
}

// Conecta no Wi-Fi usando variáveis
void conectarWiFi() {
  Serial.println("Conectando ao WiFi...");
  sendAT("AT");
  sendAT("AT+CWMODE=1");
  sendAT("AT+CWJAP=\"" + WIFI_SSID + "\",\"" + WIFI_SENHA + "\"", 6000);
  Serial.println("WiFi conectado!");
}

// =======================
// LER TAG + retorno como String
// =======================
String lerProdutoID() {
  aguardarTag();

  byte buffer[18];
  byte tamanho = sizeof(buffer);

  if (rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, bloco, &key, &(rfid.uid)) != MFRC522::STATUS_OK) {
    Serial.println("Falha na autenticação!");
    return "";
  }

  if (rfid.MIFARE_Read(bloco, buffer, &tamanho) != MFRC522::STATUS_OK) {
    Serial.println("Falha na leitura!");
    return "";
  }

  String produto = "";
  for (int i = 0; i < 16; i++) {
    if (buffer[i] == 0 || buffer[i] == 0x20) break; // ignora espaços
    produto += (char)buffer[i];
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  return produto;
}

// =======================
// ENVIAR PARA API
// =======================
void enviarParaAPI(String produtoID) {
  if (produtoID == "") return;

  String json = "{\"locacao_id\":12,\"produto_id\":" + produtoID + ",\"qtde\":1}";

  Serial.println("Enviando para API:");
  Serial.println(json);

  sendAT("AT+CIPSTART=\"TCP\",\"api.luvr.com.br\",80", 4000);
  int len = json.length() + 100;
  sendAT("AT+CIPSEND=" + String(len), 2000);

  String req =
    "POST /itemlocacao HTTP/1.1\r\n"
    "Host: api.luvr.com.br\r\n"
    "Content-Type: application/json\r\n"
    "Content-Length: " + String(json.length()) + "\r\n\r\n" +
    json;

  esp.print(req);
  delay(2000);
  sendAT("AT+CIPCLOSE");
}

// =======================
// FUNÇÕES DE TAG (LER, GRAVAR, EXCLUIR)
// =======================
void lerTag() {
  String produtoID = lerProdutoID();
  if (produtoID != "") {
    Serial.print("Produto lido: ");
    Serial.println(produtoID);
    enviarParaAPI(produtoID);
  }
}

void gravarTag() {
  aguardarTag();
  Serial.println("Digite o texto para gravar (max 16 chars):");
  while (!Serial.available());
  String texto = Serial.readStringUntil('\n');
  texto.trim();
  if (texto.length() > 16) texto = texto.substring(0, 16);

  byte dados[16];
  for (int i = 0; i < 16; i++) {
    if (i < texto.length()) dados[i] = texto[i];
    else dados[i] = 0x20;
  }

  if (rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, bloco, &key, &(rfid.uid)) != MFRC522::STATUS_OK) {
    Serial.println("Falha na autenticação!");
    return;
  }

  if (rfid.MIFARE_Write(bloco, dados, 16) != MFRC522::STATUS_OK) {
    Serial.println("Erro ao escrever!");
  } else {
    Serial.println("Gravado com sucesso!");
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

void excluirTag() {
  aguardarTag();
  byte dados[16];
  for (int i = 0; i < 16; i++) dados[i] = 0x00;

  if (rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, bloco, &key, &(rfid.uid)) != MFRC522::STATUS_OK) {
    Serial.println("Falha na autenticação!");
    return;
  }

  if (rfid.MIFARE_Write(bloco, dados, 16) != MFRC522::STATUS_OK) {
    Serial.println("Erro ao apagar!");
  } else {
    Serial.println("Bloco apagado com sucesso!");
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

// =======================
// SETUP
// =======================
void setup() {
  Serial.begin(9600);
  esp.begin(115200);
  SPI.begin();
  rfid.PCD_Init();
  inicializarChavePadrao();

  conectarWiFi();

  Serial.println("Leitor RC522 pronto!");
  Serial.println("--------------------------------");
  Serial.println("1 - Ler tag");
  Serial.println("2 - Gravar tag");
  Serial.println("3 - Excluir tag");
  Serial.println("--------------------------------");
}

// =======================
// LOOP PRINCIPAL
// =======================
void loop() {
  if (!Serial.available()) return;

  char opcao = Serial.read();

  switch (opcao) {
    case '1':
      lerTag();
      break;

    case '2':
      gravarTag();
      break;

    case '3':
      excluirTag();
      break;

    default:
      Serial.println("Opção inválida!");
  }

  Serial.println("------------------------------------------\n");
  Serial.println("--- Olá, selecione a operação desejada.---\n");
  Serial.println("------------------------------------------\n");
  Serial.println("| 1 ) Cadastrar produto                  |\n");
  Serial.println("| 2 ) Excluir produto                    |\n");
  Serial.println("| 3 ) Ler produto                        |\n");
  Serial.println("------------------------------------------\n");
  Serial.println("Digite sua opção: ");
}




