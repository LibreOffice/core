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

#include <salusereventlist.hxx>
#include <salwtype.hxx>

#include <algorithm>
#include <cstdlib>
#include <exception>
#include <typeinfo>

#include <com/sun/star/uno/Exception.hpp>
#include <tools/debug.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <sal/log.hxx>
#include <sal/types.h>
#include <svdata.hxx>

SalUserEventList::SalUserEventList()
    : m_bAllUserEventProcessedSignaled( true )
    , m_aProcessingThread(0)
{
}

SalUserEventList::~SalUserEventList() COVERITY_NOEXCEPT_FALSE
{
}

void SalUserEventList::insertFrame( SalFrame* pFrame )
{
    auto aPair = m_aFrames.insert( pFrame );
    assert( aPair.second ); (void) aPair;
}

void SalUserEventList::eraseFrame( SalFrame* pFrame )
{
    auto it = m_aFrames.find( pFrame );
    assert( it != m_aFrames.end() );
    if ( it != m_aFrames.end() )
        m_aFrames.erase( it );
}

bool SalUserEventList::DispatchUserEvents( bool bHandleAllCurrentEvents )
{
    bool bWasEvent = false;
    oslThreadIdentifier aCurId = osl::Thread::getCurrentIdentifier();

    DBG_TESTSOLARMUTEX();
    std::unique_lock aResettableListGuard(m_aUserEventsMutex);

    if (!m_aUserEvents.empty())
    {
        if (bHandleAllCurrentEvents)
        {
            if (m_aProcessingUserEvents.empty())
                m_aProcessingUserEvents.swap(m_aUserEvents);
            else
                m_aProcessingUserEvents.splice(m_aProcessingUserEvents.end(), m_aUserEvents);
        }
        else if (m_aProcessingUserEvents.empty())
        {
            m_aProcessingUserEvents.push_back( m_aUserEvents.front() );
            m_aUserEvents.pop_front();
        }
    }

    if (HasUserEvents_NoLock())
    {
        bWasEvent = true;
        m_aProcessingThread = aCurId;

        SalUserEvent aEvent( nullptr, nullptr, SalEvent::NONE );
        do {
            if (m_aProcessingUserEvents.empty() || aCurId != m_aProcessingThread)
                break;
            aEvent = m_aProcessingUserEvents.front();
            m_aProcessingUserEvents.pop_front();

            // remember to reset the guard before break or continue the loop
            aResettableListGuard.unlock();

            if ( !isFrameAlive( aEvent.m_pFrame ) )
            {
                if ( aEvent.m_nEvent == SalEvent::UserEvent )
                    delete static_cast< ImplSVEvent* >( aEvent.m_pData );
                aResettableListGuard.lock();
                continue;
            }

            /*
            * Current policy is that scheduler tasks aren't allowed to throw an exception.
            * Because otherwise the exception is caught somewhere totally unrelated.
            * TODO Ideally we could capture a proper backtrace and feed this into breakpad,
            *   which is do-able, but requires writing some assembly.
            * See also Scheduler::CallbackTaskScheduling
            */
#ifdef IOS
            ProcessEvent( aEvent );
#else
            // the noexcept here means that (a) we abort and (b) debuggers will
            // likely trigger at the throw site instead of here, making the debugging
            // experience better when something goes wrong.
            auto process = [&aEvent, this] () noexcept { ProcessEvent(aEvent); };
            process();
#endif
            aResettableListGuard.lock();
            if (!bHandleAllCurrentEvents)
                break;
        }
        while( true );
    }

    if ( !m_bAllUserEventProcessedSignaled && !HasUserEvents_NoLock() )
    {
        m_bAllUserEventProcessedSignaled = true;
        TriggerAllUserEventsProcessed();
    }

    return bWasEvent;
}

void SalUserEventList::RemoveEvent( SalFrame* pFrame, void* pData, SalEvent nEvent )
{
    SalUserEvent aEvent( pFrame, pData, nEvent );

    std::unique_lock aGuard( m_aUserEventsMutex );
    auto it = std::find( m_aUserEvents.begin(), m_aUserEvents.end(), aEvent );
    if ( it != m_aUserEvents.end() )
    {
        m_aUserEvents.erase( it );
    }
    else
    {
        it = std::find( m_aProcessingUserEvents.begin(), m_aProcessingUserEvents.end(), aEvent );
        if ( it != m_aProcessingUserEvents.end() )
        {
            m_aProcessingUserEvents.erase( it );
        }
    }

    if ( !m_bAllUserEventProcessedSignaled && !HasUserEvents_NoLock() )
    {
        m_bAllUserEventProcessedSignaled = true;
        TriggerAllUserEventsProcessed();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
