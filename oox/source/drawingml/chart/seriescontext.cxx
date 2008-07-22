/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: seriescontext.cxx,v $
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

#include "oox/drawingml/chart/seriescontext.hxx"
#include "oox/drawingml/shapepropertiescontext.hxx"
#include "oox/drawingml/textbodycontext.hxx"
#include "oox/drawingml/chart/datasourcecontext.hxx"
#include "oox/drawingml/chart/seriesmodel.hxx"
#include "oox/drawingml/chart/titlecontext.hxx"

using ::rtl::OUString;
using ::oox::core::ContextHandler2;
using ::oox::core::ContextHandler2Helper;
using ::oox::core::ContextWrapper;

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

namespace {

ContextWrapper lclDataLabelSharedCreateContext(
        ContextHandler2& rContext, sal_Int32 nElement, const AttributeList& rAttribs, DataLabelModelBase& orModel )
{
    if( rContext.isRootElement() ) switch( nElement )
    {
        case C_TOKEN( delete ):
            // default is 'false', not 'true' as specified
            orModel.mbDeleted = rAttribs.getBool( XML_val, false );
            return false;
        case C_TOKEN( dLblPos ):
            orModel.monLabelPos = rAttribs.getToken( XML_val, XML_TOKEN_INVALID );
            return false;
        case C_TOKEN( numFmt ):
            orModel.maNumberFormat.setAttributes( rAttribs );
            return false;
        case C_TOKEN( showBubbleSize ):
            orModel.mobShowBubbleSize = rAttribs.getBool( XML_val );
            return false;
        case C_TOKEN( showCatName ):
            orModel.mobShowCatName = rAttribs.getBool( XML_val );
            return false;
        case C_TOKEN( showLegendKey ):
            orModel.mobShowLegendKey = rAttribs.getBool( XML_val );
            return false;
        case C_TOKEN( showPercent ):
            orModel.mobShowPercent = rAttribs.getBool( XML_val );
            return false;
        case C_TOKEN( showSerName ):
            orModel.mobShowSerName = rAttribs.getBool( XML_val );
            return false;
        case C_TOKEN( showVal ):
            orModel.mobShowVal = rAttribs.getBool( XML_val );
            return false;
        case C_TOKEN( separator ):
            // collect separator text in onEndElement()
            return true;
        case C_TOKEN( spPr ):
            return new ShapePropertiesContext( rContext, orModel.mxShapeProp.create() );
        case C_TOKEN( txPr ):
            return new TextBodyContext( rContext, orModel.mxTextProp.create() );
    }
    return false;
}

void lclDataLabelSharedEndElement( ContextHandler2& rContext, const OUString& rChars, DataLabelModelBase& orModel )
{
    switch( rContext.getCurrentElement() )
    {
        case C_TOKEN( separator ):
            orModel.moaSeparator = rChars;
        break;
    }
}

} // namespace

// ============================================================================

DataLabelContext::DataLabelContext( ContextHandler2Helper& rParent, DataLabelModel& rModel ) :
    ContextBase< DataLabelModel >( rParent, rModel )
{
}

DataLabelContext::~DataLabelContext()
{
}

ContextWrapper DataLabelContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( isRootElement() ) switch( nElement )
    {
        case C_TOKEN( idx ):
            mrModel.mnIndex = rAttribs.getInteger( XML_val, -1 );
            return false;
        case C_TOKEN( layout ):
            return new LayoutContext( *this, mrModel.mxLayout.create() );
        case C_TOKEN( tx ):
            return new TextContext( *this, mrModel.mxText.create() );
    }
    return lclDataLabelSharedCreateContext( *this, nElement, rAttribs, mrModel );
}

void DataLabelContext::onEndElement( const OUString& rChars )
{
    lclDataLabelSharedEndElement( *this, rChars, mrModel );
}

// ============================================================================

DataLabelsContext::DataLabelsContext( ContextHandler2Helper& rParent, DataLabelsModel& rModel ) :
    ContextBase< DataLabelsModel >( rParent, rModel )
{
}

DataLabelsContext::~DataLabelsContext()
{
}

ContextWrapper DataLabelsContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( isRootElement() ) switch( nElement )
    {
        case C_TOKEN( dLbl ):
            return new DataLabelContext( *this, mrModel.maPointLabels.create() );
        case C_TOKEN( leaderLines ):
            return new ShapePrWrapperContext( *this, mrModel.mxLeaderLines.create() );
        case C_TOKEN( showLeaderLines ):
            // default is 'false', not 'true' as specified
            mrModel.mbShowLeaderLines = rAttribs.getBool( XML_val, false );
            return false;
    }
    return lclDataLabelSharedCreateContext( *this, nElement, rAttribs, mrModel );
}

void DataLabelsContext::onEndElement( const OUString& rChars )
{
    lclDataLabelSharedEndElement( *this, rChars, mrModel );
}

// ============================================================================

ErrorBarContext::ErrorBarContext( ContextHandler2Helper& rParent, ErrorBarModel& rModel ) :
    ContextBase< ErrorBarModel >( rParent, rModel )
{
}

ErrorBarContext::~ErrorBarContext()
{
}

ContextWrapper ErrorBarContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( isRootElement() ) switch( nElement )
    {
        case C_TOKEN( errBarType ):
            mrModel.mnTypeId = rAttribs.getToken( XML_val, XML_both );
            return false;
        case C_TOKEN( errDir ):
            mrModel.mnDirection = rAttribs.getToken( XML_val, XML_TOKEN_INVALID );
            return false;
        case C_TOKEN( errValType ):
            mrModel.mnValueType = rAttribs.getToken( XML_val, XML_fixedVal );
            return false;
        case C_TOKEN( minus ):
            return new DataSourceContext( *this, mrModel.maSources.create( ErrorBarModel::MINUS ) );
        case C_TOKEN( noEndCap ):
            // default is 'false', not 'true' as specified
            mrModel.mbNoEndCap = rAttribs.getBool( XML_val, false );
            return false;
        case C_TOKEN( plus ):
            return new DataSourceContext( *this, mrModel.maSources.create( ErrorBarModel::PLUS ) );
        case C_TOKEN( spPr ):
            return new ShapePropertiesContext( *this, mrModel.mxShapeProp.create() );
        case C_TOKEN( val ):
            mrModel.mfValue = rAttribs.getDouble( XML_val, 0.0 );
            return false;
    }
    return false;
}

// ============================================================================

TrendlineLabelContext::TrendlineLabelContext( ContextHandler2Helper& rParent, TrendlineLabelModel& rModel ) :
    ContextBase< TrendlineLabelModel >( rParent, rModel )
{
}

TrendlineLabelContext::~TrendlineLabelContext()
{
}

ContextWrapper TrendlineLabelContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( isRootElement() ) switch( nElement )
    {
        case C_TOKEN( layout ):
            return new LayoutContext( *this, mrModel.mxLayout.create() );
        case C_TOKEN( numFmt ):
            mrModel.maNumberFormat.setAttributes( rAttribs );
            return false;
        case C_TOKEN( spPr ):
            return new ShapePropertiesContext( *this, mrModel.mxShapeProp.create() );
        case C_TOKEN( tx ):
            return new TextContext( *this, mrModel.mxText.create() );
        case C_TOKEN( txPr ):
            return new TextBodyContext( *this, mrModel.mxTextProp.create() );
    }
    return false;
}

// ============================================================================

TrendlineContext::TrendlineContext( ContextHandler2Helper& rParent, TrendlineModel& rModel ) :
    ContextBase< TrendlineModel >( rParent, rModel )
{
}

TrendlineContext::~TrendlineContext()
{
}

ContextWrapper TrendlineContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( isRootElement() ) switch( nElement )
    {
        case C_TOKEN( backward ):
            mrModel.mfBackward = rAttribs.getDouble( XML_val, 0.0 );
            return false;
        case C_TOKEN( dispEq ):
            // default is 'false', not 'true' as specified
            mrModel.mbDispEquation = rAttribs.getBool( XML_val, false );
            return false;
        case C_TOKEN( dispRSqr ):
            // default is 'false', not 'true' as specified
            mrModel.mbDispRSquared = rAttribs.getBool( XML_val, false );
            return false;
        case C_TOKEN( forward ):
            mrModel.mfForward = rAttribs.getDouble( XML_val, 0.0 );
            return false;
        case C_TOKEN( intercept ):
            mrModel.mfIntercept = rAttribs.getDouble( XML_val, 0.0 );
            return false;
        case C_TOKEN( name ):
            return true;
        case C_TOKEN( order ):
            mrModel.mnOrder = rAttribs.getInteger( XML_val, 2 );
            return false;
        case C_TOKEN( period ):
            mrModel.mnPeriod = rAttribs.getInteger( XML_val, 2 );
            return false;
        case C_TOKEN( spPr ):
            return new ShapePropertiesContext( *this, mrModel.mxShapeProp.create() );
        case C_TOKEN( trendlineLbl ):
            return new TrendlineLabelContext( *this, mrModel.mxLabel.create() );
        case C_TOKEN( trendlineType ):
            mrModel.mnTypeId = rAttribs.getToken( XML_val, XML_linear );
            return false;
    }
    return false;
}

void TrendlineContext::onEndElement( const ::rtl::OUString& rChars )
{
    switch( getCurrentElement() )
    {
        case C_TOKEN( name ):
            mrModel.maName = rChars;
        break;
    }
}

// ============================================================================

DataPointContext::DataPointContext( ContextHandler2Helper& rParent, DataPointModel& rModel ) :
    ContextBase< DataPointModel >( rParent, rModel )
{
}

DataPointContext::~DataPointContext()
{
}

ContextWrapper DataPointContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case C_TOKEN( dPt ):
            switch( nElement )
            {
                case C_TOKEN( bubble3D ):
                    mrModel.mobBubble3d = rAttribs.getBool( XML_val );
                    return false;
                case C_TOKEN( explosion ):
                    // if the 'val' attribute is missing, series explosion remains unchanged
                    mrModel.monExplosion = rAttribs.getInteger( XML_val );
                    return false;
                case C_TOKEN( idx ):
                    mrModel.mnIndex = rAttribs.getInteger( XML_val, -1 );
                    return false;
                case C_TOKEN( invertIfNegative ):
                    // default is 'false', not 'true' as specified (value not derived from series!)
                    mrModel.mbInvertNeg = rAttribs.getBool( XML_val, false );
                    return false;
                case C_TOKEN( marker ):
                    return true;
                case C_TOKEN( spPr ):
                    return new ShapePropertiesContext( *this, mrModel.mxShapeProp.create() );
            }
        break;

        case C_TOKEN( marker ):
            switch( nElement )
            {
                case C_TOKEN( size ):
                    mrModel.monMarkerSize = rAttribs.getInteger( XML_val, 5 );
                    return false;
                case C_TOKEN( spPr ):
                    return new ShapePropertiesContext( *this, mrModel.mxMarkerProp.create() );
                case C_TOKEN( symbol ):
                    mrModel.monMarkerSymbol = rAttribs.getToken( XML_val, XML_none );
                    return false;
            }
        break;
    }
    return false;
}

// ============================================================================

SeriesContextBase::SeriesContextBase( ContextHandler2Helper& rParent, SeriesModel& rModel ) :
    ContextBase< SeriesModel >( rParent, rModel )
{
}

SeriesContextBase::~SeriesContextBase()
{
}

ContextWrapper SeriesContextBase::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case C_TOKEN( ser ):
            switch( nElement )
            {
                case C_TOKEN( idx ):
                    mrModel.mnIndex = rAttribs.getInteger( XML_val, -1 );
                    return false;
                case C_TOKEN( order ):
                    mrModel.mnOrder = rAttribs.getInteger( XML_val, -1 );
                    return false;
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
                    return false;
                case C_TOKEN( spPr ):
                    return new ShapePropertiesContext( *this, mrModel.mxMarkerProp.create() );
                case C_TOKEN( symbol ):
                    mrModel.mnMarkerSymbol = rAttribs.getToken( XML_val, XML_none );
                    return false;
            }
        break;
    }
    return false;
}

// ============================================================================

AreaSeriesContext::AreaSeriesContext( ContextHandler2Helper& rParent, SeriesModel& rModel ) :
    SeriesContextBase( rParent, rModel )
{
}

AreaSeriesContext::~AreaSeriesContext()
{
}

ContextWrapper AreaSeriesContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case C_TOKEN( ser ):
            switch( nElement )
            {
                case C_TOKEN( cat ):
                    return new DataSourceContext( *this, mrModel.maSources.create( SeriesModel::CATEGORIES ) );
                case C_TOKEN( errBars ):
                    return new ErrorBarContext( *this, mrModel.maErrorBars.create() );
                case C_TOKEN( dLbls ):
                    return new DataLabelsContext( *this, mrModel.mxLabels.create() );
                case C_TOKEN( dPt ):
                    return new DataPointContext( *this, mrModel.maPoints.create() );
                case C_TOKEN( trendline ):
                    return new TrendlineContext( *this, mrModel.maTrendlines.create() );
                case C_TOKEN( val ):
                    return new DataSourceContext( *this, mrModel.maSources.create( SeriesModel::VALUES ) );
            }
        break;
    }
    return SeriesContextBase::onCreateContext( nElement, rAttribs );
}

// ============================================================================

BarSeriesContext::BarSeriesContext( ContextHandler2Helper& rParent, SeriesModel& rModel ) :
    SeriesContextBase( rParent, rModel )
{
}

BarSeriesContext::~BarSeriesContext()
{
}

ContextWrapper BarSeriesContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case C_TOKEN( ser ):
            switch( nElement )
            {
                case C_TOKEN( cat ):
                    return new DataSourceContext( *this, mrModel.maSources.create( SeriesModel::CATEGORIES ) );
                case C_TOKEN( dLbls ):
                    return new DataLabelsContext( *this, mrModel.mxLabels.create() );
                case C_TOKEN( dPt ):
                    return new DataPointContext( *this, mrModel.maPoints.create() );
                case C_TOKEN( errBars ):
                    return new ErrorBarContext( *this, mrModel.maErrorBars.create() );
                case C_TOKEN( invertIfNegative ):
                    // default is 'false', not 'true' as specified
                    mrModel.mbInvertNeg = rAttribs.getBool( XML_val, false );
                    return false;
                case C_TOKEN( shape ):
                    // missing attribute does not change shape type to 'box' as specified
                    mrModel.monShape = rAttribs.getToken( XML_val );
                    return false;
                case C_TOKEN( trendline ):
                    return new TrendlineContext( *this, mrModel.maTrendlines.create() );
                case C_TOKEN( val ):
                    return new DataSourceContext( *this, mrModel.maSources.create( SeriesModel::VALUES ) );
            }
        break;
    }
    return SeriesContextBase::onCreateContext( nElement, rAttribs );
}

// ============================================================================

BubbleSeriesContext::BubbleSeriesContext( ContextHandler2Helper& rParent, SeriesModel& rModel ) :
    SeriesContextBase( rParent, rModel )
{
}

BubbleSeriesContext::~BubbleSeriesContext()
{
}

ContextWrapper BubbleSeriesContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case C_TOKEN( ser ):
            switch( nElement )
            {
                case C_TOKEN( bubble3D ):
                    // default is 'false', not 'true' as specified
                    mrModel.mbBubble3d = rAttribs.getBool( XML_val, false );
                    return false;
                case C_TOKEN( bubbleSize ):
                    return new DataSourceContext( *this, mrModel.maSources.create( SeriesModel::POINTS ) );
                case C_TOKEN( dLbls ):
                    return new DataLabelsContext( *this, mrModel.mxLabels.create() );
                case C_TOKEN( dPt ):
                    return new DataPointContext( *this, mrModel.maPoints.create() );
                case C_TOKEN( errBars ):
                    return new ErrorBarContext( *this, mrModel.maErrorBars.create() );
                case C_TOKEN( invertIfNegative ):
                    // default is 'false', not 'true' as specified
                    mrModel.mbInvertNeg = rAttribs.getBool( XML_val, false );
                    return false;
                case C_TOKEN( trendline ):
                    return new TrendlineContext( *this, mrModel.maTrendlines.create() );
                case C_TOKEN( xVal ):
                    return new DataSourceContext( *this, mrModel.maSources.create( SeriesModel::CATEGORIES ) );
                case C_TOKEN( yVal ):
                    return new DataSourceContext( *this, mrModel.maSources.create( SeriesModel::VALUES ) );
            }
        break;
    }
    return SeriesContextBase::onCreateContext( nElement, rAttribs );
}

// ============================================================================

LineSeriesContext::LineSeriesContext( ContextHandler2Helper& rParent, SeriesModel& rModel ) :
    SeriesContextBase( rParent, rModel )
{
}

LineSeriesContext::~LineSeriesContext()
{
}

ContextWrapper LineSeriesContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case C_TOKEN( ser ):
            switch( nElement )
            {
                case C_TOKEN( cat ):
                    return new DataSourceContext( *this, mrModel.maSources.create( SeriesModel::CATEGORIES ) );
                case C_TOKEN( dLbls ):
                    return new DataLabelsContext( *this, mrModel.mxLabels.create() );
                case C_TOKEN( dPt ):
                    return new DataPointContext( *this, mrModel.maPoints.create() );
                case C_TOKEN( errBars ):
                    return new ErrorBarContext( *this, mrModel.maErrorBars.create() );
                case C_TOKEN( marker ):
                    return true;
                case C_TOKEN( smooth ):
                    // default is 'false', not 'true' as specified
                    mrModel.mbSmooth = rAttribs.getBool( XML_val, false );
                    return false;
                case C_TOKEN( trendline ):
                    return new TrendlineContext( *this, mrModel.maTrendlines.create() );
                case C_TOKEN( val ):
                    return new DataSourceContext( *this, mrModel.maSources.create( SeriesModel::VALUES ) );
            }
        break;
    }
    return SeriesContextBase::onCreateContext( nElement, rAttribs );
}

// ============================================================================

PieSeriesContext::PieSeriesContext( ContextHandler2Helper& rParent, SeriesModel& rModel ) :
    SeriesContextBase( rParent, rModel )
{
}

PieSeriesContext::~PieSeriesContext()
{
}

ContextWrapper PieSeriesContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case C_TOKEN( ser ):
            switch( nElement )
            {
                case C_TOKEN( cat ):
                    return new DataSourceContext( *this, mrModel.maSources.create( SeriesModel::CATEGORIES ) );
                case C_TOKEN( dLbls ):
                    return new DataLabelsContext( *this, mrModel.mxLabels.create() );
                case C_TOKEN( dPt ):
                    return new DataPointContext( *this, mrModel.maPoints.create() );
                case C_TOKEN( explosion ):
                    mrModel.mnExplosion = rAttribs.getInteger( XML_val, 0 );
                    return false;
                case C_TOKEN( val ):
                    return new DataSourceContext( *this, mrModel.maSources.create( SeriesModel::VALUES ) );
            }
        break;
    }
    return SeriesContextBase::onCreateContext( nElement, rAttribs );
}

// ============================================================================

RadarSeriesContext::RadarSeriesContext( ContextHandler2Helper& rParent, SeriesModel& rModel ) :
    SeriesContextBase( rParent, rModel )
{
}

RadarSeriesContext::~RadarSeriesContext()
{
}

ContextWrapper RadarSeriesContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case C_TOKEN( ser ):
            switch( nElement )
            {
                case C_TOKEN( cat ):
                    return new DataSourceContext( *this, mrModel.maSources.create( SeriesModel::CATEGORIES ) );
                case C_TOKEN( dLbls ):
                    return new DataLabelsContext( *this, mrModel.mxLabels.create() );
                case C_TOKEN( dPt ):
                    return new DataPointContext( *this, mrModel.maPoints.create() );
                case C_TOKEN( marker ):
                    return true;
                case C_TOKEN( smooth ):
                    // default is 'false', not 'true' as specified
                    mrModel.mbSmooth = rAttribs.getBool( XML_val, false );
                    return false;
                case C_TOKEN( val ):
                    return new DataSourceContext( *this, mrModel.maSources.create( SeriesModel::VALUES ) );
            }
        break;
    }
    return SeriesContextBase::onCreateContext( nElement, rAttribs );
}

// ============================================================================

ScatterSeriesContext::ScatterSeriesContext( ContextHandler2Helper& rParent, SeriesModel& rModel ) :
    SeriesContextBase( rParent, rModel )
{
}

ScatterSeriesContext::~ScatterSeriesContext()
{
}

ContextWrapper ScatterSeriesContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case C_TOKEN( ser ):
            switch( nElement )
            {
                case C_TOKEN( dLbls ):
                    return new DataLabelsContext( *this, mrModel.mxLabels.create() );
                case C_TOKEN( dPt ):
                    return new DataPointContext( *this, mrModel.maPoints.create() );
                case C_TOKEN( errBars ):
                    return new ErrorBarContext( *this, mrModel.maErrorBars.create() );
                case C_TOKEN( marker ):
                    return true;
                case C_TOKEN( smooth ):
                    // default is 'false', not 'true' as specified
                    mrModel.mbSmooth = rAttribs.getBool( XML_val, false );
                    return false;
                case C_TOKEN( trendline ):
                    return new TrendlineContext( *this, mrModel.maTrendlines.create() );
                case C_TOKEN( xVal ):
                    return new DataSourceContext( *this, mrModel.maSources.create( SeriesModel::CATEGORIES ) );
                case C_TOKEN( yVal ):
                    return new DataSourceContext( *this, mrModel.maSources.create( SeriesModel::VALUES ) );
            }
        break;
    }
    return SeriesContextBase::onCreateContext( nElement, rAttribs );
}

// ============================================================================

SurfaceSeriesContext::SurfaceSeriesContext( ContextHandler2Helper& rParent, SeriesModel& rModel ) :
    SeriesContextBase( rParent, rModel )
{
}

SurfaceSeriesContext::~SurfaceSeriesContext()
{
}

ContextWrapper SurfaceSeriesContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
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

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

