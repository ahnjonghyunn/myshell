#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LINE 1024  // 명령어 최대 길이
#define MAX_ARGS 64    // 명령어 인수 최대 개수
#define PATH_FILE ".myshell"

// 명령어 파싱 함수
void parse_command(char *line, char **args) {
    int i = 0;
    args[i] = strtok(line, " \t\n");
    while (args[i] != NULL && i < MAX_ARGS - 1) {
        i++;
        args[i] = strtok(NULL, " \t\n");
    }
    args[i] = NULL;
}

// PATH 설정 함수
void set_path() {
    int fd = open(PATH_FILE, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open PATH file");
        exit(1);
    }

    char path[MAX_LINE];
    ssize_t bytesRead = read(fd, path, sizeof(path) - 1);
    if (bytesRead == -1) {
        perror("Failed to read PATH file");
        close(fd);
        exit(1);
    }
    path[bytesRead] = '\0';  // 널 종단 문자 추가

    strtok(path, "\n");  // 줄바꿈 문자 제거
    if (strncmp(path, "PATH=", 5) == 0) {
        setenv("PATH", path + 5, 1);  // "PATH=" 부분을 건너뛰고 설정
    } else {
        fprintf(stderr, "Invalid PATH format in %s\n", PATH_FILE);
        close(fd);
        exit(1);
    }

    close(fd);
}

// 파일 존재 여부 확인 함수
void check_file_existence(char *filename) {
    if (access(filename, F_OK) != 0) {
        printf("파일이 존재하지 않음\n");
    } else {
        printf("파일 존재함\n");
    }
}

// 명령어 실행 함수
void execute_command(char **args) {
    pid_t pid = fork(); // 자식 프로세스 생성
    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    } else if (pid == 0) {
        // 자식 프로세스: 명령어 실행
        if (execvp(args[0], args) == -1) {
            perror("Command execution failed");
        }
        exit(1);
    } else {
        // 부모 프로세스: 자식 프로세스가 끝날 때까지 대기
        wait(NULL);
    }
}

int main(int argc, char *argv[]) {
    char line[MAX_LINE];  // 명령어 입력 버퍼
    char *args[MAX_ARGS]; // 명령어 인수 배열

    // PATH 설정
    set_path();
	
	// 파일 존재 여부 확인
    if (access(argv[1], F_OK) != 0) {
        printf("파일이 존재하지 않음\n");
    } else {
        printf("파일 존재함\n");
    }

    while (1) {
        // 프롬프트 출력
        printf("myshell> ");
        if (fgets(line, sizeof(line), stdin) == NULL) {
            break; // EOF 입력 시 루프 종료
        }

        // 명령어 파싱
        parse_command(line, args);

        // 빈 명령어 처리
        if (args[0] == NULL) {
            continue;
        }

        // "exit" 명령어 처리
        if (strcmp(args[0], "exit") == 0) {
            break;
        }

      

        // 일반 명령어 실행
        execute_command(args);
    }

    return 0;
}
