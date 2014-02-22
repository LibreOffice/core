/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "StockDataInterpreter.hxx"
#include "DataSeries.hxx"
#include "macros.hxx"
#include "DataSeriesHelper.hxx"
#include "CommonConverters.hxx"
#include "ContainerHelper.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>

#include <vector>
#include <algorithm>
#include <iterator>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using namespace ::std;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using namespace ::chart::ContainerHelper;

namespace chart
{


StockDataInterpreter::StockDataInterpreter(
    StockChartTypeTemplate::StockVariant eVariant,
    const Reference< uno::XComponentContext > & xContext ) :
        DataInterpreter( xContext ),
        m_eStockVariant( eVariant )
{}

StockDataInterpreter::~StockDataInterpreter()
{}

StockChartTypeTemplate::StockVariant StockDataInterpreter::GetStockVariant() const
{
    return m_eStockVariant;
}


InterpretedData SAL_CALL StockDataInterpreter::interpretDataSource(
    const Reference< data::XDataSource >& xSource,
    const Sequence< beans::PropertyValue >& rArguments,
    const Sequence< Reference< XDataSeries > >& rSeriesToReUse )
    throw (uno::RuntimeException)
{
    if( ! xSource.is())
        return InterpretedData();

    Reference< data::XLabeledDataSequence > xCategories;
    Sequence< Reference< data::XLabeledDataSequence > > aData( xSource->getDataSequences() );
    const sal_Int32 nDataCount( aData.getLength());

    
    const StockChartTypeTemplate::StockVariant eVar( GetStockVariant());
    const bool bHasOpenValues (( eVar == StockChartTypeTemplate::OPEN_LOW_HI_CLOSE ) ||
                               ( eVar == StockChartTypeTemplate::VOL_OPEN_LOW_HI_CLOSE ));
    const bool bHasVolume (( eVar == StockChartTypeTemplate::VOL_LOW_HI_CLOSE ) ||
                           ( eVar == StockChartTypeTemplate::VOL_OPEN_LOW_HI_CLOSE ));
    const bool bHasCategories( HasCategories( rArguments, aData ));

    
    
    sal_Int32 nNumberOfNecessarySequences( 3 );
    if( bHasOpenValues )
        ++nNumberOfNecessarySequences;
    if( bHasVolume )
        ++nNumberOfNecessarySequences;

    
    
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
    
    Sequence< Sequence< Sequence< Reference< data::XLabeledDataSequence > > > > aSequences( nNumberOfGroups );
    sal_Int32 nBarGroupIndex( 0 );
    sal_Int32 nCandleStickGroupIndex( nNumberOfGroups - 1 );

    
    if( nRemaining > 0  )
        ++nCandleStickSeries;
    aSequences[nCandleStickGroupIndex].realloc( nCandleStickSeries );
    if( bHasVolume )
    {
        
        
        
        if( nRemaining > 1 )
            ++nVolumeSeries;
        aSequences[nBarGroupIndex].realloc( nVolumeSeries );
    }

    
    sal_Int32 nSourceIndex = 0;   

    
    if( bHasCategories )
    {
        xCategories.set( aData[nSourceIndex] );
        ++nSourceIndex;
    }

    
    for( sal_Int32 nLabeledSeqIdx=0; nLabeledSeqIdx<nNumOfFullSeries; ++nLabeledSeqIdx )
    {
        
        if( bHasVolume )
        {
            aSequences[nBarGroupIndex][nLabeledSeqIdx].realloc( 1 );
            aSequences[nBarGroupIndex][nLabeledSeqIdx][0].set( aData[nSourceIndex] );
            if( aData[nSourceIndex].is())
                SetRole( aData[nSourceIndex]->getValues(), "values-y");
            ++nSourceIndex;
        }

        sal_Int32 nSeqIdx = 0;
        if( bHasOpenValues )
        {
            aSequences[nCandleStickGroupIndex][nLabeledSeqIdx].realloc( 4 );
            aSequences[nCandleStickGroupIndex][nLabeledSeqIdx][nSeqIdx].set( aData[nSourceIndex] );
            if( aData[nSourceIndex].is())
                SetRole( aData[nSourceIndex]->getValues(), "values-first");
            ++nSourceIndex, ++nSeqIdx;
        }
        else
            aSequences[nCandleStickGroupIndex][nLabeledSeqIdx].realloc( 3 );

        aSequences[nCandleStickGroupIndex][nLabeledSeqIdx][nSeqIdx].set( aData[nSourceIndex] );
        if( aData[nSourceIndex].is())
            SetRole( aData[nSourceIndex]->getValues(), "values-min");
        ++nSourceIndex, ++nSeqIdx;

        aSequences[nCandleStickGroupIndex][nLabeledSeqIdx][nSeqIdx].set( aData[nSourceIndex] );
        if( aData[nSourceIndex].is())
            SetRole( aData[nSourceIndex]->getValues(), "values-max");
        ++nSourceIndex, ++nSeqIdx;

        aSequences[nCandleStickGroupIndex][nLabeledSeqIdx][nSeqIdx].set( aData[nSourceIndex] );
        if( aData[nSourceIndex].is())
            SetRole( aData[nSourceIndex]->getValues(), "values-last");
        ++nSourceIndex, ++nSeqIdx;
    }

    
    if( bHasVolume && nRemaining > 1 )
    {
        OSL_ASSERT( nVolumeSeries > nNumOfFullSeries );
        aSequences[nBarGroupIndex][nVolumeSeries - 1].realloc( 1 );
        OSL_ASSERT( nDataCount > nSourceIndex );
        if( aData[nSourceIndex].is())
            SetRole( aData[nSourceIndex]->getValues(), "values-y");
        aSequences[nBarGroupIndex][nVolumeSeries - 1][0].set( aData[nSourceIndex] );
        ++nSourceIndex;
        --nRemaining;
        OSL_ENSURE( nRemaining, "additional bar should only be used if there is at least one more sequence for a candle stick" );
    }

    
    if( nRemaining > 0 )
    {
        OSL_ASSERT( nCandleStickSeries > nNumOfFullSeries );
        const sal_Int32 nSeriesIndex = nCandleStickSeries - 1;
        aSequences[nCandleStickGroupIndex][nSeriesIndex].realloc( nRemaining );
        OSL_ASSERT( nDataCount > nSourceIndex );

        
        sal_Int32 nSeqIdx( 0 );
        aSequences[nCandleStickGroupIndex][nSeriesIndex][nSeqIdx].set( aData[nSourceIndex] );
        if( aData[nSourceIndex].is())
            SetRole( aData[nSourceIndex]->getValues(), "values-min");
        ++nSourceIndex, ++nSeqIdx;

        
        if( nSeqIdx < nRemaining )
        {
            aSequences[nCandleStickGroupIndex][nSeriesIndex][nSeqIdx].set( aData[nSourceIndex] );
            if( aData[nSourceIndex].is())
                SetRole( aData[nSourceIndex]->getValues(), "values-max");
            ++nSourceIndex, ++nSeqIdx;
        }

        
        OSL_ENSURE( bHasOpenValues || nSeqIdx >= nRemaining, "could have created full series" );
        if( nSeqIdx < nRemaining )
        {
            aSequences[nCandleStickGroupIndex][nSeriesIndex][nSeqIdx].set( aData[nSourceIndex] );
            if( aData[nSourceIndex].is())
                SetRole( aData[nSourceIndex]->getValues(), "values-last");
            ++nSourceIndex, ++nSeqIdx;
        }

        
        OSL_ENSURE( nSeqIdx >= nRemaining, "could have created full series" );
    }

    
    Sequence< Sequence< Reference< XDataSeries > > > aResultSeries( nNumberOfGroups );
    sal_Int32 nGroupIndex, nReUsedSeriesIdx = 0;
    for( nGroupIndex=0; nGroupIndex<nNumberOfGroups; ++nGroupIndex )
    {
        const sal_Int32 nNumSeriesData = aSequences[nGroupIndex].getLength();
        aResultSeries[nGroupIndex].realloc( nNumSeriesData );
        for( sal_Int32 nSeriesIdx = 0; nSeriesIdx < nNumSeriesData; ++nSeriesIdx, ++nReUsedSeriesIdx )
        {
            try
            {
                Reference< XDataSeries > xSeries;
                if( nReUsedSeriesIdx < rSeriesToReUse.getLength())
                    xSeries.set( rSeriesToReUse[nReUsedSeriesIdx] );
                else
                    xSeries.set( new DataSeries( GetComponentContext() ) );
                OSL_ASSERT( xSeries.is() );
                Reference< data::XDataSink > xSink( xSeries, uno::UNO_QUERY_THROW );
                OSL_ASSERT( xSink.is() );
                xSink->setData( aSequences[nGroupIndex][nSeriesIdx] );
                aResultSeries[nGroupIndex][nSeriesIdx].set( xSeries );
            }
            catch( const uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        }
    }

    return InterpretedData( aResultSeries, xCategories );
}






sal_Bool SAL_CALL StockDataInterpreter::isDataCompatible(
    const InterpretedData& aInterpretedData )
    throw (uno::RuntimeException)
{
    
    sal_Int32 nNumberOfNecessarySequences = 3;
    
    StockChartTypeTemplate::StockVariant eVar( GetStockVariant());
    if( ( eVar == StockChartTypeTemplate::OPEN_LOW_HI_CLOSE ) ||
        ( eVar == StockChartTypeTemplate::VOL_OPEN_LOW_HI_CLOSE ))
        ++nNumberOfNecessarySequences;
    
    bool bHasVolume = (( eVar == StockChartTypeTemplate::VOL_LOW_HI_CLOSE ) ||
                       ( eVar == StockChartTypeTemplate::VOL_OPEN_LOW_HI_CLOSE ));

    
    if( aInterpretedData.Series.getLength() < (bHasVolume ? 2 : 1 ))
        return sal_False;

    
    if( bHasVolume )
    {
        if( ! DataInterpreter::isDataCompatible(
                InterpretedData( Sequence< Sequence< Reference< XDataSeries > > >(
                                     aInterpretedData.Series.getConstArray(), 1 ),
                                 aInterpretedData.Categories )))
            return sal_False;
    }

    
    {
        OSL_ASSERT( aInterpretedData.Series.getLength() > (bHasVolume ? 1 : 0));
        Sequence< Reference< XDataSeries > > aSeries( aInterpretedData.Series[(bHasVolume ? 1 : 0)] );
        if(!aSeries.getLength())
            return sal_False;
        for( sal_Int32 i=0; i<aSeries.getLength(); ++i )
        {
            try
            {
                Reference< data::XDataSource > xSrc( aSeries[i], uno::UNO_QUERY_THROW );
                Sequence< Reference< data::XLabeledDataSequence > > aSeq( xSrc->getDataSequences());
                if( aSeq.getLength() != nNumberOfNecessarySequences )
                    return sal_False;
            }
            catch( const uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        }
    }

    
    

    return sal_True;
}

InterpretedData SAL_CALL StockDataInterpreter::reinterpretDataSeries(
    const InterpretedData& aInterpretedData )
    throw (uno::RuntimeException)
{
    
    return aInterpretedData;
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
