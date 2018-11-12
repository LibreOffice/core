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

#include <PotentialRegressionCurveCalculator.hxx>
#include <RegressionCalculationHelper.hxx>
#include <SpecialCharacters.hxx>

#include <rtl/math.hxx>
#include <rtl/ustrbuf.hxx>

using namespace ::com::sun::star;

namespace chart
{

PotentialRegressionCurveCalculator::PotentialRegressionCurveCalculator()
    : m_fSlope(0.0)
    , m_fIntercept(0.0)
    , m_fSign(1.0)
{
    ::rtl::math::setNan( & m_fSlope );
    ::rtl::math::setNan( & m_fIntercept );
}

PotentialRegressionCurveCalculator::~PotentialRegressionCurveCalculator()
{}

// ____ XRegressionCurveCalculator ____
void SAL_CALL PotentialRegressionCurveCalculator::recalculateRegression(
    const uno::Sequence< double >& aXValues,
    const uno::Sequence< double >& aYValues )
{
    RegressionCalculationHelper::tDoubleVectorPair aValues(
        RegressionCalculationHelper::cleanup(
            aXValues, aYValues,
            RegressionCalculationHelper::isValidAndBothPositive()));
    m_fSign = 1.0;

    size_t nMax = aValues.first.size();
    if( nMax <= 1 )  // at least 2 points
    {
        aValues = RegressionCalculationHelper::cleanup(
                    aXValues, aYValues,
                    RegressionCalculationHelper::isValidAndXPositiveAndYNegative());
        nMax = aValues.first.size();
        if( nMax <= 1 )
        {
            ::rtl::math::setNan( & m_fSlope );
            ::rtl::math::setNan( & m_fIntercept );
            ::rtl::math::setNan( & m_fCorrelationCoeffitient );
            return;
        }
        m_fSign = -1.0;
    }

    double fAverageX = 0.0, fAverageY = 0.0;
    size_t i = 0;
    for( i = 0; i < nMax; ++i )
    {
        fAverageX += log( aValues.first[i] );
        fAverageY += log( m_fSign * aValues.second[i] );
    }

    const double fN = static_cast< double >( nMax );
    fAverageX /= fN;
    fAverageY /= fN;

    double fQx = 0.0, fQy = 0.0, fQxy = 0.0;
    for( i = 0; i < nMax; ++i )
    {
        double fDeltaX = log( aValues.first[i] ) - fAverageX;
        double fDeltaY = log( m_fSign * aValues.second[i] ) - fAverageY;

        fQx  += fDeltaX * fDeltaX;
        fQy  += fDeltaY * fDeltaY;
        fQxy += fDeltaX * fDeltaY;
    }

    m_fSlope = fQxy / fQx;
    m_fIntercept = fAverageY - m_fSlope * fAverageX;
    m_fCorrelationCoeffitient = fQxy / sqrt( fQx * fQy );

    m_fIntercept = m_fSign * exp( m_fIntercept );
}

double SAL_CALL PotentialRegressionCurveCalculator::getCurveValue( double x )
{
    double fResult;
    ::rtl::math::setNan( & fResult );

    if( ! ( ::rtl::math::isNan( m_fSlope ) ||
            ::rtl::math::isNan( m_fIntercept )))
    {
        fResult = m_fIntercept * pow( x, m_fSlope );
    }

    return fResult;
}

uno::Sequence< geometry::RealPoint2D > SAL_CALL PotentialRegressionCurveCalculator::getCurveValues(
    double min, double max, ::sal_Int32 nPointCount,
    const uno::Reference< chart2::XScaling >& xScalingX,
    const uno::Reference< chart2::XScaling >& xScalingY,
    sal_Bool bMaySkipPointsInCalculation )
{
    if( bMaySkipPointsInCalculation &&
        isLogarithmicScaling( xScalingX ) &&
        isLogarithmicScaling( xScalingY ))
    {
        // optimize result
        uno::Sequence< geometry::RealPoint2D > aResult( 2 );
        aResult[0].X = min;
        aResult[0].Y = getCurveValue( min );
        aResult[1].X = max;
        aResult[1].Y = getCurveValue( max );

        return aResult;
    }
    return RegressionCurveCalculator::getCurveValues( min, max, nPointCount, xScalingX, xScalingY, bMaySkipPointsInCalculation );
}

OUString PotentialRegressionCurveCalculator::ImplGetRepresentation(
    const uno::Reference< util::XNumberFormatter >& xNumFormatter,
    sal_Int32 nNumberFormatKey, sal_Int32* pFormulaMaxWidth /* = nullptr */ ) const
{
    bool bHasIntercept = !rtl::math::approxEqual( fabs(m_fIntercept), 1.0 );
    OUStringBuffer aBuf( mYName + " = " );
    sal_Int32 nLineLength = aBuf.getLength();
    sal_Int32 nValueLength=0;
    if ( pFormulaMaxWidth && *pFormulaMaxWidth > 0 ) // count nValueLength
    {
        sal_Int32 nCharMin = nLineLength + mXName.getLength() + 3;  // 3 = "^" + 2 extra characters
        if ( m_fIntercept != 0.0 && m_fSlope != 0.0 )
        {
            if ( m_fIntercept < 0.0 )
                nCharMin += 2;  // "- "
            if ( bHasIntercept )
                nValueLength = (*pFormulaMaxWidth - nCharMin) / 2;
        }
        if ( nValueLength == 0 ) // not yet calculated
            nValueLength = *pFormulaMaxWidth - nCharMin;
        if ( nValueLength <= 0 )
            nValueLength = 1;
    }

    if( m_fIntercept == 0.0 )
    {
        aBuf.append( '0' );
    }
    else
    {
        // temporary buffer
        OUStringBuffer aTmpBuf("");
        // if nValueLength not calculated then nullptr
        sal_Int32* pValueLength = nValueLength ? &nValueLength : nullptr;
        if ( m_fIntercept < 0.0 )    // add intercept value
             aTmpBuf.append( OUStringLiteral1(aMinusSign) ).append( " " );
        if( bHasIntercept )
        {
            OUString aValueString = getFormattedString( xNumFormatter, nNumberFormatKey, fabs(m_fIntercept), pValueLength );
            if ( aValueString != "1" )  // aValueString may be rounded to 1 if nValueLength is small
            {
                aTmpBuf.append( aValueString ).append( " " );
            }
        }
        if( m_fSlope != 0.0 )  // add slope value
        {
            aTmpBuf.append( mXName ).append( "^" );
            aTmpBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, m_fSlope, pValueLength ));
        }
        addStringToEquation( aBuf, nLineLength, aTmpBuf, pFormulaMaxWidth );
    }

    return aBuf.makeStringAndClear();
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
