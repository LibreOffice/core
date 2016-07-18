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

#include <connectivity/dbconversion.hxx>
#include <connectivity/dbcharset.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <rtl/ustrbuf.hxx>
#include <rtl/math.hxx>
#include <unotools/datetime.hxx>
#include <sstream>
#include <iomanip>

#define MAX_DAYS    3636532

namespace
{
    const sal_Int64 nanoSecInSec = 1000000000;
    const sal_Int16 secInMin  = 60;
    const sal_Int16 minInHour = 60;

    const sal_Int64 secMask  = 1000000000;
    const sal_Int64 minMask  = 100000000000LL;
    const sal_Int64 hourMask = 10000000000000LL;

    const double fNanoSecondsPerDay = nanoSecInSec * secInMin * minInHour * 24.0;
}


namespace dbtools
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;


    css::util::Date const & DBTypeConversion::getStandardDate()
    {
        static css::util::Date STANDARD_DB_DATE(1,1,1900);
        return STANDARD_DB_DATE;
    }

    OUString DBTypeConversion::toDateString(const css::util::Date& rDate)
    {
        sal_Char s[11];
        snprintf(s,
                sizeof(s),
                "%04d-%02d-%02d",
                (int)rDate.Year,
                (int)rDate.Month,
                (int)rDate.Day);
        s[10] = 0;
        return OUString::createFromAscii(s);
    }

    OUString DBTypeConversion::toTimeStringS(const css::util::Time& rTime)
    {
        std::ostringstream ostr;
        using std::setw;
        ostr.fill('0');
        ostr << setw(2) << rTime.Hours   << ":"
             << setw(2) << rTime.Minutes << ":"
             << setw(2) << rTime.Seconds;
        return OUString::createFromAscii(ostr.str().c_str());
    }

    OUString DBTypeConversion::toTimeString(const css::util::Time& rTime)
    {
        std::ostringstream ostr;
        using std::setw;
        ostr.fill('0');
        ostr << setw(2) << rTime.Hours   << ":"
             << setw(2) << rTime.Minutes << ":"
             << setw(2) << rTime.Seconds << "."
             << setw(9) << rTime.NanoSeconds;
        return OUString::createFromAscii(ostr.str().c_str());
    }

    OUString DBTypeConversion::toDateTimeString(const css::util::DateTime& _rDateTime)
    {
        css::util::Date aDate(_rDateTime.Day,_rDateTime.Month,_rDateTime.Year);
        OUStringBuffer aTemp(toDateString(aDate));
        aTemp.append(" ");
        css::util::Time const aTime(_rDateTime.NanoSeconds, _rDateTime.Seconds,
                _rDateTime.Minutes, _rDateTime.Hours, _rDateTime.IsUTC);
        aTemp.append( toTimeString(aTime) );
        return  aTemp.makeStringAndClear();
    }

    css::util::Date DBTypeConversion::toDate(sal_Int32 _nVal)
    {
        css::util::Date aReturn;
        aReturn.Day = (sal_uInt16)(_nVal % 100);
        aReturn.Month = (sal_uInt16)((_nVal  / 100) % 100);
        aReturn.Year = (sal_uInt16)(_nVal  / 10000);
        return aReturn;
    }


    css::util::Time DBTypeConversion::toTime(sal_Int64 _nVal)
    {
        css::util::Time aReturn;
        sal_uInt64 unVal = static_cast<sal_uInt64>(_nVal >= 0 ? _nVal : -_nVal);
        aReturn.Hours = unVal / hourMask;
        aReturn.Minutes = (unVal / minMask) % 100;
        aReturn.Seconds = (unVal / secMask) % 100;
        aReturn.NanoSeconds = unVal % secMask;
        return aReturn;
    }

    sal_Int64 DBTypeConversion::getNsFromTime(const css::util::Time& rVal)
    {
        sal_Int32   nHour     = rVal.Hours;
        sal_Int32   nMin      = rVal.Minutes;
        sal_Int32   nSec      = rVal.Seconds;
        sal_Int32   nNanoSec  = rVal.NanoSeconds;

        return nNanoSec +
               nSec  * nanoSecInSec +
               nMin  * (secInMin * nanoSecInSec) +
               nHour * (minInHour * secInMin * nanoSecInSec);
    }


    static const sal_Int32 aDaysInMonth[12] = {   31, 28, 31, 30, 31, 30,
                                            31, 31, 30, 31, 30, 31 };


    static bool implIsLeapYear(sal_Int32 _nYear)
    {
        return  (   (   ((_nYear % 4) == 0)
                    &&  ((_nYear % 100) != 0)
                    )
                )
                ||  ((_nYear % 400) == 0)
                ;
    }


    static sal_Int32 implDaysInMonth(sal_Int32 _nMonth, sal_Int32 _nYear)
    {
        OSL_ENSURE(_nMonth > 0 && _nMonth < 13,"Month as invalid value!");
        if (_nMonth != 2)
            return aDaysInMonth[_nMonth-1];
        else
        {
            if (implIsLeapYear(_nYear))
                return aDaysInMonth[_nMonth-1] + 1;
            else
                return aDaysInMonth[_nMonth-1];
        }
    }


    static sal_Int32 implRelativeToAbsoluteNull(const css::util::Date& _rDate)
    {
        sal_Int32 nDays = 0;

        // ripped this code from the implementation of tools::Date
        sal_Int32 nNormalizedYear = _rDate.Year - 1;
        nDays = nNormalizedYear * 365;
        // leap years
        nDays += (nNormalizedYear / 4) - (nNormalizedYear / 100) + (nNormalizedYear / 400);

        for (sal_Int32 i = 1; i < _rDate.Month; ++i)
            nDays += implDaysInMonth(i, _rDate.Year);

        nDays += _rDate.Day;
        return nDays;
    }

    static void implBuildFromRelative( sal_Int32 nDays, sal_uInt16& rDay, sal_uInt16& rMonth, sal_Int16& rYear)
    {
        sal_Int32   nTempDays;
        sal_Int32   i = 0;
        bool    bCalc;

        do
        {
            nTempDays = nDays;
            rYear = (sal_uInt16)((nTempDays / 365) - i);
            nTempDays -= (rYear-1) * 365;
            nTempDays -= ((rYear-1) / 4) - ((rYear-1) / 100) + ((rYear-1) / 400);
            bCalc = false;
            if ( nTempDays < 1 )
            {
                i++;
                bCalc = true;
            }
            else
            {
                if ( nTempDays > 365 )
                {
                    if ( (nTempDays != 366) || !implIsLeapYear( rYear ) )
                    {
                        i--;
                        bCalc = true;
                    }
                }
            }
        }
        while ( bCalc );

        rMonth = 1;
        while ( nTempDays > implDaysInMonth( rMonth, rYear ) )
        {
            nTempDays -= implDaysInMonth( rMonth, rYear );
            rMonth++;
        }
        rDay = (sal_uInt16)nTempDays;
    }

    sal_Int32 DBTypeConversion::toDays(const css::util::Date& _rVal, const css::util::Date& _rNullDate)
    {
        return implRelativeToAbsoluteNull(_rVal) - implRelativeToAbsoluteNull(_rNullDate);
    }


    double DBTypeConversion::toDouble(const css::util::Date& rVal, const css::util::Date& _rNullDate)
    {
        return (double)toDays(rVal, _rNullDate);
    }


    double DBTypeConversion::toDouble(const css::util::Time& rVal)
    {
        return (double)getNsFromTime(rVal) / fNanoSecondsPerDay;
    }


    double DBTypeConversion::toDouble(const css::util::DateTime& _rVal, const css::util::Date& _rNullDate)
    {
        sal_Int64   nTime     = toDays(css::util::Date(_rVal.Day, _rVal.Month, _rVal.Year), _rNullDate);
        css::util::Time aTimePart;

        aTimePart.Hours             = _rVal.Hours;
        aTimePart.Minutes           = _rVal.Minutes;
        aTimePart.Seconds           = _rVal.Seconds;
        aTimePart.NanoSeconds       = _rVal.NanoSeconds;

        return ((double)nTime) + toDouble(aTimePart);
    }

    static void addDays(sal_Int32 nDays, css::util::Date& _rDate)
    {
        sal_Int32   nTempDays = implRelativeToAbsoluteNull( _rDate );

        nTempDays += nDays;
        if ( nTempDays > MAX_DAYS )
        {
            _rDate.Day      = 31;
            _rDate.Month    = 12;
            _rDate.Year     = 9999;
        }
        else if ( nTempDays <= 0 )
        {
            _rDate.Day      = 1;
            _rDate.Month    = 1;
            _rDate.Year     = 00;
        }
        else
            implBuildFromRelative( nTempDays, _rDate.Day, _rDate.Month, _rDate.Year );
    }

    static void subDays( sal_Int32 nDays, css::util::Date& _rDate )
    {
        sal_Int32   nTempDays = implRelativeToAbsoluteNull( _rDate );

        nTempDays -= nDays;
        if ( nTempDays > MAX_DAYS )
        {
            _rDate.Day      = 31;
            _rDate.Month    = 12;
            _rDate.Year     = 9999;
        }
        else if ( nTempDays <= 0 )
        {
            _rDate.Day      = 1;
            _rDate.Month    = 1;
            _rDate.Year     = 00;
        }
        else
            implBuildFromRelative( nTempDays, _rDate.Day, _rDate.Month, _rDate.Year );
    }

    css::util::Date DBTypeConversion::toDate(double dVal, const css::util::Date& _rNullDate)
    {
        css::util::Date aRet = _rNullDate;

        if (dVal >= 0)
            addDays((sal_Int32)dVal,aRet);
        else
            subDays((sal_uInt32)(-dVal),aRet);
            //  x -= (sal_uInt32)(-nDays);

        return aRet;
    }

    css::util::Time DBTypeConversion::toTime(double dVal, short nDigits)
    {
        sal_Int32 nDays     = (sal_Int32)dVal;
        sal_Int64 nNS;
        {
            double fSeconds((dVal - (double)nDays) * (fNanoSecondsPerDay / nanoSecInSec));
            fSeconds = ::rtl::math::round( fSeconds, nDigits );
            nNS = fSeconds * nanoSecInSec;
        }

        sal_Int16 nSign;
        if ( nNS < 0 )
        {
            nNS *= -1;
            nSign = -1;
        }
        else
            nSign = 1;

        css::util::Time aRet;
        // normalize time
        // we have to sal_Int32 here because otherwise we get an overflow
        sal_Int64 nNanoSeconds      = nNS;
        sal_Int32 nSeconds          = nNanoSeconds / nanoSecInSec;
        sal_Int32 nMinutes          = nSeconds / secInMin;

        aRet.NanoSeconds            = nNanoSeconds % nanoSecInSec;
        aRet.Seconds                = nSeconds % secInMin;
        aRet.Hours                  = nMinutes / minInHour;
        aRet.Minutes                = nMinutes % minInHour;

        // assemble time
        sal_Int64 nTime = nSign *
                          (aRet.NanoSeconds +
                           aRet.Seconds * secMask +
                           aRet.Minutes * minMask +
                           aRet.Hours   * hourMask);

        if(nTime < 0)
        {
            aRet.NanoSeconds  = nanoSecInSec-1;
            aRet.Seconds      = secInMin-1;
            aRet.Minutes      = minInHour-1;
            aRet.Hours        = 23;
        }
        return aRet;
    }

    css::util::DateTime DBTypeConversion::toDateTime(double dVal, const css::util::Date& _rNullDate)
    {
        css::util::Date aDate = toDate(dVal, _rNullDate);
        // there is not enough precision in a double to have both a date
        // and a time up to nanoseconds -> limit to microseconds to have
        // correct rounding, that is e.g. 13:00:00.000000000 instead of
        // 12:59:59.999999790
        css::util::Time aTime = toTime(dVal, 6);

        css::util::DateTime aRet;

        aRet.Day          = aDate.Day;
        aRet.Month        = aDate.Month;
        aRet.Year         = aDate.Year;

        aRet.NanoSeconds  = aTime.NanoSeconds;
        aRet.Minutes      = aTime.Minutes;
        aRet.Seconds      = aTime.Seconds;
        aRet.Hours        = aTime.Hours;


        return aRet;
    }

    css::util::Date DBTypeConversion::toDate(const OUString& _sSQLString)
    {
        // get the token out of a string
        static sal_Unicode sDateSep = '-';

        sal_Int32 nIndex    = 0;
        sal_uInt16  nYear   = 0,
                    nMonth  = 0,
                    nDay    = 0;
        nYear   = (sal_uInt16)_sSQLString.getToken(0,sDateSep,nIndex).toInt32();
        if(nIndex != -1)
        {
            nMonth = (sal_uInt16)_sSQLString.getToken(0,sDateSep,nIndex).toInt32();
            if(nIndex != -1)
                nDay = (sal_uInt16)_sSQLString.getToken(0,sDateSep,nIndex).toInt32();
        }

        return css::util::Date(nDay,nMonth,nYear);
    }


    css::util::DateTime DBTypeConversion::toDateTime(const OUString& _sSQLString)
    {
        //@see http://java.sun.com/j2se/1.4.2/docs/api/java/sql/Timestamp.html#valueOf(java.lang.String)
        //@see http://java.sun.com/j2se/1.4.2/docs/api/java/sql/Date.html#valueOf(java.lang.String)
        //@see http://java.sun.com/j2se/1.4.2/docs/api/java/sql/Time.html#valueOf(java.lang.String)

        // the date part
        css::util::Date aDate = toDate(_sSQLString);
        css::util::Time aTime;
        sal_Int32 nSeparation = _sSQLString.indexOf( ' ' );
        if ( -1 != nSeparation )
        {
            const sal_Unicode *p = _sSQLString.getStr() + nSeparation;
            const sal_Unicode *const begin = p;
            while (isspace(*p)) { ++p; }
            nSeparation += p - begin;
            aTime = toTime( _sSQLString.copy( nSeparation ) );
        }

        return css::util::DateTime(aTime.NanoSeconds, aTime.Seconds, aTime.Minutes, aTime.Hours,
                        aDate.Day, aDate.Month, aDate.Year, false);
    }


    css::util::Time DBTypeConversion::toTime(const OUString& _sSQLString)
    {
        css::util::Time aTime;
        ::utl::ISO8601parseTime(_sSQLString, aTime);
        return aTime;
    }


}   // namespace dbtools


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
