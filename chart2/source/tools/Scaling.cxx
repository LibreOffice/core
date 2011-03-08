/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "Scaling.hxx"
#include <rtl/math.hxx>
#include "com/sun/star/uno/RuntimeException.hpp"

namespace
{

static const ::rtl::OUString lcl_aServiceName_Logarithmic(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.LogarithmicScaling" ));
static const ::rtl::OUString lcl_aServiceName_Exponential(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.ExponentialScaling" ));
static const ::rtl::OUString lcl_aServiceName_Linear(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.LinearScaling" ));
static const ::rtl::OUString lcl_aServiceName_Power(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.PowerScaling" ));

static const ::rtl::OUString lcl_aImplementationName_Logarithmic(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart2.LogarithmicScaling" ));
static const ::rtl::OUString lcl_aImplementationName_Exponential(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart2.ExponentialScaling" ));
static const ::rtl::OUString lcl_aImplementationName_Linear(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart2.LinearScaling" ));
static const ::rtl::OUString lcl_aImplementationName_Power(
    RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.chart2.PowerScaling" ));
}

//.............................................................................
namespace chart
{
//.............................................................................
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

    ::rtl::OUString SAL_CALL
LogarithmicScaling::getServiceName()
    throw (uno::RuntimeException)
{
    return lcl_aServiceName_Logarithmic;
}

uno::Sequence< ::rtl::OUString > LogarithmicScaling::getSupportedServiceNames_Static()
{
    return uno::Sequence< ::rtl::OUString >( & lcl_aServiceName_Logarithmic, 1 );
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( LogarithmicScaling, lcl_aServiceName_Logarithmic )

// ----------------------------------------

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

    ::rtl::OUString SAL_CALL
ExponentialScaling::getServiceName()
    throw (uno::RuntimeException)
{
    return lcl_aServiceName_Exponential;
}

uno::Sequence< ::rtl::OUString > ExponentialScaling::getSupportedServiceNames_Static()
{
    return uno::Sequence< ::rtl::OUString >( & lcl_aServiceName_Exponential, 1 );
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( ExponentialScaling, lcl_aServiceName_Exponential )

// ----------------------------------------

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

    ::rtl::OUString SAL_CALL
LinearScaling::getServiceName()
    throw (uno::RuntimeException)
{
    return lcl_aServiceName_Linear;
}

uno::Sequence< ::rtl::OUString > LinearScaling::getSupportedServiceNames_Static()
{
    return uno::Sequence< ::rtl::OUString >( & lcl_aServiceName_Linear, 1 );
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( LinearScaling, lcl_aServiceName_Linear )

// ----------------------------------------

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

    ::rtl::OUString SAL_CALL
PowerScaling::getServiceName()
    throw (uno::RuntimeException)
{
    return lcl_aServiceName_Power;
}

uno::Sequence< ::rtl::OUString > PowerScaling::getSupportedServiceNames_Static()
{
    return uno::Sequence< ::rtl::OUString >( & lcl_aServiceName_Power, 1 );
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
APPHELPER_XSERVICEINFO_IMPL( PowerScaling, lcl_aServiceName_Power )

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
