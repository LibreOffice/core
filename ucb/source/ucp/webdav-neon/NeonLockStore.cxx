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


#include <ne_uri.h>
#include "rtl/ustring.hxx"
#include "osl/time.h"
#include "osl/thread.hxx"
#include <osl/diagnose.h>
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

    explicit TickerThread( NeonLockStore & rLockStore )
        : Thread( "NeonTickerThread" ), m_bFinish( false )
        , m_rLockStore( rLockStore )
    {
    }

    void finish() { m_bFinish = true; }

private:

    virtual void execute() override;
};

} // namespace webdav_ucp

void TickerThread::execute()
{
    SAL_INFO( "ucb.ucp.webdav", "TickerThread: start." );

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

    SAL_INFO( "ucb.ucp.webdav", "TickerThread: stop." );
}

NeonLockStore::NeonLockStore()
    : m_pNeonLockStore( ne_lockstore_create() )
{
    /*
     * ne_lockstore_create() never returns a NULL; neon calls abort() in case of an out-of-memory
     * situation.
     * Please see:
     * <http://www.webdav.org/neon/doc/html/refneon.html>
     * topic title "Memory handling", copied here verbatim:
     *
     * "neon does not attempt to cope gracefully with an out-of-memory situation;
     *  instead, by default, the abort function is called to immediately terminate
     *  the process. An application may register a custom function which will be
     *  called before abort in such a situation; see ne_oom_callback."
     */
}

NeonLockStore::~NeonLockStore()
{
    osl::ResettableMutexGuard aGuard(m_aMutex);
    stopTicker(aGuard);
    aGuard.reset(); // actually no threads should even try to access members now

    // release active locks, if any.
    SAL_WARN_IF( !m_aLockInfoMap.empty(), "ucb.ucp.webdav", "NeonLockStore::~NeonLockStore - Releasing active locks!" );

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

void NeonLockStore::startTicker()
{
    osl::MutexGuard aGuard( m_aMutex );

    if ( !m_pTickerThread.is() )
    {
        m_pTickerThread = new TickerThread( *this );
        m_pTickerThread->launch();
    }
}

void NeonLockStore::stopTicker(osl::ClearableMutexGuard & rGuard)
{
    rtl::Reference<TickerThread> pTickerThread;

    if (m_pTickerThread.is())
    {
        m_pTickerThread->finish(); // needs mutex
        // the TickerThread may run refreshLocks() at most once after this
        pTickerThread = m_pTickerThread;
        m_pTickerThread.clear();
    }

    rGuard.clear();

    if (pTickerThread.is())
        pTickerThread->join(); // without m_aMutex locked (to prevent deadlock)
}

void NeonLockStore::registerSession( HttpSession * pHttpSession )
{
    osl::MutexGuard aGuard( m_aMutex );

    ne_lockstore_register( m_pNeonLockStore, pHttpSession );
}

NeonLock * NeonLockStore::findByUri( OUString const & rUri )
{
    osl::MutexGuard aGuard( m_aMutex );

    ne_uri aUri;
    ne_uri_parse( OUStringToOString(
        rUri, RTL_TEXTENCODING_UTF8 ).getStr(), &aUri );
    return ne_lockstore_findbyuri( m_pNeonLockStore, &aUri );
}

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

void NeonLockStore::removeLock( NeonLock * pLock )
{
    osl::ClearableMutexGuard aGuard( m_aMutex );

    m_aLockInfoMap.erase( pLock );
    ne_lockstore_remove( m_pNeonLockStore, pLock );

    if ( m_aLockInfoMap.empty() )
        stopTicker(aGuard);
}

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
