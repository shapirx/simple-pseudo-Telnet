#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h> 
#include <sys/socket.h>
#include <fcntl.h>

#define SERV_PORT 45000
#define BUFOR 1000

// Klienci
int id = -1;

struct klient_inf{
    struct sockaddr_in klient_address;
    int klient_address_len;
    int pipeIn[2];
    int pipeOut[2];
};

struct klient_inf klienci[256];

//----------------------------------------------------------
int main(){
	int listenfd, connfd;
	socklen_t klient_len;
    struct sockaddr_in klient_addres, serwer_addres;
    char Addr[100];
    listenfd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&serwer_addres, sizeof(serwer_addres));
    serwer_addres.sin_family = AF_INET;
    serwer_addres.sin_addr.s_addr = htonl(INADDR_ANY);
   	serwer_addres.sin_port = htons(SERV_PORT);
   	struct timeval read_timeout;
    read_timeout.tv_sec = 0;
    read_timeout.tv_usec = 10;
    setsockopt(listenfd, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);
    bind(listenfd, &serwer_addres, sizeof(serwer_addres));
    inet_ntop(AF_INET, &serwer_addres.sin_addr, Addr, 100);
	for(;;) Echo(listenfd, klient_addres);
}

void Echo(int socketfd, struct sockaddr_in klient_addres){
    pid_t pid;
    int n, i;
    const int BUFSIZE = 4096;
    char bufor[100];
    char line[10000];
    char Addr[100];
    int len_klient_addres;
    for(;;)
    {
        n = recvfrom(socketfd, line, 100, 0, &klient_addres, &len_klient_addres);
        if(n <= 0){
            for(i=0; i < 255; i++){
                if(klienci[i].klient_address_len == 0)
                    continue;
                while((n = read(klienci[i].pipeOut[0], bufor, 100)) > 0){
                    sendto(socketfd, bufor, n, 0, &klienci[i].klient_address, klienci[i].klient_address_len);
                    //printf("Wyslano %d bajtow\n", n);
                    //printf("\n Bufor:[%d]\n %s",bufor[0],bufor);
                }
            }
        }else{
            line[n] = '\0';
            inet_ntop(AF_INET, &klient_addres.sin_addr, Addr, 100);
            if(n <= 1){
                printf("\nSERWER: POLACZENIE ZAKONCZONO");
                return;
            }
            /// Sprawdzanie Czy Klient jest podłączony i zarejestrowany
            if(line[0] == 0x00 && line[1] == 120){
                id++;
                printf("\nPoprawnie Dodano Klienta o adresie ID=[%s] Klient ID = [%d] \n",Addr,id);
                line[0] = id;
                sendto(socketfd, line, 1, 0, &klient_addres, sizeof(klient_addres)); 
                //Tworzenie uchwytów
                klienci[id].klient_address = klient_addres;
                klienci[id].klient_address_len = sizeof(klient_addres);
                
                pipe(klienci[id].pipeOut);
                pipe(klienci[id].pipeIn);
                fcntl(klienci[id].pipeOut[0], F_SETFL, fcntl(klienci[i].pipeOut[0], F_GETFL) | O_NONBLOCK);
                //fcntl(klienci[id].pipeIn[0], F_SETFL, fcntl(klienci[i].pipeIn[0], F_GETFL) | O_NONBLOCK);
                pid = fork();
                if (pid < 0)
                {
                    fprintf(stderr, "fork Failed" );
                    return 1;
                }else if(pid == 0){
                    //printf("\ntest\n");
                    char innerbuf[100];
                    int x = 0;
                    while((x = read(klienci[id].pipeIn[0], innerbuf, 100)) > 0){
                        innerbuf[x] = '\0';
                        FILE *hook = popen(innerbuf, "r");
                        char buffer[BUFSIZE];
                        buffer[0] = 0x01;
                		while(fgets(&buffer[1], BUFSIZE-1,hook)){
                            printf(buffer);
                            write(klienci[id].pipeOut[1], buffer, strlen(buffer));
                            //printf("Wpisanie %d bajow\n", strlen(buffer));
                		    //sendto(socketfd, buffer, strlen(buffer) + 1, 0, &klient_addres, sizeof(klient_addres));
                	    }
                	    buffer[0] = 0x02;
                		write(klienci[id].pipeOut[1], buffer, 1);
                	    pclose(hook);
                    }
            	    exit(0);
                }
            }else{
                printf("\nSERWER: Wyslany z komputera o adresie IP=%s KlientID = [%d] , zapytanie: %s\n", Addr,line[0],&line[1]);
                //printf("\n Jestem\n");
                write(klienci[line[0]].pipeIn[1], &line[1], strlen(&line[1]));
            }
        }
    }
   	 printf("\nSERWER: Obsluga Klienta o adresie IP=%s zostala zakonczona", Addr);
}
