/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "DateHelper.hxx"
#include "DateScaling.hxx"
#include <rtl/math.hxx>
#include <com/sun/star/chart/TimeUnit.hpp>

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;

bool DateHelper::IsInSameYear( const Date& rD1, const Date& rD2 )
{
    return rD1.GetYear() == rD2.GetYear();
}
bool DateHelper::IsInSameMonth( const Date& rD1, const Date& rD2 )
{
    return (rD1.GetYear() == rD2.GetYear())
        && (rD1.GetMonth() == rD2.GetMonth());
}
long DateHelper::GetMonthsBetweenDates( Date aD1, Date aD2 )
{
    Date aHelp = aD1;
    long nSign = 1;
    if( aD1 < aD2 )
    {
        aD1 = aD2;
        aD2 = aHelp;
        nSign = -1;
    }

    return nSign*( ( aD1.GetMonth() - aD2.GetMonth() )
        + ( aD1.GetYear() - aD2.GetYear() )*12 );
}

Date DateHelper::GetDateSomeMonthsAway( const Date& rD, long nMonthDistance )
{
    Date aRet(rD);
    long nMonth = rD.GetMonth()+nMonthDistance;
    long nNewMonth = nMonth%12;
    long nNewYear = rD.GetYear() + nMonth/12;
    if( nMonth <= 0 || !nNewMonth )
        nNewYear--;
    if( nNewMonth <= 0 )
        nNewMonth += 12;
    aRet.SetMonth( sal_uInt16(nNewMonth) );
    aRet.SetYear( sal_uInt16(nNewYear) );
    while(!aRet.IsValid())
        aRet--;
    return aRet;
}

Date DateHelper::GetDateSomeYearsAway( const Date& rD, long nYearDistance )
{
    Date aRet(rD);
    const long nFutureYear (rD.GetYear()+nYearDistance);
    aRet.SetYear(static_cast<sal_uInt16>(nFutureYear));
    if ( ! aRet.IsValid())
    {
        // The Date class has the nasty property to store years modulo
        // 10000.  In order to handle (probably invalid) very large
        // year values more gracefully than with an infinite loop we
        // check that condition and return an invalid date.
        if (nFutureYear < 10000)
        {
            while ( ! aRet.IsValid())
                --aRet;
        }
    }
    return aRet;
}

bool DateHelper::IsLessThanOneMonthAway( const Date& rD1, const Date& rD2 )
{
    Date aDMin( DateHelper::GetDateSomeMonthsAway( rD1, -1 ) );
    Date aDMax( DateHelper::GetDateSomeMonthsAway( rD1, 1 ) );

    if( rD2 > aDMin && rD2 < aDMax )
        return true;
    return false;
}

bool DateHelper::IsLessThanOneYearAway( const Date& rD1, const Date& rD2 )
{
    Date aDMin( DateHelper::GetDateSomeYearsAway( rD1, -1 ) );
    Date aDMax( DateHelper::GetDateSomeYearsAway( rD1, 1 ) );

    if( rD2 > aDMin && rD2 < aDMax )
        return true;
    return false;
}

double DateHelper::RasterizeDateValue( double fValue, const Date& rNullDate, long TimeResolution )
{
    Date aDate(rNullDate); aDate += static_cast<long>(::rtl::math::approxFloor(fValue));
    switch(TimeResolution)
    {
        case ::com::sun::star::chart::TimeUnit::DAY:
            break;
        case ::com::sun::star::chart::TimeUnit::YEAR:
            aDate.SetMonth(1);
            aDate.SetDay(1);
            break;
        case ::com::sun::star::chart::TimeUnit::MONTH:
        default:
            aDate.SetDay(1);
            break;
    }
    return aDate - rNullDate;
}

//.............................................................................
} //namespace chart
//.............................................................................
