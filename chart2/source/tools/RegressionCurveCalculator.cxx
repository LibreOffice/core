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

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace chart
{

RegressionCurveCalculator::RegressionCurveCalculator() :
        m_fCorrelationCoeffitient( 0.0 )
{
    ::rtl::math::setNan( & m_fCorrelationCoeffitient );
}

RegressionCurveCalculator::~RegressionCurveCalculator()
{}

bool RegressionCurveCalculator::isLinearScaling(
    const Reference< chart2::XScaling > & xScaling )
{
    // no scaling means linear
    if( !xScaling.is())
        return true;
    static OUString aLinScalingServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.LinearScaling" ));
    uno::Reference< lang::XServiceName > xServiceName( xScaling, uno::UNO_QUERY );
    return (xServiceName.is() && xServiceName->getServiceName().equals( aLinScalingServiceName ));
}

bool RegressionCurveCalculator::isLogarithmicScaling(
    const Reference< chart2::XScaling > & xScaling )
{
    static OUString aLogScalingServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.chart2.LogarithmicScaling" ));
    uno::Reference< lang::XServiceName > xServiceName( xScaling, uno::UNO_QUERY );
    return (xServiceName.is() && xServiceName->getServiceName().equals( aLogScalingServiceName ));
}


OUString RegressionCurveCalculator::getFormattedString(
    const Reference< util::XNumberFormatter >& xNumFormatter,
    ::sal_Int32 nNumberFormatKey,
    double fNumber ) const
{
    OUString aResult;

    if( xNumFormatter.is())
        aResult = xNumFormatter->convertNumberToString( nNumberFormatKey, fNumber );
    else
        aResult = NUMBER_TO_STR( fNumber );

    return aResult;
}

Sequence< geometry::RealPoint2D > SAL_CALL RegressionCurveCalculator::getCurveValues(
    double min, double max, ::sal_Int32 nPointCount,
    const Reference< chart2::XScaling >& xScalingX,
    const Reference< chart2::XScaling >& /* xScalingY */,
    ::sal_Bool /* bMaySkipPointsInCalculation */ )
    throw (lang::IllegalArgumentException,
           uno::RuntimeException)
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
    throw (uno::RuntimeException)
{
    return m_fCorrelationCoeffitient;
}

OUString SAL_CALL RegressionCurveCalculator::getRepresentation()
    throw (uno::RuntimeException)
{
    return ImplGetRepresentation( Reference< util::XNumberFormatter >(), 0 );
}

OUString SAL_CALL RegressionCurveCalculator::getFormattedRepresentation(
    const Reference< util::XNumberFormatsSupplier > & xNumFmtSupplier,
    ::sal_Int32 nNumberFormatKey )
    throw (uno::RuntimeException)
{
    // create and prepare a number formatter
    if( !xNumFmtSupplier.is())
        return getRepresentation();
    Reference< uno::XComponentContext > xContext( comphelper::getProcessComponentContext(), uno::UNO_QUERY_THROW );
    Reference< util::XNumberFormatter > xNumFormatter( util::NumberFormatter::create(xContext), uno::UNO_QUERY_THROW );
    xNumFormatter->attachNumberFormatsSupplier( xNumFmtSupplier );

    return ImplGetRepresentation( xNumFormatter, nNumberFormatKey );
}


} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
