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

#include <LinearRegressionCurveCalculator.hxx>

using namespace ::com::sun::star;

namespace chart
{

LinearRegressionCurveCalculator::LinearRegressionCurveCalculator() :
    PolynomialRegressionCurveCalculator()
{}

LinearRegressionCurveCalculator::~LinearRegressionCurveCalculator()
{}

void LinearRegressionCurveCalculator::setRegressionProperties(
    sal_Int32 /*aDegree*/,
    sal_Bool  aForceIntercept,
    double    aInterceptValue,
    sal_Int32 aPeriod,
    sal_Int32 /*nMovingType*/)
{
    PolynomialRegressionCurveCalculator::setRegressionProperties(
                                            1,
                                            aForceIntercept,
                                            aInterceptValue,
                                            aPeriod,
                                            0);
}

uno::Sequence< geometry::RealPoint2D > SAL_CALL LinearRegressionCurveCalculator::getCurveValues(
    double min, double max, ::sal_Int32 nPointCount,
    const uno::Reference< chart2::XScaling >& xScalingX,
    const uno::Reference< chart2::XScaling >& xScalingY,
    sal_Bool bMaySkipPointsInCalculation )
{
    if( bMaySkipPointsInCalculation &&
        isLinearScaling( xScalingX ) &&
        isLinearScaling( xScalingY ))
    {
        // optimize result
        uno::Sequence< geometry::RealPoint2D > aResult( 2 );
        aResult[0].X = min;
        aResult[0].Y = getCurveValue( min );
        aResult[1].X = max;
        aResult[1].Y = getCurveValue( max );

        return aResult;
    }
    return RegressionCurveCalculator::getCurveValues( min, max, nPointCount, xScalingX, xScalingY, bMaySkipPointsInCalculation );
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
