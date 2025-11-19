# Projeto Leitor RFID com ESP-01 e Arduino

## Descrição
Este projeto permite ler, gravar e excluir informações em tags RFID usando um **leitor RC522**, conectado a um **Arduino**. Os dados lidos podem ser enviados para uma API via **ESP-01**, que se conecta à rede Wi-Fi. O sistema valida tanto o leitor quanto a conexão Wi-Fi antes de permitir operações.

---

## Componentes usados
- **Arduino Uno / Nano / Mega**  
- **Leitor RFID RC522**  
- **Módulo Wi-Fi ESP-01**  
- **Tags / Cartões RFID (MIFARE 1K)**  
- **Jumpers e fonte 5V**  

---

## Bibliotecas utilizadas
- `MFRC522.h` → para comunicação com o leitor RC522 via SPI.  
- `SPI.h` → comunicação SPI com o RC522.  
- `SoftwareSerial.h` → comunicação serial com o ESP-01.

---

## Funcionamento
1. Ao ligar, o Arduino inicializa o leitor RC522 e verifica se ele está conectado.  
2. Em seguida, tenta conectar o ESP-01 à rede Wi-Fi usando credenciais definidas no código.  
3. Se o leitor ou a conexão falharem, o sistema avisa no Serial e não prossegue.  
4. No loop principal, o usuário pode:  
   - Ler uma tag (`opção 1`)  
   - Gravar dados em uma tag (`opção 2`)  
   - Apagar dados de uma tag (`opção 3`)  
5. Ao ler uma tag, os dados podem ser enviados para uma API REST via HTTP POST.  

---

## Recursos implementados
- Reconexão automática do Wi-Fi em caso de falha.  
- Validação do leitor RC522 antes do uso.  
- Funções completas para leitura, gravação e exclusão de tags.  
- Mensagens claras via Serial para orientar o usuário.

---

## Como usar
1. Configure o Wi-Fi no código:  
```cpp
String WIFI_SSID = "SEU_SSID";
String WIFI_SENHA = "SUA_SENHA";
