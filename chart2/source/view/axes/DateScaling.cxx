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
#include "com/sun/star/uno/RuntimeException.hpp"
#include <cppuhelper/supportsservice.hxx>

namespace
{

static const char lcl_aServiceName_DateScaling[] = "com.sun.star.chart2.DateScaling";
static const char lcl_aServiceName_InverseDateScaling[] = "com.sun.star.chart2.InverseDateScaling";

static const double lcl_fNumberOfMonths = 12.0;//todo: this needs to be offered by basic tools Date class if it should be more generic
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
    throw (uno::RuntimeException, std::exception)
{
    double fResult(value);
    if( ::rtl::math::isNan( value ) || ::rtl::math::isInf( value ) )
        ::rtl::math::setNan( & fResult );
    else
    {
        Date aDate(m_aNullDate);
        aDate += static_cast<long>(::rtl::math::approxFloor(value));
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
                    if( YEAR==m_nTimeUnit )
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
    throw (uno::RuntimeException, std::exception)
{
    return new InverseDateScaling( m_aNullDate, m_nTimeUnit, m_bShifted );
}

OUString SAL_CALL DateScaling::getServiceName()
    throw (uno::RuntimeException, std::exception)
{
    return OUString(lcl_aServiceName_DateScaling);
}

uno::Sequence< OUString > DateScaling::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aSeq { lcl_aServiceName_DateScaling };
    return aSeq;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
OUString SAL_CALL DateScaling::getImplementationName()
    throw( css::uno::RuntimeException, std::exception )
{
    return getImplementationName_Static();
}

OUString DateScaling::getImplementationName_Static()
{
    return OUString(lcl_aServiceName_DateScaling);
}

sal_Bool SAL_CALL DateScaling::supportsService( const OUString& rServiceName )
    throw( css::uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL DateScaling::getSupportedServiceNames()
    throw( css::uno::RuntimeException, std::exception )
{
    return getSupportedServiceNames_Static();
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
    throw (uno::RuntimeException, std::exception)
{
    double fResult(value);
    if( ::rtl::math::isNan( value ) || ::rtl::math::isInf( value ) )
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
                    if( YEAR==m_nTimeUnit )
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
    throw (uno::RuntimeException, std::exception)
{
    return new DateScaling( m_aNullDate, m_nTimeUnit, m_bShifted );
}

OUString SAL_CALL InverseDateScaling::getServiceName()
    throw (uno::RuntimeException, std::exception)
{
    return OUString(lcl_aServiceName_InverseDateScaling);
}

uno::Sequence< OUString > InverseDateScaling::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aSeq( 1 );
    aSeq.getArray()[0] = lcl_aServiceName_InverseDateScaling;
    return aSeq;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
OUString SAL_CALL InverseDateScaling::getImplementationName()
    throw( css::uno::RuntimeException, std::exception )
{
    return getImplementationName_Static();
}

OUString InverseDateScaling::getImplementationName_Static()
{
    return OUString(lcl_aServiceName_InverseDateScaling);
}

sal_Bool SAL_CALL InverseDateScaling::supportsService( const OUString& rServiceName )
    throw( css::uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL InverseDateScaling::getSupportedServiceNames()
    throw( css::uno::RuntimeException, std::exception )
{
    return getSupportedServiceNames_Static();
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
