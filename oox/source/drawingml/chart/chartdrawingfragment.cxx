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

#include "drawingml/chart/chartdrawingfragment.hxx"

#include <osl/diagnose.h>

#include "oox/core/xmlfilterbase.hxx"
#include "oox/drawingml/connectorshapecontext.hxx"
#include "oox/drawingml/graphicshapecontext.hxx"
#include "oox/drawingml/shapecontext.hxx"
#include "oox/drawingml/shapegroupcontext.hxx"

namespace oox {
namespace drawingml {
namespace chart {

using namespace ::com::sun::star;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::uno;
using namespace ::oox::core;

ShapeAnchor::ShapeAnchor( bool bRelSize ) :
    mbRelSize( bRelSize )
{
}

void ShapeAnchor::importExt( const AttributeList& rAttribs )
{
    OSL_ENSURE( !mbRelSize, "ShapeAnchor::importExt - unexpected 'cdr:ext' element" );
    maSize.Width = rAttribs.getHyper( XML_cx, 0 );
    maSize.Height = rAttribs.getHyper( XML_cy, 0 );
}

void ShapeAnchor::setPos( sal_Int32 nElement, sal_Int32 nParentContext, const OUString& rValue )
{
    AnchorPosModel* pAnchorPos = nullptr;
    switch( nParentContext )
    {
        case CDR_TOKEN( from ):
            pAnchorPos = &maFrom;
        break;
        case CDR_TOKEN( to ):
            OSL_ENSURE( mbRelSize, "ShapeAnchor::setPos - unexpected 'cdr:to' element" );
            pAnchorPos = &maTo;
        break;
        default:
            OSL_FAIL( "ShapeAnchor::setPos - unexpected parent element" );
    }
    if( pAnchorPos ) switch( nElement )
    {
        case CDR_TOKEN( x ):    pAnchorPos->mfX = rValue.toDouble();    break;
        case CDR_TOKEN( y ):    pAnchorPos->mfY = rValue.toDouble();    break;
        default:    OSL_FAIL( "ShapeAnchor::setPos - unexpected element" );
    }
}

EmuRectangle ShapeAnchor::calcAnchorRectEmu( const EmuRectangle& rChartRect ) const
{
    EmuRectangle aAnchorRect( -1, -1, -1, -1 );

    OSL_ENSURE( maFrom.isValid(), "ShapeAnchor::calcAnchorRectEmu - invalid from position" );
    OSL_ENSURE( mbRelSize ? maTo.isValid() : maSize.isValid(), "ShapeAnchor::calcAnchorRectEmu - invalid to/size" );
    if( maFrom.isValid() && (mbRelSize ? maTo.isValid() : maSize.isValid()) )
    {
        // calculate shape position
        aAnchorRect.X = static_cast< sal_Int64 >( maFrom.mfX * rChartRect.Width + 0.5 );
        aAnchorRect.Y = static_cast< sal_Int64 >( maFrom.mfY * rChartRect.Height + 0.5 );

        // calculate shape size
        if( mbRelSize )
        {
            aAnchorRect.Width = static_cast< sal_Int64 >( maTo.mfX * rChartRect.Width + 0.5 ) - aAnchorRect.X;
            if( aAnchorRect.Width < 0 )
            {
                aAnchorRect.X += aAnchorRect.Width;
                aAnchorRect.Width *= -1;
            }
            aAnchorRect.Height = static_cast< sal_Int64 >( maTo.mfY * rChartRect.Height + 0.5 ) - aAnchorRect.Y;
            if( aAnchorRect.Height < 0 )
            {
                aAnchorRect.Y += aAnchorRect.Height;
                aAnchorRect.Height *= -1;
            }
        }
        else
        {
            aAnchorRect.setSize( maSize );
        }
    }

    return aAnchorRect;
}

ChartDrawingFragment::ChartDrawingFragment( XmlFilterBase& rFilter,
        const OUString& rFragmentPath, const Reference< XShapes >& rxDrawPage,
        const awt::Size& rChartSize, const awt::Point& rShapesOffset, bool bOleSupport ) :
    FragmentHandler2( rFilter, rFragmentPath ),
    mxDrawPage( rxDrawPage ),
    mbOleSupport( bOleSupport )
{
    maChartRectEmu.X = convertHmmToEmu( rShapesOffset.X );
    maChartRectEmu.Y = convertHmmToEmu( rShapesOffset.Y );
    maChartRectEmu.Width = convertHmmToEmu( rChartSize.Width );
    maChartRectEmu.Height = convertHmmToEmu( rChartSize.Height );
}

ChartDrawingFragment::~ChartDrawingFragment()
{
}

ContextHandlerRef ChartDrawingFragment::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nElement == C_TOKEN( userShapes ) ) return this;
        break;

        case C_TOKEN( userShapes ):
            switch( nElement )
            {
                case CDR_TOKEN( absSizeAnchor ):
                    mxAnchor.reset( new ShapeAnchor( false ) );
                    return this;
                case CDR_TOKEN( relSizeAnchor ):
                    mxAnchor.reset( new ShapeAnchor( true ) );
                    return this;
            }
        break;

        case CDR_TOKEN( absSizeAnchor ):
        case CDR_TOKEN( relSizeAnchor ):
            switch( nElement )
            {
                case CDR_TOKEN( sp ):
                    mxShape.reset( new Shape( "com.sun.star.drawing.CustomShape" ) );
                    return new ShapeContext( *this, ShapePtr(), mxShape );
                case CDR_TOKEN( cxnSp ):
                    mxShape.reset( new Shape( "com.sun.star.drawing.ConnectorShape" ) );
                    return new ConnectorShapeContext( *this, ShapePtr(), mxShape );
                case CDR_TOKEN( pic ):
                    mxShape.reset( new Shape( "com.sun.star.drawing.GraphicObjectShape" ) );
                    return new GraphicShapeContext( *this, ShapePtr(), mxShape );
                case CDR_TOKEN( graphicFrame ):
                    if( !mbOleSupport )
                        return nullptr;
                    mxShape.reset( new Shape( "com.sun.star.drawing.GraphicObjectShape" ) );
                    return new GraphicalObjectFrameContext( *this, ShapePtr(), mxShape, true );
                case CDR_TOKEN( grpSp ):
                    mxShape.reset( new Shape( "com.sun.star.drawing.GroupShape" ) );
                    return new ShapeGroupContext( *this, ShapePtr(), mxShape );

                case CDR_TOKEN( from ):
                case CDR_TOKEN( to ):
                    return this;

                case CDR_TOKEN( ext ):
                    if( mxAnchor.get() ) mxAnchor->importExt( rAttribs );
                    return nullptr;
            }
        break;

        case CDR_TOKEN( from ):
        case CDR_TOKEN( to ):
            switch( nElement )
            {
                case CDR_TOKEN( x ):
                case CDR_TOKEN( y ):
                    return this;        // collect value in onEndElement()
            }
        break;
    }
    return nullptr;
}

void ChartDrawingFragment::onCharacters( const OUString& rChars )
{
    if( isCurrentElement( CDR_TOKEN( x ), CDR_TOKEN( y ) ) && mxAnchor.get() )
        mxAnchor->setPos( getCurrentElement(), getParentElement(), rChars );
}

void ChartDrawingFragment::onEndElement()
{
    if( isCurrentElement( CDR_TOKEN( absSizeAnchor ), CDR_TOKEN( relSizeAnchor ) ) )
    {
        if( mxDrawPage.is() && mxShape.get() && mxAnchor.get() )
        {
            EmuRectangle aShapeRectEmu = mxAnchor->calcAnchorRectEmu( maChartRectEmu );
            if( (aShapeRectEmu.X >= 0) && (aShapeRectEmu.Y >= 0) && (aShapeRectEmu.Width >= 0) && (aShapeRectEmu.Height >= 0) )
            {
                // TODO: DrawingML implementation expects 32-bit coordinates for EMU rectangles (change that to EmuRectangle)
                awt::Rectangle aShapeRectEmu32(
                    getLimitedValue< sal_Int32, sal_Int64 >( aShapeRectEmu.X, 0, SAL_MAX_INT32 ),
                    getLimitedValue< sal_Int32, sal_Int64 >( aShapeRectEmu.Y, 0, SAL_MAX_INT32 ),
                    getLimitedValue< sal_Int32, sal_Int64 >( aShapeRectEmu.Width, 0, SAL_MAX_INT32 ),
                    getLimitedValue< sal_Int32, sal_Int64 >( aShapeRectEmu.Height, 0, SAL_MAX_INT32 ) );
                basegfx::B2DHomMatrix aMatrix;
                mxShape->addShape( getFilter(), getFilter().getCurrentTheme(), mxDrawPage, aMatrix, mxShape->getFillProperties(), &aShapeRectEmu32 );
            }
        }
        mxShape.reset();
        mxAnchor.reset();
    }
}

} // namespace chart
} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
