/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

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
