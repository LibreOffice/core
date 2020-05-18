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

#include "XYDataInterpreter.hxx"
#include <DataSeries.hxx>
#include <DataSeriesHelper.hxx>
#include <CommonConverters.hxx>
#include <com/sun/star/chart2/data/XDataSink.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <tools/diagnose_ex.h>
#include <sal/log.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using namespace ::std;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart
{

XYDataInterpreter::XYDataInterpreter() :
        DataInterpreter()
{
}

XYDataInterpreter::~XYDataInterpreter()
{
}

// ____ XDataInterpreter ____
chart2::InterpretedData SAL_CALL XYDataInterpreter::interpretDataSource(
    const Reference< chart2::data::XDataSource >& xSource,
    const Sequence< beans::PropertyValue >& aArguments,
    const Sequence< Reference< XDataSeries > >& aSeriesToReUse )
{
    if( ! xSource.is())
        return InterpretedData();

    const Sequence< Reference< data::XLabeledDataSequence > > aData( xSource->getDataSequences() );

    Reference< data::XLabeledDataSequence > xValuesX;
    vector< Reference< data::XLabeledDataSequence > > aSequencesVec;

    Reference< data::XLabeledDataSequence > xCategories;
    bool bHasCategories = HasCategories( aArguments, aData );
    bool bUseCategoriesAsX = UseCategoriesAsX( aArguments );

    // parse data
    bool bCategoriesUsed = false;
    bool bSetXValues = aData.getLength()>1;
    for( Reference< data::XLabeledDataSequence > const & labelData : aData )
    {
        try
        {
            if( bHasCategories && ! bCategoriesUsed )
            {
                xCategories.set( labelData );
                if( xCategories.is())
                {
                    SetRole( xCategories->getValues(), "categories");
                    if( bUseCategoriesAsX )
                        bSetXValues = false;
                }
                bCategoriesUsed = true;
            }
            else if( !xValuesX.is() && bSetXValues )
            {
                xValuesX.set( labelData );
                if( xValuesX.is())
                    SetRole( xValuesX->getValues(), "values-x");
            }
            else
            {
                aSequencesVec.push_back( labelData );
                if( labelData.is())
                    SetRole( labelData->getValues(), "values-y");
            }
        }
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }

    // create DataSeries
    vector< Reference< XDataSeries > > aSeriesVec;
    aSeriesVec.reserve( aSequencesVec.size());

    Reference< data::XLabeledDataSequence > xClonedXValues = xValuesX;
    Reference< util::XCloneable > xCloneable( xValuesX, uno::UNO_QUERY );

    sal_Int32 nSeriesIndex = 0;
    for (auto const& elem : aSequencesVec)
    {
        vector< Reference< data::XLabeledDataSequence > > aNewData;

        if( nSeriesIndex && xCloneable.is() )
            xClonedXValues.set( xCloneable->createClone(), uno::UNO_QUERY );
        if( xValuesX.is() )
            aNewData.push_back( xClonedXValues );

        aNewData.push_back(elem);

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
        ++nSeriesIndex;
    }

    Sequence< Sequence< Reference< XDataSeries > > > aSeries(1);
    aSeries[0] = comphelper::containerToSequence( aSeriesVec );
    return InterpretedData( aSeries, xCategories );
}

chart2::InterpretedData SAL_CALL XYDataInterpreter::reinterpretDataSeries(
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

            // values-y
            Reference< data::XLabeledDataSequence > xValuesY(
                DataSeriesHelper::getDataSequenceByRole( xSeriesSource, "values-y" ));
            Reference< data::XLabeledDataSequence > xValuesX(
                DataSeriesHelper::getDataSequenceByRole( xSeriesSource, "values-x" ));
            // re-use values-... as values-x/values-y
            if( ! xValuesX.is() ||
                ! xValuesY.is())
            {
                vector< Reference< data::XLabeledDataSequence > > aValueSeqVec(
                    DataSeriesHelper::getAllDataSequencesByRole(
                        xSeriesSource->getDataSequences(), "values" ));
                if( xValuesX.is())
                    aValueSeqVec.erase( find( aValueSeqVec.begin(), aValueSeqVec.end(), xValuesX ));
                if( xValuesY.is())
                    aValueSeqVec.erase( find( aValueSeqVec.begin(), aValueSeqVec.end(), xValuesY ));

                size_t nIndex = 0;
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
            if( xValuesY.is())
            {
                if( xValuesX.is())
                {
                    aNewSequences.realloc(2);
                    aNewSequences[0] = xValuesX;
                    aNewSequences[1] = xValuesY;
                }
                else
                {
                    aNewSequences.realloc(1);
                    aNewSequences[0] = xValuesY;
                }
            }

            const Sequence< Reference< data::XLabeledDataSequence > > aSeqs( xSeriesSource->getDataSequences());
            if( aSeqs.getLength() != aNewSequences.getLength() )
            {
#ifdef DBG_UTIL
                for( auto const & j : aSeqs )
                {
                    SAL_WARN_IF((j == xValuesY || j == xValuesX), "chart2.template", "All sequences should be used" );
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

// criterion: all series must have exactly two data::XLabeledDataSequences
sal_Bool SAL_CALL XYDataInterpreter::isDataCompatible(
    const chart2::InterpretedData& aInterpretedData )
{
    const Sequence< Reference< XDataSeries > > aSeries( FlattenSequence( aInterpretedData.Series ));
    for( Reference< XDataSeries > const & dataSeries : aSeries )
    {
        try
        {
            Reference< data::XDataSource > xSrc( dataSeries, uno::UNO_QUERY_THROW );
            Sequence< Reference< data::XLabeledDataSequence > > aSeq( xSrc->getDataSequences());
            if( aSeq.getLength() != 2 )
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
