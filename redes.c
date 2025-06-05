#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

#define POLYNOMIAL 0x5B  // x^6 + x^4 + x^3 + x + 1

// Gera mensagem aleatória de 32 bits
int generate_random_message() {
    return (rand() << 16) | rand();
}

// Imprime binário em buffer
void append_binary(char *buffer, uint64_t number, int bits) {
    for (int i = bits - 1; i >= 0; i--) {
        sprintf(buffer + strlen(buffer), "%ld", (number >> i) & 1);
    }
}

// Cálculo CRC por divisão módulo 2
uint8_t calculate_crc_modulo(int message, char *log) {
    uint64_t data = (uint64_t)((uint32_t)message) << 6;
    const uint64_t poly = POLYNOMIAL;

    char temp[128];
    sprintf(log + strlen(log), "=== Cálculo do CRC por divisão módulo 2 ===\n");

    sprintf(log + strlen(log), "Mensagem original: ");
    append_binary(log, (uint32_t)message, 32);
    sprintf(log + strlen(log), "\nMensagem com 6 zeros anexados: ");
    append_binary(log, data, 38);
    sprintf(log + strlen(log), "\nPolinômio: ");
    append_binary(log, poly, 7);
    sprintf(log + strlen(log), "\n\n");

    int step = 1;
    for (int i = 37; i >= 6; i--) {
        if ((data >> i) & 1) {
            data ^= (poly << (i - 6));
            sprintf(temp, "Passo %2d: ", step++);
            strcat(log, temp);
            uint64_t part = data >> (i - 6);
            append_binary(log, part, 7);
            sprintf(temp, " (XOR aplicado na posição %d)\n", i);
            strcat(log, temp);
        }
    }

    uint8_t crc = data & 0x3F;
    sprintf(log + strlen(log), "\nCRC resultante: ");
    append_binary(log, crc, 6);
    sprintf(log + strlen(log), "\n");

    return crc;
}

// Cálculo CRC via LFSR
uint8_t calculate_crc_lfsr(int message, char *log) {
    uint8_t lfsr = 0;
    char temp[128];

    sprintf(log + strlen(log), "=== Cálculo do CRC via LFSR ===\n");
    sprintf(log + strlen(log), "Estado inicial do LFSR: ");
    append_binary(log, lfsr, 6);
    sprintf(log + strlen(log), "\n");

    for (int i = 31; i >= 0; i--) {
        uint8_t bit = ((uint32_t)message >> i) & 1;
        uint8_t feedback = (lfsr >> 5) ^ bit;

        lfsr = (lfsr << 1) & 0x3F;
        if (feedback) {
            lfsr ^= 0x0B; // x³, x¹, x⁰
            lfsr ^= 0x10; // x⁴
        }

        sprintf(temp, "Bit %2d: %d | Estado do LFSR: ", 32 - i, bit);
        strcat(log, temp);
        append_binary(log, lfsr, 6);
        sprintf(temp, " (feedback: %d)\n", feedback);
        strcat(log, temp);
    }

    sprintf(log + strlen(log), "\nCRC resultante: ");
    append_binary(log, lfsr, 6);
    sprintf(log + strlen(log), "\n");

    return lfsr;
}

// Relatório HTML
void generate_report(int message, uint8_t crc_modulo, uint8_t crc_lfsr, const char *log_modulo, const char *log_lfsr) {
    FILE *f = fopen("report.html", "w");
    if (!f) {
        perror("Erro ao criar o arquivo HTML");
        return;
    }

    fprintf(f,
        "<!DOCTYPE html><html lang='pt-BR'><head><meta charset='UTF-8'>"
        "<title>Relatório CRC</title>"
        "<style>"
        "body { font-family: 'Segoe UI', sans-serif; background-color: #f0f2f5; padding: 40px; }"
        ".header { background-color: #2c3453; color: white; text-align: center; padding: 30px; border-radius: 6px; margin-bottom: 30px; }"
        "h1{ color:rgb(255, 255, 255); }"
        "code, pre { font-family: 'Courier New', monospace; border-radius: 6px; }"
        ".section { background: white; padding: 25px; margin: 30px 0; border-radius: 10px; box-shadow: 0 4px 10px rgba(0,0,0,0.1); }"
        ".crc { font-weight: bold; color: #006600; }"
        ".error { color: #c0392b; font-weight: bold; }"
        ".success { color: #27ae60; font-weight: bold; }"
        "a { color: #1a0dab; text-decoration: none; } a:hover { text-decoration: underline; }"
        "</style></head><body>"
    );

    // Cabeçalho
    fprintf(f,
        "<div class='header'>"
        "<h1>Relatório de Cálculo CRC</h1>"
        "<p><strong>Curso:</strong> INF1640/INF1643 (Redes)</p>"
        "<p><strong>Professor:</strong> Sérgio Colcher</p>"
        "<p><strong>Aluno:</strong> Thiago Pereira Camerato</p>"
        "<p><strong>matrícula:</strong> 2212580</p>"
        "<p><strong>Data:</strong> Jun 5 2025</p>"
        "</div>"
    );

    // Mensagem
    fprintf(f, "<div class='section'><h2>Mensagem Aleatória</h2>");
    fprintf(f, "<p><strong>Mensagem (32 bits):</strong> <code>");
    for (int i = 31; i >= 0; i--) fprintf(f, "%d", ((uint32_t)message >> i) & 1);
    fprintf(f, "</code> (0x%08X)</p>", (uint32_t)message);
    fprintf(f, "<p>Trecho em código:</p><pre style='background:#f4f4f4; color:#000; padding:12px;'>"
           "int generate_random_message() {\n"
           "    return (rand() << 16) | rand();\n"
           "}"
           "</pre></div>");

    // Divisão módulo 2
    fprintf(f, "<div class='section'><h2>1) Cálculo do CRC por Divisão Módulo 2</h2>");
    fprintf(f, "<p>Trecho de código:</p><pre style='background:#f4f4f4; color:#000; padding:12px;'>"
               "uint64_t data = (uint64_t)message << 6;\n"
               "for (int i = 37; i >= 6; i--) {\n"
               "    if ((data >> i) & 1) {\n"
               "        data ^= (poly << (i - 6));\n"
               "    }\n"
               "}\n"
               "uint8_t crc = data & 0x3F;"
               "</pre>");
    fprintf(f, "<p><strong>CRC final:</strong> <span class='crc'><code>");
    for (int i = 5; i >= 0; i--) fprintf(f, "%d", (crc_modulo >> i) & 1);
    fprintf(f, "</code></span></p>");
    fprintf(f, "<p><strong>Saída detalhada:</strong></p><pre style='background:#f4f4f4; color:#000; padding:12px;'>%s</pre></div>", log_modulo);

    // LFSR
    fprintf(f, "<div class='section'><h2>2) Cálculo do CRC via LFSR</h2>");
    fprintf(f, "<p>Trecho de código:</p><pre style='background:#f4f4f4; color:#000; padding:12px;'>"
               "uint8_t feedback = (lfsr >> 5) ^ bit;\n"
               "lfsr = (lfsr << 1) & 0x3F;\n"
               "if (feedback) {\n"
               "    lfsr ^= 0x0B; // taps: x³, x¹, x⁰\n"
               "    lfsr ^= 0x10; // x⁴\n"
               "}"
               "</pre>");
    fprintf(f, "<p><strong>CRC final via LFSR:</strong> <span class='crc'><code>");
    for (int i = 5; i >= 0; i--) fprintf(f, "%d", (crc_lfsr >> i) & 1);
    fprintf(f, "</code></span></p>");
    fprintf(f, "<p><strong>Evolução completa:</strong></p><pre style='background:#f4f4f4; color:#000; padding:12px;'>%s</pre></div>", log_lfsr);

    // Comparação
    fprintf(f, "<div class='section'><h2>3) Comparação dos Resultados</h2>");
    fprintf(f, "<p><strong>Divisão módulo 2:</strong> <code>");
    for (int i = 5; i >= 0; i--) fprintf(f, "%d", (crc_modulo >> i) & 1);
    fprintf(f, "</code><br><strong>LFSR:</strong> <code>");
    for (int i = 5; i >= 0; i--) fprintf(f, "%d", (crc_lfsr >> i) & 1);
    fprintf(f, "</code></p>");
    fprintf(f, crc_modulo == crc_lfsr
        ? "<p class='success'>✅ Resultados IDÊNTICOS - Validação bem-sucedida!</p>"
        : "<p class='error'>❌ ERRO: Resultados divergentes!</p>");
    fprintf(f, "</div>");

    // Link do GitHub
    fprintf(f,
        "<div class='section'>"
        "<h2>4) Código-Fonte</h2>"
        "<p>O código-fonte completo deste relatório está disponível no GitHub:</p>"
        "<p><a href='https://github.com/thiagocamerato757/INF1640.git' target='_blank'>https://github.com/thiagocamerato757/INF1640.git</a></p>"
        "</div>"
    );

    fprintf(f, "</body></html>");
    fclose(f);

    printf("\nRelatório HTML gerado com sucesso: report.html\n");
}

// Main
int main() {
    srand(time(NULL));
    int message = generate_random_message();

    char log_modulo[8192] = "";
    char log_lfsr[8192] = "";

    uint8_t crc_modulo = calculate_crc_modulo(message, log_modulo);
    uint8_t crc_lfsr = calculate_crc_lfsr(message, log_lfsr);

    generate_report(message, crc_modulo, crc_lfsr, log_modulo, log_lfsr);
    return 0;
}
