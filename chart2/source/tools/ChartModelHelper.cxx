/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ChartModelHelper.cxx,v $
 * $Revision: 1.12 $
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

//static
uno::Reference< XDiagram > ChartModelHelper::findDiagram( const uno::Reference< frame::XModel >& xModel )
{
    uno::Reference< XChartDocument > xChartDoc( xModel, uno::UNO_QUERY );
    if( xChartDoc.is())
        return ChartModelHelper::findDiagram( xChartDoc );
    return NULL;
}

// static
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

// static
::std::vector< uno::Reference< XDataSeries > > ChartModelHelper::getDataSeries(
    const uno::Reference< XChartDocument > & xChartDoc )
{
    ::std::vector< uno::Reference< XDataSeries > > aResult;

    uno::Reference< XDiagram > xDiagram = ChartModelHelper::findDiagram( xChartDoc );
    if( xDiagram.is())
        aResult = DiagramHelper::getDataSeriesFromDiagram( xDiagram );

    return aResult;
}

// static
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

awt::Size ChartModelHelper::getPageSize( const uno::Reference< frame::XModel >& xModel )
{
    awt::Size aPageSize( 8000, 7000 );
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

//.............................................................................
} //namespace chart
//.............................................................................
