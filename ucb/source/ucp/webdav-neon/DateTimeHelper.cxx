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


#include <stdio.h>
#include <osl/time.h>
#include <com/sun/star/util/DateTime.hpp>
#include "DateTimeHelper.hxx"

using namespace com::sun::star::util;

using namespace webdav_ucp;

using ::rtl::OUString;
using ::rtl::OString;

bool DateTimeHelper::ISO8601_To_DateTime (const OUString& s,
    DateTime& dateTime)
{
    OString aDT (s.getStr(), s.getLength(), RTL_TEXTENCODING_ASCII_US);

    int year, month, day, hours, minutes, off_hours, off_minutes, fix;
    double seconds;

    // 2001-01-01T12:30:00Z
    int n = sscanf( aDT.getStr(), "%04d-%02d-%02dT%02d:%02d:%lfZ",
                    &year, &month, &day, &hours, &minutes, &seconds );
    if ( n == 6 )
    {
        fix = 0;
    }
    else
    {
        // 2001-01-01T12:30:00+03:30
        n = sscanf( aDT.getStr(), "%04d-%02d-%02dT%02d:%02d:%lf+%02d:%02d",
                    &year, &month, &day, &hours, &minutes, &seconds,
                    &off_hours, &off_minutes );
        if ( n == 8 )
        {
            fix = - off_hours * 3600 - off_minutes * 60;
        }
        else
        {
            // 2001-01-01T12:30:00-03:30
            n = sscanf( aDT.getStr(), "%04d-%02d-%02dT%02d:%02d:%lf-%02d:%02d",
                        &year, &month, &day, &hours, &minutes, &seconds,
                        &off_hours, &off_minutes );
            if ( n == 8 )
            {
                fix = off_hours * 3600 + off_minutes * 60;
            }
            else
            {
                return false;
            }
        }
    }

    // Convert to local time...

    oslDateTime aDateTime;
    aDateTime.NanoSeconds = 0;
    aDateTime.Seconds     = sal::static_int_cast< sal_uInt16 >(seconds); // 0-59
    aDateTime.Minutes     = sal::static_int_cast< sal_uInt16 >(minutes); // 0-59
    aDateTime.Hours       = sal::static_int_cast< sal_uInt16 >(hours); // 0-23
    aDateTime.Day         = sal::static_int_cast< sal_uInt16 >(day); // 1-31
    aDateTime.DayOfWeek   = 0;          // 0-6, 0 = Sunday
    aDateTime.Month       = sal::static_int_cast< sal_uInt16 >(month); // 1-12
    aDateTime.Year        = sal::static_int_cast< sal_uInt16 >(year);

    TimeValue aTimeValue;
    if ( osl_getTimeValueFromDateTime( &aDateTime, &aTimeValue ) )
    {
        aTimeValue.Seconds += fix;

        if ( osl_getLocalTimeFromSystemTime( &aTimeValue, &aTimeValue ) )
        {
            if ( osl_getDateTimeFromTimeValue( &aTimeValue, &aDateTime ) )
            {
                dateTime.Year    = aDateTime.Year;
                dateTime.Month   = aDateTime.Month;
                dateTime.Day     = aDateTime.Day;
                dateTime.Hours   = aDateTime.Hours;
                dateTime.Minutes = aDateTime.Minutes;
                dateTime.Seconds = aDateTime.Seconds;

                return true;
             }
        }
    }

    return false;
}

sal_Int32 DateTimeHelper::convertMonthToInt (const OUString& month)
{
    if (month.compareToAscii ("Jan") == 0)
        return 1;
    else if (month.compareToAscii ("Feb") == 0)
        return 2;
    else if (month.compareToAscii ("Mar") == 0)
        return 3;
    else if (month.compareToAscii ("Apr") == 0)
        return 4;
    else if (month.compareToAscii ("May") == 0)
        return 5;
    else if (month.compareToAscii ("Jun") == 0)
        return 6;
    else if (month.compareToAscii ("Jul") == 0)
        return 7;
    else if (month.compareToAscii ("Aug") == 0)
        return 8;
    else if (month.compareToAscii ("Sep") == 0)
        return 9;
    else if (month.compareToAscii ("Oct") == 0)
        return 10;
    else if (month.compareToAscii ("Nov") == 0)
        return 11;
    else if (month.compareToAscii ("Dec") == 0)
        return 12;
    else
        return 0;
}

bool DateTimeHelper::RFC2068_To_DateTime (const OUString& s,
    DateTime& dateTime)
{
    int year;
    int day;
    int hours;
    int minutes;
    int seconds;
    sal_Char string_month[3 + 1];
    sal_Char string_day[3 + 1];

    sal_Int32 found = s.indexOf (',');
    if (found != -1)
    {
        OString aDT (s.getStr(), s.getLength(), RTL_TEXTENCODING_ASCII_US);

        // RFC 1123
        found = sscanf (aDT.getStr(), "%3s, %2d %3s %4d %2d:%2d:%2d GMT",
                        string_day, &day, string_month, &year, &hours, &minutes, &seconds);
        if (found != 7)
        {
            // RFC 1036
            found = sscanf (aDT.getStr(), "%3s, %2d-%3s-%2d %2d:%2d:%2d GMT",
                            string_day, &day, string_month, &year, &hours, &minutes, &seconds);
        }
        found = (found == 7) ? 1 : 0;
    }
    else
    {
        OString aDT (s.getStr(), s.getLength(), RTL_TEXTENCODING_ASCII_US);

        // ANSI C's asctime () format
        found = sscanf (aDT.getStr(), "%3s %3s %d %2d:%2d:%2d %4d",
                        string_day, string_month,
                        &day, &hours, &minutes, &seconds, &year);
        found = (found == 7) ? 1 : 0;
    }

    if (found)
    {
        found = 0;

        int month = DateTimeHelper::convertMonthToInt (
                            OUString::createFromAscii (string_month));
        if (month)
        {
            // Convert to local time...

            oslDateTime aDateTime;
            aDateTime.NanoSeconds = 0;
            aDateTime.Seconds     = sal::static_int_cast< sal_uInt16 >(seconds);
                // 0-59
            aDateTime.Minutes     = sal::static_int_cast< sal_uInt16 >(minutes);
                // 0-59
            aDateTime.Hours       = sal::static_int_cast< sal_uInt16 >(hours);
                // 0-23
            aDateTime.Day         = sal::static_int_cast< sal_uInt16 >(day);
                // 1-31
            aDateTime.DayOfWeek   = 0; //dayofweek;  // 0-6, 0 = Sunday
            aDateTime.Month       = sal::static_int_cast< sal_uInt16 >(month);
                // 1-12
            aDateTime.Year        = sal::static_int_cast< sal_uInt16 >(year);

            TimeValue aTimeValue;
            if ( osl_getTimeValueFromDateTime( &aDateTime,
                                                &aTimeValue ) )
            {
                if ( osl_getLocalTimeFromSystemTime( &aTimeValue,
                                                        &aTimeValue ) )
                {
                    if ( osl_getDateTimeFromTimeValue( &aTimeValue,
                                                        &aDateTime ) )
                    {
                        dateTime.Year    = aDateTime.Year;
                        dateTime.Month   = aDateTime.Month;
                        dateTime.Day     = aDateTime.Day;
                        dateTime.Hours   = aDateTime.Hours;
                        dateTime.Minutes = aDateTime.Minutes;
                        dateTime.Seconds = aDateTime.Seconds;

                        found = 1;
                    }
                }
            }
        }
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
