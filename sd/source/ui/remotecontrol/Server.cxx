
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>

#include "Server.hxx"
#include "Receiver.hxx"




#include <boost/thread.hpp>
using namespace std;
using namespace sd::remotecontrol;

Server::Server(char* aEncryptionKey, XSlideShowController aController)
:     mReceiver()
{
    if ( (mSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        // Error opening
    }

    sockaddr_in aAddress;
    aAddress.sin_family = AF_INET;
    aAddress.sin_port = htons(PORT);
    aAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    if ( bind( mSocket, (struct sockaddr*) &aAddress, sizeof(aAddress) ) == -1 )
    {
        // Error binding
    }

    if ( listen ( mSocket, 3 ) == -1 )
    {
        // Error listening
    }
    while (true)
    {
        mNewSocket = accept( mSocket, (struct sockaddr*) &aAddress, (socklen_t*) &aAddrLen );
        boost::thread t( boost::bind(&Server::listenThread, this ))
    }
    // TODO: pass mNewSocket to the thread.
}

Server::~Server()
{
}

// Run as a thread
void Server::listenThread()
{
    char aTemp;
    vector<char> aBuffer();
    while (true)
    {
        int aLength;
        while ( recv( mSocket, *aTemp, 1, 0) && aTemp != 0x0d ) // look for newline
        {
            aBuffer.push_back( aTemp );
            // TODO: decryption
        }
        OUString aLengthString = OUString( aBuffer.front(), aBuffer.size(), RTL_TEXTENCODING_UNICODE );
        const sal_Char* aLengthChar = aLengthString.convertToString( *aCLength, RTL_TEXTENCODING_ASCII_US, 0).getStr();

        sal_uInt32 aLen = strtol( aLengthChar, NULL, 10);

        char *aMessage = new char[aLen];
        recv( mSocket, *aMessage, aLen, 0);
        // Parse.
        mReceiver.parseCommand( aCommand );

        delete [] aMessage;
        // TODO: deal with transmision errors gracefully.
    }
}


// void Server::clientConnected()
// {
//   our_mServerList.insert( our_mServerList.end, new Server() );
//   thread aThread = Boost::thread( listen );
//
// }
//
// void Server::clientDisconnected()
// {
//   our_mServerList::iterator aBegin = our_mServerList.begin;
//   while ( *aBegin != this ) {
//     aBegin++;
//   }
//   if ( *aBegin == this )
//   {
//     our_mServerList.erase( this );
//     delete this;
//   }
// }
