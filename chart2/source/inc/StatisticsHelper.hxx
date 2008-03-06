/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StatisticsHelper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 17:09:22 $
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
#ifndef CHART2_STATISTICSHELPER_HXX
#define CHART2_STATISTICSHELPER_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>

namespace chart
{

class StatisticsHelper
{
public:
    /** Calculates 1/n * sum (x_i - x_mean)^2.

        @see http://mathworld.wolfram.com/Variance.html

        @param bUnbiasedEstimator
            If true, 1/(n-1) * sum (x_i - x_mean)^2 is returned.
     */
    static double getVariance( const ::com::sun::star::uno::Sequence< double > & rData,
        bool bUnbiasedEstimator = false );

    // square root of the variance
    static double getStandardDeviation( const ::com::sun::star::uno::Sequence< double > & rData );

    // also called "Standard deviation of the mean (SDOM)"
    static double getStandardError( const ::com::sun::star::uno::Sequence< double > & rData );

    static ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XLabeledDataSequence >
        getErrorLabeledDataSequenceFromDataSource(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::data::XDataSource > & xDataSource,
            bool bPositiveValue,
            bool bYError = true );

    static ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XDataSequence >
        getErrorDataSequenceFromDataSource(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::data::XDataSource > & xDataSource,
            bool bPositiveValue,
            bool bYError = true );

    static double getErrorFromDataSource(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XDataSource > & xDataSource,
        sal_Int32 nIndex,
        bool bPositiveValue,
        bool bYError = true );

    static void setErrorDataSequence(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XDataSource > & xDataSource,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XDataProvider > & xDataProvider,
        const ::rtl::OUString & rNewRange,
        bool bPositiveValue,
        bool bYError = true,
        ::rtl::OUString * pXMLRange = 0 );

    /// @return the newly created or existing error bar object
    static ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet >
        addErrorBars(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDataSeries > & xDataSeries,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XComponentContext > & xContext,
            sal_Int32 nStyle,
            bool bYError = true );

    static ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet >
        getErrorBars(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XDataSeries > & xDataSeries,
            bool bYError = true );

    static bool hasErrorBars(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSeries > & xDataSeries,
        bool bYError = true );

    static void removeErrorBars(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSeries > & xDataSeries,
        bool bYError = true );

    static bool usesErrorBarRanges(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDataSeries > & xDataSeries,
        bool bYError = true );

private:
    // not implemented
    StatisticsHelper();
};

} //  namespace chart

// CHART2_STATISTICSHELPER_HXX
#endif
