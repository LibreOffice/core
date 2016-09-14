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

#include <tools/time.hxx>
#include <vcl/timer.hxx>
#include "saltimer.hxx"

void Timer::SetDeletionFlags()
{
    // If no AutoTimer, then stop.
    if ( !mbAuto )
    {
        mpSchedulerData->mbDelete = true;
        mbActive = false;
    }
}

bool Timer::ReadyForSchedule( bool /* bIdle */, sal_uInt64 nTimeNow ) const
{
    return (mpSchedulerData->mnUpdateTime + mnTimeout) <= nTimeNow;
}

bool Timer::IsIdle() const
{
    return false;
}

sal_uInt64 Timer::UpdateMinPeriod( sal_uInt64 nMinPeriod, sal_uInt64 nTimeNow ) const
{
    sal_uInt64 nWakeupTime = mpSchedulerData->mnUpdateTime + mnTimeout;
    if( nWakeupTime <= nTimeNow )
        return Scheduler::ImmediateTimeoutMs;
    else
    {
        sal_uInt64 nSleepTime = nWakeupTime - nTimeNow;
        return ( nSleepTime < nMinPeriod ) ? nSleepTime : nMinPeriod;
    }
}

Timer::Timer(const sal_Char *pDebugName)
    : Task( pDebugName )
    , mnTimeout( Scheduler::ImmediateTimeoutMs )
    , mbAuto( false )
{
    mePriority = TaskPriority::HIGHEST;
}

void Timer::Invoke()
{
    maInvokeHandler.Call( this );
}

void Timer::Invoke( Timer *arg )
{
    maInvokeHandler.Call( arg );
}

void Timer::Start()
{
    Task::Start();
    Task::StartTimer( mnTimeout );
}

void Timer::SetTimeout( sal_uInt64 nNewTimeout )
{
    mnTimeout = nNewTimeout;
    // If timer is active, then renew clock.
    if ( mbActive )
        StartTimer( mnTimeout );
}

AutoTimer::AutoTimer( const sal_Char *pDebugName )
    : Timer( pDebugName )
{
    mbAuto = true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
