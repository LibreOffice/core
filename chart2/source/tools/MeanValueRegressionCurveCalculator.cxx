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

#include <MeanValueRegressionCurveCalculator.hxx>

#include <osl/diagnose.h>

#include <cmath>
#include <limits>

using namespace ::com::sun::star;

namespace chart
{

MeanValueRegressionCurveCalculator::MeanValueRegressionCurveCalculator() :
        m_fMeanValue( std::numeric_limits<double>::quiet_NaN() )
{
}

MeanValueRegressionCurveCalculator::~MeanValueRegressionCurveCalculator()
{}

// ____ XRegressionCurveCalculator ____
void SAL_CALL MeanValueRegressionCurveCalculator::recalculateRegression(
    const uno::Sequence< double >& /*aXValues*/,
    const uno::Sequence< double >& aYValues )
{
    sal_Int32 nMax = aYValues.getLength();
    double fSumY = 0.0;

    for (double y : aYValues)
    {
        if (std::isnan(y) || std::isinf(y))
            --nMax;
        else
            fSumY += y;
    }

    m_fCorrelationCoefficient = 0.0;

    if( nMax == 0 )
    {
        m_fMeanValue = std::numeric_limits<double>::quiet_NaN();
    }
    else
    {
        m_fMeanValue = fSumY / static_cast< double >( nMax );

        // correlation coefficient: standard deviation
        if( nMax > 1 )
        {
            double fErrorSum = 0.0;
            for (double y : aYValues)
            {
                if (!std::isnan(y) && !std::isinf(y))
                {
                    double v = m_fMeanValue - y;
                    fErrorSum += (v*v);
                }
            }
            OSL_ASSERT( fErrorSum >= 0.0 );
            m_fCorrelationCoefficient = sqrt( fErrorSum / (nMax - 1 ));
        }
    }
}

double SAL_CALL MeanValueRegressionCurveCalculator::getCurveValue( double /*x*/ )
{
    return m_fMeanValue;
}

uno::Sequence< geometry::RealPoint2D > SAL_CALL MeanValueRegressionCurveCalculator::getCurveValues(
    double min, double max, ::sal_Int32 nPointCount,
    const uno::Reference< chart2::XScaling >& xScalingX,
    const uno::Reference< chart2::XScaling >& xScalingY,
    sal_Bool bMaySkipPointsInCalculation )
{
    if( bMaySkipPointsInCalculation )
    {
        // optimize result
        uno::Sequence< geometry::RealPoint2D > aResult{ { min, m_fMeanValue },
                                                        { max, m_fMeanValue } };

        return aResult;
    }
    return RegressionCurveCalculator::getCurveValues( min, max, nPointCount, xScalingX, xScalingY, bMaySkipPointsInCalculation );
}

OUString MeanValueRegressionCurveCalculator::ImplGetRepresentation(
    const uno::Reference< util::XNumberFormatter >& xNumFormatter,
    sal_Int32 nNumberFormatKey, sal_Int32* pFormulaLength /* = nullptr */ ) const
{
    OUString aBuf(mYName + " = ");
    if ( pFormulaLength )
    {
        *pFormulaLength -= aBuf.getLength();
        if ( *pFormulaLength <= 0 )
            return "###";
    }
    return ( aBuf + getFormattedString( xNumFormatter, nNumberFormatKey, m_fMeanValue, pFormulaLength ) );
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
