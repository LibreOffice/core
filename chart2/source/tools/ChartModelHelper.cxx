/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "ChartModelHelper.hxx"
#include "macros.hxx"
#include "DiagramHelper.hxx"
#include "DataSourceHelper.hxx"
#include "ControllerLockGuard.hxx"
#include "RangeHighlighter.hxx"
#include "InternalDataProvider.hxx"

#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/XVisualObject.hpp>
#include <com/sun/star/view/XSelectionChangeListener.hpp>

// header for define DBG_ASSERT
#include <tools/debug.hxx>

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

uno::Reference< chart2::data::XRangeHighlighter > ChartModelHelper::createRangeHighlighter(
        const uno::Reference< view::XSelectionSupplier > & xSelectionSupplier )
{
    return new RangeHighlighter( xSelectionSupplier );
}

uno::Reference< chart2::data::XDataProvider > ChartModelHelper::createInternalDataProvider(
    const uno::Reference< ::com::sun::star::chart2::XChartDocument >& xChartDoc, bool bConnectToModel )
{
    return new InternalDataProvider( xChartDoc, bConnectToModel );
}

uno::Reference< XDiagram > ChartModelHelper::findDiagram( const uno::Reference< frame::XModel >& xModel )
{
    uno::Reference< XChartDocument > xChartDoc( xModel, uno::UNO_QUERY );
    if( xChartDoc.is())
        return ChartModelHelper::findDiagram( xChartDoc );
    return NULL;
}

uno::Reference< XDiagram > ChartModelHelper::findDiagram( const uno::Reference< chart2::XChartDocument >& xChartDoc )
{
    try
    {
        if( xChartDoc.is())
            return xChartDoc->getFirstDiagram();
    }
    catch( uno::Exception & ex )
    {
        ASSERT_EXCEPTION( ex );
    }
    return NULL;
}

uno::Reference< XCoordinateSystem > ChartModelHelper::getFirstCoordinateSystem( const uno::Reference< frame::XModel >& xModel )
{
    uno::Reference< XCoordinateSystem > XCooSys;
    uno::Reference< XCoordinateSystemContainer > xCooSysCnt( ChartModelHelper::findDiagram( xModel ), uno::UNO_QUERY );
    if( xCooSysCnt.is() )
    {
        uno::Sequence< uno::Reference< XCoordinateSystem > > aCooSysSeq( xCooSysCnt->getCoordinateSystems() );
        if( aCooSysSeq.getLength() )
            XCooSys = aCooSysSeq[0];
    }
    return XCooSys;
}

::std::vector< uno::Reference< XDataSeries > > ChartModelHelper::getDataSeries(
    const uno::Reference< XChartDocument > & xChartDoc )
{
    ::std::vector< uno::Reference< XDataSeries > > aResult;

    uno::Reference< XDiagram > xDiagram = ChartModelHelper::findDiagram( xChartDoc );
    if( xDiagram.is())
        aResult = DiagramHelper::getDataSeriesFromDiagram( xDiagram );

    return aResult;
}

::std::vector< uno::Reference< XDataSeries > > ChartModelHelper::getDataSeries(
    const uno::Reference< frame::XModel > & xModel )
{
    return getDataSeries( uno::Reference< chart2::XChartDocument >( xModel, uno::UNO_QUERY ));
}


uno::Reference< XChartType > ChartModelHelper::getChartTypeOfSeries(
                                const uno::Reference< frame::XModel >& xModel
                              , const uno::Reference< XDataSeries >&   xGivenDataSeries )
{
    return DiagramHelper::getChartTypeOfSeries( ChartModelHelper::findDiagram( xModel ), xGivenDataSeries );
}

awt::Size ChartModelHelper::getDefaultPageSize()
{
    return awt::Size( 16000, 9000 );
}

awt::Size ChartModelHelper::getPageSize( const uno::Reference< frame::XModel >& xModel )
{
    awt::Size aPageSize( ChartModelHelper::getDefaultPageSize() );
    uno::Reference< embed::XVisualObject > xVisualObject(xModel,uno::UNO_QUERY);
    DBG_ASSERT(xVisualObject.is(),"need xVisualObject for page size");
    if( xVisualObject.is() )
        aPageSize = xVisualObject->getVisualAreaSize( embed::Aspects::MSOLE_CONTENT );
    return aPageSize;
}

void ChartModelHelper::setPageSize( const awt::Size& rSize, const uno::Reference< frame::XModel >& xModel )
{
    uno::Reference< embed::XVisualObject > xVisualObject(xModel,uno::UNO_QUERY);
    DBG_ASSERT(xVisualObject.is(),"need xVisualObject for page size");
    if( xVisualObject.is() )
        xVisualObject->setVisualAreaSize( embed::Aspects::MSOLE_CONTENT, rSize );
}

void ChartModelHelper::triggerRangeHighlighting( const uno::Reference< frame::XModel >& xModel )
{
    uno::Reference< chart2::data::XDataReceiver > xDataReceiver( xModel, uno::UNO_QUERY );
    if( xDataReceiver.is() )
    {
        uno::Reference< view::XSelectionChangeListener > xSelectionChangeListener( xDataReceiver->getRangeHighlighter(), uno::UNO_QUERY );
        //trigger selection of cell range
        if( xSelectionChangeListener.is() )
        {
            lang::EventObject aEvent( xSelectionChangeListener );
            xSelectionChangeListener->selectionChanged( aEvent );
        }
    }
}

bool ChartModelHelper::isIncludeHiddenCells( const uno::Reference< frame::XModel >& xChartModel )
{
    bool bIncluded = true;  // hidden cells are included by default.

    uno::Reference< chart2::XDiagram > xDiagram( ChartModelHelper::findDiagram(xChartModel) );
    if (!xDiagram.is())
        return bIncluded;

    uno::Reference< beans::XPropertySet > xProp( xDiagram, uno::UNO_QUERY );
    if (!xProp.is())
        return bIncluded;

    try
    {
        xProp->getPropertyValue(C2U("IncludeHiddenCells")) >>= bIncluded;
    }
    catch( const beans::UnknownPropertyException& )
    {
    }

    return bIncluded;
}

bool ChartModelHelper::setIncludeHiddenCells( bool bIncludeHiddenCells, const uno::Reference< frame::XModel >& xChartModel )
{
    bool bChanged = false;
    try
    {
        ControllerLockGuard aLockedControllers( xChartModel );

        uno::Reference< beans::XPropertySet > xDiagramProperties( ChartModelHelper::findDiagram(xChartModel), uno::UNO_QUERY );
        if (xDiagramProperties.is())
        {
            bool bOldValue = bIncludeHiddenCells;
            xDiagramProperties->getPropertyValue( C2U("IncludeHiddenCells") ) >>= bOldValue;
            if( bOldValue == bIncludeHiddenCells )
                bChanged = true;

            //set the property on all instances in all cases to get the different objects in sync!

            uno::Any aNewValue = uno::makeAny(bIncludeHiddenCells);

            try
            {
                uno::Reference< chart2::XChartDocument > xChartDoc( xChartModel, uno::UNO_QUERY );
                if( xChartDoc.is() )
                {
                    uno::Reference< beans::XPropertySet > xDataProviderProperties( xChartDoc->getDataProvider(), uno::UNO_QUERY );
                    if( xDataProviderProperties.is() )
                        xDataProviderProperties->setPropertyValue(C2U("IncludeHiddenCells"), aNewValue );
                }
            }
            catch( const beans::UnknownPropertyException& )
            {
                //the property is optional!
            }

            try
            {
                uno::Reference< chart2::data::XDataSource > xUsedData( DataSourceHelper::getUsedData( xChartModel ) );
                if( xUsedData.is() )
                {
                    uno::Reference< beans::XPropertySet > xProp;
                    uno::Sequence< uno::Reference< chart2::data::XLabeledDataSequence > > aData( xUsedData->getDataSequences());
                    for( sal_Int32 i=0; i<aData.getLength(); ++i )
                    {
                        xProp.set( uno::Reference< beans::XPropertySet >( aData[i]->getValues(), uno::UNO_QUERY ) );
                        if(xProp.is())
                            xProp->setPropertyValue(C2U("IncludeHiddenCells"), aNewValue );
                        xProp.set( uno::Reference< beans::XPropertySet >( aData[i]->getLabel(), uno::UNO_QUERY ) );
                        if(xProp.is())
                            xProp->setPropertyValue(C2U("IncludeHiddenCells"), aNewValue );
                    }
                }
            }
            catch( const beans::UnknownPropertyException& )
            {
                //the property is optional!
            }

            xDiagramProperties->setPropertyValue( C2U("IncludeHiddenCells"), aNewValue);
        }
    }
    catch (uno::Exception& e)
    {
        ASSERT_EXCEPTION(e);
    }
    return bChanged;
}

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
