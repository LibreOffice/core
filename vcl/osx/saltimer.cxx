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

#include <rtl/math.hxx>
#include <tools/time.hxx>

#include "osx/saltimer.h"
#include "osx/salnstimer.h"
#include "osx/saldata.hxx"
#include "osx/salframe.h"
#include "osx/salinst.h"


void ImplNSAppPostEvent( short nEventId, BOOL bAtStart, int nUserData )
{
    ReleasePoolHolder aPool;
SAL_WNODEPRECATED_DECLARATIONS_PUSH
// 'NSApplicationDefined' is deprecated: first deprecated in macOS 10.12
    NSEvent* pEvent = [NSEvent otherEventWithType: NSApplicationDefined
SAL_WNODEPRECATED_DECLARATIONS_POP
                               location: NSZeroPoint
                               modifierFlags: 0
                               timestamp: [[NSProcessInfo processInfo] systemUptime]
                               windowNumber: 0
                               context: nil
                               subtype: nEventId
                               data1: nUserData
                               data2: 0];
    if ( nil == pEvent )
        std::abort();
    [NSApp postEvent: pEvent atStart: bAtStart];
}

void AquaSalTimer::queueDispatchTimerEvent( bool bAtStart )
{
    Stop();
    ImplNSAppPostEvent( AquaSalInstance::DispatchTimerEvent,
                        bAtStart, GetNextEventVersion() );
}

void AquaSalTimer::Start( sal_uLong nMS )
{
    SalData* pSalData = GetSalData();

    if( !pSalData->mpInstance->IsMainThread() )
    {
        ImplNSAppPostEvent( AquaSalInstance::AppStartTimerEvent, YES, nMS );
        return;
    }

    if ( 0 == nMS && !pSalData->mpInstance->mbIsLiveResize )
        queueDispatchTimerEvent( NO );
    else
    {
        NSTimeInterval aTI = double(nMS) / 1000.0;
        if( m_pRunningTimer != nil )
        {
            if ([m_pRunningTimer isValid] && rtl::math::approxEqual(
                    [m_pRunningTimer timeInterval], aTI))
            {
                // set new fire date
                [m_pRunningTimer setFireDate: [NSDate dateWithTimeIntervalSinceNow: aTI]];
            }
            else
                Stop();
        }
        else
            Stop();
        if( m_pRunningTimer == nil )
        {
            m_pRunningTimer = [[NSTimer scheduledTimerWithTimeInterval: aTI
                                                    target: [[[TimerCallbackCaller alloc] init] autorelease]
                                                    selector: @selector(timerElapsed:)
                                                    userInfo: nil
                                                    repeats: NO
                                           ] retain];
            /* #i84055# add timer to tracking run loop mode,
               so they also elapse while e.g. life resize
            */
            [[NSRunLoop currentRunLoop] addTimer: m_pRunningTimer forMode: NSEventTrackingRunLoopMode];
        }
    }
}

void AquaSalTimer::Stop()
{
    assert( GetSalData()->mpInstance->IsMainThread() );

    if( m_pRunningTimer != nil )
    {
        [m_pRunningTimer invalidate];
        [m_pRunningTimer release];
        m_pRunningTimer = nil;
    }
    InvalidateEvent();
}

void AquaSalTimer::callTimerCallback()
{
    ImplSVData* pSVData = ImplGetSVData();
    SolarMutexGuard aGuard;
    if( pSVData->maSchedCtx.mpSalTimer )
        pSVData->maSchedCtx.mpSalTimer->CallCallback();
}

void AquaSalTimer::handleTimerElapsed()
{
    if ( GetSalData()->mpInstance->mbIsLiveResize )
    {
        // Stop the timer, as it is just invalidated after the firing function
        Stop();
        callTimerCallback();
    }
    else
        queueDispatchTimerEvent( YES );
}

void AquaSalTimer::handleDispatchTimerEvent( NSEvent *pEvent )
{
    if ( IsValidEventVersion( [pEvent data1] ) )
        callTimerCallback();
}

void AquaSalTimer::handleStartTimerEvent( NSEvent* pEvent )
{
    ImplSVData* pSVData = ImplGetSVData();
    if( pSVData->maSchedCtx.mpSalTimer )
    {
        NSTimeInterval posted = [pEvent timestamp] + NSTimeInterval([pEvent data1])/1000.0;
        NSTimeInterval current = [NSDate timeIntervalSinceReferenceDate];
        sal_uLong nTimeoutMS = 0;
        if( (posted - current) > 0.0 )
            nTimeoutMS = ceil( (posted - current) * 1000 );
        Start( nTimeoutMS );
    }
}

bool AquaSalTimer::IsTimerElapsed() const
{
    assert( !(ExistsValidEvent() && m_pRunningTimer) );
    if ( ExistsValidEvent() )
        return true;
    if ( !m_pRunningTimer )
        return false;
    NSDate* pDt = [m_pRunningTimer fireDate];
    return pDt && ([pDt timeIntervalSinceNow] < 0);
}

AquaSalTimer::AquaSalTimer( )
    : m_pRunningTimer( nil )
{
}

AquaSalTimer::~AquaSalTimer()
{
    Stop();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
