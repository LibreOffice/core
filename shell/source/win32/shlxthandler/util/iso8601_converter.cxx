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

#include "sal/config.h"

#include <stdlib.h>

#include "iso8601_converter.hxx"
#include "utilities.hxx"

#include <sstream>
#include <iomanip>


/* Converts ISO 8601 conform date/time
   represenation to the representation
   conforming to the current locale
*/
std::wstring iso8601_date_to_local_date(const std::wstring& isoDate )
{
    ::std::wstring ws8601DateTime(isoDate);

    // Get rid of the optional milliseconds part if it exists.
    // Function accepts date/time as a combined date/time string in extended ISO8601 format,
    // which is yyyy-mm-ddThh:mm:ss[.mmm]. Last part is the optional "fraction of second" part,
    // that's why we cut off at 19.
    if (ws8601DateTime.length() > 19)
    {
      ws8601DateTime.erase(19, ::std::basic_string<char>::npos);
    }

    if ( ws8601DateTime.length() == 19 )
    {
        std::string asDateTime = WStringToString( ws8601DateTime );
        SYSTEMTIME DateTime;
        DateTime.wYear         = ( unsigned short )strtol( asDateTime.substr( 0, 4 ).c_str(), nullptr, 10 );
        DateTime.wMonth        = ( unsigned short )strtol( asDateTime.substr( 5, 2 ).c_str(), nullptr, 10 );
        DateTime.wDayOfWeek    =  0;
        DateTime.wDay          = ( unsigned short )strtol( asDateTime.substr( 8, 2 ).c_str(), nullptr, 10 );
        DateTime.wHour         = ( unsigned short )strtol( asDateTime.substr( 11,2 ).c_str(), nullptr, 10 );
        DateTime.wMinute       = ( unsigned short )strtol( asDateTime.substr( 14,2 ).c_str(), nullptr, 10 );
        DateTime.wSecond       = ( unsigned short )strtol( asDateTime.substr( 17,2 ).c_str(), nullptr, 10 );
        DateTime.wMilliseconds =  0;

        //get Date info from structure
        WCHAR DateBuffer[ MAX_PATH ];
        int DateSize = GetDateFormatW(
            LOCALE_SYSTEM_DEFAULT,
            0,
            &DateTime,
            nullptr,
            DateBuffer,
            MAX_PATH );

        if ( DateSize )
            ws8601DateTime.assign(DateBuffer);
        else
            ws8601DateTime = StringToWString( asDateTime );

        //get Time info from structure
        WCHAR TimeBuffer[ MAX_PATH ];

        int TimeSize =  GetTimeFormatW(
            LOCALE_SYSTEM_DEFAULT,
            0,
            &DateTime,
            nullptr,
            TimeBuffer,
            MAX_PATH );

        if ( TimeSize )
        {
            ws8601DateTime.append(L" ");
            ws8601DateTime.append(TimeBuffer);
        }
        else
            ws8601DateTime = StringToWString( asDateTime );
    }

    return ws8601DateTime;
}


/* Converts ISO 8601 conform duration
   representation to the representation
   conforming to the current locale

   Expect format PTnHnMnS according to
   ISO 8601 where n is abitrary number
   of digits
*/

std::wstring iso8601_duration_to_local_duration(const std::wstring& iso8601duration)
{
    std::wstring days;
    std::wstring hours;
    std::wstring minutes;
    std::wstring seconds;

    std::wstring::const_iterator iter     = iso8601duration.begin();
    std::wstring::const_iterator iter_end = iso8601duration.end();

    std::wstring num;

    for (/**/; iter != iter_end; ++iter)
    {
        if (isdigit(*iter))
        {
            num += *iter;
        }
        else
        {
            if (*iter == L'D' || *iter == L'd')
                days = num;
            else if (*iter == L'H' || *iter == L'h')
                hours = num;
            else if (*iter == L'M' || *iter == L'm')
                minutes = num;
            else if (*iter == L'S' || *iter == L's')
                seconds = num;

            num.clear();
        }
    }

    if (days.length() > 0)
    {
        int h = ((_wtoi(days.c_str()) * 24) + _wtoi(hours.c_str()));
        wchar_t buff[10];
        _itow(h, buff, 10);
        hours = buff;
    }

    std::wostringstream oss;
    oss << std::setw(2) << std::setfill(wchar_t('0')) << hours   << L":" <<
           std::setw(2) << std::setfill(wchar_t('0')) << minutes << L":" <<
           std::setw(2) << std::setfill(wchar_t('0')) << seconds;
    return oss.str();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
