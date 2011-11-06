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
#include "DateScaling.hxx"
#include <com/sun/star/chart/TimeUnit.hpp>
#include <rtl/math.hxx>
#include "com/sun/star/uno/RuntimeException.hpp"

namespace
{

static const ::rtl::OUString lcl_aServiceName_DateScaling(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.DateScaling" ));
static const ::rtl::OUString lcl_aServiceName_InverseDateScaling(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.InverseDateScaling" ));

static const ::rtl::OUString lcl_aImplementationName_DateScaling(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart2.DateScaling" ));
static const ::rtl::OUString lcl_aImplementationName_InverseDateScaling(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart2.InverseDateScaling" ));

static const double lcl_fNumberOfMonths = 12.0;//todo: this needs to be offered by basic tools Date class if it should be more generic
}

//.............................................................................
namespace chart
{
//.............................................................................
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
    throw (uno::RuntimeException)
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
                fResult *= lcl_fNumberOfMonths;//asssuming equal count of months in each year
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
    throw (uno::RuntimeException)
{
    return new InverseDateScaling( m_aNullDate, m_nTimeUnit, m_bShifted );
}

::rtl::OUString SAL_CALL DateScaling::getServiceName()
    throw (uno::RuntimeException)
{
    return lcl_aServiceName_DateScaling;
}

uno::Sequence< ::rtl::OUString > DateScaling::getSupportedServiceNames_Static()
{
    return uno::Sequence< ::rtl::OUString >( & lcl_aServiceName_DateScaling, 1 );
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( DateScaling, lcl_aServiceName_DateScaling )

// ----------------------------------------

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
    throw (uno::RuntimeException)
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
                Date aDate;
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
    throw (uno::RuntimeException)
{
    return new DateScaling( m_aNullDate, m_nTimeUnit, m_bShifted );
}

::rtl::OUString SAL_CALL InverseDateScaling::getServiceName()
    throw (uno::RuntimeException)
{
    return lcl_aServiceName_InverseDateScaling;
}

uno::Sequence< ::rtl::OUString > InverseDateScaling::getSupportedServiceNames_Static()
{
    return uno::Sequence< ::rtl::OUString >( & lcl_aServiceName_InverseDateScaling, 1 );
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( InverseDateScaling, lcl_aServiceName_InverseDateScaling )

//.............................................................................
} //namespace chart
//.............................................................................
