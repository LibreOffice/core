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


#ifndef INCLUDED_SERFLOCKSTORE_HXX
#define INCLUDED_SERFLOCKSTORE_HXX

#include <map>
#include "osl/mutex.hxx"
#include "rtl/ref.hxx"
#include "SerfTypes.hxx"

namespace http_dav_ucp
{

class TickerThread;
class SerfSession;

struct ltptr
{
    bool operator()( const SerfLock * p1, const SerfLock * p2 ) const
    {
        return p1 < p2;
    }
};

typedef struct _LockInfo
{
    rtl::Reference< SerfSession > xSession;
    sal_Int32 nLastChanceToSendRefreshRequest;

    _LockInfo()
        : nLastChanceToSendRefreshRequest( -1 ) {}

    _LockInfo( rtl::Reference< SerfSession > const & _xSession,
              sal_Int32 _nLastChanceToSendRefreshRequest )
    : xSession( _xSession ),
      nLastChanceToSendRefreshRequest( _nLastChanceToSendRefreshRequest ) {}

} LockInfo;

typedef std::map< SerfLock *, LockInfo, ltptr > LockInfoMap;

class SerfLockStore
{
    osl::Mutex         m_aMutex;
//    ne_lock_store    * m_pSerfLockStore;
    TickerThread     * m_pTickerThread;
    LockInfoMap        m_aLockInfoMap;

public:
    SerfLockStore();
    ~SerfLockStore();

    void registerSession( HttpSession * pHttpSession );

    SerfLock * findByUri( rtl::OUString const & rUri );

    void addLock( SerfLock * pLock,
                  rtl::Reference< SerfSession > const & xSession,
                  // time in seconds since Jan 1 1970
                  // -1: infinite lock, no refresh
                  sal_Int32 nLastChanceToSendRefreshRequest );

    void updateLock( SerfLock * pLock,
                     sal_Int32 nLastChanceToSendRefreshRequest );

    void removeLock( SerfLock * pLock );

    void refreshLocks();

private:
    void startTicker();
    void stopTicker();
};

} // namespace http_dav_ucp

#endif // INCLUDED_SERFLOCKSTORE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
