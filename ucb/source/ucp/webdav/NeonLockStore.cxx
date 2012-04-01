/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include "warnings_guard_ne_locks.h"
#include <ne_uri.h>
#include "rtl/ustring.hxx"
#include "osl/time.h"
#include "osl/thread.hxx"
#include "salhelper/thread.hxx"
#include "NeonSession.hxx"
#include "NeonLockStore.hxx"

using namespace webdav_ucp;

namespace webdav_ucp {

class TickerThread : public salhelper::Thread
{
    bool m_bFinish;
    NeonLockStore & m_rLockStore;

public:

    TickerThread( NeonLockStore & rLockStore )
    : Thread( "NeonTickerThread" ), m_bFinish( false ),
      m_rLockStore( rLockStore ) {}

    void finish() { m_bFinish = true; }

private:

    virtual void execute();
};

} // namespace webdav_ucp

// -------------------------------------------------------------------
void TickerThread::execute()
{
    OSL_TRACE( "TickerThread: start." );

    // we have to go through the loop more often to be able to finish ~quickly
    const int nNth = 25;

    int nCount = nNth;
    while ( !m_bFinish )
    {
        if ( nCount-- <= 0 )
        {
            m_rLockStore.refreshLocks();
            nCount = nNth;
        }

        TimeValue aTV;
        aTV.Seconds = 0;
        aTV.Nanosec = 1000000000 / nNth;
        salhelper::Thread::wait( aTV );
    }

    OSL_TRACE( "TickerThread: stop." );
}

// -------------------------------------------------------------------
NeonLockStore::NeonLockStore()
    : m_pNeonLockStore( ne_lockstore_create() )
{
    OSL_ENSURE( m_pNeonLockStore, "Unable to create neon lock store!" );
}

// -------------------------------------------------------------------
NeonLockStore::~NeonLockStore()
{
    stopTicker();

    // release active locks, if any.
    OSL_ENSURE( m_aLockInfoMap.empty(),
                "NeonLockStore::~NeonLockStore - Releasing active locks!" );

    LockInfoMap::const_iterator it( m_aLockInfoMap.begin() );
    const LockInfoMap::const_iterator end( m_aLockInfoMap.end() );
    while ( it != end )
    {
        NeonLock * pLock = (*it).first;
        (*it).second.xSession->UNLOCK( pLock );

        ne_lockstore_remove( m_pNeonLockStore, pLock );
        ne_lock_destroy( pLock );

        ++it;
    }

    ne_lockstore_destroy( m_pNeonLockStore );
}

// -------------------------------------------------------------------
void NeonLockStore::startTicker()
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pTickerThread.is() )
    {
        m_pTickerThread = new TickerThread( *this );
        m_pTickerThread->launch();
    }
}

// -------------------------------------------------------------------
void NeonLockStore::stopTicker()
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( m_pTickerThread.is() )
    {
        m_pTickerThread->finish();
        m_pTickerThread->join();
        m_pTickerThread.clear();
    }
}

// -------------------------------------------------------------------
void NeonLockStore::registerSession( HttpSession * pHttpSession )
{
    osl::MutexGuard aGuard( m_aMutex );

    ne_lockstore_register( m_pNeonLockStore, pHttpSession );
}

// -------------------------------------------------------------------
NeonLock * NeonLockStore::findByUri( rtl::OUString const & rUri )
{
    osl::MutexGuard aGuard( m_aMutex );

    ne_uri aUri;
    ne_uri_parse( rtl::OUStringToOString(
        rUri, RTL_TEXTENCODING_UTF8 ).getStr(), &aUri );
    return ne_lockstore_findbyuri( m_pNeonLockStore, &aUri );
}

// -------------------------------------------------------------------
void NeonLockStore::addLock( NeonLock * pLock,
                             rtl::Reference< NeonSession > const & xSession,
                             sal_Int32 nLastChanceToSendRefreshRequest )
{
    osl::MutexGuard aGuard( m_aMutex );

    ne_lockstore_add( m_pNeonLockStore, pLock );
    m_aLockInfoMap[ pLock ]
        = LockInfo( xSession, nLastChanceToSendRefreshRequest );

    startTicker();
}

// -------------------------------------------------------------------
void NeonLockStore::updateLock( NeonLock * pLock,
                                sal_Int32 nLastChanceToSendRefreshRequest )
{
    osl::MutexGuard aGuard( m_aMutex );

    LockInfoMap::iterator it( m_aLockInfoMap.find( pLock ) );
    OSL_ENSURE( it != m_aLockInfoMap.end(),
                "NeonLockStore::updateLock: lock not found!" );

    if ( it != m_aLockInfoMap.end() )
    {
        (*it).second.nLastChanceToSendRefreshRequest
            = nLastChanceToSendRefreshRequest;
    }
}

// -------------------------------------------------------------------
void NeonLockStore::removeLock( NeonLock * pLock )
{
    osl::MutexGuard aGuard( m_aMutex );

    m_aLockInfoMap.erase( pLock );
    ne_lockstore_remove( m_pNeonLockStore, pLock );

    if ( m_aLockInfoMap.empty() )
        stopTicker();
}

// -------------------------------------------------------------------
void NeonLockStore::refreshLocks()
{
    osl::MutexGuard aGuard( m_aMutex );

    LockInfoMap::iterator it( m_aLockInfoMap.begin() );
    const LockInfoMap::const_iterator end( m_aLockInfoMap.end() );
    while ( it != end )
    {
        LockInfo & rInfo = (*it).second;
        if ( rInfo.nLastChanceToSendRefreshRequest != -1 )
        {
            // 30 seconds or less remaining until lock expires?
            TimeValue t1;
            osl_getSystemTime( &t1 );
            if ( rInfo.nLastChanceToSendRefreshRequest - 30
                     <= sal_Int32( t1.Seconds ) )
            {
                // refresh the lock.
                sal_Int32 nlastChanceToSendRefreshRequest = -1;
                if ( rInfo.xSession->LOCK(
                         (*it).first,
                         /* out param */ nlastChanceToSendRefreshRequest ) )
                {
                    rInfo.nLastChanceToSendRefreshRequest
                        = nlastChanceToSendRefreshRequest;
                }
                else
                {
                    // refresh failed. stop auto-refresh.
                    rInfo.nLastChanceToSendRefreshRequest = -1;
                }
            }
        }
        ++it;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
