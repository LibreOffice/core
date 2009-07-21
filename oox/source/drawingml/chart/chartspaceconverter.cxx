/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: chartspaceconverter.cxx,v $
 *
 * $Revision: 1.4 $
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

#include "oox/drawingml/chart/chartspaceconverter.hxx"
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/chart/MissingValueTreatment.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include "oox/core/xmlfilterbase.hxx"
#include "oox/drawingml/chart/chartconverter.hxx"
#include "oox/drawingml/chart/chartdrawingfragment.hxx"
#include "oox/drawingml/chart/chartspacemodel.hxx"
#include "oox/drawingml/chart/plotareaconverter.hxx"
#include "oox/drawingml/chart/titleconverter.hxx"
#include "properties.hxx"

using ::rtl::OUString;
using ::com::sun::star::awt::Point;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::util::XNumberFormatsSupplier;
using ::com::sun::star::drawing::XDrawPageSupplier;
using ::com::sun::star::drawing::XShapes;
using ::com::sun::star::chart2::XDiagram;
using ::com::sun::star::chart2::XTitled;
using ::com::sun::star::chart2::data::XDataReceiver;

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

ChartSpaceConverter::ChartSpaceConverter( const ConverterRoot& rParent, ChartSpaceModel& rModel ) :
    ConverterBase< ChartSpaceModel >( rParent, rModel )
{
}

ChartSpaceConverter::~ChartSpaceConverter()
{
}

void ChartSpaceConverter::convertFromModel( const Reference< XShapes >& rxExternalPage, const Point& rChartPos )
{
    /*  create data provider (virtual function in the ChartConverter class,
        derived converters may create an external data provider) */
    getChartConverter().createDataProvider( getChartDocument() );

    // attach number formatter of container document to data receiver
    try
    {
        Reference< XDataReceiver > xDataRec( getChartDocument(), UNO_QUERY_THROW );
        Reference< XNumberFormatsSupplier > xNumFmtSupp( getFilter().getModel(), UNO_QUERY_THROW );
        xDataRec->attachNumberFormatsSupplier( xNumFmtSupp );
    }
    catch( Exception& )
    {
    }

    // formatting of the chart background
    PropertySet aBackPropSet( getChartDocument()->getPageBackground() );
    getFormatter().convertFrameFormatting( aBackPropSet, mrModel.mxShapeProp, OBJECTTYPE_CHARTSPACE );

    // convert plot area (container of all chart type groups)
    PlotAreaConverter aPlotAreaConv( *this, mrModel.mxPlotArea.getOrCreate() );
    aPlotAreaConv.convertFromModel( mrModel.mxView3D.getOrCreate() );

    // plot area converter has created the diagram object
    Reference< XDiagram > xDiagram = getChartDocument()->getFirstDiagram();

    // convert wall and floor formatting in 3D charts
    if( xDiagram.is() && aPlotAreaConv.isWall3dChart() )
    {
        WallFloorConverter aFloorConv( *this, mrModel.mxFloor.getOrCreate() );
        aFloorConv.convertFromModel( xDiagram, OBJECTTYPE_FLOOR );

        WallFloorConverter aWallConv( *this, mrModel.mxBackWall.getOrCreate() );
        aWallConv.convertFromModel( xDiagram, OBJECTTYPE_WALL );
    }

    // chart title
    if( !mrModel.mbAutoTitleDel ) try
    {
        /*  If the title model is missing, but the chart shows exactly one
            series, the series title is shown as chart title. */
        OUString aAutoTitle = aPlotAreaConv.getAutomaticTitle();
        if( mrModel.mxTitle.is() || (aAutoTitle.getLength() > 0) )
        {
            if( aAutoTitle.getLength() == 0 )
                aAutoTitle = CREATE_OUSTRING( "Chart Title" );
            Reference< XTitled > xTitled( getChartDocument(), UNO_QUERY_THROW );
            TitleConverter aTitleConv( *this, mrModel.mxTitle.getOrCreate() );
            aTitleConv.convertFromModel( xTitled, aAutoTitle, OBJECTTYPE_CHARTTITLE );
        }
    }
    catch( Exception& )
    {
    }

    // legend
    if( xDiagram.is() && mrModel.mxLegend.is() )
    {
        LegendConverter aLegendConv( *this, *mrModel.mxLegend );
        aLegendConv.convertFromModel( xDiagram );
    }

    // treatment of missing values
    if( xDiagram.is() )
    {
        using namespace ::com::sun::star::chart::MissingValueTreatment;
        sal_Int32 nMissingValues = LEAVE_GAP;
        switch( mrModel.mnDispBlanksAs )
        {
            case XML_gap:   nMissingValues = LEAVE_GAP; break;
            case XML_zero:  nMissingValues = USE_ZERO;  break;
            case XML_span:  nMissingValues = CONTINUE;  break;
        }
        PropertySet aDiaProp( xDiagram );
        aDiaProp.setProperty( PROP_MissingValueTreatment, nMissingValues );
    }

    Reference< com::sun::star::chart::XChartDocument > xOldChartDoc( getChartDocument(), UNO_QUERY );
    if( xOldChartDoc.is() )
    {
        /*  Set the IncludeHiddenCells property via the old API as only this
            ensures that the data provider and all created sequences get this
            flag correctly. */
        PropertySet aOldDiaProp( xOldChartDoc->getDiagram() );
        aOldDiaProp.setProperty( PROP_IncludeHiddenCells, !mrModel.mbPlotVisOnly );
    }

    // embedded drawing shapes
    if( mrModel.maDrawingPath.getLength() > 0 ) try
    {
        /*  Get the internal draw page of the chart document, if no external
            drawing page has been passed. */
        Reference< XShapes > xShapes;
        Point aShapesOffset( 0, 0 );
        if( rxExternalPage.is() )
        {
            xShapes = rxExternalPage;
            // offset for embedded shapes to move them inside the chart area
            aShapesOffset = rChartPos;
        }
        else
        {
            Reference< XDrawPageSupplier > xDrawPageSupp( getChartDocument(), UNO_QUERY_THROW );
            xShapes.set( xDrawPageSupp->getDrawPage(), UNO_QUERY_THROW );
        }

        /*  If an external drawing page is passed, all embedded shapes will be
            inserted there (used e.g. with 'chart sheets' in spreadsheet
            documents). In this case, all types of shapes including OLE objects
            are supported. If the shapes are inserted into the internal chart
            drawing page instead, it is not possible to embed OLE objects. */
        bool bOleSupport = rxExternalPage.is();

        // now, xShapes is not null anymore
        getFilter().importFragment( new ChartDrawingFragment(
            getFilter(), mrModel.maDrawingPath, xShapes, getChartSize(), aShapesOffset, bOleSupport ) );
    }
    catch( Exception& )
    {
    }
}

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

