/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: plotareacontext.cxx,v $
 * $Revision: 1.6 $
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

#include "oox/drawingml/chart/plotareacontext.hxx"
#include "oox/drawingml/shapepropertiescontext.hxx"
#include "oox/drawingml/chart/axiscontext.hxx"
#include "oox/drawingml/chart/plotareamodel.hxx"
#include "oox/drawingml/chart/seriescontext.hxx"
#include "oox/drawingml/chart/titlecontext.hxx"
#include "oox/drawingml/chart/typegroupcontext.hxx"

using ::oox::core::ContextHandler2Helper;
using ::oox::core::ContextHandlerRef;

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

View3DContext::View3DContext( ContextHandler2Helper& rParent, View3DModel& rModel ) :
    ContextBase< View3DModel >( rParent, rModel )
{
}

View3DContext::~View3DContext()
{
}

ContextHandlerRef View3DContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case C_TOKEN( view3D ):
            switch( nElement )
            {
                case C_TOKEN( depthPercent ):
                    mrModel.mnDepthPercent = rAttribs.getInteger( XML_val, 100 );
                    return 0;
                case C_TOKEN( hPercent ):
                    mrModel.monHeightPercent = rAttribs.getInteger( XML_val, 100 );
                    return 0;
                case C_TOKEN( perspective ):
                    mrModel.mnPerspective = rAttribs.getInteger( XML_val, 30 );
                    return 0;
                case C_TOKEN( rAngAx ):
                    // default is 'false', not 'true' as specified
                    mrModel.mbRightAngled = rAttribs.getBool( XML_val, false );
                    return 0;
                case C_TOKEN( rotX ):
                    // default value dependent on chart type
                    mrModel.monRotationX = rAttribs.getInteger( XML_val );
                    return 0;
                case C_TOKEN( rotY ):
                    // default value dependent on chart type
                    mrModel.monRotationY = rAttribs.getInteger( XML_val );
                    return 0;
            }
        break;
    }
    return 0;
}

// ============================================================================

WallFloorContext::WallFloorContext( ContextHandler2Helper& rParent, WallFloorModel& rModel ) :
    ContextBase< WallFloorModel >( rParent, rModel )
{
}

WallFloorContext::~WallFloorContext()
{
}

ContextHandlerRef WallFloorContext::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    switch( getCurrentElement() )
    {
        case C_TOKEN( backWall ):
        case C_TOKEN( floor ):
        case C_TOKEN( sideWall ):
            switch( nElement )
            {
                case C_TOKEN( pictureOptions ):
                    return new PictureOptionsContext( *this, mrModel.mxPicOptions.create() );
                case C_TOKEN( spPr ):
                    return new ShapePropertiesContext( *this, mrModel.mxShapeProp.create() );
            }
        break;
    }
    return 0;
}

// ============================================================================

PlotAreaContext::PlotAreaContext( ContextHandler2Helper& rParent, PlotAreaModel& rModel ) :
    ContextBase< PlotAreaModel >( rParent, rModel )
{
}

PlotAreaContext::~PlotAreaContext()
{
}

ContextHandlerRef PlotAreaContext::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    switch( getCurrentElement() )
    {
        case C_TOKEN( plotArea ):
            switch( nElement )
            {
                case C_TOKEN( area3DChart ):
                case C_TOKEN( areaChart ):
                    return new AreaTypeGroupContext( *this, mrModel.maTypeGroups.create( nElement ) );
                case C_TOKEN( bar3DChart ):
                case C_TOKEN( barChart ):
                    return new BarTypeGroupContext( *this, mrModel.maTypeGroups.create( nElement ) );
                case C_TOKEN( bubbleChart ):
                    return new BubbleTypeGroupContext( *this, mrModel.maTypeGroups.create( nElement ) );
                case C_TOKEN( line3DChart ):
                case C_TOKEN( lineChart ):
                case C_TOKEN( stockChart ):
                    return new LineTypeGroupContext( *this, mrModel.maTypeGroups.create( nElement ) );
                case C_TOKEN( doughnutChart ):
                case C_TOKEN( ofPieChart ):
                case C_TOKEN( pie3DChart ):
                case C_TOKEN( pieChart ):
                    return new PieTypeGroupContext( *this, mrModel.maTypeGroups.create( nElement ) );
                case C_TOKEN( radarChart ):
                    return new RadarTypeGroupContext( *this, mrModel.maTypeGroups.create( nElement ) );
                case C_TOKEN( scatterChart ):
                    return new ScatterTypeGroupContext( *this, mrModel.maTypeGroups.create( nElement ) );
                case C_TOKEN( surface3DChart ):
                case C_TOKEN( surfaceChart ):
                    return new SurfaceTypeGroupContext( *this, mrModel.maTypeGroups.create( nElement ) );

                case C_TOKEN( catAx ):
                    return new CatAxisContext( *this, mrModel.maAxes.create( nElement ) );
                case C_TOKEN( dateAx ):
                    return new DateAxisContext( *this, mrModel.maAxes.create( nElement ) );
                case C_TOKEN( serAx ):
                    return new SerAxisContext( *this, mrModel.maAxes.create( nElement ) );
                case C_TOKEN( valAx ):
                    return new ValAxisContext( *this, mrModel.maAxes.create( nElement ) );

                case C_TOKEN( layout ):
                    return new LayoutContext( *this, mrModel.mxLayout.create() );
                case C_TOKEN( spPr ):
                    return new ShapePropertiesContext( *this, mrModel.mxShapeProp.create() );
            }
        break;
    }
    return 0;
}

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

