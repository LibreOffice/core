/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"

#include <ne_locks.h>
#include <ne_uri.h>
#include "rtl/ustring.hxx"
#include "osl/time.h"
#include "osl/thread.hxx"
#include "SerfSession.hxx"
#include "SerfLockStore.hxx"

using namespace http_dav_ucp;

namespace http_dav_ucp {

class TickerThread : public osl::Thread
{
    bool m_bFinish;
    SerfLockStore & m_rLockStore;

public:

    TickerThread( SerfLockStore & rLockStore )
    : osl::Thread(), m_bFinish( false ), m_rLockStore( rLockStore ) {}

    void finish() { m_bFinish = true; }

protected:

    virtual void SAL_CALL run();
};

} // namespace http_dav_ucp

// -------------------------------------------------------------------
void TickerThread::run()
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
        wait( aTV );
    }

    OSL_TRACE( "TickerThread: stop." );
}

// -------------------------------------------------------------------
SerfLockStore::SerfLockStore()
    : m_pSerfLockStore( ne_lockstore_create() ),
      m_pTickerThread( 0 )
{
    OSL_ENSURE( m_pSerfLockStore, "Unable to create neon lock store!" );
}

// -------------------------------------------------------------------
SerfLockStore::~SerfLockStore()
{
    stopTicker();

    // release active locks, if any.
    OSL_ENSURE( m_aLockInfoMap.size() == 0,
                "SerfLockStore::~SerfLockStore - Releasing active locks!" );

    LockInfoMap::const_iterator it( m_aLockInfoMap.begin() );
    const LockInfoMap::const_iterator end( m_aLockInfoMap.end() );
    while ( it != end )
    {
        SerfLock * pLock = (*it).first;
        (*it).second.xSession->UNLOCK( pLock );

        ne_lockstore_remove( m_pSerfLockStore, pLock );
        ne_lock_destroy( pLock );

        ++it;
    }

    ne_lockstore_destroy( m_pSerfLockStore );
}

// -------------------------------------------------------------------
void SerfLockStore::startTicker()
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pTickerThread )
    {
        m_pTickerThread = new TickerThread( *this );
        m_pTickerThread->create();
    }
}

// -------------------------------------------------------------------
void SerfLockStore::stopTicker()
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( m_pTickerThread )
    {
        m_pTickerThread->finish();
        m_pTickerThread->join();
        delete m_pTickerThread;
        m_pTickerThread = 0;
    }
}

// -------------------------------------------------------------------
void SerfLockStore::registerSession( HttpSession * pHttpSession )
{
    osl::MutexGuard aGuard( m_aMutex );

    ne_lockstore_register( m_pSerfLockStore, pHttpSession );
}

// -------------------------------------------------------------------
SerfLock * SerfLockStore::findByUri( rtl::OUString const & rUri )
{
    osl::MutexGuard aGuard( m_aMutex );

    ne_uri aUri;
    ne_uri_parse( rtl::OUStringToOString(
        rUri, RTL_TEXTENCODING_UTF8 ).getStr(), &aUri );
    return ne_lockstore_findbyuri( m_pSerfLockStore, &aUri );
}

// -------------------------------------------------------------------
void SerfLockStore::addLock( SerfLock * pLock,
                             rtl::Reference< SerfSession > const & xSession,
                             sal_Int32 nLastChanceToSendRefreshRequest )
{
    osl::MutexGuard aGuard( m_aMutex );

    ne_lockstore_add( m_pSerfLockStore, pLock );
    m_aLockInfoMap[ pLock ]
        = LockInfo( xSession, nLastChanceToSendRefreshRequest );

    startTicker();
}

// -------------------------------------------------------------------
void SerfLockStore::updateLock( SerfLock * pLock,
                                sal_Int32 nLastChanceToSendRefreshRequest )
{
    osl::MutexGuard aGuard( m_aMutex );

    LockInfoMap::iterator it( m_aLockInfoMap.find( pLock ) );
    OSL_ENSURE( it != m_aLockInfoMap.end(),
                "SerfLockStore::updateLock: lock not found!" );

    if ( it != m_aLockInfoMap.end() )
    {
        (*it).second.nLastChanceToSendRefreshRequest
            = nLastChanceToSendRefreshRequest;
    }
}

// -------------------------------------------------------------------
void SerfLockStore::removeLock( SerfLock * pLock )
{
    osl::MutexGuard aGuard( m_aMutex );

    m_aLockInfoMap.erase( pLock );
    ne_lockstore_remove( m_pSerfLockStore, pLock );

    if ( m_aLockInfoMap.size() == 0 )
        stopTicker();
}

// -------------------------------------------------------------------
void SerfLockStore::refreshLocks()
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
