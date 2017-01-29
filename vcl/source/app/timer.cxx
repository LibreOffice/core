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
#include <vcl/scheduler.hxx>
#include "saltimer.hxx"
#include "schedulerimpl.hxx"

void Timer::SetDeletionFlags()
{
    // If no AutoTimer, then stop.
    if ( !mbAuto )
        Task::SetDeletionFlags();
}

sal_uInt64 Timer::UpdateMinPeriod( sal_uInt64, sal_uInt64 nTimeNow ) const
{
    sal_uInt64 nWakeupTime = GetSchedulerData()->mnUpdateTime + mnTimeout;
    return ( nWakeupTime <= nTimeNow )
        ? Scheduler::ImmediateTimeoutMs : nWakeupTime - nTimeNow;
}

Timer::Timer( bool bAuto, const sal_Char *pDebugName )
    : Task( pDebugName )
    , mnTimeout( Scheduler::ImmediateTimeoutMs )
    , mbAuto( bAuto )
{
    SetPriority( TaskPriority::HIGHEST );
}

Timer::Timer( const sal_Char *pDebugName )
    : Timer( false, pDebugName )
{
}

Timer::Timer( const Timer& rTimer )
    : Timer( rTimer.mbAuto, rTimer.GetDebugName() )
{
    maInvokeHandler = rTimer.maInvokeHandler;
    mnTimeout = rTimer.mnTimeout;
}

Timer::~Timer()
{
}

Timer& Timer::operator=( const Timer& rTimer )
{
    Task::operator=( rTimer );
    maInvokeHandler = rTimer.maInvokeHandler;
    mnTimeout = rTimer.mnTimeout;
    SAL_WARN_IF( mbAuto != rTimer.mbAuto, "vcl.schedule",
        "Copying Timer with different mbAuto value!" );
    return *this;
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
    if ( IsActive() )
        StartTimer( mnTimeout );
}

AutoTimer::AutoTimer( const sal_Char *pDebugName )
    : Timer( true, pDebugName )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
