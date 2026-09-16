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
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <rtl/character.hxx>
#include <rtl/math.hxx>
#include <sal/log.hxx>
#include <unotools/datetime.hxx>
#include <comphelper/date.hxx>
#include <o3tl/safeint.hxx>
#include <o3tl/string_view.hxx>
#include <sstream>
#include <iomanip>

namespace
{
    const sal_Int64 nanoSecInSec = 1000000000;
    const sal_Int16 secInMin  = 60;
    const sal_Int16 minInHour = 60;

    const sal_Int64 secMask  = 1000000000;
    const sal_Int64 minMask  = 100000000000LL;
    const sal_Int64 hourMask = 10000000000000LL;

    const double fNanoSecondsPerDay = nanoSecInSec * secInMin * minInHour * 24.0;

    //  32767-12-31 in "(days since 0001-01-01) + 1" format
    const sal_Int32 maxDays =  11967896;
    // -32768-01-01 in "(days since 0001-01-01) + 1" format
    // Yes, I know it is currently unused. Will have to be used
    // when we implement negative years. Writing down the correct
    // value for future reference.
    // *** Please don't remove just because it is unused ***
    // Lionel Élie Mamane 2017-08-02
    // const sal_Int32 minDays = -11968270;
}


namespace dbtools
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;


    css::util::Date const & DBTypeConversion::getStandardDate()
    {
        static css::util::Date STANDARD_DB_DATE(30,12,1899);
        return STANDARD_DB_DATE;
    }

    OUString DBTypeConversion::toDateString(const css::util::Date& rDate)
    {
        std::ostringstream ostr;
        using std::setw;
        ostr.fill('0');
        ostr << setw(4) << rDate.Year  << "-"
             << setw(2) << rDate.Month << "-"
             << setw(2) << rDate.Day;
        return OUString::createFromAscii(ostr.str());
    }

    OUString DBTypeConversion::toTimeStringS(const css::util::Time& rTime)
    {
        std::ostringstream ostr;
        using std::setw;
        ostr.fill('0');
        ostr << setw(2) << rTime.Hours   << ":"
             << setw(2) << rTime.Minutes << ":"
             << setw(2) << rTime.Seconds;
        return OUString::createFromAscii(ostr.str());
    }

    OUString DBTypeConversion::toTimeString(const css::util::Time& rTime)
    {
        std::ostringstream ostr;
        using std::setw;
        ostr.fill('0');
        ostr << setw(2) << rTime.Hours   << ":"
             << setw(2) << rTime.Minutes << ":"
             << setw(2) << rTime.Seconds;
        if (rTime.NanoSeconds)
        {
            std::ostringstream ostrFraction;
            ostrFraction.fill('0');
            ostrFraction << setw(9) << rTime.NanoSeconds;
            // Trim trailing zeros if exist, since some backends (e.g. Firebird)
            // reject a literal with more fractional digits than they support.
            std::string sFraction = ostrFraction.str();
            sFraction.erase(sFraction.find_last_not_of('0') + 1);
            ostr << "." << sFraction;
        }
        return OUString::createFromAscii(ostr.str());
    }

    OUString DBTypeConversion::toDateTimeString(const css::util::DateTime& _rDateTime)
    {
        css::util::Date aDate(_rDateTime.Day,_rDateTime.Month,_rDateTime.Year);
        css::util::Time const aTime(_rDateTime.NanoSeconds, _rDateTime.Seconds,
                _rDateTime.Minutes, _rDateTime.Hours, _rDateTime.IsUTC);
        return toDateString(aDate) + " " + toTimeString(aTime);
    }

    bool DBTypeConversion::roundTimeFraction(css::util::Time& rTime, sal_Int32 nMaxFractionDigits)
    {
        // Round the fraction to at most nMaxFractionDigits digits,
        // carrying into seconds/minutes/hours as needed.
        if (nMaxFractionDigits >= 9 || nMaxFractionDigits < 0)
            return false;

        sal_uInt32 nRoundingUnit = 1;
        for (sal_Int32 i = 0; i < 9 - nMaxFractionDigits; ++i)
            nRoundingUnit *= 10;

        sal_uInt32 nNanoSeconds
            = ((rTime.NanoSeconds + nRoundingUnit / 2) / nRoundingUnit) * nRoundingUnit;
        bool bCarriedPastMidnight = false;
        if (nNanoSeconds >= o3tl::make_unsigned(nanoSecInSec))
        {
            nNanoSeconds -= static_cast<sal_uInt32>(nanoSecInSec);
            if (++rTime.Seconds >= secInMin)
            {
                rTime.Seconds -= secInMin;
                if (++rTime.Minutes >= minInHour)
                {
                    rTime.Minutes -= minInHour;
                    // Time has no day of its own to carry into so wrap
                    // at 24h and report it to the caller.
                    if (++rTime.Hours >= 24)
                    {
                        rTime.Hours -= 24;
                        bCarriedPastMidnight = true;
                    }
                }
            }
        }
        rTime.NanoSeconds = nNanoSeconds;
        return bCarriedPastMidnight;
    }

    css::util::Date DBTypeConversion::toDate(const sal_Int32 _nVal)
    {
        css::util::Date aReturn;
        aReturn.Day = static_cast<sal_uInt16>(_nVal % 100);
        aReturn.Month = static_cast<sal_uInt16>((_nVal  / 100) % 100);
        aReturn.Year = static_cast<sal_uInt16>(_nVal  / 10000);
        return aReturn;
    }


    css::util::Time DBTypeConversion::toTime(const sal_Int64 _nVal)
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

    static sal_Int32 implRelativeToAbsoluteNull(const css::util::Date& _rDate)
    {
        if (_rDate.Day == 0 && _rDate.Month == 0 && _rDate.Year == 0)
        {
            // 0000-00-00 is *NOT* a valid date and passing it to the date
            // conversion even when normalizing rightly asserts. Whatever we
            // return here, it will be wrong. The old before commit
            // 52ff16771ac160d27fd7beb78a4cfba22ad84f06 wrong implementation
            // calculated -365 for that, effectively that would be a date of
            // -0001-01-01 now but it was likely assumed that would be
            // 0000-00-01 or even 0000-00-00 instead. Try if we get away with 0
            // for -0001-12-31, the same that
            // comphelper::date::convertDateToDaysNormalizing()
            // would return if comphelper::date::normalize() wouldn't ignore
            // such "empty" date.

            return 0;
        }
        return comphelper::date::convertDateToDaysNormalizing( _rDate.Day, _rDate.Month, _rDate.Year);
    }

    sal_Int32 DBTypeConversion::toDays(const css::util::Date& _rVal, const css::util::Date& _rNullDate)
    {
        return implRelativeToAbsoluteNull(_rVal) - implRelativeToAbsoluteNull(_rNullDate);
    }


    double DBTypeConversion::toDouble(const css::util::Date& rVal, const css::util::Date& _rNullDate)
    {
        return static_cast<double>(toDays(rVal, _rNullDate));
    }


    double DBTypeConversion::toDouble(const css::util::Time& rVal)
    {
        return static_cast<double>(getNsFromTime(rVal)) / fNanoSecondsPerDay;
    }


    double DBTypeConversion::toDouble(const css::util::DateTime& _rVal, const css::util::Date& _rNullDate)
    {
        sal_Int64   nTime     = toDays(css::util::Date(_rVal.Day, _rVal.Month, _rVal.Year), _rNullDate);
        css::util::Time aTimePart;

        aTimePart.Hours             = _rVal.Hours;
        aTimePart.Minutes           = _rVal.Minutes;
        aTimePart.Seconds           = _rVal.Seconds;
        aTimePart.NanoSeconds       = _rVal.NanoSeconds;

        return static_cast<double>(nTime) + toDouble(aTimePart);
    }

    static void addDays(const sal_Int32 nDays, css::util::Date& _rDate)
    {
        sal_Int64 nTempDays = implRelativeToAbsoluteNull(_rDate);

        nTempDays += nDays;
        if ( nTempDays > maxDays )
        {
            _rDate.Day      = 31;
            _rDate.Month    = 12;
            _rDate.Year     = 9999;
        }
        // TODO: can we replace that check by minDays? Would allow dates BCE
        else if ( nTempDays <= 0 )
        {
            _rDate.Day      = 1;
            _rDate.Month    = 1;
            _rDate.Year     = 1;
        }
        else
            comphelper::date::convertDaysToDate( nTempDays, _rDate.Day, _rDate.Month, _rDate.Year );
    }

    void DBTypeConversion::roundDateTimeFraction(css::util::DateTime& rDateTime, sal_Int32 nMaxFractionDigits)
    {
        css::util::Time aTime(rDateTime.NanoSeconds, rDateTime.Seconds, rDateTime.Minutes,
                              rDateTime.Hours, rDateTime.IsUTC);
        if (roundTimeFraction(aTime, nMaxFractionDigits))
        {
            // Unlike a bare Time, a DateTime does have a date to carry into,
            // so 23:59:59.99995 becomes the next day at 00:00:00 rather than
            // an out-of-range 24:00:00 on the original date.
            css::util::Date aDate(rDateTime.Day, rDateTime.Month, rDateTime.Year);
            addDays(1, aDate);
            rDateTime.Day   = aDate.Day;
            rDateTime.Month = aDate.Month;
            rDateTime.Year  = aDate.Year;
        }
        rDateTime.Hours       = aTime.Hours;
        rDateTime.Minutes     = aTime.Minutes;
        rDateTime.Seconds     = aTime.Seconds;
        rDateTime.NanoSeconds = aTime.NanoSeconds;
    }

    static void subDays(const sal_Int32 nDays, css::util::Date& _rDate )
    {
        sal_Int64 nTempDays = implRelativeToAbsoluteNull(_rDate);

        nTempDays -= nDays;
        if ( nTempDays > maxDays )
        {
            _rDate.Day      = 31;
            _rDate.Month    = 12;
            _rDate.Year     = 9999;
        }
        // TODO: can we replace that check by minDays? Would allow dates BCE
        else if ( nTempDays <= 0 )
        {
            _rDate.Day      = 1;
            _rDate.Month    = 1;
            _rDate.Year     = 1;
        }
        else
            comphelper::date::convertDaysToDate( nTempDays, _rDate.Day, _rDate.Month, _rDate.Year );
    }

    css::util::Date DBTypeConversion::toDate(const double dVal, const css::util::Date& _rNullDate)
    {
        css::util::Date aRet = _rNullDate;

        if (dVal >= 0)
            addDays(static_cast<sal_Int32>(dVal),aRet);
        else
            subDays(static_cast<sal_uInt32>(-dVal),aRet);
            //  x -= (sal_uInt32)(-nDays);

        return aRet;
    }

    css::util::Time DBTypeConversion::toTime(const double dVal, short nDigits)
    {
        const double nDays = std::trunc(dVal);
        double fSeconds((dVal - nDays) * (fNanoSecondsPerDay / nanoSecInSec));
        fSeconds = ::rtl::math::round(fSeconds, nDigits);
        sal_Int64 nNS = fSeconds * nanoSecInSec;

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

    css::util::DateTime DBTypeConversion::toDateTime(const double dVal, const css::util::Date& _rNullDate)
    {
        css::util::DateTime aRet;

        if (!std::isfinite(dVal))
        {
            SAL_WARN("connectivity.commontools", "DateTime has invalid value: " << dVal);
            return aRet;
        }

        css::util::Date aDate = toDate(dVal, _rNullDate);
        // there is not enough precision in a double to have both a date
        // and a time up to nanoseconds -> limit to microseconds to have
        // correct rounding, that is e.g. 13:00:00.000000000 instead of
        // 12:59:59.999999790
        css::util::Time aTime = toTime(dVal, 6);

        aRet.Day          = aDate.Day;
        aRet.Month        = aDate.Month;
        aRet.Year         = aDate.Year;

        aRet.NanoSeconds  = aTime.NanoSeconds;
        aRet.Minutes      = aTime.Minutes;
        aRet.Seconds      = aTime.Seconds;
        aRet.Hours        = aTime.Hours;


        return aRet;
    }

    css::util::Date DBTypeConversion::toDate(std::u16string_view _sSQLString)
    {
        // get the token out of a string
        static const sal_Unicode sDateSep = '-';

        sal_Int32 nIndex    = 0;
        sal_uInt16  nYear   = 0,
                    nMonth  = 0,
                    nDay    = 0;
        nYear   = static_cast<sal_uInt16>(o3tl::toInt32(o3tl::getToken(_sSQLString, 0,sDateSep,nIndex)));
        if(nIndex != -1)
        {
            nMonth = static_cast<sal_uInt16>(o3tl::toInt32(o3tl::getToken(_sSQLString, 0,sDateSep,nIndex)));
            if(nIndex != -1)
                nDay = static_cast<sal_uInt16>(o3tl::toInt32(o3tl::getToken(_sSQLString, 0,sDateSep,nIndex)));
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
            while (rtl::isAsciiWhiteSpace(*p)) { ++p; }
            nSeparation += p - begin;
            aTime = toTime( _sSQLString.subView( nSeparation ) );
        }

        return css::util::DateTime(aTime.NanoSeconds, aTime.Seconds, aTime.Minutes, aTime.Hours,
                        aDate.Day, aDate.Month, aDate.Year, false);
    }


    css::util::Time DBTypeConversion::toTime(std::u16string_view _sSQLString)
    {
        css::util::Time aTime;
        ::utl::ISO8601parseTime(_sSQLString, aTime);
        return aTime;
    }


}   // namespace dbtools


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
