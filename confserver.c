/*--------------------------------------------------------------------*/
/* conference server */

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <errno.h>

#include <stdlib.h>

#include <unistd.h>

extern char * recvtext(int sd);
extern int sendtext(int sd, char *msg);

extern int startserver();
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
int fd_isset(int fd, fd_set *fsp) {
	return FD_ISSET(fd, fsp);
}
/* main routine */
int main(int argc, char *argv[]) {
	int servsock; /* server socket descriptor */

	fd_set livesdset; /* set of live client sockets */
	int livesdmax; /* largest live client socket descriptor */

	/* check usage */
	if (argc != 1) {
		fprintf(stderr, "usage : %s\n", argv[0]);
		exit(1);
	}

	/* get ready to receive requests */
	servsock = startserver();

	if (servsock == -1) {
		perror("Error on starting server: ");
		exit(1);
	}

	/*
	 FILL HERE:
	 init the set of live clients
	 */

	FD_ZERO(&livesdset);
 	FD_SET(servsock, &livesdset);

	livesdmax = servsock;
	fd_set rst;


	/* receive requests and process them */
	while (1) {
		int frsock; /* loop variable */
		int mflag = 0 ;

		FD_ZERO(&rst);
		rst = livesdset;
		FD_SET(servsock, &rst);
		//rst = livesdset;


		 if (select(livesdmax + 1, &rst, NULL, NULL, NULL) < 0) {
			perror("Error: select error-s");
			exit(1);
		}

		/* look for messages from live clients */
		for (frsock = 3; frsock <= livesdmax; frsock++) {
			/* skip the listen socket */
			/* this case is covered separately */
			if (frsock == servsock)
				continue;

			if ( FD_ISSET(frsock, &rst)!=0 ) {
				char * clienthost; /* host name of the client */
				ushort clientport; /* port number of the client */


				struct hostent *chost;
				struct sockaddr_in client;
				socklen_t cLen = sizeof(client);
				getpeername(frsock, (struct sockaddr *)&client , &cLen);
				clienthost = gethostbyaddr(&(client.sin_addr.s_addr), sizeof(client.sin_addr.s_addr), AF_INET)->h_name;
				clientport = ntohs(client.sin_port);
				char* msg;


				/* read the message */
				msg = recvtext(frsock);
				if (!msg) {
					/* disconnect from client */
					printf("admin: disconnect from '%s(%hu)'\n", clienthost,
							clientport);

				FD_CLR(frsock, &rst);
				FD_CLR(frsock, &livesdset);
				mflag = 1;
				if(frsock == livesdmax){
					int i;
					int temp=0;
					for(i=servsock; i<livesdmax; i++){
						if(FD_ISSET(i,&livesdset) && i>temp){
							temp = i;
						}
					}
					livesdmax=temp;
				}

					/* close the socket */
					close(frsock);
				} else {

					printf("%s(%hu): %s", clienthost, clientport, msg);
					 int j;
					for(j=servsock+1; j<=livesdmax+1; j++){
						if(j!=frsock && (FD_ISSET(j, &livesdset)!=0)){
							sendtext(j,msg);
							//printf("send to %d, %s\n", j, msg);
							mflag = 1;
						}
					}

					/* display the message */
					//printf("%s(%hu): %s", clienthost, clientport, msg);
					mflag = 1;
					/* free the message */
					free(msg);
				}
			}
		}

		/* look for connect requests */
		if (mflag == 0 && FD_ISSET(frsock, &livesdset)==0  ) {

			struct sockaddr_in newClient;
			socklen_t leng = sizeof(newClient);
			int csd;
			csd = accept(servsock, (void *)&newClient, &leng);

			/* if accept is fine? */
			if (csd != -1) {
				char * clienthost; /* host name of the client */
				ushort clientport; /* port number of the client */

				 if(csd > livesdmax){
					 livesdmax = csd;
				 }

				struct hostent *chost;
				int cLen;
				chost = gethostbyaddr(&(newClient.sin_addr.s_addr), sizeof(newClient.sin_addr), AF_INET);
				clienthost = chost->h_name;
				clientport = ntohs(newClient.sin_port);
				printf("admin: connect from '%s' at '%hu'\n", clienthost,
						clientport);

				 FD_SET(frsock, &livesdset);

			} else {
				perror("accept");
				exit(0);
			}
		}

	}
	return 0;
}
/*--------------------------------------------------------------------*/
