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
#include "LinearRegressionCurveCalculator.hxx"
#include "macros.hxx"
#include "RegressionCalculationHelper.hxx"

#include <rtl/math.hxx>
#include <rtl/ustrbuf.hxx>

using namespace ::com::sun::star;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

namespace chart
{

LinearRegressionCurveCalculator::LinearRegressionCurveCalculator() :
        m_fSlope( 0.0 ),
        m_fIntercept( 0.0 )
{
    ::rtl::math::setNan( & m_fSlope );
    ::rtl::math::setNan( & m_fIntercept );
}

LinearRegressionCurveCalculator::~LinearRegressionCurveCalculator()
{}

// ____ XRegressionCurveCalculator ____
void SAL_CALL LinearRegressionCurveCalculator::recalculateRegression(
    const uno::Sequence< double >& aXValues,
    const uno::Sequence< double >& aYValues )
    throw (uno::RuntimeException)
{
    RegressionCalculationHelper::tDoubleVectorPair aValues(
        RegressionCalculationHelper::cleanup(
            aXValues, aYValues,
            RegressionCalculationHelper::isValid()));

    const size_t nMax = aValues.first.size();
    if( nMax == 0 )
    {
        ::rtl::math::setNan( & m_fSlope );
        ::rtl::math::setNan( & m_fIntercept );
        ::rtl::math::setNan( & m_fCorrelationCoeffitient );
        return;
    }

    const double fN = static_cast< double >( nMax );
    double fSumX = 0.0, fSumY = 0.0, fSumXSq = 0.0, fSumYSq = 0.0, fSumXY = 0.0;
    for( size_t i = 0; i < nMax; ++i )
    {
        fSumX   += aValues.first[i];
        fSumY   += aValues.second[i];
        fSumXSq += aValues.first[i]  * aValues.first[i];
        fSumYSq += aValues.second[i] * aValues.second[i];
        fSumXY  += aValues.first[i]  * aValues.second[i];
    }

    m_fSlope = (fN * fSumXY - fSumX * fSumY) / ( fN * fSumXSq - fSumX * fSumX );
    m_fIntercept = (fSumY - m_fSlope * fSumX) / fN;

    m_fCorrelationCoeffitient = ( fN * fSumXY - fSumX * fSumY ) /
        sqrt( ( fN * fSumXSq - fSumX * fSumX ) *
              ( fN * fSumYSq - fSumY * fSumY ) );
}

double SAL_CALL LinearRegressionCurveCalculator::getCurveValue( double x )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    double fResult;
    ::rtl::math::setNan( & fResult );

    if( ! ( ::rtl::math::isNan( m_fSlope ) ||
            ::rtl::math::isNan( m_fIntercept )))
    {
        fResult = m_fSlope * x + m_fIntercept;
    }

    return fResult;
}

uno::Sequence< geometry::RealPoint2D > SAL_CALL LinearRegressionCurveCalculator::getCurveValues(
    double min, double max, ::sal_Int32 nPointCount,
    const uno::Reference< chart2::XScaling >& xScalingX,
    const uno::Reference< chart2::XScaling >& xScalingY,
    ::sal_Bool bMaySkipPointsInCalculation )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    if( bMaySkipPointsInCalculation &&
        isLinearScaling( xScalingX ) &&
        isLinearScaling( xScalingY ))
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

OUString LinearRegressionCurveCalculator::ImplGetRepresentation(
    const uno::Reference< util::XNumberFormatter >& xNumFormatter,
    ::sal_Int32 nNumberFormatKey ) const
{
    OUStringBuffer aBuf( C2U( "f(x) = " ));

    bool bHaveSlope = false;

    if( m_fSlope != 0.0 )
    {
        if( ::rtl::math::approxEqual( fabs( m_fSlope ), 1.0 ))
        {
            if( m_fSlope < 0 )
                aBuf.append( UC_MINUS_SIGN );
        }
        else
            aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, m_fSlope ));
        aBuf.append( sal_Unicode( 'x' ));
        bHaveSlope = true;
    }

    if( bHaveSlope )
    {
        if( m_fIntercept < 0.0 )
        {
            aBuf.append( UC_SPACE );
            aBuf.append( UC_MINUS_SIGN );
            aBuf.append( UC_SPACE );
            aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, fabs( m_fIntercept )));
        }
        else if( m_fIntercept > 0.0 )
        {
            aBuf.appendAscii( RTL_CONSTASCII_STRINGPARAM( " + " ));
            aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, m_fIntercept ));
        }
    }
    else
    {
        aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, m_fIntercept ));
    }

    return aBuf.makeStringAndClear();
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
