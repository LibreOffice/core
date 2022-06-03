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

#include <unotools/datetime.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/syslocale.hxx>
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <tools/datetime.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/math.hxx>
#include <osl/diagnose.h>
#include <comphelper/string.hxx>
#include <o3tl/string_view.hxx>
#include <cstddef>
#include <sstream>

namespace
{
    bool checkAllNumber(std::u16string_view rString)
    {
        sal_Int32 nPos = 0;
        sal_Int32 nLen = rString.size();

        // skip white space
        while( nPos < nLen && ' ' == rString[nPos] )
            nPos++;

        if( nPos < nLen && '-' == rString[nPos] )
            nPos++;

        // get number
        while( nPos < nLen &&
               '0' <= rString[nPos] &&
               '9' >= rString[nPos] )
        {
            nPos++;
        }

        return nPos == nLen;
    }

    /** convert string to number with optional min and max values */
    bool convertNumber32(sal_Int32& rValue,
                         std::u16string_view rString,
                         sal_Int32 /*nMin*/ = -1, sal_Int32 /*nMax*/ = -1)
    {
        if (!checkAllNumber(rString))
        {
            rValue = 0;
            return false;
        }

        rValue = o3tl::toInt32(rString);
        return true;
    }

    bool convertNumber64(sal_Int64& rValue,
                         std::u16string_view rString,
                         sal_Int64 /*nMin*/ = -1, sal_Int64 /*nMax*/ = -1)
    {
        if (!checkAllNumber(rString))
        {
            rValue = 0;
            return false;
        }

        rValue = o3tl::toInt64(rString);
        return true;
    }

    // although the standard calls for fixed-length (zero-padded) tokens
    // (in their integer part), we are here liberal and allow shorter tokens
    // (when there are separators, else it is ambiguous).
    // Note that:
    //   the token separator is OPTIONAL
    //   empty string is a valid token! (to recognise hh or hhmm or hh:mm formats)
    // returns: success / failure
    // in case of failure, no reference argument is changed
    // arguments:
    //   i_str: string to extract token from
    //   index: index in i_str where to start tokenizing
    //          after return, start of *next* token (if any)
    //          if this was the last token, then the value is UNDEFINED
    //   o_strInt:    output; integer part of token
    //   o_bFraction: output; was there a fractional part?
    //   o_strFrac:   output; fractional part of token
    bool impl_getISO8601TimeToken(std::u16string_view i_str, std::size_t &nPos, OUString &resInt, bool &bFraction, OUString &resFrac)
    {
        bFraction = false;
        // all tokens are of length 2
        const std::size_t nEndPos = nPos + 2;
        const sal_Unicode c0 = '0';
        const sal_Unicode c9 = '9';
        const sal_Unicode sep = ':';
        for (;nPos < nEndPos && nPos < i_str.size(); ++nPos)
        {
            const sal_Unicode c = i_str[nPos];
            if (c == sep)
                return true;
            if (c < c0 || c > c9)
                return false;
            resInt += OUStringChar(c);
        }
        if (nPos == 0)
            return false;
        if (nPos == i_str.size() || i_str[nPos] == sep)
            return true;
        if (i_str[nPos] == ',' || i_str[nPos] == '.')
        {
            bFraction = true;
            ++nPos;
            for (; nPos < i_str.size(); ++nPos)
            {
                const sal_Unicode c = i_str[nPos];
                if (c == 'Z' || c == '+' || c == '-')
                {
                    --nPos; // we don't want to skip the tz separator
                    return true;
                }
                if (c == sep)
                    // fractional part allowed only in *last* token
                    return false;
                if (c < c0 || c > c9)
                    return false;
                resFrac += OUStringChar(c);
            }
            OSL_ENSURE(nPos == i_str.size(), "impl_getISO8601TimeToken internal error; expected to be at end of string");
            return true;
        }
        if (i_str[nPos] == 'Z' || i_str[nPos] == '+' || i_str[nPos] == '-')
        {
            --nPos; // we don't want to skip the tz separator
            return true;
        }
        else
            return false;
    }
    bool getISO8601TimeToken(std::u16string_view i_str, std::size_t &io_index, OUString &o_strInt, bool &o_bFraction, OUString &o_strFrac)
    {
        OUString resInt;
        OUString resFrac;
        bool bFraction = false;
        std::size_t index = io_index;
        if(!impl_getISO8601TimeToken(i_str, index, resInt, bFraction, resFrac))
            return false;
        else
        {
            io_index = index+1;
            o_strInt = resInt;
            o_strFrac = resFrac;
            o_bFraction = bFraction;
            return true;
        }
    }
    bool getISO8601TimeZoneToken(std::u16string_view i_str, std::size_t &io_index, OUString &o_strInt)
    {
        const sal_Unicode c0 = '0';
        const sal_Unicode c9 = '9';
        const sal_Unicode sep = ':';
        if (i_str[io_index] == 'Z') // UTC timezone indicator
        {
            ++io_index;
            o_strInt = "Z";
            return true;
        }
        else if (i_str[io_index] == '+' || i_str[io_index] == '-') // other timezones indicator
        {
            ++io_index;
            o_strInt.clear();
            for (; io_index < i_str.size(); ++io_index)
            {
                const sal_Unicode c = i_str[io_index];
                if ((c < c0 || c > c9) && c != sep)
                    return false;
                o_strInt += OUStringChar(c);
            }
            return true;
        }
        else
            return false;
    }
}

namespace utl
{
const LocaleDataWrapper& GetLocaleData()
{
    static SvtSysLocale ourSysLocale;
    return ourSysLocale.GetLocaleData();
}

DateTime GetDateTime(const css::util::DateTime& _rDT) { return DateTime(_rDT); }

OUString GetDateTimeString(const css::util::DateTime& _rDT)
{
    // String with date and time information (#i20172#)
    DateTime aDT(GetDateTime(_rDT));
    const LocaleDataWrapper& rLoDa = GetLocaleData();

    return rLoDa.getDate(aDT) + " " + rLoDa.getTime(aDT);
}

OUString GetDateTimeString(sal_Int32 _nDate, sal_Int32 _nTime)
{
    const LocaleDataWrapper& rLoDa = GetLocaleData();

    Date aDate(_nDate);
    tools::Time aTime(_nTime * tools::Time::nanoPerCenti);
    return rLoDa.getDate(aDate) + ", " + rLoDa.getTime(aTime);
}

OUString GetDateString(const css::util::DateTime& _rDT)
{
    return GetLocaleData().getDate(GetDateTime(_rDT));
}

void typeConvert(const Date& _rDate, css::util::Date& _rOut)
{
    _rOut.Day = _rDate.GetDay();
    _rOut.Month = _rDate.GetMonth();
    _rOut.Year = _rDate.GetYear();
}

void typeConvert(const css::util::Date& _rDate, Date& _rOut)
{
    _rOut = Date(_rDate.Day, _rDate.Month, _rDate.Year);
}

void typeConvert(const DateTime& _rDateTime, css::util::DateTime& _rOut)
{
    _rOut.Year = _rDateTime.GetYear();
    _rOut.Month = _rDateTime.GetMonth();
    _rOut.Day = _rDateTime.GetDay();
    _rOut.Hours = _rDateTime.GetHour();
    _rOut.Minutes = _rDateTime.GetMin();
    _rOut.Seconds = _rDateTime.GetSec();
    _rOut.NanoSeconds = _rDateTime.GetNanoSec();
}

void typeConvert(const css::util::DateTime& _rDateTime, DateTime& _rOut)
{
    Date aDate(_rDateTime.Day, _rDateTime.Month, _rDateTime.Year);
    tools::Time aTime(_rDateTime.Hours, _rDateTime.Minutes, _rDateTime.Seconds, _rDateTime.NanoSeconds);
    _rOut = DateTime(aDate, aTime);
}

OUString toISO8601(const css::util::DateTime& rDateTime)
{
    OUStringBuffer rBuffer(32);
    rBuffer.append(static_cast<sal_Int32>(rDateTime.Year));
    rBuffer.append('-');
    if( rDateTime.Month < 10 )
        rBuffer.append('0');
    rBuffer.append(static_cast<sal_Int32>(rDateTime.Month));
    rBuffer.append('-');
    if( rDateTime.Day < 10 )
        rBuffer.append('0');
    rBuffer.append(static_cast<sal_Int32>(rDateTime.Day));

    if( rDateTime.NanoSeconds != 0 ||
        rDateTime.Seconds     != 0 ||
        rDateTime.Minutes     != 0 ||
        rDateTime.Hours       != 0 )
    {
        rBuffer.append('T');
        if( rDateTime.Hours < 10 )
            rBuffer.append('0');
        rBuffer.append(static_cast<sal_Int32>(rDateTime.Hours));
        rBuffer.append(':');
        if( rDateTime.Minutes < 10 )
            rBuffer.append('0');
        rBuffer.append(static_cast<sal_Int32>(rDateTime.Minutes));
        rBuffer.append(':');
        if( rDateTime.Seconds < 10 )
            rBuffer.append('0');
        rBuffer.append(static_cast<sal_Int32>(rDateTime.Seconds));
        if ( rDateTime.NanoSeconds > 0)
        {
            OSL_ENSURE(rDateTime.NanoSeconds < 1000000000,"NanoSeconds cannot be more than 999 999 999");
            rBuffer.append(',');
            std::ostringstream ostr;
            ostr.fill('0');
            ostr.width(9);
            ostr << rDateTime.NanoSeconds;
            rBuffer.append(OUString::createFromAscii(ostr.str().c_str()));
        }
    }
    return rBuffer.makeStringAndClear();
}

/** convert ISO8601 DateTime String to util::DateTime */
bool ISO8601parseDateTime(std::u16string_view rString, css::util::DateTime& rDateTime)
{
    bool bSuccess = true;

    std::u16string_view aDateStr, aTimeStr;
    css::util::Date aDate;
    css::util::Time aTime;
    size_t nPos = rString.find( 'T' );
    if ( nPos != std::u16string_view::npos )
    {
        aDateStr = rString.substr( 0, nPos );
        aTimeStr = rString.substr( nPos + 1 );
    }
    else
        aDateStr = rString;         // no separator: only date part

    bSuccess = ISO8601parseDate(aDateStr, aDate);

    if ( bSuccess && !aTimeStr.empty() )           // time is optional
    {
        bSuccess = ISO8601parseTime(aTimeStr, aTime);
    }

    if (bSuccess)
    {
        rDateTime = css::util::DateTime(aTime.NanoSeconds, aTime.Seconds, aTime.Minutes, aTime.Hours,
               aDate.Day, aDate.Month, aDate.Year, false);
    }

    return bSuccess;
}

/** convert ISO8601 Date String to util::Date */
// TODO: supports only calendar dates YYYY-MM-DD
// MISSING: calendar dates YYYYMMDD YYYY-MM
//          year, week date, ordinal date
bool ISO8601parseDate(std::u16string_view aDateStr, css::util::Date& rDate)
{
    const sal_Int32 nDateTokens {comphelper::string::getTokenCount(aDateStr, '-')};

    if (nDateTokens<1 || nDateTokens>3)
        return false;

    sal_Int32 nYear    = 1899;
    sal_Int32 nMonth   = 12;
    sal_Int32 nDay     = 30;

    sal_Int32 nIdx {0};
    if ( !convertNumber32( nYear, o3tl::getToken(aDateStr, 0, '-', nIdx ), 0, 9999 ) )
        return false;
    if ( nDateTokens >= 2 )
        if ( !convertNumber32( nMonth, o3tl::getToken(aDateStr, 0, '-', nIdx ), 0, 12 ) )
            return false;
    if ( nDateTokens >= 3 )
        if ( !convertNumber32( nDay, o3tl::getToken(aDateStr, 0, '-', nIdx ), 0, 31 ) )
            return false;

    rDate.Year = static_cast<sal_uInt16>(nYear);
    rDate.Month = static_cast<sal_uInt16>(nMonth);
    rDate.Day = static_cast<sal_uInt16>(nDay);

    return true;
}

/** convert ISO8601 Time String to util::Time */
bool ISO8601parseTime(std::u16string_view aTimeStr, css::util::Time& rTime)
{
    sal_Int32 nHour    = 0;
    sal_Int32 nMin     = 0;
    sal_Int32 nSec     = 0;
    sal_Int32 nNanoSec = 0;

    std::size_t n = 0;
    OUString tokInt;
    OUString tokFrac;
    OUString tokTz;
    bool bFrac = false;
    // hours
    bool bSuccess = getISO8601TimeToken(aTimeStr, n, tokInt, bFrac, tokFrac);
    if (!bSuccess)
        return false;

    if ( bFrac && n < aTimeStr.size())
    {
        // is it junk or the timezone?
        bSuccess = getISO8601TimeZoneToken(aTimeStr, n, tokTz);
        if (!bSuccess)
            return false;
    }
    bSuccess = convertNumber32( nHour, tokInt, 0, 23 );
    if (!bSuccess)
        return false;

    if (bFrac)
    {
        sal_Int64 fracNumerator;
        bSuccess = convertNumber64(fracNumerator, tokFrac);
        if ( bSuccess )
        {
            double frac = static_cast<double>(fracNumerator) / pow(static_cast<double>(10), static_cast<double>(tokFrac.getLength()));
             // minutes
            OSL_ENSURE(frac < 1 && frac >= 0, "ISO8601parse internal error frac hours (of hours) not between 0 and 1");
            frac *= 60;
            nMin = floor(frac);
            frac -=  nMin;
            // seconds
            OSL_ENSURE(frac < 1 && frac >= 0, "ISO8601parse internal error frac minutes (of hours) not between 0 and 1");
            frac *= 60;
            nSec = floor(frac);
            frac -=  nSec;
            // nanoseconds
            OSL_ENSURE(frac < 1 && frac >= 0, "ISO8601parse internal error frac seconds (of hours) not between 0 and 1");
            frac *= 1000000000;
            nNanoSec = ::rtl::math::round(frac);
        }
        goto end;
    }
    if(n >= aTimeStr.size())
        goto end;

    // minutes
    bSuccess = getISO8601TimeToken(aTimeStr, n, tokInt, bFrac, tokFrac);
    if (!bSuccess)
        return false;
    if ( bFrac && n < aTimeStr.size())
    {
        // is it junk or the timezone?
        bSuccess = getISO8601TimeZoneToken(aTimeStr, n, tokTz);
        if (!bSuccess)
            return false;
    }
    bSuccess = convertNumber32( nMin, tokInt, 0, 59 );
    if (!bSuccess)
        return false;
    if (bFrac)
    {
        sal_Int64 fracNumerator;
        bSuccess = convertNumber64(fracNumerator, tokFrac);
        if ( bSuccess )
        {
            double frac = static_cast<double>(fracNumerator) / pow(static_cast<double>(10), static_cast<double>(tokFrac.getLength()));
            // seconds
            OSL_ENSURE(frac < 1 && frac >= 0, "ISO8601parse internal error frac minutes (of minutes) not between 0 and 1");
            frac *= 60;
            nSec = floor(frac);
            frac -=  nSec;
            // nanoseconds
            OSL_ENSURE(frac < 1 && frac >= 0, "ISO8601parse internal error frac seconds (of minutes) not between 0 and 1");
            frac *= 1000000000;
            nNanoSec = ::rtl::math::round(frac);
        }
        goto end;
    }
    if(n >= aTimeStr.size())
        goto end;

    // seconds
    bSuccess = getISO8601TimeToken(aTimeStr, n, tokInt, bFrac, tokFrac);
    if (!bSuccess)
        return false;
    if (n < aTimeStr.size())
    {
        // is it junk or the timezone?
        bSuccess = getISO8601TimeZoneToken(aTimeStr, n, tokTz);
        if (!bSuccess)
            return false;
    }
    // max 60 for leap seconds
    bSuccess = convertNumber32( nSec, tokInt, 0, 60 );
    if (!bSuccess)
        return false;
    if (bFrac)
    {
        sal_Int64 fracNumerator;
        bSuccess = convertNumber64(fracNumerator, tokFrac);
        if ( bSuccess )
        {
            double frac = static_cast<double>(fracNumerator) / pow(static_cast<double>(10), static_cast<double>(tokFrac.getLength()));
            // nanoseconds
            OSL_ENSURE(frac < 1 && frac >= 0, "ISO8601parse internal error frac seconds (of seconds) not between 0 and 1");
            frac *= 1000000000;
            nNanoSec = ::rtl::math::round(frac);
        }
        goto end;
    }

    end:
    if (bSuccess)
    {
        // normalise time
        const int secondsOverflow = (nSec == 60) ? 61 : 60;
        if (nNanoSec == 1000000000)
        {
            nNanoSec = 0;
            ++nSec;
        }
        if(nSec == secondsOverflow)
        {
            nSec = 0;
            ++nMin;
        }
        if(nMin == 60)
        {
            nMin = 0;
            ++nHour;
        }
        if(!tokTz.isEmpty())
            rTime.IsUTC = (tokTz == "Z");

        rTime.Hours = static_cast<sal_uInt16>(nHour);
        rTime.Minutes = static_cast<sal_uInt16>(nMin);
        rTime.Seconds = static_cast<sal_uInt16>(nSec);
        rTime.NanoSeconds = nNanoSec;
    }

    return bSuccess;
}

}   // namespace utl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
