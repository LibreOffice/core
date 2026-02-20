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

#include "saltime.hxx"
#include "tz.hxx"

#include <cstdint>
#include <osl/time.h>
#include <time.h>
#include <unistd.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

#ifdef __MACH__
typedef mach_timespec_t osl_time_t;
#else
#if defined(_POSIX_TIMERS)
#define USE_CLOCK_GETTIME
typedef struct timespec osl_time_t;
#else
typedef struct timeval osl_time_t;
#endif
#endif
static osl_time_t startTime;

sal_Bool SAL_CALL osl_getSystemTime(TimeValue* tv)
{
#ifdef __MACH__
    clock_serv_t cclock;
    mach_timespec_t mts;

    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);

    tv->Seconds = mts.tv_sec;
    tv->Nanosec = mts.tv_nsec;
#else
    int res;
    osl_time_t tp;
#if defined(USE_CLOCK_GETTIME)
    res = clock_gettime(CLOCK_REALTIME, &tp);
#else
    res = gettimeofday(&tp, NULL);
#endif

    if (res != 0)
    {
        return false;
    }

    tv->Seconds = tp.tv_sec;
    #if defined(USE_CLOCK_GETTIME)
    tv->Nanosec = tp.tv_nsec;
    #else
    tv->Nanosec = tp.tv_usec * 1000;
    #endif
#endif
    return true;
}

sal_Bool SAL_CALL osl_getDateTimeFromTimeValue( const TimeValue* pTimeVal, oslDateTime* pDateTime )
{
    osl::tz::BrokenDown bd;
    if (!osl::tz::epochToUtc(static_cast<std::int64_t>(pTimeVal->Seconds), bd))
        return false;

    pDateTime->NanoSeconds = pTimeVal->Nanosec;
    pDateTime->Seconds = bd.second;
    pDateTime->Minutes = bd.minute;
    pDateTime->Hours = bd.hour;
    pDateTime->Day = bd.day;
    pDateTime->DayOfWeek = bd.dayOfWeek;
    pDateTime->Month = bd.month;
    pDateTime->Year = bd.year;

    return true;
}

sal_Bool SAL_CALL osl_getTimeValueFromDateTime( const oslDateTime* pDateTime, TimeValue* pTimeVal )
{
    /* The API says pDateTime is in GMT, so this is a pure UTC-to-epoch conversion. */
    osl::tz::BrokenDown bd;
    bd.year = pDateTime->Year;
    bd.month = pDateTime->Month;
    bd.day = pDateTime->Day;
    bd.hour = pDateTime->Hours;
    bd.minute = pDateTime->Minutes;
    bd.second = pDateTime->Seconds;
    bd.dayOfWeek = 0; // unused for this conversion

    std::int64_t epoch;
    if (!osl::tz::utcToEpoch(bd, epoch))
        return false;

    pTimeVal->Seconds = static_cast<sal_uInt32>(epoch);
    pTimeVal->Nanosec = pDateTime->NanoSeconds;

    return true;
}

sal_Bool SAL_CALL osl_getLocalTimeFromSystemTime( const TimeValue* pSystemTimeVal, TimeValue* pLocalTimeVal )
{
    std::int64_t utcEpoch = static_cast<std::int64_t>(pSystemTimeVal->Seconds);
    std::int32_t offset = osl::tz::getUtcOffsetForUtcTime(utcEpoch);
    std::int64_t localEpoch = utcEpoch + offset;

    if (localEpoch < 0)
        return false;

    pLocalTimeVal->Seconds = static_cast<sal_uInt32>(localEpoch);
    pLocalTimeVal->Nanosec = pSystemTimeVal->Nanosec;

    return true;
}

sal_Bool SAL_CALL osl_getSystemTimeFromLocalTime( const TimeValue* pLocalTimeVal, TimeValue* pSystemTimeVal )
{
    std::int64_t localEpoch = static_cast<std::int64_t>(pLocalTimeVal->Seconds);
    std::int32_t offset = osl::tz::getUtcOffsetForLocalTime(localEpoch);
    std::int64_t utcEpoch = localEpoch - offset;

    if (utcEpoch < 0)
        return false;

    pSystemTimeVal->Seconds = static_cast<sal_uInt32>(utcEpoch);
    pSystemTimeVal->Nanosec = pLocalTimeVal->Nanosec;

    return true;
}

void sal_initGlobalTimer()
{
#ifdef __MACH__
  clock_serv_t cclock;

  host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
  clock_get_time(cclock, &startTime);
  mach_port_deallocate(mach_task_self(), cclock);
#else /* ! (MACOSX || IOS) */
#if defined(USE_CLOCK_GETTIME)
  clock_gettime(CLOCK_REALTIME, &startTime);
#else /* Ndef USE_CLOCK_GETTIME */
  gettimeofday( &startTime, NULL );
#endif /* NDef USE_CLOCK_GETTIME */
#endif /* ! (MACOSX || IOS) */
}

sal_uInt32 SAL_CALL osl_getGlobalTimer()
{
    sal_uInt32 nSeconds;

#ifdef __MACH__
    clock_serv_t cclock;
    mach_timespec_t currentTime;

    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &currentTime);
    mach_port_deallocate(mach_task_self(), cclock);

    nSeconds = ( currentTime.tv_sec - startTime.tv_sec );
    nSeconds = ( nSeconds * 1000 ) + static_cast<long>(( currentTime.tv_nsec - startTime.tv_nsec) / 1000000 );
#else
    osl_time_t currentTime;

#if defined(USE_CLOCK_GETTIME)
    clock_gettime(CLOCK_REALTIME, &currentTime);
#else
    gettimeofday( &currentTime, NULL );
#endif

    nSeconds = static_cast<sal_uInt32>( currentTime.tv_sec - startTime.tv_sec );
#if defined(USE_CLOCK_GETTIME)
    nSeconds = ( nSeconds * 1000 ) + static_cast<long>(( currentTime.tv_nsec - startTime.tv_nsec) / 1000000 );
#else
    nSeconds = ( nSeconds * 1000 ) + (long) (( currentTime.tv_usec - startTime.tv_usec) / 1000 );
#endif
#endif
    return nSeconds;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
