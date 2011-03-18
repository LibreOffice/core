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

#include "RegressionCurveCalculator.hxx"
#include "RegressionCalculationHelper.hxx"
#include "servicenames_coosystems.hxx"

#include <comphelper/processfactory.hxx>
#include <rtl/math.hxx>

#include <com/sun/star/lang/XServiceName.hpp>

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
    Reference< util::XNumberFormatter > xNumFormatter;
    Reference< lang::XMultiServiceFactory > xFact( comphelper::getProcessServiceFactory(), uno::UNO_QUERY );
    if( xFact.is())
        xNumFormatter.set( xFact->createInstance(
                               OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.NumberFormatter"))), uno::UNO_QUERY );
    if( !xNumFormatter.is())
        return getRepresentation();
    xNumFormatter->attachNumberFormatsSupplier( xNumFmtSupplier );

    return ImplGetRepresentation( xNumFormatter, nNumberFormatKey );
}


} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
