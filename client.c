/*
TODO:
    finish service() (cmd interface that helps you to "speak" with the server
    implement basic commands to move between directories
    and more...
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <sys/stat.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#define FTP_PORT 21
#define BUFSIZE 1024

int socket_create(char* ip);
int login(int sockfd, char* ip);
void reply_handle(int code);
int service(int sockfd, char* ip);

/* function reply_handle
    code - server reply code
    prints a description for a reply code
    TODO:
        remove unneeded codes
*/
void reply_handle(int code) {
    switch(code) {
        case 200:
            printf("Command okay\n");
            break;
        case 202:
            printf("Command not implemented, superfluous at this site.\n");
            break;
        case 500:
            printf("Syntax error, command not recognized.\n");
            break;
        case 501:
            printf("Syntax error in parameters or arguments.\n");
            break;
        case 502:
            printf("Command not implemented.\n");
            break;
        case 503:
            printf("Bad sequence of commands.\n");
            break;
        case 504:
            printf("Command not implemented for that parameter.\n");
            break;
        case 530:
            printf("Not logged in.\n");
            break;
        case 550:
            printf("Requested action not taken (e.g., file not found, no access).\n");
            break;
        case 551:
            printf("Requested action aborted: page type unknown.\n");
            break;
        case 552:
            printf("Requested file action aborted.\n");
            printf("Exceeded storage allocation (for current directory or dataset).");
            break;
        case 553:
            printf("Requested action not taken.\n");
            printf("File name not allowed\n");
            break;
        default: 
            printf("No description for code %d", code);
    }
}

/* function socket_create
    ip - server ip address (argv[1])
    creates socket and connects to server
    returns socket fd, else -1
*/
int socket_create(char* ip) {
    int sockfd = 0;
    struct sockaddr_in* srv_addr = malloc(sizeof(struct sockaddr_in));

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket() error");
        return -1;
    }

    srv_addr->sin_family = AF_INET;

    int res;
    if ((res = inet_aton(ip, &srv_addr->sin_addr)) == 0) {
        perror("Invalid IP address!");
        return -1;
    }

    srv_addr->sin_port = htons(FTP_PORT);

    if((res = connect(sockfd, (struct sockaddr*) srv_addr, sizeof(struct sockaddr))) == -1) {
        perror("Connect() error");
        return -1;
    }

    free(srv_addr);
    return sockfd;
}

/* function login
    sockfd - socket descriptor
    ip - server ip address(argv[1])
    sends USER and PASS commands to server
    returns 0 on successful completion, else -1 
*/
int login(int sockfd, char* ip) {
    int res, ftp_code;
    char buffer[BUFSIZE];
    char* str;

    printf("Connection established, waiting for response\n");
     
    while(res = recv(sockfd, buffer, BUFSIZE, 0) >  0) {
        sscanf(buffer, "%d", &ftp_code);
        printf("%s", buffer);

        if(ftp_code != 220) {
            reply_handle(ftp_code);
            return -1;
        }

        str = strstr(buffer, "220");
        if(str != NULL) {
            break;
        }
        memset(buffer, 0, res); 
    }
    
    // Send username
    char inpt_buff[50];
    printf("Name for %s\n", ip);
    memset(buffer, 0, sizeof(buffer));
    scanf("%s", inpt_buff);

    sprintf(buffer, "USER %s\r\n", inpt_buff);
    res = send(sockfd, buffer, BUFSIZE, 0);

    memset(buffer, 0, sizeof(buffer));
    res = recv(sockfd, buffer, BUFSIZE, 0);

    sscanf(buffer, "%d", &ftp_code);
    if(ftp_code != 331) {
        reply_handle(ftp_code);
        return -1;
    }
    printf("%s\n", buffer);

    // Send password
    memset(inpt_buff, 0, sizeof(inpt_buff));
    printf("Password for %s\n", ip);
    scanf("%s", inpt_buff);

    sprintf(buffer, "PASS %s\r\n", inpt_buff);
    res = send(sockfd, buffer, BUFSIZE, 0);

    memset(buffer, 0, sizeof(buffer));
    res = recv(sockfd, buffer, BUFSIZE, 0);

    sscanf(buffer, "%d", &ftp_code);
    if(ftp_code != 230) {
        reply_handle(ftp_code);
        return -1; 
    }
    printf("%s\n", buffer);

    return 0;
}

int main(int argc, char* argv[]) {
    int sockfd = socket_create(argv[1]);
 
    if(sockfd == -1) {
        perror("Error creating a socket");
        return EXIT_FAILURE;
    } 

    if((login(sockfd, argv[1])) < 0) {
        perror("Error while logining in");
        return EXIT_FAILURE;
    }

    //service(sockfd, argv[1]);

    close(sockfd);
    return EXIT_SUCCESS;
}
