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
#include <SpecialUnicodes.hxx>

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
    if( nMax <= 1 ) // at least 2 points
    {
        aValues = RegressionCalculationHelper::cleanup(
                    aXValues, aYValues,
                    RegressionCalculationHelper::isValidAndYNegative());
        nMax = aValues.first.size();
        if( nMax <= 1 )
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
    sal_Int32 nNumberFormatKey, sal_Int32* pFormulaMaxWidth /* = nullptr */ ) const
{
    double fIntercept = exp(m_fLogIntercept);
    bool bHasSlope = !rtl::math::approxEqual( exp(m_fLogSlope), 1.0 );
    bool bHasLogSlope = !rtl::math::approxEqual( fabs(m_fLogSlope), 1.0 );
    bool bHasIntercept = !rtl::math::approxEqual( fIntercept, 1.0 ) && fIntercept != 0.0;

    OUStringBuffer aBuf( "f(x) = " );
    sal_Int32 nLineLength = aBuf.getLength();
    sal_Int32 nValueLength=0;
    if ( pFormulaMaxWidth && *pFormulaMaxWidth > 0 )
    {          // count characters different from coefficients
        sal_Int32 nCharMin = nLineLength + 11;  // 11 = "exp( ", " x )" + 2 extra characters
        if ( m_fSign < 0.0 )
            nCharMin += 2;
        if ( fIntercept == 0.0 || ( !bHasSlope && m_fLogIntercept != 0.0 ) )
            nCharMin += 3; // " + " special case where equation is writen exp( a + b x )
        if ( ( bHasIntercept || fIntercept == 0.0 || ( !bHasSlope && m_fLogIntercept != 0.0 ) ) &&
               bHasLogSlope )
            nValueLength = ( *pFormulaMaxWidth - nCharMin ) / 2;
        else
            nValueLength = *pFormulaMaxWidth - nCharMin;
        if ( nValueLength <= 0 )
            nValueLength = 1;
    }
                    // temporary buffer
    OUStringBuffer aTmpBuf("");
        // if nValueLength not calculated then nullptr
    sal_Int32* pValueLength = nValueLength ? &nValueLength : nullptr;
    if ( m_fSign < 0.0 )
        aTmpBuf.append(aMinusSign).append(" ");
    if ( bHasIntercept )
    {
        OUString aValueString = getFormattedString( xNumFormatter, nNumberFormatKey, fIntercept, pValueLength );
        if ( aValueString != "1" )  // aValueString may be rounded to 1 if nValueLength is small
        {
            aTmpBuf.append( aValueString + " " );
            addStringToEquation( aBuf, nLineLength, aTmpBuf, pFormulaMaxWidth );
            aTmpBuf.truncate();
        }
    }
    aTmpBuf.append( "exp( " );
    if ( !bHasIntercept )
    {
        if ( fIntercept == 0.0 ||  // underflow, a true zero is impossible
           ( !bHasSlope && m_fLogIntercept != 0.0 ) )   // show logarithmic output, if intercept and slope both are near one
        {                                               // otherwise drop output of intercept, which is 1 here
            OUString aValueString = getFormattedString( xNumFormatter, nNumberFormatKey, m_fLogIntercept, pValueLength );
            if ( aValueString != "0" )  // aValueString may be rounded to 0 if nValueLength is small
            {
                aTmpBuf.append( aValueString + ( (m_fLogSlope < 0.0) ? OUStringBuffer(" ") : OUStringBuffer(" + ") ) );
            }
        }
    }
    if ( m_fLogSlope < 0.0 )
        aTmpBuf.append(aMinusSign).append(" ");
    if ( bHasLogSlope )
    {
        OUString aValueString = getFormattedString( xNumFormatter, nNumberFormatKey, fabs(m_fLogSlope), pValueLength );
        if ( aValueString != "1" )  // aValueString may be rounded to 1 if nValueLength is small
        {
            aTmpBuf.append( aValueString + " " );
        }
    }
    aTmpBuf.append( "x )");
    addStringToEquation( aBuf, nLineLength, aTmpBuf, pFormulaMaxWidth );

    return aBuf.makeStringAndClear();
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
