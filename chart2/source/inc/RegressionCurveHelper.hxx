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

#include <svx/chrtitem.hxx>
#include "charttoolsdllapi.hxx"

#include <vector>

namespace com::sun::star::beans { class XPropertySet; }
namespace com::sun::star::chart2 { class XDataSeries; }
namespace com::sun::star::chart2 { class XDiagram; }
namespace com::sun::star::chart2 { class XRegressionCurve; }
namespace com::sun::star::chart2 { class XRegressionCurveCalculator; }
namespace com::sun::star::chart2 { class XRegressionCurveContainer; }
namespace com::sun::star::chart2::data { class XDataSource; }
namespace com::sun::star::frame { class XModel; }

namespace chart::RegressionCurveHelper
{
    /// returns a model mean-value line
    css::uno::Reference<css::chart2::XRegressionCurve> createMeanValueLine();

    /// returns a model regression curve
    css::uno::Reference<css::chart2::XRegressionCurve>
        createRegressionCurveByServiceName( const OUString& aServiceName );

    OOO_DLLPUBLIC_CHARTTOOLS bool hasMeanValueLine(
        const css::uno::Reference<css::chart2::XRegressionCurveContainer> & xRegCnt );

    OOO_DLLPUBLIC_CHARTTOOLS bool isMeanValueLine(
        const css::uno::Reference<css::chart2::XRegressionCurve> & xRegCurve );

    OOO_DLLPUBLIC_CHARTTOOLS css::uno::Reference<css::chart2::XRegressionCurve>
        getMeanValueLine(
            const css::uno::Reference<css::chart2::XRegressionCurveContainer> & xRegCnt );

    /** creates a mean-value line and adds it to the container.

         @param xSeriesProp
             If set, this property-set will be used to apply a line color
     */
    OOO_DLLPUBLIC_CHARTTOOLS void addMeanValueLine(
        css::uno::Reference<css::chart2::XRegressionCurveContainer> const & xRegCnt,
        const css::uno::Reference<css::beans::XPropertySet>& xSeriesProp );

    OOO_DLLPUBLIC_CHARTTOOLS void removeMeanValueLine(
        css::uno::Reference<css::chart2::XRegressionCurveContainer> const & xRegCnt );

    /** Returns the first regression curve found that is not of type
        mean-value line
     */
    OOO_DLLPUBLIC_CHARTTOOLS css::uno::Reference<css::chart2::XRegressionCurve>
        getFirstCurveNotMeanValueLine(
            const css::uno::Reference<css::chart2::XRegressionCurveContainer>& xCurveContainer );

    /** Returns the regression curve found at the index provided.
     */
    OOO_DLLPUBLIC_CHARTTOOLS css::uno::Reference<css::chart2::XRegressionCurve>
        getRegressionCurveAtIndex(
            const css::uno::Reference<css::chart2::XRegressionCurveContainer>& xCurveContainer,
            sal_Int32 aIndex);

    /** Returns the type of the first regression curve found that is not of type
        mean-value line
     */
    OOO_DLLPUBLIC_CHARTTOOLS SvxChartRegress getFirstRegressTypeNotMeanValueLine(
        const css::uno::Reference<css::chart2::XRegressionCurveContainer>& xCurveContainer );

    OOO_DLLPUBLIC_CHARTTOOLS SvxChartRegress getRegressionType(
        const css::uno::Reference<css::chart2::XRegressionCurve>& xCurve );

    /** @param xPropertySource is taken as source to copy all properties from if
               not null
        @param xEquationProperties is set at the new regression curve as
               equation properties if not null
    */
    OOO_DLLPUBLIC_CHARTTOOLS css::uno::Reference<css::chart2::XRegressionCurve>
        addRegressionCurve(
            SvxChartRegress eType,
            css::uno::Reference<css::chart2::XRegressionCurveContainer> const & xCurveContainer,
            const css::uno::Reference<css::beans::XPropertySet >& xPropertySource =
                    css::uno::Reference<css::beans::XPropertySet>(),
            const css::uno::Reference<css::beans::XPropertySet>& xEquationProperties =
                    css::uno::Reference<css::beans::XPropertySet>() );

    OOO_DLLPUBLIC_CHARTTOOLS bool removeAllExceptMeanValueLine(
        css::uno::Reference<css::chart2::XRegressionCurveContainer> const & xCurveContainer );

    OOO_DLLPUBLIC_CHARTTOOLS void removeEquations(
        css::uno::Reference<css::chart2::XRegressionCurveContainer> const & xCurveContainer );

    OOO_DLLPUBLIC_CHARTTOOLS css::uno::Reference<css::chart2::XRegressionCurve>
        changeRegressionCurveType(
            SvxChartRegress eType,
            css::uno::Reference<css::chart2::XRegressionCurveContainer> const & xRegressionCurveContainer,
            css::uno::Reference<css::chart2::XRegressionCurve> const & xRegressionCurve );

    /// returns a calculator object for regression curves (used by the view)
    OOO_DLLPUBLIC_CHARTTOOLS css::uno::Reference<css::chart2::XRegressionCurveCalculator>
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
    void initializeCurveCalculator(
        const css::uno::Reference<css::chart2::XRegressionCurveCalculator>& xOutCurveCalculator,
        const css::uno::Reference<css::chart2::data::XDataSource>& xSource,
        bool bUseXValuesIfAvailable );

    /** Same method as above, but uses the given XModel to determine the
        parameter bUseXValuesIfAvailable in the above function.  It is also
        necessary that the data::XDataSource is an XDataSeries, thus this parameter
        also changed.
     */
    OOO_DLLPUBLIC_CHARTTOOLS void initializeCurveCalculator(
        const css::uno::Reference<css::chart2::XRegressionCurveCalculator>& xOutCurveCalculator,
        const css::uno::Reference<css::chart2::XDataSeries>& xSeries,
        const css::uno::Reference<css::frame::XModel>& xModel );

    OOO_DLLPUBLIC_CHARTTOOLS OUString getUINameForRegressionCurve(
        const css::uno::Reference<css::chart2::XRegressionCurve>& xCurve );

    OOO_DLLPUBLIC_CHARTTOOLS OUString getRegressionCurveName(
        const css::uno::Reference<css::chart2::XRegressionCurve>& xCurve );

    OOO_DLLPUBLIC_CHARTTOOLS OUString getRegressionCurveGenericName(
        const css::uno::Reference<css::chart2::XRegressionCurve>& xCurve );

    OOO_DLLPUBLIC_CHARTTOOLS OUString getRegressionCurveSpecificName(
        const css::uno::Reference<css::chart2::XRegressionCurve>& xCurve );

    OOO_DLLPUBLIC_CHARTTOOLS std::vector<css::uno::Reference<css::chart2::XRegressionCurve> >
        getAllRegressionCurvesNotMeanValueLine(
                const css::uno::Reference<css::chart2::XDiagram>& xDiagram );

    OOO_DLLPUBLIC_CHARTTOOLS void resetEquationPosition(
        const css::uno::Reference<css::chart2::XRegressionCurve>& xCurve );

    /// @return the index of the given curve in the given container. -1 if not contained
    OOO_DLLPUBLIC_CHARTTOOLS sal_Int32 getRegressionCurveIndex(
        const css::uno::Reference<css::chart2::XRegressionCurveContainer>& xContainer,
        const css::uno::Reference<css::chart2::XRegressionCurve>& xCurve );

    OOO_DLLPUBLIC_CHARTTOOLS bool hasEquation(const css::uno::Reference<css::chart2::XRegressionCurve>& xCurve );

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
