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

ExponentialRegressionCurveCalculator::ExponentialRegressionCurveCalculator()
    : m_fLogSlope(0.0)
    , m_fLogIntercept(0.0)
    , m_fSign(1.0)
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
    throw (uno::RuntimeException, std::exception)
{
    RegressionCalculationHelper::tDoubleVectorPair aValues(
        RegressionCalculationHelper::cleanup(
            aXValues, aYValues,
            RegressionCalculationHelper::isValidAndYPositive()));
    m_fSign = 1.0;

    size_t nMax = aValues.first.size();
    if( nMax == 0 )
    {
        aValues = RegressionCalculationHelper::cleanup(
                    aXValues, aYValues,
                    RegressionCalculationHelper::isValidAndYNegative());
        nMax = aValues.first.size();
        if( nMax == 0 )
        {
            ::rtl::math::setNan( & m_fLogSlope );
            ::rtl::math::setNan( & m_fLogIntercept );
            ::rtl::math::setNan( & m_fCorrelationCoeffitient );// actual it is coefficient of determination
            return;
        }
        m_fSign = -1.0;
    }

    double fAverageX = 0.0, fAverageY = 0.0;
    double fLogIntercept = ( mForceIntercept && (m_fSign * mInterceptValue)>0 ) ? log(m_fSign * mInterceptValue) : 0.0;
    std::vector<double> yVector;
    yVector.resize(nMax, 0.0);

    size_t i = 0;
    for( i = 0; i < nMax; ++i )
    {
        double yValue = log( m_fSign *aValues.second[i] );
        if (mForceIntercept)
        {
            yValue -= fLogIntercept;
        }
        else
        {
            fAverageX += aValues.first[i];
            fAverageY += yValue;
        }
        yVector[i] = yValue;
    }

    const double fN = static_cast< double >( nMax );
    fAverageX /= fN;
    fAverageY /= fN;

    double fQx = 0.0, fQy = 0.0, fQxy = 0.0;
    for( i = 0; i < nMax; ++i )
    {
        double fDeltaX = aValues.first[i] - fAverageX;
        double fDeltaY = yVector[i] - fAverageY;

        fQx  += fDeltaX * fDeltaX;
        fQy  += fDeltaY * fDeltaY;
        fQxy += fDeltaX * fDeltaY;
    }

    m_fLogSlope = fQxy / fQx;
    m_fLogIntercept = mForceIntercept ? fLogIntercept : fAverageY - m_fLogSlope * fAverageX;
    m_fCorrelationCoeffitient = fQxy / sqrt( fQx * fQy );
}

double SAL_CALL ExponentialRegressionCurveCalculator::getCurveValue( double x )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException, std::exception)
{
    double fResult;
    ::rtl::math::setNan( & fResult );

    if( ! ( ::rtl::math::isNan( m_fLogSlope ) ||
            ::rtl::math::isNan( m_fLogIntercept )))
    {
        fResult = m_fSign * exp(m_fLogIntercept + x * m_fLogSlope);
    }

    return fResult;
}

uno::Sequence< geometry::RealPoint2D > SAL_CALL ExponentialRegressionCurveCalculator::getCurveValues(
    double min, double max, ::sal_Int32 nPointCount,
    const uno::Reference< chart2::XScaling >& xScalingX,
    const uno::Reference< chart2::XScaling >& xScalingY,
    sal_Bool bMaySkipPointsInCalculation )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException, std::exception)
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
    ::sal_Int32 nNumberFormatKey, ::sal_Int32 nFormulaLength /* = 0 */ ) const
{
    OUString aHash("###");
    if ( nFormulaLength > 0 && nFormulaLength <= 7 )
        return aHash;
    double fIntercept = m_fSign * exp(m_fLogIntercept);
    bool bHasSlope = !rtl::math::approxEqual( exp(m_fLogSlope), 1.0 );
    bool bIsLogSlop1 = rtl::math::approxEqual( fabs(m_fLogSlope), 1.0 );
    bool bHasIntercept = !rtl::math::approxEqual( m_fSign*fIntercept, 1.0 );

    OUStringBuffer aBuf( "f(x) = ");
    if ( nFormulaLength > 0 )
        nFormulaLength -= aBuf.getLength();

    if ( fIntercept == 0.0)
    {
        if ( nFormulaLength > 0 )
        {
            if ( m_fSign < 0.0 )
                nFormulaLength -= 2;
            nFormulaLength = (nFormulaLength - 12)/2; // "exp( ", " + ", " x )"
            if ( nFormulaLength <= 0 )
                return aHash;
        }
        if ( m_fSign < 0.0 )
            aBuf.append( "- " );
        // underflow, a true zero is impossible
        aBuf.append( "exp( ");
        aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, m_fLogIntercept, nFormulaLength) );
        aBuf.append( (m_fLogSlope < 0.0) ? " - " : " + ");
        aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, fabs(m_fLogSlope), nFormulaLength ) );
        aBuf.append( " x )");
    }
    else
    {
        if (bHasIntercept)
        {
            if ( nFormulaLength > 0 )
            {
                nFormulaLength -= 9; // "exp( ", " x )"
                if ( !bIsLogSlop1 )
                    nFormulaLength /= 2;
                if ( m_fLogSlope < 0.0 )
                    nFormulaLength --;
                if ( nFormulaLength <= 0 )
                    return aHash;
            }
            aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, fIntercept, nFormulaLength ) );
            aBuf.append( " exp( ");
            if ( m_fLogSlope < 0.0 )
                aBuf.append( "-");
            if ( !bIsLogSlop1 )
                aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, fabs(m_fLogSlope), nFormulaLength ) );
            aBuf.append( " x )");
        }
        else
        {
            // show logarithmic output, if intercept and slope both are near one
            // otherwise drop output of intercept, which is 1 here
            if ( m_fSign < 0.0 )
                aBuf.append( "- " );
            aBuf.append( " exp( ");
            if (!bHasSlope)
            {
                if ( nFormulaLength > 0 )
                {
                    if ( m_fSign < 0.0 )
                        nFormulaLength -= 2;
                    nFormulaLength -= 12; // "exp( ", " + ", " x )"
                    if ( !bIsLogSlop1 && m_fLogIntercept != 0.0 )
                        nFormulaLength /= 2;
                    if ( nFormulaLength <= 0 )
                        return aHash;
                }
                if ( m_fLogIntercept != 0.0 )
                {
                    aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, m_fLogIntercept, nFormulaLength ) );
                    aBuf.append( (m_fLogSlope < 0.0) ? " - " : " + ");
                }
                else if ( m_fLogSlope < 0.0 )
                        aBuf.append( " - " );
                if ( !bIsLogSlop1 )
                    aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, fabs(m_fLogSlope), nFormulaLength ) );
            }
            else
            {
                if ( nFormulaLength > 0 )
                {
                    if ( m_fSign < 0.0 )
                        nFormulaLength -= 2;
                    nFormulaLength -= 9; // "exp( ", " x )"
                    if ( nFormulaLength <= 0 )
                        return aHash;
                }
                if ( m_fLogSlope < 0.0 )
                    aBuf.append( "-");
                if ( !bIsLogSlop1 )
                    aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, fabs(m_fLogSlope), nFormulaLength ) );
            }
            aBuf.append( " x )");
        }
    }

    return aBuf.makeStringAndClear();
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
