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

#include "drawingml/chart/seriescontext.hxx"

#include "drawingml/shapepropertiescontext.hxx"
#include "drawingml/textbodycontext.hxx"
#include "drawingml/chart/datasourcecontext.hxx"
#include "drawingml/chart/seriesmodel.hxx"
#include "drawingml/chart/titlecontext.hxx"
#include <oox/core/xmlfilterbase.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>

namespace oox {
namespace drawingml {
namespace chart {

using ::oox::core::ContextHandler2;
using ::oox::core::ContextHandler2Helper;
using ::oox::core::ContextHandlerRef;

namespace {

ContextHandlerRef lclDataLabelSharedCreateContext( ContextHandler2& rContext,
        sal_Int32 nElement, const AttributeList& rAttribs, DataLabelModelBase& orModel, bool bMSO2007 )
{
    if( rContext.isRootElement() ) switch( nElement )
    {
        case C_TOKEN( delete ):
            orModel.mbDeleted = rAttribs.getBool( XML_val, !bMSO2007 );
            return nullptr;
        case C_TOKEN( dLblPos ):
            orModel.monLabelPos = rAttribs.getToken( XML_val, XML_TOKEN_INVALID );
            return nullptr;
        case C_TOKEN( numFmt ):
            orModel.maNumberFormat.setAttributes( rAttribs );
            return nullptr;
        case C_TOKEN( showBubbleSize ):
            orModel.mobShowBubbleSize = rAttribs.getBool( XML_val );
            return nullptr;
        case C_TOKEN( showCatName ):
            orModel.mobShowCatName = rAttribs.getBool( XML_val );
            return nullptr;
        case C_TOKEN( showLegendKey ):
            orModel.mobShowLegendKey = rAttribs.getBool( XML_val );
            return nullptr;
        case C_TOKEN( showPercent ):
            orModel.mobShowPercent = rAttribs.getBool( XML_val );
            return nullptr;
        case C_TOKEN( showSerName ):
            orModel.mobShowSerName = rAttribs.getBool( XML_val );
            return nullptr;
        case C_TOKEN( showVal ):
            orModel.mobShowVal = rAttribs.getBool( XML_val );
            return nullptr;
        case C_TOKEN( separator ):
            // collect separator text in onCharacters()
            return &rContext;
        case C_TOKEN( spPr ):
            return new ShapePropertiesContext( rContext, orModel.mxShapeProp.create() );
        case C_TOKEN( txPr ):
            return new TextBodyContext( rContext, orModel.mxTextProp.create() );
    }
    return nullptr;
}

void lclDataLabelSharedCharacters( ContextHandler2& rContext, const OUString& rChars, DataLabelModelBase& orModel )
{
    if( rContext.isCurrentElement( C_TOKEN( separator ) ) )
        orModel.moaSeparator = rChars;
}

} // namespace

DataLabelContext::DataLabelContext( ContextHandler2Helper& rParent, DataLabelModel& rModel ) :
    ContextBase< DataLabelModel >( rParent, rModel )
{
}

DataLabelContext::~DataLabelContext()
{
}

ContextHandlerRef DataLabelContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    mrModel.mbDeleted = false;
    if( isRootElement() ) switch( nElement )
    {
        case C_TOKEN( idx ):
            mrModel.mnIndex = rAttribs.getInteger( XML_val, -1 );
            return nullptr;
        case C_TOKEN( layout ):
            return new LayoutContext( *this, mrModel.mxLayout.create() );
        case C_TOKEN( tx ):
            return new TextContext( *this, mrModel.mxText.create() );
    }
    bool bMSO2007 = getFilter().isMSO2007Document();
    return lclDataLabelSharedCreateContext( *this, nElement, rAttribs, mrModel, bMSO2007 );
}

void DataLabelContext::onCharacters( const OUString& rChars )
{
    lclDataLabelSharedCharacters( *this, rChars, mrModel );
}

DataLabelsContext::DataLabelsContext( ContextHandler2Helper& rParent, DataLabelsModel& rModel ) :
    ContextBase< DataLabelsModel >( rParent, rModel )
{
}

DataLabelsContext::~DataLabelsContext()
{
}

ContextHandlerRef DataLabelsContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    mrModel.mbDeleted = false;
    bool bMSO2007Doc = getFilter().isMSO2007Document();
    if( isRootElement() ) switch( nElement )
    {
        case C_TOKEN( dLbl ):
            return new DataLabelContext( *this, mrModel.maPointLabels.create(bMSO2007Doc) );
        case C_TOKEN( leaderLines ):
            return new ShapePrWrapperContext( *this, mrModel.mxLeaderLines.create() );
        case C_TOKEN( showLeaderLines ):
            mrModel.mbShowLeaderLines = rAttribs.getBool( XML_val, !bMSO2007Doc );
            return nullptr;
    }
    return lclDataLabelSharedCreateContext( *this, nElement, rAttribs, mrModel, bMSO2007Doc );
}

void DataLabelsContext::onCharacters( const OUString& rChars )
{
    lclDataLabelSharedCharacters( *this, rChars, mrModel );
}

PictureOptionsContext::PictureOptionsContext( ContextHandler2Helper& rParent, PictureOptionsModel& rModel ) :
    ContextBase< PictureOptionsModel >( rParent, rModel )
{
}

PictureOptionsContext::~PictureOptionsContext()
{
}

ContextHandlerRef PictureOptionsContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    bool bMSO2007Doc = getFilter().isMSO2007Document();
    if( isRootElement() ) switch( nElement )
    {
        case C_TOKEN( applyToEnd ):
            mrModel.mbApplyToEnd = rAttribs.getBool( XML_val, !bMSO2007Doc );
            return nullptr;
        case C_TOKEN( applyToFront ):
            mrModel.mbApplyToFront = rAttribs.getBool( XML_val, !bMSO2007Doc );
            return nullptr;
        case C_TOKEN( applyToSides ):
            mrModel.mbApplyToSides = rAttribs.getBool( XML_val, !bMSO2007Doc );
            return nullptr;
        case C_TOKEN( pictureFormat ):
            mrModel.mnPictureFormat = rAttribs.getToken( XML_val, XML_stretch );
            return nullptr;
        case C_TOKEN( pictureStackUnit ):
            mrModel.mfStackUnit = rAttribs.getDouble( XML_val, 1.0 );
            return nullptr;
    }
    return nullptr;
}

ErrorBarContext::ErrorBarContext( ContextHandler2Helper& rParent, ErrorBarModel& rModel ) :
    ContextBase< ErrorBarModel >( rParent, rModel )
{
}

ErrorBarContext::~ErrorBarContext()
{
}

ContextHandlerRef ErrorBarContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    bool bMSO2007Doc = getFilter().isMSO2007Document();
    if( isRootElement() ) switch( nElement )
    {
        case C_TOKEN( errBarType ):
            mrModel.mnTypeId = rAttribs.getToken( XML_val, XML_both );
            return nullptr;
        case C_TOKEN( errDir ):
            mrModel.mnDirection = rAttribs.getToken( XML_val, XML_TOKEN_INVALID );
            return nullptr;
        case C_TOKEN( errValType ):
            mrModel.mnValueType = rAttribs.getToken( XML_val, XML_fixedVal );
            return nullptr;
        case C_TOKEN( minus ):
            return new DataSourceContext( *this, mrModel.maSources.create( ErrorBarModel::MINUS ) );
        case C_TOKEN( noEndCap ):
            mrModel.mbNoEndCap = rAttribs.getBool( XML_val, !bMSO2007Doc );
            return nullptr;
        case C_TOKEN( plus ):
            return new DataSourceContext( *this, mrModel.maSources.create( ErrorBarModel::PLUS ) );
        case C_TOKEN( spPr ):
            return new ShapePropertiesContext( *this, mrModel.mxShapeProp.create() );
        case C_TOKEN( val ):
            mrModel.mfValue = rAttribs.getDouble( XML_val, 0.0 );
            return nullptr;
    }
    return nullptr;
}

TrendlineLabelContext::TrendlineLabelContext( ContextHandler2Helper& rParent, TrendlineLabelModel& rModel ) :
    ContextBase< TrendlineLabelModel >( rParent, rModel )
{
}

TrendlineLabelContext::~TrendlineLabelContext()
{
}

ContextHandlerRef TrendlineLabelContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( isRootElement() ) switch( nElement )
    {
        case C_TOKEN( layout ):
            return new LayoutContext( *this, mrModel.mxLayout.create() );
        case C_TOKEN( numFmt ):
            mrModel.maNumberFormat.setAttributes( rAttribs );
            return nullptr;
        case C_TOKEN( spPr ):
            return new ShapePropertiesContext( *this, mrModel.mxShapeProp.create() );
        case C_TOKEN( tx ):
            return new TextContext( *this, mrModel.mxText.create() );
        case C_TOKEN( txPr ):
            return new TextBodyContext( *this, mrModel.mxTextProp.create() );
    }
    return nullptr;
}

TrendlineContext::TrendlineContext( ContextHandler2Helper& rParent, TrendlineModel& rModel ) :
    ContextBase< TrendlineModel >( rParent, rModel )
{
}

TrendlineContext::~TrendlineContext()
{
}

ContextHandlerRef TrendlineContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    bool bMSO2007Doc = getFilter().isMSO2007Document();
    if( isRootElement() ) switch( nElement )
    {
        case C_TOKEN( backward ):
            mrModel.mfBackward = rAttribs.getDouble( XML_val, 0.0 );
            return nullptr;
        case C_TOKEN( dispEq ):
            mrModel.mbDispEquation = rAttribs.getBool( XML_val, !bMSO2007Doc );
            return nullptr;
        case C_TOKEN( dispRSqr ):
            mrModel.mbDispRSquared = rAttribs.getBool( XML_val, !bMSO2007Doc );
            return nullptr;
        case C_TOKEN( forward ):
            mrModel.mfForward = rAttribs.getDouble( XML_val, 0.0 );
            return nullptr;
        case C_TOKEN( intercept ):
            mrModel.mfIntercept = rAttribs.getDouble( XML_val, 0.0 );
            return nullptr;
        case C_TOKEN( name ):
            return this;    // collect name in onCharacters()
        case C_TOKEN( order ):
            mrModel.mnOrder = rAttribs.getInteger( XML_val, 2 );
            return nullptr;
        case C_TOKEN( period ):
            mrModel.mnPeriod = rAttribs.getInteger( XML_val, 2 );
            return nullptr;
        case C_TOKEN( spPr ):
            return new ShapePropertiesContext( *this, mrModel.mxShapeProp.create() );
        case C_TOKEN( trendlineLbl ):
            return new TrendlineLabelContext( *this, mrModel.mxLabel.create() );
        case C_TOKEN( trendlineType ):
            mrModel.mnTypeId = rAttribs.getToken( XML_val, XML_linear );
            return nullptr;
    }
    return nullptr;
}

void TrendlineContext::onCharacters( const OUString& rChars )
{
    if( isCurrentElement( C_TOKEN( name ) ) )
        mrModel.maName = rChars;
}

DataPointContext::DataPointContext( ContextHandler2Helper& rParent, DataPointModel& rModel ) :
    ContextBase< DataPointModel >( rParent, rModel )
{
}

DataPointContext::~DataPointContext()
{
}

ContextHandlerRef DataPointContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    bool bMSO2007Doc = getFilter().isMSO2007Document();
    switch( getCurrentElement() )
    {
        case C_TOKEN( dPt ):
            switch( nElement )
            {
                case C_TOKEN( bubble3D ):
                    mrModel.mobBubble3d = rAttribs.getBool( XML_val );
                    return nullptr;
                case C_TOKEN( explosion ):
                    // if the 'val' attribute is missing, series explosion remains unchanged
                    mrModel.monExplosion = rAttribs.getInteger( XML_val );
                    return nullptr;
                case C_TOKEN( idx ):
                    mrModel.mnIndex = rAttribs.getInteger( XML_val, -1 );
                    return nullptr;
                case C_TOKEN( invertIfNegative ):
                    mrModel.mbInvertNeg = rAttribs.getBool( XML_val, !bMSO2007Doc );
                    return nullptr;
                case C_TOKEN( marker ):
                    return this;
                case C_TOKEN( pictureOptions ):
                    return new PictureOptionsContext( *this, mrModel.mxPicOptions.create(bMSO2007Doc) );
                case C_TOKEN( spPr ):
                    return new ShapePropertiesContext( *this, mrModel.mxShapeProp.create() );
            }
        break;

        case C_TOKEN( marker ):
            switch( nElement )
            {
                case C_TOKEN( size ):
                    mrModel.monMarkerSize = rAttribs.getInteger( XML_val, 5 );
                    return nullptr;
                case C_TOKEN( spPr ):
                    return new ShapePropertiesContext( *this, mrModel.mxMarkerProp.create() );
                case C_TOKEN( symbol ):
                    mrModel.monMarkerSymbol = rAttribs.getToken( XML_val, XML_none );
                    return nullptr;
            }
        break;
    }
    return nullptr;
}

SeriesContextBase::SeriesContextBase( ContextHandler2Helper& rParent, SeriesModel& rModel ) :
    ContextBase< SeriesModel >( rParent, rModel )
{
}

SeriesContextBase::~SeriesContextBase()
{
}

ContextHandlerRef SeriesContextBase::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case C_TOKEN( ser ):
            switch( nElement )
            {
                case C_TOKEN( idx ):
                    mrModel.mnIndex = rAttribs.getInteger( XML_val, -1 );
                    return nullptr;
                case C_TOKEN( order ):
                    mrModel.mnOrder = rAttribs.getInteger( XML_val, -1 );
                    return nullptr;
                case C_TOKEN( spPr ):
                    return new ShapePropertiesContext( *this, mrModel.mxShapeProp.create() );
                case C_TOKEN( tx ):
                    return new TextContext( *this, mrModel.mxText.create() );
            }
        break;

        case C_TOKEN( marker ):
            switch( nElement )
            {
                case C_TOKEN( size ):
                    mrModel.mnMarkerSize = rAttribs.getInteger( XML_val, 5 );
                    return nullptr;
                case C_TOKEN( spPr ):
                    return new ShapePropertiesContext( *this, mrModel.mxMarkerProp.create() );
                case C_TOKEN( symbol ):
                    mrModel.mnMarkerSymbol = rAttribs.getToken( XML_val, XML_none );
                    return nullptr;
            }
        break;
    }
    return nullptr;
}

AreaSeriesContext::AreaSeriesContext( ContextHandler2Helper& rParent, SeriesModel& rModel ) :
    SeriesContextBase( rParent, rModel )
{
}

AreaSeriesContext::~AreaSeriesContext()
{
}

ContextHandlerRef AreaSeriesContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    bool bMSO2007Doc = getFilter().isMSO2007Document();
    switch( getCurrentElement() )
    {
        case C_TOKEN( ser ):
            switch( nElement )
            {
                case C_TOKEN( cat ):
                    return new DataSourceContext( *this, mrModel.maSources.create( SeriesModel::CATEGORIES ) );
                case C_TOKEN( errBars ):
                    return new ErrorBarContext( *this, mrModel.maErrorBars.create(bMSO2007Doc) );
                case C_TOKEN( dLbls ):
                    return new DataLabelsContext( *this, mrModel.mxLabels.create(bMSO2007Doc) );
                case C_TOKEN( dPt ):
                    return new DataPointContext( *this, mrModel.maPoints.create(bMSO2007Doc) );
                case C_TOKEN( trendline ):
                    return new TrendlineContext( *this, mrModel.maTrendlines.create(bMSO2007Doc) );
                case C_TOKEN( val ):
                    return new DataSourceContext( *this, mrModel.maSources.create( SeriesModel::VALUES ) );
            }
        break;
    }
    return SeriesContextBase::onCreateContext( nElement, rAttribs );
}

BarSeriesContext::BarSeriesContext( ContextHandler2Helper& rParent, SeriesModel& rModel ) :
    SeriesContextBase( rParent, rModel )
{
}

BarSeriesContext::~BarSeriesContext()
{
}

ContextHandlerRef BarSeriesContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    bool bMSO2007Doc = getFilter().isMSO2007Document();
    switch( getCurrentElement() )
    {
        case C_TOKEN( ser ):
            switch( nElement )
            {
                case C_TOKEN( cat ):
                    return new DataSourceContext( *this, mrModel.maSources.create( SeriesModel::CATEGORIES ) );
                case C_TOKEN( dLbls ):
                    return new DataLabelsContext( *this, mrModel.mxLabels.create(bMSO2007Doc) );
                case C_TOKEN( dPt ):
                    return new DataPointContext( *this, mrModel.maPoints.create(bMSO2007Doc) );
                case C_TOKEN( errBars ):
                    return new ErrorBarContext( *this, mrModel.maErrorBars.create(bMSO2007Doc) );
                case C_TOKEN( invertIfNegative ):
                    mrModel.mbInvertNeg = rAttribs.getBool( XML_val, !bMSO2007Doc );
                    return nullptr;
                case C_TOKEN( pictureOptions ):
                    return new PictureOptionsContext( *this, mrModel.mxPicOptions.create(bMSO2007Doc) );
                case C_TOKEN( shape ):
                    mrModel.monShape = rAttribs.getToken( bMSO2007Doc ? XML_val : XML_box );
                    return nullptr;
                case C_TOKEN( trendline ):
                    return new TrendlineContext( *this, mrModel.maTrendlines.create(bMSO2007Doc) );
                case C_TOKEN( val ):
                    return new DataSourceContext( *this, mrModel.maSources.create( SeriesModel::VALUES ) );
            }
        break;
    }
    return SeriesContextBase::onCreateContext( nElement, rAttribs );
}

BubbleSeriesContext::BubbleSeriesContext( ContextHandler2Helper& rParent, SeriesModel& rModel ) :
    SeriesContextBase( rParent, rModel )
{
}

BubbleSeriesContext::~BubbleSeriesContext()
{
}

ContextHandlerRef BubbleSeriesContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    bool bMSO2007Doc = getFilter().isMSO2007Document();
    switch( getCurrentElement() )
    {
        case C_TOKEN( ser ):
            switch( nElement )
            {
                case C_TOKEN( bubble3D ):
                    mrModel.mbBubble3d = rAttribs.getBool( XML_val, !bMSO2007Doc );
                    return nullptr;
                case C_TOKEN( bubbleSize ):
                    return new DataSourceContext( *this, mrModel.maSources.create( SeriesModel::POINTS ) );
                case C_TOKEN( dLbls ):
                    return new DataLabelsContext( *this, mrModel.mxLabels.create(bMSO2007Doc) );
                case C_TOKEN( dPt ):
                    return new DataPointContext( *this, mrModel.maPoints.create(bMSO2007Doc) );
                case C_TOKEN( errBars ):
                    return new ErrorBarContext( *this, mrModel.maErrorBars.create(bMSO2007Doc) );
                case C_TOKEN( invertIfNegative ):
                    mrModel.mbInvertNeg = rAttribs.getBool( XML_val, !bMSO2007Doc );
                    return nullptr;
                case C_TOKEN( trendline ):
                    return new TrendlineContext( *this, mrModel.maTrendlines.create(bMSO2007Doc) );
                case C_TOKEN( xVal ):
                    return new DataSourceContext( *this, mrModel.maSources.create( SeriesModel::CATEGORIES ) );
                case C_TOKEN( yVal ):
                    return new DataSourceContext( *this, mrModel.maSources.create( SeriesModel::VALUES ) );
            }
        break;
    }
    return SeriesContextBase::onCreateContext( nElement, rAttribs );
}

LineSeriesContext::LineSeriesContext( ContextHandler2Helper& rParent, SeriesModel& rModel ) :
    SeriesContextBase( rParent, rModel )
{
}

LineSeriesContext::~LineSeriesContext()
{
}

ContextHandlerRef LineSeriesContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    bool bMSO2007Doc = getFilter().isMSO2007Document();
    switch( getCurrentElement() )
    {
        case C_TOKEN( ser ):
            switch( nElement )
            {
                case C_TOKEN( cat ):
                    return new DataSourceContext( *this, mrModel.maSources.create( SeriesModel::CATEGORIES ) );
                case C_TOKEN( dLbls ):
                    return new DataLabelsContext( *this, mrModel.mxLabels.create(bMSO2007Doc) );
                case C_TOKEN( dPt ):
                    return new DataPointContext( *this, mrModel.maPoints.create(bMSO2007Doc) );
                case C_TOKEN( errBars ):
                    return new ErrorBarContext( *this, mrModel.maErrorBars.create(bMSO2007Doc) );
                case C_TOKEN( marker ):
                    return this;
                case C_TOKEN( smooth ):
                    mrModel.mbSmooth = rAttribs.getBool( XML_val, !bMSO2007Doc );
                    return nullptr;
                case C_TOKEN( trendline ):
                    return new TrendlineContext( *this, mrModel.maTrendlines.create(bMSO2007Doc) );
                case C_TOKEN( val ):
                    return new DataSourceContext( *this, mrModel.maSources.create( SeriesModel::VALUES ) );
            }
        break;
    }
    return SeriesContextBase::onCreateContext( nElement, rAttribs );
}

PieSeriesContext::PieSeriesContext( ContextHandler2Helper& rParent, SeriesModel& rModel ) :
    SeriesContextBase( rParent, rModel )
{
}

PieSeriesContext::~PieSeriesContext()
{
}

ContextHandlerRef PieSeriesContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    bool bMSO2007Doc = getFilter().isMSO2007Document();
    switch( getCurrentElement() )
    {
        case C_TOKEN( ser ):
            switch( nElement )
            {
                case C_TOKEN( cat ):
                    return new DataSourceContext( *this, mrModel.maSources.create( SeriesModel::CATEGORIES ) );
                case C_TOKEN( dLbls ):
                    return new DataLabelsContext( *this, mrModel.mxLabels.create(bMSO2007Doc) );
                case C_TOKEN( dPt ):
                    return new DataPointContext( *this, mrModel.maPoints.create(bMSO2007Doc) );
                case C_TOKEN( explosion ):
                    mrModel.mnExplosion = rAttribs.getInteger( XML_val, 0 );
                    return nullptr;
                case C_TOKEN( val ):
                    return new DataSourceContext( *this, mrModel.maSources.create( SeriesModel::VALUES ) );
            }
        break;
    }
    return SeriesContextBase::onCreateContext( nElement, rAttribs );
}

RadarSeriesContext::RadarSeriesContext( ContextHandler2Helper& rParent, SeriesModel& rModel ) :
    SeriesContextBase( rParent, rModel )
{
}

RadarSeriesContext::~RadarSeriesContext()
{
}

ContextHandlerRef RadarSeriesContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    bool bMSO2007Doc = getFilter().isMSO2007Document();
    switch( getCurrentElement() )
    {
        case C_TOKEN( ser ):
            switch( nElement )
            {
                case C_TOKEN( cat ):
                    return new DataSourceContext( *this, mrModel.maSources.create( SeriesModel::CATEGORIES ) );
                case C_TOKEN( dLbls ):
                    return new DataLabelsContext( *this, mrModel.mxLabels.create(bMSO2007Doc) );
                case C_TOKEN( dPt ):
                    return new DataPointContext( *this, mrModel.maPoints.create(bMSO2007Doc) );
                case C_TOKEN( marker ):
                    return this;
                case C_TOKEN( smooth ):
                    mrModel.mbSmooth = rAttribs.getBool( XML_val, bMSO2007Doc );
                    return nullptr;
                case C_TOKEN( val ):
                    return new DataSourceContext( *this, mrModel.maSources.create( SeriesModel::VALUES ) );
            }
        break;
    }
    return SeriesContextBase::onCreateContext( nElement, rAttribs );
}

ScatterSeriesContext::ScatterSeriesContext( ContextHandler2Helper& rParent, SeriesModel& rModel ) :
    SeriesContextBase( rParent, rModel )
{
}

ScatterSeriesContext::~ScatterSeriesContext()
{
}

ContextHandlerRef ScatterSeriesContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    bool bMSO2007Doc = getFilter().isMSO2007Document();
    switch( getCurrentElement() )
    {
        case C_TOKEN( ser ):
            switch( nElement )
            {
                case C_TOKEN( dLbls ):
                    return new DataLabelsContext( *this, mrModel.mxLabels.create(bMSO2007Doc) );
                case C_TOKEN( dPt ):
                    return new DataPointContext( *this, mrModel.maPoints.create(bMSO2007Doc) );
                case C_TOKEN( errBars ):
                    return new ErrorBarContext( *this, mrModel.maErrorBars.create(bMSO2007Doc) );
                case C_TOKEN( marker ):
                    return this;
                case C_TOKEN( smooth ):
                    mrModel.mbSmooth = rAttribs.getBool( XML_val, !bMSO2007Doc );
                    return nullptr;
                case C_TOKEN( trendline ):
                    return new TrendlineContext( *this, mrModel.maTrendlines.create(bMSO2007Doc) );
                case C_TOKEN( xVal ):
                    return new DataSourceContext( *this, mrModel.maSources.create( SeriesModel::CATEGORIES ) );
                case C_TOKEN( yVal ):
                    return new DataSourceContext( *this, mrModel.maSources.create( SeriesModel::VALUES ) );
            }
        break;
    }
    return SeriesContextBase::onCreateContext( nElement, rAttribs );
}

SurfaceSeriesContext::SurfaceSeriesContext( ContextHandler2Helper& rParent, SeriesModel& rModel ) :
    SeriesContextBase( rParent, rModel )
{
}

SurfaceSeriesContext::~SurfaceSeriesContext()
{
}

ContextHandlerRef SurfaceSeriesContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case C_TOKEN( ser ):
            switch( nElement )
            {
                case C_TOKEN( cat ):
                    return new DataSourceContext( *this, mrModel.maSources.create( SeriesModel::CATEGORIES ) );
                case C_TOKEN( val ):
                    return new DataSourceContext( *this, mrModel.maSources.create( SeriesModel::VALUES ) );
            }
        break;
    }
    return SeriesContextBase::onCreateContext( nElement, rAttribs );
}

} // namespace chart
} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
