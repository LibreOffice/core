/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DataSeriesHelper.hxx,v $
 * $Revision: 1.7.24.1 $
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
#ifndef CHART2_DATASERIESHELPER_HXX
#define CHART2_DATASERIESHELPER_HXX

#include <com/sun/star/chart2/data/XDataSequence.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/XCoordinateSystem.hpp>
#include <com/sun/star/chart2/XChartType.hpp>

#include "StackMode.hxx"

#include <vector>
#include <functional>

namespace chart
{

namespace DataSeriesHelper
{

::rtl::OUString GetRole(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XLabeledDataSequence >& xLabeledDataSequence );

/** Retrieves the data sequence in the given data source that matches the
    given role.  If more than one sequences match the role, the first match
    is returned.  If no sequence matches, an empty reference is returned.

    @param aSource
        The data source containing all data sequences to be searched through.

    @param aRole
        The role that is to be filtered out.
*/
::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XLabeledDataSequence >
    getDataSequenceByRole( const ::com::sun::star::uno::Reference<
                               ::com::sun::star::chart2::data::XDataSource > & xSource,
                           ::rtl::OUString aRole,
                           bool bMatchPrefix = false );

/** Retrieves all data sequences in the given data source that match the given
    role.

    @param aSource
        The data source containing all data sequences to be searched through.

    @param aRole
        The role that is to be filtered out.
*/
::std::vector<
  ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XLabeledDataSequence > >
    getAllDataSequencesByRole( const ::com::sun::star::uno::Sequence<
                                   ::com::sun::star::uno::Reference<
                                       ::com::sun::star::chart2::data::XLabeledDataSequence > > & aDataSequences,
                               ::rtl::OUString aRole,
                               bool bMatchPrefix = false );

/** Retrieves all data sequences found in the given data series and puts them
    into a data source.  The order of sequences will match the order of the data
    series.
 */
::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::data::XDataSource >
    getDataSource( const ::com::sun::star::uno::Sequence<
                          ::com::sun::star::uno::Reference<
                              ::com::sun::star::chart2::XDataSeries > > & aSeries );


/** Get the label of a series (e.g. for the legend)

    @param rLabelSequenceRole
        The data sequence contained in xSeries that has this role will be used
        to take its label.
 */
::rtl::OUString getDataSeriesLabel(
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XDataSeries > & xSeries,
    const ::rtl::OUString & rLabelSequenceRole );

/** Get the label of a labeled sequence including neccessary automatic generation
 */
::rtl::OUString getLabelForLabeledDataSequence(
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::data::XLabeledDataSequence > & xLabeledSeq );

void setStackModeAtSeries(
    const ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries > > & aSeries,
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XCoordinateSystem > & xCorrespondingCoordinateSystem,
    StackMode eStackMode );

sal_Int32 getAttachedAxisIndex(
    const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries > & xSeries );

/// @param nAxisIndex, if -1 it is determined by the given data series via getAttachedAxisIndex
sal_Int32 getNumberFormatKeyFromAxis(
    const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries > & xSeries,
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XCoordinateSystem > & xCorrespondingCoordinateSystem,
    sal_Int32 nDimensionIndex,
    sal_Int32 nAxisIndex = -1 );

::com::sun::star::uno::Reference< ::com::sun::star::chart2::XCoordinateSystem >
    getCoordinateSystemOfSeries(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries > & xSeries,
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDiagram > & xDiagram );

::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartType >
    getChartTypeOfSeries(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries > & xSeries,
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDiagram > & xDiagram );

void deleteSeries(
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XDataSeries > & xSeries,
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XChartType > & xChartType );

void switchSymbolsOnOrOff(
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > & xSeriesProperties,
    bool bSymbolsOn, sal_Int32 nSeriesIndex );

void switchLinesOnOrOff(
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > & xSeriesProperties,
    bool bLinesOn );

void makeLinesThickOrThin( const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > & xSeriesProperties, bool bThick );

void setPropertyAlsoToAllAttributedDataPoints(
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XDataSeries >& xSeries,
        const ::rtl::OUString& rPropertyName,
        const ::com::sun::star::uno::Any& rPropertyValue );

bool hasAttributedDataPointDifferentValue(
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XDataSeries >& xSeries,
    const ::rtl::OUString& rPropertyName,
    const ::com::sun::star::uno::Any& rPropertyValue );

} //  namespace DataSeriesHelper
} //  namespace chart

// CHART2_DATASERIESHELPER_HXX
#endif
