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

#include "BubbleDataInterpreter.hxx"
#include <DataSeries.hxx>
#include <DataSeriesHelper.hxx>
#include <CommonConverters.hxx>
#include <com/sun/star/chart2/data/XDataSink.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <tools/diagnose_ex.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using namespace ::std;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart
{

BubbleDataInterpreter::BubbleDataInterpreter() :
        DataInterpreter()
{
}

BubbleDataInterpreter::~BubbleDataInterpreter()
{
}

// ____ XDataInterpreter ____
chart2::InterpretedData SAL_CALL BubbleDataInterpreter::interpretDataSource(
    const Reference< chart2::data::XDataSource >& xSource,
    const Sequence< beans::PropertyValue >& aArguments,
    const Sequence< Reference< XDataSeries > >& aSeriesToReUse )
{
    if( ! xSource.is())
        return InterpretedData();

    Sequence< Reference< data::XLabeledDataSequence > > aData( xSource->getDataSequences() );

    Reference< data::XLabeledDataSequence > xValuesX;
    vector< Reference< data::XLabeledDataSequence > > aYValuesVector;
    vector< Reference< data::XLabeledDataSequence > > aSizeValuesVector;

    Reference< data::XLabeledDataSequence > xCategories;
    bool bHasCategories = HasCategories( aArguments, aData );
    bool bUseCategoriesAsX = UseCategoriesAsX( aArguments );

    sal_Int32 nDataSeqCount = aData.getLength();

    bool bSetXValues = bHasCategories ? ( (nDataSeqCount-1) > 2 && (nDataSeqCount-1) % 2 != 0 )
                                 :( nDataSeqCount > 2 && nDataSeqCount % 2 != 0 );

    bool bCategoriesUsed = false;
    bool bNextIsYValues = bHasCategories ? nDataSeqCount>2 : nDataSeqCount>1;
    for( sal_Int32 nDataIdx = 0; nDataIdx < nDataSeqCount; ++nDataIdx )
    {
        try
        {
            if( bHasCategories && !bCategoriesUsed )
            {
                xCategories.set( aData[nDataIdx] );
                if( xCategories.is())
                {
                    SetRole( xCategories->getValues(), "categories");
                    if( bUseCategoriesAsX )
                    {
                        bSetXValues = false;
                        bNextIsYValues = nDataSeqCount > 2;
                    }
                }
                bCategoriesUsed = true;
            }
            else if( !xValuesX.is() && bSetXValues )
            {
                xValuesX.set( aData[nDataIdx] );
                if( xValuesX.is())
                    SetRole( xValuesX->getValues(), "values-x");
            }
            else if( bNextIsYValues )
            {
                aYValuesVector.push_back( aData[nDataIdx] );
                if( aData[nDataIdx].is())
                    SetRole( aData[nDataIdx]->getValues(), "values-y");
                bNextIsYValues = false;
            }
            else if( !bNextIsYValues )
            {
                aSizeValuesVector.push_back( aData[nDataIdx] );
                if( aData[nDataIdx].is())
                    SetRole( aData[nDataIdx]->getValues(), "values-size");
                bNextIsYValues = (nDataSeqCount-(nDataIdx+1)) >= 2;//two or more left
            }
        }
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }

    // create DataSeries
    sal_Int32 nSeriesIndex = 0;
    vector< Reference< XDataSeries > > aSeriesVec;
    aSeriesVec.reserve( aSizeValuesVector.size());

    Reference< data::XLabeledDataSequence > xClonedXValues = xValuesX;
    Reference< util::XCloneable > xCloneableX( xValuesX, uno::UNO_QUERY );

    for( size_t nN = 0; nN < aSizeValuesVector.size(); ++nN, ++nSeriesIndex )
    {
        vector< Reference< data::XLabeledDataSequence > > aNewData;
        if( xValuesX.is() )
        {
            if( nN > 0 && xCloneableX.is() )
                xClonedXValues.set( xCloneableX->createClone(), uno::UNO_QUERY );
            aNewData.push_back( xClonedXValues );
        }
        if( aYValuesVector.size() > nN )
            aNewData.push_back( aYValuesVector[nN] );
        aNewData.push_back(aSizeValuesVector[nN]);

        Reference< XDataSeries > xSeries;
        if( nSeriesIndex < aSeriesToReUse.getLength())
            xSeries.set( aSeriesToReUse[nSeriesIndex] );
        else
            xSeries.set( new DataSeries );
        OSL_ASSERT( xSeries.is() );
        Reference< data::XDataSink > xSink( xSeries, uno::UNO_QUERY );
        OSL_ASSERT( xSink.is() );
        xSink->setData( comphelper::containerToSequence( aNewData ) );

        aSeriesVec.push_back( xSeries );
    }

    Sequence< Sequence< Reference< XDataSeries > > > aSeries(1);
    aSeries[0] = comphelper::containerToSequence( aSeriesVec );
    return InterpretedData( aSeries, xCategories );
}

chart2::InterpretedData SAL_CALL BubbleDataInterpreter::reinterpretDataSeries(
    const chart2::InterpretedData& aInterpretedData )
{
    InterpretedData aResult( aInterpretedData );

    sal_Int32 i=0;
    Sequence< Reference< XDataSeries > > aSeries( FlattenSequence( aInterpretedData.Series ));
    const sal_Int32 nCount = aSeries.getLength();
    for( ; i<nCount; ++i )
    {
        try
        {
            Reference< data::XDataSource > xSeriesSource( aSeries[i], uno::UNO_QUERY_THROW );
            Sequence< Reference< data::XLabeledDataSequence > > aNewSequences;

            Reference< data::XLabeledDataSequence > xValuesSize(
                DataSeriesHelper::getDataSequenceByRole( xSeriesSource, "values-size" ));
            Reference< data::XLabeledDataSequence > xValuesY(
                DataSeriesHelper::getDataSequenceByRole( xSeriesSource, "values-y" ));
            Reference< data::XLabeledDataSequence > xValuesX(
                DataSeriesHelper::getDataSequenceByRole( xSeriesSource, "values-x" ));

            if( ! xValuesX.is() ||
                ! xValuesY.is() ||
                ! xValuesSize.is() )
            {
                vector< Reference< data::XLabeledDataSequence > > aValueSeqVec(
                    DataSeriesHelper::getAllDataSequencesByRole(
                        xSeriesSource->getDataSequences(), "values" ));
                if( xValuesX.is())
                    aValueSeqVec.erase( find( aValueSeqVec.begin(), aValueSeqVec.end(), xValuesX ));
                if( xValuesY.is())
                    aValueSeqVec.erase( find( aValueSeqVec.begin(), aValueSeqVec.end(), xValuesY ));
                if( xValuesSize.is())
                    aValueSeqVec.erase( find( aValueSeqVec.begin(), aValueSeqVec.end(), xValuesSize ));

                size_t nIndex = 0;

                if( ! xValuesSize.is() &&
                    aValueSeqVec.size() > nIndex )
                {
                    xValuesSize.set( aValueSeqVec[nIndex++] );
                    if( xValuesSize.is())
                        SetRole( xValuesSize->getValues(), "values-size");
                }

                if( ! xValuesY.is() &&
                    aValueSeqVec.size() > nIndex )
                {
                    xValuesY.set( aValueSeqVec[nIndex++] );
                    if( xValuesY.is())
                        SetRole( xValuesY->getValues(), "values-y");
                }

                if( ! xValuesX.is() &&
                    aValueSeqVec.size() > nIndex )
                {
                    xValuesX.set( aValueSeqVec[nIndex++] );
                    if( xValuesX.is())
                        SetRole( xValuesY->getValues(), "values-x");
                }
            }
            if( xValuesSize.is())
            {
                if( xValuesY.is() )
                {
                    if( xValuesX.is() )
                    {
                        aNewSequences.realloc(3);
                        aNewSequences[0] = xValuesX;
                        aNewSequences[1] = xValuesY;
                        aNewSequences[2] = xValuesSize;
                    }
                    else
                    {
                        aNewSequences.realloc(2);
                        aNewSequences[0] = xValuesY;
                        aNewSequences[1] = xValuesSize;
                    }
                }
                else
                {
                    aNewSequences.realloc(1);
                    aNewSequences[0] = xValuesSize;
                }
            }

            const Sequence< Reference< data::XLabeledDataSequence > > aSeqs( xSeriesSource->getDataSequences());
            if( aSeqs.getLength() != aNewSequences.getLength() )
            {
#if OSL_DEBUG_LEVEL > 0
                for( auto const & j : aSeqs )
                {
                    assert( (j == xValuesY || j == xValuesX || j == xValuesSize) && "All sequences should be used" );
                }
#endif
                Reference< data::XDataSink > xSink( xSeriesSource, uno::UNO_QUERY_THROW );
                xSink->setData( aNewSequences );
            }
        }
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }

    return aResult;
}

sal_Bool SAL_CALL BubbleDataInterpreter::isDataCompatible(
    const chart2::InterpretedData& aInterpretedData )
{
    const Sequence< Reference< XDataSeries > > aSeries( FlattenSequence( aInterpretedData.Series ));
    for( Reference< XDataSeries >  const & dataSeries : aSeries )
    {
        try
        {
            Reference< data::XDataSource > xSrc( dataSeries, uno::UNO_QUERY_THROW );
            Sequence< Reference< data::XLabeledDataSequence > > aSeq( xSrc->getDataSequences());
            if( aSeq.getLength() != 3 )
                return false;
        }
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }

    return true;
}

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
