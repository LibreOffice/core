/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "Transmitter.hxx"

using rtl::OString;
using namespace std;
using namespace osl; // Sockets etc.
using namespace sd;

Transmitter::Transmitter( StreamSocket &aSocket )
  : Thread( "TransmitterThread" ),
    mStreamSocket( aSocket ),
    mQueuesNotEmpty(),
    mQueueMutex()
{
    launch();
    // Start a thread
}

void
Transmitter::execute()
{
    fprintf( stderr, "Waiting\n" );
    while( mQueuesNotEmpty.wait() )
    {
        while ( true )
        {
            osl::MutexGuard aQueueGuard( mQueueMutex );
            while ( mHighPriority.size() )
            {
                OString aMessage = mHighPriority.front();
                mHighPriority.pop();
                fprintf(stderr , " Writing HIGHP:\n%s<<END>>", aMessage.getStr() );
                mStreamSocket.write( aMessage.getStr(), aMessage.getLength() );
            }

            if( mLowPriority.size() )
            {
                OString aMessage = mLowPriority.front();
                mLowPriority.pop();
                fprintf(stderr , " Writing LOWP:\n%s<<END>>", aMessage.getStr() );
                mStreamSocket.write( aMessage.getStr(), aMessage.getLength() );
            }

            if ( (mLowPriority.size() == 0) && (mHighPriority.size() == 0) )
            {
                mQueuesNotEmpty.reset();
                break;
            }
        }

    }

}

Transmitter::~Transmitter()
{

}

void Transmitter::addMessage( OString aMessage, Priority aPriority )
{
    osl::MutexGuard aQueueGuard( mQueueMutex );
    switch ( aPriority )
    {
        case Priority::LOW:
            mLowPriority.push( aMessage );
            break;
        case Priority::HIGH:
            mHighPriority.push( aMessage );
            break;
    }
    mQueuesNotEmpty.set();

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */