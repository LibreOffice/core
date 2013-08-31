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
#include <stdio.h>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <rtl/ustrbuf.hxx>
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

//.........................................................................
namespace dbtools
{
//.........................................................................


    using namespace ::comphelper;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::util;
    namespace utl = ::com::sun::star::util;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;


    //------------------------------------------------------------------------------
    ::com::sun::star::util::Date DBTypeConversion::getStandardDate()
    {
        static ::com::sun::star::util::Date STANDARD_DB_DATE(1,1,1900);
        return STANDARD_DB_DATE;
    }
    //------------------------------------------------------------------------------
    OUString DBTypeConversion::toDateString(const utl::Date& rDate)
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
    //------------------------------------------------------------------
    OUString DBTypeConversion::toTimeStringS(const utl::Time& rTime)
    {
        std::ostringstream ostr;
        using std::setw;
        ostr.fill('0');
        ostr << setw(2) << rTime.Hours   << ":"
             << setw(2) << rTime.Minutes << ":"
             << setw(2) << rTime.Seconds;
        return OUString::createFromAscii(ostr.str().c_str());
    }
    //------------------------------------------------------------------
    OUString DBTypeConversion::toTimeString(const utl::Time& rTime)
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
    //------------------------------------------------------------------
    OUString DBTypeConversion::toDateTimeString(const utl::DateTime& _rDateTime)
    {
        utl::Date aDate(_rDateTime.Day,_rDateTime.Month,_rDateTime.Year);
        OUStringBuffer aTemp(toDateString(aDate));
        aTemp.appendAscii(" ");
        utl::Time const aTime(_rDateTime.NanoSeconds, _rDateTime.Seconds,
                _rDateTime.Minutes, _rDateTime.Hours, _rDateTime.IsUTC);
        aTemp.append( toTimeString(aTime) );
        return  aTemp.makeStringAndClear();
    }
    //------------------------------------------------------------------------------
    utl::Date DBTypeConversion::toDate(sal_Int32 _nVal)
    {
        utl::Date aReturn;
        aReturn.Day = (sal_uInt16)(_nVal % 100);
        aReturn.Month = (sal_uInt16)((_nVal  / 100) % 100);
        aReturn.Year = (sal_uInt16)(_nVal  / 10000);
        return aReturn;
    }

    //------------------------------------------------------------------------------
    utl::Time DBTypeConversion::toTime(sal_Int64 _nVal)
    {
        utl::Time aReturn;
        sal_uInt64 unVal = static_cast<sal_uInt64>(_nVal >= 0 ? _nVal : -_nVal);
        aReturn.Hours = unVal / hourMask;
        aReturn.Minutes = (unVal / minMask) % 100;
        aReturn.Seconds = (unVal / secMask) % 100;
        aReturn.NanoSeconds = unVal % secMask;
        return aReturn;
    }

    sal_Int64 DBTypeConversion::getNsFromTime(const utl::Time& rVal)
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

    //------------------------------------------------------------------------------
    static const sal_Int32 aDaysInMonth[12] = {   31, 28, 31, 30, 31, 30,
                                            31, 31, 30, 31, 30, 31 };

    //------------------------------------------------------------------------------
    static sal_Bool implIsLeapYear(sal_Int32 _nYear)
    {
        return  (   (   ((_nYear % 4) == 0)
                    &&  ((_nYear % 100) != 0)
                    )
                )
                ||  ((_nYear % 400) == 0)
                ;
    }

    //------------------------------------------------------------------------------
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

    //------------------------------------------------------------------------------
    static sal_Int32 implRelativeToAbsoluteNull(const utl::Date& _rDate)
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
    //------------------------------------------------------------------------------
    static void implBuildFromRelative( sal_Int32 nDays, sal_uInt16& rDay, sal_uInt16& rMonth, sal_Int16& rYear)
    {
        sal_Int32   nTempDays;
        sal_Int32   i = 0;
        sal_Bool    bCalc;

        do
        {
            nTempDays = nDays;
            rYear = (sal_uInt16)((nTempDays / 365) - i);
            nTempDays -= (rYear-1) * 365;
            nTempDays -= ((rYear-1) / 4) - ((rYear-1) / 100) + ((rYear-1) / 400);
            bCalc = sal_False;
            if ( nTempDays < 1 )
            {
                i++;
                bCalc = sal_True;
            }
            else
            {
                if ( nTempDays > 365 )
                {
                    if ( (nTempDays != 366) || !implIsLeapYear( rYear ) )
                    {
                        i--;
                        bCalc = sal_True;
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
    //------------------------------------------------------------------------------
    sal_Int32 DBTypeConversion::toDays(const utl::Date& _rVal, const utl::Date& _rNullDate)
    {
        return implRelativeToAbsoluteNull(_rVal) - implRelativeToAbsoluteNull(_rNullDate);
    }

    //------------------------------------------------------------------------------
    double DBTypeConversion::toDouble(const utl::Date& rVal, const utl::Date& _rNullDate)
    {
        return (double)toDays(rVal, _rNullDate);
    }

    //------------------------------------------------------------------------------
    double DBTypeConversion::toDouble(const utl::Time& rVal)
    {
        return (double)getNsFromTime(rVal) / fNanoSecondsPerDay;
    }

    //------------------------------------------------------------------------------
    double DBTypeConversion::toDouble(const utl::DateTime& _rVal, const utl::Date& _rNullDate)
    {
        sal_Int64   nTime     = toDays(utl::Date(_rVal.Day, _rVal.Month, _rVal.Year), _rNullDate);
        utl::Time aTimePart;

        aTimePart.Hours             = _rVal.Hours;
        aTimePart.Minutes           = _rVal.Minutes;
        aTimePart.Seconds           = _rVal.Seconds;
        aTimePart.NanoSeconds       = _rVal.NanoSeconds;

        return ((double)nTime) + toDouble(aTimePart);
    }
    // -------------------------------------------------------------------------
    static void addDays(sal_Int32 nDays, utl::Date& _rDate)
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
    // -----------------------------------------------------------------------
    static void subDays( sal_Int32 nDays, utl::Date& _rDate )
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
    // -------------------------------------------------------------------------
    utl::Date DBTypeConversion::toDate(double dVal, const utl::Date& _rNullDate)
    {
        utl::Date aRet = _rNullDate;

        if (dVal >= 0)
            addDays((sal_Int32)dVal,aRet);
        else
            subDays((sal_uInt32)(-dVal),aRet);
            //  x -= (sal_uInt32)(-nDays);

        return aRet;
    }
    // -------------------------------------------------------------------------
    utl::Time DBTypeConversion::toTime(double dVal)
    {
        sal_Int32 nDays     = (sal_Int32)dVal;
        sal_Int64 nNS = static_cast<sal_Int64>((dVal - (double)nDays) * fNanoSecondsPerDay + 0.5);

        sal_Int16 nSign;
        if ( nNS < 0 )
        {
            nNS *= -1;
            nSign = -1;
        }
        else
            nSign = 1;

        utl::Time xRet;
        // normalize time
        // we have to sal_Int32 here because otherwise we get an overflow
        sal_Int64 nNanoSeconds      = nNS;
        sal_Int32 nSeconds          = nNanoSeconds / nanoSecInSec;
        sal_Int32 nMinutes          = nSeconds / secInMin;

        xRet.NanoSeconds            = nNanoSeconds % nanoSecInSec;
        xRet.Seconds                = nSeconds % secInMin;
        xRet.Hours                  = nMinutes / minInHour;
        xRet.Minutes                = nMinutes % minInHour;

        // assemble time
        sal_Int64 nTime = nSign *
                          (xRet.NanoSeconds +
                           xRet.Seconds * secMask +
                           xRet.Minutes * minMask +
                           xRet.Hours   * hourMask);

        if(nTime < 0)
        {
            xRet.NanoSeconds  = nanoSecInSec-1;
            xRet.Seconds      = secInMin-1;
            xRet.Minutes      = minInHour-1;
            xRet.Hours        = 23;
        }
        return xRet;
    }
    //------------------------------------------------------------------------------
    utl::DateTime DBTypeConversion::toDateTime(double dVal, const utl::Date& _rNullDate)
    {
        utl::Date aDate = toDate(dVal, _rNullDate);
        utl::Time aTime = toTime(dVal);

        utl::DateTime xRet;

        xRet.Day          = aDate.Day;
        xRet.Month        = aDate.Month;
        xRet.Year         = aDate.Year;

        xRet.NanoSeconds  = aTime.NanoSeconds;
        xRet.Minutes      = aTime.Minutes;
        xRet.Seconds      = aTime.Seconds;
        xRet.Hours        = aTime.Hours;


        return xRet;
    }
    //------------------------------------------------------------------------------
    utl::Date DBTypeConversion::toDate(const OUString& _sSQLString)
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

        return utl::Date(nDay,nMonth,nYear);
    }

    //-----------------------------------------------------------------------------
    utl::DateTime DBTypeConversion::toDateTime(const OUString& _sSQLString)
    {
        //@see http://java.sun.com/j2se/1.4.2/docs/api/java/sql/Timestamp.html#valueOf(java.lang.String)
        //@see http://java.sun.com/j2se/1.4.2/docs/api/java/sql/Date.html#valueOf(java.lang.String)
        //@see http://java.sun.com/j2se/1.4.2/docs/api/java/sql/Time.html#valueOf(java.lang.String)

        // the date part
        utl::Date aDate = toDate(_sSQLString);
        utl::Time aTime;
        sal_Int32 nSeparation = _sSQLString.indexOf( ' ' );
        if ( -1 != nSeparation )
        {
            const sal_Unicode *p = _sSQLString.getStr() + nSeparation;
            const sal_Unicode *const begin = p;
            for(;isspace(*p);++p);
            nSeparation += p - begin;
            aTime = toTime( _sSQLString.copy( nSeparation ) );
        }

        return utl::DateTime(aTime.NanoSeconds, aTime.Seconds, aTime.Minutes, aTime.Hours,
                        aDate.Day, aDate.Month, aDate.Year, false);
    }

    //-----------------------------------------------------------------------------
    utl::Time DBTypeConversion::toTime(const OUString& _sSQLString)
    {
        utl::Time aTime;
        ::utl::ISO8601parseTime(_sSQLString, aTime);
        return aTime;
    }

//.........................................................................
}   // namespace dbtools
//.........................................................................


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
