/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <stdlib.h>
#include <vector>

#include "sddll.hxx"
#include "Server.hxx"

using namespace std;
using namespace sd;

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
        vector<char> aBuffer;
        char aReadBuffer[100];
        aRet = mStreamSocket.read


        int aRet;
        char aTemp;
        while ( (aRet = mStreamSocket.read( &aTemp, 1)) && aTemp != 0x0d ) // look for newline
        {
            aBuffer.push_back( aTemp );
        }
        if (aRet != 1) // Error reading or connection closed
        {
            return;
        }
        aBuffer.push_back('\0');
        rtl::OString aTempStr( &aBuffer.front() );

        const sal_Char* aLengthChar = aTempStr.getStr();
        sal_Int32 aLen = strtol( aLengthChar, NULL, 10);

        char *aMessage = new char[aLen+1];
        aMessage[aLen] = '\0';

        if( mStreamSocket.read( (void*) aMessage, aLen ) != aLen) // Error reading or connection closed
        {
            delete [] aMessage;
            return;
        }

        aTempStr = rtl::OString( aMessage ); //, (sal_Int32) aLen, CHARSET, 0u
        const sal_Char* aCommandChar = aTempStr.getStr();

        mReceiver.parseCommand( aCommandChar, aTempStr.getLength(), mStreamSocket );
        delete [] aMessage;

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