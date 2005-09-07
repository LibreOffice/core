/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RegressionCurveHelper.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:45:30 $
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

#ifndef _COM_SUN_STAR_CHART2_XREGRESSIONCURVE_HPP_
#include <com/sun/star/chart2/XRegressionCurve.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XREGRESSIONCURVECALCULATOR_HPP_
#include <com/sun/star/chart2/XRegressionCurveCalculator.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XREGRESSIONCURVECONTAINER_HPP_
#include <com/sun/star/chart2/XRegressionCurveContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDATASOURCE_HPP_
#include <com/sun/star/chart2/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDATASERIES_HPP_
#include <com/sun/star/chart2/XDataSeries.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

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

    /** creates a mean-value line and adds it to the container.

         @param xSeriesProp
             If set, this property-set will be used to apply a line color
     */
    static void RegressionCurveHelper::addMeanValueLine(
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
        REGRESSION_TYPE_POWER
    };

    /** Returns the type of the first regression curve found that is not of type
        mean-value line
     */
    static tRegressionType getRegressType(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XRegressionCurveContainer > & xRegCnt );

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

        A sequence having the role "x-values" will be used as x-values for the
        calculation if found.  Otherwise a sequence (1, 2, 3, ...) of category
        indexes will be used for the recalculateRegression() method of the
        regression curve.

        The first sequence having the role "y-values" will be used as y-values
        for the recalculateRegression() method of the regression curve.

        @param bUseXValuesIfAvailable
            If false, the sequence (1, 2, 3, ...) will always be used, even if
            there is a data-sequence with role "x-values"
     */
    static void initializeCurveCalculator(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XRegressionCurveCalculator > & xOutCurveCalculator,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSource > & xSource,
        bool bUseXValuesIfAvailable = true );

    /** Same method as above, but uses the given XModel to determine the
        parameter bUseXValuesIfAvailable in the above function.  It is also
        necessary that the XDataSource is an XDataSeries, thus this parameter
        also changed.
     */
    static void initializeCurveCalculator(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XRegressionCurveCalculator > & xOutCurveCalculator,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSeries > & xSeries,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::frame::XModel > & xModel );

private:
    // not implemented
    RegressionCurveHelper();
};

} //  namespace chart

// CHART2_REGRESSIONCURVEHELPER_HXX
#endif
