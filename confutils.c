/*--------------------------------------------------------------------*/
/* functions to connect clients and server */

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <errno.h>

#include <stdlib.h>

#include <unistd.h>

#define MAXNAMELEN 256
/*--------------------------------------------------------------------*/

/*----------------------------------------------------------------*/
/* prepare server to accept requests
 returns file descriptor of socket
 returns -1 on error
 */
int startserver() {
	int sd; /* socket descriptor */

	char * servhost; /* full name of this host */
	ushort servport; /* port assigned to this server */


	 struct sockaddr_in sa;
	 struct hostent *hp;
	 memset(&sa, 0, sizeof(sa));
	 //sd = socket(AF_INET,SOCK_STREAM,0);
	 if((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	       fprintf(stderr, "Can't create socket.\n");
	       return -1;
	   }

	 sa.sin_family = AF_INET;
	 sa.sin_port = htons(0);
	 sa.sin_addr.s_addr = htonl(INADDR_ANY);
	 if(bind(sd, (struct sockaddr *) &sa, sizeof(sa))!= -1){
		//  perror("bind");
		//  exit(1);
	 }
	/* we are ready to receive connections */
	listen(sd, 5);


	 servhost = malloc(MAXNAMELEN);
	 if( gethostname(servhost, MAXNAMELEN) < 0 )
	 		return -1;
	 if( (hp = gethostbyname(servhost)) < 0 )
	 		return -1;

	 strcpy(servhost, hp->h_name);

	socklen_t salen = sizeof(sa);


	if (getsockname(sd, (struct sockaddr *)&sa, &salen) < 0){
 		perror("Error: getsockname() error");
 		exit(1);
 	}

	servport = ntohs(sa.sin_port);

	/* ready to accept requests */
	printf("admin: started server on '%s' at '%hu'\n", servhost, servport);
	free(servhost);
	return (sd);
}
/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/
/*
 establishes connection with the server
 returns file descriptor of socket
 returns -1 on error
 */
int hooktoserver(char *servhost, ushort servport) {
	int sd; /* socket descriptor */

	ushort clientport; /* port assigned to this client */


	sd = socket(PF_INET, SOCK_STREAM, 0);

	 struct sockaddr_in sin;
	 bzero(&sin, sizeof(struct sockaddr_in));
	 		 		//printf("%s, %d\n", servhost, servport);
	 sin.sin_family = AF_INET;
	 	
	 sin.sin_port = htons(servport);

//printf("GGGGGGG\n");
	 struct hostent *hostEnt;

	 if(hostEnt = gethostbyname(servhost))
			 memcpy(&sin.sin_addr, hostEnt->h_addr, hostEnt->h_length);
	 else {
			 fprintf(stderr, "Can't get host by name.\n");
			 close(sd);
			 return -1;
	 }

	 if(connect(sd, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
			 fprintf(stderr, "Can't connect to server.\n");
			 close(sd);
			 return -1;
	 }

	 int sinlen = sizeof(sin);


	 if (getsockname(sd, (struct sockaddr *)&sin, &sinlen) < 0){
	 	perror("Error: getsockname() error");
	 	exit(1);
	 }

	 clientport = ntohs(sin.sin_port);
	/* succesful. return socket descriptor */
	printf("admin: connected to server on '%s' at '%hu' thru '%hu'\n", servhost,
			servport, clientport);
	printf(">");
	fflush(stdout);
	return (sd);
}
/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/
int readn(int sd, char *buf, int n) {
	int toberead;
	char * ptr;

	toberead = n;
	ptr = buf;
	while (toberead > 0) {
		int byteread;

		byteread = read(sd, ptr, toberead);
		if (byteread <= 0) {
			if (byteread == -1)
				perror("read");
			return (0);
		}

		toberead -= byteread;
		ptr += byteread;
	}
	return (1);
}

char *recvtext(int sd) {
	char *msg;
	long len;

	/* read the message length */
	if (!readn(sd, (char *) &len, sizeof(len))) {
		return (NULL);
	}
	len = ntohl(len);

	/* allocate space for message text */
	msg = NULL;
	if (len > 0) {
		msg = (char *) malloc(len);
		if (!msg) {
			fprintf(stderr, "error : unable to malloc\n");
			return (NULL);
		}

		/* read the message text */
		if (!readn(sd, msg, len)) {
			free(msg);
			return (NULL);
		}
	}

	/* done reading */
	return (msg);
}

int sendtext(int sd, char *msg) {
	long len;

	/* write lent */
	len = (msg ? strlen(msg) + 1 : 0);
	len = htonl(len);
	write(sd, (char *) &len, sizeof(len));

	/* write message text */
	len = ntohl(len);
	if (len > 0)
		write(sd, msg, len);
	return (1);
}
/*----------------------------------------------------------------*/
