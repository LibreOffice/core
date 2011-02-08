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
#include "precompiled_sc.hxx"



#include "refreshtimer.hxx"


ScRefreshTimerProtector::ScRefreshTimerProtector( ScRefreshTimerControl * const * pp )
        :
        ppControl( pp )
{
    if ( ppControl && *ppControl )
    {
        (*ppControl)->SetAllowRefresh( sal_False );
        // wait for any running refresh in another thread to finnish
        ::vos::OGuard aGuard( (*ppControl)->GetMutex() );
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
        ::vos::OGuard aGuard( (*ppControl)->GetMutex() );
        maTimeoutHdl.Call( this );
        // restart from now on, don't execute immediately again if timed out
        // a second time during refresh
        if ( IsActive() )
            Start();
    }
}

