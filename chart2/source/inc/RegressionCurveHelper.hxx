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
#ifndef CHART2_REGRESSIONCURVEHELPER_HXX
#define CHART2_REGRESSIONCURVEHELPER_HXX

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
    SAL_DLLPRIVATE static ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XRegressionCurve >
        createMeanValueLine(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XComponentContext > & xContext );

    /// returns a model regression curve
    SAL_DLLPRIVATE static ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XRegressionCurve >
        createRegressionCurveByServiceName(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XComponentContext > & xContext,
            ::rtl::OUString aServiceName );

    // ------------------------------------------------------------

    static bool hasMeanValueLine(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XRegressionCurveContainer > & xRegCnt );

    static bool isMeanValueLine(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XRegressionCurve > & xRegCurve );

    static ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XRegressionCurve >
        getMeanValueLine(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XRegressionCurveContainer > & xRegCnt );

    /** creates a mean-value line and adds it to the container.

         @param xSeriesProp
             If set, this property-set will be used to apply a line color
     */
    static void addMeanValueLine(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XRegressionCurveContainer > & xRegCnt,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > & xSeriesProp );

    static void removeMeanValueLine(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XRegressionCurveContainer > & xRegCnt );

    enum tRegressionType
    {
        REGRESSION_TYPE_NONE,
        REGRESSION_TYPE_LINEAR,
        REGRESSION_TYPE_LOG,
        REGRESSION_TYPE_EXP,
        REGRESSION_TYPE_POWER,
        REGRESSION_TYPE_MEAN_VALUE,
        REGRESSION_TYPE_UNKNOWN
    };

    /** Returns the first regression curve found that is not of type
        mean-value line
     */
    static ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XRegressionCurve >
        getFirstCurveNotMeanValueLine(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XRegressionCurveContainer > & xRegCnt );

    /** Returns the type of the first regression curve found that is not of type
        mean-value line
     */
    static tRegressionType getFirstRegressTypeNotMeanValueLine(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XRegressionCurveContainer > & xRegCnt );

    static tRegressionType getRegressionType(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XRegressionCurve > & xCurve );

    /** @param xPropertySource is taken as source to copy all properties from if
               not null
        @param xEquationProperties is set at the new regression curve as
               equation properties if not null
    */
    static void addRegressionCurve( tRegressionType eType,
            ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XRegressionCurveContainer > & xRegCnt,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XComponentContext > & xContext,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet >& xPropertySource =
                                ::com::sun::star::uno::Reference<
                                    ::com::sun::star::beans::XPropertySet >(),
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet >& xEquationProperties =
                                ::com::sun::star::uno::Reference<
                                    ::com::sun::star::beans::XPropertySet >()
        );

    static bool removeAllExceptMeanValueLine(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XRegressionCurveContainer > & xRegCnt );

    static void removeEquations(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XRegressionCurveContainer > & xRegCnt );

    /** adds the given regression curve if there was none before. If there are
        regression curves, the first one is replaced by the one given by the
        type. All remaining curves are remnoved.

        <p>This fuction ignores mean-value lines.</p>
     */
    static void replaceOrAddCurveAndReduceToOne(
        tRegressionType eType,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XRegressionCurveContainer > & xRegCnt,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );

    // ------------------------------------------------------------

    /// returns a calculator object for regression curves (used by the view)
    static ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XRegressionCurveCalculator >
        createRegressionCurveCalculatorByServiceName(
            ::rtl::OUString aServiceName );

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
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XRegressionCurveCalculator > & xOutCurveCalculator,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XDataSource > & xSource,
        bool bUseXValuesIfAvailable = true );

    /** Same method as above, but uses the given XModel to determine the
        parameter bUseXValuesIfAvailable in the above function.  It is also
        necessary that the data::XDataSource is an XDataSeries, thus this parameter
        also changed.
     */
    static void initializeCurveCalculator(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XRegressionCurveCalculator > & xOutCurveCalculator,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSeries > & xSeries,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & xModel );

    static ::rtl::OUString getUINameForRegressionCurve( const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XRegressionCurve >& xCurve );

    static ::std::vector< ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XRegressionCurve > > getAllRegressionCurvesNotMeanValueLine(
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XDiagram > & xDiagram );

    static void resetEquationPosition( const ::com::sun::star::uno::Reference<
                                       ::com::sun::star::chart2::XRegressionCurve > & xCurve );

    /// @return the index of the given curve in the given container. -1 if not contained
    static sal_Int32 getRegressionCurveIndex(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XRegressionCurveContainer > & xContainer,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XRegressionCurve > & xCurve );

    static bool hasEquation(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XRegressionCurve > & xCurve );

private:
    // not implemented
    RegressionCurveHelper();
};

} //  namespace chart

// CHART2_REGRESSIONCURVEHELPER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
