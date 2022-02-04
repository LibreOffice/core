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

#include "StockDataInterpreter.hxx"
#include "StockChartTypeTemplate.hxx"
#include <DataSeries.hxx>
#include <LabeledDataSequence.hxx>
#include <com/sun/star/chart2/data/XDataSink.hpp>
#include <tools/diagnose_ex.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using namespace ::std;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart
{

// explicit
StockDataInterpreter::StockDataInterpreter(
    StockChartTypeTemplate::StockVariant eVariant ) :
        m_eStockVariant( eVariant )
{}

StockDataInterpreter::~StockDataInterpreter()
{}

// ____ XDataInterpreter ____
InterpretedData StockDataInterpreter::interpretDataSource(
    const Reference< data::XDataSource >& xSource,
    const Sequence< beans::PropertyValue >& rArguments,
    const std::vector< rtl::Reference< ::chart::DataSeries > >& rSeriesToReUse )
{
    if( ! xSource.is())
        return InterpretedData();

    rtl::Reference< LabeledDataSequence > xCategories;
    std::vector< rtl::Reference< LabeledDataSequence > > aData = DataInterpreter::getDataSequences(xSource);
    const sal_Int32 nDataCount( aData.size());

    // sub-type properties
    const StockChartTypeTemplate::StockVariant eVar( GetStockVariant());
    const bool bHasOpenValues (( eVar == StockChartTypeTemplate::StockVariant::Open ) ||
                               ( eVar == StockChartTypeTemplate::StockVariant::VolumeOpen ));
    const bool bHasVolume (( eVar == StockChartTypeTemplate::StockVariant::Volume ) ||
                           ( eVar == StockChartTypeTemplate::StockVariant::VolumeOpen ));
    const bool bHasCategories( HasCategories( rArguments, aData ));

    // necessary roles for "full series"
    // low/high/close
    sal_Int32 nNumberOfNecessarySequences( 3 );
    if( bHasOpenValues )
        ++nNumberOfNecessarySequences;
    if( bHasVolume )
        ++nNumberOfNecessarySequences;

    // calculate number of full series (nNumOfFullSeries) and the number of remaining
    // sequences used for additional "incomplete series" (nRemaining)
    sal_Int32 nNumOfFullSeries( 0 );
    sal_Int32 nRemaining( 0 );
    {
        sal_Int32 nAvailableSequences( nDataCount );
        if( bHasCategories )
            --nAvailableSequences;
        nNumOfFullSeries = nAvailableSequences / nNumberOfNecessarySequences;
        nRemaining = nAvailableSequences % nNumberOfNecessarySequences;
    }
    sal_Int32 nCandleStickSeries = nNumOfFullSeries;
    sal_Int32 nVolumeSeries = nNumOfFullSeries;

    sal_Int32 nNumberOfGroups( bHasVolume ? 2 : 1 );
    // sequences of data::XLabeledDataSequence per series per group
    Sequence< Sequence< Sequence< Reference< data::XLabeledDataSequence > > > > aSequences( nNumberOfGroups );
    auto pSequences = aSequences.getArray();
    const sal_Int32 nBarGroupIndex( 0 );
    const sal_Int32 nCandleStickGroupIndex( nNumberOfGroups - 1 );

    // allocate space for labeled sequences
    if( nRemaining > 0  )
        ++nCandleStickSeries;
    pSequences[nCandleStickGroupIndex].realloc( nCandleStickSeries );
    auto pCandleStickGroup = pSequences[nCandleStickGroupIndex].getArray();
    if( bHasVolume )
    {
        // if there are remaining sequences, the first one is taken for
        // additional close values, the second one is taken as volume, if volume
        // is used
        if( nRemaining > 1 )
            ++nVolumeSeries;
        pSequences[nBarGroupIndex].realloc( nVolumeSeries );
    }
    auto pBarGroup = pSequences[nBarGroupIndex].getArray();

    // create data
    sal_Int32 nSourceIndex = 0;   // index into aData sequence

    // 1. categories
    if( bHasCategories )
    {
        xCategories = aData[nSourceIndex];
        ++nSourceIndex;
    }

    // 2. create "full" series
    for( sal_Int32 nLabeledSeqIdx=0; nLabeledSeqIdx<nNumOfFullSeries; ++nLabeledSeqIdx )
    {
        // bar
        if( bHasVolume )
        {
            pBarGroup[nLabeledSeqIdx].realloc( 1 );
            pBarGroup[nLabeledSeqIdx].getArray()[0].set( aData[nSourceIndex] );
            if( aData[nSourceIndex].is())
                SetRole( aData[nSourceIndex]->getValues(), "values-y");
            ++nSourceIndex;
        }

        sal_Int32 nSeqIdx = 0;
        if( bHasOpenValues )
        {
            pCandleStickGroup[nLabeledSeqIdx].realloc( 4 );
            pCandleStickGroup[nLabeledSeqIdx].getArray()[nSeqIdx].set( aData[nSourceIndex] );
            if( aData[nSourceIndex].is())
                SetRole( aData[nSourceIndex]->getValues(), "values-first");
            ++nSourceIndex;
            ++nSeqIdx;
        }
        else
            pCandleStickGroup[nLabeledSeqIdx].realloc( 3 );
        auto pLabeledSeq = pCandleStickGroup[nLabeledSeqIdx].getArray();

        pLabeledSeq[nSeqIdx].set( aData[nSourceIndex] );
        if( aData[nSourceIndex].is())
            SetRole( aData[nSourceIndex]->getValues(), "values-min");
        ++nSourceIndex;
        ++nSeqIdx;

        pLabeledSeq[nSeqIdx].set( aData[nSourceIndex] );
        if( aData[nSourceIndex].is())
            SetRole( aData[nSourceIndex]->getValues(), "values-max");
        ++nSourceIndex;
        ++nSeqIdx;

        pLabeledSeq[nSeqIdx].set( aData[nSourceIndex] );
        if( aData[nSourceIndex].is())
            SetRole( aData[nSourceIndex]->getValues(), "values-last");
        ++nSourceIndex;
        ++nSeqIdx;
    }

    // 3. create series with remaining sequences
    if( bHasVolume && nRemaining > 1 )
    {
        OSL_ASSERT( nVolumeSeries > nNumOfFullSeries );
        pBarGroup[nVolumeSeries - 1].realloc( 1 );
        OSL_ASSERT( nDataCount > nSourceIndex );
        if( aData[nSourceIndex].is())
            SetRole( aData[nSourceIndex]->getValues(), "values-y");
        pBarGroup[nVolumeSeries - 1].getArray()[0].set( aData[nSourceIndex] );
        ++nSourceIndex;
        --nRemaining;
        OSL_ENSURE( nRemaining, "additional bar should only be used if there is at least one more sequence for a candle stick" );
    }

    // candle-stick
    if( nRemaining > 0 )
    {
        OSL_ASSERT( nCandleStickSeries > nNumOfFullSeries );
        const sal_Int32 nSeriesIndex = nCandleStickSeries - 1;
        pCandleStickGroup[nSeriesIndex].realloc( nRemaining );
        auto pLabeledSeq = pCandleStickGroup[nSeriesIndex].getArray();
        OSL_ASSERT( nDataCount > nSourceIndex );

        // 1. low
        sal_Int32 nSeqIdx( 0 );
        pLabeledSeq[nSeqIdx].set( aData[nSourceIndex] );
        if( aData[nSourceIndex].is())
            SetRole( aData[nSourceIndex]->getValues(), "values-min");
        ++nSourceIndex;
        ++nSeqIdx;

        // 2. high
        if( nSeqIdx < nRemaining )
        {
            pLabeledSeq[nSeqIdx].set( aData[nSourceIndex] );
            if( aData[nSourceIndex].is())
                SetRole( aData[nSourceIndex]->getValues(), "values-max");
            ++nSourceIndex;
            ++nSeqIdx;
        }

        // 3. close
        OSL_ENSURE( bHasOpenValues || nSeqIdx >= nRemaining, "could have created full series" );
        if( nSeqIdx < nRemaining )
        {
            pLabeledSeq[nSeqIdx].set( aData[nSourceIndex] );
            if( aData[nSourceIndex].is())
                SetRole( aData[nSourceIndex]->getValues(), "values-last");
            ++nSourceIndex;
            ++nSeqIdx;
        }

        // 4. open
        OSL_ENSURE( nSeqIdx >= nRemaining, "could have created full series" );
    }

    // create DataSeries
    Sequence< Sequence< Reference< XDataSeries > > > aResultSeries( nNumberOfGroups );
    auto pResultSeries = aResultSeries.getArray();
    sal_Int32 nGroupIndex, nReUsedSeriesIdx = 0;
    for( nGroupIndex=0; nGroupIndex<nNumberOfGroups; ++nGroupIndex )
    {
        const sal_Int32 nNumSeriesData = aSequences[nGroupIndex].getLength();
        pResultSeries[nGroupIndex].realloc( nNumSeriesData );
        auto pResultSerie = pResultSeries[nGroupIndex].getArray();
        for( sal_Int32 nSeriesIdx = 0; nSeriesIdx < nNumSeriesData; ++nSeriesIdx, ++nReUsedSeriesIdx )
        {
            try
            {
                rtl::Reference< DataSeries > xSeries;
                if( nReUsedSeriesIdx < static_cast<sal_Int32>(rSeriesToReUse.size()))
                    xSeries = rSeriesToReUse[nReUsedSeriesIdx];
                else
                    xSeries = new DataSeries;
                assert( xSeries.is() );
                xSeries->setData( aSequences[nGroupIndex][nSeriesIdx] );
                pResultSerie[nSeriesIdx].set( xSeries );
            }
            catch( const uno::Exception & )
            {
                DBG_UNHANDLED_EXCEPTION("chart2");
            }
        }
    }

    return { aResultSeries, xCategories };
}

// criterion: there must be two groups for stock-charts with volume and all
// series must have the correct number of data::XLabeledDataSequences

// todo: skip first criterion? (to allow easy switch from stock-chart without
// volume to one with volume)
bool StockDataInterpreter::isDataCompatible(
    const InterpretedData& aInterpretedData )
{
    // high/low/close
    sal_Int32 nNumberOfNecessarySequences = 3;
    // open
    StockChartTypeTemplate::StockVariant eVar( GetStockVariant());
    if( ( eVar == StockChartTypeTemplate::StockVariant::Open ) ||
        ( eVar == StockChartTypeTemplate::StockVariant::VolumeOpen ))
        ++nNumberOfNecessarySequences;
    // volume
    bool bHasVolume = (( eVar == StockChartTypeTemplate::StockVariant::Volume ) ||
                       ( eVar == StockChartTypeTemplate::StockVariant::VolumeOpen ));

    // 1. correct number of sub-types
    if( aInterpretedData.Series.getLength() < (bHasVolume ? 2 : 1 ))
        return false;

    // 2. a. volume -- use default check
    if( bHasVolume )
    {
        if( ! DataInterpreter::isDataCompatible(
                { Sequence< Sequence< Reference< XDataSeries > > >(
                                     aInterpretedData.Series.getConstArray(), 1 ),
                  aInterpretedData.Categories }))
            return false;
    }

    // 2. b. candlestick
    {
        OSL_ASSERT( aInterpretedData.Series.getLength() > (bHasVolume ? 1 : 0));
        const Sequence< Reference< XDataSeries > > aSeries( aInterpretedData.Series[(bHasVolume ? 1 : 0)] );
        if(!aSeries.hasElements())
            return false;
        for( Reference< XDataSeries > const & dataSeries : aSeries )
        {
            try
            {
                Reference< data::XDataSource > xSrc( dataSeries, uno::UNO_QUERY_THROW );
                Sequence< Reference< data::XLabeledDataSequence > > aSeq( xSrc->getDataSequences());
                if( aSeq.getLength() != nNumberOfNecessarySequences )
                    return false;
            }
            catch( const uno::Exception & )
            {
                DBG_UNHANDLED_EXCEPTION("chart2");
            }
        }
    }

    // 2. c. additional series
    // ignore

    return true;
}

InterpretedData StockDataInterpreter::reinterpretDataSeries(
    const InterpretedData& aInterpretedData )
{
    // prerequisite: StockDataInterpreter::isDataCompatible() returned true
    return aInterpretedData;
}

uno::Any StockDataInterpreter::getChartTypeSpecificData(
    const OUString& sKey )
{
    if( sKey == "stock variant" )
    {
        StockChartTypeTemplate::StockVariant eStockVariant( GetStockVariant());
        std::map< StockChartTypeTemplate::StockVariant, sal_Int32 > aTranslation {
            { StockChartTypeTemplate::StockVariant::NONE, 0 },
            { StockChartTypeTemplate::StockVariant::Open, 1 },
            { StockChartTypeTemplate::StockVariant::Volume, 2 },
            { StockChartTypeTemplate::StockVariant::VolumeOpen, 3 }
        };
        return uno::Any( aTranslation[eStockVariant] );
    }
    return uno::Any();
}

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
