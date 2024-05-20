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


#pragma once

#include <map>
#include <mutex>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/ucb/Lock.hpp>
#include <utility>
#include <comphelper/lok.hxx>

#include "CurlSession.hxx"

namespace http_dav_ucp
{

class TickerThread;

struct LockInfo
{
    OUString m_sToken;
    css::ucb::Lock m_Lock;
    rtl::Reference<CurlSession> m_xSession;
    sal_Int32 m_nLastChanceToSendRefreshRequest;

    LockInfo()
        : m_nLastChanceToSendRefreshRequest( -1 ) {}

    LockInfo( OUString  sToken,
              css::ucb::Lock aLock,
              rtl::Reference<CurlSession> xSession,
              sal_Int32 nLastChanceToSendRefreshRequest )
        : m_sToken(std::move(sToken))
        , m_Lock(std::move(aLock))
        , m_xSession(std::move(xSession))
        , m_nLastChanceToSendRefreshRequest(nLastChanceToSendRefreshRequest)
    {}
};

typedef std::map< OUString, LockInfo > LockInfoMap;

class SerfLockStore : public comphelper::LibreOfficeKit::ThreadJoinable
{
    std::mutex         m_aMutex;
    rtl::Reference< TickerThread > m_pTickerThread;
    LockInfoMap        m_aLockInfoMap;

public:
    SerfLockStore();
    ~SerfLockStore();

    OUString const* getLockTokenForURI(OUString const& rURI, css::ucb::Lock const* pLock);

    void addLock( const OUString& rURI,
                  css::ucb::Lock const& rLock,
                  const OUString& sToken,
                  rtl::Reference<CurlSession> const & xSession,
                  // time in seconds since Jan 1 1970
                  // -1: infinite lock, no refresh
                  sal_Int32 nLastChanceToSendRefreshRequest );

    void removeLock(const OUString& rURI);

    void refreshLocks();

    // comphelper::LibreOfficeKit::ThreadJoinable
    virtual bool joinThreads() override;
    virtual void startThreads() override;

private:
    void removeLockImpl(std::unique_lock<std::mutex> & rGuard, const OUString& rURI);
    void startTicker(std::unique_lock<std::mutex> & rGuard);
    void stopTicker(std::unique_lock<std::mutex> & rGuard);
};

} // namespace http_dav_ucp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
