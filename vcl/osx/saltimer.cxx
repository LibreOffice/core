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
bool AquaSalTimer::bDispatchTimer = false;

void ImplSalStartTimer( sal_uLong nMS )
{
    SalData* pSalData = GetSalData();
    if( pSalData->mpFirstInstance->isNSAppThread() )
    {
        AquaSalTimer::bDispatchTimer = true;
        NSTimeInterval aTI = double(nMS)/1000.0;
        if( AquaSalTimer::pRunningTimer != nil )
        {
            if (rtl::math::approxEqual(
                    [AquaSalTimer::pRunningTimer timeInterval], aTI))
            {
                // set new fire date
                [AquaSalTimer::pRunningTimer setFireDate: [NSDate dateWithTimeIntervalSinceNow: aTI]];
            }
            else
            {
                [AquaSalTimer::pRunningTimer invalidate];
                AquaSalTimer::pRunningTimer = nil;
            }
        }
        if( AquaSalTimer::pRunningTimer == nil )
        {
            AquaSalTimer::pRunningTimer = [NSTimer scheduledTimerWithTimeInterval: aTI
                                                   target: [[[TimerCallbackCaller alloc] init] autorelease]
                                                   selector: @selector(timerElapsed:)
                                                   userInfo: nil
                                                   repeats: YES];
            /* #i84055# add timer to tracking run loop mode,
               so they also elapse while e.g. life resize
            */
            [[NSRunLoop currentRunLoop] addTimer: AquaSalTimer::pRunningTimer forMode: NSEventTrackingRunLoopMode];
        }
    }
    else
    {
        SalData::ensureThreadAutoreleasePool();
        // post an event so we can get into the main thread
        NSEvent* pEvent = [NSEvent otherEventWithType: NSApplicationDefined
                                   location: NSZeroPoint
                                   modifierFlags: 0
                                   timestamp: [NSDate timeIntervalSinceReferenceDate]
                                   windowNumber: 0
                                   context: nil
                                   subtype: AquaSalInstance::AppStartTimerEvent
                                   data1: (int)nMS
                                   data2: 0 ];
        if( pEvent )
            [NSApp postEvent: pEvent atStart: YES];
    }
}

void ImplSalStopTimer()
{
    AquaSalTimer::bDispatchTimer = false;
}

void AquaSalTimer::handleStartTimerEvent( NSEvent* pEvent )
{
    ImplSVData* pSVData = ImplGetSVData();
    if( pSVData->mpSalTimer )
    {
        NSTimeInterval posted = [pEvent timestamp] + NSTimeInterval([pEvent data1])/1000.0;
        NSTimeInterval current = [NSDate timeIntervalSinceReferenceDate];
        if( (posted - current) <= 0.0 )
        {
            SolarMutexGuard aGuard;
            if( pSVData->mpSalTimer )
            {
                // timer already elapsed since event posted
                pSVData->mpSalTimer->CallCallback();
            }
        }
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
