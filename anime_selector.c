#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <locale.h>
#include <stdarg.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define MAX_ANIME 500
#define MAX_NAME_LENGTH 100

void uprintf(const char *fmt, ...) {
    char buf[4096];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

#ifdef _WIN32
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    if (h != INVALID_HANDLE_VALUE) {
        int wlen = MultiByteToWideChar(CP_UTF8, 0, buf, -1, NULL, 0);
        if (wlen > 0) {
            wchar_t *w = (wchar_t *)malloc(wlen * sizeof(wchar_t));
            MultiByteToWideChar(CP_UTF8, 0, buf, -1, w, wlen);
            DWORD written;
            WriteConsoleW(h, w, wlen - 1, &written, NULL);
            free(w);
            return;
        }
        wlen = MultiByteToWideChar(CP_ACP, 0, buf, -1, NULL, 0);
        if (wlen > 0) {
            wchar_t *w = (wchar_t *)malloc(wlen * sizeof(wchar_t));
            MultiByteToWideChar(CP_ACP, 0, buf, -1, w, wlen);
            DWORD written;
            WriteConsoleW(h, w, wlen - 1, &written, NULL);
            free(w);
            return;
        }
    }
#endif

    fputs(buf, stdout);
}

#define printf(...) uprintf(__VA_ARGS__)

typedef struct {
    char name[MAX_NAME_LENGTH];
} Anime;

Anime animes[MAX_ANIME];
int anime_count = 0;

int load_json(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        printf("错误：无法打开文件 %s\n", filename);
        return -1;
    }
    
    char line[256];
    int in_array = 0;
    
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "\"anime\"")) {
            in_array = 1;
            continue;
        }
        
        if (in_array && strstr(line, "\"")) {
            char* start = strchr(line, '"');
            if (start) {
                start++;
                char* end = strchr(start, '"');
                if (end) {
                    *end = '\0';
                    if (anime_count < MAX_ANIME) {
                        strncpy(animes[anime_count].name, start, MAX_NAME_LENGTH - 1);
                        animes[anime_count].name[MAX_NAME_LENGTH - 1] = '\0';
                        anime_count++;
                    }
                }
            }
        }
        
        if (strchr(line, ']')) {
            break;
        }
    }
    
    fclose(fp);
    return anime_count;
}

void print_banner() {
    printf("\n");
    printf("========================================\n");
    printf("                                        \n");
    printf("        今天看什么番？                  \n");
    printf("                                        \n");
    printf("        Anime Random Selector          \n");
    printf("                                        \n");
    printf("========================================\n");
    printf("\n");
}

void print_recommendation(int index) {
    printf("\n");
    printf("----------------------------------------\n");
    printf("                                        \n");
    printf("           今日推荐                     \n");
    printf("                                        \n");
    printf("      [%s]   \n", animes[index].name);
    printf("                                        \n");
    printf("        看看看!!!!                 \n");
    printf("                                        \n");
    printf("----------------------------------------\n");
    printf("\n");
}

void print_menu() {
    printf("[1] 换个推荐\n");
    printf("[2] 查看番库列表\n");
    printf("[3] 搜索番剧\n");
    printf("[4] 番库统计\n");
    printf("[0] 退出\n");
    printf("\n");
    printf("请选择: ");
}

void print_all_animes() {
    printf("\n");
    printf("========================================\n");
    printf("           番库列表 (%d部)             \n", anime_count);
    printf("========================================\n");
    printf("\n");
    
    for (int i = 0; i < anime_count; i++) {
        printf("%3d. %s\n", i + 1, animes[i].name);
        if ((i + 1) % 20 == 0) {
            printf("\n");
            printf("按回车继续...");
            getchar();
        }
    }
    
    printf("\n");
    printf("========================================\n");
}

void search_anime() {
    char keyword[100];
    printf("\n");
    printf("请输入搜索关键词: ");
    fgets(keyword, sizeof(keyword), stdin);
    keyword[strcspn(keyword, "\n")] = '\0';
    
    printf("\n");
    printf("========================================\n");
    printf("           搜索结果                     \n");
    printf("========================================\n");
    printf("\n");
    
    int found = 0;
    for (int i = 0; i < anime_count; i++) {
        if (strstr(animes[i].name, keyword)) {
            printf("[%s]\n", animes[i].name);
            found++;
        }
    }
    
    printf("\n");
    if (found == 0) {
        printf("未找到包含[%s]的番剧\n", keyword);
    } else {
        printf("共找到 %d 部番剧\n", found);
    }
    printf("\n");
    printf("========================================\n");
}

void print_statistics() {
    printf("\n");
    printf("========================================\n");
    printf("           番库统计                     \n");
    printf("========================================\n");
    printf("                                        \n");
    printf("      总计: %d 部动漫                  \n", anime_count);
    printf("                                        \n");
    printf("========================================\n");
}

int main() {
    setlocale(LC_ALL, "");
    
    #ifdef _WIN32
        SetConsoleOutputCP(65001);
        SetConsoleCP(65001);
    #endif
    
    print_banner();
    
    if (load_json("anime_database.json") < 0) {
        printf("\n  请确保 anime_database.json 文件在当前目录下\n");
        return 1;
    }
    
    printf("成功加载 %d 部动漫\n", anime_count);
    
    srand((unsigned int)time(NULL));
    int current_index = rand() % anime_count;
    
    print_recommendation(current_index);
    
    int choice;
    while (1) {
        print_menu();
        
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            printf("\n无效选择，请重新输入\n\n");
            continue;
        }
        getchar();
        
        switch (choice) {
            case 1:
                current_index = rand() % anime_count;
                print_recommendation(current_index);
                break;
            case 2:
                print_all_animes();
                break;
            case 3:
                search_anime();
                break;
            case 4:
                print_statistics();
                break;
            case 0:
                printf("\n");
                printf("感谢使用！下次再见！\n");
                printf("\n");
                return 0;
            default:
                printf("\n无效选择，请重新输入\n\n");
        }
    }
    
    return 0;
}
