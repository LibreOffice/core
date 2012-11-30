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

#include "refreshtimer.hxx"


ScRefreshTimerProtector::ScRefreshTimerProtector( ScRefreshTimerControl * const * pp )
        :
        ppControl( pp )
{
    if ( ppControl && *ppControl )
    {
        (*ppControl)->SetAllowRefresh( false );
        // wait for any running refresh in another thread to finnish
        ::osl::MutexGuard aGuard( (*ppControl)->GetMutex() );
    }
}


ScRefreshTimer::~ScRefreshTimer()
{
    if ( IsActive() )
        Stop();
    RemoveFromControl();
}


void ScRefreshTimer::SetRefreshDelay( sal_uLong nSeconds )
{
    sal_Bool bActive = IsActive();
    if ( bActive && !nSeconds )
        Stop();
    SetTimeout( nSeconds * 1000 );
    if ( !bActive && nSeconds )
        Start();
}


void ScRefreshTimer::Timeout()
{
    if ( ppControl && *ppControl && (*ppControl)->IsRefreshAllowed() )
    {
        // now we COULD make the call in another thread ...
        ::osl::MutexGuard aGuard( (*ppControl)->GetMutex() );
        maTimeoutHdl.Call( this );
        // restart from now on, don't execute immediately again if timed out
        // a second time during refresh
        if ( IsActive() )
            Start();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
