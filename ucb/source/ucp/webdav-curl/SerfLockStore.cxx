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
#include <sal/log.hxx>
#include <osl/time.h>
#include <osl/thread.hxx>
#include <salhelper/thread.hxx>

#include <com/sun/star/ucb/LockScope.hpp>

#include "CurlSession.hxx"
#include "SerfLockStore.hxx"

using namespace http_dav_ucp;

namespace http_dav_ucp {

class TickerThread : public salhelper::Thread
{
    bool m_bFinish;
    SerfLockStore & m_rLockStore;

public:

    explicit TickerThread( SerfLockStore & rLockStore )
        : Thread( "WebDavTickerThread" ), m_bFinish( false ),
          m_rLockStore( rLockStore ) {}

    void finish() { m_bFinish = true; }

private:

    virtual void execute();
};

} // namespace http_dav_ucp


void TickerThread::execute()
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
        salhelper::Thread::wait( aTV );
    }

    SAL_INFO("ucb.ucp.webdav",  "TickerThread: stop." );
}


SerfLockStore::SerfLockStore()
    : m_bFinishing( false )
{
}


SerfLockStore::~SerfLockStore()
{
    stopTicker();
    m_bFinishing = true;

    // release active locks, if any.
    SAL_WARN_IF( !m_aLockInfoMap.empty(), "ucb.ucp.webdav",
                "SerfLockStore::~SerfLockStore - Releasing active locks!" );

    for ( auto& rLockInfo : m_aLockInfoMap )
    {
        rLockInfo.second.m_xSession->NonInteractive_UNLOCK(rLockInfo.first);
    }
}

bool SerfLockStore::finishing() const
{
    return m_bFinishing;
}

void SerfLockStore::startTicker()
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pTickerThread.is() )
    {
        m_pTickerThread = new TickerThread( *this );
        m_pTickerThread->launch();
    }
}


void SerfLockStore::stopTicker()
{
    rtl::Reference<TickerThread> pTickerThread;
    {
        osl::MutexGuard aGuard( m_aMutex );

        if (!m_pTickerThread.is())
        {
            return; // nothing to do
        }
        m_pTickerThread->finish(); // needs mutex
        // the TickerThread may run refreshLocks() at most once after this
        pTickerThread = m_pTickerThread;
        m_pTickerThread.clear();
    }

    pTickerThread->join(); // without m_aMutex locked (to prevent deadlock)
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

OUString const*
SerfLockStore::getLockTokenForURI(OUString const& rURI, css::ucb::Lock const*const pLock)
{
    osl::MutexGuard aGuard( m_aMutex );

    auto const it(m_aLockInfoMap.find(rURI));

    if (it == m_aLockInfoMap.end())
    {
        return nullptr;
    }
    if (!pLock) // any lock will do
    {
        return &it->second.m_sToken;
    }
    // 0: EXCLUSIVE 1: SHARED
    if (it->second.m_Lock.Scope == ucb::LockScope_SHARED && pLock->Scope == ucb::LockScope_EXCLUSIVE)
    {
        return nullptr;
    }
    assert(it->second.m_Lock.Type == pLock->Type); // only WRITE possible
    if (it->second.m_Lock.Depth < pLock->Depth)
    {
        return nullptr;
    }
    assert(it->second.m_Lock.Owner == pLock->Owner); // only own locks expected
    // ignore Timeout ?
    return &it->second.m_sToken;
}

void SerfLockStore::addLock( const OUString& rURI,
                             ucb::Lock const& rLock,
                             const OUString& sToken,
                             rtl::Reference<CurlSession> const & xSession,
                             sal_Int32 nLastChanceToSendRefreshRequest )
{
    osl::MutexGuard aGuard( m_aMutex );

    m_aLockInfoMap[ rURI ]
        = LockInfo(sToken, rLock, xSession, nLastChanceToSendRefreshRequest);

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

    for ( auto& rLockInfo : m_aLockInfoMap )
    {
        LockInfo & rInfo = rLockInfo.second;
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
                if (rInfo.m_xSession->NonInteractive_LOCK(
                         rLockInfo.first, nlastChanceToSendRefreshRequest))
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
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
