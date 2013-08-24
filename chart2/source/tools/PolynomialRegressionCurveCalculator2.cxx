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

#include <cmath>
#include <rtl/math.hxx>
#include <rtl/ustrbuf.hxx>

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

    const size_t aNoValues = aValues.first.size();

    std::vector<sal_Int32> aDegrees;

    printf("%d\n", mDegree);

    for( sal_Int32 i = 0; i < mDegree+1; i++ )
    {
        aDegrees.push_back(i);
    }

    float_50 yAverage = 0.0;

    std::vector<float_50> aTransposedMatrix;
    aTransposedMatrix.resize(aNoValues * aDegrees.size(), 0.0);

    std::vector<float_50> yVector;
    yVector.resize(aNoValues, 0.0);

    for( size_t i = 0; i < aNoValues; i++ )
    {
        float_50 x = aValues.first[i];
        float_50 y = aValues.second[i];
        yVector[i] = y;
        yAverage += y;

        for( size_t j = 0; j < aDegrees.size(); j++ )
        {
            aTransposedMatrix[i + j * aNoValues] = boost::multiprecision::pow((float_50)x, aDegrees[j]);
        }
    }
    yAverage /= aNoValues;

    sal_Int32 m = aNoValues;
    sal_Int32 n = aDegrees.size();

    sal_Int32 minorSize = std::min(m, n);

    std::vector<float_50> rDiag;
    rDiag.resize(minorSize, 0.0);

    for (sal_Int32 minor = 0; minor < minorSize; minor++)
    {
        float_50 xNormSqr = 0.0;
        for (sal_Int32 row = minor; row < m; row++)
        {
            float_50 c = aTransposedMatrix[row + minor * aNoValues];
            xNormSqr += c * c;
        }

        float_50 a = 0.0;
        if (aTransposedMatrix[minor + minor * aNoValues] > 0.0)
            a = -boost::multiprecision::sqrt(xNormSqr);
        else
            a = boost::multiprecision::sqrt(xNormSqr);
        rDiag[minor] = a;

        if (a != 0.0)
        {
            aTransposedMatrix[minor + minor * aNoValues] -= a;

            for (sal_Int32 col = minor + 1; col < n; col++)
            {
                float_50 alpha = 0;
                for (sal_Int32 row = minor; row < m; row++)
                {
                    alpha -= aTransposedMatrix[row + col * aNoValues] * aTransposedMatrix[row + minor * aNoValues];
                }
                alpha /= a * aTransposedMatrix[minor + minor * aNoValues];

                for (sal_Int32 row = minor; row < m; row++) {
                    aTransposedMatrix[row + col * aNoValues] -= alpha * aTransposedMatrix[row + minor * aNoValues];
                }
            }
        }
    }

    // apply Householder transforms to solve Q.y = b
    for (sal_Int32 minor = 0; minor < minorSize; minor++)
    {
        float_50 dotProduct = 0;

        for (sal_Int32 row = minor; row < m; row++)
        {
            dotProduct += yVector[row] * aTransposedMatrix[row + minor * aNoValues];
        }
        dotProduct /= rDiag[minor] * aTransposedMatrix[minor + minor * aNoValues];

        for (sal_Int32 row = minor; row < m; row++)
        {
            yVector[row] += dotProduct * aTransposedMatrix[row + minor * aNoValues];
        }

    }

    mResult.clear();
    mResult.resize(n, (float_50) 0.0);

    // solve triangular system R.x = y
    for (sal_Int32 row = rDiag.size() - 1; row >= 0; --row)
    {
        yVector[row] /= rDiag[row];
        float_50 yRow = yVector[row];
        mResult[row] = yRow;

        for (sal_Int32 i = 0; i < row; i++)
        {
            yVector[i] -= yRow * aTransposedMatrix[i + row * aNoValues];
        }
    }

    // Calculate correlation coeffitient
    float_50 aSumError = 0.0;
    float_50 aSumTotal = 0.0;

    for( size_t i = 0; i < aNoValues; ++i )
    {
        float_50 x = aValues.first[i];
        float_50 yActual = aValues.second[i];
        float_50 yPredicted = getCurveValue( (double) x );
        aSumTotal += (yActual - yAverage) * (yActual - yAverage);
        aSumError += (yActual - yPredicted) * (yActual - yPredicted);
    }

    float_50 aRSquared = 1.0 - (aSumError / aSumTotal);

    if (aRSquared > 0.0)
        aRSquared = boost::multiprecision::sqrt(aRSquared);
    else
        aRSquared = 0.0;
    m_fCorrelationCoeffitient = (double) aRSquared;
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

    float_50 m = 0.0;
    float_50 aa;
    for (size_t i = 0; i<mResult.size(); i++)
    {
        aa = boost::multiprecision::pow((float_50)x, i);
        aa = aa * mResult[i];
        m += aa;
    }
    fResult = (double) m;
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
        double aValue = (double) mResult[i];
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
            if (i == 1)
            {
                aBuf.appendAscii( "x" );
            }
            else
            {
                aBuf.appendAscii( "x^" );
                aBuf.append(i);
            }
        }
    }

    return aBuf.makeStringAndClear();
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
