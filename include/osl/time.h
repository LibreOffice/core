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

#ifndef INCLUDED_OSL_TIME_H
#define INCLUDED_OSL_TIME_H

#include "sal/config.h"

#if defined LIBO_INTERNAL_ONLY && defined __cplusplus
#include <chrono>
#endif

#include "sal/saldllapi.h"
#include "sal/types.h"

#ifdef _WIN32
#   pragma pack(push, 8)
#endif

/* Time since Jan-01-1970 */

#if defined LIBO_INTERNAL_ONLY && defined __cplusplus

struct TimeValue {
    TimeValue() = default;

    constexpr TimeValue(sal_uInt32 seconds, sal_uInt32 nanoseconds):
        Seconds(seconds), Nanosec(nanoseconds) {}

    template<typename Rep, typename Period> constexpr
    TimeValue(std::chrono::duration<Rep, Period> const & duration):
        Seconds(
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                duration).count() / 1000000000),
        Nanosec(
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                duration).count() % 1000000000)
    {}

    sal_uInt32 Seconds;
    sal_uInt32 Nanosec;
};

#else

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    sal_uInt32 Seconds;
    sal_uInt32 Nanosec;
} TimeValue;

#ifdef __cplusplus
}
#endif

#endif

#if defined(_WIN32)
#   pragma pack(pop)
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _oslDateTime
{
    /** contains the nanoseconds
    */
    sal_uInt32 NanoSeconds;

    /** contains the seconds (0-59).
    */
    sal_uInt16 Seconds;

    /** contains the minutes (0-59).
    */
    sal_uInt16 Minutes;

    /** contains the hour (0-23).
    */
    sal_uInt16 Hours;

    /** is the day of month (1-31).
    */
    sal_uInt16 Day;

    /** is the day of week (0-6 , 0 : Sunday).
    */
    sal_uInt16 DayOfWeek;

    /** is the month of year (1-12).
    */
    sal_uInt16 Month;

    /** is the year.
    */
    sal_Int16 Year;

} oslDateTime;


/** Get the current system time as TimeValue.
    @retval false if any error occurs.
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_getSystemTime(
        TimeValue* pTimeVal );


/** Get the GMT from a TimeValue and fill a struct oslDateTime
    @param[in] pTimeVal TimeValue
    @param[out] pDateTime On success it receives a struct oslDateTime

    @return sal_False if any error occurs else sal_True.
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_getDateTimeFromTimeValue(
        const TimeValue* pTimeVal, oslDateTime* pDateTime );


/** Get the GMT from a oslDateTime and fill a TimeValue
    @param[in] pDateTime oslDateTime
    @param[out] pTimeVal On success it receives a TimeValue

    @return sal_False if any error occurs else sal_True.
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_getTimeValueFromDateTime(
        const oslDateTime* pDateTime, TimeValue* pTimeVal );


/** Convert GMT to local time
    @param[in] pSystemTimeVal system time to convert
    @param[out] pLocalTimeVal On success it receives the local time

    @return sal_False if any error occurs else sal_True.
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_getLocalTimeFromSystemTime(
        const TimeValue* pSystemTimeVal, TimeValue* pLocalTimeVal );


/** Convert local time to GMT
    @param[in] pLocalTimeVal local time to convert
    @param[out] pSystemTimeVal On success it receives the system time

    @return sal_False if any error occurs else sal_True.
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_getSystemTimeFromLocalTime(
        const TimeValue* pLocalTimeVal, TimeValue* pSystemTimeVal );


/** Get the value of the global timer
    @return current timer value in milli seconds
 */

SAL_DLLPUBLIC sal_uInt32 SAL_CALL osl_getGlobalTimer(void);

#ifdef __cplusplus
}
#endif

#endif // INCLUDED_OSL_TIME_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
