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

#include "LogarithmicRegressionCurveCalculator.hxx"
#include "macros.hxx"
#include "RegressionCalculationHelper.hxx"

#include <rtl/math.hxx>
#include <rtl/ustrbuf.hxx>

using namespace ::com::sun::star;

namespace chart
{

LogarithmicRegressionCurveCalculator::LogarithmicRegressionCurveCalculator() :
        m_fSlope( 0.0 ),
        m_fIntercept( 0.0 )
{
    ::rtl::math::setNan( & m_fSlope );
    ::rtl::math::setNan( & m_fIntercept );
}

LogarithmicRegressionCurveCalculator::~LogarithmicRegressionCurveCalculator()
{}

// ____ XRegressionCurve ____
void SAL_CALL LogarithmicRegressionCurveCalculator::recalculateRegression(
    const uno::Sequence< double >& aXValues,
    const uno::Sequence< double >& aYValues )
    throw (uno::RuntimeException)
{
    RegressionCalculationHelper::tDoubleVectorPair aValues(
        RegressionCalculationHelper::cleanup(
            aXValues, aYValues,
            RegressionCalculationHelper::isValidAndXPositive()));

    const size_t nMax = aValues.first.size();
    if( nMax == 0 )
    {
        ::rtl::math::setNan( & m_fSlope );
        ::rtl::math::setNan( & m_fIntercept );
        ::rtl::math::setNan( & m_fCorrelationCoeffitient );
        return;
    }

    double fAverageX = 0.0, fAverageY = 0.0;
    size_t i = 0;
    for( i = 0; i < nMax; ++i )
    {
        fAverageX += log( aValues.first[i] );
        fAverageY += aValues.second[i];
    }

    const double fN = static_cast< double >( nMax );
    fAverageX /= fN;
    fAverageY /= fN;

    double fQx = 0.0, fQy = 0.0, fQxy = 0.0;
    for( i = 0; i < nMax; ++i )
    {
        double fDeltaX = log( aValues.first[i] ) - fAverageX;
        double fDeltaY = aValues.second[i] - fAverageY;

        fQx  += fDeltaX * fDeltaX;
        fQy  += fDeltaY * fDeltaY;
        fQxy += fDeltaX * fDeltaY;
    }

    m_fSlope = fQxy / fQx;
    m_fIntercept = fAverageY - m_fSlope * fAverageX;
    m_fCorrelationCoeffitient = fQxy / sqrt( fQx * fQy );
}

double SAL_CALL LogarithmicRegressionCurveCalculator::getCurveValue( double x )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    double fResult;
    ::rtl::math::setNan( & fResult );

    if( ! ( ::rtl::math::isNan( m_fSlope ) ||
            ::rtl::math::isNan( m_fIntercept )))
    {
        fResult = m_fSlope * log( x ) + m_fIntercept;
    }

    return fResult;
}

uno::Sequence< geometry::RealPoint2D > SAL_CALL LogarithmicRegressionCurveCalculator::getCurveValues(
    double min, double max, ::sal_Int32 nPointCount,
    const uno::Reference< chart2::XScaling >& xScalingX,
    const uno::Reference< chart2::XScaling >& xScalingY,
    ::sal_Bool bMaySkipPointsInCalculation )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
{
    if( bMaySkipPointsInCalculation &&
        isLogarithmicScaling( xScalingX ) &&
        isLinearScaling( xScalingY ))
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

OUString LogarithmicRegressionCurveCalculator::ImplGetRepresentation(
    const uno::Reference< util::XNumberFormatter >& xNumFormatter,
    ::sal_Int32 nNumberFormatKey ) const
{
    OUStringBuffer aBuf( "f(x) = ");

    bool bHaveSlope = false;

    if( m_fSlope != 0.0 )
    {
        if( ::rtl::math::approxEqual( fabs( m_fSlope ), 1.0 ))
        {
            if( m_fSlope < 0 )
                aBuf.append( UC_MINUS_SIGN );
        }
        else
        {
            aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, m_fSlope ));
            aBuf.append( UC_SPACE );
        }
        aBuf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "ln(x)" ));
        bHaveSlope = true;
    }

    if( bHaveSlope )
    {
        if( m_fIntercept < 0.0 )
        {
            aBuf.append( UC_SPACE );
            aBuf.append( UC_MINUS_SIGN );
            aBuf.append( UC_SPACE );
            aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, fabs( m_fIntercept )));
        }
        else if( m_fIntercept > 0.0 )
        {
            aBuf.appendAscii( RTL_CONSTASCII_STRINGPARAM( " + " ));
            aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, m_fIntercept ));
        }
    }
    else
    {
        aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, m_fIntercept ));
    }

    return aBuf.makeStringAndClear();
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
