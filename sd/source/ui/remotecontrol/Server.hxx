
#ifndef _SD_IMPRESSREMOTE_SERVER_HXX
#define _SD_IMPRESSREMOTE_SERVER_HXX

// SERVER
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <osl/socket.hxx>
//#include <com/sun/star/presentation/AnimationEffect.hpp>

#include <salhelper/thread.hxx>

#include "Receiver.hxx"

/**
* The port for use for the main communication between LibO and remote control app.
*/
#define PORT 1599

#define CHARSET RTL_TEXTENCODING_UTF8

namespace sd
{

    class Server : public salhelper::Thread
    {
        public:
	    static void setup();
        private:
            Server();
            ~Server();
	    static Server *spServer;
            osl::AcceptorSocket mSocket;
            osl::StreamSocket mStreamSocket;
            void listenThread();
            Receiver mReceiver;
            void execute();
    };
}

#endif // _SD_IMPRESSREMOTE_SERVER_HXX
