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

namespace
{

static const OUString lcl_aServiceName_Logarithmic( "com.sun.star.chart2.LogarithmicScaling" );
static const OUString lcl_aServiceName_Exponential( "com.sun.star.chart2.ExponentialScaling" );
static const OUString lcl_aServiceName_Linear( "com.sun.star.chart2.LinearScaling" );
static const OUString lcl_aServiceName_Power( "com.sun.star.chart2.PowerScaling" );

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

    double SAL_CALL
LogarithmicScaling::doScaling( double value )
    throw (uno::RuntimeException)
{
    double fResult;
    if( ::rtl::math::isNan( value ) || ::rtl::math::isInf( value ) )
        ::rtl::math::setNan( & fResult );
    else
        fResult = log( value ) / m_fLogOfBase;
    return fResult;
}

    uno::Reference< XScaling > SAL_CALL
LogarithmicScaling::getInverseScaling()
    throw (uno::RuntimeException)
{
    return new ExponentialScaling( m_fBase );
}

    OUString SAL_CALL
LogarithmicScaling::getServiceName()
    throw (uno::RuntimeException)
{
    return lcl_aServiceName_Logarithmic;
}

uno::Sequence< OUString > LogarithmicScaling::getSupportedServiceNames_Static()
{
    return uno::Sequence< OUString >( & lcl_aServiceName_Logarithmic, 1 );
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( LogarithmicScaling, lcl_aServiceName_Logarithmic )

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

    double SAL_CALL
ExponentialScaling::doScaling( double value )
    throw (uno::RuntimeException)
{
    double fResult;
    if( ::rtl::math::isNan( value ) || ::rtl::math::isInf( value ) )
        ::rtl::math::setNan( & fResult );
    else
        fResult = pow( m_fBase, value );
    return fResult;
}

    uno::Reference< XScaling > SAL_CALL
ExponentialScaling::getInverseScaling()
    throw (uno::RuntimeException)
{
    return new LogarithmicScaling( m_fBase );
}

    OUString SAL_CALL
ExponentialScaling::getServiceName()
    throw (uno::RuntimeException)
{
    return lcl_aServiceName_Exponential;
}

uno::Sequence< OUString > ExponentialScaling::getSupportedServiceNames_Static()
{
    return uno::Sequence< OUString >( & lcl_aServiceName_Exponential, 1 );
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( ExponentialScaling, lcl_aServiceName_Exponential )

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
    throw (uno::RuntimeException)
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
    throw (uno::RuntimeException)
{
    // ToDo: ApproxEqual ?
    if( m_fSlope == 0 )
        throw uno::RuntimeException();

    return new LinearScaling( 1.0 / m_fSlope, m_fOffset / m_fSlope );
}

    OUString SAL_CALL
LinearScaling::getServiceName()
    throw (uno::RuntimeException)
{
    return lcl_aServiceName_Linear;
}

uno::Sequence< OUString > LinearScaling::getSupportedServiceNames_Static()
{
    return uno::Sequence< OUString >( & lcl_aServiceName_Linear, 1 );
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( LinearScaling, lcl_aServiceName_Linear )

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
    throw (uno::RuntimeException)
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
    throw (uno::RuntimeException)
{
    // ToDo: ApproxEqual ?
    if( m_fExponent == 0 )
        throw uno::RuntimeException();

    return new PowerScaling( 1.0 / m_fExponent );
}

    OUString SAL_CALL
PowerScaling::getServiceName()
    throw (uno::RuntimeException)
{
    return lcl_aServiceName_Power;
}

uno::Sequence< OUString > PowerScaling::getSupportedServiceNames_Static()
{
    return uno::Sequence< OUString >( & lcl_aServiceName_Power, 1 );
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( PowerScaling, lcl_aServiceName_Power )

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
