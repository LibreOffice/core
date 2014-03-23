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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include "aqua/saltimer.h"
#include "aqua/salnstimer.h"
#include "aqua/saldata.hxx"
#include "aqua/salframe.h"
#include "aqua/salinst.h"

// =======================================================================

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
            if( [AquaSalTimer::pRunningTimer timeInterval] == aTI )
                // set new fire date
                [AquaSalTimer::pRunningTimer setFireDate: [NSDate dateWithTimeIntervalSinceNow: aTI]];
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
            YIELD_GUARD;
            // timer already elapsed since event posted
            pSVData->mpSalTimer->CallCallback();
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


