#if !defined(__WIN32_SOCKET_TEMPLATE_H__)

#include <stdio.h>
#include <unistd.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

void close_nb_socket(int sockfd);
int open_nb_socket(const char *addr, const char *port);

int open_nb_socket(const char *addr, const char *port) {

    WSADATA wsaData;
    int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (res != 0) {
        fprintf(stderr, "error: WSAStartup failed with error: %i", res);
        return -1;
    }

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;      // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;  // Must be TCP
    hints.ai_protocol = IPPROTO_TCP;  //

    struct addrinfo *p, *servinfo;
    // get address information 
    int rv = getaddrinfo(addr, port, &hints, &servinfo);
    if (rv != 0) {
        fprintf(stderr, "error: getaddrinfo: %s", gai_strerror(rv));
        WSACleanup();
        return -1;
    }

    /* open the first possible socket */
    SOCKET hSocket = INVALID_SOCKET;
    for (p = servinfo; p != NULL; p = p->ai_next) {
        hSocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

        if (hSocket == INVALID_SOCKET) {
            continue;
        }

        // connect to server
        if (connect(hSocket, p->ai_addr, (int)p->ai_addrlen) != INVALID_SOCKET) {
            break;
        }

        closesocket(hSocket);
        hSocket = INVALID_SOCKET;

    }

    // free servinfo
    freeaddrinfo(servinfo);

    if (p == NULL) {               // No address succeeded
        fprintf(stderr, "error: Could not connect");
        WSACleanup();
        return -1;
    }

    // make non-blocking
    if (hSocket != INVALID_SOCKET) {
        uint32_t mode = 1;  // FIONBIO returns size on 32b
        ioctlsocket(hSocket, FIONBIO, (u_long *)&mode);
    }

    int flag = 1;
    res = setsockopt(hSocket, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag));
    if (res != 0) {
        closesocket(hSocket);
        WSACleanup();
        return -1;
    }

    return hSocket;
}

void close_nb_socket(int sockfd) {
    if (sockfd != -1) {
        close(sockfd);
    }
}
#endif

#endif
