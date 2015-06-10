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
#include <vcl/timer.hxx>
#include "svdata.hxx"

void Idle::Invoke()
{
    maIdleHdl.Call( this );
}

Idle& Idle::operator=( const Idle& rIdle )
{
    Scheduler::operator=(rIdle);
    maIdleHdl = rIdle.maIdleHdl;
    return *this;
}

Idle::Idle( const sal_Char *pDebugName ) : Scheduler( pDebugName )
{
}

Idle::Idle( const Idle& rIdle ) : Scheduler(rIdle)
{
    maIdleHdl = rIdle.maIdleHdl;
}

void Idle::Start()
{
    Scheduler::Start();
    ImplSVData* pSVData = ImplGetSVData();
    Timer::ImplStartTimer( pSVData, 0 );
}

bool Idle::ReadyForSchedule( bool bTimer )
{
    // tdf#91727 - We need to re-work this to allow only UI idle handlers
    //             and not timeouts to be processed in some limited scenarios
    (void)bTimer;
    return true; // !bTimer
}

sal_uInt64 Idle::UpdateMinPeriod( sal_uInt64 /* nMinPeriod */, sal_uInt64 /* nTime */ )
{
    return 1;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
