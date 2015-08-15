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

#include "PotentialRegressionCurveCalculator.hxx"
#include "macros.hxx"
#include "RegressionCalculationHelper.hxx"

#include <rtl/math.hxx>
#include <rtl/ustrbuf.hxx>

using namespace ::com::sun::star;

namespace chart
{

PotentialRegressionCurveCalculator::PotentialRegressionCurveCalculator()
    : m_fSlope(0.0)
    , m_fIntercept(0.0)
    , m_fSign(1.0)
{
    ::rtl::math::setNan( & m_fSlope );
    ::rtl::math::setNan( & m_fIntercept );
}

PotentialRegressionCurveCalculator::~PotentialRegressionCurveCalculator()
{}

// ____ XRegressionCurveCalculator ____
void SAL_CALL PotentialRegressionCurveCalculator::recalculateRegression(
    const uno::Sequence< double >& aXValues,
    const uno::Sequence< double >& aYValues )
    throw (uno::RuntimeException, std::exception)
{
    RegressionCalculationHelper::tDoubleVectorPair aValues(
        RegressionCalculationHelper::cleanup(
            aXValues, aYValues,
            RegressionCalculationHelper::isValidAndBothPositive()));
    m_fSign = 1.0;

    size_t nMax = aValues.first.size();
    if( nMax == 0 )
    {
        aValues = RegressionCalculationHelper::cleanup(
                    aXValues, aYValues,
                    RegressionCalculationHelper::isValidAndXPositiveAndYNegative());
         nMax = aValues.first.size();
         if( nMax == 0 )
         {
            ::rtl::math::setNan( & m_fSlope );
            ::rtl::math::setNan( & m_fIntercept );
            ::rtl::math::setNan( & m_fCorrelationCoeffitient );
            return;
         }
         m_fSign = -1.0;
    }

    double fAverageX = 0.0, fAverageY = 0.0;
    size_t i = 0;
    for( i = 0; i < nMax; ++i )
    {
        fAverageX += log( aValues.first[i] );
        fAverageY += log( m_fSign * aValues.second[i] );
    }

    const double fN = static_cast< double >( nMax );
    fAverageX /= fN;
    fAverageY /= fN;

    double fQx = 0.0, fQy = 0.0, fQxy = 0.0;
    for( i = 0; i < nMax; ++i )
    {
        double fDeltaX = log( aValues.first[i] ) - fAverageX;
        double fDeltaY = log( m_fSign * aValues.second[i] ) - fAverageY;

        fQx  += fDeltaX * fDeltaX;
        fQy  += fDeltaY * fDeltaY;
        fQxy += fDeltaX * fDeltaY;
    }

    m_fSlope = fQxy / fQx;
    m_fIntercept = fAverageY - m_fSlope * fAverageX;
    m_fCorrelationCoeffitient = fQxy / sqrt( fQx * fQy );

    m_fIntercept = m_fSign * exp( m_fIntercept );
}

double SAL_CALL PotentialRegressionCurveCalculator::getCurveValue( double x )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException, std::exception)
{
    double fResult;
    ::rtl::math::setNan( & fResult );

    if( ! ( ::rtl::math::isNan( m_fSlope ) ||
            ::rtl::math::isNan( m_fIntercept )))
    {
        fResult = m_fIntercept * pow( x, m_fSlope );
    }

    return fResult;
}

uno::Sequence< geometry::RealPoint2D > SAL_CALL PotentialRegressionCurveCalculator::getCurveValues(
    double min, double max, ::sal_Int32 nPointCount,
    const uno::Reference< chart2::XScaling >& xScalingX,
    const uno::Reference< chart2::XScaling >& xScalingY,
    sal_Bool bMaySkipPointsInCalculation )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException, std::exception)
{
    if( bMaySkipPointsInCalculation &&
        isLogarithmicScaling( xScalingX ) &&
        isLogarithmicScaling( xScalingY ))
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

OUString PotentialRegressionCurveCalculator::ImplGetRepresentation(
    const uno::Reference< util::XNumberFormatter >& xNumFormatter,
    ::sal_Int32 nNumberFormatKey, ::sal_Int32 nFormulaLength /* = 0 */ ) const
{
    if ( nFormulaLength > 0 && nFormulaLength <= 7 )
        return OUString("###");
    OUStringBuffer aBuf( "f(x) = ");
    if ( nFormulaLength > 0 )
        nFormulaLength -= aBuf.getLength();

    if( m_fIntercept == 0.0 )
    {
        aBuf.append( '0');
    }
    else if( m_fSlope == 0.0 )
    {
        aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, m_fIntercept, nFormulaLength ));
    }
    else
    {
        if( ! rtl::math::approxEqual( std::abs( m_fIntercept ), 1.0 ) )
        {
            if (nFormulaLength > 0 )
                nFormulaLength = ( nFormulaLength - 3 ) /2;
            aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, m_fIntercept, nFormulaLength ));
            aBuf.append( ' ');
        }
        else
        {
            if ( m_fIntercept < 0.0 )
                aBuf.append( "- " );
            if ( nFormulaLength > 0 )
                nFormulaLength -= 2 + (m_fIntercept < 0.0 ? 2 : 0);
        }
        aBuf.append( "x^" );
        aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, m_fSlope, nFormulaLength ));
    }

    return aBuf.makeStringAndClear();
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
