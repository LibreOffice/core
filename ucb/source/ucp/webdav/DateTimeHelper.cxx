/*************************************************************************
 *
 *  $RCSfile: DateTimeHelper.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kso $ $Date: 2000-10-16 14:55:20 $
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
#include <stdio.h>

#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif

#include "DateTimeHelper.hxx"

using namespace com::sun::star::util;
using namespace rtl;

using namespace webdav_ucp;

bool DateTimeHelper::ISO8601_To_DateTime (const OUString& s,
    DateTime& dateTime)
{
    const char* _s = OUStringToOString (s, RTL_TEXTENCODING_ASCII_US).getStr ();

    sal_Int32 year;
    sal_Int32 month;
    sal_Int32 day;
    sal_Int32 hours;
    sal_Int32 minutes;
    sal_Int32 seconds;

    sal_Int32 i = sscanf (_s, "%4d-%2d-%2dT%2d:%2d:%2d",
        &year, &month, &day,
        &hours, &minutes, &seconds);
    if (i != 6)
        return false;

    dateTime.Year = year;
    dateTime.Month = month;
    dateTime.Day = day;
    dateTime.Hours = hours;
    dateTime.Minutes = minutes;
    dateTime.Seconds = seconds;

    return true;
}

sal_Int32 DateTimeHelper::convertMonthToInt (const OUString& day)
{
    if (day.compareToAscii ("Jan") == 0)
        return 1;
    else if (day.compareToAscii ("Feb") == 0)
        return 2;
    else if (day.compareToAscii ("Mar") == 0)
        return 3;
    else if (day.compareToAscii ("Apr") == 0)
        return 4;
    else if (day.compareToAscii ("May") == 0)
        return 5;
    else if (day.compareToAscii ("Jun") == 0)
        return 6;
    else if (day.compareToAscii ("Jul") == 0)
        return 7;
    else if (day.compareToAscii ("Aug") == 0)
        return 8;
    else if (day.compareToAscii ("Sep") == 0)
        return 9;
    else if (day.compareToAscii ("Oct") == 0)
        return 10;
    else if (day.compareToAscii ("Nov") == 0)
        return 11;
    else if (day.compareToAscii ("Dec") == 0)
        return 12;
    else
        return 0;
}

bool DateTimeHelper::RFC2068_To_DateTime (const OUString& s,
    DateTime& dateTime)
{
    sal_Int32 year;
    sal_Int32 month;
    sal_Int32 day;
    sal_Int32 hours;
    sal_Int32 minutes;
    sal_Int32 seconds;
    sal_Char string_month[3 + 1];

    sal_Int32 found = s.indexOf (',');
    if (found != -1)
    {
        OUString _s = s.copy (found);
        const sal_Char* __s =
            OUStringToOString (_s, RTL_TEXTENCODING_ASCII_US).getStr ();

        // RFC 1123
        found = sscanf (__s, ", %2d %3s %4d %2d:%2d:%2d GMT",
            &day, string_month, &year,
            &hours, &minutes, &seconds);
        if (found != 6)
        {
            // RFC 1036
            found = sscanf (__s, ", %2d-%3s-%2d %2d:%2d:%2d GMT",
                &day, &string_month, &year,
                &hours, &minutes, &seconds);
        }
        found = (found == 6) ? 1 : 0;
    }
    else
    {
        char string_day[3 + 1];

        const sal_Char* _s =
            OUStringToOString (s, RTL_TEXTENCODING_ASCII_US).getStr ();

        // ANSI C's asctime () format
        found = sscanf (_s, "%3s %3s %d %2d:%2d:%2d %4d",
            string_day, string_month,
            &day,
            &hours, &minutes, &seconds,
            &year);
        found = (found == 7) ? 1 : 0;
    }

    if (found)
    {
        month = DateTimeHelper::convertMonthToInt (OUString::createFromAscii (string_month));
        if (month)
        {
            dateTime.Year = year;
            dateTime.Month = month;
            dateTime.Day = day;
            dateTime.Hours = hours;
            dateTime.Minutes = minutes;
            dateTime.Seconds = seconds;
        }
        else
            found = 0;
    }

    return (found) ? true : false;
}

bool DateTimeHelper::convert (const OUString& s, DateTime& dateTime)
{
    if (ISO8601_To_DateTime (s, dateTime))
        return true;
    else if (RFC2068_To_DateTime (s, dateTime))
        return true;
    else
        return false;
}

