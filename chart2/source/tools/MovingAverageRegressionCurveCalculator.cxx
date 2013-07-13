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

#include "MovingAverageRegressionCurveCalculator.hxx"
#include "RegressionCalculationHelper.hxx"
#include "macros.hxx"

#include <rtl/math.hxx>
#include <rtl/ustrbuf.hxx>

using namespace ::com::sun::star;


namespace chart
{

MovingAverageRegressionCurveCalculator::MovingAverageRegressionCurveCalculator()
{}

MovingAverageRegressionCurveCalculator::~MovingAverageRegressionCurveCalculator()
{}

// ____ XRegressionCurveCalculator ____
void SAL_CALL MovingAverageRegressionCurveCalculator::recalculateRegression(
    const uno::Sequence< double >& aXValues,
    const uno::Sequence< double >& aYValues )
    throw (uno::RuntimeException)
{
    ::rtl::math::setNan( & m_fCorrelationCoeffitient );

    RegressionCalculationHelper::tDoubleVectorPair aValues(
        RegressionCalculationHelper::cleanup(
            aXValues, aYValues,
            RegressionCalculationHelper::isValid()));

    const size_t aSize = aValues.first.size();

    aYList.clear();
    aXList.clear();

    for( size_t i = mPeriod - 1; i < aSize; ++i )
    {
        double yAvg;
        yAvg = 0.0;

        for (sal_Int32 j = 0; j < mPeriod; j++)
        {
            yAvg += aValues.second[i - j];
        }
        yAvg /= mPeriod;

        double x = aValues.first[i];
        aYList.push_back(yAvg);
        aXList.push_back(x);
    }
}

double SAL_CALL MovingAverageRegressionCurveCalculator::getCurveValue( double /*x*/ )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    double fResult;
    rtl::math::setNan(&fResult);
    return fResult;
}

uno::Sequence< geometry::RealPoint2D > SAL_CALL MovingAverageRegressionCurveCalculator::getCurveValues(
    double /*min*/, double /*max*/, sal_Int32 /*nPointCount*/,
    const uno::Reference< chart2::XScaling >& /*xScalingX*/,
    const uno::Reference< chart2::XScaling >& /*xScalingY*/,
    sal_Bool /*bMaySkipPointsInCalculation*/ )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    uno::Sequence< geometry::RealPoint2D > aResult( aYList.size() );

    for( size_t i = 0; i < aYList.size(); ++i )
    {
        aResult[i].X = aXList[i];
        aResult[i].Y = aYList[i];
    }
    return aResult;
}

OUString MovingAverageRegressionCurveCalculator::ImplGetRepresentation(
    const uno::Reference< util::XNumberFormatter >& /*xNumFormatter*/,
    ::sal_Int32 /*nNumberFormatKey*/ ) const
{
    OUStringBuffer aBuf( "f(x) = N/A");

    return aBuf.makeStringAndClear();
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
