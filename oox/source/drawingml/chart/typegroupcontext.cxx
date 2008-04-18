/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: typegroupcontext.cxx,v $
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

#include "oox/drawingml/chart/typegroupcontext.hxx"
#include "oox/drawingml/chart/seriescontext.hxx"
#include "oox/drawingml/chart/typegroupmodel.hxx"

using ::oox::core::ContextHandler2Helper;
using ::oox::core::ContextWrapper;

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

UpDownBarsContext::UpDownBarsContext( ContextHandler2Helper& rParent, UpDownBarsModel& rModel ) :
    ContextBase< UpDownBarsModel >( rParent, rModel )
{
}

UpDownBarsContext::~UpDownBarsContext()
{
}

ContextWrapper UpDownBarsContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case C_TOKEN( upDownBars ):
            switch( nElement )
            {
                case C_TOKEN( downBars ):
                    return new ShapePrWrapperContext( *this, mrModel.mxDownBars.create() );
                case C_TOKEN( gapWidth ):
                    mrModel.mnGapWidth = rAttribs.getInteger( XML_val, 150 );
                    return false;
                case C_TOKEN( upBars ):
                    return new ShapePrWrapperContext( *this, mrModel.mxUpBars.create() );
            }
        break;
    }
    return false;
}

// ============================================================================

AreaTypeGroupContext::AreaTypeGroupContext( ContextHandler2Helper& rParent, TypeGroupModel& rModel ) :
    TypeGroupContextBase( rParent, rModel )
{
}

AreaTypeGroupContext::~AreaTypeGroupContext()
{
}

ContextWrapper AreaTypeGroupContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( isRootElement() ) switch( nElement )
    {
        case C_TOKEN( axId ):
            mrModel.maAxisIds.push_back( rAttribs.getInteger( XML_val, -1 ) );
            return false;
        case C_TOKEN( dLbls ):
            return new DataLabelsContext( *this, mrModel.mxLabels.create() );
        case C_TOKEN( dropLines ):
            return new ShapePrWrapperContext( *this, mrModel.mxDropLines.create() );
        case C_TOKEN( gapDepth ):
            mrModel.mnGapDepth = rAttribs.getInteger( XML_val, 150 );
            return false;
        case C_TOKEN( grouping ):
            mrModel.mnGrouping = rAttribs.getToken( XML_val, XML_standard );
            return false;
        case C_TOKEN( ser ):
            return new AreaSeriesContext( *this, mrModel.maSeries.create() );
        case C_TOKEN( varyColors ):
            mrModel.mbVaryColors = rAttribs.getBool( XML_val, true );
            return false;
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

ContextWrapper BarTypeGroupContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( isRootElement() ) switch( nElement )
    {
        case C_TOKEN( axId ):
            mrModel.maAxisIds.push_back( rAttribs.getInteger( XML_val, -1 ) );
            return false;
        case C_TOKEN( barDir ):
            mrModel.mnBarDir = rAttribs.getToken( XML_val, XML_col );
            return false;
        case C_TOKEN( dLbls ):
            return new DataLabelsContext( *this, mrModel.mxLabels.create() );
        case C_TOKEN( gapDepth ):
            mrModel.mnGapDepth = rAttribs.getInteger( XML_val, 150 );
            return false;
        case C_TOKEN( gapWidth ):
            mrModel.mnGapWidth = rAttribs.getInteger( XML_val, 150 );
            return false;
        case C_TOKEN( grouping ):
            // default is XML_standard and *not* XML_clustered as specified
            mrModel.mnGrouping = rAttribs.getToken( XML_val, XML_standard );
            return false;
        case C_TOKEN( overlap ):
            mrModel.mnOverlap = rAttribs.getInteger( XML_val, 0 );
            return false;
        case C_TOKEN( ser ):
            return new BarSeriesContext( *this, mrModel.maSeries.create() );
        case C_TOKEN( serLines ):
            return new ShapePrWrapperContext( *this, mrModel.mxSerLines.create() );
        case C_TOKEN( shape ):
            mrModel.mnShape = rAttribs.getToken( XML_val, XML_box );
            return false;
        case C_TOKEN( varyColors ):
            mrModel.mbVaryColors = rAttribs.getBool( XML_val, true );
            return false;
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

ContextWrapper BubbleTypeGroupContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( isRootElement() ) switch( nElement )
    {
        case C_TOKEN( axId ):
            mrModel.maAxisIds.push_back( rAttribs.getInteger( XML_val, -1 ) );
            return false;
        case C_TOKEN( bubble3D ):
            mrModel.mbBubble3d = rAttribs.getBool( XML_val, true );
            return false;
        case C_TOKEN( bubbleScale ):
            mrModel.mnBubbleScale = rAttribs.getInteger( XML_val, 100 );
            return false;
        case C_TOKEN( dLbls ):
            return new DataLabelsContext( *this, mrModel.mxLabels.create() );
        case C_TOKEN( ser ):
            return new BubbleSeriesContext( *this, mrModel.maSeries.create() );
        case C_TOKEN( showNegBubbles ):
            mrModel.mbShowNegBubbles = rAttribs.getBool( XML_val, true );
            return false;
        case C_TOKEN( sizeRepresents ):
            mrModel.mnSizeRepresents = rAttribs.getToken( XML_val, XML_area );
            return false;
        case C_TOKEN( varyColors ):
            mrModel.mbVaryColors = rAttribs.getBool( XML_val, true );
            return false;
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

ContextWrapper LineTypeGroupContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( isRootElement() ) switch( nElement )
    {
        case C_TOKEN( axId ):
            mrModel.maAxisIds.push_back( rAttribs.getInteger( XML_val, -1 ) );
            return false;
        case C_TOKEN( dLbls ):
            return new DataLabelsContext( *this, mrModel.mxLabels.create() );
        case C_TOKEN( dropLines ):
            return new ShapePrWrapperContext( *this, mrModel.mxDropLines.create() );
        case C_TOKEN( gapDepth ):
            mrModel.mnGapDepth = rAttribs.getInteger( XML_val, 150 );
            return false;
        case C_TOKEN( grouping ):
            mrModel.mnGrouping = rAttribs.getToken( XML_val, XML_standard );
            return false;
        case C_TOKEN( hiLowLines ):
            return new ShapePrWrapperContext( *this, mrModel.mxHiLowLines.create() );
        case C_TOKEN( marker ):
            /*  This value is *ignored* by Excel. Markers can be switched off
                only by setting the <c:ser> -> <c:marker> -> <c:symbol> element
                to 'none'. */
            mrModel.mbShowMarker = rAttribs.getBool( XML_val, true );
            return false;
        case C_TOKEN( ser ):
            return new LineSeriesContext( *this, mrModel.maSeries.create() );
        case C_TOKEN( smooth ):
            /*  This value is *ignored* by Excel. Line smoothing is always
                controlled by the series. */
            mrModel.mbSmooth = rAttribs.getBool( XML_val, true );
            return false;
        case C_TOKEN( varyColors ):
            mrModel.mbVaryColors = rAttribs.getBool( XML_val, true );
            return false;
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

ContextWrapper PieTypeGroupContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( isRootElement() ) switch( nElement )
    {
        case C_TOKEN( dLbls ):
            return new DataLabelsContext( *this, mrModel.mxLabels.create() );
        case C_TOKEN( firstSliceAng ):
            mrModel.mnFirstAngle = rAttribs.getInteger( XML_val, 0 );
            return false;
        case C_TOKEN( gapWidth ):
            mrModel.mnGapWidth = rAttribs.getInteger( XML_val, 150 );
            return false;
        case C_TOKEN( holeSize ):
            mrModel.mnHoleSize = rAttribs.getInteger( XML_val, 10 );
            return false;
        case C_TOKEN( ofPieType ):
            mrModel.mnOfPieType = rAttribs.getToken( XML_val, XML_pie );
            return false;
        case C_TOKEN( secondPieSize ):
            mrModel.mnSecondPieSize = rAttribs.getInteger( XML_val, 75 );
            return false;
        case C_TOKEN( ser ):
            return new PieSeriesContext( *this, mrModel.maSeries.create() );
        case C_TOKEN( serLines ):
            return new ShapePrWrapperContext( *this, mrModel.mxSerLines.create() );
        case C_TOKEN( splitPos ):
            mrModel.mfSplitPos = rAttribs.getDouble( XML_val, 0.0 );
            return false;
        case C_TOKEN( splitType ):
            mrModel.mnSplitType = rAttribs.getToken( XML_val, XML_auto );
            return false;
        case C_TOKEN( varyColors ):
            mrModel.mbVaryColors = rAttribs.getBool( XML_val, true );
            return false;
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

ContextWrapper RadarTypeGroupContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( isRootElement() ) switch( nElement )
    {
        case C_TOKEN( axId ):
            mrModel.maAxisIds.push_back( rAttribs.getInteger( XML_val, -1 ) );
            return false;
        case C_TOKEN( dLbls ):
            return new DataLabelsContext( *this, mrModel.mxLabels.create() );
        case C_TOKEN( radarStyle ):
            mrModel.mnRadarStyle = rAttribs.getToken( XML_val, XML_standard );
            return false;
        case C_TOKEN( ser ):
            return new RadarSeriesContext( *this, mrModel.maSeries.create() );
        case C_TOKEN( varyColors ):
            mrModel.mbVaryColors = rAttribs.getBool( XML_val, true );
            return false;
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

ContextWrapper ScatterTypeGroupContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( isRootElement() ) switch( nElement )
    {
        case C_TOKEN( axId ):
            mrModel.maAxisIds.push_back( rAttribs.getInteger( XML_val, -1 ) );
            return false;
        case C_TOKEN( dLbls ):
            return new DataLabelsContext( *this, mrModel.mxLabels.create() );
        case C_TOKEN( scatterStyle ):
            mrModel.mnScatterStyle = rAttribs.getInteger( XML_val, XML_marker );
            return false;
        case C_TOKEN( ser ):
            return new ScatterSeriesContext( *this, mrModel.maSeries.create() );
        case C_TOKEN( varyColors ):
            mrModel.mbVaryColors = rAttribs.getBool( XML_val, true );
            return false;
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

ContextWrapper SurfaceTypeGroupContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( isRootElement() ) switch( nElement )
    {
        case C_TOKEN( axId ):
            mrModel.maAxisIds.push_back( rAttribs.getInteger( XML_val, -1 ) );
            return false;
        case C_TOKEN( ser ):
            return new SurfaceSeriesContext( *this, mrModel.maSeries.create() );
        case C_TOKEN( wireframe ):
            mrModel.mbWireframe = rAttribs.getBool( XML_val, true );
            return false;
    }
    return false;
}

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

