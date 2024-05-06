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

#include "XYDataInterpreter.hxx"
#include <DataSeries.hxx>
#include <DataSeriesHelper.hxx>
#include <CommonConverters.hxx>
#include <com/sun/star/util/XCloneable.hpp>
#include <comphelper/diagnose_ex.hxx>
#include <sal/log.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart
{

XYDataInterpreter::XYDataInterpreter()
{
}

XYDataInterpreter::~XYDataInterpreter()
{
}

// ____ XDataInterpreter ____
InterpretedData XYDataInterpreter::interpretDataSource(
    const Reference< chart2::data::XDataSource >& xSource,
    const Sequence< beans::PropertyValue >& aArguments,
    const std::vector< rtl::Reference< DataSeries > >& aSeriesToReUse )
{
    if( ! xSource.is())
        return InterpretedData();

    std::vector< uno::Reference< chart2::data::XLabeledDataSequence > > aData = DataInterpreter::getDataSequences(xSource);

    uno::Reference< chart2::data::XLabeledDataSequence > xValuesX;
    std::vector< uno::Reference< chart2::data::XLabeledDataSequence > > aSequencesVec;

    uno::Reference< chart2::data::XLabeledDataSequence > xCategories;
    bool bHasCategories = HasCategories( aArguments, aData );
    bool bUseCategoriesAsX = UseCategoriesAsX( aArguments );

    // parse data
    bool bCategoriesUsed = false;
    bool bSetXValues = aData.size()>1;
    for( uno::Reference< chart2::data::XLabeledDataSequence > const & labelData : aData )
    {
        try
        {
            if( bHasCategories && ! bCategoriesUsed )
            {
                xCategories = labelData;
                if( xCategories.is())
                {
                    SetRole( xCategories->getValues(), u"categories"_ustr);
                    if( bUseCategoriesAsX )
                        bSetXValues = false;
                }
                bCategoriesUsed = true;
            }
            else if( !xValuesX.is() && bSetXValues )
            {
                xValuesX = labelData;
                if( xValuesX.is())
                    SetRole( xValuesX->getValues(), u"values-x"_ustr);
            }
            else
            {
                aSequencesVec.push_back( labelData );
                if( labelData.is())
                    SetRole( labelData->getValues(), u"values-y"_ustr);
            }
        }
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }

    // create DataSeries
    std::vector< rtl::Reference< DataSeries > > aSeriesVec;
    aSeriesVec.reserve( aSequencesVec.size());

    Reference< data::XLabeledDataSequence > xClonedXValues = xValuesX;
    Reference< util::XCloneable > xCloneable( xValuesX, uno::UNO_QUERY );

    std::size_t nSeriesIndex = 0;
    for (auto const& elem : aSequencesVec)
    {
        std::vector< uno::Reference< chart2::data::XLabeledDataSequence > > aNewData;

        if( nSeriesIndex && xCloneable.is() )
            xClonedXValues.set( xCloneable->createClone(), uno::UNO_QUERY );
        if( xValuesX.is() )
            aNewData.push_back( xClonedXValues );

        aNewData.push_back(elem);

        rtl::Reference< DataSeries > xSeries;
        if( nSeriesIndex < aSeriesToReUse.size())
            xSeries = aSeriesToReUse[nSeriesIndex];
        else
            xSeries = new DataSeries;
        assert( xSeries.is() );
        xSeries->setData( aNewData );

        aSeriesVec.push_back( xSeries );
        ++nSeriesIndex;
    }

    return { { aSeriesVec }, xCategories };
}

InterpretedData XYDataInterpreter::reinterpretDataSeries(
    const InterpretedData& aInterpretedData )
{
    InterpretedData aResult( aInterpretedData );

    sal_Int32 i=0;
    std::vector< rtl::Reference< DataSeries > > aSeries = FlattenSequence( aInterpretedData.Series );
    const sal_Int32 nCount = aSeries.size();
    for( ; i<nCount; ++i )
    {
        try
        {
            std::vector< uno::Reference< data::XLabeledDataSequence > > aNewSequences;

            // values-y
            uno::Reference< chart2::data::XLabeledDataSequence > xValuesY(
                DataSeriesHelper::getDataSequenceByRole( aSeries[i], u"values-y"_ustr ));
            uno::Reference< chart2::data::XLabeledDataSequence > xValuesX(
                DataSeriesHelper::getDataSequenceByRole( aSeries[i], u"values-x"_ustr ));
            // re-use values-... as values-x/values-y
            if( ! xValuesX.is() ||
                ! xValuesY.is())
            {
                std::vector< uno::Reference< chart2::data::XLabeledDataSequence > > aValueSeqVec(
                    DataSeriesHelper::getAllDataSequencesByRole(
                        aSeries[i]->getDataSequences2(), u"values"_ustr ));
                if( xValuesX.is())
                    aValueSeqVec.erase( find( aValueSeqVec.begin(), aValueSeqVec.end(), xValuesX ));
                if( xValuesY.is())
                    aValueSeqVec.erase( find( aValueSeqVec.begin(), aValueSeqVec.end(), xValuesY ));

                size_t nIndex = 0;
                if( ! xValuesY.is() &&
                    aValueSeqVec.size() > nIndex )
                {
                    xValuesY = aValueSeqVec[nIndex++];
                    if( xValuesY.is())
                        SetRole( xValuesY->getValues(), u"values-y"_ustr);
                }

                if( ! xValuesX.is() &&
                    aValueSeqVec.size() > nIndex )
                {
                    xValuesX = aValueSeqVec[nIndex++];
                    if( xValuesX.is())
                        SetRole( xValuesY->getValues(), u"values-x"_ustr);
                }
            }
            if( xValuesY.is())
            {
                if( xValuesX.is())
                {
                    aNewSequences = { xValuesX, xValuesY };
                }
                else
                {
                    aNewSequences = { xValuesY };
                }
            }

            const std::vector< uno::Reference< data::XLabeledDataSequence > > & aSeqs = aSeries[i]->getDataSequences2();
            if( aSeqs.size() != aNewSequences.size() )
            {
#ifdef DBG_UTIL
                for( auto const & j : aSeqs )
                {
                    SAL_WARN_IF((j == xValuesY || j == xValuesX), "chart2.template", "All sequences should be used" );
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

// criterion: all series must have exactly two data::XLabeledDataSequences
bool XYDataInterpreter::isDataCompatible(
    const InterpretedData& aInterpretedData )
{
    const std::vector< rtl::Reference< DataSeries > > aSeries = FlattenSequence( aInterpretedData.Series );
    for( rtl::Reference< DataSeries > const & dataSeries : aSeries )
    {
        try
        {
            if( dataSeries->getDataSequences2().size() != 2 )
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
