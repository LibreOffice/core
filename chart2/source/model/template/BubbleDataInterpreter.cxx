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
#include <LabeledDataSequence.hxx>
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

BubbleDataInterpreter::BubbleDataInterpreter()
{
}

BubbleDataInterpreter::~BubbleDataInterpreter()
{
}

// ____ XDataInterpreter ____
InterpretedData BubbleDataInterpreter::interpretDataSource(
    const Reference< chart2::data::XDataSource >& xSource,
    const Sequence< beans::PropertyValue >& aArguments,
    const std::vector< rtl::Reference< DataSeries > >& aSeriesToReUse )
{
    if( ! xSource.is())
        return InterpretedData();

    std::vector< uno::Reference< chart2::data::XLabeledDataSequence > > aData = DataInterpreter::getDataSequences(xSource);

    uno::Reference< chart2::data::XLabeledDataSequence > xValuesX;
    vector< uno::Reference< chart2::data::XLabeledDataSequence > > aYValuesVector;
    vector< uno::Reference< chart2::data::XLabeledDataSequence > > aSizeValuesVector;

    uno::Reference< chart2::data::XLabeledDataSequence > xCategories;
    bool bHasCategories = HasCategories( aArguments, aData );
    bool bUseCategoriesAsX = UseCategoriesAsX( aArguments );

    sal_Int32 nDataSeqCount = aData.size();

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
                xCategories = aData[nDataIdx];
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
                xValuesX = aData[nDataIdx];
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
    vector< rtl::Reference< DataSeries > > aSeriesVec;
    aSeriesVec.reserve( aSizeValuesVector.size());

    Reference< data::XLabeledDataSequence > xClonedXValues = xValuesX;
    Reference< util::XCloneable > xCloneableX( xValuesX, uno::UNO_QUERY );

    for( size_t nN = 0; nN < aSizeValuesVector.size(); ++nN, ++nSeriesIndex )
    {
        vector< uno::Reference< chart2::data::XLabeledDataSequence > > aNewData;
        if( xValuesX.is() )
        {
            if( nN > 0 && xCloneableX.is() )
                xClonedXValues.set( xCloneableX->createClone(), uno::UNO_QUERY );
            aNewData.push_back( xClonedXValues );
        }
        if( aYValuesVector.size() > nN )
            aNewData.push_back( aYValuesVector[nN] );
        aNewData.push_back(aSizeValuesVector[nN]);

        rtl::Reference< DataSeries > xSeries;
        if( nSeriesIndex < static_cast<sal_Int32>(aSeriesToReUse.size()))
            xSeries = aSeriesToReUse[nSeriesIndex];
        else
            xSeries = new DataSeries;
        assert( xSeries.is() );
        xSeries->setData( aNewData );

        aSeriesVec.push_back( xSeries );
    }

    return { { aSeriesVec }, xCategories };
}

InterpretedData BubbleDataInterpreter::reinterpretDataSeries(
    const InterpretedData& aInterpretedData )
{
    InterpretedData aResult( aInterpretedData );

    sal_Int32 i=0;
    std::vector< rtl::Reference< DataSeries > > aSeries( FlattenSequence( aInterpretedData.Series ));
    const sal_Int32 nCount = aSeries.size();
    for( ; i<nCount; ++i )
    {
        try
        {
            std::vector< uno::Reference< chart2::data::XLabeledDataSequence > > aNewSequences;

            uno::Reference< chart2::data::XLabeledDataSequence > xValuesSize(
                DataSeriesHelper::getDataSequenceByRole( aSeries[i], "values-size" ));
            uno::Reference< chart2::data::XLabeledDataSequence > xValuesY(
                DataSeriesHelper::getDataSequenceByRole( aSeries[i], "values-y" ));
            uno::Reference< chart2::data::XLabeledDataSequence > xValuesX(
                DataSeriesHelper::getDataSequenceByRole( aSeries[i], "values-x" ));

            if( ! xValuesX.is() ||
                ! xValuesY.is() ||
                ! xValuesSize.is() )
            {
                vector< uno::Reference< chart2::data::XLabeledDataSequence > > aValueSeqVec(
                    DataSeriesHelper::getAllDataSequencesByRole(
                        aSeries[i]->getDataSequences2(), "values" ));
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
                    xValuesSize = aValueSeqVec[nIndex++];
                    if( xValuesSize.is())
                        SetRole( xValuesSize->getValues(), "values-size");
                }

                if( ! xValuesY.is() &&
                    aValueSeqVec.size() > nIndex )
                {
                    xValuesY = aValueSeqVec[nIndex++];
                    if( xValuesY.is())
                        SetRole( xValuesY->getValues(), "values-y");
                }

                if( ! xValuesX.is() &&
                    aValueSeqVec.size() > nIndex )
                {
                    xValuesX = aValueSeqVec[nIndex++];
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
                        aNewSequences = { xValuesX, xValuesY, xValuesSize };
                    }
                    else
                    {
                        aNewSequences = { xValuesY, xValuesSize };
                    }
                }
                else
                {
                    aNewSequences = { xValuesSize };
                }
            }

            const std::vector< uno::Reference< data::XLabeledDataSequence > > & aSeqs = aSeries[i]->getDataSequences2();
            if( aSeqs.size() != aNewSequences.size() )
            {
#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
                for( auto const & j : aSeqs )
                {
                    assert( (j == xValuesY || j == xValuesX || j == xValuesSize) && "All sequences should be used" );
                }
#endif
                aSeries[i]->setData( aNewSequences );
            }
        }
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }

    return aResult;
}

bool BubbleDataInterpreter::isDataCompatible(
    const InterpretedData& aInterpretedData )
{
    const std::vector< rtl::Reference< DataSeries > > aSeries( FlattenSequence( aInterpretedData.Series ));
    for( rtl::Reference< DataSeries >  const & dataSeries : aSeries )
    {
        try
        {
            if( dataSeries->getDataSequences2().size() != 3 )
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
