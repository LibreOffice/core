/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "XYDataInterpreter.hxx"
#include "DataSeries.hxx"
#include "macros.hxx"
#include "DataSeriesHelper.hxx"
#include "CommonConverters.hxx"
#include "ContainerHelper.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/chart2/data/XDataSink.hpp>
#include <com/sun/star/util/XCloneable.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using namespace ::std;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::rtl::OUString;

namespace chart
{

XYDataInterpreter::XYDataInterpreter(
    const uno::Reference< uno::XComponentContext > & xContext ) :
        DataInterpreter( xContext )
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
    throw (uno::RuntimeException)
{
    if( ! xSource.is())
        return InterpretedData();

    Sequence< Reference< data::XLabeledDataSequence > > aData( xSource->getDataSequences() );

    Reference< data::XLabeledDataSequence > xValuesX;
    vector< Reference< data::XLabeledDataSequence > > aSequencesVec;

    Reference< data::XLabeledDataSequence > xCategories;
    bool bHasCategories = HasCategories( aArguments, aData );
    bool bUseCategoriesAsX = UseCategoriesAsX( aArguments );

    // parse data
    bool bCategoriesUsed = false;
    bool bSetXValues = aData.getLength()>(bCategoriesUsed?2:1);
    for( sal_Int32 nDataIdx= 0; nDataIdx < aData.getLength(); ++nDataIdx )
    {
        try
        {
            if( bHasCategories && ! bCategoriesUsed )
            {
                xCategories.set( aData[nDataIdx] );
                if( xCategories.is())
                {
                    SetRole( xCategories->getValues(), C2U("categories"));
                    if( bUseCategoriesAsX )
                        bSetXValues = false;
                }
                bCategoriesUsed = true;
            }
            else if( !xValuesX.is() && bSetXValues )
            {
                xValuesX.set( aData[nDataIdx] );
                if( xValuesX.is())
                    SetRole( xValuesX->getValues(), C2U("values-x"));
            }
            else
            {
                aSequencesVec.push_back( aData[nDataIdx] );
                if( aData[nDataIdx].is())
                    SetRole( aData[nDataIdx]->getValues(), C2U("values-y"));
            }
        }
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }

    // create DataSeries
    vector< Reference< data::XLabeledDataSequence > >::const_iterator
          aSequencesVecIt = aSequencesVec.begin();

    sal_Int32 nSeriesIndex = 0;
    vector< Reference< XDataSeries > > aSeriesVec;
    aSeriesVec.reserve( aSequencesVec.size());

    Reference< data::XLabeledDataSequence > xClonedXValues = xValuesX;
    Reference< util::XCloneable > xCloneable( xValuesX, uno::UNO_QUERY );

    for( ;aSequencesVecIt != aSequencesVec.end(); ++aSequencesVecIt, ++nSeriesIndex )
    {
        vector< Reference< data::XLabeledDataSequence > > aNewData;

        if( aSequencesVecIt != aSequencesVec.begin() && xCloneable.is() )
            xClonedXValues.set( xCloneable->createClone(), uno::UNO_QUERY );
        if( xValuesX.is() )
            aNewData.push_back( xClonedXValues );

        aNewData.push_back( *aSequencesVecIt );

        Reference< XDataSeries > xSeries;
        if( nSeriesIndex < aSeriesToReUse.getLength())
            xSeries.set( aSeriesToReUse[nSeriesIndex] );
        else
            xSeries.set( new DataSeries( GetComponentContext() ) );
        OSL_ASSERT( xSeries.is() );
        Reference< data::XDataSink > xSink( xSeries, uno::UNO_QUERY );
        OSL_ASSERT( xSink.is() );
        xSink->setData( ContainerHelper::ContainerToSequence( aNewData ) );

        aSeriesVec.push_back( xSeries );
    }

    Sequence< Sequence< Reference< XDataSeries > > > aSeries(1);
    aSeries[0] = ContainerHelper::ContainerToSequence( aSeriesVec );
    return InterpretedData( aSeries, xCategories );
}

chart2::InterpretedData SAL_CALL XYDataInterpreter::reinterpretDataSeries(
    const chart2::InterpretedData& aInterpretedData )
    throw (uno::RuntimeException)
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
                DataSeriesHelper::getDataSequenceByRole( xSeriesSource, C2U("values-y"), false ));
            Reference< data::XLabeledDataSequence > xValuesX(
                DataSeriesHelper::getDataSequenceByRole( xSeriesSource, C2U("values-x"), false ));
            // re-use values-... as values-x/values-y
            if( ! xValuesX.is() ||
                ! xValuesY.is())
            {
                vector< Reference< data::XLabeledDataSequence > > aValueSeqVec(
                    DataSeriesHelper::getAllDataSequencesByRole(
                        xSeriesSource->getDataSequences(), C2U("values"), true ));
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
                        SetRole( xValuesY->getValues(), C2U("values-y"));
                }

                if( ! xValuesX.is() &&
                    aValueSeqVec.size() > nIndex )
                {
                    xValuesX.set( aValueSeqVec[nIndex++] );
                    if( xValuesX.is())
                        SetRole( xValuesY->getValues(), C2U("values-x"));
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

            Sequence< Reference< data::XLabeledDataSequence > > aSeqs( xSeriesSource->getDataSequences());
            if( aSeqs.getLength() != aNewSequences.getLength() )
            {
#if OSL_DEBUG_LEVEL > 1
                sal_Int32 j=0;
                for( ; j<aSeqs.getLength(); ++j )
                {
                    OSL_ENSURE( aSeqs[j] == xValuesY || aSeqs[j] == xValuesX, "All sequences should be used" );
                }
#endif
                Reference< data::XDataSink > xSink( xSeriesSource, uno::UNO_QUERY_THROW );
                xSink->setData( aNewSequences );
            }
        }
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }

    return aResult;
}

// criterion: all series must have exactly two data::XLabeledDataSequences
sal_Bool SAL_CALL XYDataInterpreter::isDataCompatible(
    const chart2::InterpretedData& aInterpretedData )
    throw (uno::RuntimeException)
{
    Sequence< Reference< XDataSeries > > aSeries( FlattenSequence( aInterpretedData.Series ));
    for( sal_Int32 i=0; i<aSeries.getLength(); ++i )
    {
        try
        {
            Reference< data::XDataSource > xSrc( aSeries[i], uno::UNO_QUERY_THROW );
            Sequence< Reference< data::XLabeledDataSequence > > aSeq( xSrc->getDataSequences());
            if( aSeq.getLength() != 2 )
                return sal_False;
        }
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }

    return sal_True;
}

} // namespace chart
