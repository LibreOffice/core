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

#include "osx/saltimer.h"
#include "osx/salnstimer.h"
#include "osx/saldata.hxx"
#include "osx/salframe.h"
#include "osx/salinst.h"

NSTimer* AquaSalTimer::pRunningTimer = nil;

static void ImplSalStopTimer();

void ImplNSAppPostEvent( short nEventId, BOOL bAtStart, int nUserData )
{
    ReleasePoolHolder aPool;
SAL_WNODEPRECATED_DECLARATIONS_PUSH
// 'NSApplicationDefined' is deprecated: first deprecated in macOS 10.12
    NSEvent* pEvent = [NSEvent otherEventWithType: NSApplicationDefined
SAL_WNODEPRECATED_DECLARATIONS_POP
                               location: NSZeroPoint
                               modifierFlags: 0
                               timestamp: 0
                               windowNumber: 0
                               context: nil
                               subtype: nEventId
                               data1: nUserData
                               data2: 0];
    assert( pEvent );
    if ( nil == pEvent )
        return;
    if ( NO == bAtStart )
    {
        // nextEventMatchingMask has to run in the main thread!
        assert([NSThread isMainThread]);

        // Posting an event to the end of an empty queue fails,
        // so we peek the queue and post to the start, if empty.
        // Some Qt bugs even indicate nextEvent without dequeue
        // sometimes blocks, so we dequeue and re-add the event.
SAL_WNODEPRECATED_DECLARATIONS_PUSH
// 'NSAnyEventMask' is deprecated: first deprecated in macOS 10.12
        NSEvent* pPeekEvent = [NSApp nextEventMatchingMask: NSAnyEventMask
SAL_WNODEPRECATED_DECLARATIONS_POP
                               untilDate: nil
                               inMode: NSDefaultRunLoopMode
                               dequeue: YES];
        if ( nil == pPeekEvent )
            bAtStart = YES;
        else
            [NSApp postEvent: pPeekEvent atStart: YES];
    }
    [NSApp postEvent: pEvent atStart: bAtStart];
}

static void ImplSalStartTimer( sal_uLong nMS )
{
    SalData* pSalData = GetSalData();

    if( !pSalData->mpFirstInstance->IsMainThread() )
    {
        ImplNSAppPostEvent( AquaSalInstance::AppStartTimerEvent, YES, nMS );
        return;
    }

    if ( 0 == nMS )
    {
        ImplSalStopTimer();
        ImplNSAppPostEvent( AquaSalInstance::DispatchTimerEvent, NO );
    }
    else
    {
        NSTimeInterval aTI = double(nMS) / 1000.0;
        if( AquaSalTimer::pRunningTimer != nil )
        {
            if (rtl::math::approxEqual(
                    [AquaSalTimer::pRunningTimer timeInterval], aTI))
            {
                // set new fire date
                [AquaSalTimer::pRunningTimer setFireDate: [NSDate dateWithTimeIntervalSinceNow: aTI]];
            }
            else
                ImplSalStopTimer();
        }
        if( AquaSalTimer::pRunningTimer == nil )
        {
            AquaSalTimer::pRunningTimer = [[NSTimer scheduledTimerWithTimeInterval: aTI
                                                    target: [[[TimerCallbackCaller alloc] init] autorelease]
                                                    selector: @selector(timerElapsed:)
                                                    userInfo: nil
                                                    repeats: NO
                                           ] retain];
            /* #i84055# add timer to tracking run loop mode,
               so they also elapse while e.g. life resize
            */
            [[NSRunLoop currentRunLoop] addTimer: AquaSalTimer::pRunningTimer forMode: NSEventTrackingRunLoopMode];
        }
    }
}

static void ImplSalStopTimer()
{
    if( AquaSalTimer::pRunningTimer != nil )
    {
        [AquaSalTimer::pRunningTimer invalidate];
        [AquaSalTimer::pRunningTimer release];
        AquaSalTimer::pRunningTimer = nil;
    }
}

void AquaSalTimer::handleDispatchTimerEvent()
{
    ImplSVData* pSVData = ImplGetSVData();
    SolarMutexGuard aGuard;
    if( pSVData->maSchedCtx.mpSalTimer )
        pSVData->maSchedCtx.mpSalTimer->CallCallback();
}

void AquaSalTimer::handleStartTimerEvent( NSEvent* pEvent )
{
    ImplSVData* pSVData = ImplGetSVData();
    if( pSVData->maSchedCtx.mpSalTimer )
    {
        NSTimeInterval posted = [pEvent timestamp] + NSTimeInterval([pEvent data1])/1000.0;
        NSTimeInterval current = [NSDate timeIntervalSinceReferenceDate];
        if( (posted - current) <= 0.0 )
            handleDispatchTimerEvent();
        else
            ImplSalStartTimer( sal_uLong( [pEvent data1] ) );
    }
}

AquaSalTimer::AquaSalTimer( )
{
}

AquaSalTimer::~AquaSalTimer()
{
    ImplSalStopTimer();
}

void AquaSalTimer::Start( sal_uLong nMS )
{
    ImplSalStartTimer( nMS );
}

void AquaSalTimer::Stop()
{
    ImplSalStopTimer();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
