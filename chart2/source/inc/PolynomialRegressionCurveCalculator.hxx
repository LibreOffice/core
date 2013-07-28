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
#ifndef CHART2_POLYNOMALREGRESSIONCURVECALCULATOR_HXX
#define CHART2_POLYNOMALREGRESSIONCURVECALCULATOR_HXX

#include "RegressionCurveCalculator.hxx"
#include <vector>

namespace chart
{

class PolynomialRegressionCurveCalculator : public RegressionCurveCalculator
{
public:
    PolynomialRegressionCurveCalculator();
    virtual ~PolynomialRegressionCurveCalculator();

protected:
    virtual OUString ImplGetRepresentation(
        const com::sun::star::uno::Reference< com::sun::star::util::XNumberFormatter >& xNumFormatter,
        sal_Int32 nNumberFormatKey ) const;

    virtual double SAL_CALL getCurveValue( double x )
        throw (com::sun::star::lang::IllegalArgumentException,
               com::sun::star::uno::RuntimeException);

private:
    // ____ XRegressionCurveCalculator ____
    virtual void SAL_CALL recalculateRegression(
        const com::sun::star::uno::Sequence< double >& aXValues,
        const com::sun::star::uno::Sequence< double >& aYValues )
        throw (com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence< com::sun::star::geometry::RealPoint2D > SAL_CALL getCurveValues(
        double min,
        double max,
        sal_Int32 nPointCount,
        const com::sun::star::uno::Reference< com::sun::star::chart2::XScaling >& xScalingX,
        const com::sun::star::uno::Reference< com::sun::star::chart2::XScaling >& xScalingY,
        sal_Bool bMaySkipPointsInCalculation )
        throw (com::sun::star::lang::IllegalArgumentException,
               com::sun::star::uno::RuntimeException);

    std::vector<double> mCoefficients;
};

} //  namespace chart

#endif // CHART2_POLYNOMALREGRESSIONCURVECALCULATOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
