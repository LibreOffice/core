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

#include <ChartModelHelper.hxx>
#include <Diagram.hxx>
#include <DataSource.hxx>
#include <DataSourceHelper.hxx>
#include <ControllerLockGuard.hxx>
#include <InternalDataProvider.hxx>
#include <ChartModel.hxx>
#include <BaseCoordinateSystem.hxx>
#include <ChartType.hxx>
#include <DataSeries.hxx>

#include <com/sun/star/chart/ChartDataRowSource.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#include <comphelper/diagnose_ex.hxx>

namespace chart
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

rtl::Reference< InternalDataProvider > ChartModelHelper::createInternalDataProvider(
    const rtl::Reference<::chart::ChartModel>& xChartDoc, bool bConnectToModel )
{
    bool bDefaultDataInColumns(true);

    // #i120559# Try to access the current state of "DataRowSource" for the
    // chart data and use it as default for creating a new InternalDataProvider
    if(xChartDoc.is())
    {
        // old XChartDocument interface
        css::uno::Reference< css::chart::XChartDocument > xDoc(static_cast<cppu::OWeakObject*>(xChartDoc.get()), uno::UNO_QUERY);

        if(xDoc.is())
        {
            css::uno::Reference< css::chart::XDiagram > aDiagram = xDoc->getDiagram();

            if(aDiagram.is())
            {
                css::uno::Reference< css::beans::XPropertySet > xProp(aDiagram, uno::UNO_QUERY);

                if(xProp.is())
                {
                    css::chart::ChartDataRowSource aDataRowSource(css::chart::ChartDataRowSource_COLUMNS);

                    xProp->getPropertyValue( u"DataRowSource"_ustr ) >>= aDataRowSource;

                    bDefaultDataInColumns = (aDataRowSource == css::chart::ChartDataRowSource_COLUMNS);
                }
            }
        }
    }

    return new InternalDataProvider( xChartDoc, bConnectToModel, bDefaultDataInColumns );
}

rtl::Reference< BaseCoordinateSystem > ChartModelHelper::getFirstCoordinateSystem( const rtl::Reference<::chart::ChartModel>& xModel )
{
    rtl::Reference< Diagram > xDiagram = xModel->getFirstChartDiagram();
    if( xDiagram.is() )
    {
        auto aCooSysSeq( xDiagram->getBaseCoordinateSystems() );
        if( !aCooSysSeq.empty() )
            return aCooSysSeq[0];
    }
    return nullptr;
}

std::vector< rtl::Reference< DataSeries > > ChartModelHelper::getDataSeries(
    const rtl::Reference<::chart::ChartModel> & xChartDoc )
{
    std::vector< rtl::Reference< DataSeries > > aResult;

    rtl::Reference< Diagram > xDiagram = xChartDoc->getFirstChartDiagram();
    if( xDiagram.is())
        aResult = xDiagram->getDataSeries();

    return aResult;
}

rtl::Reference< ChartType > ChartModelHelper::getChartTypeOfSeries(
                                const rtl::Reference<::chart::ChartModel>& xModel
                              , const rtl::Reference< DataSeries >&   xGivenDataSeries )
{
    rtl::Reference<Diagram> xDiagram = xModel->getFirstChartDiagram();
    return xDiagram ? xDiagram->getChartTypeOfSeries( xGivenDataSeries ) : nullptr;
}

awt::Size ChartModelHelper::getDefaultPageSize()
{
    return awt::Size( 16000, 9000 );
}

awt::Size ChartModelHelper::getPageSize( const rtl::Reference<::chart::ChartModel>& xModel )
{
    awt::Size aPageSize( ChartModelHelper::getDefaultPageSize() );
    OSL_ENSURE(xModel.is(),"need xVisualObject for page size");
    if( xModel.is() )
        aPageSize = xModel->getVisualAreaSize( embed::Aspects::MSOLE_CONTENT );
    return aPageSize;
}

void ChartModelHelper::triggerRangeHighlighting( const rtl::Reference<::chart::ChartModel>& xModel )
{
    if( xModel.is() )
    {
        uno::Reference< view::XSelectionChangeListener > xSelectionChangeListener( xModel->getRangeHighlighter(), uno::UNO_QUERY );
        //trigger selection of cell range
        if( xSelectionChangeListener.is() )
        {
            lang::EventObject aEvent( xSelectionChangeListener );
            xSelectionChangeListener->selectionChanged( aEvent );
        }
    }
}

bool ChartModelHelper::isIncludeHiddenCells( const rtl::Reference<::chart::ChartModel>& xChartModel )
{
    bool bIncluded = true;  // hidden cells are included by default.

    rtl::Reference< Diagram > xDiagram( xChartModel->getFirstChartDiagram() );
    if (!xDiagram.is())
        return bIncluded;

    try
    {
        xDiagram->getPropertyValue(u"IncludeHiddenCells"_ustr) >>= bIncluded;
    }
    catch( const beans::UnknownPropertyException& )
    {
    }

    return bIncluded;
}

bool ChartModelHelper::setIncludeHiddenCells( bool bIncludeHiddenCells, ChartModel& rModel )
{
    bool bChanged = false;
    try
    {
        ControllerLockGuard aLockedControllers( rModel );

        uno::Reference< beans::XPropertySet > xDiagramProperties( rModel.getFirstDiagram(), uno::UNO_QUERY );
        if (xDiagramProperties.is())
        {
            bool bOldValue = bIncludeHiddenCells;
            xDiagramProperties->getPropertyValue( u"IncludeHiddenCells"_ustr ) >>= bOldValue;
            if( bOldValue == bIncludeHiddenCells )
                bChanged = true;

            //set the property on all instances in all cases to get the different objects in sync!

            uno::Any aNewValue(bIncludeHiddenCells);

            try
            {
                uno::Reference< beans::XPropertySet > xDataProviderProperties( rModel.getDataProvider(), uno::UNO_QUERY );
                if( xDataProviderProperties.is() )
                    xDataProviderProperties->setPropertyValue(u"IncludeHiddenCells"_ustr, aNewValue );
            }
            catch( const beans::UnknownPropertyException& )
            {
                //the property is optional!
            }

            try
            {
                rtl::Reference< DataSource > xUsedData = DataSourceHelper::getUsedData( rModel );
                if( xUsedData.is() )
                {
                    uno::Reference< beans::XPropertySet > xProp;
                    const uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > aData( xUsedData->getDataSequences());
                    for( uno::Reference< chart2::data::XLabeledDataSequence > const & labeledData : aData )
                    {
                        xProp.set( uno::Reference< beans::XPropertySet >( labeledData->getValues(), uno::UNO_QUERY ) );
                        if(xProp.is())
                            xProp->setPropertyValue(u"IncludeHiddenCells"_ustr, aNewValue );
                        xProp.set( uno::Reference< beans::XPropertySet >( labeledData->getLabel(), uno::UNO_QUERY ) );
                        if(xProp.is())
                            xProp->setPropertyValue(u"IncludeHiddenCells"_ustr, aNewValue );
                    }
                }
            }
            catch( const beans::UnknownPropertyException& )
            {
                //the property is optional!
            }

            xDiagramProperties->setPropertyValue( u"IncludeHiddenCells"_ustr, aNewValue);
        }
    }
    catch (const uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION("chart2", "" );
    }
    return bChanged;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
