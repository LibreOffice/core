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


#ifndef INCLUDED_NEONLOCKSTORE_HXX
#define INCLUDED_NEONLOCKSTORE_HXX

#include <map>
#include "ne_locks.h"
#include "osl/mutex.hxx"
#include "rtl/ref.hxx"
#include "NeonTypes.hxx"

namespace webdav_ucp
{

class TickerThread;
class NeonSession;

struct ltptr
{
    bool operator()( const NeonLock * p1, const NeonLock * p2 ) const
    {
        return p1 < p2;
    }
};

typedef struct _LockInfo
{
    rtl::Reference< NeonSession > xSession;
    sal_Int32 nLastChanceToSendRefreshRequest;

    _LockInfo()
        : nLastChanceToSendRefreshRequest( -1 ) {}

    _LockInfo( rtl::Reference< NeonSession > const & _xSession,
              sal_Int32 _nLastChanceToSendRefreshRequest )
    : xSession( _xSession ),
      nLastChanceToSendRefreshRequest( _nLastChanceToSendRefreshRequest ) {}

} LockInfo;

typedef std::map< NeonLock *, LockInfo, ltptr > LockInfoMap;

class NeonLockStore
{
    osl::Mutex         m_aMutex;
    ne_lock_store    * m_pNeonLockStore;
    TickerThread     * m_pTickerThread;
    LockInfoMap        m_aLockInfoMap;

public:
    NeonLockStore();
    ~NeonLockStore();

    void registerSession( HttpSession * pHttpSession );

    NeonLock * findByUri( rtl::OUString const & rUri );

    void addLock( NeonLock * pLock,
                  rtl::Reference< NeonSession > const & xSession,
                  // time in seconds since Jan 1 1970
                  // -1: infinite lock, no refresh
                  sal_Int32 nLastChanceToSendRefreshRequest );

    void updateLock( NeonLock * pLock,
                     sal_Int32 nLastChanceToSendRefreshRequest );

    void removeLock( NeonLock * pLock );

    void refreshLocks();

private:
    void startTicker();
    void stopTicker();
};

} // namespace webdav_ucp

#endif // INCLUDED_NEONLOCKSTORE_HXX
