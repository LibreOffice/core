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

#include <sal/config.h>

#include <comphelper/solarmutex.hxx>

#include <assert.h>
#include <cstdlib>

namespace comphelper {

SolarMutex::SolarMutex() {}

SolarMutex::~SolarMutex() {}

namespace {
    static SolarMutex* pSolarMutex = nullptr;
}

void SolarMutex::setSolarMutex( SolarMutex *pMutex )
{
    assert((pMutex && !pSolarMutex) || !pMutex);
    pSolarMutex = pMutex;
}

SolarMutex *SolarMutex::get()
{
    return pSolarMutex;
}

GenericSolarMutex::GenericSolarMutex()
    : m_nCount( 0 )
    , m_nThreadId( 0 )
    , m_aBeforeReleaseHandler( nullptr )
{
    setSolarMutex( this );
}

GenericSolarMutex::~GenericSolarMutex()
{
    setSolarMutex( nullptr );
}

void GenericSolarMutex::doAcquire( const sal_uInt32 nLockCount )
{
    for ( sal_uInt32 n = nLockCount; n ; --n )
        m_aMutex.acquire();
    m_nThreadId = osl::Thread::getCurrentIdentifier();
    m_nCount += nLockCount;
}

sal_uInt32 GenericSolarMutex::doRelease( bool bUnlockAll )
{
    if ( m_nCount == 0 )
        std::abort();
    if ( m_nThreadId != osl::Thread::getCurrentIdentifier() )
        std::abort();

    const sal_uInt32 nCount = bUnlockAll ? m_nCount : 1;
    m_nCount -= nCount;

    if ( 0 == m_nCount )
    {
        if ( m_aBeforeReleaseHandler )
            m_aBeforeReleaseHandler();
        m_nThreadId = 0;
    }

    for ( sal_uInt32 n = nCount ; n ; --n )
        m_aMutex.release();

    return nCount;
}

bool GenericSolarMutex::IsCurrentThread() const
{
    return m_nThreadId == osl::Thread::getCurrentIdentifier();
}

bool GenericSolarMutex::tryToAcquire()
{
    if ( m_aMutex.tryToAcquire() )
    {
        m_nThreadId = osl::Thread::getCurrentIdentifier();
        m_nCount++;
        return true;
    }
    else
        return false;
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
