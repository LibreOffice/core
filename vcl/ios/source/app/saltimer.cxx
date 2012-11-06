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


#include "ios/saltimer.h"
#include "ios/salnstimer.h"
#include "ios/saldata.hxx"
#include "ios/salframe.h"
#include "ios/salinst.h"

// =======================================================================

NSTimer* IosSalTimer::pRunningTimer = nil;
bool IosSalTimer::bDispatchTimer = false;


void ImplSalStartTimer( sal_uLong nMS )
{
    SalData* pSalData = GetSalData();
    if( pSalData->mpFirstInstance->isUIAppThread() )
    {
        IosSalTimer::bDispatchTimer = true;
        NSTimeInterval aTI = double(nMS)/1000.0;
        if( IosSalTimer::pRunningTimer != nil )
        {
            if( [IosSalTimer::pRunningTimer timeInterval] == aTI )
                // set new fire date
                [IosSalTimer::pRunningTimer setFireDate: [NSDate dateWithTimeIntervalSinceNow: aTI]];
            else
            {
                [IosSalTimer::pRunningTimer invalidate];
                IosSalTimer::pRunningTimer = nil;
            }
        }
        if( IosSalTimer::pRunningTimer == nil )
        {
            IosSalTimer::pRunningTimer = [NSTimer scheduledTimerWithTimeInterval: aTI
                                                   target: [[[TimerCallbackCaller alloc] init] autorelease]
                                                   selector: @selector(timerElapsed:)
                                                   userInfo: nil
                                                   repeats: YES];
        }
    }
    else
    {
        SalData::ensureThreadAutoreleasePool();
        // post an event so we can get into the main thread
        // ???
    }
}

void ImplSalStopTimer()
{
    IosSalTimer::bDispatchTimer = false;
}

IosSalTimer::IosSalTimer( )
{
}

IosSalTimer::~IosSalTimer()
{
    ImplSalStopTimer();
}

void IosSalTimer::Start( sal_uLong nMS )
{
    ImplSalStartTimer( nMS );
}

void IosSalTimer::Stop()
{
    ImplSalStopTimer();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
