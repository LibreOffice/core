/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ExponentialRegressionCurveCalculator.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 12:05:27 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "ExponentialRegressionCurveCalculator.hxx"
#include "macros.hxx"
#include "RegressionCalculationHelper.hxx"

#include <rtl/math.hxx>
#include <rtl/ustrbuf.hxx>

using namespace ::com::sun::star;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

namespace chart
{

ExponentialRegressionCurveCalculator::ExponentialRegressionCurveCalculator() :
        m_fSlope( 0.0 ),
        m_fIntercept( 0.0 )
{
    ::rtl::math::setNan( & m_fSlope );
    ::rtl::math::setNan( & m_fIntercept );
}

ExponentialRegressionCurveCalculator::~ExponentialRegressionCurveCalculator()
{}

// ____ XRegressionCurveCalculator ____
void SAL_CALL ExponentialRegressionCurveCalculator::recalculateRegression(
    const uno::Sequence< double >& aXValues,
    const uno::Sequence< double >& aYValues )
    throw (uno::RuntimeException)
{
    RegressionCalculationHelper::tDoubleVectorPair aValues(
        RegressionCalculationHelper::cleanup(
            aXValues, aYValues,
            RegressionCalculationHelper::isValidAndYPositive()));

    const size_t nMax = aValues.first.size();
    if( nMax == 0 )
    {
        ::rtl::math::setNan( & m_fSlope );
        ::rtl::math::setNan( & m_fIntercept );
        ::rtl::math::setNan( & m_fCorrelationCoeffitient );
        return;
    }

    double fAverageX = 0.0, fAverageY = 0.0;
    size_t i = 0;
    for( i = 0; i < nMax; ++i )
    {
        fAverageX += aValues.first[i];
        fAverageY += log( aValues.second[i] );
    }

    const double fN = static_cast< double >( nMax );
    fAverageX /= fN;
    fAverageY /= fN;

    double fQx = 0.0, fQy = 0.0, fQxy = 0.0;
    for( i = 0; i < nMax; ++i )
    {
        double fDeltaX = aValues.first[i] - fAverageX;
        double fDeltaY = log( aValues.second[i] ) - fAverageY;

        fQx  += fDeltaX * fDeltaX;
        fQy  += fDeltaY * fDeltaY;
        fQxy += fDeltaX * fDeltaY;
    }

    m_fSlope = fQxy / fQx;
    m_fIntercept = fAverageY - m_fSlope * fAverageX;
    m_fCorrelationCoeffitient = fQxy / sqrt( fQx * fQy );

    m_fSlope = exp( m_fSlope );
    m_fIntercept = exp( m_fIntercept );
}

double SAL_CALL ExponentialRegressionCurveCalculator::getCurveValue( double x )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    double fResult;
    ::rtl::math::setNan( & fResult );

    if( ! ( ::rtl::math::isNan( m_fSlope ) ||
            ::rtl::math::isNan( m_fIntercept )))
    {
        fResult = m_fIntercept * pow( m_fSlope, x );
    }

    return fResult;
}

uno::Sequence< geometry::RealPoint2D > SAL_CALL ExponentialRegressionCurveCalculator::getCurveValues(
    double min, double max, ::sal_Int32 nPointCount,
    const uno::Reference< chart2::XScaling >& xScalingX,
    const uno::Reference< chart2::XScaling >& xScalingY,
    ::sal_Bool bMaySkipPointsInCalculation )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    if( bMaySkipPointsInCalculation &&
        isLinearScaling( xScalingX ) &&
        isLogarithmicScaling( xScalingY ))
    {
        // optimize result
        uno::Sequence< geometry::RealPoint2D > aResult( 2 );
        aResult[0].X = min;
        aResult[0].Y = this->getCurveValue( min );
        aResult[1].X = max;
        aResult[1].Y = this->getCurveValue( max );

        return aResult;
    }

    return RegressionCurveCalculator::getCurveValues( min, max, nPointCount, xScalingX, xScalingY, bMaySkipPointsInCalculation );
}


OUString ExponentialRegressionCurveCalculator::ImplGetRepresentation(
    const uno::Reference< util::XNumberFormatter >& xNumFormatter,
    ::sal_Int32 nNumberFormatKey ) const
{
    OUStringBuffer aBuf( C2U( "f(x) = " ));

    if( m_fIntercept == 0.0 ||
        m_fSlope == 0.0 )
    {
        aBuf.append( sal_Unicode( '0' ));
    }
    else if( rtl::math::approxEqual( m_fSlope, 1.0 ) )
    {
        aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, m_fIntercept ));
    }
    else
    {
        if( ! rtl::math::approxEqual( m_fIntercept, 1.0 ) )
        {
            aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, m_fIntercept ));
            aBuf.append( sal_Unicode( 0x00b7 ));
        }

        if( m_fSlope < 0.0 )
            aBuf.append( sal_Unicode( '(' ));
        aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, m_fSlope ));
        if( m_fSlope < 0.0 )
            aBuf.append( sal_Unicode( ')' ));
        aBuf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "^x" ));
    }

    return aBuf.makeStringAndClear();
}

} //  namespace chart
