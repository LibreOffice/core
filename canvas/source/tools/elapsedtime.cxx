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


#include "osl/time.h"
#include "osl/diagnose.h"
#include "canvas/elapsedtime.hxx"

#if defined(WNT)

#if defined _MSC_VER
#pragma warning(push,1)
#endif

// TEMP!!!
// Awaiting corresponding functionality in OSL
//
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winbase.h>
#include <mmsystem.h>
#endif

#if defined _MSC_VER
#pragma warning(pop)
#endif

#include <algorithm>
#include <limits>

namespace canvas {
namespace tools {


#if defined(WNT)
// TODO(Q2): is 0 okay for the failure case here?
double ElapsedTime::getSystemTime()
{
    // TEMP!!!
    // Awaiting corresponding functionality in OSL
    //

    // is there a performance counter available?
    static bool bTimeSetupDone( false );
    static bool bPerfTimerAvailable( false );
    static LONGLONG nPerfCountFreq;

    // TODO(F1): This _might_ cause problems, as it prevents correct
    // time handling for very long lifetimes of this class's
    // surrounding component in memory. When the difference between
    // current sys time and nInitialCount exceeds IEEE double's
    // mantissa, time will start to run jerky.
    static LONGLONG nInitialCount;

    if( !bTimeSetupDone )
    {
        if( QueryPerformanceFrequency(
                reinterpret_cast<LARGE_INTEGER *>(&nPerfCountFreq) ) )
        {
            // read initial time:
            QueryPerformanceCounter(
                reinterpret_cast<LARGE_INTEGER *>(&nInitialCount) );
            bPerfTimerAvailable = true;
        }
        bTimeSetupDone = true;
    }

    if( bPerfTimerAvailable )
    {
        LONGLONG nCurrCount;
        QueryPerformanceCounter(
            reinterpret_cast<LARGE_INTEGER *>(&nCurrCount) );
        nCurrCount -= nInitialCount;
        return double(nCurrCount) / nPerfCountFreq;
    }
    else
    {
        LONGLONG nCurrTime = timeGetTime();
        return double(nCurrTime) / 1000.0;
    }
}

#else // ! WNT

// TODO(Q2): is 0 okay for the failure case here?
double ElapsedTime::getSystemTime()
{
    TimeValue aTimeVal;
    if( osl_getSystemTime( &aTimeVal ) )
        return ((aTimeVal.Nanosec * 10e-10) + aTimeVal.Seconds);
    else
        return 0.0;
}

#endif

ElapsedTime::ElapsedTime()
    : m_pTimeBase(),
      m_fLastQueriedTime( 0.0 ),
      m_fStartTime( getSystemTime() ),
      m_fFrozenTime( 0.0 ),
      m_bInPauseMode( false ),
      m_bInHoldMode( false )
{
}

ElapsedTime::ElapsedTime(
    boost::shared_ptr<ElapsedTime> const & pTimeBase )
    : m_pTimeBase( pTimeBase ),
      m_fLastQueriedTime( 0.0 ),
      m_fStartTime( getCurrentTime() ),
      m_fFrozenTime( 0.0 ),
      m_bInPauseMode( false ),
      m_bInHoldMode( false )
{
}

void ElapsedTime::reset()
{
    m_fLastQueriedTime = 0.0;
    m_fStartTime = getCurrentTime();
    m_fFrozenTime = 0.0;
    m_bInPauseMode = false;
    m_bInHoldMode = false;
}

void ElapsedTime::adjustTimer( double fOffset, bool /*bLimitToLastQueriedTime*/ )
{
    // to make getElapsedTime() become _larger_, have to reduce
    // m_fStartTime.
    m_fStartTime -= fOffset;

    // also adjust frozen time, this method must _always_ affect the
    // value returned by getElapsedTime()!
    if (m_bInHoldMode || m_bInPauseMode)
        m_fFrozenTime += fOffset;
}

double ElapsedTime::getCurrentTime() const
{
    return m_pTimeBase.get() == 0
        ? getSystemTime() : m_pTimeBase->getElapsedTimeImpl();
}

double ElapsedTime::getElapsedTime() const
{
    m_fLastQueriedTime = getElapsedTimeImpl();
    return m_fLastQueriedTime;
}

double ElapsedTime::getElapsedTimeImpl() const
{
    if (m_bInHoldMode || m_bInPauseMode)
        return m_fFrozenTime;

    return getCurrentTime() - m_fStartTime;
}

void ElapsedTime::pauseTimer()
{
    m_fFrozenTime = getElapsedTimeImpl();
    m_bInPauseMode = true;
}

void ElapsedTime::continueTimer()
{
    m_bInPauseMode = false;

    // stop pausing, time runs again. Note that
    // getElapsedTimeImpl() honors hold mode, i.e. a
    // continueTimer() in hold mode will preserve the latter
    const double fPauseDuration( getElapsedTimeImpl() - m_fFrozenTime );

    // adjust start time, such that subsequent getElapsedTime() calls
    // will virtually start from m_fFrozenTime.
    m_fStartTime += fPauseDuration;
}

void ElapsedTime::holdTimer()
{
    // when called during hold mode (e.g. more than once per time
    // object), the original hold time will be maintained.
    m_fFrozenTime = getElapsedTimeImpl();
    m_bInHoldMode = true;
}

void ElapsedTime::releaseTimer()
{
    m_bInHoldMode = false;
}

} // namespace tools
} // namespace canvas

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
