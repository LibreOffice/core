/*************************************************************************
 *
 *  $RCSfile: time.c,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: mfe $ $Date: 2001-02-27 15:49:07 $
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


#include "system.h"

#include <osl/diagnose.h>
#include <osl/time.h>
#include <sys/timeb.h>


sal_Bool SAL_CALL osl_getSystemTime(TimeValue* TimeValue)
{
    struct timeval tp;

    gettimeofday(&tp, NULL);

    TimeValue->Seconds = tp.tv_sec;
    TimeValue->Nanosec = tp.tv_usec * 1000;

    return (sal_True);
}


/*--------------------------------------------------
 * osl_getDateTimeFromTimeValue
 *-------------------------------------------------*/

sal_Bool SAL_CALL osl_getDateTimeFromTimeValue( TimeValue* pTimeVal, oslDateTime* pDateTime )
{
    struct tm   *pSystemTime;
    time_t      atime;

    atime = (time_t)pTimeVal->Seconds;

    /* Convert time from type time_t to struct tm */
    pSystemTime = gmtime( &atime );


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

sal_Bool SAL_CALL osl_getTimeValueFromDateTime( oslDateTime* pDateTime, TimeValue* pTimeVal )
{
    struct tm   aSystemTime;
    time_t      aSeconds;

    /* Convert struct oslDateTime to struct tm */
    aSystemTime.tm_sec          =   pDateTime->Seconds;
    aSystemTime.tm_min          =   pDateTime->Minutes;
    aSystemTime.tm_hour         =   pDateTime->Hours;
    aSystemTime.tm_mday         =   pDateTime->Day;
    aSystemTime.tm_wday         =   pDateTime->DayOfWeek;

    if ( pDateTime->Month > 1)
        aSystemTime.tm_mon      =   pDateTime->Month - 1;
    else
        return sal_False;

    if ( pDateTime->Year >= 1900)
        aSystemTime.tm_year     =   pDateTime->Year - 1900;
    else
        return sal_False;

    aSystemTime.tm_isdst        =   0;
    aSystemTime.tm_wday         =   0;
    aSystemTime.tm_yday         =   0;

    /* Convert time to calendar value */
    aSeconds = mktime( &aSystemTime );

    if (aSeconds != (time_t) -1)
    {
        tzset();

        /* timezone corrections */
#if defined(MACOSX) || defined(FREEBSD) || defined(NETBSD)
        if ( (sal_Int64) aSeconds > (aSystemTime.tm_gmtoff * -1) )
            aSeconds += aSystemTime.tm_gmtoff;
#else
        if ( (sal_Int64) aSeconds > timezone )
            aSeconds -= timezone;
#endif
        else
            return sal_False;

        pTimeVal->Seconds = aSeconds;
        pTimeVal->Nanosec = pDateTime->NanoSeconds;

        return sal_True;
    }

    return sal_False;
}


/*--------------------------------------------------
 * osl_getLocalTimeFromSystemTime
 *--------------------------------------------------*/

sal_Bool SAL_CALL osl_getLocalTimeFromSystemTime( TimeValue* pSystemTimeVal, TimeValue* pLocalTimeVal )
{
    sal_Int64   bias;
    struct tm   *pLocalTime;
    time_t      atime;

    atime = (time_t)pSystemTimeVal->Seconds;

    tzset();

    /* timezone an daylight saving time */
#if defined(MACOSX) || defined(FREEBSD) || defined(NETBSD)
    pLocalTime = localtime( &atime );
    bias = pLocalTime->tm_gmtoff * -1;
#else
    if ( daylight != 0)
        bias = timezone - 3600;
    else
        bias = timezone;
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

sal_Bool SAL_CALL osl_getSystemTimeFromLocalTime( TimeValue* pLocalTimeVal, TimeValue* pSystemTimeVal )
{
    sal_Int64   bias;
    struct tm   *pLocalTime;
    time_t      atime;

    atime = (time_t)pLocalTimeVal->Seconds;

    tzset();

    /* timezone an daylight saving time */
#if defined(MACOSX) || defined(FREEBSD) || defined(NETBSD)
    /* Convert atime, which is a local time, to it's GMT equivalent. Then, get
     * the timezone offset for the local time for the GMT equivalent time. Note
     * that we cannot directly use local time to determine the timezone offset
     * because GMT is the only reliable time that we can determine timezone
     * offset from.
     */
    atime = mktime( gmtime( &atime ) );
    pLocalTime = localtime( &atime );
    bias = pLocalTime->tm_gmtoff * -1;
#else
    if ( daylight != 0)
        bias = timezone - 3600;
    else
        bias = timezone;
#endif

    if ( (sal_Int64) pSystemTimeVal->Seconds + bias > 0)
    {
        pSystemTimeVal->Seconds = pLocalTimeVal->Seconds + bias;
        pSystemTimeVal->Nanosec = pLocalTimeVal->Nanosec;

        return sal_True;
    }

    return sal_False;
}



static struct timeb startTime;
static sal_Bool bGlobalTimer = sal_False;

sal_uInt32 SAL_CALL osl_getGlobalTimer()
{
  struct timeb currentTime;
  sal_uInt32 nSeconds;

  if ( bGlobalTimer == sal_False )
  {
      ftime( &startTime );
      bGlobalTimer=sal_True;
  }

  ftime( &currentTime );

  nSeconds = (sal_uInt32)( currentTime.time - startTime.time );

  return ( nSeconds * 1000 ) + (long)( currentTime.millitm - startTime.millitm );
}
