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

#include <SpecialUnicodes.hxx>

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
    throw (uno::RuntimeException, std::exception)
{
    rtl::math::setNan(&m_fCorrelationCoeffitient);

    RegressionCalculationHelper::tDoubleVectorPair aValues(
        RegressionCalculationHelper::cleanup( aXValues, aYValues, RegressionCalculationHelper::isValid()));

    const sal_Int32 aNoValues = aValues.first.size();

    const sal_Int32 aNoPowers = mForceIntercept ? mDegree : mDegree + 1;

    mCoefficients.clear();
    mCoefficients.resize(aNoPowers, 0.0);

    double yAverage = 0.0;

    std::vector<double> aQRTransposed;
    aQRTransposed.resize(aNoValues * aNoPowers, 0.0);

    std::vector<double> yVector;
    yVector.resize(aNoValues, 0.0);

    for(sal_Int32 i = 0; i < aNoValues; i++)
    {
        double yValue = aValues.second[i];
        if (mForceIntercept)
            yValue -= mInterceptValue;
        yVector[i] = yValue;
        yAverage += yValue;
    }
    yAverage /= aNoValues;

    for(sal_Int32 j = 0; j < aNoPowers; j++)
    {
        sal_Int32 aPower = mForceIntercept ? j+1 : j;
        sal_Int32 aColumnIndex = j * aNoValues;
        for(sal_Int32 i = 0; i < aNoValues; i++)
        {
            double xValue = aValues.first[i];
            aQRTransposed[i + aColumnIndex] = std::pow(xValue, (int) aPower);
        }
    }

    // QR decomposition - based on org.apache.commons.math.linear.QRDecomposition from apache commons math (ASF)
    sal_Int32 aMinorSize = std::min(aNoValues, aNoPowers);

    std::vector<double> aDiagonal;
    aDiagonal.resize(aMinorSize, 0.0);

    // Calculate Householder reflectors
    for (sal_Int32 aMinor = 0; aMinor < aMinorSize; aMinor++)
    {
        double aNormSqr = 0.0;
        for (sal_Int32 x = aMinor; x < aNoValues; x++)
        {
            double c = aQRTransposed[x + aMinor * aNoValues];
            aNormSqr += c * c;
        }

        double a;

        if (aQRTransposed[aMinor + aMinor * aNoValues] > 0.0)
            a = -std::sqrt(aNormSqr);
        else
            a = std::sqrt(aNormSqr);

        aDiagonal[aMinor] = a;

        if (a != 0.0)
        {
            aQRTransposed[aMinor + aMinor * aNoValues] -= a;

            for (sal_Int32 aColumn = aMinor + 1; aColumn < aNoPowers; aColumn++)
            {
                double alpha = 0.0;
                for (sal_Int32 aRow = aMinor; aRow < aNoValues; aRow++)
                {
                    alpha -= aQRTransposed[aRow + aColumn * aNoValues] * aQRTransposed[aRow + aMinor * aNoValues];
                }
                alpha /= a * aQRTransposed[aMinor + aMinor * aNoValues];

                for (sal_Int32 aRow = aMinor; aRow < aNoValues; aRow++)
                {
                    aQRTransposed[aRow + aColumn * aNoValues] -= alpha * aQRTransposed[aRow + aMinor * aNoValues];
                }
            }
        }
    }

    // Solve the linear equation
    for (sal_Int32 aMinor = 0; aMinor < aMinorSize; aMinor++)
    {
        double aDotProduct = 0;

        for (sal_Int32 aRow = aMinor; aRow < aNoValues; aRow++)
        {
            aDotProduct += yVector[aRow] * aQRTransposed[aRow + aMinor * aNoValues];
        }
        aDotProduct /= aDiagonal[aMinor] * aQRTransposed[aMinor + aMinor * aNoValues];

        for (sal_Int32 aRow = aMinor; aRow < aNoValues; aRow++)
        {
            yVector[aRow] += aDotProduct * aQRTransposed[aRow + aMinor * aNoValues];
        }

    }

    for (sal_Int32 aRow = aDiagonal.size() - 1; aRow >= 0; aRow--)
    {
        yVector[aRow] /= aDiagonal[aRow];
        double yRow = yVector[aRow];
        mCoefficients[aRow] = yRow;

        for (sal_Int32 i = 0; i < aRow; i++)
        {
            yVector[i] -= yRow * aQRTransposed[i + aRow * aNoValues];
        }
    }

    if(mForceIntercept)
    {
        mCoefficients.insert(mCoefficients.begin(), mInterceptValue);
    }

    // Calculate correlation coeffitient
    double aSumError = 0.0;
    double aSumTotal = 0.0;
    double aSumYpred2 = 0.0;

    for( sal_Int32 i = 0; i < aNoValues; i++ )
    {
        double xValue = aValues.first[i];
        double yActual = aValues.second[i];
        double yPredicted = getCurveValue( xValue );
        aSumTotal += (yActual - yAverage) * (yActual - yAverage);
        aSumError += (yActual - yPredicted) * (yActual - yPredicted);
        if(mForceIntercept)
            aSumYpred2 += (yPredicted - mInterceptValue) * (yPredicted - mInterceptValue);
    }

    double aRSquared = 0.0;
    if(mForceIntercept)
    {
        aRSquared = aSumYpred2 / (aSumError + aSumYpred2);
    }
    else
    {
        aRSquared = 1.0 - (aSumError / aSumTotal);
    }

    if (aRSquared > 0.0)
        m_fCorrelationCoeffitient = std::sqrt(aRSquared);
    else
        m_fCorrelationCoeffitient = 0.0;
}

double SAL_CALL PolynomialRegressionCurveCalculator::getCurveValue( double x )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException, std::exception)
{
    double fResult;
    rtl::math::setNan(&fResult);

    if (mCoefficients.empty())
    {
        return fResult;
    }

    sal_Int32 aNoCoefficients = (sal_Int32) mCoefficients.size();

    // Horner's method
    fResult = 0.0;
    for (sal_Int32 i = aNoCoefficients - 1; i >= 0; i--)
    {
        fResult = mCoefficients[i] + (x * fResult);
    }
    return fResult;
}

OUString PolynomialRegressionCurveCalculator::ImplGetRepresentation(
    const uno::Reference< util::XNumberFormatter >& xNumFormatter,
    sal_Int32 nNumberFormatKey, sal_Int32* pFormulaMaxWidth /* = nullptr */ ) const
{
    OUStringBuffer aBuf( "f(x) = " );

    sal_Int32 nValueLength=0;
    sal_Int32 aLastIndex = mCoefficients.size() - 1;

    if ( pFormulaMaxWidth && *pFormulaMaxWidth > 0 )
    {
        sal_Int32 nCharMin = aBuf.getLength(); // count characters different from coefficients
        double nCoefficients = aLastIndex + 1.0; // number of coefficients
        for (sal_Int32 i = aLastIndex; i >= 0; i--)
        {
            double aValue = mCoefficients[i];
            if ( aValue == 0.0 )
            { // do not count coeffitient if it is 0
                nCoefficients --;
                continue;
            }
            if ( rtl::math::approxEqual( fabs( aValue ) , 1.0 ) )
            { // do not count coeffitient if it is 1
                nCoefficients --;
                if ( i == 0 ) // intercept = 1
                    nCharMin ++;
            }
            if ( i != aLastIndex )
                nCharMin += 3; // " + "
            if ( i > 0 )
            {
                 nCharMin += 1; // "x"
                if ( i > 1 )
                    nCharMin +=1; // "^i"
                if ( i >= 10 )
                    nCharMin ++; // 2 digits for i
            }
        }
        nValueLength = ( *pFormulaMaxWidth - nCharMin ) / nCoefficients;
        if ( nValueLength <= 0 )
            nValueLength = 1;
    }

    bool bFindValue = false;
    sal_Int32 nLineLength = aBuf.getLength();
    for (sal_Int32 i = aLastIndex; i >= 0; i--)
    {
        double aValue = mCoefficients[i];
        OUStringBuffer aTmpBuf(""); // temporary buffer
        if (aValue == 0.0)
        {
            continue;
        }
        else if (aValue < 0.0)
        {
            if ( bFindValue ) // if it is not the first aValue
                aTmpBuf.append( " " );
            aTmpBuf.append(aMinusSign).append(" ");
            aValue = - aValue;
        }
        else
        {
            if ( bFindValue ) // if it is not the first aValue
                aTmpBuf.append( " + " );
        }
        bFindValue = true;

        // if nValueLength not calculated then nullptr
        sal_Int32* pValueLength = nValueLength ? &nValueLength : nullptr;
        OUString aValueString = getFormattedString( xNumFormatter, nNumberFormatKey, aValue, pValueLength );
        if ( i == 0 || aValueString != "1" )  // aValueString may be rounded to 1 if nValueLength is small
            aTmpBuf.append( aValueString );

        if(i > 0)
        {
            aTmpBuf.append( "x" );
            if (i > 1)
            {
                if (i < 10) // simple case if only one digit
                    aTmpBuf.append( aSuperscriptFigures[ i ] );
                else
                {
                    OUString aValueOfi = OUString::number( i );
                    for ( sal_Int32 n = 0; n < aValueOfi.getLength() ; n++ )
                    {
                        sal_Int32 nIndex = aValueOfi[n] - sal_Unicode ( '0' );
                        aTmpBuf.append( aSuperscriptFigures[ nIndex ] );
                    }
                }
            }
        }
        addStringToEquation( aBuf, nLineLength, aTmpBuf, pFormulaMaxWidth );
    }
    if ( aBuf.toString() == "f(x) = " )
        aBuf.append( "0" );

    return aBuf.makeStringAndClear();
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
