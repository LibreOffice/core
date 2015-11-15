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

#include "Scaling.hxx"
#include <rtl/math.hxx>
#include "com/sun/star/uno/RuntimeException.hpp"
#include <cppuhelper/supportsservice.hxx>

namespace
{

static const char lcl_aServiceName_Logarithmic[] = "com.sun.star.chart2.LogarithmicScaling";
static const char lcl_aServiceName_Exponential[] = "com.sun.star.chart2.ExponentialScaling";
static const char lcl_aServiceName_Linear[] = "com.sun.star.chart2.LinearScaling";
static const char lcl_aServiceName_Power[] = "com.sun.star.chart2.PowerScaling";

}

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

LogarithmicScaling::LogarithmicScaling( const uno::Reference< uno::XComponentContext > & xContext ) :
        m_fBase( 10.0 ),
        m_fLogOfBase( log( 10.0 ) ),
        m_xContext( xContext )
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
    throw (uno::RuntimeException, std::exception)
{
    double fResult;
    if( ::rtl::math::isNan( value ) || ::rtl::math::isInf( value ) )
        ::rtl::math::setNan( & fResult );
    else
        fResult = log( value ) / m_fLogOfBase;
    return fResult;
}

uno::Reference< XScaling > SAL_CALL LogarithmicScaling::getInverseScaling()
    throw (uno::RuntimeException, std::exception)
{
    return new ExponentialScaling( m_fBase );
}

OUString SAL_CALL LogarithmicScaling::getServiceName()
    throw (uno::RuntimeException, std::exception)
{
    return OUString(lcl_aServiceName_Logarithmic);
}

uno::Sequence< OUString > LogarithmicScaling::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aSeq { lcl_aServiceName_Logarithmic };
    return aSeq;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
OUString SAL_CALL LogarithmicScaling::getImplementationName()
    throw( css::uno::RuntimeException, std::exception )
{
    return getImplementationName_Static();
}

OUString LogarithmicScaling::getImplementationName_Static()
{
    return OUString(lcl_aServiceName_Logarithmic);
}

sal_Bool SAL_CALL LogarithmicScaling::supportsService( const OUString& rServiceName )
    throw( css::uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL LogarithmicScaling::getSupportedServiceNames()
    throw( css::uno::RuntimeException, std::exception )
{
    return getSupportedServiceNames_Static();
}

ExponentialScaling::ExponentialScaling( const uno::Reference< uno::XComponentContext > & xContext ) :
        m_fBase( 10.0 ),
        m_xContext( xContext )
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
    throw (uno::RuntimeException, std::exception)
{
    double fResult;
    if( ::rtl::math::isNan( value ) || ::rtl::math::isInf( value ) )
        ::rtl::math::setNan( & fResult );
    else
        fResult = pow( m_fBase, value );
    return fResult;
}

uno::Reference< XScaling > SAL_CALL ExponentialScaling::getInverseScaling()
    throw (uno::RuntimeException, std::exception)
{
    return new LogarithmicScaling( m_fBase );
}

OUString SAL_CALL ExponentialScaling::getServiceName()
    throw (uno::RuntimeException, std::exception)
{
    return OUString(lcl_aServiceName_Exponential);
}

uno::Sequence< OUString > ExponentialScaling::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aSeq { lcl_aServiceName_Exponential };
    return aSeq;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
OUString SAL_CALL ExponentialScaling::getImplementationName()
    throw( css::uno::RuntimeException, std::exception )
{
    return getImplementationName_Static();
}

OUString ExponentialScaling::getImplementationName_Static()
{
    return OUString(lcl_aServiceName_Exponential);
}

sal_Bool SAL_CALL ExponentialScaling::supportsService( const OUString& rServiceName )
    throw( css::uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL ExponentialScaling::getSupportedServiceNames()
    throw( css::uno::RuntimeException, std::exception )
{
    return getSupportedServiceNames_Static();
}

LinearScaling::LinearScaling( const uno::Reference< uno::XComponentContext > & xContext ) :
        m_fSlope( 1.0 ),
        m_fOffset( 0.0 ),
        m_xContext( xContext )
{}

LinearScaling::LinearScaling( double fSlope, double fOffset ) :
        m_fSlope( fSlope ),
        m_fOffset( fOffset )
{}

LinearScaling::~LinearScaling()
{}

double SAL_CALL LinearScaling::doScaling( double value )
    throw (uno::RuntimeException, std::exception)
{
    double fResult;
    if( ::rtl::math::isNan( value ) || ::rtl::math::isInf( value ) )
        ::rtl::math::setNan( & fResult );
    else
        fResult = m_fOffset + m_fSlope * value;
    return fResult;
}

uno::Reference< XScaling > SAL_CALL
    LinearScaling::getInverseScaling()
    throw (uno::RuntimeException, std::exception)
{
    // ToDo: ApproxEqual ?
    if( m_fSlope == 0 )
        throw uno::RuntimeException();

    return new LinearScaling( 1.0 / m_fSlope, m_fOffset / m_fSlope );
}

OUString SAL_CALL LinearScaling::getServiceName()
    throw (uno::RuntimeException, std::exception)
{
    return OUString(lcl_aServiceName_Linear);
}

uno::Sequence< OUString > LinearScaling::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aSeq { lcl_aServiceName_Linear };
    return aSeq;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
OUString SAL_CALL LinearScaling::getImplementationName()
    throw( css::uno::RuntimeException, std::exception )
{
    return getImplementationName_Static();
}

OUString LinearScaling::getImplementationName_Static()
{
    return OUString(lcl_aServiceName_Linear) ;
}

sal_Bool SAL_CALL LinearScaling::supportsService( const OUString& rServiceName )
    throw( css::uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL LinearScaling::getSupportedServiceNames()
    throw( css::uno::RuntimeException, std::exception )
{
    return getSupportedServiceNames_Static();
}

PowerScaling::PowerScaling( const uno::Reference< uno::XComponentContext > & xContext ) :
        m_fExponent( 10.0 ),
        m_xContext( xContext )
{}

PowerScaling::PowerScaling( double fExponent ) :
        m_fExponent( fExponent )
{}

PowerScaling::~PowerScaling()
{}

double SAL_CALL PowerScaling::doScaling( double value )
    throw (uno::RuntimeException, std::exception)
{
    double fResult;
    if( ::rtl::math::isNan( value ) || ::rtl::math::isInf( value ) )
        ::rtl::math::setNan( & fResult );
    else
        fResult = pow( value, m_fExponent );
    return fResult;
}

uno::Reference< XScaling > SAL_CALL
    PowerScaling::getInverseScaling()
    throw (uno::RuntimeException, std::exception)
{
    // ToDo: ApproxEqual ?
    if( m_fExponent == 0 )
        throw uno::RuntimeException();

    return new PowerScaling( 1.0 / m_fExponent );
}

    OUString SAL_CALL
PowerScaling::getServiceName()
    throw (uno::RuntimeException, std::exception)
{
    return OUString(lcl_aServiceName_Power);
}

uno::Sequence< OUString > PowerScaling::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aSeq { lcl_aServiceName_Power };
    return aSeq;
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
OUString SAL_CALL PowerScaling::getImplementationName()
    throw( css::uno::RuntimeException, std::exception )
{
    return getImplementationName_Static();
}

OUString PowerScaling::getImplementationName_Static()
{
    return OUString(lcl_aServiceName_Power);
}

sal_Bool SAL_CALL PowerScaling::supportsService( const OUString& rServiceName )
    throw( css::uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL PowerScaling::getSupportedServiceNames()
    throw( css::uno::RuntimeException, std::exception )
{
    return getSupportedServiceNames_Static();
}

} //namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_chart2_LinearScaling_get_implementation(css::uno::XComponentContext *context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new chart::LinearScaling(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_chart2_ExponentialScaling_get_implementation(css::uno::XComponentContext *context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new chart::ExponentialScaling(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_chart2_LogarithmicScaling_get_implementation(css::uno::XComponentContext *context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new chart::LogarithmicScaling(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_chart2_PowerScaling_get_implementation(css::uno::XComponentContext *context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new chart::PowerScaling(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
