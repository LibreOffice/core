/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "refreshtimer.hxx"
#include "refreshtimerprotector.hxx"

void ScRefreshTimerControl::SetAllowRefresh( bool b )
{
    if ( b && nBlockRefresh )
        --nBlockRefresh;
    else if ( !b && nBlockRefresh < (sal_uInt16)(~0) )
        ++nBlockRefresh;
}

ScRefreshTimerProtector::ScRefreshTimerProtector( ScRefreshTimerControl * const * pp )
        :
        ppControl( pp )
{
    if ( ppControl && *ppControl )
    {
        (*ppControl)->SetAllowRefresh( false );
        
        ::osl::MutexGuard aGuard( (*ppControl)->GetMutex() );
    }
}

ScRefreshTimerProtector::~ScRefreshTimerProtector()
{
    if ( ppControl && *ppControl )
        (*ppControl)->SetAllowRefresh( true );
}

ScRefreshTimer::ScRefreshTimer() : ppControl(0)
{
    SetTimeout( 0 );
}

ScRefreshTimer::ScRefreshTimer( sal_uLong nSeconds ) : ppControl(0)
{
    SetTimeout( nSeconds * 1000 );
    Start();
}

ScRefreshTimer::ScRefreshTimer( const ScRefreshTimer& r ) : AutoTimer( r ), ppControl(0)
{
}

ScRefreshTimer::~ScRefreshTimer()
{
    if ( IsActive() )
        Stop();
}

ScRefreshTimer& ScRefreshTimer::operator=( const ScRefreshTimer& r )
{
    SetRefreshControl(0);
    AutoTimer::operator=( r );
    return *this;
}

bool ScRefreshTimer::operator==( const ScRefreshTimer& r ) const
{
    return GetTimeout() == r.GetTimeout();
}

bool ScRefreshTimer::operator!=( const ScRefreshTimer& r ) const
{
    return !ScRefreshTimer::operator==( r );
}

void ScRefreshTimer::StartRefreshTimer()
{
    Start();
}

void ScRefreshTimer::SetRefreshControl( ScRefreshTimerControl * const * pp )
{
    ppControl = pp;
}

void ScRefreshTimer::SetRefreshHandler( const Link& rLink )
{
    SetTimeoutHdl( rLink );
}

sal_uLong ScRefreshTimer::GetRefreshDelay() const
{
    return GetTimeout() / 1000;
}

void ScRefreshTimer::StopRefreshTimer()
{
    Stop();
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
        
        ::osl::MutexGuard aGuard( (*ppControl)->GetMutex() );
        maTimeoutHdl.Call( this );
        
        
        if ( IsActive() )
            Start();
    }
}

void ScRefreshTimer::Start()
{
    if ( GetTimeout() )
        AutoTimer::Start();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
