#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>

#define SERV_PORT 45000



int main(int argc, char** argv){
    int socketfd;
    int connfd;
    struct sockaddr_in serwer_addres;

    socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&serwer_addres, sizeof(serwer_addres));
    serwer_addres.sin_family = AF_INET;
    serwer_addres.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &serwer_addres.sin_addr);

    StrKli(socketfd, serwer_addres);

    printf("\nKLIENT: Program zostal zakonczony\n");
    exit(0);
}

void StrKli(int socketfd, struct sockaddr_in serwer_addres)
{
    int n;
    char linia[101];
    char liniarcv[100];
    //HandShake
    char helloServer[2];
    int klientId = 0x00;
    helloServer[0] = klientId;
    helloServer[1] = 'x';
    sendto(socketfd,helloServer,10,0,&serwer_addres,sizeof(serwer_addres));
    //Zapisanie klienta do servera
    //
    n = recvfrom(socketfd, liniarcv, 100, 0, NULL, NULL);
    printf("\nOtrzymałeś ID: %d\n",liniarcv[0]);
    klientId = liniarcv[0];
    //printf("KlientID: %d",klientId);
    //
    linia[0] = klientId;
    while(fgets(&linia[1], 100, stdin) != NULL){
        n = strlen(linia);
        if( n <= 1 ) return;
        if ( sendto(socketfd,linia,n,0,&serwer_addres,sizeof(serwer_addres)) == -1){
            printf("Blad");
        }
	    //sendto(socketfd,linia,n,0,&serwer_addres,sizeof(serwer_addres));
	    do{
	        n = recvfrom(socketfd, liniarcv, 100, 0, NULL, NULL);
	        liniarcv[n] = '\0';
	        fputs(liniarcv, stdout);
	        //printf("\n ---- L[0] = %d , Len = %ld , lastPlaced = %d \n",liniarcv[0],strlen(liniarcv),liniarcv[strlen(liniarcv)-1]);
	    }while(liniarcv[strlen(liniarcv)-1] != 0x02);
    }
    
}
