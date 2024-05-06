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

#include <MovingAverageRegressionCurveCalculator.hxx>
#include <RegressionCalculationHelper.hxx>
#include <ResId.hxx>
#include <strings.hrc>

#include <algorithm>
#include <limits>

#include <com/sun/star/chart2/MovingAverageType.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

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
{
    m_fCorrelationCoefficient = std::numeric_limits<double>::quiet_NaN();

    RegressionCalculationHelper::tDoubleVectorPair aValues(
        RegressionCalculationHelper::cleanup(
            aXValues, aYValues,
            RegressionCalculationHelper::isValid()));

    aYList.clear();
    aXList.clear();

    // For formulas, see
    // https://docs.oasis-open.org/office/OpenDocument/v1.3/cs02/part3-schema/OpenDocument-v1.3-cs02-part3-schema.html#property-chart_regression-moving-type

    switch (mnMovingType)
    {
        case MovingAverageType::Central:
        {

            calculateValuesCentral(std::move(aValues));
            break;
        }

        case MovingAverageType::AveragedAbscissa:
        {
            calculateValues(std::move(aValues), true);
            break;
        }
        case MovingAverageType::Prior:
        default:
        {
            calculateValues(std::move(aValues), false);
            break;
        }
    }
}

void MovingAverageRegressionCurveCalculator::calculateValuesCentral(
    RegressionCalculationHelper::tDoubleVectorPair aValues)
{
    const size_t aSize = aValues.first.size();
    if (aSize == 0)
        return;
    for (size_t i = mPeriod - 1; i < aSize; ++i)
    {
        double yAvg = 0.0;

        for (sal_Int32 j = 0; j < mPeriod; j++)
        {
            yAvg += aValues.second[i - j];
        }
        yAvg /= mPeriod;
        aYList.push_back(yAvg);
    }
    sal_Int32 nPeriodLocal = (mPeriod % 2 == 0) ? (mPeriod / 2) : ((mPeriod - 1) / 2);
    for (size_t i = nPeriodLocal; i < aSize - 1; ++i)
    {
        double x = aValues.first[i];
        aXList.push_back(x);
    }
}

void MovingAverageRegressionCurveCalculator::calculateValues(
    RegressionCalculationHelper::tDoubleVectorPair aValues, bool bUseXAvg)
{
    const size_t aSize = aValues.first.size();
    for (size_t i = mPeriod - 1; i < aSize; ++i)
    {
        double xAvg = 0.0;
        double yAvg = 0.0;

        for (sal_Int32 j = 0; j < mPeriod; j++)
        {
            xAvg += aValues.first[i - j];
            yAvg += aValues.second[i - j];
        }
        yAvg /= mPeriod;
        xAvg /= mPeriod;

        aYList.push_back(yAvg);
        if (bUseXAvg)
        {
            aXList.push_back(xAvg);
        }
        else
        {
            double x = aValues.first[i];
            aXList.push_back(x);
        }
    }
}

double SAL_CALL MovingAverageRegressionCurveCalculator::getCurveValue( double /*x*/ )
{
    return std::numeric_limits<double>::quiet_NaN();
}

uno::Sequence< geometry::RealPoint2D > SAL_CALL MovingAverageRegressionCurveCalculator::getCurveValues(
    double /*min*/, double /*max*/, sal_Int32 /*nPointCount*/,
    const uno::Reference< chart2::XScaling >& /*xScalingX*/,
    const uno::Reference< chart2::XScaling >& /*xScalingY*/,
    sal_Bool /*bMaySkipPointsInCalculation*/ )
{
    size_t nSize = std::min(aXList.size(), aYList.size());
    uno::Sequence< geometry::RealPoint2D > aResult( nSize );
    std::transform(aXList.begin(), aXList.begin() + nSize, aYList.begin(), aResult.getArray(),
                   [](const auto& x, const auto& y) { return geometry::RealPoint2D(x, y); });
    return aResult;
}

OUString MovingAverageRegressionCurveCalculator::ImplGetRepresentation(
    const uno::Reference< util::XNumberFormatter >& /*xNumFormatter*/,
    sal_Int32 /*nNumberFormatKey*/, sal_Int32* /*pFormulaLength = nullptr */ ) const
{
    OUString aRet = SchResId( STR_OBJECT_MOVING_AVERAGE_WITH_PARAMETERS );
    // change text for Moving Average
    OUString aWildcard( u"%PERIOD"_ustr );
    sal_Int32 nIndex = aRet.indexOf( aWildcard );
    if( nIndex != -1 )
    {  // replace period
        aRet = aRet.replaceAt( nIndex, aWildcard.getLength(), OUString::number(mPeriod) );
    }
    return aRet;
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
