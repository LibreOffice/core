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

#include <Scaling.hxx>
#include <rtl/math.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <cppuhelper/supportsservice.hxx>

namespace com::sun::star::uno { class XComponentContext; }

namespace
{

const OUStringLiteral lcl_aServiceName_Logarithmic = u"com.sun.star.chart2.LogarithmicScaling";
const OUStringLiteral lcl_aServiceName_Exponential = u"com.sun.star.chart2.ExponentialScaling";
const OUStringLiteral lcl_aServiceName_Linear = u"com.sun.star.chart2.LinearScaling";
const OUStringLiteral lcl_aServiceName_Power = u"com.sun.star.chart2.PowerScaling";

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

double SAL_CALL LogarithmicScaling::doScaling( double value )
{
    double fResult;
    if( std::isnan( value ) || std::isinf( value ) )
        ::rtl::math::setNan( & fResult );
    else
        fResult = log( value ) / m_fLogOfBase;
    return fResult;
}

uno::Reference< XScaling > SAL_CALL LogarithmicScaling::getInverseScaling()
{
    return new ExponentialScaling( m_fBase );
}

OUString SAL_CALL LogarithmicScaling::getServiceName()
{
    return lcl_aServiceName_Logarithmic;
}

OUString SAL_CALL LogarithmicScaling::getImplementationName()
{
    return lcl_aServiceName_Logarithmic;
}

sal_Bool SAL_CALL LogarithmicScaling::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL LogarithmicScaling::getSupportedServiceNames()
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

double SAL_CALL ExponentialScaling::doScaling( double value )
{
    double fResult;
    if( std::isnan( value ) || std::isinf( value ) )
        ::rtl::math::setNan( & fResult );
    else
        fResult = pow( m_fBase, value );
    return fResult;
}

uno::Reference< XScaling > SAL_CALL ExponentialScaling::getInverseScaling()
{
    return new LogarithmicScaling( m_fBase );
}

OUString SAL_CALL ExponentialScaling::getServiceName()
{
    return lcl_aServiceName_Exponential;
}

OUString SAL_CALL ExponentialScaling::getImplementationName()
{
    return lcl_aServiceName_Exponential;
}

sal_Bool SAL_CALL ExponentialScaling::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL ExponentialScaling::getSupportedServiceNames()
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

double SAL_CALL LinearScaling::doScaling( double value )
{
    double fResult;
    if( std::isnan( value ) || std::isinf( value ) )
        ::rtl::math::setNan( & fResult );
    else
        fResult = m_fOffset + m_fSlope * value;
    return fResult;
}

uno::Reference< XScaling > SAL_CALL
    LinearScaling::getInverseScaling()
{
    // ToDo: ApproxEqual ?
    if( m_fSlope == 0 )
        throw uno::RuntimeException();

    return new LinearScaling( 1.0 / m_fSlope, m_fOffset / m_fSlope );
}

OUString SAL_CALL LinearScaling::getServiceName()
{
    return lcl_aServiceName_Linear;
}

OUString SAL_CALL LinearScaling::getImplementationName()
{
    return lcl_aServiceName_Linear ;
}

sal_Bool SAL_CALL LinearScaling::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL LinearScaling::getSupportedServiceNames()
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

double SAL_CALL PowerScaling::doScaling( double value )
{
    double fResult;
    if( std::isnan( value ) || std::isinf( value ) )
        ::rtl::math::setNan( & fResult );
    else
        fResult = pow( value, m_fExponent );
    return fResult;
}

uno::Reference< XScaling > SAL_CALL
    PowerScaling::getInverseScaling()
{
    // ToDo: ApproxEqual ?
    if( m_fExponent == 0 )
        throw uno::RuntimeException();

    return new PowerScaling( 1.0 / m_fExponent );
}

    OUString SAL_CALL
PowerScaling::getServiceName()
{
    return lcl_aServiceName_Power;
}

OUString SAL_CALL PowerScaling::getImplementationName()
{
    return lcl_aServiceName_Power;
}

sal_Bool SAL_CALL PowerScaling::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL PowerScaling::getSupportedServiceNames()
{
    return { lcl_aServiceName_Power };
}

} //namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_chart2_LinearScaling_get_implementation(css::uno::XComponentContext *,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new chart::LinearScaling );
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_chart2_ExponentialScaling_get_implementation(css::uno::XComponentContext *,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new chart::ExponentialScaling );
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_chart2_LogarithmicScaling_get_implementation(css::uno::XComponentContext *,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new chart::LogarithmicScaling );
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_chart2_PowerScaling_get_implementation(css::uno::XComponentContext *,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new chart::PowerScaling );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
