/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: chartdrawingfragment.cxx,v $
 *
 * $Revision: 1.1 $
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

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::awt::Point;
using ::com::sun::star::awt::Rectangle;
using ::com::sun::star::awt::Size;
using ::com::sun::star::drawing::XShapes;
using ::oox::core::ContextHandlerRef;
using ::oox::core::FragmentHandler2;
using ::oox::core::XmlFilterBase;

namespace oox {
namespace drawingml {
namespace chart {

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
            OSL_ENSURE( false, "ShapeAnchor::setPos - unexpected parent element" );
    }
    if( pAnchorPos ) switch( nElement )
    {
        case CDR_TOKEN( x ):    pAnchorPos->mfX = rValue.toDouble();    break;
        case CDR_TOKEN( y ):    pAnchorPos->mfY = rValue.toDouble();    break;
        default:    OSL_ENSURE( false, "ShapeAnchor::setPos - unexpected element" );
    }
}

Rectangle ShapeAnchor::calcEmuLocation( const EmuRectangle& rEmuChartRect ) const
{
    Rectangle aLoc( -1, -1, -1, -1 );

    OSL_ENSURE( maFrom.isValid(), "ShapeAnchor::calcEmuLocation - invalid from position" );
    OSL_ENSURE( mbRelSize ? maTo.isValid() : maSize.isValid(), "ShapeAnchor::calcEmuLocation - invalid to/size" );
    if( maFrom.isValid() && (mbRelSize ? maTo.isValid() : maSize.isValid()) )
    {
        // calculate shape position
        aLoc.X = getLimitedValue< sal_Int32, double >( maFrom.mfX * rEmuChartRect.Width, 0, SAL_MAX_INT32 );
        aLoc.Y = getLimitedValue< sal_Int32, double >( maFrom.mfY * rEmuChartRect.Height, 0, SAL_MAX_INT32 );

        // calculate shape size
        if( mbRelSize )
        {
            aLoc.Width = getLimitedValue< sal_Int32, double >( maTo.mfX * rEmuChartRect.Width, 0, SAL_MAX_INT32 ) - aLoc.X;
            if( aLoc.Width < 0 )
            {
                aLoc.X += aLoc.Width;
                aLoc.Width *= -1;
            }
            aLoc.Height = getLimitedValue< sal_Int32, double >( maTo.mfY * rEmuChartRect.Height, 0, SAL_MAX_INT32 ) - aLoc.Y;
            if( aLoc.Height < 0 )
            {
                aLoc.Y += aLoc.Height;
                aLoc.Height *= -1;
            }
        }
        else
        {
            aLoc.Width = getLimitedValue< sal_Int32, sal_Int64 >( maSize.Width, 0, SAL_MAX_INT32 );
            aLoc.Height = getLimitedValue< sal_Int32, sal_Int64 >( maSize.Height, 0, SAL_MAX_INT32 );
        }
    }

    return aLoc;
}
// ============================================================================

ChartDrawingFragment::ChartDrawingFragment( XmlFilterBase& rFilter,
        const OUString& rFragmentPath, const Reference< XShapes >& rxDrawPage,
        const Size& rChartSize, const Point& rShapesOffset, bool bOleSupport ) :
    FragmentHandler2( rFilter, rFragmentPath ),
    mxDrawPage( rxDrawPage ),
    mbOleSupport( bOleSupport )
{
    maEmuChartRect.X = static_cast< sal_Int64 >( rShapesOffset.X ) * 360;
    maEmuChartRect.Y = static_cast< sal_Int64 >( rShapesOffset.Y ) * 360;
    maEmuChartRect.Width = static_cast< sal_Int64 >( rChartSize.Width ) * 360;
    maEmuChartRect.Height = static_cast< sal_Int64 >( rChartSize.Height ) * 360;
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
                    mxShape.reset( new Shape( "com.sun.star.drawing.OLE2Shape" ) );
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

void ChartDrawingFragment::onEndElement( const OUString& rChars )
{
    switch( getCurrentElement() )
    {
        case CDR_TOKEN( x ):
        case CDR_TOKEN( y ):
            if( mxAnchor.get() ) mxAnchor->setPos( getCurrentElement(), getPreviousElement(), rChars );
        break;

        case CDR_TOKEN( absSizeAnchor ):
        case CDR_TOKEN( relSizeAnchor ):
            if( mxDrawPage.is() && mxShape.get() && mxAnchor.get() )
            {
                Rectangle aLoc = mxAnchor->calcEmuLocation( maEmuChartRect );
                if( (aLoc.X >= 0) && (aLoc.Y >= 0) && (aLoc.Width >= 0) && (aLoc.Height >= 0) )
                    mxShape->addShape( getFilter(), getFilter().getCurrentTheme(), mxDrawPage, &aLoc );
            }
            mxShape.reset();
            mxAnchor.reset();
        break;
    }
}

// ============================================================================

} // namespace chart
} // namespace drawingml
} // namespace oox

