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

#include <cerrno>

#if defined(_WIN32)
#include <windows.h>
#elif defined UNX
#include <unistd.h>
#include <limits.h>
#include <math.h>
#include <sys/time.h>
#endif

#include <time.h>
#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

#include <sal/log.hxx>
#include <tools/time.hxx>
#include <osl/diagnose.h>

#if defined(SOLARIS) && defined(__GNUC__)
extern long altzone;
#endif

namespace {

    const sal_Int64 secMask  = SAL_CONST_INT64(1000000000);
    const sal_Int64 minMask  = SAL_CONST_INT64(100000000000);
    const sal_Int64 hourMask = SAL_CONST_INT64(10000000000000);

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
#if defined(_WIN32)
    SYSTEMTIME aDateTime;
    GetLocalTime( &aDateTime );

    // construct time
    nTime = aDateTime.wHour         * hourMask +
            aDateTime.wMinute       * minMask +
            aDateTime.wSecond       * secMask +
            aDateTime.wMilliseconds * 1000000;
#else
    // determine time
    struct timespec tsTime;
#if defined( __MACH__ )
    // OS X does not have clock_gettime, use clock_get_time
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    tsTime.tv_sec  = mts.tv_sec;
    tsTime.tv_nsec = mts.tv_nsec;
#else
    // CLOCK_REALTIME should be supported
    // on any modern Unix, but be extra cautious
    if (clock_gettime(CLOCK_REALTIME, &tsTime) != 0)
    {
        struct timeval tvTime;
        OSL_VERIFY( gettimeofday(&tvTime, nullptr) != 0 );
        tsTime.tv_sec  = tvTime.tv_sec;
        tsTime.tv_nsec = tvTime.tv_usec * 1000;
    }
#endif // __MACH__

    // construct time
    struct tm aTime;
    time_t nTmpTime = tsTime.tv_sec;
    if ( localtime_r( &nTmpTime, &aTime ) )
    {
        nTime = aTime.tm_hour * hourMask +
                aTime.tm_min  * minMask +
                aTime.tm_sec  * secMask +
                tsTime.tv_nsec;
    }
    else
        nTime = 0;
#endif // WNT
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
            nSec  * secMask +
            nMin  * minMask +
            nHour * hourMask;
}

void tools::Time::SetHour( sal_uInt16 nNewHour )
{
    short  nSign          = (nTime >= 0) ? +1 : -1;
    sal_Int32   nMin      = GetMin();
    sal_Int32   nSec      = GetSec();
    sal_Int32   nNanoSec  = GetNanoSec();

    nTime = nSign *
            ( nNanoSec +
              nSec  * secMask +
              nMin  * minMask +
              nNewHour * hourMask );
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
              nSec  * secMask +
              nNewMin  * minMask +
              nHour * hourMask );
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
              nNewSec  * secMask +
              nMin  * minMask +
              nHour * hourMask );
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
              nSec  * secMask +
              nMin  * minMask +
              nHour * hourMask );
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
             nHour * 360000 );
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
    tools::Time aTime( 0, (sal_uInt16)abs( nTempTime ) );
    if ( nTempTime > 0 )
        aTime = -aTime;
    return aTime;
#else
    static sal_uInt64   nCacheTicks = 0;
    static sal_Int32    nCacheSecOffset = -1;
    sal_uInt64          nTicks = tools::Time::GetSystemTicks();
    time_t          nTime;
    tm              aTM;
    sal_Int32           nLocalTime;
    sal_Int32           nUTC;
    short           nTempTime;

    // determine value again if needed
    if ( (nCacheSecOffset == -1)            ||
         ((nTicks - nCacheTicks) > 360000)  ||
         ( nTicks < nCacheTicks ) // handle overflow
         )
    {
        nTime = time( nullptr );
        localtime_r( &nTime, &aTM );
        nLocalTime = mktime( &aTM );
#if defined( SOLARIS )
        // Solaris gmtime_r() seems not to handle daylight saving time
        // flags correctly
        nUTC = nLocalTime + ( aTM.tm_isdst == 0 ? timezone : altzone );
#elif defined( LINUX )
        // Linux mktime() seems not to handle tm_isdst correctly
        nUTC = nLocalTime - aTM.tm_gmtoff;
#else
         gmtime_r( &nTime, &aTM );
         nUTC = mktime( &aTM );
#endif
         nCacheTicks = nTicks;
         nCacheSecOffset = (nLocalTime-nUTC) / 60;
    }

    nTempTime = abs( nCacheSecOffset );
    tools::Time aTime( 0, (sal_uInt16)nTempTime );
    if ( nCacheSecOffset < 0 )
        aTime = -aTime;
    return aTime;
#endif
}

sal_uInt64 tools::Time::GetSystemTicks()
{
#if defined(_WIN32)
    static LARGE_INTEGER nTicksPerSecond;
    static bool bTicksPerSecondInitialized = false;
    if (!bTicksPerSecondInitialized)
    {
        QueryPerformanceFrequency(&nTicksPerSecond);
        bTicksPerSecondInitialized = true;
    }

    LARGE_INTEGER nPerformanceCount;
    QueryPerformanceCounter(&nPerformanceCount);

    return static_cast<sal_uInt64>(
        (nPerformanceCount.QuadPart*1000)/nTicksPerSecond.QuadPart);
#else
    timeval tv;
    int n = gettimeofday (&tv, nullptr);
    if (n == -1) {
        int e = errno;
        SAL_WARN("tools.datetime", "gettimeofday failed: " << e);
    }
    return static_cast<sal_uInt64>(tv.tv_sec) * 1000
        + (static_cast<sal_uInt64>(tv.tv_usec) + 500) / 1000;
#endif
}

} /* namespace tools */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
