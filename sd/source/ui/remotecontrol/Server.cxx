
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>

#include "sddll.hxx"
#include "Server.hxx"
#include "Receiver.hxx"



using namespace std;
using namespace sd;
using rtl::OUString;
using rtl::OString;

Server::Server()
:  Thread( "ServerThread" ), mSocket(), mStreamSocket(), mReceiver()
{

//         boost::thread t( boost::bind(&Server::listenThread, this ))

}

Server::~Server()
{
}

// Run as a thread
void Server::listenThread()
{
    char aTemp;
    vector<char> aBuffer;
    while (true)
    {
        int aRet;
        while ( (aRet = mStreamSocket.read( &aTemp, 1)) && aTemp != 0x0d ) // look for newline
        {
            aBuffer.push_back( aTemp );
            // TODO: decryption
        }
        if (aRet != 1) // Error reading or connection closed
        {
            return;
        }
        OUString aLengthString = OUString( &aBuffer.front(), aBuffer.size(), RTL_TEXTENCODING_UNICODE );
        OString aTempStr;
        aLengthString.convertToString( &aTempStr, RTL_TEXTENCODING_ASCII_US, 0);
        const sal_Char* aLengthChar = aTempStr.getStr();

        sal_Int32 aLen = strtol( aLengthChar, NULL, 10);

        char *aMessage = new char[aLen];

        if( mStreamSocket.read( (void*) aMessage, aLen ) != aLen)// Error reading or connection closed
        {
            return;
        }
        // Parse.
        mReceiver.parseCommand( aMessage, aLen, NULL );

        delete [] aMessage;

        // TODO: deal with transmision errors gracefully.
    }
}


void Server::execute()
{

    osl::SocketAddr aAddr( "", PORT );
    if ( !mSocket.bind( aAddr ) )
    {
        // Error binding
    }

    if ( !mSocket.listen() )
    {
        // Error listening
    }
    while ( true )
    {
        mSocket.acceptConnection( mStreamSocket );
        listenThread();
    }

}

Server *sd::Server::spServer = NULL;

void Server::setup()
{
  if (spServer)
    return;

  spServer = new Server();
  spServer->launch();
}

void SdDLL::RegisterRemotes()
{
  fprintf( stderr, "Register our remote control goodness\n" );
  sd::Server::setup();
}
