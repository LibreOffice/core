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

#include "RegressionCurveCalculator.hxx"
#include "RegressionCalculationHelper.hxx"
#include "servicenames_coosystems.hxx"

#include <comphelper/processfactory.hxx>
#include <rtl/math.hxx>

#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/util/NumberFormatter.hpp>

#include <comphelper/numbers.hxx>
#include <comphelper/extract.hxx>

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart
{

RegressionCurveCalculator::RegressionCurveCalculator() :
        m_fCorrelationCoeffitient(0.0),
        mDegree(2),
        mForceIntercept(false),
        mInterceptValue(0.0),
        mPeriod(2),
        mXName("x"), mYName("f(x)")
{
    rtl::math::setNan( &m_fCorrelationCoeffitient );
    rtl::math::setNan( &mInterceptValue );
}

RegressionCurveCalculator::~RegressionCurveCalculator()
{}

bool RegressionCurveCalculator::isLinearScaling(
    const Reference< chart2::XScaling > & xScaling )
{
    // no scaling means linear
    if( !xScaling.is())
        return true;
    uno::Reference< lang::XServiceName > xServiceName( xScaling, uno::UNO_QUERY );
    return xServiceName.is() && xServiceName->getServiceName() == "com.sun.star.chart2.LinearScaling";
}

bool RegressionCurveCalculator::isLogarithmicScaling(
    const Reference< chart2::XScaling > & xScaling )
{
    uno::Reference< lang::XServiceName > xServiceName( xScaling, uno::UNO_QUERY );
    return xServiceName.is() && xServiceName->getServiceName() == "com.sun.star.chart2.LogarithmicScaling";
}

void RegressionCurveCalculator::setRegressionProperties(
    sal_Int32   aDegree,
    sal_Bool    aForceIntercept,
    double      aInterceptValue,
    sal_Int32   aPeriod )
{
    mDegree = aDegree;
    mForceIntercept = aForceIntercept;
    mInterceptValue = aInterceptValue;
    mPeriod  = aPeriod;
}

OUString RegressionCurveCalculator::getFormattedString(
    const Reference< util::XNumberFormatter >& xNumFormatter,
    sal_Int32 nNumberFormatKey,
    double fNumber, sal_Int32* pStringLength /* = nullptr */ )
{
    if ( pStringLength && *pStringLength <= 0 )
        return OUString("###");
    OUString aResult;

    if( xNumFormatter.is() )
    {
        bool bStandard = ::cppu::any2bool( ::comphelper::getNumberFormatProperty( xNumFormatter, nNumberFormatKey, "StandardFormat" ) );
        if( pStringLength && bStandard )
        {   // round fNumber to *pStringLength characters
            const sal_Int32 nMinDigit = 6; // minimum significant digits for General format
            sal_Int32 nSignificantDigit = ( *pStringLength <= nMinDigit ? nMinDigit : *pStringLength );
            aResult = OStringToOUString(
                        ::rtl::math::doubleToString( fNumber, rtl_math_StringFormat_G1, nSignificantDigit, '.', true ),
                                        RTL_TEXTENCODING_ASCII_US );
            // count characters different from significant digits (decimal separator, scientific notation)
            sal_Int32 nExtraChar = aResult.getLength() - *pStringLength;
            if ( nExtraChar > 0 && *pStringLength > nMinDigit )
            {
                nSignificantDigit = *pStringLength - nExtraChar;
                if ( nSignificantDigit < nMinDigit )
                    nSignificantDigit = nMinDigit;
                aResult = OStringToOUString(
                    ::rtl::math::doubleToString( fNumber, rtl_math_StringFormat_G1, nSignificantDigit, '.', true ),
                                            RTL_TEXTENCODING_ASCII_US );
            }
            fNumber = ::rtl::math::stringToDouble( aResult, '.', ',' );
        }
        aResult = xNumFormatter->convertNumberToString( nNumberFormatKey, fNumber );
    }
    else
    {
        sal_Int32 nStringLength = 4;  // default length
        if ( pStringLength )
            nStringLength = *pStringLength;
        aResult = OStringToOUString(
                      ::rtl::math::doubleToString( fNumber, rtl_math_StringFormat_G1, nStringLength, '.', true ),
                      RTL_TEXTENCODING_ASCII_US );
    }
    return aResult;
}

Sequence< geometry::RealPoint2D > SAL_CALL RegressionCurveCalculator::getCurveValues(
    double min, double max, ::sal_Int32 nPointCount,
    const Reference< chart2::XScaling >& xScalingX,
    const Reference< chart2::XScaling >& /* xScalingY */,
    sal_Bool /* bMaySkipPointsInCalculation */ )
{
    if( nPointCount < 2 )
        throw lang::IllegalArgumentException();

    // determine if scaling and inverse scaling for x-values work
    bool bDoXScaling( xScalingX.is());
    uno::Reference< chart2::XScaling > xInverseScaling;
    if( bDoXScaling )
        xInverseScaling.set( xScalingX->getInverseScaling());
    bDoXScaling = bDoXScaling && xInverseScaling.is();

    Sequence< geometry::RealPoint2D > aResult( nPointCount );

    double fMin( min );
    double fFact = (max - min) / double(nPointCount-1);

    if( bDoXScaling )
    {
        fMin = xScalingX->doScaling( min );
        fFact = (xScalingX->doScaling( max ) - fMin) / double(nPointCount-1);
    }

    for(sal_Int32 nP=0; nP<nPointCount; nP++)
    {
        double x = fMin + nP * fFact;
        if( bDoXScaling )
            x = xInverseScaling->doScaling( x );
        aResult[nP].X = x;
        aResult[nP].Y = this->getCurveValue( x );
    }

    return aResult;
}

double SAL_CALL RegressionCurveCalculator::getCorrelationCoefficient()
{
    return m_fCorrelationCoeffitient;
}

OUString SAL_CALL RegressionCurveCalculator::getRepresentation()
{
    return ImplGetRepresentation( Reference< util::XNumberFormatter >(), 0 );
}

OUString SAL_CALL RegressionCurveCalculator::getFormattedRepresentation(
    const Reference< util::XNumberFormatsSupplier > & xNumFmtSupplier,
    sal_Int32 nNumberFormatKey, sal_Int32 nFormulaLength )
{
    // create and prepare a number formatter
    if( !xNumFmtSupplier.is())
        return getRepresentation();
    Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext(), uno::UNO_QUERY_THROW );
    Reference< util::XNumberFormatter > xNumFormatter( util::NumberFormatter::create(xContext), uno::UNO_QUERY_THROW );
    xNumFormatter->attachNumberFormatsSupplier( xNumFmtSupplier );

    if ( nFormulaLength > 0 )
        return ImplGetRepresentation( xNumFormatter, nNumberFormatKey, &nFormulaLength );
    return ImplGetRepresentation( xNumFormatter, nNumberFormatKey );
}

void RegressionCurveCalculator::addStringToEquation(
        OUStringBuffer& aStrEquation, sal_Int32& nLineLength, OUStringBuffer& aAddString, sal_Int32* pMaxWidth)
{
    if ( pMaxWidth && ( nLineLength + aAddString.getLength() > *pMaxWidth ) )
    {  // wrap line
        aStrEquation.append( "\n " ); // start new line with a blank
        nLineLength = 1;
    }
    aStrEquation.append( aAddString );
    nLineLength += aAddString.getLength();
}

void SAL_CALL RegressionCurveCalculator::setXYNames( const OUString& aXName, const OUString& aYName )
{
    if ( aXName.isEmpty() )
        mXName = OUString ("x");
    else
        mXName = aXName;
    if ( aYName.isEmpty() )
        mYName = OUString ("f(x)");
    else
        mYName = aYName;
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
