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
#include "system.h"

#include "filetime.hxx"
#include "time.hxx"

#include <osl/diagnose.h>
#include <osl/time.h>
#include <sys/timeb.h>

sal_Bool SAL_CALL osl_getSystemTime(TimeValue* pTimeVal)
{
    unsigned __int64 CurTime;

    typedef VOID (WINAPI *GetSystemTimePreciseAsFileTime_PROC)(LPFILETIME);

    OSL_ASSERT(pTimeVal != nullptr);

    static GetSystemTimePreciseAsFileTime_PROC pGetSystemTimePreciseAsFileTime = [&]()
    {
        HMODULE hModule = GetModuleHandleW( L"Kernel32.dll" );
        return reinterpret_cast<GetSystemTimePreciseAsFileTime_PROC>(
                GetProcAddress(hModule, "GetSystemTimePreciseAsFileTime"));
    }();

    // use ~1 microsecond resolution if available
    if (pGetSystemTimePreciseAsFileTime)
        pGetSystemTimePreciseAsFileTime(reinterpret_cast<LPFILETIME>(&CurTime));
    else
    {
        SYSTEMTIME SystemTime;
        GetSystemTime(&SystemTime);
        SystemTimeToFileTime(&SystemTime, reinterpret_cast<LPFILETIME>(&CurTime));
    }

    static const unsigned __int64 OffTime = [] {
        SYSTEMTIME SystemTime;
        SystemTime.wYear = 1970;
        SystemTime.wMonth = 1;
        SystemTime.wDayOfWeek = 0;
        SystemTime.wDay = 1;
        SystemTime.wHour = 0;
        SystemTime.wMinute = 0;
        SystemTime.wSecond = 0;
        SystemTime.wMilliseconds = 0;

        unsigned __int64 ft;
        SystemTimeToFileTime(&SystemTime, reinterpret_cast<LPFILETIME>(&ft));
        return ft;
    }();

    const unsigned __int64 Value = CurTime - OffTime;

    pTimeVal->Seconds  = static_cast<unsigned long>(Value / 10000000L);
    pTimeVal->Nanosec  = static_cast<unsigned long>((Value % 10000000L) * 100);

    return true;
}

sal_Bool SAL_CALL osl_getDateTimeFromTimeValue( const TimeValue* pTimeVal, oslDateTime* pDateTime )
{
    FILETIME    aFileTime;
    SYSTEMTIME  aSystemTime;

    if ( TimeValueToFileTime(pTimeVal, &aFileTime) )
    {
        if ( FileTimeToSystemTime( &aFileTime, &aSystemTime ) )
        {
            pDateTime->NanoSeconds  =   pTimeVal->Nanosec;

            pDateTime->Seconds      =   aSystemTime.wSecond;
            pDateTime->Minutes      =   aSystemTime.wMinute;
            pDateTime->Hours        =   aSystemTime.wHour;
            pDateTime->Day          =   aSystemTime.wDay;
            pDateTime->DayOfWeek    =   aSystemTime.wDayOfWeek;
            pDateTime->Month        =   aSystemTime.wMonth;
            pDateTime->Year         =   aSystemTime.wYear;

            return true;
        }
    }

    return false;
}

sal_Bool SAL_CALL osl_getTimeValueFromDateTime( const oslDateTime* pDateTime, TimeValue* pTimeVal )
{
    FILETIME    aFileTime;
    SYSTEMTIME  aSystemTime;

    aSystemTime.wMilliseconds   =   0;
    aSystemTime.wSecond         =   pDateTime->Seconds;
    aSystemTime.wMinute         =   pDateTime->Minutes;
    aSystemTime.wHour           =   pDateTime->Hours;
    aSystemTime.wDay            =   pDateTime->Day;
    aSystemTime.wDayOfWeek      =   pDateTime->DayOfWeek;
    aSystemTime.wMonth          =   pDateTime->Month;
    aSystemTime.wYear           =   pDateTime->Year;

    if ( SystemTimeToFileTime( &aSystemTime, &aFileTime ) )
    {
        if (FileTimeToTimeValue( &aFileTime, pTimeVal  ) )
        {
            pTimeVal->Nanosec = pDateTime->NanoSeconds;
            return true;
        }
    }

    return false;
}

sal_Bool SAL_CALL osl_getLocalTimeFromSystemTime( const TimeValue* pSystemTimeVal, TimeValue* pLocalTimeVal )
{
    TIME_ZONE_INFORMATION aTimeZoneInformation;

    // get timezone information
    DWORD Success = GetTimeZoneInformation( &aTimeZoneInformation );
    if (Success == TIME_ZONE_ID_INVALID)
        return false;

    sal_Int64 bias = aTimeZoneInformation.Bias;

    // add bias for daylight saving time
    if ( Success == TIME_ZONE_ID_DAYLIGHT )
        bias+=aTimeZoneInformation.DaylightBias;

    if ( static_cast<sal_Int64>(pSystemTimeVal->Seconds) > ( bias * 60 ) )
    {
        pLocalTimeVal->Seconds = static_cast<sal_uInt32>(pSystemTimeVal->Seconds - ( bias * 60) );
        pLocalTimeVal->Nanosec = pSystemTimeVal->Nanosec;

        return true;
    }
    return false;
}

sal_Bool SAL_CALL osl_getSystemTimeFromLocalTime( const TimeValue* pLocalTimeVal, TimeValue* pSystemTimeVal )
{
    TIME_ZONE_INFORMATION aTimeZoneInformation;

    // get timezone information
    DWORD Success = GetTimeZoneInformation( &aTimeZoneInformation );
    if ( Success == TIME_ZONE_ID_INVALID )
        return false;

    sal_Int64 bias = aTimeZoneInformation.Bias;

    // add bias for daylight saving time
    if ( Success == TIME_ZONE_ID_DAYLIGHT )
        bias+=aTimeZoneInformation.DaylightBias;

    if ( static_cast<sal_Int64>(pLocalTimeVal->Seconds) + ( bias * 60 ) > 0 )
    {
        pSystemTimeVal->Seconds = static_cast<sal_uInt32>( pLocalTimeVal->Seconds + ( bias * 60) );
        pSystemTimeVal->Nanosec = pLocalTimeVal->Nanosec;

        return true;
    }

    return false;
}

static struct _timeb startTime;
void sal_initGlobalTimer()
{
    _ftime( &startTime );
}

sal_uInt32 SAL_CALL osl_getGlobalTimer(void)
{
  struct _timeb currentTime;
  sal_uInt32 nSeconds;

  _ftime( &currentTime );

  nSeconds = static_cast<sal_uInt32>( currentTime.time - startTime.time );

  return ( nSeconds * 1000 ) + static_cast<long>( currentTime.millitm - startTime.millitm );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
