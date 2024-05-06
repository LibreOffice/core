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

#include <DataInterpreter.hxx>
#include <DataSeries.hxx>
#include <DataSource.hxx>
#include <DataSeriesHelper.hxx>
#include <CommonConverters.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/diagnose_ex.hxx>

#include <algorithm>
#include <cstddef>

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

#ifdef DEBUG_CHART2_TEMPLATE
namespace
{
void lcl_ShowDataSource( const Reference< data::XDataSource > & xSource );
}
#endif

namespace chart
{

DataInterpreter::DataInterpreter()
{}

DataInterpreter::~DataInterpreter()
{}

// ____ XDataInterpreter ____
InterpretedData DataInterpreter::interpretDataSource(
    const Reference< data::XDataSource >& xSource,
    const Sequence< beans::PropertyValue >& aArguments,
    const std::vector< rtl::Reference< DataSeries > >& aSeriesToReUse )
{
    if( ! xSource.is())
        return InterpretedData();

#ifdef DEBUG_CHART2_TEMPLATE
    lcl_ShowDataSource( xSource );
#endif

    std::vector< uno::Reference< chart2::data::XLabeledDataSequence > > aData = getDataSequences(xSource);

    uno::Reference< chart2::data::XLabeledDataSequence > xCategories;
    std::vector< uno::Reference< chart2::data::XLabeledDataSequence > > aSequencesVec;

    // check if we should use categories

    bool bHasCategories( HasCategories( aArguments, aData ));

    // parse data
    bool bCategoriesUsed = false;
    for( uno::Reference< chart2::data::XLabeledDataSequence > const & labeledData : aData )
    {
        try
        {
            if( bHasCategories && ! bCategoriesUsed )
            {
                xCategories = labeledData;
                if( xCategories.is())
                    SetRole( xCategories->getValues(), u"categories"_ustr);
                bCategoriesUsed = true;
            }
            else
            {
                aSequencesVec.push_back( labeledData );
                if( labeledData.is())
                    SetRole( labeledData->getValues(), u"values-y"_ustr);
            }
        }
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }

    // create DataSeries
    std::size_t nSeriesIndex = 0;
    std::vector< rtl::Reference< DataSeries > > aSeriesVec;
    aSeriesVec.reserve( aSequencesVec.size());

    for (auto const& elem : aSequencesVec)
    {
        std::vector< uno::Reference< chart2::data::XLabeledDataSequence > > aNewData { elem };
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

InterpretedData DataInterpreter::reinterpretDataSeries(
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
            std::vector< uno::Reference< data::XLabeledDataSequence > > aNewSequences;

            // values-y
            uno::Reference< data::XLabeledDataSequence > xValuesY =
                DataSeriesHelper::getDataSequenceByRole( aSeries[i], u"values-y"_ustr );
            // re-use values-... as values-y
            if( ! xValuesY.is())
            {
                xValuesY =
                    DataSeriesHelper::getDataSequenceByRole( aSeries[i], u"values"_ustr, true );
                if( xValuesY.is())
                    SetRole( xValuesY->getValues(), u"values-y"_ustr);
            }
            if( xValuesY.is())
            {
                aNewSequences = { xValuesY };
            }

            const std::vector< uno::Reference< data::XLabeledDataSequence > > & aSeqs = aSeries[i]->getDataSequences2();
            if( aSeqs.size() != aNewSequences.size() )
            {
#ifdef DEBUG_CHART2_TEMPLATE
                sal_Int32 j=0;
                for( ; j<aSeqs.(); ++j )
                {
                    assert( aSeqs[j] == xValuesY && "All sequences should be used" );
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

// criterion: all series must have exactly one data::XLabeledDataSequence
bool DataInterpreter::isDataCompatible(
    const InterpretedData& aInterpretedData )
{
    const std::vector< rtl::Reference< DataSeries > > aSeries( FlattenSequence( aInterpretedData.Series ));
    for( rtl::Reference< DataSeries > const & i : aSeries )
    {
        try
        {
            if( i->getDataSequences2().size() != 1 )
                return false;
        }
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }

    return true;
}

namespace
{

struct lcl_LabeledSequenceEquals
{
    explicit lcl_LabeledSequenceEquals( const Reference< data::XLabeledDataSequence > & xLSeqToCmp ) :
            m_bHasLabels ( false ),
            m_bHasValues ( false )
    {
        if( !xLSeqToCmp.is())
            return;

        Reference< data::XDataSequence > xSeq( xLSeqToCmp->getValues());
        if( xSeq.is())
        {
            m_bHasValues = true;
            m_aValuesRangeRep = xSeq->getSourceRangeRepresentation();
        }

        xSeq.set( xLSeqToCmp->getLabel());
        if( xSeq.is())
        {
            m_bHasLabels = true;
            m_aLabelRangeRep = xSeq->getSourceRangeRepresentation();
        }
    }

    bool operator() ( const Reference< data::XLabeledDataSequence > & xSeq )
    {
        if( ! xSeq.is())
            return false;

        Reference< data::XDataSequence > xSeqValues( xSeq->getValues() );
        Reference< data::XDataSequence > xSeqLabels( xSeq->getLabel() );
        bool bHasValues = xSeqValues.is();
        bool bHasLabels = xSeqLabels.is();

        return ( ( (m_bHasValues == bHasValues) &&
                   (!bHasValues || m_aValuesRangeRep == xSeqValues->getSourceRangeRepresentation()) ) &&
                 ( (m_bHasLabels == bHasLabels) &&
                   (!bHasLabels || m_aLabelRangeRep == xSeqLabels->getSourceRangeRepresentation()) )
            );
    }

private:
    bool m_bHasLabels;
    bool m_bHasValues;
    OUString m_aValuesRangeRep;
    OUString m_aLabelRangeRep;
};

} // anonymous namespace

rtl::Reference< DataSource > DataInterpreter::mergeInterpretedData(
    const InterpretedData& aInterpretedData )
{
    std::vector< Reference< data::XLabeledDataSequence > > aResultVec;
    aResultVec.reserve( aInterpretedData.Series.size() +
                        1 // categories
        );

    if( aInterpretedData.Categories.is())
        aResultVec.push_back( aInterpretedData.Categories );

    const std::vector< rtl::Reference< DataSeries > > aSeries = FlattenSequence( aInterpretedData.Series );
    for( rtl::Reference< DataSeries > const & dataSeries : aSeries )
    {
        try
        {
            // add all sequences of data series
            for( uno::Reference< data::XLabeledDataSequence > const & xAdd : dataSeries->getDataSequences2() )
            {
                // only add if sequence is not yet in the result
                if( none_of( aResultVec.begin(), aResultVec.end(),
                             lcl_LabeledSequenceEquals( xAdd )) )
                {
                    aResultVec.push_back( xAdd );
                }
            }
        }
        catch( const uno::Exception & )
        {
            DBG_UNHANDLED_EXCEPTION("chart2");
        }
    }

    return new DataSource(aResultVec);
}

uno::Any DataInterpreter::getChartTypeSpecificData(
    const OUString & )
{
    return uno::Any();
}

// convenience methods

OUString DataInterpreter::GetRole( const Reference< data::XDataSequence > & xSeq )
{
    OUString aResult;
    if( ! xSeq.is())
        return aResult;

    try
    {
        Reference< beans::XPropertySet > xProp( xSeq, uno::UNO_QUERY_THROW );
        xProp->getPropertyValue( u"Role"_ustr) >>= aResult;
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
    return aResult;
}

void DataInterpreter::SetRole( const Reference< data::XDataSequence > & xSeq, const OUString & rRole )
{
    if( ! xSeq.is())
        return;
    try
    {
        Reference< beans::XPropertySet > xProp( xSeq, uno::UNO_QUERY_THROW );
        xProp->setPropertyValue( u"Role"_ustr, uno::Any( rRole ));
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

uno::Any DataInterpreter::GetProperty(
    const Sequence< beans::PropertyValue > & aArguments,
    std::u16string_view rName )
{
    for( sal_Int32 i=aArguments.getLength(); i--; )
    {
        if( aArguments[i].Name == rName )
            return aArguments[i].Value;
    }
    return uno::Any();
}

bool DataInterpreter::HasCategories(
    const Sequence< beans::PropertyValue > & rArguments,
    const std::vector< uno::Reference< chart2::data::XLabeledDataSequence > > & rData )
{
    bool bHasCategories = false;

    if( rArguments.hasElements() )
        GetProperty( rArguments, u"HasCategories" ) >>= bHasCategories;

    for( std::size_t nLSeqIdx=0; ! bHasCategories && nLSeqIdx<rData.size(); ++nLSeqIdx )
        bHasCategories = ( rData[nLSeqIdx].is() && GetRole( rData[nLSeqIdx]->getValues() ) == "categories");

    return bHasCategories;
}

bool DataInterpreter::UseCategoriesAsX( const Sequence< beans::PropertyValue > & rArguments )
{
    bool bUseCategoriesAsX = true;
    if( rArguments.hasElements() )
        GetProperty( rArguments, u"UseCategoriesAsX" ) >>= bUseCategoriesAsX;
    return bUseCategoriesAsX;
}

OUString SAL_CALL DataInterpreter::getImplementationName()
{
    return u"com.sun.star.comp.chart2.DataInterpreter"_ustr;
}

sal_Bool SAL_CALL DataInterpreter::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence< OUString > SAL_CALL DataInterpreter::getSupportedServiceNames()
{
    return { u"com.sun.star.chart2.DataInterpreter"_ustr };
}

std::vector< uno::Reference< chart2::data::XLabeledDataSequence > > DataInterpreter::getDataSequences(
        const css::uno::Reference< css::chart2::data::XDataSource >& xSource)
{
    std::vector< uno::Reference< chart2::data::XLabeledDataSequence > > aData;
    for (const Reference< data::XLabeledDataSequence > & rLDS : xSource->getDataSequences() )
    {
        aData.push_back(rLDS);
    }
    return aData;
}

} // namespace chart

#ifdef DEBUG_CHART2_TEMPLATE
namespace
{

void lcl_ShowDataSource( const Reference< data::XDataSource > & xSource )
{
    if( ! xSource.is())
        return;

    SAL_INFO("chart2", "DataSource in DataInterpreter:" );
    Sequence< Reference< data::XLabeledDataSequence > > aSequences( xSource->getDataSequences());
    Reference< beans::XPropertySet > xProp;
    OUString aId;
    const sal_Int32 nMax = aSequences.getLength();
    for( sal_Int32 k = 0; k < nMax; ++k )
    {
        if( aSequences[k].is())
        {
            OUString aSourceRepr("<none>");
            if( aSequences[k]->getValues().is())
                aSourceRepr = aSequences[k]->getValues()->getSourceRangeRepresentation();
            xProp.set( aSequences[k]->getValues(), uno::UNO_QUERY );
            if( xProp.is() &&
                ( xProp->getPropertyValue( "Role") >>= aId ))
            {
                SAL_INFO("chart2", "  <data sequence " << k << "> Role: " << aId << ", Source: "<< aSourceRepr);
            }
            else
            {
                SAL_INFO("chart2", "  <data sequence " << k << "> unknown Role, Source: " << aSourceRepr );
            }

            aSourceRepr = "<none>";
            if( aSequences[k]->getLabel().is())
                aSourceRepr = aSequences[k]->getLabel()->getSourceRangeRepresentation();
            xProp.set( aSequences[k]->getLabel(), uno::UNO_QUERY );
            if( xProp.is() &&
                ( xProp->getPropertyValue( "Role") >>= aId ))
            {
                SAL_INFO("chart2", "  <data sequence label " << k << "> Role: " << aId
                        << ", Source: " << aSourceRepr );
            }
            else
            {
                SAL_INFO("chart2", "  <data sequence label " << k << "> unknown Role, Source: " << aSourceRepr );
            }
        }
    }
}

}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
