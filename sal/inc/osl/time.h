/*************************************************************************
 *
 *  $RCSfile: time.h,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-13 14:46:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _OSL_TIME_H_
#define _OSL_TIME_H_

#ifndef _OSL_TYPES_H_
#   include <osl/types.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************/
/* TimeValue                                                                */
/****************************************************************************/

/* The TimeValue was included from osl/types.h. Since types.h is going to be
    removed, the TimeValue declaration is now in this header. To prevent migration
    problems TimeValue is still in osl/types.h for the time being. As soon as
    osl/types has been removed the #ifndef _TIMEVALUE can be removed in this file
*/
#ifndef _TIMEVALUE
#define _TIMEVALUE

#ifdef SAL_W32
#   pragma pack(push, 8)
#elif defined(SAL_OS2)
#   pragma pack(1)
#endif

/* Time since Jan-01-1970 */

typedef struct {
    sal_uInt32 Seconds;
    sal_uInt32 Nanosec;
} TimeValue;

#ifdef SAL_W32
#   pragma pack(pop)
#elif defined(SAL_OS2)
#   pragma pack()
#endif

#endif /* define _TIMEVALUE */

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

sal_uInt32 SAL_CALL osl_getGlobalTimer();

#ifdef __cplusplus
}
#endif

#endif  /* _OSL_TIME_H_ */

