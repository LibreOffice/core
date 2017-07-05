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

#include <filetime.hxx>
#include <time.hxx>

#include <osl/diagnose.h>
#include <osl/time.h>
#include <sys/timeb.h>

// osl_getSystemTime

sal_Bool SAL_CALL osl_getSystemTime(TimeValue* pTimeVal)
{
    SYSTEMTIME SystemTime;
    FILETIME   CurTime, OffTime;
    __int64    Value;

    typedef VOID (WINAPI *GetSystemTimePreciseAsFileTime_PROC)(LPFILETIME);

    static HMODULE hModule = nullptr;
    static GetSystemTimePreciseAsFileTime_PROC pGetSystemTimePreciseAsFileTime = nullptr;

    assert(pTimeVal);

    if ( !hModule )
    {
        hModule = GetModuleHandleA( "Kernel32.dll" );
        if ( hModule )
            pGetSystemTimePreciseAsFileTime = reinterpret_cast<GetSystemTimePreciseAsFileTime_PROC>(
                GetProcAddress(hModule, "GetSystemTimePreciseAsFileTime"));
    }

    // use ~1 microsecond resolution if available
    if (pGetSystemTimePreciseAsFileTime)
        pGetSystemTimePreciseAsFileTime(&CurTime);
    else
    {
        GetSystemTime(&SystemTime);
        SystemTimeToFileTime(&SystemTime, &CurTime);
    }

    SystemTime.wYear         = 1970;
    SystemTime.wMonth        = 1;
    SystemTime.wDayOfWeek    = 0;
    SystemTime.wDay          = 1;
    SystemTime.wHour         = 0;
    SystemTime.wMinute       = 0;
    SystemTime.wSecond       = 0;
    SystemTime.wMilliseconds = 0;

    SystemTimeToFileTime(&SystemTime, &OffTime);

    Value = *reinterpret_cast<__int64 *>(&CurTime) - *reinterpret_cast<__int64 *>(&OffTime);

    pTimeVal->Seconds  = (unsigned long) (Value / 10000000L);
    pTimeVal->Nanosec  = (unsigned long)((Value % 10000000L) * 100);

    return true;
}

// osl_getDateTimeFromTimeValue

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

// osl_getTimeValueFromDateTime

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

// osl_getLocalTimeFromSystemTime

sal_Bool SAL_CALL osl_getLocalTimeFromSystemTime( const TimeValue* pSystemTimeVal, TimeValue* pLocalTimeVal )
{
    TIME_ZONE_INFORMATION aTimeZoneInformation;
    DWORD Success;
    sal_Int64   bias;

    // get timezone information
    if ( ( Success=GetTimeZoneInformation( &aTimeZoneInformation ) ) != TIME_ZONE_ID_INVALID)
    {
        bias=aTimeZoneInformation.Bias;

        // add bias for daylight saving time
        if ( Success== TIME_ZONE_ID_DAYLIGHT )
            bias+=aTimeZoneInformation.DaylightBias;

        if ( (sal_Int64) pSystemTimeVal->Seconds > ( bias * 60 ) )
        {
            pLocalTimeVal->Seconds = (sal_uInt32) (pSystemTimeVal->Seconds - ( bias * 60) );
            pLocalTimeVal->Nanosec = pSystemTimeVal->Nanosec;

            return true;
        }
    }

    return false;
}

// osl_getSystemTimeFromLocalTime

sal_Bool SAL_CALL osl_getSystemTimeFromLocalTime( const TimeValue* pLocalTimeVal, TimeValue* pSystemTimeVal )
{
    TIME_ZONE_INFORMATION aTimeZoneInformation;
    DWORD Success;
    sal_Int64   bias;

    // get timezone information
    if ( ( Success=GetTimeZoneInformation( &aTimeZoneInformation ) ) != TIME_ZONE_ID_INVALID)
    {
        bias=aTimeZoneInformation.Bias;

        // add bias for daylight saving time
        if ( Success== TIME_ZONE_ID_DAYLIGHT )
            bias+=aTimeZoneInformation.DaylightBias;

        if ( (sal_Int64) pLocalTimeVal->Seconds + ( bias * 60 ) > 0 )
        {
            pSystemTimeVal->Seconds = (sal_uInt32) ( pLocalTimeVal->Seconds + ( bias * 60) );
            pSystemTimeVal->Nanosec = pLocalTimeVal->Nanosec;

            return true;
        }
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

  nSeconds = (sal_uInt32)( currentTime.time - startTime.time );

  return ( nSeconds * 1000 ) + (long)( currentTime.millitm - startTime.millitm );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
