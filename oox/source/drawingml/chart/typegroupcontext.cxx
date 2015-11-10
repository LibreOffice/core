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

#include "drawingml/chart/typegroupcontext.hxx"

#include "drawingml/chart/seriescontext.hxx"
#include "drawingml/chart/typegroupmodel.hxx"

namespace oox {
namespace drawingml {
namespace chart {

using ::oox::core::ContextHandler2Helper;
using ::oox::core::ContextHandlerRef;

UpDownBarsContext::UpDownBarsContext( ContextHandler2Helper& rParent, UpDownBarsModel& rModel ) :
    ContextBase< UpDownBarsModel >( rParent, rModel )
{
}

UpDownBarsContext::~UpDownBarsContext()
{
}

ContextHandlerRef UpDownBarsContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
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
                    return nullptr;
                case C_TOKEN( upBars ):
                    return new ShapePrWrapperContext( *this, mrModel.mxUpBars.create() );
            }
        break;
    }
    return nullptr;
}

AreaTypeGroupContext::AreaTypeGroupContext( ContextHandler2Helper& rParent, TypeGroupModel& rModel ) :
    TypeGroupContextBase( rParent, rModel )
{
}

AreaTypeGroupContext::~AreaTypeGroupContext()
{
}

ContextHandlerRef AreaTypeGroupContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    bool bMSO2007Doc = getFilter().isMSO2007Document();
    if( isRootElement() ) switch( nElement )
    {
        case C_TOKEN( axId ):
            mrModel.maAxisIds.push_back( rAttribs.getInteger( XML_val, -1 ) );
            return nullptr;
        case C_TOKEN( dLbls ):
            return new DataLabelsContext( *this, mrModel.mxLabels.create(bMSO2007Doc) );
        case C_TOKEN( dropLines ):
            return new ShapePrWrapperContext( *this, mrModel.mxDropLines.create() );
        case C_TOKEN( gapDepth ):
            mrModel.mnGapDepth = rAttribs.getInteger( XML_val, 150 );
            return nullptr;
        case C_TOKEN( grouping ):
            mrModel.mnGrouping = rAttribs.getToken( XML_val, bMSO2007Doc ? XML_standard : XML_clustered );
            return nullptr;
        case C_TOKEN( ser ):
            return new AreaSeriesContext( *this, mrModel.maSeries.create(bMSO2007Doc) );
        case C_TOKEN( varyColors ):
            mrModel.mbVaryColors = rAttribs.getBool( XML_val, !bMSO2007Doc );
            return nullptr;
    }
    return nullptr;
}

BarTypeGroupContext::BarTypeGroupContext( ContextHandler2Helper& rParent, TypeGroupModel& rModel ) :
    TypeGroupContextBase( rParent, rModel )
{
}

BarTypeGroupContext::~BarTypeGroupContext()
{
}

ContextHandlerRef BarTypeGroupContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    bool bMSO2007Doc = getFilter().isMSO2007Document();
    if( isRootElement() ) switch( nElement )
    {
        case C_TOKEN( axId ):
            mrModel.maAxisIds.push_back( rAttribs.getInteger( XML_val, -1 ) );
            return nullptr;
        case C_TOKEN( barDir ):
            mrModel.mnBarDir = rAttribs.getToken( XML_val, XML_col );
            return nullptr;
        case C_TOKEN( dLbls ):
            return new DataLabelsContext( *this, mrModel.mxLabels.create(bMSO2007Doc) );
        case C_TOKEN( gapDepth ):
            mrModel.mnGapDepth = rAttribs.getInteger( XML_val, 150 );
            return nullptr;
        case C_TOKEN( gapWidth ):
            mrModel.mnGapWidth = rAttribs.getInteger( XML_val, 150 );
            return nullptr;
        case C_TOKEN( grouping ):
            mrModel.mnGrouping = rAttribs.getToken( XML_val, bMSO2007Doc ? XML_standard : XML_clustered );
            return nullptr;
        case C_TOKEN( overlap ):
            mrModel.mnOverlap = rAttribs.getInteger( XML_val, 0 );
            return nullptr;
        case C_TOKEN( ser ):
            return new BarSeriesContext( *this, mrModel.maSeries.create(bMSO2007Doc) );
        case C_TOKEN( serLines ):
            return new ShapePrWrapperContext( *this, mrModel.mxSerLines.create() );
        case C_TOKEN( shape ):
            mrModel.mnShape = rAttribs.getToken( XML_val, XML_box );
            return nullptr;
        case C_TOKEN( varyColors ):
            mrModel.mbVaryColors = rAttribs.getBool( XML_val, !bMSO2007Doc );
            return nullptr;
    }
    return nullptr;
}

BubbleTypeGroupContext::BubbleTypeGroupContext( ContextHandler2Helper& rParent, TypeGroupModel& rModel ) :
    TypeGroupContextBase( rParent, rModel )
{
}

BubbleTypeGroupContext::~BubbleTypeGroupContext()
{
}

ContextHandlerRef BubbleTypeGroupContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    bool bMSO2007Doc = getFilter().isMSO2007Document();
    if( isRootElement() ) switch( nElement )
    {
        case C_TOKEN( axId ):
            mrModel.maAxisIds.push_back( rAttribs.getInteger( XML_val, -1 ) );
            return nullptr;
        case C_TOKEN( bubble3D ):
            mrModel.mbBubble3d = rAttribs.getBool( XML_val, !bMSO2007Doc );
            return nullptr;
        case C_TOKEN( bubbleScale ):
            mrModel.mnBubbleScale = rAttribs.getInteger( XML_val, 100 );
            return nullptr;
        case C_TOKEN( dLbls ):
            return new DataLabelsContext( *this, mrModel.mxLabels.create(bMSO2007Doc) );
        case C_TOKEN( ser ):
            return new BubbleSeriesContext( *this, mrModel.maSeries.create(bMSO2007Doc) );
        case C_TOKEN( showNegBubbles ):
            mrModel.mbShowNegBubbles = rAttribs.getBool( XML_val, !bMSO2007Doc );
            return nullptr;
        case C_TOKEN( sizeRepresents ):
            mrModel.mnSizeRepresents = rAttribs.getToken( XML_val, XML_area );
            return nullptr;
        case C_TOKEN( varyColors ):
            mrModel.mbVaryColors = rAttribs.getBool( XML_val, !bMSO2007Doc );
            return nullptr;
    }
    return nullptr;
}

LineTypeGroupContext::LineTypeGroupContext( ContextHandler2Helper& rParent, TypeGroupModel& rModel ) :
    TypeGroupContextBase( rParent, rModel )
{
}

LineTypeGroupContext::~LineTypeGroupContext()
{
}

ContextHandlerRef LineTypeGroupContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    bool bMSO2007Doc = getFilter().isMSO2007Document();
    if( isRootElement() ) switch( nElement )
    {
        case C_TOKEN( axId ):
            mrModel.maAxisIds.push_back( rAttribs.getInteger( XML_val, -1 ) );
            return nullptr;
        case C_TOKEN( dLbls ):
            return new DataLabelsContext( *this, mrModel.mxLabels.create(bMSO2007Doc) );
        case C_TOKEN( dropLines ):
            return new ShapePrWrapperContext( *this, mrModel.mxDropLines.create() );
        case C_TOKEN( gapDepth ):
            mrModel.mnGapDepth = rAttribs.getInteger( XML_val, 150 );
            return nullptr;
        case C_TOKEN( grouping ):
            mrModel.mnGrouping = rAttribs.getToken( XML_val, bMSO2007Doc ? XML_standard : XML_clustered );
            return nullptr;
        case C_TOKEN( hiLowLines ):
            return new ShapePrWrapperContext( *this, mrModel.mxHiLowLines.create() );
        case C_TOKEN( marker ):
            mrModel.mbShowMarker = rAttribs.getBool( XML_val, !bMSO2007Doc );
            return nullptr;
        case C_TOKEN( ser ):
            return new LineSeriesContext( *this, mrModel.maSeries.create(bMSO2007Doc) );
        case C_TOKEN( smooth ):
            mrModel.mbSmooth = rAttribs.getBool( XML_val, !bMSO2007Doc );
            return nullptr;
        case C_TOKEN( upDownBars ):
            return new UpDownBarsContext( *this, mrModel.mxUpDownBars.create() );
        case C_TOKEN( varyColors ):
            mrModel.mbVaryColors = rAttribs.getBool( XML_val, !bMSO2007Doc );
            return nullptr;
    }
    return nullptr;
}

PieTypeGroupContext::PieTypeGroupContext( ContextHandler2Helper& rParent, TypeGroupModel& rModel ) :
    TypeGroupContextBase( rParent, rModel )
{
}

PieTypeGroupContext::~PieTypeGroupContext()
{
}

ContextHandlerRef PieTypeGroupContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    bool bMSO2007Doc = getFilter().isMSO2007Document();
    if( isRootElement() ) switch( nElement )
    {
        case C_TOKEN( dLbls ):
            return new DataLabelsContext( *this, mrModel.mxLabels.create(bMSO2007Doc) );
        case C_TOKEN( firstSliceAng ):
            mrModel.mnFirstAngle = rAttribs.getInteger( XML_val, 0 );
            return nullptr;
        case C_TOKEN( gapWidth ):
            mrModel.mnGapWidth = rAttribs.getInteger( XML_val, 150 );
            return nullptr;
        case C_TOKEN( holeSize ):
            mrModel.mnHoleSize = rAttribs.getInteger( XML_val, 10 );
            return nullptr;
        case C_TOKEN( ofPieType ):
            mrModel.mnOfPieType = rAttribs.getToken( XML_val, XML_pie );
            return nullptr;
        case C_TOKEN( secondPieSize ):
            mrModel.mnSecondPieSize = rAttribs.getInteger( XML_val, 75 );
            return nullptr;
        case C_TOKEN( ser ):
            return new PieSeriesContext( *this, mrModel.maSeries.create(bMSO2007Doc) );
        case C_TOKEN( serLines ):
            return new ShapePrWrapperContext( *this, mrModel.mxSerLines.create() );
        case C_TOKEN( splitPos ):
            mrModel.mfSplitPos = rAttribs.getDouble( XML_val, 0.0 );
            return nullptr;
        case C_TOKEN( splitType ):
            mrModel.mnSplitType = rAttribs.getToken( XML_val, XML_auto );
            return nullptr;
        case C_TOKEN( varyColors ):
            mrModel.mbVaryColors = rAttribs.getBool( XML_val, !bMSO2007Doc );
            return nullptr;
    }
    return nullptr;
}

RadarTypeGroupContext::RadarTypeGroupContext( ContextHandler2Helper& rParent, TypeGroupModel& rModel ) :
    TypeGroupContextBase( rParent, rModel )
{
}

RadarTypeGroupContext::~RadarTypeGroupContext()
{
}

ContextHandlerRef RadarTypeGroupContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    bool bMSO2007Doc = getFilter().isMSO2007Document();
    if( isRootElement() ) switch( nElement )
    {
        case C_TOKEN( axId ):
            mrModel.maAxisIds.push_back( rAttribs.getInteger( XML_val, -1 ) );
            return nullptr;
        case C_TOKEN( dLbls ):
            return new DataLabelsContext( *this, mrModel.mxLabels.create(bMSO2007Doc) );
        case C_TOKEN( radarStyle ):
            mrModel.mnRadarStyle = rAttribs.getToken( XML_val, XML_standard );
            return nullptr;
        case C_TOKEN( ser ):
            return new RadarSeriesContext( *this, mrModel.maSeries.create(bMSO2007Doc) );
        case C_TOKEN( varyColors ):
            mrModel.mbVaryColors = rAttribs.getBool( XML_val, !bMSO2007Doc );
            return nullptr;
    }
    return nullptr;
}

ScatterTypeGroupContext::ScatterTypeGroupContext( ContextHandler2Helper& rParent, TypeGroupModel& rModel ) :
    TypeGroupContextBase( rParent, rModel )
{
}

ScatterTypeGroupContext::~ScatterTypeGroupContext()
{
}

ContextHandlerRef ScatterTypeGroupContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    bool bMSO2007Doc = getFilter().isMSO2007Document();
    if( isRootElement() ) switch( nElement )
    {
        case C_TOKEN( axId ):
            mrModel.maAxisIds.push_back( rAttribs.getInteger( XML_val, -1 ) );
            return nullptr;
        case C_TOKEN( dLbls ):
            return new DataLabelsContext( *this, mrModel.mxLabels.create(bMSO2007Doc) );
        case C_TOKEN( scatterStyle ):
            mrModel.mnScatterStyle = rAttribs.getInteger( XML_val, XML_marker );
            return nullptr;
        case C_TOKEN( ser ):
            return new ScatterSeriesContext( *this, mrModel.maSeries.create(bMSO2007Doc) );
        case C_TOKEN( varyColors ):
            mrModel.mbVaryColors = rAttribs.getBool( XML_val, !bMSO2007Doc );
            return nullptr;
    }
    return nullptr;
}

SurfaceTypeGroupContext::SurfaceTypeGroupContext( ContextHandler2Helper& rParent, TypeGroupModel& rModel ) :
    TypeGroupContextBase( rParent, rModel )
{
}

SurfaceTypeGroupContext::~SurfaceTypeGroupContext()
{
}

ContextHandlerRef SurfaceTypeGroupContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    bool bMSO2007Doc = getFilter().isMSO2007Document();
    if( isRootElement() ) switch( nElement )
    {
        case C_TOKEN( axId ):
            mrModel.maAxisIds.push_back( rAttribs.getInteger( XML_val, -1 ) );
            return nullptr;
        case C_TOKEN( ser ):
            return new SurfaceSeriesContext( *this, mrModel.maSeries.create(bMSO2007Doc) );
        case C_TOKEN( wireframe ):
            mrModel.mbWireframe = rAttribs.getBool( XML_val, !bMSO2007Doc );
            return nullptr;
    }
    return nullptr;
}

} // namespace chart
} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
