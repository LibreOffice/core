/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: typegroupcontext.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:37:03 $
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

