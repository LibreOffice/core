/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "oox/drawingml/chart/chartspaceconverter.hxx"

#include <com/sun/star/chart/MissingValueTreatment.hpp>
#include <com/sun/star/chart/XChartDocument.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/chart2/XTitled.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include "oox/core/xmlfilterbase.hxx"
#include "oox/drawingml/chart/chartconverter.hxx"
#include "oox/drawingml/chart/chartdrawingfragment.hxx"
#include "oox/drawingml/chart/chartspacemodel.hxx"
#include "oox/drawingml/chart/plotareaconverter.hxx"
#include "oox/drawingml/chart/titleconverter.hxx"

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::makeAny;
using ::com::sun::star::util::XNumberFormatsSupplier;
using ::com::sun::star::drawing::XDrawPageSupplier;
using ::com::sun::star::drawing::XShapes;
using ::com::sun::star::chart2::XDiagram;
using ::com::sun::star::chart2::XTitled;
using ::com::sun::star::chart2::data::XDataReceiver;
using ::com::sun::star::beans::XPropertySet;

namespace oox {
namespace drawingml {
namespace chart {



using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::chart2;
using namespace ::com::sun::star::chart2::data;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;



ChartSpaceConverter::ChartSpaceConverter( const ConverterRoot& rParent, ChartSpaceModel& rModel ) :
    ConverterBase< ChartSpaceModel >( rParent, rModel )
{
}

ChartSpaceConverter::~ChartSpaceConverter()
{
}

void ChartSpaceConverter::convertFromModel( const Reference< XShapes >& rxExternalPage, const awt::Point& rChartPos )
{
    if( !getChartConverter() )
        return;

    /*  create data provider (virtual function in the ChartConverter class,
        derived converters may create an external data provider) */
    getChartConverter()->createDataProvider( getChartDocument() );

    
    try
    {
        Reference< XDataReceiver > xDataRec( getChartDocument(), UNO_QUERY_THROW );
        Reference< XNumberFormatsSupplier > xNumFmtSupp( getFilter().getModel(), UNO_QUERY_THROW );
        xDataRec->attachNumberFormatsSupplier( xNumFmtSupp );
    }
    catch( Exception& )
    {
    }

    
    PropertySet aBackPropSet( getChartDocument()->getPageBackground() );
    getFormatter().convertFrameFormatting( aBackPropSet, mrModel.mxShapeProp, OBJECTTYPE_CHARTSPACE );

    
    PlotAreaConverter aPlotAreaConv( *this, mrModel.mxPlotArea.getOrCreate() );
    aPlotAreaConv.convertFromModel( mrModel.mxView3D.getOrCreate() );

    
    Reference< XDiagram > xDiagram = getChartDocument()->getFirstDiagram();

    
    if( xDiagram.is() && aPlotAreaConv.isWall3dChart() )
    {
        WallFloorConverter aFloorConv( *this, mrModel.mxFloor.getOrCreate() );
        aFloorConv.convertFromModel( xDiagram, OBJECTTYPE_FLOOR );

        WallFloorConverter aWallConv( *this, mrModel.mxBackWall.getOrCreate() );
        aWallConv.convertFromModel( xDiagram, OBJECTTYPE_WALL );
    }

    
    if( !mrModel.mbAutoTitleDel ) try
    {
        /*  If the title model is missing, but the chart shows exactly one
            series, the series title is shown as chart title. */
        OUString aAutoTitle = aPlotAreaConv.getAutomaticTitle();
        if( mrModel.mxTitle.is() || !aAutoTitle.isEmpty() )
        {
            if( aAutoTitle.isEmpty() )
                aAutoTitle = "Chart Title";
            Reference< XTitled > xTitled( getChartDocument(), UNO_QUERY_THROW );
            TitleConverter aTitleConv( *this, mrModel.mxTitle.getOrCreate() );
            aTitleConv.convertFromModel( xTitled, aAutoTitle, OBJECTTYPE_CHARTTITLE );
        }
    }
    catch( Exception& )
    {
    }

    
    if( xDiagram.is() && mrModel.mxLegend.is() )
    {
        LegendConverter aLegendConv( *this, *mrModel.mxLegend );
        aLegendConv.convertFromModel( xDiagram );
    }

    
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

    /*  Following all conversions needing the old Chart1 API that involves full
        initialization of the chart view. */
    namespace cssc = ::com::sun::star::chart;
    Reference< cssc::XChartDocument > xChart1Doc( getChartDocument(), UNO_QUERY );
    if( xChart1Doc.is() )
    {
        /*  Set the IncludeHiddenCells property via the old API as only this
            ensures that the data provider and all created sequences get this
            flag correctly. */
        PropertySet aDiaProp( xChart1Doc->getDiagram() );
        aDiaProp.setProperty( PROP_IncludeHiddenCells, !mrModel.mbPlotVisOnly );

        
        aPlotAreaConv.convertPositionFromModel();

        
        convertTitlePositions();
    }

    
    if( !mrModel.maDrawingPath.isEmpty() ) try
    {
        /*  Get the internal draw page of the chart document, if no external
            drawing page has been passed. */
        Reference< XShapes > xShapes;
        awt::Point aShapesOffset( 0, 0 );
        if( rxExternalPage.is() )
        {
            xShapes = rxExternalPage;
            
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

        
        getFilter().importFragment( new ChartDrawingFragment(
            getFilter(), mrModel.maDrawingPath, xShapes, getChartSize(), aShapesOffset, bOleSupport ) );
    }
    catch( Exception& )
    {
    }

    
    if ( mrModel.mbPivotChart )
    {
        PropertySet aProps( getChartDocument() );
        aProps.setProperty( PROP_DisableDataTableDialog , true );
        aProps.setProperty( PROP_DisableComplexChartTypes , true );
    }

    if(!mrModel.maSheetPath.isEmpty() )
    {
        Reference< ::com::sun::star::chart::XChartDocument > xChartDoc( getChartDocument(), UNO_QUERY );
        PropertySet aProps( xChartDoc->getDiagram() );
        aProps.setProperty( PROP_ExternalData , uno::makeAny(mrModel.maSheetPath) );
    }
}



} 
} 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
