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

#include "MeanValueRegressionCurveCalculator.hxx"
#include "macros.hxx"

#include <rtl/math.hxx>
#include <rtl/ustrbuf.hxx>

using namespace ::com::sun::star;

namespace chart
{

MeanValueRegressionCurveCalculator::MeanValueRegressionCurveCalculator() :
        m_fMeanValue( 0.0 )
{
    ::rtl::math::setNan( & m_fMeanValue );
}

MeanValueRegressionCurveCalculator::~MeanValueRegressionCurveCalculator()
{}

// ____ XRegressionCurveCalculator ____
void SAL_CALL MeanValueRegressionCurveCalculator::recalculateRegression(
    const uno::Sequence< double >& /*aXValues*/,
    const uno::Sequence< double >& aYValues )
    throw (uno::RuntimeException)
{
    const sal_Int32 nDataLength = aYValues.getLength();
    sal_Int32 nMax = nDataLength;
    double fSumY = 0.0;
    const double * pY = aYValues.getConstArray();

    for( sal_Int32 i = 0; i < nDataLength; ++i )
    {
        if( ::rtl::math::isNan( pY[i] ) ||
            ::rtl::math::isInf( pY[i] ))
            --nMax;
        else
            fSumY += pY[i];
    }

    m_fCorrelationCoeffitient = 0.0;

    if( nMax == 0 )
    {
        ::rtl::math::setNan( & m_fMeanValue );
    }
    else
    {
        m_fMeanValue = fSumY / static_cast< double >( nMax );

        // correlation coefficient: standard deviation
        if( nMax > 1 )
        {
            double fErrorSum = 0.0;
            for( sal_Int32 i = 0; i < nDataLength; ++i )
            {
                if( !::rtl::math::isNan( pY[i] ) &&
                    !::rtl::math::isInf( pY[i] ))
                {
                    double v = m_fMeanValue - pY[i];
                    fErrorSum += (v*v);
                }
            }
            OSL_ASSERT( fErrorSum >= 0.0 );
            m_fCorrelationCoeffitient = sqrt( fErrorSum / (nMax - 1 ));
        }
    }
}

double SAL_CALL MeanValueRegressionCurveCalculator::getCurveValue( double /*x*/ )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    return m_fMeanValue;
}

uno::Sequence< geometry::RealPoint2D > SAL_CALL MeanValueRegressionCurveCalculator::getCurveValues(
    double min, double max, ::sal_Int32 nPointCount,
    const uno::Reference< chart2::XScaling >& xScalingX,
    const uno::Reference< chart2::XScaling >& xScalingY,
    ::sal_Bool bMaySkipPointsInCalculation )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    if( bMaySkipPointsInCalculation )
    {
        // optimize result
        uno::Sequence< geometry::RealPoint2D > aResult( 2 );
        aResult[0].X = min;
        aResult[0].Y = m_fMeanValue;
        aResult[1].X = max;
        aResult[1].Y = m_fMeanValue;

        return aResult;
    }
    return RegressionCurveCalculator::getCurveValues( min, max, nPointCount, xScalingX, xScalingY, bMaySkipPointsInCalculation );
}

OUString MeanValueRegressionCurveCalculator::ImplGetRepresentation(
    const uno::Reference< util::XNumberFormatter >& xNumFormatter,
    ::sal_Int32 nNumberFormatKey ) const
{
    OUString aBuf = "f(x) = " +
                    getFormattedString( xNumFormatter, nNumberFormatKey, m_fMeanValue );

    return aBuf;
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
