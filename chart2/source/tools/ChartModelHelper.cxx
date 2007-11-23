/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartModelHelper.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 12:04:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "ChartModelHelper.hxx"
#include "macros.hxx"
#include "DiagramHelper.hxx"

#ifndef _COM_SUN_STAR_CHART2_DATA_XDATARECEIVER_HPP_
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart2/XChartDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XCHARTTYPECONTAINER_HPP_
#include <com/sun/star/chart2/XChartTypeContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XCOORDINATESYSTEMCONTAINER_HPP_
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDATASERIESCONTAINER_HPP_
#include <com/sun/star/chart2/XDataSeriesContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_EMBED_ASPECTS_HPP_
#include <com/sun/star/embed/Aspects.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XVISUALOBJECT_HPP_
#include <com/sun/star/embed/XVisualObject.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONCHANGELISTENER_HPP_
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#endif

// header for define DBG_ASSERT
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

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
