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
#ifndef INCLUDED_CHART2_SOURCE_INC_REGRESSIONCURVEHELPER_HXX
#define INCLUDED_CHART2_SOURCE_INC_REGRESSIONCURVEHELPER_HXX

#include <com/sun/star/chart2/XRegressionCurve.hpp>
#include <com/sun/star/chart2/XRegressionCurveCalculator.hpp>
#include <com/sun/star/chart2/XRegressionCurveContainer.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include "charttoolsdllapi.hxx"

#include <vector>

namespace chart
{

class OOO_DLLPUBLIC_CHARTTOOLS RegressionCurveHelper
{
public:
    /// returns a model mean-value line
    SAL_DLLPRIVATE static css::uno::Reference<css::chart2::XRegressionCurve>
        createMeanValueLine(const css::uno::Reference<css::uno::XComponentContext> & xContext );

    /// returns a model regression curve
    SAL_DLLPRIVATE static css::uno::Reference<css::chart2::XRegressionCurve>
        createRegressionCurveByServiceName(
            const css::uno::Reference<css::uno::XComponentContext> & xContext,
            const OUString& aServiceName );

    static bool hasMeanValueLine(
        const css::uno::Reference<css::chart2::XRegressionCurveContainer> & xRegCnt );

    static bool isMeanValueLine(
        const css::uno::Reference<css::chart2::XRegressionCurve> & xRegCurve );

    static css::uno::Reference<css::chart2::XRegressionCurve>
        getMeanValueLine(
            const css::uno::Reference<css::chart2::XRegressionCurveContainer> & xRegCnt );

    /** creates a mean-value line and adds it to the container.

         @param xSeriesProp
             If set, this property-set will be used to apply a line color
     */
    static void addMeanValueLine(
        css::uno::Reference<css::chart2::XRegressionCurveContainer>& xRegCnt,
        const css::uno::Reference<css::uno::XComponentContext>& xContext,
        const css::uno::Reference<css::beans::XPropertySet>& xSeriesProp );

    static void removeMeanValueLine(
        css::uno::Reference<css::chart2::XRegressionCurveContainer>& xRegCnt );

    enum tRegressionType
    {
        REGRESSION_TYPE_NONE,
        REGRESSION_TYPE_LINEAR,
        REGRESSION_TYPE_LOG,
        REGRESSION_TYPE_EXP,
        REGRESSION_TYPE_POWER,
        REGRESSION_TYPE_POLYNOMIAL,
        REGRESSION_TYPE_MOVING_AVERAGE,
        REGRESSION_TYPE_MEAN_VALUE,
        REGRESSION_TYPE_UNKNOWN
    };

    /** Returns the first regression curve found that is not of type
        mean-value line
     */
    static css::uno::Reference<com::sun::star::chart2::XRegressionCurve>
        getFirstCurveNotMeanValueLine(
            const css::uno::Reference<css::chart2::XRegressionCurveContainer>& xCurveContainer );

    /** Returns the regression curve found at the index provided.
     */
    static css::uno::Reference<css::chart2::XRegressionCurve>
        getRegressionCurveAtIndex(
            const css::uno::Reference<css::chart2::XRegressionCurveContainer>& xCurveContainer,
            sal_Int32 aIndex);

    /** Returns the type of the first regression curve found that is not of type
        mean-value line
     */
    static tRegressionType getFirstRegressTypeNotMeanValueLine(
        const css::uno::Reference<css::chart2::XRegressionCurveContainer>& xCurveContainer );

    static tRegressionType getRegressionType(
        const css::uno::Reference<css::chart2::XRegressionCurve>& xCurve );

    /** @param xPropertySource is taken as source to copy all properties from if
               not null
        @param xEquationProperties is set at the new regression curve as
               equation properties if not null
    */
    static css::uno::Reference<css::chart2::XRegressionCurve>
        addRegressionCurve(
            tRegressionType eType,
            css::uno::Reference<css::chart2::XRegressionCurveContainer>& xCurveContainer,
            const css::uno::Reference<css::uno::XComponentContext>& xContext,
            const css::uno::Reference<css::beans::XPropertySet >& xPropertySource =
                    css::uno::Reference<css::beans::XPropertySet>(),
            const css::uno::Reference<css::beans::XPropertySet>& xEquationProperties =
                    css::uno::Reference<css::beans::XPropertySet>() );

    static bool removeAllExceptMeanValueLine(
        css::uno::Reference<css::chart2::XRegressionCurveContainer>& xCurveContainer );

    static void removeEquations(
        css::uno::Reference<css::chart2::XRegressionCurveContainer>& xCurveContainer );

    static css::uno::Reference<css::chart2::XRegressionCurve>
        changeRegressionCurveType(
            tRegressionType eType,
            css::uno::Reference<css::chart2::XRegressionCurveContainer>& xRegressionCurveContainer,
            css::uno::Reference<css::chart2::XRegressionCurve>& xRegressionCurve,
            const css::uno::Reference<css::uno::XComponentContext>& xContext );

    /// returns a calculator object for regression curves (used by the view)
    static css::uno::Reference<css::chart2::XRegressionCurveCalculator>
        createRegressionCurveCalculatorByServiceName( const OUString& aServiceName );

    /** recalculates the regression parameters according to the data given in
        the data source.

        A sequence having the role "values-x" will be used as x-values for the
        calculation if found.  Otherwise a sequence (1, 2, 3, ...) of category
        indexes will be used for the recalculateRegression() method of the
        regression curve.

        The first sequence having the role "values-y" will be used as y-values
        for the recalculateRegression() method of the regression curve.

        @param bUseXValuesIfAvailable
            If false, the sequence (1, 2, 3, ...) will always be used, even if
            there is a data-sequence with role "values-x"
     */
    SAL_DLLPRIVATE static void initializeCurveCalculator(
        const css::uno::Reference<css::chart2::XRegressionCurveCalculator>& xOutCurveCalculator,
        const css::uno::Reference<css::chart2::data::XDataSource>& xSource,
        bool bUseXValuesIfAvailable = true );

    /** Same method as above, but uses the given XModel to determine the
        parameter bUseXValuesIfAvailable in the above function.  It is also
        necessary that the data::XDataSource is an XDataSeries, thus this parameter
        also changed.
     */
    static void initializeCurveCalculator(
        const css::uno::Reference<css::chart2::XRegressionCurveCalculator>& xOutCurveCalculator,
        const css::uno::Reference<css::chart2::XDataSeries>& xSeries,
        const css::uno::Reference<css::frame::XModel>& xModel );

    static OUString getUINameForRegressionCurve(
        const css::uno::Reference<css::chart2::XRegressionCurve>& xCurve );

    static OUString getRegressionCurveName(
        const css::uno::Reference<css::chart2::XRegressionCurve>& xCurve );

    static OUString getRegressionCurveGenericName(
        const css::uno::Reference<css::chart2::XRegressionCurve>& xCurve );

    static OUString getRegressionCurveSpecificName(
        const css::uno::Reference<css::chart2::XRegressionCurve>& xCurve );

    static std::vector<css::uno::Reference<css::chart2::XRegressionCurve> >
        getAllRegressionCurvesNotMeanValueLine(
                const css::uno::Reference<css::chart2::XDiagram>& xDiagram );

    static void resetEquationPosition(
        const css::uno::Reference<css::chart2::XRegressionCurve>& xCurve );

    /// @return the index of the given curve in the given container. -1 if not contained
    static sal_Int32 getRegressionCurveIndex(
        const css::uno::Reference<css::chart2::XRegressionCurveContainer>& xContainer,
        const css::uno::Reference<css::chart2::XRegressionCurve>& xCurve );

    static bool hasEquation(const css::uno::Reference<css::chart2::XRegressionCurve>& xCurve );

private:
    // instatiation not allowed
    RegressionCurveHelper();
};

} //  namespace chart

#endif // INCLUDED_CHART2_SOURCE_INC_REGRESSIONCURVEHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
