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

#include <DataSourceHelper.hxx>
#include <ChartModel.hxx>
#include <ChartModelHelper.hxx>
#include <ChartTypeManager.hxx>
#include <Diagram.hxx>
#include <DataSeries.hxx>
#include <DataSeriesHelper.hxx>
#include <DataSource.hxx>
#include <ControllerLockGuard.hxx>
#include <CachedDataSequence.hxx>
#include <LabeledDataSequence.hxx>
#include <unonames.hxx>

#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/data/XLabeledDataSequence.hpp>

#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart/ErrorBarStyle.hpp>
#include <comphelper/diagnose_ex.hxx>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace
{
void lcl_addRanges( std::vector< OUString > & rOutResult,
                    const uno::Reference< data::XLabeledDataSequence > & xLabeledSeq )
{
    if( ! xLabeledSeq.is())
        return;
    uno::Reference< data::XDataSequence > xSeq( xLabeledSeq->getLabel());
    if( xSeq.is())
        rOutResult.push_back( xSeq->getSourceRangeRepresentation());
    xSeq.set( xLabeledSeq->getValues());
    if( xSeq.is())
        rOutResult.push_back( xSeq->getSourceRangeRepresentation());
}

void lcl_addDataSourceRanges(
    std::vector< OUString > & rOutResult,
    const uno::Reference< data::XDataSource > & xDataSource )
{
    if( xDataSource.is() )
    {
        const auto aDataSequences(xDataSource->getDataSequences());
        for (const auto& rDataSequence : aDataSequences)
            lcl_addRanges(rOutResult, rDataSequence);
    }
}

void lcl_addErrorBarRanges(
    std::vector< OUString > & rOutResult,
    const rtl::Reference< DataSeries > & xDataSeries )
{
    if( !xDataSeries.is())
        return;

    try
    {
        uno::Reference< beans::XPropertySet > xErrorBarProp;
        if( ( xDataSeries->getPropertyValue( CHART_UNONAME_ERRORBAR_Y) >>= xErrorBarProp ) &&
            xErrorBarProp.is())
        {
            sal_Int32 eStyle = css::chart::ErrorBarStyle::NONE;
            if( ( xErrorBarProp->getPropertyValue( u"ErrorBarStyle"_ustr) >>= eStyle ) &&
                eStyle == css::chart::ErrorBarStyle::FROM_DATA )
            {
                uno::Reference< data::XDataSource > xErrorBarDataSource( xErrorBarProp, uno::UNO_QUERY );
                if( xErrorBarDataSource.is() )
                    lcl_addDataSourceRanges( rOutResult, xErrorBarDataSource );
            }
        }

        if( ( xDataSeries->getPropertyValue(CHART_UNONAME_ERRORBAR_X) >>= xErrorBarProp ) && xErrorBarProp.is())
        {
            sal_Int32 eStyle = css::chart::ErrorBarStyle::NONE;
            if( ( xErrorBarProp->getPropertyValue(u"ErrorBarStyle"_ustr) >>= eStyle ) &&
                eStyle == css::chart::ErrorBarStyle::FROM_DATA )
            {
                uno::Reference< data::XDataSource > xErrorBarDataSource( xErrorBarProp, uno::UNO_QUERY );
                if( xErrorBarDataSource.is() )
                    lcl_addDataSourceRanges( rOutResult, xErrorBarDataSource );
            }
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
}

} // anonymous namespace

Reference< chart2::data::XDataSequence > DataSourceHelper::createCachedDataSequence()
{
    return new ::chart::CachedDataSequence();
}

Reference< chart2::data::XDataSequence > DataSourceHelper::createCachedDataSequence( const OUString& rSingleText )
{
    return new ::chart::CachedDataSequence( rSingleText );
}

rtl::Reference< LabeledDataSequence > DataSourceHelper::createLabeledDataSequence(
        const Reference< chart2::data::XDataSequence >& xValues ,
        const Reference< chart2::data::XDataSequence >& xLabels )
{
    return new ::chart::LabeledDataSequence( xValues, xLabels );
}

rtl::Reference< LabeledDataSequence > DataSourceHelper::createLabeledDataSequence(
        const Reference< chart2::data::XDataSequence >& xValues )
{
    return new ::chart::LabeledDataSequence( xValues );
}

rtl::Reference< LabeledDataSequence > DataSourceHelper::createLabeledDataSequence()
{
    return new ::chart::LabeledDataSequence;
}

uno::Sequence< beans::PropertyValue > DataSourceHelper::createArguments(
                                            bool bUseColumns, bool bFirstCellAsLabel, bool bHasCategories )
{
    css::chart::ChartDataRowSource eRowSource = css::chart::ChartDataRowSource_ROWS;
    if( bUseColumns )
        eRowSource = css::chart::ChartDataRowSource_COLUMNS;

    return
    {
        { u"DataRowSource"_ustr, -1, uno::Any( eRowSource), beans::PropertyState_DIRECT_VALUE },
        { u"FirstCellAsLabel"_ustr, -1, uno::Any( bFirstCellAsLabel ), beans::PropertyState_DIRECT_VALUE },
        { u"HasCategories"_ustr, -1, uno::Any( bHasCategories ), beans::PropertyState_DIRECT_VALUE }
    };
}

uno::Sequence< beans::PropertyValue > DataSourceHelper::createArguments(
                                            const OUString & rRangeRepresentation,
                                            const uno::Sequence< sal_Int32 >& rSequenceMapping,
                                            bool bUseColumns, bool bFirstCellAsLabel, bool bHasCategories )
{
    uno::Sequence< beans::PropertyValue > aArguments( createArguments( bUseColumns, bFirstCellAsLabel, bHasCategories ));
    aArguments.realloc( aArguments.getLength() + 1 );
    aArguments.getArray()[aArguments.getLength() - 1] =
        beans::PropertyValue( u"CellRangeRepresentation"_ustr
                              , -1, uno::Any( rRangeRepresentation )
                              , beans::PropertyState_DIRECT_VALUE );
    if( rSequenceMapping.hasElements() )
    {
        aArguments.realloc( aArguments.getLength() + 1 );
        aArguments.getArray()[aArguments.getLength() - 1] =
            beans::PropertyValue( u"SequenceMapping"_ustr
                                , -1, uno::Any( rSequenceMapping )
                                , beans::PropertyState_DIRECT_VALUE );
    }
    return aArguments;
}

void DataSourceHelper::readArguments( const uno::Sequence< beans::PropertyValue >& rArguments
                                     , OUString & rRangeRepresentation, uno::Sequence< sal_Int32 >& rSequenceMapping
            , bool& bUseColumns, bool& bFirstCellAsLabel, bool& bHasCategories )
{
    for(const beans::PropertyValue& rProperty : rArguments)
    {
        if ( rProperty.Name == "DataRowSource" )
        {
            css::chart::ChartDataRowSource eRowSource;
            if( rProperty.Value >>= eRowSource )
                bUseColumns = (eRowSource==css::chart::ChartDataRowSource_COLUMNS);
        }
        else if ( rProperty.Name == "FirstCellAsLabel" )
        {
            rProperty.Value >>= bFirstCellAsLabel;
        }
        else if ( rProperty.Name == "HasCategories" )
        {
            rProperty.Value >>= bHasCategories;
        }
        else if ( rProperty.Name == "CellRangeRepresentation" )
        {
            rProperty.Value >>= rRangeRepresentation;
        }
        else if ( rProperty.Name == "SequenceMapping" )
        {
            rProperty.Value >>= rSequenceMapping;
        }
    }
}

rtl::Reference< DataSource > DataSourceHelper::pressUsedDataIntoRectangularFormat(
        const rtl::Reference< ChartModel >& xChartDoc )
{
    std::vector< Reference< chart2::data::XLabeledDataSequence > > aResultVector;

    //categories are always the first sequence
    rtl::Reference< Diagram > xDiagram( xChartDoc->getFirstChartDiagram());

    Reference< chart2::data::XLabeledDataSequence > xCategories;
    if (xDiagram)
        xCategories = xDiagram->getCategories();
    if( xCategories.is() )
        aResultVector.push_back( xCategories );

    std::vector< rtl::Reference< DataSeries > > aSeriesVector;
    if (xDiagram)
        aSeriesVector = xDiagram->getDataSeries();
    uno::Reference< chart2::data::XDataSource > xSeriesSource =
        DataSeriesHelper::getDataSource( aSeriesVector );
    const Sequence< Reference< chart2::data::XLabeledDataSequence > > aDataSequences( xSeriesSource->getDataSequences() );

    //the first x-values is always the next sequence //todo ... other x-values get lost for old format
    Reference< chart2::data::XLabeledDataSequence > xXValues(
        DataSeriesHelper::getDataSequenceByRole( xSeriesSource, u"values-x"_ustr ) );
    if( xXValues.is() )
        aResultVector.push_back( xXValues );

    //add all other sequences now without x-values
    for(  Reference< chart2::data::XLabeledDataSequence > const & labeledData : aDataSequences )
    {
        OUString aRole = DataSeriesHelper::getRole(labeledData);
        if( aRole != "values-x" )
            aResultVector.push_back( labeledData );
    }

    return new DataSource( aResultVector );
}

uno::Sequence< OUString > DataSourceHelper::getUsedDataRanges(
    const rtl::Reference< Diagram > & xDiagram )
{
    std::vector< OUString > aResult;

    if( xDiagram.is())
    {
        uno::Reference< data::XLabeledDataSequence > xCategories( xDiagram->getCategories() );
        if( xCategories.is() )
            lcl_addRanges( aResult, xCategories );

        std::vector< rtl::Reference< DataSeries > > aSeriesVector( xDiagram->getDataSeries() );
        for (auto const& series : aSeriesVector)
        {
            lcl_addDataSourceRanges( aResult, series );
            lcl_addErrorBarRanges( aResult, series );
        }
    }

    return comphelper::containerToSequence( aResult );
}

uno::Sequence< OUString > DataSourceHelper::getUsedDataRanges( const rtl::Reference<::chart::ChartModel> & xChartModel )
{
    rtl::Reference< Diagram > xDiagram( xChartModel->getFirstChartDiagram() );
    return getUsedDataRanges( xDiagram );
}

rtl::Reference< DataSource > DataSourceHelper::getUsedData(
    ChartModel& rModel )
{
    std::vector< uno::Reference< chart2::data::XLabeledDataSequence > > aResult;

    rtl::Reference< Diagram > xDiagram =  rModel.getFirstChartDiagram();
    uno::Reference< chart2::data::XLabeledDataSequence > xCategories( xDiagram->getCategories() );
    if( xCategories.is() )
        aResult.push_back( xCategories );

    std::vector< rtl::Reference< DataSeries > > aSeriesVector = ChartModelHelper::getDataSeries( &rModel );
    for (auto const& series : aSeriesVector)
    {
        const std::vector< uno::Reference< chart2::data::XLabeledDataSequence > > & aDataSequences( series->getDataSequences2() );
        aResult.insert( aResult.end(), aDataSequences.begin(), aDataSequences.end() );
    }

    return new DataSource( aResult );
}

bool DataSourceHelper::detectRangeSegmentation(
    const rtl::Reference<::chart::ChartModel>& xChartModel
    , OUString& rOutRangeString
    , css::uno::Sequence< sal_Int32 >& rSequenceMapping
    , bool& rOutUseColumns
    , bool& rOutFirstCellAsLabel
    , bool& rOutHasCategories )
{
    bool bSomethingDetected = false;

    if( !xChartModel.is() )
        return bSomethingDetected;
    uno::Reference< data::XDataProvider >  xDataProvider( xChartModel->getDataProvider() );
    if( !xDataProvider.is() )
        return bSomethingDetected;

    try
    {
        DataSourceHelper::readArguments(
            xDataProvider->detectArguments( pressUsedDataIntoRectangularFormat( xChartModel ) ),
            rOutRangeString, rSequenceMapping, rOutUseColumns, rOutFirstCellAsLabel, rOutHasCategories );
        bSomethingDetected = !rOutRangeString.isEmpty();

        rtl::Reference<Diagram> xDiagram = xChartModel->getFirstChartDiagram();
        uno::Reference< chart2::data::XLabeledDataSequence > xCategories;
        if (xDiagram)
            xCategories = xDiagram->getCategories();
        rOutHasCategories = xCategories.is();
    }
    catch( uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }
    return bSomethingDetected;
}

bool DataSourceHelper::allArgumentsForRectRangeDetected(
    const rtl::Reference<::chart::ChartModel>& xChartDocument )
{
    bool bHasDataRowSource = false;
    bool bHasFirstCellAsLabel = false;
    bool bHasCellRangeRepresentation = false;

    uno::Reference< data::XDataProvider > xDataProvider( xChartDocument->getDataProvider() );
    if( !xDataProvider.is() )
        return false;

    try
    {
        const uno::Sequence< beans::PropertyValue > aArguments(
            xDataProvider->detectArguments( pressUsedDataIntoRectangularFormat( xChartDocument )));
        for(const beans::PropertyValue& rProperty : aArguments)
        {
            if ( rProperty.Name == "DataRowSource" )
            {
                bHasDataRowSource =
                    (rProperty.Value.hasValue() && rProperty.Value.isExtractableTo(
                        cppu::UnoType<css::chart::ChartDataRowSource>::get()));
            }
            else if ( rProperty.Name == "FirstCellAsLabel" )
            {
                bHasFirstCellAsLabel =
                    (rProperty.Value.hasValue() && rProperty.Value.isExtractableTo(cppu::UnoType<bool>::get()));
            }
            else if ( rProperty.Name == "CellRangeRepresentation" )
            {
                OUString aRange;
                bHasCellRangeRepresentation =
                    (rProperty.Value.hasValue() && (rProperty.Value >>= aRange) && !aRange.isEmpty());
            }
        }
    }
    catch( const uno::Exception & )
    {
        DBG_UNHANDLED_EXCEPTION("chart2");
    }

    return (bHasCellRangeRepresentation && bHasDataRowSource && bHasFirstCellAsLabel);
}

void DataSourceHelper::setRangeSegmentation(
            const rtl::Reference<::chart::ChartModel>& xChartModel
            , const css::uno::Sequence< sal_Int32 >& rSequenceMapping
            , bool bUseColumns , bool bFirstCellAsLabel, bool bUseCategories )
{
    uno::Reference< data::XDataProvider > xDataProvider( xChartModel->getDataProvider() );
    if( !xDataProvider.is() )
        return;
    rtl::Reference< Diagram > xDiagram( xChartModel->getFirstChartDiagram() );
    if( !xDiagram.is() )
        return;
    rtl::Reference< ::chart::ChartTypeManager > xChartTypeManager = xChartModel->getTypeManager();
    if( !xChartTypeManager.is() )
        return;

    OUString aRangeString;
    bool bDummy;
    uno::Sequence< sal_Int32 > aDummy;
    readArguments( xDataProvider->detectArguments( pressUsedDataIntoRectangularFormat( xChartModel )),
                   aRangeString, aDummy, bDummy, bDummy, bDummy );

    uno::Sequence< beans::PropertyValue > aArguments(
        createArguments( aRangeString, rSequenceMapping, bUseColumns, bFirstCellAsLabel, bUseCategories ) );

    uno::Reference< chart2::data::XDataSource > xDataSource( xDataProvider->createDataSource(
                                                                 aArguments ) );
    if( !xDataSource.is() )
        return;

    ControllerLockGuardUNO aCtrlLockGuard( xChartModel );
    xDiagram->setDiagramData( xDataSource, aArguments );
}

Sequence< OUString > DataSourceHelper::getRangesFromLabeledDataSequence(
    const Reference< data::XLabeledDataSequence > & xLSeq )
{
    Sequence< OUString > aResult;
    if( xLSeq.is())
    {
        Reference< data::XDataSequence > xLabel( xLSeq->getLabel());
        Reference< data::XDataSequence > xValues( xLSeq->getValues());

        if( xLabel.is())
        {
            if( xValues.is())
            {
                aResult = { xLabel->getSourceRangeRepresentation(),
                            xValues->getSourceRangeRepresentation() };
            }
            else
            {
                aResult = { xLabel->getSourceRangeRepresentation() };
            }
        }
        else if( xValues.is())
        {
            aResult = { xValues->getSourceRangeRepresentation() };
        }
    }
    return aResult;
}

OUString DataSourceHelper::getRangeFromValues(
    const Reference< data::XLabeledDataSequence > & xLSeq )
{
    OUString aResult;
    if( xLSeq.is() )
    {
        Reference< data::XDataSequence > xValues( xLSeq->getValues() );
        if( xValues.is() )
            aResult = xValues->getSourceRangeRepresentation();
    }
    return aResult;
}

Sequence< OUString > DataSourceHelper::getRangesFromDataSource( const Reference< data::XDataSource > & xSource )
{
    std::vector< OUString > aResult;
    if( xSource.is())
    {
        const Sequence< Reference< data::XLabeledDataSequence > > aLSeqSeq( xSource->getDataSequences());
        for( Reference< data::XLabeledDataSequence > const & labeledData : aLSeqSeq )
        {
            Reference< data::XDataSequence > xLabel( labeledData->getLabel());
            Reference< data::XDataSequence > xValues( labeledData->getValues());

            if( xLabel.is())
                aResult.push_back( xLabel->getSourceRangeRepresentation());
            if( xValues.is())
                aResult.push_back( xValues->getSourceRangeRepresentation());
        }
    }
    return comphelper::containerToSequence( aResult );
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
