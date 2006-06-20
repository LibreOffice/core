/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: elapsedtime.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:17:18 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "osl/time.h"
#include "osl/diagnose.h"
#include "canvas/elapsedtime.hxx"

#if defined(WIN) || defined(WNT)

// TEMP!!!
// Awaiting corresponding functionality in OSL
//
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winbase.h>
#include <mmsystem.h>
#endif

#include <algorithm>
#include <limits>

namespace canvas {
namespace tools {


#if defined(WIN) || defined(WNT)
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
        return (double)nCurrCount / nPerfCountFreq;
    }
    else
    {
        LONGLONG nCurrTime = timeGetTime();
        return (double)nCurrTime / 1000.0;
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

boost::shared_ptr<ElapsedTime> const & ElapsedTime::getTimeBase() const
{
    return m_pTimeBase;
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
#if 0
    if (bLimitToLastQueriedTime) {
        const double fCurrentTime = getElapsedTimeImpl();
        if (m_fLastQueriedTime > (fCurrentTime + fOffset)) {
            // TODO(Q3): Once the dust has settled, reduce to
            // OSL_TRACE here!
            OSL_ENSURE( false, "### adjustTimer(): clamping!" );
            fOffset = (m_fLastQueriedTime - fCurrentTime);
        }
    }
#endif
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
