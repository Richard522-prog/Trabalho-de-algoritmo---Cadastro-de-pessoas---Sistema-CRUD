#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>
#include <string.h>
#include <wctype.h>

#define FILE_NAME "pessoas.bin"

typedef struct {
    wchar_t nome[100];
    wchar_t cpf[20];
    int idade;
    wchar_t email[100];
} Pessoa;

void limparBuffer() {
    wchar_t c;
    while ((c = getwchar()) != L'\n' && c != WEOF);
}

void removerNovaLinha(wchar_t *str) {
    size_t len = wcslen(str);
    if (len > 0 && str[len - 1] == L'\n') {
        str[len - 1] = L'\0';
    }
}

int validarCPF(const wchar_t *cpf) {
    if (wcslen(cpf) != 11) return 0;

    for (int i = 0; i < 11; i++) {
        if (!iswdigit(cpf[i])) return 0;
    }
    return 1;
}

int validarEmail(const wchar_t *email) {
    size_t len = wcslen(email);

    if (len < 6) return 0;

    if (wcschr(email, L'@') == NULL) return 0;

    const wchar_t *final = &email[len - 4];
    if (wcscmp(final, L".com") != 0) return 0;

    return 1;
}

int cpfExiste(const wchar_t *cpf) {
    FILE *f = fopen(FILE_NAME, "rb");
    if (!f) return 0;

    Pessoa p;
    while (fread(&p, sizeof(Pessoa), 1, f)) {
        if (wcscmp(p.cpf, cpf) == 0) {
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

void cadastrarPessoa() {
    Pessoa p;
    wprintf(L"\n--- Novo Cadastro ---\n");

    while (1) {
        wprintf(L"Digite o CPF (11 numeros): ");
        fgetws(p.cpf, 20, stdin);
        removerNovaLinha(p.cpf);

        if (!validarCPF(p.cpf)) {
            wprintf(L"[ERRO] CPF deve ter exatamente 11 digitos numericos.\n");
            continue;
        }

        if (cpfExiste(p.cpf)) {
            wprintf(L"[ERRO] Este CPF ja esta cadastrado!\n");
            return;
        }

        break;
    }

    wprintf(L"Nome: ");
    fgetws(p.nome, 100, stdin);
    removerNovaLinha(p.nome);

    wprintf(L"Idade: ");
    while (wscanf(L"%d", &p.idade) != 1) {
        wprintf(L"[ERRO] Digite apenas numeros para a idade: ");
        limparBuffer();
    }
    limparBuffer();

    while (1) {
        wprintf(L"E-mail (deve ter @ e terminar com .com): ");
        fgetws(p.email, 100, stdin);
        removerNovaLinha(p.email);

        if (validarEmail(p.email)) {
            break;
        } else {
            wprintf(L"[ERRO] Email invalido! Verifique o @ e o .com\n");
        }
    }

    FILE *f = fopen(FILE_NAME, "ab");
    if (!f) {
        wprintf(L"Erro ao abrir arquivo.\n");
        return;
    }
    fwrite(&p, sizeof(Pessoa), 1, f);
    fclose(f);

    wprintf(L"\nSucesso! Pessoa cadastrada.\n");
}

void listarPessoas() {
    FILE *f = fopen(FILE_NAME, "rb");
    if (!f) {
        wprintf(L"\nNenhum cadastro encontrado.\n");
        return;
    }

    Pessoa p;
    wprintf(L"\n--- Lista de Pessoas ---\n");

    while (fread(&p, sizeof(Pessoa), 1, f)) {
        wprintf(L"\nCPF: %ls\nNome: %ls\nIdade: %d\nE-mail: %ls\n",
                p.cpf, p.nome, p.idade, p.email);
        wprintf(L"-------------------------\n");
    }
    fclose(f);
}

void buscarPorCPF() {
    wchar_t cpf[20];

    wprintf(L"Digite o CPF para busca: ");
    fgetws(cpf, 20, stdin);
    removerNovaLinha(cpf);

    FILE *f = fopen(FILE_NAME, "rb");
    if (!f) {
        wprintf(L"\nBase de dados vazia.\n");
        return;
    }

    Pessoa p;
    int achou = 0;
    while (fread(&p, sizeof(Pessoa), 1, f)) {
        if (wcscmp(p.cpf, cpf) == 0) {
            wprintf(L"\n--- Encontrado ---\n");
            wprintf(L"Nome: %ls\nCPF: %ls\nIdade: %d\nE-mail: %ls\n",
                    p.nome, p.cpf, p.idade, p.email);
            achou = 1;
            break;
        }
    }
    fclose(f);

    if (!achou) wprintf(L"\nCPF nao encontrado.\n");
}

void atualizarPessoa() {
    wchar_t cpf[20];
    wchar_t bufferNome[100];
    wchar_t bufferEmail[100];

    wprintf(L"Digite o CPF para atualizar: ");
    fgetws(cpf, 20, stdin);
    removerNovaLinha(cpf);

    FILE *f = fopen(FILE_NAME, "rb");
    FILE *temp = fopen("temp.bin", "wb");

    if (!f || !temp) {
        wprintf(L"\nErro de arquivo.\n");
        if(f) fclose(f); if(temp) fclose(temp);
        return;
    }

    int encontrado = 0;
    Pessoa p;

    while (fread(&p, sizeof(Pessoa), 1, f)) {
        if (wcscmp(p.cpf, cpf) == 0) {
            encontrado = 1;
            wprintf(L"\nAtualizando dados de: %ls\n", p.nome);

            wprintf(L"Novo nome: ");
            fgetws(bufferNome, 100, stdin);
            removerNovaLinha(bufferNome);
            wcscpy(p.nome, bufferNome);

            wprintf(L"Nova idade: ");
            while (wscanf(L"%d", &p.idade) != 1) {
                wprintf(L"Digite uma idade valida: ");
                limparBuffer();
            }
            limparBuffer();

            while (1) {
                wprintf(L"Novo e-mail: ");
                fgetws(bufferEmail, 100, stdin);
                removerNovaLinha(bufferEmail);

                if (validarEmail(bufferEmail)) {
                    wcscpy(p.email, bufferEmail);
                    break;
                } else {
                    wprintf(L"[ERRO] Email invalido! Use @ e .com\n");
                }
            }
        }
        fwrite(&p, sizeof(Pessoa), 1, temp);
    }

    fclose(f);
    fclose(temp);

    remove(FILE_NAME);
    rename("temp.bin", FILE_NAME);

    if (encontrado) wprintf(L"\nAtualizado com sucesso!\n");
    else wprintf(L"\nCPF nao encontrado.\n");
}

void removerPessoa() {
    wchar_t cpf[20];

    wprintf(L"Digite o CPF para remover: ");
    fgetws(cpf, 20, stdin);
    removerNovaLinha(cpf);

    FILE *f = fopen(FILE_NAME, "rb");
    FILE *temp = fopen("temp.bin", "wb");

    if (!f || !temp) {
        wprintf(L"\nErro de arquivo.\n");
        if(f) fclose(f); if(temp) fclose(temp);
        return;
    }

    Pessoa p;
    int encontrado = 0;

    while (fread(&p, sizeof(Pessoa), 1, f)) {
        if (wcscmp(p.cpf, cpf) == 0) {
            encontrado = 1;
            continue;
        }
        fwrite(&p, sizeof(Pessoa), 1, temp);
    }

    fclose(f);
    fclose(temp);

    remove(FILE_NAME);
    rename("temp.bin", FILE_NAME);

    if (encontrado) wprintf(L"\nRemovido com sucesso!\n");
    else wprintf(L"\nCPF nao encontrado.\n");
}

int main() {
    setlocale(LC_ALL, "Portuguese");

    int opcao;

    while (1) {
        wprintf(L"\n===== MENU =====\n");
        wprintf(L"1 - Cadastrar\n");
        wprintf(L"2 - Listar\n");
        wprintf(L"3 - Buscar\n");
        wprintf(L"4 - Atualizar\n");
        wprintf(L"5 - Remover\n");
        wprintf(L"6 - Sair\n");
        wprintf(L"Opcao: ");

        if (wscanf(L"%d", &opcao) != 1) {
            limparBuffer();
            continue;
        }
        limparBuffer();

        switch (opcao) {
            case 1: cadastrarPessoa(); break;
            case 2: listarPessoas(); break;
            case 3: buscarPorCPF(); break;
            case 4: atualizarPessoa(); break;
            case 5: removerPessoa(); break;
            case 6: exit(0);
            default: wprintf(L"Opcao invalida!\n");
        }
    }
    return 0;
}
