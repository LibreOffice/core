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
        m_fLogSlope( 0.0 ),
        m_fLogIntercept( 0.0 )
{
    ::rtl::math::setNan( & m_fLogSlope );
    ::rtl::math::setNan( & m_fLogIntercept );
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
        ::rtl::math::setNan( & m_fLogSlope );
        ::rtl::math::setNan( & m_fLogIntercept );
        ::rtl::math::setNan( & m_fCorrelationCoeffitient );// actual it is coefficient of determination
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

    m_fLogSlope = fQxy / fQx;
    m_fLogIntercept = fAverageY - m_fLogSlope * fAverageX;
    m_fCorrelationCoeffitient = fQxy / sqrt( fQx * fQy );

}

double SAL_CALL ExponentialRegressionCurveCalculator::getCurveValue( double x )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    double fResult;
    ::rtl::math::setNan( & fResult );

    if( ! ( ::rtl::math::isNan( m_fLogSlope ) ||
            ::rtl::math::isNan( m_fLogIntercept )))
    {
        fResult = exp(m_fLogIntercept + x * m_fLogSlope);
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
    double fIntercept = exp(m_fLogIntercept);
    double fSlope = exp(m_fLogSlope);
    bool bHasSlope = !rtl::math::approxEqual( fSlope, 1.0 );
    bool bHasIntercept = !rtl::math::approxEqual( fIntercept, 1.0 );

    OUStringBuffer aBuf( C2U( "f(x) = " ));

    if ( fIntercept == 0.0)
    {
        // underflow, a true zero is impossible
        aBuf.append( C2U( "exp( " ));
        aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, m_fLogIntercept) );
        aBuf.append( (m_fLogSlope < 0.0) ? C2U( " - " ) : C2U( " + " ));
        aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, fabs(m_fLogSlope)) );
        aBuf.append( C2U( " x )" ));
    }
    else
    {
        if (bHasIntercept)
        {
            aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, fIntercept) );
            aBuf.append( C2U( " exp( " ));
            aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, m_fLogSlope) );
            aBuf.append( C2U( " x )" ));
        }
        else
        {
            // show logarithmic output, if intercept and slope both are near one
            // otherwise drop output of intercept, which is 1 here
            aBuf.append( C2U( " exp( " ));
            if (!bHasSlope)
            {
                aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, m_fLogIntercept) );
                aBuf.append( (m_fLogSlope < 0.0) ? C2U( " - " ) : C2U( " + " ));
                aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, fabs(m_fLogSlope)) );
            }
            else
            {
                aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, m_fLogSlope) );
            }
            aBuf.append( C2U( " x )" ));
        }
    }

    return aBuf.makeStringAndClear();
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
