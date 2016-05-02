#include <iostream>
#include <cstring>      // Needed for memset
#include <sys/socket.h> // Needed for the socket functions
#include <netdb.h>      // Needed for the socket functions

#define BUFFER_SIZE (4096)
#define PORT ("4000")

using namespace std;

int main(int argc, char const *argv[]) {

        int status;
        struct addrinfo host_info; // The struct that getaddrinfo() fills up with data.
        struct addrinfo *host_info_list; // Pointer to the to the linked list of host_info's.

        //NULL out host_info
        memset(&host_info, 0, sizeof host_info);

        host_info.ai_family = AF_UNSPEC;     // IP version not specified. Can be both.
        host_info.ai_socktype = SOCK_STREAM; // Use SOCK_STREAM for TCP or SOCK_DGRAM for UDP.
        host_info.ai_flags = AI_PASSIVE;     // IP Wildcard

        status = getaddrinfo(NULL, PORT, &host_info, &host_info_list);

        if (status != 0) {
                cout << "getaddrinfo error" << gai_strerror(status) << endl;
                return -1;
        }

        int socketfd;  // The socket descripter
        socketfd = socket(host_info_list->ai_family, host_info_list->ai_socktype,
                          host_info_list->ai_protocol);
        if (socketfd < 0) {
                std::cout << "socket error " << endl;
                return -1;
        }

        // we use to make the setsockopt() function to make sure the port is not in use
        // by a previous execution of our code. (see man page for more information)
        int yes = 1;
        status = setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        status = bind(socketfd, host_info_list->ai_addr, host_info_list->ai_addrlen);
        if (status == -1) {
                std::cout << "bind error" << std::endl;
                return -1;
        }

        std::cout << "Listen()ing for connections..."  << std::endl;
        status =  listen(socketfd, 5);
        if (status == -1) {
                std::cout << "listen error" << std::endl;
                return -1;
        }


        int new_sd;
        struct sockaddr_storage their_addr;
        socklen_t addr_size = sizeof(their_addr);
        new_sd = accept(socketfd, (struct sockaddr *)&their_addr, &addr_size);
        if (new_sd < 0)
        {
                std::cout << "listen error" << std::endl;
                return -1;
        }
        else
        {
                std::cout << "Connection accepted. Using new socketfd : "  <<  new_sd << std::endl;
        }


        std::cout << "Waiting to recieve data..."  << std::endl;
        ssize_t bytes_recieved = 0;
        char incomming_data_buffer[BUFFER_SIZE];

        //read loop
        for(bytes_recieved=0;; ) {
                bytes_recieved = 0;
                memset( incomming_data_buffer, '\0', sizeof(char)*BUFFER_SIZE );

                bytes_recieved = recv(new_sd, incomming_data_buffer,BUFFER_SIZE, 0);
                // If no data arrives, the program will just wait here until some data arrives.
                if (bytes_recieved == 0) {
                        std::cout << "client shut down." << std::endl;
                        break;
                }
                else if (bytes_recieved < 0) {
                        std::cout << "recieve error!" << std::endl;
                        break;
                }
                std::cout << bytes_recieved << " bytes recieved :" << std::endl;
                incomming_data_buffer[bytes_recieved] = '\0';
                std::cout << "Read data: \""<< incomming_data_buffer << "\"" <<std::endl;

                //Newline read, send response
                if(bytes_recieved > 0 && incomming_data_buffer[bytes_recieved - 1] == '\n') {

                        const char *msg = "pong\n";
                        std::cout << "send()ing back a message... \"" << msg << "\""<< std::endl;
                        int len;
                        ssize_t bytes_sent;
                        len = strlen(msg);
                        bytes_sent = send(new_sd, msg, len, 0);
                }
        }







        std::cout << "Stopping server..." << std::endl;
        freeaddrinfo(host_info_list);

        return 0;
}
