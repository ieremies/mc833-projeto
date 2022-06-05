/*
** server.c -- a stream socket server demo
*/

#include "../front/server.c"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define PORT "4950" // the port users will be connecting to

#define MAXBUFLEN 100

int SOCKFD; // global to be closed on exit
int START_TIME;

/**
 * @brief Função que espera por uma conexão.
 * @details Em um laço infinito, esperamos uma nova conexão chegar.
 * @param[in] socket Socket a escutar.
 */
void handle_client(int socket) {
    Payload payload;
    while (1) {
        if (recv(socket, &payload, sizeof(Payload), 0) == -1)
            perror("recv");

        if (payload.op == EXIT)
            break;
        if (payload.op < 0 || payload.op > EXIT) {
            printf("\nInvalid operation\n");
            continue;
        }

        printf("[%d] : Payload received.\n", (int)time(NULL) - START_TIME);

        handlers[payload.op](&payload.movie, socket); // execute the action
    }
}

/**
 * @brief Função para finalizar o servidor
 * @details Fechamos o socket, fazemos o backup do catálogo e terminamos o
 * programa.
 */
void sigint_handler(int sig_num) {
    close(SOCKFD);
    system("clear");
    printf("server: exiting...\n");
    backup(&CATALOG);
    sleep(1);
    exit(0);
}

int main(int argc, char *argv[]) {
    system("clear");
    CATALOG.size = 0;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    ssize_t numbytes;
    struct sockaddr_storage their_addr; // connector's address information
    char buf[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];

    START_TIME = time(NULL);
    printf("[%d] : Server started.\n", (int)time(NULL) - START_TIME);

    if (argc == 2 && strcmp(argv[1], "load") == 0)
        load_backup(&CATALOG);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    // Loop through all the results and bind to the first we can:
    for (p = servinfo; p != NULL; p = p->ai_next) {
        SOCKFD = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (SOCKFD == -1) {
            perror("server: socket");
            continue;
        }

        if (bind(SOCKFD, p->ai_addr, p->ai_addrlen) == -1) {
            close(SOCKFD);
            perror("server: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "server: failed to bind socket\n");
        exit(2);
    }

    freeaddrinfo(servinfo); // all done with this structure

    printf("server: waiting for connections...\n");

    // Make sure the socket will be cleaned:
    signal(SIGINT, sigint_handler);

    addr_len = sizeof(their_addr);
    const char *inet_address;
    while (1) {
        numbytes = recvfrom(SOCKFD, buf, MAXBUFLEN - 1, 0,
                            (struct sockaddr *)&their_addr, &addr_len);
        if (numbytes == -1) {
            perror("recvfrom");
            exit(1);
        }

        inet_address = inet_ntop(their_addr.ss_family,
                                 get_in_addr((struct sockaddr *)&their_addr), s,
                                 sizeof(s));
        printf("server: got packet from %s\n", inet_address);
        printf("server: packet is %zd bytes long\n", numbytes);
        buf[numbytes] = '\0';
        printf("server: packet contains \"%s\"\n", buf);
        backup(&CATALOG);
    }
}
