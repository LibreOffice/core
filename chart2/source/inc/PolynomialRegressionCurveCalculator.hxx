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
#pragma once

#include "RegressionCurveCalculator.hxx"
#include <vector>

namespace RegressionCalculationHelper
{
    typedef std::pair< std::vector< double >, std::vector< double > > tDoubleVectorPair;
}

namespace chart
{

class PolynomialRegressionCurveCalculator : public RegressionCurveCalculator
{
public:
    PolynomialRegressionCurveCalculator();
    virtual ~PolynomialRegressionCurveCalculator() override;

protected:
    virtual OUString ImplGetRepresentation(
        const css::uno::Reference<css::util::XNumberFormatter>& xNumFormatter,
        sal_Int32 nNumberFormatKey, sal_Int32* pFormulaMaxWidth = nullptr ) const override;

    virtual double SAL_CALL getCurveValue( double x ) override;

private:
    // ____ XRegressionCurveCalculator ____
    virtual void SAL_CALL recalculateRegression(
        const css::uno::Sequence<double>& aXValues,
        const css::uno::Sequence<double>& aYValues ) override;

    void computeCorrelationCoefficient(
        RegressionCalculationHelper::tDoubleVectorPair& rValues,
        const sal_Int32 aNoValues,
        double yAverage );

    std::vector<double> mCoefficients;
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
