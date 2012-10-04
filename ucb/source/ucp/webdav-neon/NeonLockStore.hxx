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
#ifndef INCLUDED_NEONLOCKSTORE_HXX
#define INCLUDED_NEONLOCKSTORE_HXX

#include <map>
#include "warnings_guard_ne_locks.h"
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
    rtl::Reference< TickerThread > m_pTickerThread;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
