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
}

void Transmitter::execute()
{
    while ( true )
    {
        mQueuesNotEmpty.wait();

        ::osl::MutexGuard aQueueGuard( mQueueMutex );
        if ( !mHighPriority.empty() )
        {
            OString aMessage( mHighPriority.front() );
            mHighPriority.pop();
            mStreamSocket.write( aMessage.getStr(), aMessage.getLength() );
        }
        else if ( !mLowPriority.empty() )
        {
            OString aMessage( mLowPriority.front() );
            mLowPriority.pop();
            mStreamSocket.write( aMessage.getStr(), aMessage.getLength() );
        }

        if ( mLowPriority.empty() && mHighPriority.empty() )
        {
            mQueuesNotEmpty.reset();
        }
    }

}

Transmitter::~Transmitter()
{

}

void Transmitter::addMessage( const OString& aMessage, const Priority aPriority )
{
    ::osl::MutexGuard aQueueGuard( mQueueMutex );
    switch ( aPriority )
    {
        case Priority::LOW:
            mLowPriority.push( aMessage );
            break;
        case Priority::HIGH:
            mHighPriority.push( aMessage );
            break;
    }
    if ( !mQueuesNotEmpty.check() )
    {
        mQueuesNotEmpty.set();
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */