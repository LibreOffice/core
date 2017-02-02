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
#include <vcl/scheduler.hxx>
#include "saltimer.hxx"

Idle::Idle( bool bAuto, const sal_Char *pDebugName )
    : Timer( bAuto, pDebugName )
{
    SetPriority( TaskPriority::DEFAULT_IDLE );
}

Idle::Idle( const sal_Char *pDebugName )
    : Idle( false, pDebugName )
{
}

void Idle::Start()
{
    Task::Start();

    sal_uInt64 nPeriod = Scheduler::ImmediateTimeoutMs;
    if (Scheduler::GetDeterministicMode())
    {
        switch ( GetPriority() )
        {
            case TaskPriority::LOW:
            case TaskPriority::LOWER:
            case TaskPriority::LOWEST:
                nPeriod = Scheduler::InfiniteTimeoutMs;
                break;
            default:
                break;
        }
    }

    Task::StartTimer(nPeriod);
}

bool Idle::ReadyForSchedule( bool bIdle, sal_uInt64 /* nTimeNow */ ) const
{
    // always ready if not only looking for timers.
    return bIdle;
}

bool Idle::IsIdle() const
{
    return true;
}

sal_uInt64 Idle::UpdateMinPeriod( sal_uInt64 /* nMinPeriod */, sal_uInt64 /* nTimeNow */ ) const
{
    assert(false); // idles currently don't hit this.
    return Scheduler::ImmediateTimeoutMs;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
