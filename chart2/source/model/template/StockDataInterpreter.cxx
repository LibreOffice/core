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

#include <sal/config.h>

#include <cstddef>

#include "StockDataInterpreter.hxx"
#include "StockChartTypeTemplate.hxx"
#include <DataSeries.hxx>
#include <comphelper/diagnose_ex.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

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

    uno::Reference< chart2::data::XLabeledDataSequence > xCategories;
    std::vector< uno::Reference< chart2::data::XLabeledDataSequence > > aData = DataInterpreter::getDataSequences(xSource);
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
    std::vector< std::vector< std::vector< uno::Reference< chart2::data::XLabeledDataSequence > > > > aSequences( nNumberOfGroups );
    const sal_Int32 nBarGroupIndex( 0 );
    const sal_Int32 nCandleStickGroupIndex( nNumberOfGroups - 1 );

    // allocate space for labeled sequences
    if( nRemaining > 0  )
        ++nCandleStickSeries;
    aSequences[nCandleStickGroupIndex].resize( nCandleStickSeries );
    auto & pCandleStickGroup = aSequences[nCandleStickGroupIndex];
    if( bHasVolume )
    {
        // if there are remaining sequences, the first one is taken for
        // additional close values, the second one is taken as volume, if volume
        // is used
        if( nRemaining > 1 )
            ++nVolumeSeries;
        aSequences[nBarGroupIndex].resize( nVolumeSeries );
    }
    auto & pBarGroup = aSequences[nBarGroupIndex];

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
            pBarGroup[nLabeledSeqIdx].resize( 1 );
            pBarGroup[nLabeledSeqIdx][0] = aData[nSourceIndex];
            if( aData[nSourceIndex].is())
                SetRole( aData[nSourceIndex]->getValues(), u"values-y"_ustr);
            ++nSourceIndex;
        }

        sal_Int32 nSeqIdx = 0;
        if( bHasOpenValues )
        {
            pCandleStickGroup[nLabeledSeqIdx].resize( 4 );
            pCandleStickGroup[nLabeledSeqIdx][nSeqIdx] = aData[nSourceIndex];
            if( aData[nSourceIndex].is())
                SetRole( aData[nSourceIndex]->getValues(), u"values-first"_ustr);
            ++nSourceIndex;
            ++nSeqIdx;
        }
        else
            pCandleStickGroup[nLabeledSeqIdx].resize( 3 );
        auto & pLabeledSeq = pCandleStickGroup[nLabeledSeqIdx];

        pLabeledSeq[nSeqIdx] = aData[nSourceIndex];
        if( aData[nSourceIndex].is())
            SetRole( aData[nSourceIndex]->getValues(), u"values-min"_ustr);
        ++nSourceIndex;
        ++nSeqIdx;

        pLabeledSeq[nSeqIdx] = aData[nSourceIndex];
        if( aData[nSourceIndex].is())
            SetRole( aData[nSourceIndex]->getValues(), u"values-max"_ustr);
        ++nSourceIndex;
        ++nSeqIdx;

        pLabeledSeq[nSeqIdx] = aData[nSourceIndex];
        if( aData[nSourceIndex].is())
            SetRole( aData[nSourceIndex]->getValues(), u"values-last"_ustr);
        ++nSourceIndex;
        ++nSeqIdx;
    }

    // 3. create series with remaining sequences
    if( bHasVolume && nRemaining > 1 )
    {
        OSL_ASSERT( nVolumeSeries > nNumOfFullSeries );
        pBarGroup[nVolumeSeries - 1].resize( 1 );
        OSL_ASSERT( nDataCount > nSourceIndex );
        if( aData[nSourceIndex].is())
            SetRole( aData[nSourceIndex]->getValues(), u"values-y"_ustr);
        pBarGroup[nVolumeSeries - 1][0] = aData[nSourceIndex];
        ++nSourceIndex;
        --nRemaining;
        OSL_ENSURE( nRemaining, "additional bar should only be used if there is at least one more sequence for a candle stick" );
    }

    // candle-stick
    if( nRemaining > 0 )
    {
        OSL_ASSERT( nCandleStickSeries > nNumOfFullSeries );
        const sal_Int32 nSeriesIndex = nCandleStickSeries - 1;
        pCandleStickGroup[nSeriesIndex].resize( nRemaining );
        auto & pLabeledSeq = pCandleStickGroup[nSeriesIndex];
        OSL_ASSERT( nDataCount > nSourceIndex );

        // 1. low
        sal_Int32 nSeqIdx( 0 );
        pLabeledSeq[nSeqIdx] = aData[nSourceIndex];
        if( aData[nSourceIndex].is())
            SetRole( aData[nSourceIndex]->getValues(), u"values-min"_ustr);
        ++nSourceIndex;
        ++nSeqIdx;

        // 2. high
        if( nSeqIdx < nRemaining )
        {
            pLabeledSeq[nSeqIdx] = aData[nSourceIndex];
            if( aData[nSourceIndex].is())
                SetRole( aData[nSourceIndex]->getValues(), u"values-max"_ustr);
            ++nSourceIndex;
            ++nSeqIdx;
        }

        // 3. close
        OSL_ENSURE( bHasOpenValues || nSeqIdx >= nRemaining, "could have created full series" );
        if( nSeqIdx < nRemaining )
        {
            pLabeledSeq[nSeqIdx] = aData[nSourceIndex];
            if( aData[nSourceIndex].is())
                SetRole( aData[nSourceIndex]->getValues(), u"values-last"_ustr);
            ++nSourceIndex;
            ++nSeqIdx;
        }

        // 4. open
        OSL_ENSURE( nSeqIdx >= nRemaining, "could have created full series" );
    }

    // create DataSeries
    std::vector< std::vector< rtl::Reference< DataSeries > > > aResultSeries( nNumberOfGroups );
    sal_Int32 nGroupIndex;
    std::size_t nReUsedSeriesIdx = 0;
    for( nGroupIndex=0; nGroupIndex<nNumberOfGroups; ++nGroupIndex )
    {
        const sal_Int32 nNumSeriesData = aSequences[nGroupIndex].size();
        aResultSeries[nGroupIndex].resize( nNumSeriesData );
        auto & pResultSerie = aResultSeries[nGroupIndex];
        for( sal_Int32 nSeriesIdx = 0; nSeriesIdx < nNumSeriesData; ++nSeriesIdx, ++nReUsedSeriesIdx )
        {
            try
            {
                rtl::Reference< DataSeries > xSeries;
                if( nReUsedSeriesIdx < rSeriesToReUse.size())
                    xSeries = rSeriesToReUse[nReUsedSeriesIdx];
                else
                    xSeries = new DataSeries;
                assert( xSeries.is() );
                xSeries->setData( aSequences[nGroupIndex][nSeriesIdx] );
                pResultSerie[nSeriesIdx] = xSeries;
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
    std::size_t nNumberOfNecessarySequences = 3;
    // open
    StockChartTypeTemplate::StockVariant eVar( GetStockVariant());
    if( ( eVar == StockChartTypeTemplate::StockVariant::Open ) ||
        ( eVar == StockChartTypeTemplate::StockVariant::VolumeOpen ))
        ++nNumberOfNecessarySequences;
    // volume
    bool bHasVolume = (( eVar == StockChartTypeTemplate::StockVariant::Volume ) ||
                       ( eVar == StockChartTypeTemplate::StockVariant::VolumeOpen ));

    // 1. correct number of sub-types
    if( aInterpretedData.Series.size() < (bHasVolume ? 2U : 1U ))
        return false;

    // 2. a. volume -- use default check
    if( bHasVolume )
    {
        if( ! DataInterpreter::isDataCompatible(
                { std::vector< std::vector< rtl::Reference< DataSeries > > >{
                                     aInterpretedData.Series[0] },
                  aInterpretedData.Categories }))
            return false;
    }

    // 2. b. candlestick
    {
        OSL_ASSERT( aInterpretedData.Series.size() > (bHasVolume ? 1U : 0U));
        const std::vector< rtl::Reference< DataSeries > > & aSeries = aInterpretedData.Series[(bHasVolume ? 1 : 0)];
        if(aSeries.empty())
            return false;
        for( rtl::Reference< DataSeries > const & dataSeries : aSeries )
        {
            try
            {
                if( dataSeries->getDataSequences2().size() != nNumberOfNecessarySequences )
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
