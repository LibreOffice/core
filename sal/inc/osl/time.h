/*************************************************************************
 *
 *  $RCSfile: time.h,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:17:13 $
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
/* oslDateTime */
/****************************************************************************/

typedef struct _oslDateTime
{
    //-------------------------------------------------------------------------
    /** contains the nanoseconds .
    */
    sal_uInt32 NanoSeconds;

    //-------------------------------------------------------------------------
    /** contains the seconds (0-59).
    */
    sal_uInt16 Seconds;

    //-------------------------------------------------------------------------
    /** contains the minutes (0-59).
    */
    sal_uInt16 Minutes;

    //-------------------------------------------------------------------------
    /** contains the hour (0-23).
    */
    sal_uInt16 Hours;

    //-------------------------------------------------------------------------
    /** is the day of month (1-31).
    */
    sal_uInt16 Day;

    //-------------------------------------------------------------------------
    /** is the day of week (0-6 , 0 : Sunday).
    */
    sal_uInt16 DayOfWeek;

    //-------------------------------------------------------------------------
    /** is the month of year (1-12).
    */
    sal_uInt16 Month;

    //-------------------------------------------------------------------------
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


#ifdef __cplusplus
}
#endif

#endif  /* _OSL_TIME_H_ */

/*************************************************************************
*
*    $Log: not supported by cvs2svn $
*    Revision 1.6  2000/09/18 14:28:49  willem.vandorp
*    OpenOffice header added.
*
*    Revision 1.5  2000/08/18 14:01:05  rs
*    new time functions
*
*    Revision 1.4  2000/07/26 15:12:52  rs
*    DateTime functions
*
*    Revision 1.3  2000/05/17 14:50:15  mfe
*    comments upgraded
*
*    Revision 1.2  1999/10/27 15:02:09  mfe
*    Change of Copyright, removed compiler warnings, code clean up, ...
*
*    Revision 1.1  1999/08/05 10:18:20  jsc
*    verschoben aus osl
*
*    Revision 1.7  1999/08/05 11:14:44  jsc
*    verschoben in root inc-Verzeichnis
*
*    Revision 1.6  1999/01/20 18:53:41  jsc
*    #61011# Typumstellung
*
*    Revision 1.5  1998/03/13 15:07:33  rh
*    Cleanup of enum chaos and implemntation of pipes
*
*    Revision 1.4  1998/02/16 19:34:52  rh
*    Cleanup of ports, integration of Size_t, features for process
*
*    Revision 1.3  1997/08/22 14:47:38  rh
*    TimeValue inserted
*
*    Revision 1.2  1997/07/23 07:07:02  rh
*    Change Micor to Nano-seconds
*
*    Revision 1.1  1997/07/21 19:13:13  rh
*    getSystemTime added
*
*************************************************************************/
