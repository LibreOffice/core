
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

//#include <com/sun/star/presentation/AnimationEffect.hpp>


/**
* The port for use for the main communication between LibO and remote control app.
*/
#define PORT 1599


class XSlideShowController;

class Receiver;

namespace sd
{

    class Server
        {
        public:
            Server();
            ~Server();
            void setPresentationController( XSlideShowController aController) { mController = aController; }
            
        private:
            int mSocket;
//     static vector<Server> our_mServerList;

            void listen();
            
            Receiver mReceiver;
//             Transmitter mTransmitter;
        };
    }
}

#endif // _SD_IMPRESSREMOTE_SERVER_HXX