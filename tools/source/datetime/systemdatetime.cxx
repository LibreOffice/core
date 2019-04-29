/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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
#if defined(_WIN32)
#if !defined WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#elif defined UNX
#include <sys/time.h>
#endif

#include <time.h>
#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#endif

#include <osl/diagnose.h>
#include <systemdatetime.hxx>

namespace
{
constexpr sal_Int32 ConvertYMDToInt(sal_Int32 nYear, sal_Int32 nMonth, sal_Int32 nDay)
{
    return (nYear * 10000) + (nMonth * 100) + nDay;
}

constexpr sal_Int64 ConvertHMSnToInt(sal_Int64 nHour, sal_Int64 nMin, sal_Int64 nSec,
                                     sal_Int64 nNanoSec)
{
    return (nHour * HOUR_MASK) + (nMin * MIN_MASK) + (nSec * SEC_MASK) + nNanoSec;
}
}

bool GetSystemDateTime(sal_Int32* pDate, sal_Int64* pTime)
{
#if defined(_WIN32)
    SYSTEMTIME aDateTime;
    GetLocalTime(&aDateTime);

    if (pDate)
        *pDate = ConvertYMDToInt(static_cast<sal_Int32>(aDateTime.wYear),
                                 static_cast<sal_Int32>(aDateTime.wMonth),
                                 static_cast<sal_Int32>(aDateTime.wDay));
    if (pTime)
        *pTime = ConvertHMSnToInt(aDateTime.wHour, aDateTime.wMinute, aDateTime.wSecond,
                                  aDateTime.wMilliseconds * 1000000);

    return true;
#else
    struct timespec tsTime;
#if defined(__MACH__)
    // macOS does not have clock_gettime, use clock_get_time
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    tsTime.tv_sec = mts.tv_sec;
    tsTime.tv_nsec = mts.tv_nsec;
#else
    // CLOCK_REALTIME should be supported
    // on any modern Unix, but be extra cautious
    if (clock_gettime(CLOCK_REALTIME, &tsTime) != 0)
    {
        struct timeval tvTime;
        OSL_VERIFY(gettimeofday(&tvTime, nullptr) != 0);
        tsTime.tv_sec = tvTime.tv_sec;
        tsTime.tv_nsec = tvTime.tv_usec * 1000;
    }
#endif

    struct tm aTime;
    time_t nTmpTime = tsTime.tv_sec;
    if (localtime_r(&nTmpTime, &aTime))
    {
        if (pDate)
            *pDate = ConvertYMDToInt(static_cast<sal_Int32>(aTime.tm_year + 1900),
                                     static_cast<sal_Int32>(aTime.tm_mon + 1),
                                     static_cast<sal_Int32>(aTime.tm_mday));
        if (pTime)
            *pTime = ConvertHMSnToInt(aTime.tm_hour, aTime.tm_min, aTime.tm_sec, tsTime.tv_nsec);
        return true;
    }

    return false;
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
