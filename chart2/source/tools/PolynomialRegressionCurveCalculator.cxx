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

#include "PolynomialRegressionCurveCalculator.hxx"
#include "macros.hxx"
#include "RegressionCalculationHelper.hxx"

#include <rtl/math.hxx>
#include <rtl/ustrbuf.hxx>
#include "gauss.hxx"

using namespace com::sun::star;


namespace chart
{

PolynomialRegressionCurveCalculator::PolynomialRegressionCurveCalculator()
{}

PolynomialRegressionCurveCalculator::~PolynomialRegressionCurveCalculator()
{}

// ____ XRegressionCurveCalculator ____
void SAL_CALL PolynomialRegressionCurveCalculator::recalculateRegression(
    const uno::Sequence< double >& aXValues,
    const uno::Sequence< double >& aYValues )
    throw (uno::RuntimeException)
{
    rtl::math::setNan(&m_fCorrelationCoeffitient);

    RegressionCalculationHelper::tDoubleVectorPair aValues(
        RegressionCalculationHelper::cleanup( aXValues, aYValues, RegressionCalculationHelper::isValid()));

    sal_Int32 aNoElements = mForceIntercept ? mDegree : mDegree + 1;
    sal_Int32 aNumberOfPowers = 2 * aNoElements - 1;

    std::vector<double> aPowers;
    aPowers.resize(aNumberOfPowers, 0.0);

    sal_Int32 aNoColumns = aNoElements;
    sal_Int32 aNoRows    = aNoElements + 1;

    std::vector<double> aMatrix;
    aMatrix.resize(aNoColumns * aNoRows, 0.0);

    const size_t aNoValues = aValues.first.size();

    double yAverage = 0.0;

    for( size_t i = 0; i < aNoValues; ++i )
    {
        double x = aValues.first[i];
        double y = aValues.second[i];

        for (sal_Int32 j = 0; j < aNumberOfPowers; j++)
        {
            if (mForceIntercept)
                aPowers[j] += std::pow(x, j + 2);
            else
                aPowers[j] += std::pow(x, j);
        }

        for (sal_Int32 j = 0; j < aNoElements; j++)
        {
            if (mForceIntercept)
                aMatrix[j * aNoRows + aNoElements] += std::pow(x, j + 1) * ( y - mInterceptValue );
            else
                aMatrix[j * aNoRows + aNoElements] += std::pow(x, j) * y;
        }

        yAverage += y;
    }

    yAverage = yAverage / aNoValues;

    for (sal_Int32 y = 0; y < aNoElements; y++)
    {
        for (sal_Int32 x = 0; x < aNoElements; x++)
        {
            aMatrix[y * aNoRows + x] = aPowers[y + x];
        }
    }

    mResult.clear();
    mResult.resize(aNoElements, 0.0);

    solve(aMatrix, aNoColumns, aNoRows, mResult, 1.0e-20);

    // Set intercept value if force intercept is enabled
    if (mForceIntercept) {
        mResult.insert( mResult.begin(), mInterceptValue );
    }

    // Calculate correlation coeffitient
    double aSumError = 0.0;
    double aSumTotal = 0.0;

    for( size_t i = 0; i < aNoValues; ++i )
    {
        double x = aValues.first[i];
        double yActual = aValues.second[i];
        double yPredicted = getCurveValue( x );
        aSumTotal += (yActual - yAverage) * (yActual - yAverage);
        aSumError += (yActual - yPredicted) * (yActual - yPredicted);
    }

    double aRSquared = 1.0 - (aSumError / aSumTotal);
    if (aRSquared > 0.0)
        m_fCorrelationCoeffitient = std::sqrt(aRSquared);
    else
        m_fCorrelationCoeffitient = 0.0;
}

double SAL_CALL PolynomialRegressionCurveCalculator::getCurveValue( double x )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    double fResult;
    rtl::math::setNan(&fResult);

    if (mResult.empty())
    {
        return fResult;
    }

    fResult = 0.0;
    for (size_t i = 0; i<mResult.size(); i++)
    {
        fResult += mResult[i] * std::pow(x, i);
    }
    return fResult;
}

uno::Sequence< geometry::RealPoint2D > SAL_CALL PolynomialRegressionCurveCalculator::getCurveValues(
    double min, double max, sal_Int32 nPointCount,
    const uno::Reference< chart2::XScaling >& xScalingX,
    const uno::Reference< chart2::XScaling >& xScalingY,
    sal_Bool bMaySkipPointsInCalculation )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{

    return RegressionCurveCalculator::getCurveValues( min, max, nPointCount, xScalingX, xScalingY, bMaySkipPointsInCalculation );
}

OUString PolynomialRegressionCurveCalculator::ImplGetRepresentation(
    const uno::Reference< util::XNumberFormatter >& xNumFormatter,
    sal_Int32 nNumberFormatKey ) const
{
    OUStringBuffer aBuf( "f(x) = ");

    sal_Int32 aLastIndex = mResult.size() - 1;
    for (sal_Int32 i = aLastIndex; i >= 0; i--)
    {
        double aValue = mResult[i];
        if (aValue == 0.0)
        {
            continue;
        }
        else if (aValue < 0.0)
        {
            aBuf.appendAscii( " - " );
        }
        else
        {
            if (i != aLastIndex)
                aBuf.appendAscii( " + " );
        }

        aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, std::abs( aValue ) ) );

        if(i > 0)
        {
            aBuf.appendAscii( "x^" );
            aBuf.append(i);
        }
    }

    return aBuf.makeStringAndClear();
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
