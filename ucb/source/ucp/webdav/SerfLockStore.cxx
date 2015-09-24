/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"

#include <rtl/ustring.hxx>
#include <osl/time.h>
#include <osl/thread.hxx>
#include "SerfTypes.hxx"
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
    OSL_TRACE( "TickerThread::run: start." );

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
    : m_pTickerThread( 0 )
{
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
        try
        {
            (*it).second.xSession->UNLOCK( pLock );
            (*it).second.xSession->release();
        }
        catch (DAVException & )
        {}
        ++it;
    }
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

#if 0       //not currently used
// -------------------------------------------------------------------
void SerfLockStore::registerSession( SerfSession /* aSession */ )
{
    osl::MutexGuard aGuard( m_aMutex );

}
#endif

// -------------------------------------------------------------------
SerfLock * SerfLockStore::findByUri( rtl::OUString const & rUri)
{
    osl::MutexGuard aGuard( m_aMutex );

    LockInfoMap::const_iterator it( m_aLockInfoMap.begin() );
    const LockInfoMap::const_iterator end( m_aLockInfoMap.end() );

    while ( it != end )
    {
        SerfLock * pLock = (*it).first;
        if( pLock->getResourceUri().equals( rUri ) )
        {
            return pLock;
        }
        ++it;
    }

    return static_cast<SerfLock*>(0);
}

// -------------------------------------------------------------------
void SerfLockStore::addLock( SerfLock * pLock,
                             rtl::Reference< SerfSession > const & xSession,
                             sal_Int32 nLastChanceToSendRefreshRequest )
{
    osl::MutexGuard aGuard( m_aMutex );

    m_aLockInfoMap[ pLock ]
        = LockInfo( xSession, nLastChanceToSendRefreshRequest );
    //acquire this session, needed to manage the lock refresh
    xSession->acquire();

#if OSL_DEBUG_LEVEL > 0
    rtl::OUString   aOwner;
    pLock->getLock().Owner >>= aOwner;
    rtl::OUString   aToken;
    aToken = pLock->getLock().LockTokens[0];
    OSL_TRACE("SerfLockStore::addLock: new lock added aOwner '%s', token '%s'",
              rtl::OUStringToOString(aOwner, RTL_TEXTENCODING_UTF8).getStr(),
              rtl::OUStringToOString(aToken, RTL_TEXTENCODING_UTF8).getStr() );
#endif
    startTicker();
}

#if 0       //not currently used
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
#endif

// -------------------------------------------------------------------
void SerfLockStore::removeLock( SerfLock * pLock )
{
    osl::MutexGuard aGuard( m_aMutex );

    LockInfoMap::iterator it( m_aLockInfoMap.find( pLock ) );
    if(it != m_aLockInfoMap.end())
    {
        LockInfo & rInfo = (*it).second;
        rInfo.xSession->release();
        m_aLockInfoMap.erase( pLock );
        //the caller should deallocate SerfLock class after the call!
        if ( m_aLockInfoMap.size() == 0 )
            stopTicker();
    }
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
#if OSL_DEBUG_LEVEL > 0
                ucb::Lock aLock = (*it).first->getLock();
                rtl::OUString   aOwner;
                aLock.Owner >>= aOwner;
                rtl::OUString   aToken;
                aToken = aLock.LockTokens[0];
                OSL_TRACE( "SerfLockStore::refreshLocks: refresh started for lock: aOwner '%s', token '%s'",
                    rtl::OUStringToOString(aOwner, RTL_TEXTENCODING_UTF8).getStr(),
                    rtl::OUStringToOString(aToken, RTL_TEXTENCODING_UTF8).getStr() );
#endif
                sal_Int32 nlastChanceToSendRefreshRequest = -1;
                try
                {
                    rInfo.xSession->LOCK( (*it).first,
                                          /* out param */ nlastChanceToSendRefreshRequest );
                    rInfo.nLastChanceToSendRefreshRequest
                        = nlastChanceToSendRefreshRequest;
#if OSL_DEBUG_LEVEL > 0
                    OSL_TRACE( "Lock '%s' successfully refreshed." ,
                        rtl::OUStringToOString(aToken, RTL_TEXTENCODING_UTF8).getStr() );
#endif
                }
                catch ( DAVException & e )
                {
                    // refresh failed. stop auto-refresh.
                    // TODO i126305 discuss:
                    // probably not a good idea to stop the refresh?
                    // may be just ignore and go on, it's possible the net is temporary down?
                    rInfo.nLastChanceToSendRefreshRequest = -1;
#if OSL_DEBUG_LEVEL > 0
                    OSL_TRACE( "SerfLockStore::refreshLocks: Lock '%s' not refreshed! (error: DAVException.mStatusCode %d)",
                         rtl::OUStringToOString(aToken, RTL_TEXTENCODING_UTF8).getStr(), e.getStatus()  );
#endif
                }
            }
        }
        ++it;
    }
}
