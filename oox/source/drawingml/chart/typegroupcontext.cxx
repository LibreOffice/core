/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: typegroupcontext.cxx,v $
 * $Revision: 1.3 $
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

#include "oox/drawingml/chart/typegroupcontext.hxx"
#include "oox/drawingml/chart/seriescontext.hxx"
#include "oox/drawingml/chart/typegroupmodel.hxx"

using ::oox::core::ContextHandler2Helper;
using ::oox::core::ContextWrapper;

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

AreaTypeGroupContext::AreaTypeGroupContext( ContextHandler2Helper& rParent, TypeGroupModel& rModel ) :
    TypeGroupContextBase( rParent, rModel )
{
}

AreaTypeGroupContext::~AreaTypeGroupContext()
{
}

// oox.core.ContextHandler2Helper interface -----------------------------------

ContextWrapper AreaTypeGroupContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( getCurrentElement() == getModel().getData().mnTypeId )
    {
        switch( nElement )
        {
            case C_TOKEN( ser ):
                return new SeriesContext( *this, getModel().createSeries(), C_TOKEN( tx ), C_TOKEN( cat ), C_TOKEN( val ) );
            case C_TOKEN( axId ):
                getModel().getData().maAxisIds.push_back( rAttribs.getInteger( XML_val, -1 ) );
                return false;
            case C_TOKEN( gapDepth ):
                getModel().getData().mnGapDepth = rAttribs.getInteger( XML_val, 150 );
                return false;
            case C_TOKEN( grouping ):
                getModel().getData().mnGrouping = rAttribs.getToken( XML_val, XML_standard );
                return false;
            case C_TOKEN( varyColors ):
                getModel().getData().mbVaryColors = rAttribs.getBool( XML_val, true );
                return false;
        }
    }
    return false;
}

// ============================================================================

BarTypeGroupContext::BarTypeGroupContext( ContextHandler2Helper& rParent, TypeGroupModel& rModel ) :
    TypeGroupContextBase( rParent, rModel )
{
}

BarTypeGroupContext::~BarTypeGroupContext()
{
}

// oox.core.ContextHandler2Helper interface -----------------------------------

ContextWrapper BarTypeGroupContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( getCurrentElement() == getModel().getData().mnTypeId )
    {
        switch( nElement )
        {
            case C_TOKEN( ser ):
                return new SeriesContext( *this, getModel().createSeries(), C_TOKEN( tx ), C_TOKEN( cat ), C_TOKEN( val ) );
            case C_TOKEN( axId ):
                getModel().getData().maAxisIds.push_back( rAttribs.getInteger( XML_val, -1 ) );
                return false;
            case C_TOKEN( barDir ):
                getModel().getData().mnBarDir = rAttribs.getToken( XML_val, XML_col );
                return false;
            case C_TOKEN( gapDepth ):
                getModel().getData().mnGapDepth = rAttribs.getInteger( XML_val, 150 );
                return false;
            case C_TOKEN( gapWidth ):
                getModel().getData().mnGapWidth = rAttribs.getInteger( XML_val, 150 );
                return false;
            case C_TOKEN( grouping ):
                // default is XML_standard and *not* XML_clustered as specified
                getModel().getData().mnGrouping = rAttribs.getToken( XML_val, XML_standard );
                return false;
            case C_TOKEN( overlap ):
                getModel().getData().mnOverlap = rAttribs.getInteger( XML_val, 0 );
                return false;
            case C_TOKEN( shape ):
                getModel().getData().mnShape = rAttribs.getToken( XML_val, XML_box );
                return false;
            case C_TOKEN( varyColors ):
                getModel().getData().mbVaryColors = rAttribs.getBool( XML_val, true );
                return false;
        }
    }
    return false;
}

// ============================================================================

BubbleTypeGroupContext::BubbleTypeGroupContext( ContextHandler2Helper& rParent, TypeGroupModel& rModel ) :
    TypeGroupContextBase( rParent, rModel )
{
}

BubbleTypeGroupContext::~BubbleTypeGroupContext()
{
}

// oox.core.ContextHandler2Helper interface -----------------------------------

ContextWrapper BubbleTypeGroupContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( getCurrentElement() == getModel().getData().mnTypeId )
    {
        switch( nElement )
        {
            case C_TOKEN( ser ):
                return new SeriesContext( *this, getModel().createSeries(), C_TOKEN( tx ), C_TOKEN( xVal ), C_TOKEN( yVal ), C_TOKEN( bubbleSize ) );
            case C_TOKEN( axId ):
                getModel().getData().maAxisIds.push_back( rAttribs.getInteger( XML_val, -1 ) );
                return false;
            case C_TOKEN( bubble3D ):
                getModel().getData().mbBubble3d = rAttribs.getBool( XML_val, true );
                return false;
            case C_TOKEN( bubbleScale ):
                getModel().getData().mnBubbleScale = rAttribs.getInteger( XML_val, 100 );
                return false;
            case C_TOKEN( showNegBubbles ):
                getModel().getData().mbShowNegBubbles = rAttribs.getBool( XML_val, true );
                return false;
            case C_TOKEN( sizeRepresents ):
                getModel().getData().mnSizeRepresents = rAttribs.getToken( XML_val, XML_area );
                return false;
            case C_TOKEN( varyColors ):
                getModel().getData().mbVaryColors = rAttribs.getBool( XML_val, true );
                return false;
        }
    }
    return false;
}

// ============================================================================

LineTypeGroupContext::LineTypeGroupContext( ContextHandler2Helper& rParent, TypeGroupModel& rModel ) :
    TypeGroupContextBase( rParent, rModel )
{
}

LineTypeGroupContext::~LineTypeGroupContext()
{
}

// oox.core.ContextHandler2Helper interface -----------------------------------

ContextWrapper LineTypeGroupContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( getCurrentElement() == getModel().getData().mnTypeId )
    {
        switch( nElement )
        {
            case C_TOKEN( ser ):
                return new SeriesContext( *this, getModel().createSeries(), C_TOKEN( tx ), C_TOKEN( cat ), C_TOKEN( val ) );
            case C_TOKEN( axId ):
                getModel().getData().maAxisIds.push_back( rAttribs.getInteger( XML_val, -1 ) );
                return false;
            case C_TOKEN( gapDepth ):
                getModel().getData().mnGapDepth = rAttribs.getInteger( XML_val, 150 );
                return false;
            case C_TOKEN( grouping ):
                getModel().getData().mnGrouping = rAttribs.getToken( XML_val, XML_standard );
                return false;
            case C_TOKEN( marker ):
                getModel().getData().mbShowMarker = rAttribs.getBool( XML_val, true );
                return false;
            case C_TOKEN( smooth ):
                getModel().getData().mbSmooth = rAttribs.getBool( XML_val, true );
                return false;
            case C_TOKEN( varyColors ):
                getModel().getData().mbVaryColors = rAttribs.getBool( XML_val, true );
                return false;
        }
    }
    return false;
}

// ============================================================================

PieTypeGroupContext::PieTypeGroupContext( ContextHandler2Helper& rParent, TypeGroupModel& rModel ) :
    TypeGroupContextBase( rParent, rModel )
{
}

PieTypeGroupContext::~PieTypeGroupContext()
{
}

// oox.core.ContextHandler2Helper interface -----------------------------------

ContextWrapper PieTypeGroupContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( getCurrentElement() == getModel().getData().mnTypeId )
    {
        switch( nElement )
        {
            case C_TOKEN( ser ):
                return new SeriesContext( *this, getModel().createSeries(), C_TOKEN( tx ), C_TOKEN( cat ), C_TOKEN( val ) );
            case C_TOKEN( firstSliceAng ):
                getModel().getData().mnFirstAngle = rAttribs.getInteger( XML_val, 0 );
                return false;
            case C_TOKEN( gapWidth ):
                getModel().getData().mnGapWidth = rAttribs.getInteger( XML_val, 150 );
                return false;
            case C_TOKEN( holeSize ):
                getModel().getData().mnHoleSize = rAttribs.getInteger( XML_val, 10 );
                return false;
            case C_TOKEN( ofPieType ):
                getModel().getData().mnOfPieType = rAttribs.getToken( XML_val, XML_pie );
                return false;
            case C_TOKEN( secondPieSize ):
                getModel().getData().mnSecondPieSize = rAttribs.getInteger( XML_val, 75 );
                return false;
            case C_TOKEN( splitPos ):
                getModel().getData().mfSplitPos = rAttribs.getDouble( XML_val, 0.0 );
                return false;
            case C_TOKEN( splitType ):
                getModel().getData().mnSplitType = rAttribs.getToken( XML_val, XML_auto );
                return false;
            case C_TOKEN( varyColors ):
                getModel().getData().mbVaryColors = rAttribs.getBool( XML_val, true );
                return false;
        }
    }
    return false;
}

// ============================================================================

RadarTypeGroupContext::RadarTypeGroupContext( ContextHandler2Helper& rParent, TypeGroupModel& rModel ) :
    TypeGroupContextBase( rParent, rModel )
{
}

RadarTypeGroupContext::~RadarTypeGroupContext()
{
}

// oox.core.ContextHandler2Helper interface -----------------------------------

ContextWrapper RadarTypeGroupContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( getCurrentElement() == getModel().getData().mnTypeId )
    {
        switch( nElement )
        {
            case C_TOKEN( ser ):
                return new SeriesContext( *this, getModel().createSeries(), C_TOKEN( tx ), C_TOKEN( cat ), C_TOKEN( val ) );
            case C_TOKEN( axId ):
                getModel().getData().maAxisIds.push_back( rAttribs.getInteger( XML_val, -1 ) );
                return false;
            case C_TOKEN( radarStyle ):
                getModel().getData().mnRadarStyle = rAttribs.getToken( XML_val, XML_standard );
                return false;
            case C_TOKEN( varyColors ):
                getModel().getData().mbVaryColors = rAttribs.getBool( XML_val, true );
                return false;
        }
    }
    return false;
}

// ============================================================================

ScatterTypeGroupContext::ScatterTypeGroupContext( ContextHandler2Helper& rParent, TypeGroupModel& rModel ) :
    TypeGroupContextBase( rParent, rModel )
{
}

ScatterTypeGroupContext::~ScatterTypeGroupContext()
{
}

// oox.core.ContextHandler2Helper interface -----------------------------------

ContextWrapper ScatterTypeGroupContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( getCurrentElement() == getModel().getData().mnTypeId )
    {
        switch( nElement )
        {
            case C_TOKEN( ser ):
                return new SeriesContext( *this, getModel().createSeries(), C_TOKEN( tx ), C_TOKEN( xVal ), C_TOKEN( yVal ) );
            case C_TOKEN( axId ):
                getModel().getData().maAxisIds.push_back( rAttribs.getInteger( XML_val, -1 ) );
                return false;
            case C_TOKEN( scatterStyle ):
                getModel().getData().mnScatterStyle = rAttribs.getInteger( XML_val, XML_marker );
                return false;
            case C_TOKEN( varyColors ):
                getModel().getData().mbVaryColors = rAttribs.getBool( XML_val, true );
                return false;
        }
    }
    return false;
}

// ============================================================================

SurfaceTypeGroupContext::SurfaceTypeGroupContext( ContextHandler2Helper& rParent, TypeGroupModel& rModel ) :
    TypeGroupContextBase( rParent, rModel )
{
}

SurfaceTypeGroupContext::~SurfaceTypeGroupContext()
{
}

// oox.core.ContextHandler2Helper interface -----------------------------------

ContextWrapper SurfaceTypeGroupContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( getCurrentElement() == getModel().getData().mnTypeId )
    {
        switch( nElement )
        {
            case C_TOKEN( ser ):
                return new SeriesContext( *this, getModel().createSeries(), C_TOKEN( tx ), C_TOKEN( cat ), C_TOKEN( val ) );
            case C_TOKEN( axId ):
                getModel().getData().maAxisIds.push_back( rAttribs.getInteger( XML_val, -1 ) );
                return false;
            case C_TOKEN( wireframe ):
                getModel().getData().mbWireframe = rAttribs.getBool( XML_val, true );
                return false;
        }
    }
    return false;
}

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

