/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <stdlib.h>
#include <algorithm>
#include <vector>

#include "sddll.hxx"
#include "Server.hxx"
#include "Receiver.hxx"

using namespace std;
using namespace sd;
using rtl::OString;

Server::Server()
:  Thread( "ServerThread" ), mSocket()
{
}

Server::~Server()
{
}

// Run as a thread
void Server::listenThread()
{
    Transmitter aTransmitter( mStreamSocket );
    Receiver aReceiver( &aTransmitter );
    // TODO: decryption
    while (true)
    {
        sal_uInt64 aRet, aRead;
        vector<char> aBuffer;
        vector<OString> aCommand;
        aRead = 0;
        while ( true )
        {
            aBuffer.resize( aRead + 100 );
            aRet = mStreamSocket.recv( &aBuffer[aRead], 100 );
            if ( aRet == 0 )
            {
                return; // closed
            }
            aRead += aRet;
            vector<char>::iterator aIt;
            while ( (aIt = find( aBuffer.begin() + aRead - aRet, aBuffer.end(), '\n' ))
                != aBuffer.end() )
            {
                fprintf( stderr, "we have string\n" );
                sal_uInt64 aLocation = aIt - aBuffer.begin();

                aCommand.push_back( OString( &(*aBuffer.begin()), aLocation ) );

                if ( aIt == aBuffer.begin() )
                {
                    aReceiver.parseCommand( aCommand );
                    aCommand.clear();
                }
                aBuffer.erase( aBuffer.begin(), aIt + 1 ); // Also delete the newline
                aRead -= (aLocation + 1);
            }
        }

        // TODO: deal with transmision errors gracefully.
    }
}


void Server::execute()
{
    osl::SocketAddr aAddr( "0", PORT );
    if ( !mSocket.bind( aAddr ) )
    {
        // Error binding
    }

    if ( !mSocket.listen(3) )
    {
        // Error listening
    }
    while ( true )
    {
        fprintf( stderr, "Awaiting a connection.\n" );
        mSocket.acceptConnection( mStreamSocket );
        fprintf( stderr, "Accepted a connection!\n" );
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
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
