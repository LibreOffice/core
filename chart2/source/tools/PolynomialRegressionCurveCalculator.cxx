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

using namespace ::com::sun::star;


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
    ::rtl::math::setNan( & m_fCorrelationCoeffitient );

    RegressionCalculationHelper::tDoubleVectorPair aValues(
        RegressionCalculationHelper::cleanup( aXValues, aYValues, RegressionCalculationHelper::isValid()));

    const double EPSILON( 1.0e-20 );

    int aNumberOfPolyElements = mDegree + 1;
    int aNumberOfPowers = 2 * aNumberOfPolyElements - 1;

    std::vector<double> aPowers;
    aPowers.resize(aNumberOfPowers, 0.0);

    int aNumberOfColumns = aNumberOfPolyElements;
    int aNumberOfRows = aNumberOfPolyElements + 1;

    std::vector<double> aMatrix;
    aMatrix.resize(aNumberOfColumns*aNumberOfRows, 0.0);

    const size_t aSizeOfValues = aValues.first.size();

    double yAverage = 0.0;

    aPowers[0] += aSizeOfValues;

    for( size_t i = 0; i < aSizeOfValues; ++i )
    {
        double x = aValues.first[i];
        double y = aValues.second[i];

        for (int j = 1; j < aNumberOfPowers; j++) {
            aPowers[j] += pow(x, j);
        }

        for (int j = 0; j < aNumberOfPolyElements; j++) {
            aMatrix[j * aNumberOfRows + aNumberOfPolyElements] += pow(x, j) * y;
        }

        yAverage += y;
    }

    yAverage = yAverage / aSizeOfValues;

    for (int y = 0; y < aNumberOfPolyElements; y++) {
        for (int x = 0; x < aNumberOfPolyElements; x++) {
            aMatrix[y * aNumberOfRows + x] = aPowers[y + x];
        }
    }

    mResult.clear();
    mResult.resize(aNumberOfPolyElements, 0.0);

    solve(aMatrix, aNumberOfColumns, aNumberOfRows, mResult, EPSILON);

    // Calculate correlation coeffitient

    double aSumError = 0.0;
    double aSumTotal = 0.0;

    for( size_t i = 0; i < aSizeOfValues; ++i )
    {
        double x = aValues.first[i];
        double yActual = aValues.second[i];
        double yPredicted = getCurveValue( x );
        aSumTotal += (yActual - yAverage) * (yActual - yAverage);
        aSumError += (yActual - yPredicted) * (yActual - yPredicted);
    }

    m_fCorrelationCoeffitient = sqrt(1 - (aSumError / aSumTotal));

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
        fResult += mResult[i]*pow(x,i);
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
    ::sal_Int32 nNumberFormatKey ) const
{
    OUStringBuffer aBuf( "f(x) = ");

    for (int i=mResult.size()-1; i>=0; i--)
    {
        aBuf.append(getFormattedString( xNumFormatter, nNumberFormatKey, mResult[i] ));
        if(i > 0) {
            aBuf.appendAscii( "x^" );
            aBuf.append(i);
            aBuf.append(" + ");
        }
    }

    return aBuf.makeStringAndClear();
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
