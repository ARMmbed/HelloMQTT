#if !defined(LINUX_IPSTACK_H)
#define LINUX_IPSTACK_H

class IPStack 
{
public:    
    IPStack()
    {

    }
    
    int Socket_error(const char* aString)
    {

        if (errno != EINTR && errno != EAGAIN && errno != EINPROGRESS && errno != EWOULDBLOCK)
        {
            if (strcmp(aString, "shutdown") != 0 || (errno != ENOTCONN && errno != ECONNRESET))
                printf("Socket error %s in %s for socket %d\n", strerror(errno), aString, mysock);
        }
        return errno;
    }

    int connect(const char* hostname, int port)
    {
        int type = SOCK_STREAM;
        struct sockaddr_in address;
        int rc = -1;
        sa_family_t family = AF_INET;
        struct addrinfo *result = NULL;
        struct addrinfo hints = {0, AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP, 0, NULL, NULL, NULL};

        if ((rc = getaddrinfo(hostname, NULL, &hints, &result)) == 0)
        {
            struct addrinfo* res = result;

            /* prefer ip4 addresses */
            while (res)
            {
                if (res->ai_family == AF_INET)
                {
                    result = res;
                    break;
                }
                res = res->ai_next;
            }

            if (result->ai_family == AF_INET)
            {
                address.sin_port = htons(port);
                address.sin_family = family = AF_INET;
                address.sin_addr = ((struct sockaddr_in*)(result->ai_addr))->sin_addr;
            }
            else
                rc = -1;

            freeaddrinfo(result);
        }

        if (rc == 0)
        {
            mysock = socket(family, type, 0);
            if (mysock != -1)
            {
                int opt = 1;

                //if (setsockopt(mysock, SOL_SOCKET, SO_NOSIGPIPE, (void*)&opt, sizeof(opt)) != 0)
                //  printf("Could not set SO_NOSIGPIPE for socket %d", mysock);
                
                rc = ::connect(mysock, (struct sockaddr*)&address, sizeof(address));
            }
        }

        return rc;
    }

    int read(char* buffer, int len, int timeout_ms)
    {
        struct timeval interval = {timeout_ms / 1000, (timeout_ms % 1000) * 1000};
        if (interval.tv_sec < 0 || (interval.tv_sec == 0 && interval.tv_usec <= 0))
        {
            interval.tv_sec = 0;
            interval.tv_usec = 100;
        }

        setsockopt(mysock, SOL_SOCKET, SO_RCVTIMEO, (char *)&interval, sizeof(struct timeval));

        //printf("reading %d bytes\n", len);
        int rc = ::recv(mysock, buffer, (size_t)len, 0);
        if (rc == -1)
            Socket_error("read");
        //printf("read %d bytes\n", rc);
        return rc;
    }
    
    int write(char* buffer, int len, int timeout)
    {
        struct timeval tv;

        tv.tv_sec = 0;  /* 30 Secs Timeout */
        tv.tv_usec = timeout * 1000;  // Not init'ing this can cause strange errors

        setsockopt(mysock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));
        int rc = ::write(mysock, buffer, len);
        //printf("write rc %d\n", rc);
        return rc;
    }

    int disconnect()
    {
        return ::close(mysock);
    }
    
private:

    int mysock; 
    
};

#endif