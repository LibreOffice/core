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
#ifndef INCLUDED_CHART2_SOURCE_INC_REGRESSIONCURVECALCULATOR_HXX
#define INCLUDED_CHART2_SOURCE_INC_REGRESSIONCURVECALCULATOR_HXX

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/chart2/XRegressionCurveCalculator.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>

namespace chart
{

class RegressionCurveCalculator : public
    cppu::WeakImplHelper< css::chart2::XRegressionCurveCalculator >
{
public:
    RegressionCurveCalculator();
    virtual ~RegressionCurveCalculator();

    static bool isLinearScaling(
        const css::uno::Reference< css::chart2::XScaling >& xScaling );

    static bool isLogarithmicScaling(
        const css::uno::Reference< css::chart2::XScaling >& xScaling );

protected:
    virtual OUString ImplGetRepresentation(
        const css::uno::Reference< css::util::XNumberFormatter >& xNumFormatter,
        sal_Int32 nNumberFormatKey ) const = 0;

    static OUString getFormattedString(
        const css::uno::Reference< css::util::XNumberFormatter >& xNumFormatter,
        sal_Int32 nNumberFormatKey,
        double fNumber );

    double m_fCorrelationCoeffitient;

    sal_Int32 mDegree;
    bool  mForceIntercept;
    double    mInterceptValue;
    sal_Int32 mPeriod;

    // ____ XRegressionCurveCalculator ____
    virtual void SAL_CALL setRegressionProperties(
        sal_Int32 aDegree,
        sal_Bool  aForceIntercept,
        double    aInterceptValue,
        sal_Int32 aPeriod)
            throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL recalculateRegression(
        const css::uno::Sequence< double >& aXValues,
        const css::uno::Sequence< double >& aYValues )
        throw (css::uno::RuntimeException, std::exception) override = 0;

    virtual double SAL_CALL getCurveValue( double x )
        throw (css::lang::IllegalArgumentException,
               css::uno::RuntimeException, std::exception) override = 0;

    virtual css::uno::Sequence< css::geometry::RealPoint2D > SAL_CALL getCurveValues(
        double min,
        double max,
        sal_Int32 nPointCount,
        const css::uno::Reference< css::chart2::XScaling >& xScalingX,
        const css::uno::Reference< css::chart2::XScaling >& xScalingY,
        sal_Bool bMaySkipPointsInCalculation )
        throw (css::lang::IllegalArgumentException,
               css::uno::RuntimeException, std::exception) override;

    virtual double SAL_CALL getCorrelationCoefficient()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL getRepresentation()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual OUString SAL_CALL getFormattedRepresentation(
        const css::uno::Reference< css::util::XNumberFormatsSupplier >& xNumFmtSupplier,
        sal_Int32 nNumberFormatKey )
        throw (css::uno::RuntimeException, std::exception) override;
};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_INC_REGRESSIONCURVECALCULATOR_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
