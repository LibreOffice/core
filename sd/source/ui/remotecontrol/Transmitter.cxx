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
    mQueueMutex(),
    mLowPriority(),
    mHighPriority()
{
    fprintf( stderr, "Address of low queue in constructor:%p\n", &mLowPriority );
}

void Transmitter::execute()
{
    fprintf( stderr, "Waiting\n" );
    while ( true )
    {
        mQueuesNotEmpty.wait();
        fprintf( stderr, "Continuing after condition\n" );
        while ( true )
        {
            fprintf( stderr, "Trying to acquire mutex in Transmitter Thread\n" );
            ::osl::MutexGuard aQueueGuard( mQueueMutex );
            fprintf( stderr, "Acquired mutex in Transmitter Thread\n" );
            while ( mHighPriority.size() )
            {
                OString aMessage( mHighPriority.front() );
                mHighPriority.pop();
                fprintf(stderr , " Writing HIGHP:\n%s<<END>>", aMessage.getStr() );
                mStreamSocket.write( aMessage.getStr(), aMessage.getLength() );
            }

            if( mLowPriority.size() )
            {
                OString aMessage( mLowPriority.front() );
                mLowPriority.pop();
                fprintf(stderr , " Writing LOWP:\n%s<<END>>", aMessage.getStr() );
                mStreamSocket.write( aMessage.getStr(), aMessage.getLength() );
            }

            if ( mLowPriority.empty() && mHighPriority.empty() )
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

void Transmitter::addMessage( const OString& aMessage, const Priority aPriority )
{
    fprintf(stderr, "Acquiring\n");
    ::osl::MutexGuard aQueueGuard( mQueueMutex );
    fprintf(stderr, "Acquired\n" );
    fprintf( stderr, "Address of low queue in addMessge:%p\n", &mLowPriority );
    switch ( aPriority )
    {
        case Priority::LOW:
            fprintf(stderr, "PushingLow\n");
            mLowPriority.push( aMessage );
            break;
        case Priority::HIGH:
            fprintf(stderr, "PushingHigh\n<<<%s>>>\n", aMessage.getStr() );
            mHighPriority.push( aMessage );
            break;
    }
    fprintf( stderr, "Setting\n" );
    if ( !mQueuesNotEmpty.check() )
    {
        mQueuesNotEmpty.set();
        fprintf( stderr, "Condition has now been set\n" );
    }
    else
    {
        fprintf( stderr, "Condition was already set\n" );
    }
    fprintf( stderr, "Added\n" );
    fprintf( stderr, "Front:\n<<<%s>>>\n", mHighPriority.front().getStr() );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */