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

Transmitter::Transmitter( BufferedStreamSocket* aSocket )
  : Thread( "TransmitterThread" ),
    pStreamSocket( aSocket ),
    mQueuesNotEmpty(),
    mFinishRequested(),
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

        if ( mFinishRequested.check() )
            return;

        ::osl::MutexGuard aQueueGuard( mQueueMutex );
        if ( !mHighPriority.empty() )
        {
            OString aMessage( mHighPriority.front() );
            mHighPriority.pop();
            pStreamSocket->write( aMessage.getStr(), aMessage.getLength() );
        }
        else if ( !mLowPriority.empty() )
        {
            OString aMessage( mLowPriority.front() );
            mLowPriority.pop();
            pStreamSocket->write( aMessage.getStr(), aMessage.getLength() );
        }

        if ( mLowPriority.empty() && mHighPriority.empty() )
        {
            mQueuesNotEmpty.reset();
        }
    }

}

void Transmitter::notifyFinished()
{
    mFinishRequested.set();
    mQueuesNotEmpty.set();
}

Transmitter::~Transmitter()
{

}

void Transmitter::addMessage( const OString& aMessage, const Priority aPriority )
{
    ::osl::MutexGuard aQueueGuard( mQueueMutex );
    switch ( aPriority )
    {
        case PRIORITY_LOW:
            mLowPriority.push( aMessage );
            break;
        case PRIORITY_HIGH:
            mHighPriority.push( aMessage );
            break;
    }
    if ( !mQueuesNotEmpty.check() )
    {
        mQueuesNotEmpty.set();
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */