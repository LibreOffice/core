/*************************************************************************
 *
 *  $RCSfile: dbconversion.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:38:14 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBHELPER_DBCONVERSION_HXX_
#include <connectivity/dbconversion.hxx>
#endif
#ifndef _COMPHELPER_DATETIME_HXX_
#include <comphelper/datetime.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _INC_STDIO
#include <stdio.h>
#endif



#define MAX_DAYS    3636532

//.........................................................................
namespace dbtools
{
//.........................................................................


using namespace ::comphelper;
//  using namespace ::connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::sdb;
//  using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;


//------------------------------------------------------------------------------
::com::sun::star::util::Date DBTypeConversion::getStandardDate()
{
    static ::com::sun::star::util::Date STANDARD_DB_DATE(1,1,1900);
    return STANDARD_DB_DATE;
}
//------------------------------------------------------------------------------
::rtl::OUString DBTypeConversion::toDateString(const Date& rDate)
{
    sal_Char s[11];
    snprintf(s,
            sizeof(s),
            "%04d-%02d-%02d",
            (int)rDate.Year,
            (int)rDate.Month,
            (int)rDate.Day);
    s[10] = 0;
    return ::rtl::OUString::createFromAscii(s);
}
//------------------------------------------------------------------
::rtl::OUString DBTypeConversion::toTimeString(const Time& rTime)
{
    sal_Char s[9];
    snprintf(s,
            sizeof(s),
            "%02d:%02d:%02d",
            (int)rTime.Hours,
            (int)rTime.Minutes,
            (int)rTime.Seconds);
    s[8] = 0;
    return ::rtl::OUString::createFromAscii(s);
}

//------------------------------------------------------------------
::rtl::OUString DBTypeConversion::toDateTimeString(const DateTime& _rDateTime)
{
    Date aDate(_rDateTime.Day,_rDateTime.Month,_rDateTime.Year);
    ::rtl::OUString aTemp(toDateString(aDate));
    aTemp += ::rtl::OUString::createFromAscii(" ");
    Time aTime(0,_rDateTime.Seconds,_rDateTime.Minutes,_rDateTime.Hours);
    aTemp += toTimeString(aTime);
    return  aTemp;
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
Time DBTypeConversion::toTime(sal_Int32 _nVal)
{
    Time aReturn;
    aReturn.Hours = (sal_uInt16)(((sal_uInt32)(_nVal >= 0 ? _nVal : _nVal*-1)) / 1000000);
    aReturn.Minutes = (sal_uInt16)((((sal_uInt32)(_nVal >= 0 ? _nVal : _nVal*-1)) / 10000) % 100);
    aReturn.Seconds = (sal_uInt16)((((sal_uInt32)(_nVal >= 0 ? _nVal : _nVal*-1)) / 100) % 100);
    aReturn.HundredthSeconds = (sal_uInt16)(((sal_uInt32)(_nVal >= 0 ? _nVal : _nVal*-1)) % 100);
    return aReturn;
}

const double fMilliSecondsPerDay = 86400000.0;
//------------------------------------------------------------------------------
sal_Int32 DBTypeConversion::toINT32(const Date& rVal)
{
    return ((sal_Int32)(rVal.Day%100)) +
          (((sal_Int32)(rVal.Month%100))*100) +
          (((sal_Int32) rVal.Year%10000)*10000);
}

//------------------------------------------------------------------------------
sal_Int32 DBTypeConversion::toINT32(const Time& rVal)
{
    // Zeit normalisieren
    sal_Int32 nSeconds          = rVal.Seconds + rVal.HundredthSeconds / 100;
    sal_Int32 nHundredthSeconds = rVal.HundredthSeconds % 100;
    sal_Int32 nMinutes          = rVal.Minutes + nSeconds / 60;
    nSeconds                    = nSeconds % 60;
    sal_Int32 nHours            = rVal.Hours + nMinutes / 60;
    nMinutes                    = nMinutes % 60;

    // Zeit zusammenbauen
    return (sal_Int32)(nHundredthSeconds + (nSeconds*100) + (nMinutes*10000) + (nHours*1000000));
}

//------------------------------------------------------------------------------
sal_Int64 DBTypeConversion::toINT64(const DateTime& rVal)
{
    // Zeit normalisieren
    sal_Int32 nSeconds          = rVal.Seconds + rVal.HundredthSeconds / 100;
    sal_Int32 nHundredthSeconds = rVal.HundredthSeconds % 100;
    sal_Int32 nMinutes          = rVal.Minutes + nSeconds / 60;
    nSeconds                    = nSeconds % 60;
    sal_Int32 nHours            = rVal.Hours + nMinutes / 60;
    nMinutes                    = nMinutes % 60;

    // Zeit zusammenbauen
    sal_Int32 nTime = (sal_Int32)(nHundredthSeconds + (nSeconds*100) + (nMinutes*10000) + (nHours*1000000));
    sal_Int32 nDate = ((sal_Int32)(rVal.Day%100)) + (((sal_Int32)(rVal.Month%100))*100) + (((sal_Int32) rVal.Year%10000)*10000);
    sal_Int64 nRet;

    nRet = (sal_Int64) nTime;
    nRet <<= 32;
    nRet += nDate;

    return nRet;
}

//------------------------------------------------------------------------------
sal_Int32 DBTypeConversion::getMsFromTime(const Time& rVal)
{
    sal_Int16   nSign     = (toINT32(rVal) >= 0) ? +1 : -1;
    sal_Int32   nHour     = rVal.Hours;
    sal_Int32   nMin      = rVal.Minutes;
    sal_Int32   nSec      = rVal.Seconds;
    sal_Int32   n100Sec   = rVal.HundredthSeconds;

    return (((nHour*3600000)+(nMin*60000)+(nSec*1000)+(n100Sec*10))*nSign);
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
static void implBuildFromRelative( sal_Int32 nDays, sal_uInt16& rDay, sal_uInt16& rMonth, sal_uInt16& rYear)
{
    sal_Int32   nTempDays;
    sal_Int32   i = 0;
    sal_Bool    bCalc;

    do
    {
        nTempDays = nDays;
        rYear = ((nTempDays / 365) - i);
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
    rDay = nTempDays;
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
    return (double)getMsFromTime(rVal) / fMilliSecondsPerDay;
}

//------------------------------------------------------------------------------
double DBTypeConversion::toDouble(const DateTime& _rVal, const Date& _rNullDate)
{
    sal_Int64   nTime     = toDays(Date(_rVal.Day, _rVal.Month, _rVal.Year), _rNullDate);
    sal_Int16   nSign     = (nTime >= 0) ? +1 : -1;
    sal_Int32   nHour     = _rVal.Hours;
    sal_Int32   nMin      = _rVal.Minutes;
    sal_Int32   nSec      = _rVal.Seconds;
    sal_Int32   n100Sec   = _rVal.HundredthSeconds;

    double nVal = (((nHour*3600000)+(nMin*60000)+(nSec*1000)+(n100Sec*10))*nSign);

    return ((double)nTime) + (nVal * (1/fMilliSecondsPerDay));
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
    sal_Int32 nMS = sal_Int32((dVal - (double)nDays) * fMilliSecondsPerDay + 0.5);

    sal_uInt16 nSign;
    if ( nMS < 0 )
    {
        nMS *= -1;
        nSign = -1;
    }
    else
        nSign = 1;

    Time xRet;
    // Zeit normalisieren
    // we have to sal_Int32 here because otherwise we get an overflow
    sal_Int32 nHundredthSeconds = nMS/10;
    sal_Int32 nSeconds          = nHundredthSeconds / 100;
    sal_Int32 nMinutes          = nSeconds / 60;

    xRet.HundredthSeconds       = nHundredthSeconds % 100;
    xRet.Seconds                = nSeconds % 60;
    xRet.Hours                  = nMinutes / 60;
    xRet.Minutes                = nMinutes % 60;

    // Zeit zusammenbauen
    sal_Int32 nTime = (sal_Int32)(xRet.HundredthSeconds + (xRet.Seconds*100) + (xRet.Minutes*10000) + (xRet.Hours*1000000)) * nSign;

    if(nTime < 0)
    {
        xRet.HundredthSeconds   = 99;
        xRet.Minutes            = 59;
        xRet.Seconds            = 59;
        xRet.Hours              = 23;
    }
    return xRet;
}
//------------------------------------------------------------------------------
DateTime DBTypeConversion::toDateTime(double dVal, const Date& _rNullDate)
{
    Date aDate = toDate(dVal, _rNullDate);
    Time aTime = toTime(dVal);

    DateTime xRet;

    xRet.Day                = aDate.Day;
    xRet.Month              = aDate.Month;
    xRet.Year               = aDate.Year;

    xRet.HundredthSeconds   = aTime.HundredthSeconds;
    xRet.Minutes            = aTime.Minutes;
    xRet.Seconds            = aTime.Seconds;
    xRet.Hours              = aTime.Hours;


    return xRet;
}
//------------------------------------------------------------------------------
Date DBTypeConversion::toDate(const ::rtl::OUString& _sSQLString)
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
DateTime DBTypeConversion::toDateTime(const ::rtl::OUString& _sSQLString)
{
    Date aDate = toDate(_sSQLString);
    Time aTime = toTime(_sSQLString);

    return DateTime(0,aTime.Seconds,aTime.Minutes,aTime.Hours,aDate.Day,aDate.Month,aDate.Year);
}

//-----------------------------------------------------------------------------
Time DBTypeConversion::toTime(const ::rtl::OUString& _sSQLString)
{
    static sal_Unicode sTimeSep = ':';

    sal_Int32 nIndex    = 0;
    sal_uInt16  nHour   = 0,
                nMinute = 0,
                nSecond = 0;
    nHour   = (sal_uInt16)_sSQLString.getToken(0,sTimeSep,nIndex).toInt32();
    if(nIndex != -1)
    {
        nMinute = (sal_uInt16)_sSQLString.getToken(0,sTimeSep,nIndex).toInt32();
        if(nIndex != -1)
            nSecond = (sal_uInt16)_sSQLString.getToken(0,sTimeSep,nIndex).toInt32();
    }
    return Time(0,nSecond,nMinute,nHour);
}
// -----------------------------------------------------------------------------


//.........................................................................
}   // namespace dbtools
//.........................................................................


