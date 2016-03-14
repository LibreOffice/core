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
#ifndef INCLUDED_CHART2_SOURCE_INC_DATASERIESHELPER_HXX
#define INCLUDED_CHART2_SOURCE_INC_DATASERIESHELPER_HXX

#include <com/sun/star/chart2/data/XDataSequence.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/XDataSeries.hpp>
#include <com/sun/star/chart2/XDiagram.hpp>
#include <com/sun/star/chart2/XCoordinateSystem.hpp>
#include <com/sun/star/chart2/XChartType.hpp>

#include "StackMode.hxx"
#include "charttoolsdllapi.hxx"

#include <functional>
#include <vector>

namespace chart
{

namespace DataSeriesHelper
{

OOO_DLLPUBLIC_CHARTTOOLS OUString
    getRole( const css::uno::Reference<css::chart2::data::XLabeledDataSequence>& xLabeledDataSequence );

/** Retrieves the data sequence in the given data source that matches the
    given role.  If more than one sequences match the role, the first match
    is returned.  If no sequence matches, an empty reference is returned.

    @param aSource
        The data source containing all data sequences to be searched through.

    @param aRole
        The role that is to be filtered out.
*/
OOO_DLLPUBLIC_CHARTTOOLS css::uno::Reference< css::chart2::data::XLabeledDataSequence >
    getDataSequenceByRole( const css::uno::Reference< css::chart2::data::XDataSource > & xSource,
                           const OUString& aRole,
                           bool bMatchPrefix = false );

/** Retrieves all data sequences in the given data source that match the given
    role.

    @param aSource
        The data source containing all data sequences to be searched through.

    @param aRole
        The role that is to be filtered out.
*/
OOO_DLLPUBLIC_CHARTTOOLS ::std::vector<
  css::uno::Reference< css::chart2::data::XLabeledDataSequence > >
    getAllDataSequencesByRole( const css::uno::Sequence< css::uno::Reference< css::chart2::data::XLabeledDataSequence > > & aDataSequences,
                               const OUString& aRole,
                               bool bMatchPrefix = false );

OOO_DLLPUBLIC_CHARTTOOLS
std::vector<css::uno::Reference<css::chart2::data::XLabeledDataSequence> >
getAllDataSequences(
    const css::uno::Sequence<css::uno::Reference<css::chart2::XDataSeries> >& aSeries );

/** Retrieves all data sequences found in the given data series and puts them
    into a data source.  The order of sequences will match the order of the data
    series.
 */
OOO_DLLPUBLIC_CHARTTOOLS css::uno::Reference< css::chart2::data::XDataSource >
    getDataSource( const css::uno::Sequence< css::uno::Reference< css::chart2::XDataSeries > > & aSeries );

/** Get the label of a series (e.g. for the legend)

    @param rLabelSequenceRole
        The data sequence contained in xSeries that has this role will be used
        to take its label.
 */
OOO_DLLPUBLIC_CHARTTOOLS OUString getDataSeriesLabel(
    const css::uno::Reference< css::chart2::XDataSeries > & xSeries,
    const OUString & rLabelSequenceRole );

/** Get the label of a labeled sequence including necessary automatic generation
 */
OOO_DLLPUBLIC_CHARTTOOLS OUString getLabelForLabeledDataSequence(
    const css::uno::Reference< css::chart2::data::XLabeledDataSequence > & xLabeledSeq );

OOO_DLLPUBLIC_CHARTTOOLS void setStackModeAtSeries(
    const css::uno::Sequence< css::uno::Reference< css::chart2::XDataSeries > > & aSeries,
    const css::uno::Reference< css::chart2::XCoordinateSystem > & xCorrespondingCoordinateSystem,
    StackMode eStackMode );

OOO_DLLPUBLIC_CHARTTOOLS sal_Int32 getAttachedAxisIndex(
    const css::uno::Reference< css::chart2::XDataSeries > & xSeries );

/// @param nAxisIndex, if -1 it is determined by the given data series via getAttachedAxisIndex
OOO_DLLPUBLIC_CHARTTOOLS sal_Int32 getNumberFormatKeyFromAxis(
    const css::uno::Reference< css::chart2::XDataSeries > & xSeries,
    const css::uno::Reference< css::chart2::XCoordinateSystem > & xCorrespondingCoordinateSystem,
    sal_Int32 nDimensionIndex,
    sal_Int32 nAxisIndex = -1 );

OOO_DLLPUBLIC_CHARTTOOLS
css::uno::Reference< css::chart2::XCoordinateSystem >
    getCoordinateSystemOfSeries(
        const css::uno::Reference< css::chart2::XDataSeries > & xSeries,
        const css::uno::Reference< css::chart2::XDiagram > & xDiagram );

OOO_DLLPUBLIC_CHARTTOOLS
css::uno::Reference< css::chart2::XChartType >
    getChartTypeOfSeries(
        const css::uno::Reference< css::chart2::XDataSeries > & xSeries,
        const css::uno::Reference< css::chart2::XDiagram > & xDiagram );

OOO_DLLPUBLIC_CHARTTOOLS void deleteSeries(
    const css::uno::Reference< css::chart2::XDataSeries > & xSeries,
    const css::uno::Reference< css::chart2::XChartType > & xChartType );

OOO_DLLPUBLIC_CHARTTOOLS void switchSymbolsOnOrOff(
    const css::uno::Reference< css::beans::XPropertySet > & xSeriesProperties,
    bool bSymbolsOn, sal_Int32 nSeriesIndex );

OOO_DLLPUBLIC_CHARTTOOLS void switchLinesOnOrOff(
    const css::uno::Reference< css::beans::XPropertySet > & xSeriesProperties,
    bool bLinesOn );

OOO_DLLPUBLIC_CHARTTOOLS
void makeLinesThickOrThin( const css::uno::Reference< css::beans::XPropertySet > & xSeriesProperties, bool bThick );

OOO_DLLPUBLIC_CHARTTOOLS void setPropertyAlsoToAllAttributedDataPoints(
        const css::uno::Reference< css::chart2::XDataSeries >& xSeries,
        const OUString& rPropertyName,
        const css::uno::Any& rPropertyValue );

OOO_DLLPUBLIC_CHARTTOOLS bool hasAttributedDataPointDifferentValue(
    const css::uno::Reference< css::chart2::XDataSeries >& xSeries,
    const OUString& rPropertyName,
    const css::uno::Any& rPropertyValue );

OOO_DLLPUBLIC_CHARTTOOLS bool areAllSeriesAttachedToSameAxis(
        const css::uno::Reference< css::chart2::XChartType >& xChartType,
        sal_Int32 & rOutAxisIndex );

OOO_DLLPUBLIC_CHARTTOOLS bool hasUnhiddenData( const css::uno::Reference<
        css::chart2::XDataSeries >& xSeries );

OOO_DLLPUBLIC_CHARTTOOLS
sal_Int32 translateIndexFromHiddenToFullSequence( sal_Int32 nClippedIndex, const css::uno::Reference<
        css::chart2::data::XDataSequence >& xDataSequence, bool bTranslate );

OOO_DLLPUBLIC_CHARTTOOLS bool hasDataLabelsAtSeries( const css::uno::Reference< css::chart2::XDataSeries >& xSeries );

OOO_DLLPUBLIC_CHARTTOOLS bool hasDataLabelsAtPoints( const css::uno::Reference< css::chart2::XDataSeries >& xSeries );

OOO_DLLPUBLIC_CHARTTOOLS bool hasDataLabelAtPoint( const css::uno::Reference< css::chart2::XDataSeries >& xSeries, sal_Int32 nPointIndex );

OOO_DLLPUBLIC_CHARTTOOLS void insertDataLabelsToSeriesAndAllPoints( const css::uno::Reference< css::chart2::XDataSeries >& xSeries );

OOO_DLLPUBLIC_CHARTTOOLS void insertDataLabelToPoint( const css::uno::Reference< css::beans::XPropertySet >& xPointPropertySet );

OOO_DLLPUBLIC_CHARTTOOLS void deleteDataLabelsFromSeriesAndAllPoints( const css::uno::Reference< css::chart2::XDataSeries >& xSeries );

OOO_DLLPUBLIC_CHARTTOOLS void deleteDataLabelsFromPoint( const css::uno::Reference< css::beans::XPropertySet >& xPointPropertySet );

} //  namespace DataSeriesHelper
} //  namespace chart

// INCLUDED_CHART2_SOURCE_INC_DATASERIESHELPER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
