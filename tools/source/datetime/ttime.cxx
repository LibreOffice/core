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

#include <sal/config.h>

#include <algorithm>

#if defined(_WIN32)
#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <mmsystem.h>
#elif defined UNX
#include <sys/time.h>
#endif

#include <time.h>
#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#endif

#include <rtl/math.hxx>
#include <tools/time.hxx>
#include <com/sun/star/util/DateTime.hpp>

#include <systemdatetime.hxx>

#if defined(__sun) && defined(__GNUC__)
extern long altzone;
#endif

namespace {

    const sal_Int64 nanoSecInSec = 1000000000;
    const sal_Int16 secInMin     = 60;
    const sal_Int16 minInHour    = 60;

    sal_Int64 TimeToNanoSec( const tools::Time& rTime )
    {
        short  nSign   = (rTime.GetTime() >= 0) ? +1 : -1;
        sal_Int32   nHour    = rTime.GetHour();
        sal_Int32   nMin     = rTime.GetMin();
        sal_Int32   nSec     = rTime.GetSec();
        sal_Int32   nNanoSec = rTime.GetNanoSec();

        sal_Int64 nRet = nNanoSec;
        nRet     +=            nSec * nanoSecInSec;
        nRet     +=       nMin * secInMin * nanoSecInSec;
        nRet     += nHour * minInHour * secInMin * nanoSecInSec;

        return (nRet * nSign);
    }

    tools::Time NanoSecToTime( sal_Int64 nNanoSec )
    {
        short nSign;
        if ( nNanoSec < 0 )
        {
            nNanoSec *= -1;
            nSign = -1;
        }
        else
            nSign = 1;

        tools::Time aTime( 0, 0, 0, nNanoSec );
        aTime.SetTime( aTime.GetTime() * nSign );
        return aTime;
    }

} // anonymous namespace

namespace tools {

Time::Time( TimeInitSystem )
{
    if ( !GetSystemDateTime( nullptr, &nTime ) )
        nTime = 0;
}

Time::Time( const tools::Time& rTime )
{
    nTime = rTime.nTime;
}

Time::Time( sal_uInt32 nHour, sal_uInt32 nMin, sal_uInt32 nSec, sal_uInt64 nNanoSec )
{
    init(nHour, nMin, nSec, nNanoSec);
}
Time::Time( const css::util::Time &_rTime )
{
    init(_rTime.Hours, _rTime.Minutes, _rTime.Seconds, _rTime.NanoSeconds);
}
Time::Time( const css::util::DateTime &_rDateTime )
{
    init(_rDateTime.Hours, _rDateTime.Minutes, _rDateTime.Seconds, _rDateTime.NanoSeconds);
}

void tools::Time::init( sal_uInt32 nHour, sal_uInt32 nMin, sal_uInt32 nSec, sal_uInt64 nNanoSec )
{
    // normalize time
    nSec     += nNanoSec / nanoSecInSec;
    nNanoSec %= nanoSecInSec;
    nMin     += nSec / secInMin;
    nSec     %= secInMin;
    nHour    += nMin / minInHour;
    nMin     %= minInHour;

    // construct time
    nTime = nNanoSec +
            nSec  * SEC_MASK +
            nMin  * MIN_MASK +
            nHour * HOUR_MASK;
}

void tools::Time::SetHour( sal_uInt16 nNewHour )
{
    short  nSign          = (nTime >= 0) ? +1 : -1;
    sal_Int32   nMin      = GetMin();
    sal_Int32   nSec      = GetSec();
    sal_Int32   nNanoSec  = GetNanoSec();

    nTime = nSign *
            ( nNanoSec +
              nSec  * SEC_MASK +
              nMin  * MIN_MASK +
              nNewHour * HOUR_MASK );
}

void tools::Time::SetMin( sal_uInt16 nNewMin )
{
    short  nSign          = (nTime >= 0) ? +1 : -1;
    sal_Int32   nHour     = GetHour();
    sal_Int32   nSec      = GetSec();
    sal_Int32   nNanoSec  = GetNanoSec();

    // no overflow
    nNewMin = nNewMin % minInHour;

    nTime = nSign *
            ( nNanoSec +
              nSec  * SEC_MASK +
              nNewMin  * MIN_MASK +
              nHour * HOUR_MASK );
}

void tools::Time::SetSec( sal_uInt16 nNewSec )
{
    short       nSign     = (nTime >= 0) ? +1 : -1;
    sal_Int32   nHour     = GetHour();
    sal_Int32   nMin      = GetMin();
    sal_Int32   nNanoSec   = GetNanoSec();

    // no overflow
    nNewSec = nNewSec % secInMin;

    nTime = nSign *
            ( nNanoSec +
              nNewSec  * SEC_MASK +
              nMin  * MIN_MASK +
              nHour * HOUR_MASK );
}

void tools::Time::SetNanoSec( sal_uInt32 nNewNanoSec )
{
    short       nSign     = (nTime >= 0) ? +1 : -1;
    sal_Int32   nHour     = GetHour();
    sal_Int32   nMin      = GetMin();
    sal_Int32   nSec      = GetSec();

    // no overflow
    nNewNanoSec = nNewNanoSec % nanoSecInSec;

    nTime = nSign *
            ( nNewNanoSec +
              nSec  * SEC_MASK +
              nMin  * MIN_MASK +
              nHour * HOUR_MASK );
}

sal_Int64 tools::Time::GetNSFromTime() const
{
    short       nSign     = (nTime >= 0) ? +1 : -1;
    sal_Int32   nHour     = GetHour();
    sal_Int32   nMin      = GetMin();
    sal_Int32   nSec      = GetSec();
    sal_Int32   nNanoSec  = GetNanoSec();

    return nSign *
           ( nNanoSec +
             nSec  * nanoSecInSec +
             nMin  * (secInMin * nanoSecInSec) +
             nHour * (minInHour * secInMin * nanoSecInSec) );
}

void tools::Time::MakeTimeFromNS( sal_Int64 nNS )
{
    short nSign;
    if ( nNS < 0 )
    {
        nNS *= -1;
        nSign = -1;
    }
    else
        nSign = 1;

    // avoid overflow when sal_uIntPtr is 32 bits
    tools::Time aTime( 0, 0, nNS/nanoSecInSec, nNS % nanoSecInSec );
    SetTime( aTime.GetTime() * nSign );
}

sal_Int32 tools::Time::GetMSFromTime() const
{
    short       nSign     = (nTime >= 0) ? +1 : -1;
    sal_Int32   nHour     = GetHour();
    sal_Int32   nMin      = GetMin();
    sal_Int32   nSec      = GetSec();
    sal_Int32   nNanoSec  = GetNanoSec();

    return nSign *
           ( nNanoSec/1000000 +
             nSec  * 1000 +
             nMin  * 60000 +
             nHour * 3600000 );
}

void tools::Time::MakeTimeFromMS( sal_Int32 nMS )
{
    short nSign;
    if ( nMS < 0 )
    {
        nMS *= -1;
        nSign = -1;
    }
    else
        nSign = 1;

    // avoid overflow when sal_uIntPtr is 32 bits
    tools::Time aTime( 0, 0, nMS/1000, (nMS % 1000) * 1000000 );
    SetTime( aTime.GetTime() * nSign );
}

double tools::Time::GetTimeInDays() const
{
    short  nSign      = (nTime >= 0) ? +1 : -1;
    double nHour      = GetHour();
    double nMin       = GetMin();
    double nSec       = GetSec();
    double nNanoSec   = GetNanoSec();

    return (nHour + (nMin / 60) + (nSec / (minInHour * secInMin)) + (nNanoSec / (minInHour * secInMin * nanoSecInSec))) / 24 * nSign;
}

// static
void tools::Time::GetClock( double fTimeInDays,
                            sal_uInt16& nHour, sal_uInt16& nMinute, sal_uInt16& nSecond,
                            double& fFractionOfSecond, int nFractionDecimals )
{
    const double fTime = fTimeInDays - rtl::math::approxFloor(fTimeInDays); // date part absent

    // If 0 then full day (or no day), shortcut.
    // If < 0 then approxFloor() effectively returned the ceiling (note this
    // also holds for negative fTimeInDays values) because of a near identical
    // value, shortcut this to a full day as well.
    // If >= 1.0 (actually == 1.0) then fTimeInDays is a negative small value
    // not significant for a representable time and approxFloor() returned -1,
    // shortcut to 0:0:0, otherwise it would become 24:0:0.
    if (fTime <= 0.0 || fTime >= 1.0)
    {
        nHour = nMinute = nSecond = 0;
        fFractionOfSecond = 0.0;
        return;
    }

    // In seconds, including milli and nano.
    const double fRawSeconds = fTime * tools::Time::secondPerDay;

    // Round to nanoseconds most, which is the highest resolution this could be
    // influenced by, but if the original value included a date round to at
    // most 14 significant digits (including adding 4 for *86400), otherwise we
    // might end up with a fake precision of h:m:s.999999986 which in fact
    // should had been h:m:s+1
    // BUT, leave at least 2 decimals to round. Which shouldn't be a problem in
    // practice because class Date can calculate only 8-digit days for it's
    // sal_Int16 year range, which exactly leaves us with 14-4-8=2.
    int nDec = 9;
    const double fAbsTimeInDays = fabs( fTimeInDays);
    if (fAbsTimeInDays >= 1.0)
    {
        const int nDig = static_cast<int>(ceil( log10( fAbsTimeInDays)));
        nDec = std::clamp( 10 - nDig, 2, 9 );
    }
    double fSeconds = rtl::math::round( fRawSeconds, nDec);

    // If this ended up as a full day the original value was very very close
    // but not quite. Take that.
    if (fSeconds >= tools::Time::secondPerDay)
        fSeconds = fRawSeconds;

    // Now do not round values (specifically not up), but truncate to the next
    // magnitude, so 23:59:59.99 is still 23:59:59 and not 24:00:00 (or even
    // 00:00:00 which Excel does).
    nHour = fSeconds / tools::Time::secondPerHour;
    fSeconds -= nHour * tools::Time::secondPerHour;
    nMinute = fSeconds / tools::Time::secondPerMinute;
    fSeconds -= nMinute * tools::Time::secondPerMinute;
    nSecond = fSeconds;
    fSeconds -= nSecond;

    assert(fSeconds < 1.0);     // or back to the drawing board...

    if (nFractionDecimals > 0)
    {
        // Do not simply round the fraction, otherwise .999 would end up as .00
        // again. Truncate instead if rounding would round up into an integer
        // value.
        fFractionOfSecond = rtl::math::round( fSeconds, nFractionDecimals);
        if (fFractionOfSecond >= 1.0)
            fFractionOfSecond = rtl::math::pow10Exp( std::trunc(
                        rtl::math::pow10Exp( fSeconds, nFractionDecimals)), -nFractionDecimals);
    }
    else
        fFractionOfSecond = fSeconds;
}

Time& tools::Time::operator =( const tools::Time& rTime )
{
    nTime = rTime.nTime;
    return *this;
}

Time& tools::Time::operator +=( const tools::Time& rTime )
{
    nTime = NanoSecToTime( TimeToNanoSec( *this ) +
                           TimeToNanoSec( rTime ) ).GetTime();
    return *this;
}

Time& tools::Time::operator -=( const tools::Time& rTime )
{
    nTime = NanoSecToTime( TimeToNanoSec( *this ) -
                           TimeToNanoSec( rTime ) ).GetTime();
    return *this;
}

Time operator +( const tools::Time& rTime1, const tools::Time& rTime2 )
{
    return NanoSecToTime( TimeToNanoSec( rTime1 ) +
                          TimeToNanoSec( rTime2 ) );
}

Time operator -( const tools::Time& rTime1, const tools::Time& rTime2 )
{
    return NanoSecToTime( TimeToNanoSec( rTime1 ) -
                          TimeToNanoSec( rTime2 ) );
}

bool tools::Time::IsEqualIgnoreNanoSec( const tools::Time& rTime ) const
{
    sal_Int32 n1 = (nTime       < 0 ? -static_cast<sal_Int32>(GetNanoSec())       : GetNanoSec() );
    sal_Int32 n2 = (rTime.nTime < 0 ? -static_cast<sal_Int32>(rTime.GetNanoSec()) : rTime.GetNanoSec() );
    return (nTime - n1) == (rTime.nTime - n2);
}

Time tools::Time::GetUTCOffset()
{
#if defined(_WIN32)
    TIME_ZONE_INFORMATION   aTimeZone;
    aTimeZone.Bias = 0;
    DWORD nTimeZoneRet = GetTimeZoneInformation( &aTimeZone );
    sal_Int32 nTempTime = aTimeZone.Bias;
    if ( nTimeZoneRet == TIME_ZONE_ID_STANDARD )
        nTempTime += aTimeZone.StandardBias;
    else if ( nTimeZoneRet == TIME_ZONE_ID_DAYLIGHT )
        nTempTime += aTimeZone.DaylightBias;
    tools::Time aTime( 0, static_cast<sal_uInt16>(abs( nTempTime )) );
    if ( nTempTime > 0 )
        aTime = -aTime;
    return aTime;
#else
    static sal_uInt64   nCacheTicks = 0;
    static sal_Int32    nCacheSecOffset = -1;
    sal_uInt64          nTicks = tools::Time::GetSystemTicks();
    time_t          nTime;
    tm              aTM;
    short           nTempTime;

    // determine value again if needed
    if ( (nCacheSecOffset == -1)            ||
         ((nTicks - nCacheTicks) > 360000)  ||
         ( nTicks < nCacheTicks ) // handle overflow
         )
    {
        nTime = time( nullptr );
        localtime_r( &nTime, &aTM );
        sal_Int32 nLocalTime = mktime( &aTM );
#if defined(__sun)
        // Solaris gmtime_r() seems not to handle daylight saving time
        // flags correctly
        nUTC = nLocalTime + ( aTM.tm_isdst == 0 ? timezone : altzone );
#elif defined( LINUX )
        // Linux mktime() seems not to handle tm_isdst correctly
        sal_Int32 nUTC = nLocalTime - aTM.tm_gmtoff;
#else
        gmtime_r( &nTime, &aTM );
        sal_Int32 nUTC = mktime( &aTM );
#endif
        nCacheTicks = nTicks;
        nCacheSecOffset = (nLocalTime-nUTC) / 60;
    }

    nTempTime = abs( nCacheSecOffset );
    tools::Time aTime( 0, static_cast<sal_uInt16>(nTempTime) );
    if ( nCacheSecOffset < 0 )
        aTime = -aTime;
    return aTime;
#endif
}

sal_uInt64 tools::Time::GetSystemTicks()
{
    return tools::Time::GetMonotonicTicks() / 1000;
}

#ifdef _WIN32
static LARGE_INTEGER initPerformanceFrequency()
{
    LARGE_INTEGER nTicksPerSecond = { 0, 0 };
    if (!QueryPerformanceFrequency(&nTicksPerSecond))
        nTicksPerSecond.QuadPart = 0;
    return nTicksPerSecond;
}
#endif

sal_uInt64 tools::Time::GetMonotonicTicks()
{
#ifdef _WIN32
    static const LARGE_INTEGER nTicksPerSecond = initPerformanceFrequency();
    if (nTicksPerSecond.QuadPart > 0)
    {
        LARGE_INTEGER nPerformanceCount;
        QueryPerformanceCounter(&nPerformanceCount);
        return static_cast<sal_uInt64>(
            ( nPerformanceCount.QuadPart * 1000 * 1000 ) / nTicksPerSecond.QuadPart );
    }
    else
    {
        return static_cast<sal_uInt64>( timeGetTime() * 1000 );
    }
#else
    sal_uInt64 nMicroSeconds;
#ifdef __MACH__
    static mach_timebase_info_data_t info = { 0, 0 };
    if ( 0 == info.numer )
        mach_timebase_info( &info );
    nMicroSeconds = mach_absolute_time() * static_cast<double>(info.numer / info.denom) / 1000;
#else
#if defined(USE_CLOCK_GETTIME)
    struct timespec currentTime;
    clock_gettime( CLOCK_MONOTONIC, &currentTime );
    nMicroSeconds
        = static_cast<sal_uInt64>(currentTime.tv_sec) * 1000 * 1000 + currentTime.tv_nsec / 1000;
#else
    struct timeval currentTime;
    gettimeofday( &currentTime, nullptr );
    nMicroSeconds = static_cast<sal_uInt64>(currentTime.tv_sec) * 1000 * 1000 + currentTime.tv_usec;
#endif
#endif // __MACH__
    return nMicroSeconds;
#endif // _WIN32
}

} /* namespace tools */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
