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

#include <DateHelper.hxx>
#include <rtl/math.hxx>
#include <com/sun/star/chart/TimeUnit.hpp>

namespace chart
{
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

Date DateHelper::GetDateSomeMonthsAway( const Date& rD, sal_Int32 nMonthDistance )
{
    Date aRet(rD);
    aRet.AddMonths( nMonthDistance );
    return aRet;
}

Date DateHelper::GetDateSomeYearsAway( const Date& rD, sal_Int32 nYearDistance )
{
    Date aRet(rD);
    aRet.AddYears( static_cast<sal_Int16>(nYearDistance) );
    return aRet;
}

bool DateHelper::IsLessThanOneMonthAway( const Date& rD1, const Date& rD2 )
{
    Date aDMin( DateHelper::GetDateSomeMonthsAway( rD1, -1 ) );
    Date aDMax( DateHelper::GetDateSomeMonthsAway( rD1, 1 ) );

    return rD2 > aDMin && rD2 < aDMax;
}

bool DateHelper::IsLessThanOneYearAway( const Date& rD1, const Date& rD2 )
{
    Date aDMin( DateHelper::GetDateSomeYearsAway( rD1, -1 ) );
    Date aDMax( DateHelper::GetDateSomeYearsAway( rD1, 1 ) );

    return rD2 > aDMin && rD2 < aDMax;
}

double DateHelper::RasterizeDateValue( double fValue, const Date& rNullDate, tools::Long TimeResolution )
{
    if (std::isnan(fValue))
        return fValue;

    Date aDate(rNullDate); aDate.AddDays(::rtl::math::approxFloor(fValue));
    switch(TimeResolution)
    {
        case css::chart::TimeUnit::DAY:
            break;
        case css::chart::TimeUnit::YEAR:
            aDate.SetMonth(1);
            aDate.SetDay(1);
            break;
        case css::chart::TimeUnit::MONTH:
        default:
            aDate.SetDay(1);
            break;
    }
    return aDate - rNullDate;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
