/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: iso8601_converter.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-02 09:45:13 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_shell.hxx"

#ifndef ISO8601_CONVERTER_HXX_INCLUDED
#include "internal/iso8601_converter.hxx"
#endif

#ifndef UTILITIES_HXX_INCLUDED
#include "internal/utilities.hxx"
#endif

#include <sstream>
#include <iomanip>

//-----------------------------------
/* Converts ISO 8601 conform date/time
   represenation to the representation
   conforming to the current locale
*/
std::wstring iso8601_date_to_local_date(const std::wstring& isoDate )
{
    const std::wstring CONST_SPACE(L" ");
    ::std::wstring ws8601DateTime(isoDate);

    if ( ws8601DateTime.length() == 19 )
    {
        //fill in the SYSTEMTIME structure;
        std::string asDateTime = WStringToString( ws8601DateTime );
        SYSTEMTIME DateTime;
        DateTime.wYear         = ( unsigned short )strtol( asDateTime.substr( 0, 4 ).c_str(), NULL, 10 );
        DateTime.wMonth        = ( unsigned short )strtol( asDateTime.substr( 5, 2 ).c_str(), NULL, 10 );
        DateTime.wDayOfWeek    =  0;
        DateTime.wDay          = ( unsigned short )strtol( asDateTime.substr( 8, 2 ).c_str(), NULL, 10 );
        DateTime.wHour         = ( unsigned short )strtol( asDateTime.substr( 11,2 ).c_str(), NULL, 10 );
        DateTime.wMinute       = ( unsigned short )strtol( asDateTime.substr( 14,2 ).c_str(), NULL, 10 );
        DateTime.wSecond       = ( unsigned short )strtol( asDateTime.substr( 17,2 ).c_str(), NULL, 10 );
        DateTime.wMilliseconds =  0;

        //get Date info from structure
        WCHAR DateBuffer[ MAX_PATH ];
        int DateSize = GetDateFormatW(
            LOCALE_SYSTEM_DEFAULT,
            0,
            &DateTime,
            NULL,
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
            NULL,
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

//------------------------------------
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

#if defined(_MSC_VER) && defined(_M_X64)
    oss << std::setw(2) << std::setfill(wchar_t('0')) << hours   << L":" <<
           std::setw(2) << std::setfill(wchar_t('0')) << minutes << L":" <<
           std::setw(2) << std::setfill(wchar_t('0')) << seconds;
#else
    oss << std::setw(2) << std::setfill('0') << hours   << L":" <<
           std::setw(2) << std::setfill('0') << minutes << L":" <<
           std::setw(2) << std::setfill('0') << seconds;
#endif
    return oss.str();
}

