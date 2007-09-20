/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: time.h,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-20 15:06:37 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _OSL_TIME_H_
#define _OSL_TIME_H_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************/
/* TimeValue                                                                */
/****************************************************************************/

#ifdef SAL_W32
#   pragma pack(push, 8)
#elif defined(SAL_OS2)
#   pragma pack(push, 4)
#endif

/* Time since Jan-01-1970 */

typedef struct {
    sal_uInt32 Seconds;
    sal_uInt32 Nanosec;
} TimeValue;

#if defined( SAL_W32) ||  defined(SAL_OS2)
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
sal_Bool SAL_CALL osl_getSystemTime( TimeValue* pTimeVal );


/** Get the GMT from a TimeValue and fill a struct oslDateTime
    @param pTimeVal[in] TimeValue
    @param pDateTime[out] On success it receives a struct oslDateTime

    @return sal_False if any error occurs else sal_True.
*/
sal_Bool SAL_CALL osl_getDateTimeFromTimeValue( TimeValue* pTimeVal, oslDateTime* pDateTime );


/** Get the GMT from a oslDateTime and fill a TimeValue
    @param pDateTime[in] oslDateTime
    @param pTimeVal[out] On success it receives a TimeValue

    @return sal_False if any error occurs else sal_True.
*/
sal_Bool SAL_CALL osl_getTimeValueFromDateTime( oslDateTime* pDateTime, TimeValue* pTimeVal );


/** Convert GMT to local time
    @param pSystemTimeVal[in] system time to convert
    @param pLocalTimeVal[out] On success it receives the local time

    @return sal_False if any error occurs else sal_True.
*/
sal_Bool SAL_CALL osl_getLocalTimeFromSystemTime( TimeValue* pSystemTimeVal, TimeValue* pLocalTimeVal );


/** Convert local time to GMT
    @param pLocalTimeVal[in] local time to convert
    @param pSystemTimeVal[out] On success it receives the system time

    @return sal_False if any error occurs else sal_True.
*/
sal_Bool SAL_CALL osl_getSystemTimeFromLocalTime( TimeValue* pLocalTimeVal, TimeValue* pSystemTimeVal );


/** Get the value of the global timer
    @return current timer value in milli seconds
 */

sal_uInt32 SAL_CALL osl_getGlobalTimer(void);

#ifdef __cplusplus
}
#endif

#endif  /* _OSL_TIME_H_ */

