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
#ifndef INCLUDED_CHART2_SOURCE_INC_MOVINGAVERAGEREGRESSIONCURVECALCULATOR_HXX
#define INCLUDED_CHART2_SOURCE_INC_MOVINGAVERAGEREGRESSIONCURVECALCULATOR_HXX

#include "RegressionCurveCalculator.hxx"
#include <vector>

namespace chart
{

class MovingAverageRegressionCurveCalculator : public RegressionCurveCalculator
{
public:
    MovingAverageRegressionCurveCalculator();
    virtual ~MovingAverageRegressionCurveCalculator();

protected:
    virtual OUString ImplGetRepresentation(
        const css::uno::Reference<css::util::XNumberFormatter>& xNumFormatter,
        sal_Int32 nNumberFormatKey, sal_Int32* pFormulaLength = nullptr ) const override;

private:
    // ____ XRegressionCurveCalculator ____
    virtual void SAL_CALL recalculateRegression(
        const css::uno::Sequence<double>& aXValues,
        const css::uno::Sequence<double>& aYValues )
        throw (css::uno::RuntimeException, std::exception) override;

    virtual double SAL_CALL getCurveValue( double x )
        throw (css::lang::IllegalArgumentException,
               css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence<css::geometry::RealPoint2D> SAL_CALL getCurveValues(
        double min,
        double max,
        sal_Int32 nPointCount,
        const css::uno::Reference<css::chart2::XScaling>& xScalingX,
        const css::uno::Reference<css::chart2::XScaling>& xScalingY,
        sal_Bool bMaySkipPointsInCalculation )
        throw (css::lang::IllegalArgumentException,
               css::uno::RuntimeException, std::exception) override;

    std::vector<double> aYList;
    std::vector<double> aXList;
};

} //  namespace chart

#endif // INCLUDED_CHART2_SOURCE_INC_MOVINGAVERAGEREGRESSIONCURVECALCULATOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
