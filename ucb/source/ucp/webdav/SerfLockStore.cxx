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

#include <rtl/ustring.hxx>
#include <osl/time.h>
#include <osl/thread.hxx>
#include "SerfSession.hxx"
#include "SerfLockStore.hxx"

using namespace http_dav_ucp;

namespace http_dav_ucp {

class TickerThread : public osl::Thread
{
    bool m_bFinish;
    SerfLockStore & m_rLockStore;

public:

    explicit TickerThread( SerfLockStore & rLockStore )
        : osl::Thread(), m_bFinish( false ), m_rLockStore( rLockStore ) {}

    void finish() { m_bFinish = true; }

protected:

    virtual void SAL_CALL run() override;
};

} // namespace http_dav_ucp


void TickerThread::run()
{
    osl_setThreadName("http_dav_ucp::TickerThread");

    SAL_INFO("ucb.ucp.webdav",  "TickerThread: start." );

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

    SAL_INFO("ucb.ucp.webdav",  "TickerThread: stop." );
}


SerfLockStore::SerfLockStore()
    : m_pTickerThread( 0 )
    , m_bFinishing( false )
{
}


SerfLockStore::~SerfLockStore()
{
    stopTicker();
    m_bFinishing = true;

    // release active locks, if any.
    SAL_WARN_IF( !m_aLockInfoMap.empty(), "ucb.ucp.webdav",
                "SerfLockStore::~SerfLockStore - Releasing active locks!" );

    LockInfoMap::const_iterator it( m_aLockInfoMap.begin() );
    const LockInfoMap::const_iterator end( m_aLockInfoMap.end() );
    while ( it != end )
    {
        (*it).second.m_xSession->UNLOCK( (*it).first );
        ++it;
    }
}

bool SerfLockStore::finishing() const
{
    return m_bFinishing;
}

void SerfLockStore::startTicker()
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pTickerThread )
    {
        m_pTickerThread = new TickerThread( *this );
        m_pTickerThread->create();
    }
}


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

OUString SerfLockStore::getLockToken( const OUString& rLock )
{
    osl::MutexGuard aGuard( m_aMutex );

    LockInfoMap::const_iterator it( m_aLockInfoMap.find( rLock ) );
    if ( it != m_aLockInfoMap.end() )
        return (*it).second.m_sToken;

    SAL_WARN("ucb.ucp.webdav", "SerfLockStore::getLockToken: lock not found!" );
    return OUString();
}

void SerfLockStore::addLock( const OUString& rLock,
                             const OUString& sToken,
                             rtl::Reference< SerfSession > const & xSession,
                             sal_Int32 nLastChanceToSendRefreshRequest )
{
    osl::MutexGuard aGuard( m_aMutex );

    m_aLockInfoMap[ rLock ]
        = LockInfo( sToken, xSession, nLastChanceToSendRefreshRequest );

    startTicker();
}


void SerfLockStore::updateLock( const OUString& rLock,
                                sal_Int32 nLastChanceToSendRefreshRequest )
{
    osl::MutexGuard aGuard( m_aMutex );

    LockInfoMap::iterator it( m_aLockInfoMap.find( rLock ) );
    SAL_WARN_IF( it == m_aLockInfoMap.end(), "ucb.ucp.webdav",
                "SerfLockStore::updateLock: lock not found!" );

    if ( it != m_aLockInfoMap.end() )
    {
        (*it).second.m_nLastChanceToSendRefreshRequest
            = nLastChanceToSendRefreshRequest;
    }
}


void SerfLockStore::removeLock( const OUString& rLock )
{
    osl::MutexGuard aGuard( m_aMutex );

    m_aLockInfoMap.erase( rLock );

    if ( m_aLockInfoMap.empty() )
        stopTicker();
}


void SerfLockStore::refreshLocks()
{
    osl::MutexGuard aGuard( m_aMutex );

    LockInfoMap::iterator it( m_aLockInfoMap.begin() );
    const LockInfoMap::const_iterator end( m_aLockInfoMap.end() );
    while ( it != end )
    {
        LockInfo & rInfo = (*it).second;
        if ( rInfo.m_nLastChanceToSendRefreshRequest != -1 )
        {
            // 30 seconds or less remaining until lock expires?
            TimeValue t1;
            osl_getSystemTime( &t1 );
            if ( rInfo.m_nLastChanceToSendRefreshRequest - 30
                     <= sal_Int32( t1.Seconds ) )
            {
                // refresh the lock.
                sal_Int32 nlastChanceToSendRefreshRequest = -1;
                if ( rInfo.m_xSession->LOCK(
                         (*it).first, &nlastChanceToSendRefreshRequest ) )
                {
                    rInfo.m_nLastChanceToSendRefreshRequest
                        = nlastChanceToSendRefreshRequest;
                }
                else
                {
                    // refresh failed. stop auto-refresh.
                    rInfo.m_nLastChanceToSendRefreshRequest = -1;
                }
            }
        }
        ++it;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
