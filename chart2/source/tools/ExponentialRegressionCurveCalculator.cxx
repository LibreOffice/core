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

#include "ExponentialRegressionCurveCalculator.hxx"
#include "macros.hxx"
#include "RegressionCalculationHelper.hxx"

#include <rtl/math.hxx>
#include <rtl/ustrbuf.hxx>

using namespace ::com::sun::star;

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

    OUStringBuffer aBuf( "f(x) = ");

    if ( fIntercept == 0.0)
    {
        // underflow, a true zero is impossible
        aBuf.append( "exp( ");
        aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, m_fLogIntercept) );
        aBuf.append( (m_fLogSlope < 0.0) ? " - " : " + ");
        aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, fabs(m_fLogSlope)) );
        aBuf.append( " x )");
    }
    else
    {
        if (bHasIntercept)
        {
            aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, fIntercept) );
            aBuf.append( " exp( ");
            aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, m_fLogSlope) );
            aBuf.append( " x )");
        }
        else
        {
            // show logarithmic output, if intercept and slope both are near one
            // otherwise drop output of intercept, which is 1 here
            aBuf.append( " exp( ");
            if (!bHasSlope)
            {
                aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, m_fLogIntercept) );
                aBuf.append( (m_fLogSlope < 0.0) ? " - " : " + ");
                aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, fabs(m_fLogSlope)) );
            }
            else
            {
                aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, m_fLogSlope) );
            }
            aBuf.append( " x )");
        }
    }

    return aBuf.makeStringAndClear();
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
