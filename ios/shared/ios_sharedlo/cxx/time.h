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

#ifndef _OSL_TIME_H_
#define _OSL_TIME_H_

#include "sal/config.h"

#include "sal/saldllapi.h"
#include "sal/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************/
/* TimeValue                                                                */
/****************************************************************************/

#ifdef SAL_W32
#   pragma pack(push, 8)
#endif

/* Time since Jan-01-1970 */

typedef struct {
    sal_uInt32 Seconds;
    sal_uInt32 Nanosec;
} TimeValueX;

#if defined(SAL_W32)
#   pragma pack(pop)
#endif


/****************************************************************************/
/* oslDateTime */
/****************************************************************************/

typedef struct _oslDateTime
{
    /*----------------------------------------------------------------------*/
    /** contains the nanoseconds .
    */
    sal_uInt32 NanoSeconds;

    /** contains the seconds (0-59).
    */
    sal_uInt16 Seconds;

    /*----------------------------------------------------------------------*/
    /** contains the minutes (0-59).
    */
    sal_uInt16 Minutes;

    /*----------------------------------------------------------------------*/
    /** contains the hour (0-23).
    */
    sal_uInt16 Hours;

    /*----------------------------------------------------------------------*/
    /** is the day of month (1-31).
    */
    sal_uInt16 Day;

    /*----------------------------------------------------------------------*/
    /** is the day of week (0-6 , 0 : Sunday).
    */
    sal_uInt16 DayOfWeek;

    /*----------------------------------------------------------------------*/
    /** is the month of year (1-12).
    */
    sal_uInt16 Month;

    /*----------------------------------------------------------------------*/
    /** is the year.
    */
    sal_uInt16 Year;

} oslDateTime;


/** Get the current system time as TimeValue.
    @return false if any error occurs.
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_getSystemTime(
        TimeValue* pTimeVal );


/** Get the GMT from a TimeValue and fill a struct oslDateTime
    @param[in] pTimeVal TimeValue
    @param[out] pDateTime On success it receives a struct oslDateTime

    @return sal_False if any error occurs else sal_True.
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_getDateTimeFromTimeValue(
        TimeValue* pTimeVal, oslDateTime* pDateTime );


/** Get the GMT from a oslDateTime and fill a TimeValue
    @param[in] pDateTime oslDateTime
    @param[out] pTimeVal On success it receives a TimeValue

    @return sal_False if any error occurs else sal_True.
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_getTimeValueFromDateTime(
        oslDateTime* pDateTime, TimeValue* pTimeVal );


/** Convert GMT to local time
    @param[in] pSystemTimeVal system time to convert
    @param[out] pLocalTimeVal On success it receives the local time

    @return sal_False if any error occurs else sal_True.
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_getLocalTimeFromSystemTime(
        TimeValue* pSystemTimeVal, TimeValue* pLocalTimeVal );


/** Convert local time to GMT
    @param[in] pLocalTimeVal local time to convert
    @param[out] pSystemTimeVal On success it receives the system time

    @return sal_False if any error occurs else sal_True.
*/
SAL_DLLPUBLIC sal_Bool SAL_CALL osl_getSystemTimeFromLocalTime(
        TimeValue* pLocalTimeVal, TimeValue* pSystemTimeVal );


/** Get the value of the global timer
    @return current timer value in milli seconds
 */

SAL_DLLPUBLIC sal_uInt32 SAL_CALL osl_getGlobalTimer(void);

#ifdef __cplusplus
}
#endif

#endif  /* _OSL_TIME_H_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
