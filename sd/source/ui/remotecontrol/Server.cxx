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
:  Thread( "ServerThread" ), mSocket(), mReceiver()
{
}

Server::~Server()
{
}

// Run as a thread
void Server::listenThread()
{
    // TODO: decryption
    while (true)
    {
        sal_uInt64 aRet, aRead;
        vector<char> aBuffer;
        vector<OString> aCommand;
        sal_Bool finished = false;
        aRead = 0;
        while ( !finished )
        {
            aBuffer.resize( aRead + 100 );
            aRet = mStreamSocket.recv( &aBuffer[aRead], 100 );
            if ( aRet == 0 )
            {
                return; // closed
            }
            vector<char>::iterator aIt;
            aIt = find( aBuffer.begin(), aBuffer.end(), '\n' ); // add aRead
            aRead += aRet;
            if ( aIt == aBuffer.end() )
            {
                fprintf( stderr, "Continuing\n" );
                continue;
            }
            fprintf( stderr, "parsing\n" );
            sal_uInt64 aLocation = aIt - aBuffer.begin();

            vector<char> aTemp( aLocation );
            memcpy( &(*aTemp.begin()),  &(*aBuffer.begin()), aLocation );
            aTemp.push_back( 0 );

            aBuffer.erase( aBuffer.begin(), aBuffer.begin() + aLocation + 1 ); // Also delete the newline
            aRead -= aLocation;

            aCommand.push_back( OString( &(*aTemp.begin()) ) );
            if ( (*aTemp.begin()) == 0 )
            {
                mReceiver.parseCommand( aCommand, mStreamSocket );
                aCommand.clear();
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
