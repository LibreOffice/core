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

#include "drawingml/chart/plotareacontext.hxx"

#include "drawingml/shapepropertiescontext.hxx"
#include "drawingml/chart/axiscontext.hxx"
#include "drawingml/chart/plotareamodel.hxx"
#include "drawingml/chart/seriescontext.hxx"
#include "drawingml/chart/titlecontext.hxx"
#include "drawingml/chart/typegroupcontext.hxx"

namespace oox {
namespace drawingml {
namespace chart {

using ::oox::core::ContextHandler2Helper;
using ::oox::core::ContextHandlerRef;

View3DContext::View3DContext( ContextHandler2Helper& rParent, View3DModel& rModel ) :
    ContextBase< View3DModel >( rParent, rModel )
{
}

View3DContext::~View3DContext()
{
}

ContextHandlerRef View3DContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    bool bMSO2007Doc = getFilter().isMSO2007Document();
    switch( getCurrentElement() )
    {
        case C_TOKEN( view3D ):
            switch( nElement )
            {
                case C_TOKEN( depthPercent ):
                    mrModel.mnDepthPercent = rAttribs.getInteger( XML_val, 100 );
                    return nullptr;
                case C_TOKEN( hPercent ):
                    mrModel.monHeightPercent = rAttribs.getInteger( XML_val, 100 );
                    return nullptr;
                case C_TOKEN( perspective ):
                    mrModel.mnPerspective = rAttribs.getInteger( XML_val, 30 );
                    return nullptr;
                case C_TOKEN( rAngAx ):
                    mrModel.mbRightAngled = rAttribs.getBool( XML_val, !bMSO2007Doc );
                    return nullptr;
                case C_TOKEN( rotX ):
                    // default value dependent on chart type
                    mrModel.monRotationX = rAttribs.getInteger( XML_val );
                    return nullptr;
                case C_TOKEN( rotY ):
                    // default value dependent on chart type
                    mrModel.monRotationY = rAttribs.getInteger( XML_val );
                    return nullptr;
            }
        break;
    }
    return nullptr;
}

WallFloorContext::WallFloorContext( ContextHandler2Helper& rParent, WallFloorModel& rModel ) :
    ContextBase< WallFloorModel >( rParent, rModel )
{
}

WallFloorContext::~WallFloorContext()
{
}

ContextHandlerRef WallFloorContext::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    bool bMSO2007Doc = getFilter().isMSO2007Document();
    switch( getCurrentElement() )
    {
        case C_TOKEN( backWall ):
        case C_TOKEN( floor ):
        case C_TOKEN( sideWall ):
            switch( nElement )
            {
                case C_TOKEN( pictureOptions ):
                    return new PictureOptionsContext( *this, mrModel.mxPicOptions.create(bMSO2007Doc) );
                case C_TOKEN( spPr ):
                    return new ShapePropertiesContext( *this, mrModel.mxShapeProp.create() );
            }
        break;
    }
    return nullptr;
}

DataTableContext::DataTableContext( ContextHandler2Helper& rParent, DataTableModel& rModel ) :
    ContextBase< DataTableModel >( rParent, rModel )
{
}

DataTableContext::~DataTableContext()
{
}

ContextHandlerRef DataTableContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs)
{
    switch( getCurrentElement() )
    {
        case C_TOKEN( dTable ):
            switch( nElement )
            {
                case C_TOKEN( showHorzBorder ):
                        mrModel.mbShowHBorder = rAttribs.getBool( XML_val, false );
                        break;
                case C_TOKEN( showVertBorder ):
                        mrModel.mbShowVBorder = rAttribs.getBool( XML_val, false );
                        break;
                case C_TOKEN( showOutline ):
                        mrModel.mbShowOutline = rAttribs.getBool( XML_val, false );
                        break;
            }
        break;
    }
    return nullptr;
}

PlotAreaContext::PlotAreaContext( ContextHandler2Helper& rParent, PlotAreaModel& rModel ) :
    ContextBase< PlotAreaModel >( rParent, rModel )
{
}

PlotAreaContext::~PlotAreaContext()
{
}

ContextHandlerRef PlotAreaContext::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    bool bMSO2007Doc = getFilter().isMSO2007Document();
    switch( getCurrentElement() )
    {
        case C_TOKEN( plotArea ):
            switch( nElement )
            {
                case C_TOKEN( area3DChart ):
                case C_TOKEN( areaChart ):
                    return new AreaTypeGroupContext( *this, mrModel.maTypeGroups.create( nElement, bMSO2007Doc ) );
                case C_TOKEN( bar3DChart ):
                case C_TOKEN( barChart ):
                    return new BarTypeGroupContext( *this, mrModel.maTypeGroups.create( nElement, bMSO2007Doc ) );
                case C_TOKEN( bubbleChart ):
                    return new BubbleTypeGroupContext( *this, mrModel.maTypeGroups.create( nElement, bMSO2007Doc ) );
                case C_TOKEN( line3DChart ):
                case C_TOKEN( lineChart ):
                case C_TOKEN( stockChart ):
                    return new LineTypeGroupContext( *this, mrModel.maTypeGroups.create( nElement, bMSO2007Doc ) );
                case C_TOKEN( doughnutChart ):
                case C_TOKEN( ofPieChart ):
                case C_TOKEN( pie3DChart ):
                case C_TOKEN( pieChart ):
                    return new PieTypeGroupContext( *this, mrModel.maTypeGroups.create( nElement, bMSO2007Doc ) );
                case C_TOKEN( radarChart ):
                    return new RadarTypeGroupContext( *this, mrModel.maTypeGroups.create( nElement, bMSO2007Doc ) );
                case C_TOKEN( scatterChart ):
                    return new ScatterTypeGroupContext( *this, mrModel.maTypeGroups.create( nElement, bMSO2007Doc ) );
                case C_TOKEN( surface3DChart ):
                case C_TOKEN( surfaceChart ):
                    return new SurfaceTypeGroupContext( *this, mrModel.maTypeGroups.create( nElement, bMSO2007Doc ) );

                case C_TOKEN( catAx ):
                    return new CatAxisContext( *this, mrModel.maAxes.create( nElement, bMSO2007Doc ) );
                case C_TOKEN( dateAx ):
                    return new DateAxisContext( *this, mrModel.maAxes.create( nElement, bMSO2007Doc ) );
                case C_TOKEN( serAx ):
                    return new SerAxisContext( *this, mrModel.maAxes.create( nElement, bMSO2007Doc ) );
                case C_TOKEN( valAx ):
                    return new ValAxisContext( *this, mrModel.maAxes.create( nElement, bMSO2007Doc ) );

                case C_TOKEN( layout ):
                    return new LayoutContext( *this, mrModel.mxLayout.create() );
                case C_TOKEN( spPr ):
                    return new ShapePropertiesContext( *this, mrModel.mxShapeProp.create() );
                case C_TOKEN(dTable):
                    return new DataTableContext( *this, mrModel.mxDataTable.create() );
            }
        break;
    }
    return nullptr;
}

} // namespace chart
} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
