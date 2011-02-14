/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
    aRet.SetYear( static_cast<sal_uInt16>(rD.GetYear()+nYearDistance) );
    while(!aRet.IsValid())
        aRet--;
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
