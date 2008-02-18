/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RegressionCurveHelper.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-18 15:59:39 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

#include <vector>

namespace chart
{

class RegressionCurveHelper
{
public:
    /// returns a model mean-value line
    static ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XRegressionCurve >
        createMeanValueLine(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XComponentContext > & xContext );

    /// returns a model regression curve
    static ::com::sun::star::uno::Reference<
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

    /// returns a calculator object for mean-value lines (used by the view)
    static ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XRegressionCurveCalculator >
        createMeanValueLineCalculator();

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
    static void initializeCurveCalculator(
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

private:
    // not implemented
    RegressionCurveHelper();
};

} //  namespace chart

// CHART2_REGRESSIONCURVEHELPER_HXX
#endif
