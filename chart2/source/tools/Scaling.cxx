/*************************************************************************
 *
 *  $RCSfile: Scaling.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:32 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _CHART2_SCALING_HXX
#include "Scaling.hxx"
#endif

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

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
}

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::chart2;

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

    double SAL_CALL
LogarithmicScaling::doScaling( double value )
    throw (uno::RuntimeException)
{
    return log( value ) / m_fLogOfBase;
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

// ----------------------------------------

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

    double SAL_CALL
ExponentialScaling::doScaling( double value )
    throw (uno::RuntimeException)
{
    return pow( m_fBase, value );
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

// ----------------------------------------

LinearScaling::LinearScaling( double fSlope, double fOffset ) :
        m_fSlope( fSlope ),
        m_fOffset( fOffset )
{}

LinearScaling::~LinearScaling()
{}

double SAL_CALL LinearScaling::doScaling( double value )
    throw (uno::RuntimeException)
{
    return m_fOffset + m_fSlope * value;
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

// ----------------------------------------

PowerScaling::PowerScaling( double fExponent ) :
        m_fExponent( fExponent )
{}

PowerScaling::~PowerScaling()
{}

double SAL_CALL PowerScaling::doScaling( double value )
    throw (uno::RuntimeException)
{
    return pow( value, m_fExponent );
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

//.............................................................................
} //namespace chart
//.............................................................................
