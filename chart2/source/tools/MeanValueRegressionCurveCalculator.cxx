/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "MeanValueRegressionCurveCalculator.hxx"
#include "macros.hxx"

#include <rtl/math.hxx>
#include <rtl/ustrbuf.hxx>

using namespace ::com::sun::star;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

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
    OUStringBuffer aBuf( C2U( "f(x) = " ));

    aBuf.append( getFormattedString( xNumFormatter, nNumberFormatKey, m_fMeanValue ));

    return aBuf.makeStringAndClear();
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
