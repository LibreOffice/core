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
    throw (uno::RuntimeException, std::exception)
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
           uno::RuntimeException, std::exception)
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
    sal_Bool bMaySkipPointsInCalculation )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException, std::exception)
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
    ::sal_Int32 nNumberFormatKey, ::sal_Int32 nFormulaLength /* = 0 */ ) const
{
    OUString aHash("###");
    if ( nFormulaLength > 0 && nFormulaLength <= 7 )
        return aHash;
    OUStringBuffer aBuf( "f(x) = ");
    if ( nFormulaLength > 0 )
        nFormulaLength -= aBuf.getLength();

    bool bHaveSlope = false;
    if ( m_fIntercept != 0.0 && nFormulaLength > 0 )
    {
        if ( nFormulaLength <= 3 )
            return aHash;
        nFormulaLength -= 3; // " + "
    }

    if( m_fSlope != 0.0 )
    {
        if ( nFormulaLength > 0 && nFormulaLength <= 5 )
            return aHash;
        if ( nFormulaLength > 0 )
            nFormulaLength -= 5; // "ln(x)"
        if( ::rtl::math::approxEqual( fabs( m_fSlope ), 1.0 ))
        {
            if( m_fSlope < 0.0 )
            {
                aBuf.append( UC_MINUS_SIGN );
                if ( nFormulaLength == 1)
                    return aHash;
                if ( nFormulaLength > 1 )
                    nFormulaLength --;
            }
        }
        else
        {
            if ( nFormulaLength > 0 && m_fIntercept != 0.0 )
            {
                nFormulaLength/=2;
                if ( nFormulaLength == 0 )
                    return aHash;
            }
            aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, m_fSlope, nFormulaLength ));
            aBuf.append( UC_SPACE );
        }
        aBuf.append( "ln(x)" );
        bHaveSlope = true;
    }

    if( bHaveSlope )
    {
        if( m_fIntercept < 0.0 )
        {
            aBuf.append( UC_SPACE );
            aBuf.append( UC_MINUS_SIGN );
            aBuf.append( UC_SPACE );
            aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, fabs( m_fIntercept ), nFormulaLength ));
        }
        else if( m_fIntercept > 0.0 )
        {
            aBuf.append( " + " );
            aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, m_fIntercept, nFormulaLength ));
        }
    }
    else
    {
        aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, m_fIntercept, nFormulaLength ));
    }

    return aBuf.makeStringAndClear();
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
