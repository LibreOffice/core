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
#ifndef _INC_STDIO
#include <stdio.h>
#endif
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <rtl/ustrbuf.hxx>

#define MAX_DAYS    3636532

namespace
{
    const double fMilliSecondsPerDay = 86400000.0;
    const sal_Int64 nanoSecInSec = 1000000000;
    const sal_Int16 secInMin  = 60;
    const sal_Int16 minInHour = 60;

    const sal_Int64 secMask  = 1000000000;
    const sal_Int64 minMask  = 100000000000;
    const sal_Int64 hourMask = 10000000000000;

    const double fNanoSecondsPerDay = nanoSecInSec * secInMin * minInHour * 24.0;
}

//.........................................................................
namespace dbtools
{
//.........................................................................


    using namespace ::comphelper;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::util;
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
    OUString DBTypeConversion::toDateString(const Date& rDate)
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
    OUString DBTypeConversion::toTimeString(const Time& rTime)
    {
        const size_t buflen = 19;
        sal_Char s[buflen];
        snprintf(s,
                 buflen,
                 "%02d:%02d:%02d.%09d",
                 rTime.Hours,
                 rTime.Minutes,
                 rTime.Seconds,
                 rTime.NanoSeconds);
        return OUString::createFromAscii(s);
    }

    //------------------------------------------------------------------
    OUString DBTypeConversion::toDateTimeString(const DateTime& _rDateTime)
    {
        Date aDate(_rDateTime.Day,_rDateTime.Month,_rDateTime.Year);
        OUStringBuffer aTemp(toDateString(aDate));
        aTemp.appendAscii(" ");
        Time aTime(_rDateTime.NanoSeconds,_rDateTime.Seconds,_rDateTime.Minutes,_rDateTime.Hours);
        aTemp.append( toTimeString(aTime) );
        return  aTemp.makeStringAndClear();
    }
    //------------------------------------------------------------------------------
    Date DBTypeConversion::toDate(sal_Int32 _nVal)
    {
        Date aReturn;
        aReturn.Day = (sal_uInt16)(_nVal % 100);
        aReturn.Month = (sal_uInt16)((_nVal  / 100) % 100);
        aReturn.Year = (sal_uInt16)(_nVal  / 10000);
        return aReturn;
    }

    //------------------------------------------------------------------------------
    Time DBTypeConversion::toTime(sal_Int64 _nVal)
    {
        Time aReturn;
        sal_uInt64 unVal = static_cast<sal_uInt64>(_nVal >= 0 ? _nVal : -_nVal);
        aReturn.Hours = unVal / hourMask;
        aReturn.Minutes = (unVal / minMask) % 100;
        aReturn.Seconds = (unVal / secMask) % 100;
        aReturn.NanoSeconds = unVal % secMask;
        return aReturn;
    }

    //------------------------------------------------------------------------------
    sal_Int32 DBTypeConversion::toINT32(const Date& rVal)
    {
        return ((sal_Int32)(rVal.Day%100)) +
            (((sal_Int32)(rVal.Month%100))*100) +
            (((sal_Int32) rVal.Year%10000)*10000);
    }

    //------------------------------------------------------------------------------
    sal_Int64 DBTypeConversion::toINT64(const Time& rVal)
    {
        // normalize time
        sal_Int32 nSeconds          = rVal.Seconds + rVal.NanoSeconds / nanoSecInSec;
        sal_Int32 nNanoSeconds      = rVal.NanoSeconds % nanoSecInSec;
        sal_Int32 nMinutes          = rVal.Minutes + nSeconds / secInMin;
        nSeconds                    = nSeconds % secInMin;
        sal_Int32 nHours            = rVal.Hours + nMinutes / minInHour;
        nMinutes                    = nMinutes % minInHour;

        // assemble time
        return nNanoSeconds +
               nSeconds * secMask +
               nMinutes * minMask +
               nHours   * hourMask;
    }

    //------------------------------------------------------------------------------
    sal_Int32 DBTypeConversion::getMsFromTime(const Time& rVal)
    {
        sal_Int32   nHour     = rVal.Hours;
        sal_Int32   nMin      = rVal.Minutes;
        sal_Int32   nSec      = rVal.Seconds;
        sal_Int32   nNanoSec  = rVal.NanoSeconds;

        return ((nHour*3600000)+(nMin*60000)+(nSec*1000)+(nNanoSec/1000000));
    }

    //------------------------------------------------------------------------------
    sal_Int64 DBTypeConversion::getNsFromTime(const Time& rVal)
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
    static sal_Int32 aDaysInMonth[12] = {   31, 28, 31, 30, 31, 30,
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
    static sal_Int32 implRelativeToAbsoluteNull(const Date& _rDate)
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
    sal_Int32 DBTypeConversion::toDays(const Date& _rVal, const Date& _rNullDate)
    {
        return implRelativeToAbsoluteNull(_rVal) - implRelativeToAbsoluteNull(_rNullDate);
    }

    //------------------------------------------------------------------------------
    double DBTypeConversion::toDouble(const Date& rVal, const Date& _rNullDate)
    {
        return (double)toDays(rVal, _rNullDate);
    }

    //------------------------------------------------------------------------------
    double DBTypeConversion::toDouble(const Time& rVal)
    {
        return (double)getNsFromTime(rVal) / fNanoSecondsPerDay;
    }

    //------------------------------------------------------------------------------
    double DBTypeConversion::toDouble(const DateTime& _rVal, const Date& _rNullDate)
    {
        sal_Int64   nTime     = toDays(Date(_rVal.Day, _rVal.Month, _rVal.Year), _rNullDate);
        Time aTimePart;

        aTimePart.Hours             = _rVal.Hours;
        aTimePart.Minutes           = _rVal.Minutes;
        aTimePart.Seconds           = _rVal.Seconds;
        aTimePart.NanoSeconds       = _rVal.NanoSeconds;

        return ((double)nTime) + toDouble(aTimePart);
    }
    // -------------------------------------------------------------------------
    static void addDays(sal_Int32 nDays, Date& _rDate)
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
    static void subDays( sal_Int32 nDays, Date& _rDate )
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
    Date DBTypeConversion::toDate(double dVal, const Date& _rNullDate)
    {
        Date aRet = _rNullDate;

        if (dVal >= 0)
            addDays((sal_Int32)dVal,aRet);
        else
            subDays((sal_uInt32)(-dVal),aRet);
            //  x -= (sal_uInt32)(-nDays);

        return aRet;
    }
    // -------------------------------------------------------------------------
    Time DBTypeConversion::toTime(double dVal)
    {
        sal_Int32 nDays     = (sal_Int32)dVal;
        sal_Int32 nNS = sal_Int32((dVal - (double)nDays) * fNanoSecondsPerDay + 0.5);

        sal_Int16 nSign;
        if ( nNS < 0 )
        {
            nNS *= -1;
            nSign = -1;
        }
        else
            nSign = 1;

        Time xRet;
        // normalize time
        // we have to sal_Int32 here because otherwise we get an overflow
        sal_Int32 nNanoSeconds      = nNS;
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
    DateTime DBTypeConversion::toDateTime(double dVal, const Date& _rNullDate)
    {
        Date aDate = toDate(dVal, _rNullDate);
        Time aTime = toTime(dVal);

        DateTime xRet;

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
    Date DBTypeConversion::toDate(const OUString& _sSQLString)
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

        return Date(nDay,nMonth,nYear);
    }

    //-----------------------------------------------------------------------------
    DateTime DBTypeConversion::toDateTime(const OUString& _sSQLString)
    {
        //@see http://java.sun.com/j2se/1.4.2/docs/api/java/sql/Timestamp.html#valueOf(java.lang.String)
        //@see http://java.sun.com/j2se/1.4.2/docs/api/java/sql/Date.html#valueOf(java.lang.String)
        //@see http://java.sun.com/j2se/1.4.2/docs/api/java/sql/Time.html#valueOf(java.lang.String)

        // the date part
        Date aDate = toDate(_sSQLString);
        Time aTime;
        sal_Int32 nSeparation = _sSQLString.indexOf( ' ' );
        if ( -1 != nSeparation )
            aTime = toTime( _sSQLString.copy( nSeparation ) );

        return DateTime(aTime.NanoSeconds, aTime.Seconds, aTime.Minutes, aTime.Hours,
                        aDate.Day, aDate.Month, aDate.Year);
    }

    //-----------------------------------------------------------------------------
    Time DBTypeConversion::toTime(const OUString& _sSQLString)
    {
        static sal_Unicode sTimeSep = ':';

        sal_Int32 nIndex    = 0;
        sal_uInt16  nHour   = 0,
                    nMinute = 0,
                    nSecond = 0;
        sal_uInt32  nNanoSeconds = 0;
        nHour   = (sal_uInt16)_sSQLString.getToken(0,sTimeSep,nIndex).toInt32();
        if(nIndex != -1)
        {
            nMinute = (sal_uInt16)_sSQLString.getToken(0,sTimeSep,nIndex).toInt32();
            if(nIndex != -1)
            {
                nSecond = (sal_uInt16)_sSQLString.getToken(0,sTimeSep,nIndex).toInt32();
                nIndex = 0;
                OUString sNano(_sSQLString.getToken(1,'.',nIndex));
                nNanoSeconds = sNano.toInt32();
            }
        }
        return Time(nNanoSeconds, nSecond, nMinute, nHour);
    }

//.........................................................................
}   // namespace dbtools
//.........................................................................


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
