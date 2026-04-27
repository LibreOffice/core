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
#include <sal/log.hxx>
#include <osl/diagnose.h>

#include <osl/thread.hxx>
#include <osx/salmutex.hxx>

#include <premac.h>
#include <Foundation/Foundation.h>
#include <postmac.h>

AquaSalYieldMutex::AquaSalYieldMutex()
    : m_aCodeBlock( nullptr )
    , maMainThread( osl::Thread::getCurrentIdentifier() )
    , mbNoYieldLock( false )
{
}

AquaSalYieldMutex::~AquaSalYieldMutex()
{
}

void AquaSalYieldMutex::doAcquire( sal_uInt32 nLockCount )
{
    if ( IsMainThread() )
    {
        if ( mbNoYieldLock )
            return;
        do {
            RuninmainBlock block = nullptr;
            {
                std::unique_lock<std::mutex> g(m_runInMainMutex);
                if (m_aMutex.tryToAcquire()) {
                    assert(m_aCodeBlock == nullptr);
                    m_wakeUpMain = false;
                    break;
                }
                // wait for doRelease() or RUNINMAIN_* to set the condition
                m_aInMainCondition.wait(g, [this]() { return m_wakeUpMain; });
                m_wakeUpMain = false;
                std::swap(block, m_aCodeBlock);
            }
            if ( block )
            {
                assert( !mbNoYieldLock );
                mbNoYieldLock = true;
                block();
                mbNoYieldLock = false;
                Block_release( block );
                std::scoped_lock<std::mutex> g(m_runInMainMutex);
                assert(!m_resultReady);
                m_resultReady = true;
                m_aResultCondition.notify_all();
            }
        }
        while ( true );
    }
    else
        m_aMutex.acquire();
    ++m_nCount;
    --nLockCount;

    comphelper::SolarMutex::doAcquire( nLockCount );
}

sal_uInt32 AquaSalYieldMutex::doRelease( const bool bUnlockAll )
{
    if ( mbNoYieldLock && IsMainThread() )
        return 1;
    sal_uInt32 nCount;
    {
        std::scoped_lock<std::mutex> g(m_runInMainMutex);
        // read m_nCount before doRelease
        bool const isReleased(bUnlockAll || m_nCount == 1);
        nCount = comphelper::SolarMutex::doRelease( bUnlockAll );
        if (isReleased && !IsMainThread()) {
            m_wakeUpMain = true;
            m_aInMainCondition.notify_all();
        }
    }
    return nCount;
}

bool AquaSalYieldMutex::IsCurrentThread() const
{
    if ( !mbNoYieldLock )
        return comphelper::SolarMutex::IsCurrentThread();
    else
        return IsMainThread();
}

bool AquaSalYieldMutex::IsMainThread() const
{
    return osl::Thread::getCurrentIdentifier() == maMainThread;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
