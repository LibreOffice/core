/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <Scaling.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <cppuhelper/supportsservice.hxx>

#include <cmath>
#include <limits>

namespace com::sun::star::uno { class XComponentContext; }

namespace
{

constexpr OUString lcl_aServiceName_Logarithmic = u"com.sun.star.chart2.LogarithmicScaling"_ustr;
constexpr OUString lcl_aServiceName_Exponential = u"com.sun.star.chart2.ExponentialScaling"_ustr;
constexpr OUString lcl_aServiceName_Linear = u"com.sun.star.chart2.LinearScaling"_ustr;
constexpr OUString lcl_aServiceName_Power = u"com.sun.star.chart2.PowerScaling"_ustr;

}

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

LogarithmicScaling::LogarithmicScaling() :
        m_fBase( 10.0 ),
        m_fLogOfBase( log( 10.0 ) )
{
}

LogarithmicScaling::LogarithmicScaling( double fBase ) :
        m_fBase( fBase ),
        m_fLogOfBase( log( fBase ) )
{
}

LogarithmicScaling::~LogarithmicScaling()
{
}

double LogarithmicScaling::doScaling( double value )
{
    if( std::isnan( value ) || std::isinf( value ) )
        return std::numeric_limits<double>::quiet_NaN();
    return std::log( value ) / m_fLogOfBase;
}

uno::Reference< XScaling > LogarithmicScaling::getInverseScaling()
{
    return new ExponentialScaling( m_fBase );
}

OUString LogarithmicScaling::getServiceName()
{
    return lcl_aServiceName_Logarithmic;
}

OUString LogarithmicScaling::getImplementationName()
{
    return lcl_aServiceName_Logarithmic;
}

bool LogarithmicScaling::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

cpo::uno::Sequence< OUString > LogarithmicScaling::getSupportedServiceNames()
{
    return { lcl_aServiceName_Logarithmic };
}

ExponentialScaling::ExponentialScaling() :
        m_fBase( 10.0 )
{
}

ExponentialScaling::ExponentialScaling( double fBase ) :
        m_fBase( fBase )
{
}

ExponentialScaling::~ExponentialScaling()
{
}

double ExponentialScaling::doScaling( double value )
{
    if( std::isnan( value ) || std::isinf( value ) )
        return std::numeric_limits<double>::quiet_NaN();
    return std::pow( m_fBase, value );
}

uno::Reference< XScaling > ExponentialScaling::getInverseScaling()
{
    return new LogarithmicScaling( m_fBase );
}

OUString ExponentialScaling::getServiceName()
{
    return lcl_aServiceName_Exponential;
}

OUString ExponentialScaling::getImplementationName()
{
    return lcl_aServiceName_Exponential;
}

bool ExponentialScaling::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

cpo::uno::Sequence< OUString > ExponentialScaling::getSupportedServiceNames()
{
    return { lcl_aServiceName_Exponential };
}

LinearScaling::LinearScaling() :
        m_fSlope( 1.0 ),
        m_fOffset( 0.0 )
{}

LinearScaling::LinearScaling( double fSlope, double fOffset ) :
        m_fSlope( fSlope ),
        m_fOffset( fOffset )
{}

LinearScaling::~LinearScaling()
{}

double LinearScaling::doScaling( double value )
{
    if( std::isnan( value ) || std::isinf( value ) )
        return std::numeric_limits<double>::quiet_NaN();
    return m_fOffset + m_fSlope * value;
}

uno::Reference< XScaling >
    LinearScaling::getInverseScaling()
{
    // ToDo: ApproxEqual ?
    if( m_fSlope == 0 )
        throw uno::RuntimeException(u"Divide by zero exception"_ustr);

    return new LinearScaling( 1.0 / m_fSlope, m_fOffset / m_fSlope );
}

OUString LinearScaling::getServiceName()
{
    return lcl_aServiceName_Linear;
}

OUString LinearScaling::getImplementationName()
{
    return lcl_aServiceName_Linear ;
}

bool LinearScaling::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

cpo::uno::Sequence< OUString > LinearScaling::getSupportedServiceNames()
{
    return { lcl_aServiceName_Linear };
}

PowerScaling::PowerScaling() :
        m_fExponent( 10.0 )
{}

PowerScaling::PowerScaling( double fExponent ) :
        m_fExponent( fExponent )
{}

PowerScaling::~PowerScaling()
{}

double PowerScaling::doScaling( double value )
{
    if( std::isnan( value ) || std::isinf( value ) )
        return std::numeric_limits<double>::quiet_NaN();
    return std::pow( value, m_fExponent );
}

uno::Reference< XScaling >
    PowerScaling::getInverseScaling()
{
    // ToDo: ApproxEqual ?
    if( m_fExponent == 0 )
        throw uno::RuntimeException(u"Divide by zero exception"_ustr);

    return new PowerScaling( 1.0 / m_fExponent );
}

    OUString
PowerScaling::getServiceName()
{
    return lcl_aServiceName_Power;
}

OUString PowerScaling::getImplementationName()
{
    return lcl_aServiceName_Power;
}

bool PowerScaling::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

cpo::uno::Sequence< OUString > PowerScaling::getSupportedServiceNames()
{
    return { lcl_aServiceName_Power };
}

} //namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_chart2_LinearScaling_get_implementation(css::uno::XComponentContext *,
        cpo::uno::Sequence<cpo::uno::Any> const &)
{
    return cppu::acquire(new chart::LinearScaling );
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_chart2_ExponentialScaling_get_implementation(css::uno::XComponentContext *,
        cpo::uno::Sequence<cpo::uno::Any> const &)
{
    return cppu::acquire(new chart::ExponentialScaling );
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_chart2_LogarithmicScaling_get_implementation(css::uno::XComponentContext *,
        cpo::uno::Sequence<cpo::uno::Any> const &)
{
    return cppu::acquire(new chart::LogarithmicScaling );
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_chart2_PowerScaling_get_implementation(css::uno::XComponentContext *,
        cpo::uno::Sequence<cpo::uno::Any> const &)
{
    return cppu::acquire(new chart::PowerScaling );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
