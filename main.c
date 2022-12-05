#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#define CO2_SIZE 1024
#define CO2_TOK_BUFSIZE 64
#define CO2_TOK_DELIM " \t\r\n\a"


void co2_loop(void);
char *co2_read_line(void);
char **co2_split_line(char *line);

//functions:
int co2_cd(char **args);
int co2_help(char **args);
int co2_exit(char **args);
char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};
int (*builtin_func[]) (char **) = {
  &co2_cd,
  &co2_help,
  &co2_exit
};
int co2_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

int main()
{
    co2_loop();


    return 0;
}


void co2_loop(void){
    char *line;
    char **args;
    int status;
    do {
        printf("> ");
        line = co2_read_line();
        args = co2_split_line(line);
        status = co2_execute(args);

        free(line);
        free(args);
    }while(status);

}

char *co2_read_line(void){
    int bufsize = CO2_SIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char)* bufsize);
    int c;
    if(!buffer){
        fprintf(stderr, "co2: error in allocation\n");
        exit(EXIT_FAILURE);
    }
    while(1){
        c = getchar();
        if(c==EOF || c=='\n'){
            buffer[position] = '\0';
            return buffer;
        }else{
            buffer[position] = c;
        }
        position++;

        if(position >= bufsize){
            bufsize+=CO2_SIZE;
            buffer = realloc(buffer, bufsize);
            if(!buffer){
                fprintf(stderr, "co2: error in allocation\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}


char **co2_split_line(char *line){
    int bufsize = CO2_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize*sizeof(char*));
    char *token;
    if(!tokens){
        fprintf(stderr, "co2: error in allocation\n");
        exit(EXIT_FAILURE);
    }
    token = strtok(line, CO2_TOK_DELIM);
    while(token!=NULL){
        tokens[position] = token;
        position++;
        if(position >= bufsize){
            bufsize+=CO2_TOK_BUFSIZE;
            tokens = realoc(tokens, bufsize * sizeof(char*) );
            if(!tokens){
                fprintf(stderr, "co2: error in allocation\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, CO2_TOK_DELIM);
    }
    tokens[position]=NULL;
    return tokens;
}

int co2_launch(char **args){
    pid_t pid, wpid;
    int status;
    pid = fork();
    if(pid == 0){
        if(execvp(args[0], args) == -1){
            perror("co2");
        }
        exit(EXIT_FAILURE);
    }else if(pid<0){
        perror("co2");
    }else{
        do{
            wpid = waitpid(pid, &status, WUNTRACED);
        }while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}


int co2_cd(char **args){
    if(args[1] == NULL){
        fprintf(stderr, "co2: error, no argument in \"cd\"\n");
    }else{
        if(chdir(args[1])!= 0){
            perror("co2");
        }
    }
    return 1;
}

int co2_help(char **args){
    int i;
    printf("Abdou Lahboub CO2\n");
    printf("type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");
    for (i = 0; i < co2_num_builtins(); i++) {
        printf("  %s\n", builtin_str[i]);
    }
    printf("Use the main command for information on other programs.\n");
    return 1;
}

int co2_exit(char **args){
    return 0;
}


int co2_execute(char **args){
    int i;
    if(args[0]== NULL){
        return 1;
    }
    for(i=0;i<co2_num_builtins();i++){
        if(strcmp(args[0], builtin_str[i])==0){
            return (*builtin_func[i])(args);
        }
    }
    return co2_launch(args);
}





