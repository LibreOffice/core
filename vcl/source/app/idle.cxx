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

#include <vcl/idle.hxx>
#include "saltimer.hxx"

void Idle::SetDeletionFlags()
{
    // If no AutoIdle, then stop.
    if ( !mbAuto )
        Scheduler::SetDeletionFlags();
}

void Idle::Invoke()
{
    maIdleHdl.Call( this );
}

Idle& Idle::operator=( const Idle& rIdle )
{
    Scheduler::operator=(rIdle);
    maIdleHdl = rIdle.maIdleHdl;
    mbAuto = rIdle.mbAuto;
    return *this;
}

Idle::Idle( const sal_Char *pDebugName )
    : Scheduler( pDebugName )
    , mbAuto( false )
{
    mePriority = SchedulerPriority::DEFAULT_IDLE;
}

Idle::Idle( const Idle& rIdle ) : Scheduler(rIdle)
{
    maIdleHdl = rIdle.maIdleHdl;
    mbAuto = rIdle.mbAuto;
}

void Idle::Start()
{
    Scheduler::Start();

    sal_uInt64 nPeriod = Scheduler::ImmediateTimeoutMs;
    if (Scheduler::GetDeterministicMode())
    {
        switch (mePriority)
        {
            case SchedulerPriority::DEFAULT_IDLE:
            case SchedulerPriority::LOWEST:
                nPeriod = Scheduler::InfiniteTimeoutMs;
                break;
            default:
                break;
        }
    }

    Scheduler::ImplStartTimer(nPeriod);
}

bool Idle::ReadyForSchedule( const sal_uInt64 /* nTime */ ) const
{
    // always ready if not only looking for timers.
    return true;
}

void Idle::UpdateMinPeriod( const sal_uInt64 /* nTime */, sal_uInt64 &nMinPeriod ) const
{
    nMinPeriod = ImmediateTimeoutMs; // don't wait
}

AutoIdle::AutoIdle( const sal_Char *pDebugName )
    : Idle( pDebugName )
{
}

AutoIdle::AutoIdle( const AutoIdle& rIdle ) : Idle( rIdle )
{
    mbAuto = true;
}

AutoIdle& AutoIdle::operator=( const AutoIdle& rIdle )
{
    Idle::operator=( rIdle );
    return *this;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
