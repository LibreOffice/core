/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "oox/drawingml/chart/axiscontext.hxx"
#include "oox/drawingml/shapepropertiescontext.hxx"
#include "oox/drawingml/textbodycontext.hxx"
#include "oox/drawingml/chart/axismodel.hxx"
#include "oox/drawingml/chart/titlecontext.hxx"

using ::rtl::OUString;
using ::oox::core::ContextHandlerRef;
using ::oox::core::ContextHandler2Helper;

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

AxisDispUnitsContext::AxisDispUnitsContext( ContextHandler2Helper& rParent, AxisDispUnitsModel& rModel ) :
    ContextBase< AxisDispUnitsModel >( rParent, rModel )
{
}

AxisDispUnitsContext::~AxisDispUnitsContext()
{
}

ContextHandlerRef AxisDispUnitsContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case C_TOKEN( dispUnits ):
            switch( nElement )
            {
                case C_TOKEN( builtInUnit ):
                    mrModel.mnBuiltInUnit = rAttribs.getToken( XML_val, XML_thousands );
                    return 0;
                case C_TOKEN( custUnit ):
                    mrModel.mfCustomUnit = rAttribs.getDouble( XML_val, 0.0 );
                    return 0;
                case C_TOKEN( dispUnitsLbl ):
                    return this;
            }
        break;

        case C_TOKEN( dispUnitsLbl ):
            switch( nElement )
            {
                case C_TOKEN( layout ):
                    return new LayoutContext( *this, mrModel.mxLayout.create() );
                case C_TOKEN( spPr ):
                    return new ShapePropertiesContext( *this, mrModel.mxShapeProp.create() );
                case C_TOKEN( tx ):
                    return new TextContext( *this, mrModel.mxText.create() );
                case C_TOKEN( txPr ):
                    return new TextBodyContext( *this, mrModel.mxTextProp.create() );
            }
        break;
    }
    return 0;
}

// ============================================================================

AxisContextBase::AxisContextBase( ContextHandler2Helper& rParent, AxisModel& rModel ) :
    ContextBase< AxisModel >( rParent, rModel )
{
}

AxisContextBase::~AxisContextBase()
{
}

ContextHandlerRef AxisContextBase::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case C_TOKEN( catAx ):
        case C_TOKEN( dateAx ):
        case C_TOKEN( serAx ):
        case C_TOKEN( valAx ):
            switch( nElement )
            {
                case C_TOKEN( axId ):
                    mrModel.mnAxisId = rAttribs.getInteger( XML_val, -1 );
                    return 0;
                case C_TOKEN( crossAx ):
                    mrModel.mnCrossAxisId = rAttribs.getInteger( XML_val, -1 );
                    return 0;
                case C_TOKEN( crosses ):
                    mrModel.mnCrossMode = rAttribs.getToken( XML_val, XML_autoZero );
                    return 0;
                case C_TOKEN( crossesAt ):
                    mrModel.mofCrossesAt = rAttribs.getDouble( XML_val, 0.0 );
                    return 0;
                case C_TOKEN( delete ):
                    // default is 'false', not 'true' as specified
                    mrModel.mbDeleted = rAttribs.getBool( XML_val, false );
                    return 0;
                case C_TOKEN( majorGridlines ):
                    return new ShapePrWrapperContext( *this, mrModel.mxMajorGridLines.create() );
                case C_TOKEN( majorTickMark ):
                    // default is 'out', not 'cross' as specified
                    mrModel.mnMajorTickMark = rAttribs.getToken( XML_val, XML_out );
                    return 0;
                case C_TOKEN( minorGridlines ):
                    return new ShapePrWrapperContext( *this, mrModel.mxMinorGridLines.create() );
                case C_TOKEN( minorTickMark ):
                    // default is 'none', not 'cross' as specified
                    mrModel.mnMinorTickMark = rAttribs.getToken( XML_val, XML_none );
                    return 0;
                case C_TOKEN( numFmt ):
                    mrModel.maNumberFormat.setAttributes( rAttribs );
                    return 0;
                case C_TOKEN( scaling ):
                    return this;
                case C_TOKEN( spPr ):
                    return new ShapePropertiesContext( *this, mrModel.mxShapeProp.create() );
                case C_TOKEN( tickLblPos ):
                    mrModel.mnTickLabelPos = rAttribs.getToken( XML_val, XML_nextTo );
                    return 0;
                case C_TOKEN( title ):
                    return new TitleContext( *this, mrModel.mxTitle.create() );
                case C_TOKEN( txPr ):
                    return new TextBodyContext( *this, mrModel.mxTextProp.create() );
            }
        break;

        case C_TOKEN( scaling ):
            switch( nElement )
            {
                case C_TOKEN( logBase ):
                    mrModel.mofLogBase = rAttribs.getDouble( XML_val, 0.0 );
                    return 0;
                case C_TOKEN( max ):
                    mrModel.mofMax = rAttribs.getDouble( XML_val, 0.0 );
                    return 0;
                case C_TOKEN( min ):
                    mrModel.mofMin = rAttribs.getDouble( XML_val, 0.0 );
                    return 0;
                case C_TOKEN( orientation ):
                    mrModel.mnOrientation = rAttribs.getToken( XML_val, XML_minMax );
                    return 0;
            }
        break;
    }
    return 0;
}

// ============================================================================

CatAxisContext::CatAxisContext( ContextHandler2Helper& rParent, AxisModel& rModel ) :
    AxisContextBase( rParent, rModel )
{
}

CatAxisContext::~CatAxisContext()
{
}

ContextHandlerRef CatAxisContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( isRootElement() ) switch( nElement )
    {
        case C_TOKEN( auto ):
            // default is 'false', not 'true' as specified
            mrModel.mbAuto = rAttribs.getBool( XML_val, false );
            return 0;
        case C_TOKEN( axPos ):
            mrModel.mnAxisPos = rAttribs.getToken( XML_val, XML_TOKEN_INVALID );
            return 0;
        case C_TOKEN( lblAlgn ):
            mrModel.mnLabelAlign = rAttribs.getToken( XML_val, XML_ctr );
            return 0;
        case C_TOKEN( lblOffset ):
            mrModel.mnLabelOffset = rAttribs.getInteger( XML_val, 100 );
            return 0;
        case C_TOKEN( noMultiLvlLbl ):
            // default is 'false', not 'true' as specified
            mrModel.mbNoMultiLevel = rAttribs.getBool( XML_val, false );
            return 0;
        case C_TOKEN( tickLblSkip ):
            mrModel.mnTickLabelSkip = rAttribs.getInteger( XML_val, 0 );
            return 0;
        case C_TOKEN( tickMarkSkip ):
            mrModel.mnTickMarkSkip = rAttribs.getInteger( XML_val, 0 );
            return 0;
    }
    return AxisContextBase::onCreateContext( nElement, rAttribs );
}

// ============================================================================

DateAxisContext::DateAxisContext( ContextHandler2Helper& rParent, AxisModel& rModel ) :
    AxisContextBase( rParent, rModel )
{
}

DateAxisContext::~DateAxisContext()
{
}

ContextHandlerRef DateAxisContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( isRootElement() ) switch( nElement )
    {
        case C_TOKEN( auto ):
            // default is 'false', not 'true' as specified
            mrModel.mbAuto = rAttribs.getBool( XML_val, false );
            return 0;
        case C_TOKEN( baseTimeUnit ):
            mrModel.mnBaseTimeUnit = rAttribs.getToken( XML_val, XML_days );
            return 0;
        case C_TOKEN( lblOffset ):
            mrModel.mnLabelOffset = rAttribs.getInteger( XML_val, 100 );
            return 0;
        case C_TOKEN( majorTimeUnit ):
            mrModel.mnMajorTimeUnit = rAttribs.getToken( XML_val, XML_days );
            return 0;
        case C_TOKEN( majorUnit ):
            mrModel.mofMajorUnit = rAttribs.getDouble( XML_val, 0.0 );
            return 0;
        case C_TOKEN( minorTimeUnit ):
            mrModel.mnMinorTimeUnit = rAttribs.getToken( XML_val, XML_days );
            return 0;
        case C_TOKEN( minorUnit ):
            mrModel.mofMinorUnit = rAttribs.getDouble( XML_val, 0.0 );
            return 0;
    }
    return AxisContextBase::onCreateContext( nElement, rAttribs );
}

// ============================================================================

SerAxisContext::SerAxisContext( ContextHandler2Helper& rParent, AxisModel& rModel ) :
    AxisContextBase( rParent, rModel )
{
}

SerAxisContext::~SerAxisContext()
{
}

ContextHandlerRef SerAxisContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( isRootElement() ) switch( nElement )
    {
        case C_TOKEN( tickLblSkip ):
            mrModel.mnTickLabelSkip = rAttribs.getInteger( XML_val, 0 );
            return 0;
        case C_TOKEN( tickMarkSkip ):
            mrModel.mnTickMarkSkip = rAttribs.getInteger( XML_val, 0 );
            return 0;
    }
    return AxisContextBase::onCreateContext( nElement, rAttribs );
}

// ============================================================================

ValAxisContext::ValAxisContext( ContextHandler2Helper& rParent, AxisModel& rModel ) :
    AxisContextBase( rParent, rModel )
{
}

ValAxisContext::~ValAxisContext()
{
}

ContextHandlerRef ValAxisContext::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( isRootElement() ) switch( nElement )
    {
        case C_TOKEN( crossBetween ):
            mrModel.mnCrossBetween = rAttribs.getToken( XML_val, XML_between );
            return 0;
        case C_TOKEN( dispUnits ):
            return new AxisDispUnitsContext( *this, mrModel.mxDispUnits.create() );
        case C_TOKEN( majorUnit ):
            mrModel.mofMajorUnit = rAttribs.getDouble( XML_val, 0.0 );
            return 0;
        case C_TOKEN( minorUnit ):
            mrModel.mofMinorUnit = rAttribs.getDouble( XML_val, 0.0 );
            return 0;
    }
    return AxisContextBase::onCreateContext( nElement, rAttribs );
}

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

