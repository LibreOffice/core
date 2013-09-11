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


#include "system.h"

#include <osl/diagnose.h>
#include <osl/time.h>
#include <time.h>
#include <assert.h>
#include <unistd.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

/* FIXME: detection should be done in configure script */
#if defined(MACOSX) || defined(IOS) || defined(FREEBSD) || defined(NETBSD) || \
    defined(LINUX) || defined(OPENBSD) || defined(DRAGONFLY)
#define STRUCT_TM_HAS_GMTOFF 1

#elif defined(SOLARIS)
#define HAS_ALTZONE 1
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


/*--------------------------------------------------
 * osl_getSystemTime
 *-------------------------------------------------*/

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
        return sal_False;
    }

    tv->Seconds = tp.tv_sec;
    #if defined(USE_CLOCK_GETTIME)
    tv->Nanosec = tp.tv_nsec;
    #else
    tv->Nanosec = tp.tv_usec * 1000;
    #endif
#endif
    return sal_True;
}


/*--------------------------------------------------
 * osl_getDateTimeFromTimeValue
 *-------------------------------------------------*/

sal_Bool SAL_CALL osl_getDateTimeFromTimeValue( const TimeValue* pTimeVal, oslDateTime* pDateTime )
{
    struct tm *pSystemTime;
    struct tm tmBuf;
    time_t atime;

    atime = (time_t)pTimeVal->Seconds;

    /* Convert time from type time_t to struct tm */
    pSystemTime = gmtime_r( &atime, &tmBuf );


    /* Convert struct tm to struct oslDateTime */
    if ( pSystemTime != NULL )
    {
        pDateTime->NanoSeconds  =   pTimeVal->Nanosec;
        pDateTime->Seconds      =   pSystemTime->tm_sec;
        pDateTime->Minutes      =   pSystemTime->tm_min;
        pDateTime->Hours        =   pSystemTime->tm_hour;
        pDateTime->Day          =   pSystemTime->tm_mday;
        pDateTime->DayOfWeek    =   pSystemTime->tm_wday;
        pDateTime->Month        =   pSystemTime->tm_mon + 1;
        pDateTime->Year         =   pSystemTime->tm_year  + 1900;

        return sal_True;
    }

    return sal_False;
}

/*--------------------------------------------------
 * osl_getTimeValueFromDateTime
 *--------------------------------------------------*/

sal_Bool SAL_CALL osl_getTimeValueFromDateTime( const oslDateTime* pDateTime, TimeValue* pTimeVal )
{
    struct tm   aTime;
    time_t      nSeconds;

    /* Convert struct oslDateTime to struct tm */
    aTime.tm_sec  = pDateTime->Seconds;
    aTime.tm_min  = pDateTime->Minutes;
    aTime.tm_hour = pDateTime->Hours;
    aTime.tm_mday = pDateTime->Day;
    aTime.tm_wday = pDateTime->DayOfWeek;

    if ( pDateTime->Month > 0 )
        aTime.tm_mon = pDateTime->Month - 1;
    else
        return sal_False;

    aTime.tm_year = pDateTime->Year - 1900;

    aTime.tm_isdst = -1;
    aTime.tm_wday  = 0;
    aTime.tm_yday  = 0;

#if defined(STRUCT_TM_HAS_GMTOFF)
    aTime.tm_gmtoff = 0;
#endif

    /* Convert time to calendar value */
    nSeconds = mktime( &aTime );

    /*
     * mktime expects the struct tm to be in local timezone, so we have to adjust
     * the returned value to be timezone neutral.
     */

    if ( nSeconds != (time_t) -1 )
    {
        time_t bias;

        /* timezone corrections */
        tzset();

#if defined(STRUCT_TM_HAS_GMTOFF)
        /* members of struct tm are corrected by mktime */
        bias = 0 - aTime.tm_gmtoff;

#elif defined(HAS_ALTZONE)
        /* check if daylight saving time is in effect */
        bias = aTime.tm_isdst > 0 ? altzone : timezone;
#else
        /* exspect daylight saving time to be one hour */
        bias = aTime.tm_isdst > 0 ? timezone - 3600 : timezone;
#endif

        pTimeVal->Seconds = nSeconds;
        pTimeVal->Nanosec = pDateTime->NanoSeconds;

        if ( nSeconds > bias )
            pTimeVal->Seconds -= bias;

        return sal_True;
    }

    return sal_False;
}


/*--------------------------------------------------
 * osl_getLocalTimeFromSystemTime
 *--------------------------------------------------*/

sal_Bool SAL_CALL osl_getLocalTimeFromSystemTime( const TimeValue* pSystemTimeVal, TimeValue* pLocalTimeVal )
{
    struct tm *pLocalTime;
    struct tm tmBuf;
    time_t bias;
    time_t atime;

    atime = (time_t) pSystemTimeVal->Seconds;
    pLocalTime = localtime_r( &atime, &tmBuf );

#if defined(STRUCT_TM_HAS_GMTOFF)
    /* members of struct tm are corrected by mktime */
    bias = -pLocalTime->tm_gmtoff;

#elif defined(HAS_ALTZONE)
    /* check if daylight saving time is in effect */
    bias = pLocalTime->tm_isdst > 0 ? altzone : timezone;
#else
    /* expect daylight saving time to be one hour */
    bias = pLocalTime->tm_isdst > 0 ? timezone - 3600 : timezone;
#endif

    if ( (sal_Int64) pSystemTimeVal->Seconds > bias )
    {
        pLocalTimeVal->Seconds = pSystemTimeVal->Seconds - bias;
        pLocalTimeVal->Nanosec = pSystemTimeVal->Nanosec;

        return sal_True;
    }

    return sal_False;
}

/*--------------------------------------------------
 * osl_getSystemTimeFromLocalTime
 *--------------------------------------------------*/

sal_Bool SAL_CALL osl_getSystemTimeFromLocalTime( const TimeValue* pLocalTimeVal, TimeValue* pSystemTimeVal )
{
    struct tm *pLocalTime;
    struct tm tmBuf;
    time_t bias;
    time_t atime;

    atime = (time_t) pLocalTimeVal->Seconds;

    /* Convert atime, which is a local time, to it's GMT equivalent. Then, get
     * the timezone offset for the local time for the GMT equivalent time. Note
     * that we cannot directly use local time to determine the timezone offset
     * because GMT is the only reliable time that we can determine timezone
     * offset from.
     */

    atime = mktime( gmtime_r( &atime, &tmBuf ) );
    pLocalTime = localtime_r( &atime, &tmBuf );

#if defined(STRUCT_TM_HAS_GMTOFF)
    /* members of struct tm are corrected by mktime */
    bias = 0 - pLocalTime->tm_gmtoff;

#elif defined(HAS_ALTZONE)
    /* check if daylight saving time is in effect */
    bias = pLocalTime->tm_isdst > 0 ? altzone : timezone;
#else
    /* exspect daylight saving time to be one hour */
    bias = pLocalTime->tm_isdst > 0 ? timezone - 3600 : timezone;
#endif

    if ( (sal_Int64) pLocalTimeVal->Seconds + bias > 0 )
    {
        pSystemTimeVal->Seconds = pLocalTimeVal->Seconds + bias;
        pSystemTimeVal->Nanosec = pLocalTimeVal->Nanosec;

        return sal_True;
    }

    return sal_False;
}

void sal_initGlobalTimer()
{
#ifdef __MACH__
  clock_serv_t cclock;

  host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
  clock_get_time(cclock, &startTime);
  mach_port_deallocate(mach_task_self(), cclock);
#else /* ! (MACOSX || IOS) */
  int res;
#if defined(USE_CLOCK_GETTIME)
  res = clock_gettime(CLOCK_REALTIME, &startTime);
#else /* Ndef USE_CLOCK_GETTIME */
  res = gettimeofday( &startTime, NULL );
#endif /* NDef USE_CLOCK_GETTIME */
  assert(res == 0);
  (void) res;
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
    nSeconds = ( nSeconds * 1000 ) + (long) (( currentTime.tv_nsec - startTime.tv_nsec) / 1000000 );
#else
    osl_time_t currentTime;
    int res;

#if defined(USE_CLOCK_GETTIME)
    res = clock_gettime(CLOCK_REALTIME, &currentTime);
#else
    res = gettimeofday( &currentTime, NULL );
#endif
    assert(res == 0);

    if (res != 0)
        return 0;

    nSeconds = (sal_uInt32)( currentTime.tv_sec - startTime.tv_sec );
#if defined(USE_CLOCK_GETTIME)
    nSeconds = ( nSeconds * 1000 ) + (long) (( currentTime.tv_nsec - startTime.tv_nsec) / 1000000 );
#else
    nSeconds = ( nSeconds * 1000 ) + (long) (( currentTime.tv_usec - startTime.tv_usec) / 1000 );
#endif
#endif
    return nSeconds;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
