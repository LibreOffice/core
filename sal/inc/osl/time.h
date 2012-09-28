/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
} TimeValue;

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
