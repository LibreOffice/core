/*************************************************************************
 *
 *  $RCSfile: DateConversion.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-27 12:19:34 $
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

#ifndef _CONNECTIVITY_DATECONVERSION_HXX_
#include "connectivity/DateConversion.hxx"
#endif

using namespace connectivity;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::uno;


const double fMilliSecondsPerDay = 86400000.0;
const Date STANDARD_DB_DATE(1,1,1900);
static sal_Int32 aDaysInMonth[12] = { 31, 28, 31, 30, 31, 30,
                                   31, 31, 30, 31, 30, 31 };

#define MAX_DAYS    3636532
//------------------------------------------------------------------------------
sal_Int32 DateConversion::toINT32(const Date& rVal)
{
    return ((sal_Int32)(rVal.Day%100)) +
          (((sal_Int32)(rVal.Month%100))*100) +
          (((sal_Int32) rVal.Year%10000)*10000);
}

//------------------------------------------------------------------------------
sal_Int32 DateConversion::toINT32(const Time& rVal)
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
sal_Int64 DateConversion::toINT64(const DateTime& rVal)
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
sal_Int32 DateConversion::getMsFromTime(const Time& rVal)
{
    sal_Int16   nSign     = (toINT32(rVal) >= 0) ? +1 : -1;
    sal_Int32   nHour     = rVal.Hours;
    sal_Int32   nMin      = rVal.Minutes;
    sal_Int32   nSec      = rVal.Seconds;
    sal_Int32   n100Sec   = rVal.HundredthSeconds;

    return (((nHour*3600000)+(nMin*60000)+(nSec*1000)+(n100Sec*10))*nSign);
}
//------------------------------------------------------------------------------
double DateConversion::toDouble(const Date& rVal)
{
    return (double)toINT32(rVal);
}
//------------------------------------------------------------------------------
double DateConversion::toDouble(const Time& rVal)
{
    return (double)getMsFromTime(rVal) / fMilliSecondsPerDay;
}
//------------------------------------------------------------------------------
double DateConversion::toDouble(const DateTime& rVal)
{
    sal_Int64   nTime     = toINT64(rVal);
    sal_Int16   nSign     = (nTime >= 0) ? +1 : -1;
    sal_Int32   nHour     = rVal.Hours;
    sal_Int32   nMin      = rVal.Minutes;
    sal_Int32   nSec      = rVal.Seconds;
    sal_Int32   n100Sec   = rVal.HundredthSeconds;

    double nVal = (((nHour*3600000)+(nMin*60000)+(nSec*1000)+(n100Sec*10))*nSign);

    return ((double)nTime) + (nVal * (1/fMilliSecondsPerDay));
}
// -------------------------------------------------------------------------
inline sal_Bool ImpIsLeapYear( sal_uInt16 nYear )
{
    return (((nYear % 4) == 0) && ((nYear % 100) != 0) || ((nYear % 400) == 0));
}
// -------------------------------------------------------------------------
inline sal_uInt16 DaysInMonth( sal_uInt16 nMonth, sal_uInt16 nYear )
{
    if ( nMonth != 2 )
        return aDaysInMonth[nMonth-1];
    else
    {
        if ( ((nYear % 4) == 0) && ((nYear % 100) != 0) ||
             ((nYear % 400) == 0) )
            return aDaysInMonth[nMonth-1] + 1;
        else
            return aDaysInMonth[nMonth-1];
    }
}
//------------------------------------------------------------------------------
static void DaysToDate( sal_Int32 nDays,
                        sal_uInt16& rDay, sal_uInt16& rMonth, sal_uInt16& rYear )
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
                if ( (nTempDays != 366) || !ImpIsLeapYear( rYear ) )
                {
                    i--;
                    bCalc = sal_True;
                }
            }
        }
    }
    while ( bCalc );

    rMonth = 1;
    while ( nTempDays > DaysInMonth( rMonth, rYear ) )
    {
        nTempDays -= DaysInMonth( rMonth, rYear );
        rMonth++;
    }
    rDay = nTempDays;
}
// -------------------------------------------------------------------------
static sal_Int32 DateToDays( sal_uInt16 nDay, sal_uInt16 nMonth, sal_uInt16 nYear )
{
    sal_Int32 nDays;

    nDays = ((sal_Int32)nYear-1) * 365;
    nDays += ((nYear-1) / 4) - ((nYear-1) / 100) + ((nYear-1) / 400);
    for( sal_uInt16 i = 1; i < nMonth; i++ )
        nDays += DaysInMonth(i,nYear);
    nDays += nDay;
    return nDays;
}
// -------------------------------------------------------------------------
static void addDays( sal_Int32 nDays ,Date& _rDate)
{
    sal_Int32   nTempDays = DateToDays( _rDate.Day, _rDate.Month, _rDate.Year );

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
        DaysToDate( nTempDays, _rDate.Day, _rDate.Month, _rDate.Year );
}

// -----------------------------------------------------------------------

static void subDays( sal_Int32 nDays,Date& _rDate )
{
    sal_Int32   nTempDays = DateToDays( _rDate.Day, _rDate.Month, _rDate.Year );

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
        DaysToDate( nTempDays, _rDate.Day, _rDate.Month, _rDate.Year );
}

// -------------------------------------------------------------------------
Date DateConversion::toDate(double dVal,const Date& _rSTANDARD_DB_DATE)
{
    Date aRet = _rSTANDARD_DB_DATE;
    //  DaysToDate((sal_Int32)dVal,aRet.Day,aRet.Month,aRet.Year);

    if (dVal >= 0)
        addDays((sal_Int32)dVal,aRet);
    else
        subDays((sal_uInt32)(-dVal),aRet);
        //  x -= (sal_uInt32)(-nDays);

    return aRet;
}
//------------------------------------------------------------------------------
void MakeTimeFromMS( sal_Int32 nMS )
{
    sal_uInt16 nSign;
    if ( nMS < 0 )
    {
        nMS *= -1;
        nSign = -1;
    }
    else
        nSign = 1;

    // Zeit normalisieren
    sal_uInt16 n100Sec  = nMS/10;
    sal_uInt16 nSec     = n100Sec / 100;
    n100Sec  = n100Sec % 100;
    sal_uInt16 nMin     = nSec / 60;
    nSec                = nSec % 60;
    sal_uInt16 nHour    = nMin / 60;
    nMin                = nMin % 60;

    // Zeit zusammenbauen
    sal_Int32 nTime = (sal_Int32)(n100Sec + (nSec*100) + (nMin*10000) + (nHour*1000000)) * nSign;


}
// -------------------------------------------------------------------------
Time        DateConversion::toTime(double dVal)
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
    xRet.HundredthSeconds   = nMS/10;
    xRet.Seconds            = xRet.HundredthSeconds / 100;
    xRet.HundredthSeconds   = xRet.HundredthSeconds % 100;
    xRet.Minutes            = xRet.Seconds / 60;
    xRet.Seconds            = xRet.Seconds % 60;
    xRet.Hours              = xRet.Minutes / 60;
    xRet.Minutes            = xRet.Minutes % 60;

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
DateTime    DateConversion::toDateTime(double dVal,const Date& _rSTANDARD_DB_DATE)
{
    Date aDate = DateConversion::toDate(dVal,_rSTANDARD_DB_DATE);
    Time aTime = DateConversion::toTime(dVal);


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

