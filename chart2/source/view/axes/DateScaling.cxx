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

#include "DateScaling.hxx"
#include <com/sun/star/chart/TimeUnit.hpp>
#include <rtl/math.hxx>
#include <cppuhelper/supportsservice.hxx>

namespace
{

constexpr OUStringLiteral lcl_aServiceName_DateScaling = u"com.sun.star.chart2.DateScaling";
constexpr OUStringLiteral lcl_aServiceName_InverseDateScaling
    = u"com.sun.star.chart2.InverseDateScaling";

const double lcl_fNumberOfMonths = 12.0;//todo: this needs to be offered by basic tools Date class if it should be more generic
}

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::chart::TimeUnit::DAY;
using ::com::sun::star::chart::TimeUnit::MONTH;
using ::com::sun::star::chart::TimeUnit::YEAR;

DateScaling::DateScaling( const Date& rNullDate, sal_Int32 nTimeUnit, bool bShifted )
        : m_aNullDate( rNullDate )
        , m_nTimeUnit( nTimeUnit )
        , m_bShifted( bShifted )
{
}

DateScaling::~DateScaling()
{
}

double SAL_CALL DateScaling::doScaling( double value )
{
    double fResult(value);
    if( std::isnan( value ) || std::isinf( value ) )
        ::rtl::math::setNan( & fResult );
    else
    {
        Date aDate(m_aNullDate);
        aDate.AddDays(::rtl::math::approxFloor(value));
        switch( m_nTimeUnit )
        {
            case DAY:
                fResult = value;
                if(m_bShifted)
                    fResult+=0.5;
                break;
            case YEAR:
            case MONTH:
            default:
                fResult = aDate.GetYear();
                fResult *= lcl_fNumberOfMonths;//assuming equal count of months in each year
                fResult += aDate.GetMonth();

                double fDayOfMonth = aDate.GetDay();
                fDayOfMonth -= 1.0;
                double fDaysInMonth = aDate.GetDaysInMonth();
                fResult += fDayOfMonth/fDaysInMonth;
                if(m_bShifted)
                {
                    if( m_nTimeUnit==YEAR )
                        fResult += 0.5*lcl_fNumberOfMonths;
                    else
                        fResult += 0.5;
                }
                break;
        }
    }
    return fResult;
}

uno::Reference< XScaling > SAL_CALL DateScaling::getInverseScaling()
{
    return new InverseDateScaling( m_aNullDate, m_nTimeUnit, m_bShifted );
}

OUString SAL_CALL DateScaling::getServiceName()
{
    return lcl_aServiceName_DateScaling;
}

OUString SAL_CALL DateScaling::getImplementationName()
{
    return lcl_aServiceName_DateScaling;
}

sal_Bool SAL_CALL DateScaling::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL DateScaling::getSupportedServiceNames()
{
    return { lcl_aServiceName_DateScaling };
}

InverseDateScaling::InverseDateScaling( const Date& rNullDate, sal_Int32 nTimeUnit, bool bShifted )
        : m_aNullDate( rNullDate )
        , m_nTimeUnit( nTimeUnit )
        , m_bShifted( bShifted )
{
}

InverseDateScaling::~InverseDateScaling()
{
}

double SAL_CALL InverseDateScaling::doScaling( double value )
{
    double fResult(value);
    if( std::isnan( value ) || std::isinf( value ) )
        ::rtl::math::setNan( & fResult );
    else
    {
        switch( m_nTimeUnit )
        {
            case DAY:
                if(m_bShifted)
                    value -= 0.5;
                fResult = value;
                break;
            case YEAR:
            case MONTH:
            default:
                //Date aDate(m_aNullDate);
                if(m_bShifted)
                {
                    if( m_nTimeUnit==YEAR )
                        value -= 0.5*lcl_fNumberOfMonths;
                    else
                        value -= 0.5;
                }
                Date aDate( Date::EMPTY );
                double fYear = ::rtl::math::approxFloor(value/lcl_fNumberOfMonths);
                double fMonth = ::rtl::math::approxFloor(value-(fYear*lcl_fNumberOfMonths));
                if( fMonth==0.0 )
                {
                    fYear--;
                    fMonth=12.0;
                }
                aDate.SetYear( static_cast<sal_uInt16>(fYear) );
                aDate.SetMonth( static_cast<sal_uInt16>(fMonth) );
                aDate.SetDay( 1 );
                double fMonthCount = (fYear*lcl_fNumberOfMonths)+fMonth;
                double fDay = (value-fMonthCount)*aDate.GetDaysInMonth();
                fDay += 1.0;
                aDate.SetDay( static_cast<sal_uInt16>(::rtl::math::round(fDay)) );
                fResult = aDate - m_aNullDate;
                break;
        }
    }
    return fResult;
}

uno::Reference< XScaling > SAL_CALL InverseDateScaling::getInverseScaling()
{
    return new DateScaling( m_aNullDate, m_nTimeUnit, m_bShifted );
}

OUString SAL_CALL InverseDateScaling::getServiceName()
{
    return lcl_aServiceName_InverseDateScaling;
}

OUString SAL_CALL InverseDateScaling::getImplementationName()
{
    return lcl_aServiceName_InverseDateScaling;
}

sal_Bool SAL_CALL InverseDateScaling::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL InverseDateScaling::getSupportedServiceNames()
{
    return { lcl_aServiceName_InverseDateScaling };
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
