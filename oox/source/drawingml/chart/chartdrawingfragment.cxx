/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "oox/drawingml/chart/chartdrawingfragment.hxx"

#include "oox/core/xmlfilterbase.hxx"
#include "oox/drawingml/connectorshapecontext.hxx"
#include "oox/drawingml/graphicshapecontext.hxx"
#include "oox/drawingml/shapecontext.hxx"
#include "oox/drawingml/shapegroupcontext.hxx"

namespace oox {
namespace drawingml {
namespace chart {

// ============================================================================

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::uno;
using namespace ::oox::core;

using ::rtl::OUString;

// ============================================================================

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
    AnchorPosModel* pAnchorPos = 0;
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
// ============================================================================

ChartDrawingFragment::ChartDrawingFragment( XmlFilterBase& rFilter,
        const OUString& rFragmentPath, const Reference< XShapes >& rxDrawPage,
        const Size& rChartSize, const Point& rShapesOffset, bool bOleSupport ) :
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
                        return 0;
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
                    return 0;
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
    return 0;
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
                Rectangle aShapeRectEmu32(
                    getLimitedValue< sal_Int32, sal_Int64 >( aShapeRectEmu.X, 0, SAL_MAX_INT32 ),
                    getLimitedValue< sal_Int32, sal_Int64 >( aShapeRectEmu.Y, 0, SAL_MAX_INT32 ),
                    getLimitedValue< sal_Int32, sal_Int64 >( aShapeRectEmu.Width, 0, SAL_MAX_INT32 ),
                    getLimitedValue< sal_Int32, sal_Int64 >( aShapeRectEmu.Height, 0, SAL_MAX_INT32 ) );
                basegfx::B2DHomMatrix aMatrix;
                mxShape->addShape( getFilter(), getFilter().getCurrentTheme(), mxDrawPage, aMatrix, &aShapeRectEmu32 );
            }
        }
        mxShape.reset();
        mxAnchor.reset();
    }
}

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
