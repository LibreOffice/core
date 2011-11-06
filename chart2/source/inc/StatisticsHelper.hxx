/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef CHART2_STATISTICSHELPER_HXX
#define CHART2_STATISTICSHELPER_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include "charttoolsdllapi.hxx"

namespace chart
{

class OOO_DLLPUBLIC_CHARTTOOLS StatisticsHelper
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
