/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drawingfragment.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:58:39 $
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

#include "oox/xls/drawingfragment.hxx"
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include "oox/helper/attributelist.hxx"
#include "oox/drawingml/connectorshapecontext.hxx"
#include "oox/drawingml/graphicshapecontext.hxx"
#include "oox/drawingml/shapecontext.hxx"
#include "oox/drawingml/shapegroupcontext.hxx"
#include "oox/xls/themebuffer.hxx"
#include "oox/xls/unitconverter.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::awt::Point;
using ::com::sun::star::awt::Size;
using ::com::sun::star::awt::Rectangle;
using ::com::sun::star::drawing::XDrawPageSupplier;
using ::com::sun::star::table::CellAddress;
using ::oox::drawingml::ConnectorShapeContext;
using ::oox::drawingml::GraphicalObjectFrameContext;
using ::oox::drawingml::GraphicShapeContext;
using ::oox::drawingml::Shape;
using ::oox::drawingml::ShapePtr;
using ::oox::drawingml::ShapeContext;
using ::oox::drawingml::ShapeGroupContext;

namespace oox {
namespace xls {

// ============================================================================

OoxAnchorPosition::OoxAnchorPosition() :
    mnX( -1 ),
    mnY( -1 )
{
}

// ----------------------------------------------------------------------------

OoxAnchorSize::OoxAnchorSize() :
    mnWidth( -1 ),
    mnHeight( -1 )
{
}

// ----------------------------------------------------------------------------

OoxAnchorCell::OoxAnchorCell() :
    mnCol( -1 ),
    mnRow( -1 ),
    mnColOffset( 0 ),
    mnRowOffset( 0 )
{
}

// ----------------------------------------------------------------------------

OoxAnchorClientData::OoxAnchorClientData() :
    mbLocksWithSheet( true ),
    mbPrintsWithSheet( true )
{
}

// ============================================================================

ShapeAnchor::ShapeAnchor( const WorksheetHelper& rHelper ) :
    WorksheetHelper( rHelper ),
    meType( ANCHOR_INVALID ),
    mnEditAs( XML_twoCell )
{
}

void ShapeAnchor::importAbsoluteAnchor( const AttributeList& )
{
    meType = ANCHOR_ABSOLUTE;
}

void ShapeAnchor::importOneCellAnchor( const AttributeList& )
{
    meType = ANCHOR_ONECELL;
}

void ShapeAnchor::importTwoCellAnchor( const AttributeList& rAttribs )
{
    meType = ANCHOR_TWOCELL;
    mnEditAs = rAttribs.getToken( XML_editAs, XML_twoCell );
}

void ShapeAnchor::importPos( const AttributeList& rAttribs )
{
    OSL_ENSURE( meType == ANCHOR_ABSOLUTE, "ShapeAnchor::importPos - unexpected 'xdr:pos' element" );
    maPos.mnX = rAttribs.getInteger64( XML_x, 0 );
    maPos.mnY = rAttribs.getInteger64( XML_y, 0 );
}

void ShapeAnchor::importExt( const AttributeList& rAttribs )
{
    OSL_ENSURE( (meType == ANCHOR_ABSOLUTE) || (meType == ANCHOR_ONECELL), "ShapeAnchor::importExt - unexpected 'xdr:ext' element" );
    maSize.mnWidth = rAttribs.getInteger64( XML_cx, 0 );
    maSize.mnHeight = rAttribs.getInteger64( XML_cy, 0 );
}

void ShapeAnchor::importClientData( const AttributeList& rAttribs )
{
    maClientData.mbLocksWithSheet  = rAttribs.getBool( XML_fLocksWithSheet, true );
    maClientData.mbPrintsWithSheet = rAttribs.getBool( XML_fPrintsWithSheet, true );
}

void ShapeAnchor::setCellPos( sal_Int32 nElement, sal_Int32 nParentContext, const OUString& rValue )
{
    OoxAnchorCell* pAnchorCell = 0;
    switch( nParentContext )
    {
        case XDR_TOKEN( from ):
            OSL_ENSURE( (meType == ANCHOR_ONECELL) || (meType == ANCHOR_TWOCELL), "ShapeAnchor::setCellPos - unexpected 'xdr:from' element" );
            pAnchorCell = &maFrom;
        break;
        case XDR_TOKEN( to ):
            OSL_ENSURE( meType == ANCHOR_TWOCELL, "ShapeAnchor::setCellPos - unexpected 'xdr:to' element" );
            pAnchorCell = &maTo;
        break;
        default:
            OSL_ENSURE( false, "ShapeAnchor::setCellPos - unexpected parent element" );
    }
    if( pAnchorCell ) switch( nElement )
    {
        case XDR_TOKEN( col ):      pAnchorCell->mnCol = rValue.toInt32();          break;
        case XDR_TOKEN( row ):      pAnchorCell->mnRow = rValue.toInt32();          break;
        case XDR_TOKEN( colOff ):   pAnchorCell->mnColOffset = rValue.toInt64();    break;
        case XDR_TOKEN( rowOff ):   pAnchorCell->mnRowOffset = rValue.toInt64();    break;
        default:    OSL_ENSURE( false, "ShapeAnchor::setCellPos - unexpected element" );
    }
}

Rectangle ShapeAnchor::calcApiLocation( const Size& rApiSheetSize, const OoxAnchorSize& rEmuSheetSize ) const
{
    AddressConverter& rAddrConv = getAddressConverter();
    UnitConverter& rUnitConv = getUnitConverter();
    Rectangle aApiLoc( -1, -1, -1, -1 );

    // calculate shape position
    switch( meType )
    {
        case ANCHOR_ABSOLUTE:
            OSL_ENSURE( maPos.isValid(), "ShapeAnchor::calcApiLocation - invalid position" );
            if( maPos.isValid() && (maPos.mnX < rEmuSheetSize.mnWidth) && (maPos.mnY < rEmuSheetSize.mnHeight) )
            {
                aApiLoc.X = rUnitConv.scaleToMm100( static_cast< double >( maPos.mnX ), UNIT_EMU );
                aApiLoc.Y = rUnitConv.scaleToMm100( static_cast< double >( maPos.mnY ), UNIT_EMU );
            }
        break;
        case ANCHOR_ONECELL:
        case ANCHOR_TWOCELL:
            OSL_ENSURE( maFrom.isValid(), "ShapeAnchor::calcApiLocation - invalid position" );
            if( maFrom.isValid() && rAddrConv.checkCol( maFrom.mnCol, true ) && rAddrConv.checkRow( maFrom.mnRow, true ) )
            {
                Point aPoint = getCellPosition( maFrom.mnCol, maFrom.mnRow );
                aApiLoc.X = aPoint.X + rUnitConv.scaleToMm100( static_cast< double >( maFrom.mnColOffset ), UNIT_EMU );
                aApiLoc.Y = aPoint.Y + rUnitConv.scaleToMm100( static_cast< double >( maFrom.mnRowOffset ), UNIT_EMU );
            }
        break;
        case ANCHOR_INVALID:
            OSL_ENSURE( false, "ShapeAnchor::calcApiLocation - invalid anchor" );
        break;
    }

    // calculate shape size
    if( (aApiLoc.X >= 0) && (aApiLoc.Y >= 0) ) switch( meType )
    {
        case ANCHOR_ABSOLUTE:
        case ANCHOR_ONECELL:
            OSL_ENSURE( maSize.isValid(), "ShapeAnchor::calcApiLocation - invalid size" );
            if( maSize.isValid() )
            {
                aApiLoc.Width = ::std::min< sal_Int32 >(
                    rUnitConv.scaleToMm100( static_cast< double >( maSize.mnWidth ), UNIT_EMU ),
                    rApiSheetSize.Width - aApiLoc.X );
                aApiLoc.Height = ::std::min< sal_Int32 >(
                    rUnitConv.scaleToMm100( static_cast< double >( maSize.mnHeight ), UNIT_EMU ),
                    rApiSheetSize.Height - aApiLoc.Y );
            }
        break;
        case ANCHOR_TWOCELL:
            OSL_ENSURE( maTo.isValid(), "ShapeAnchor::calcApiLocation - invalid position" );
            if( maTo.isValid() )
            {
                /*  Pass a valid cell address to getCellPosition(), otherwise
                    nothing is returned, even if either row or column is valid. */
                CellAddress aToCell = rAddrConv.createValidCellAddress( BinAddress( maTo.mnCol, maTo.mnRow ), getSheetIndex(), true );
                Point aPoint = getCellPosition( aToCell.Column, aToCell.Row );
                // width
                aApiLoc.Width = rApiSheetSize.Width - aApiLoc.X;
                if( aToCell.Column == maTo.mnCol )
                {
                    aPoint.X += rUnitConv.scaleToMm100( static_cast< double >( maTo.mnColOffset ), UNIT_EMU );
                    aApiLoc.Width = ::std::min< sal_Int32 >( aPoint.X - aApiLoc.X + 1, aApiLoc.Width );
                }
                // height
                aApiLoc.Height = rApiSheetSize.Height - aApiLoc.Y;
                if( aToCell.Row == maTo.mnRow )
                {
                    aPoint.Y += rUnitConv.scaleToMm100( static_cast< double >( maTo.mnRowOffset ), UNIT_EMU );
                    aApiLoc.Height = ::std::min< sal_Int32 >( aPoint.Y - aApiLoc.Y + 1, aApiLoc.Height );
                }
            }
        break;
        case ANCHOR_INVALID:
        break;
    }

    return aApiLoc;
}

Rectangle ShapeAnchor::calcEmuLocation( const OoxAnchorSize& rEmuSheetSize ) const
{
    AddressConverter& rAddrConv = getAddressConverter();
    UnitConverter& rUnitConv = getUnitConverter();

    Size aSheetSize(
        getLimitedValue< sal_Int32, sal_Int64 >( rEmuSheetSize.mnWidth, 0, SAL_MAX_INT32 ),
        getLimitedValue< sal_Int32, sal_Int64 >( rEmuSheetSize.mnHeight, 0, SAL_MAX_INT32 ) );
    Rectangle aLoc( -1, -1, -1, -1 );

    // calculate shape position
    switch( meType )
    {
        case ANCHOR_ABSOLUTE:
            OSL_ENSURE( maPos.isValid(), "ShapeAnchor::calcEmuLocation - invalid position" );
            if( maPos.isValid() && (maPos.mnX < aSheetSize.Width) && (maPos.mnY < aSheetSize.Height) )
            {
                aLoc.X = static_cast< sal_Int32 >( maPos.mnX );
                aLoc.Y = static_cast< sal_Int32 >( maPos.mnY );
            }
        break;
        case ANCHOR_ONECELL:
        case ANCHOR_TWOCELL:
            OSL_ENSURE( maFrom.isValid(), "ShapeAnchor::calcEmuLocation - invalid position" );
            if( maFrom.isValid() && rAddrConv.checkCol( maFrom.mnCol, true ) && rAddrConv.checkRow( maFrom.mnRow, true ) )
            {
                Point aPoint = getCellPosition( maFrom.mnCol, maFrom.mnRow );
                sal_Int64 nX = static_cast< sal_Int64 >( rUnitConv.scaleFromMm100( aPoint.X, UNIT_EMU ) ) + maFrom.mnColOffset;
                sal_Int64 nY = static_cast< sal_Int64 >( rUnitConv.scaleFromMm100( aPoint.Y, UNIT_EMU ) ) + maFrom.mnRowOffset;
                if( (nX < aSheetSize.Width) && (nY < aSheetSize.Height) )
                {
                    aLoc.X = static_cast< sal_Int32 >( nX );
                    aLoc.Y = static_cast< sal_Int32 >( nY );
                }
            }
        break;
        case ANCHOR_INVALID:
            OSL_ENSURE( false, "ShapeAnchor::calcEmuLocation - invalid anchor" );
        break;
    }

    // calculate shape size
    if( (aLoc.X >= 0) && (aLoc.Y >= 0) ) switch( meType )
    {
        case ANCHOR_ABSOLUTE:
        case ANCHOR_ONECELL:
            OSL_ENSURE( maSize.isValid(), "ShapeAnchor::calcEmuLocation - invalid size" );
            if( maSize.isValid() )
            {
                aLoc.Width = static_cast< sal_Int32 >( ::std::min< sal_Int64 >( maSize.mnWidth, aSheetSize.Width - aLoc.X ) );
                aLoc.Height = static_cast< sal_Int32 >( ::std::min< sal_Int64 >( maSize.mnHeight, aSheetSize.Height - aLoc.Y ) );
            }
        break;
        case ANCHOR_TWOCELL:
            OSL_ENSURE( maTo.isValid(), "ShapeAnchor::calcEmuLocation - invalid position" );
            if( maTo.isValid() )
            {
                /*  Pass a valid cell address to getCellPosition(), otherwise
                    nothing is returned, even if either row or column is valid. */
                CellAddress aToCell = rAddrConv.createValidCellAddress( BinAddress( maTo.mnCol, maTo.mnRow ), getSheetIndex(), true );
                Point aPoint = getCellPosition( aToCell.Column, aToCell.Row );
                sal_Int64 nX = static_cast< sal_Int64 >( rUnitConv.scaleFromMm100( aPoint.X, UNIT_EMU ) );
                sal_Int64 nY = static_cast< sal_Int64 >( rUnitConv.scaleFromMm100( aPoint.Y, UNIT_EMU ) );
                // width
                aLoc.Width = aSheetSize.Width - aLoc.X;
                if( aToCell.Column == maTo.mnCol )
                {
                    nX += maTo.mnColOffset;
                    aLoc.Width = static_cast< sal_Int32 >( ::std::min< sal_Int64 >( nX - aLoc.X + 1, aLoc.Width ) );
                }
                // height
                aLoc.Height = aSheetSize.Height - aLoc.Y;
                if( aToCell.Row == maTo.mnRow )
                {
                    nY += maTo.mnRowOffset;
                    aLoc.Height = static_cast< sal_Int32 >( ::std::min< sal_Int64 >( nY - aLoc.Y + 1, aLoc.Height ) );
                }
            }
        break;
        case ANCHOR_INVALID:
        break;
    }

    // add 1 mm (36K EMUs) in X direction to correct display error
    if( aLoc.X >= 0 )
        aLoc.X += 36000;

    return aLoc;
}

// ============================================================================

OoxDrawingFragment::OoxDrawingFragment( const WorksheetHelper& rHelper, const OUString& rFragmentPath ) :
    OoxWorksheetFragmentBase( rHelper, rFragmentPath )
{
    Reference< XDrawPageSupplier > xDrawPageSupp( getXSpreadsheet(), UNO_QUERY );
    if( xDrawPageSupp.is() )
        mxDrawPage.set( xDrawPageSupp->getDrawPage(), UNO_QUERY );
    OSL_ENSURE( mxDrawPage.is(), "OoxDrawingFragment::OoxDrawingFragment - missing drawing page" );

    maApiSheetSize = getDrawPageSize();
    maEmuSheetSize.mnWidth = static_cast< sal_Int64 >( getUnitConverter().scaleFromMm100( maApiSheetSize.Width, UNIT_EMU ) );
    maEmuSheetSize.mnHeight = static_cast< sal_Int64 >( getUnitConverter().scaleFromMm100( maApiSheetSize.Height, UNIT_EMU ) );
}

// oox.core.ContextHandler2Helper interface -----------------------------------

ContextWrapper OoxDrawingFragment::onCreateContext( sal_Int32 nElement, const AttributeList& )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            return  (nElement == XDR_TOKEN( wsDr ));
        case XDR_TOKEN( wsDr ):
            return  (nElement == XDR_TOKEN( absoluteAnchor )) ||
                    (nElement == XDR_TOKEN( oneCellAnchor )) ||
                    (nElement == XDR_TOKEN( twoCellAnchor ));
        case XDR_TOKEN( absoluteAnchor ):
        case XDR_TOKEN( oneCellAnchor ):
        case XDR_TOKEN( twoCellAnchor ):
            switch( nElement )
            {
                case XDR_TOKEN( sp ):
                    mxShape.reset( new Shape( "com.sun.star.drawing.CustomShape" ) );
                    return new ShapeContext( *this, ShapePtr(), mxShape );
                case XDR_TOKEN( cxnSp ):
                    mxShape.reset( new Shape( "com.sun.star.drawing.ConnectorShape" ) );
                    return new ConnectorShapeContext( *this, ShapePtr(), mxShape );
                case XDR_TOKEN( pic ):
                    mxShape.reset( new Shape( "com.sun.star.drawing.GraphicObjectShape" ) );
                    return new GraphicShapeContext( *this, ShapePtr(), mxShape );
                case XDR_TOKEN( graphicFrame ):
                    mxShape.reset( new Shape( "com.sun.star.drawing.OLE2Shape" ) );
                    return new GraphicalObjectFrameContext( *this, ShapePtr(), mxShape );
                case XDR_TOKEN( grpSp ):
                    mxShape.reset( new Shape( "com.sun.star.drawing.GroupShape" ) );
                    return new ShapeGroupContext( *this, ShapePtr(), mxShape );
            }
            return  (nElement == XDR_TOKEN( pos )) ||
                    (nElement == XDR_TOKEN( ext )) ||
                    (nElement == XDR_TOKEN( from )) ||
                    (nElement == XDR_TOKEN( to )) ||
                    (nElement == XDR_TOKEN( clientData ));
        case XDR_TOKEN( from ):
        case XDR_TOKEN( to ):
            return  (nElement == XDR_TOKEN( col )) ||
                    (nElement == XDR_TOKEN( row )) ||
                    (nElement == XDR_TOKEN( colOff )) ||
                    (nElement == XDR_TOKEN( rowOff ));
    }
    return false;
}

void OoxDrawingFragment::onStartElement( const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XDR_TOKEN( absoluteAnchor ):
            mxAnchor.reset( new ShapeAnchor( *this ) );
            mxAnchor->importAbsoluteAnchor( rAttribs );
        break;
        case XDR_TOKEN( oneCellAnchor ):
            mxAnchor.reset( new ShapeAnchor( *this ) );
            mxAnchor->importOneCellAnchor( rAttribs );
        break;
        case XDR_TOKEN( twoCellAnchor ):
            mxAnchor.reset( new ShapeAnchor( *this ) );
            mxAnchor->importTwoCellAnchor( rAttribs );
        break;
        case XDR_TOKEN( pos ):
            if( mxAnchor.get() ) mxAnchor->importPos( rAttribs );
        break;
        case XDR_TOKEN( ext ):
            if( mxAnchor.get() ) mxAnchor->importExt( rAttribs );
        break;
        case XDR_TOKEN( clientData ):
            if( mxAnchor.get() ) mxAnchor->importClientData( rAttribs );
        break;
    }
}

void OoxDrawingFragment::onEndElement( const OUString& rChars )
{
    switch( getCurrentElement() )
    {
        case XDR_TOKEN( col ):
        case XDR_TOKEN( row ):
        case XDR_TOKEN( colOff ):
        case XDR_TOKEN( rowOff ):
            if( mxAnchor.get() ) mxAnchor->setCellPos( getCurrentElement(), getPreviousElement(), rChars );
        break;
        case XDR_TOKEN( absoluteAnchor ):
        case XDR_TOKEN( oneCellAnchor ):
        case XDR_TOKEN( twoCellAnchor ):
            if( mxDrawPage.is() && mxShape.get() && mxAnchor.get() )
            {
                Rectangle aLoc = mxAnchor->calcEmuLocation( maEmuSheetSize );
                if( (aLoc.X >= 0) && (aLoc.Y >= 0) && (aLoc.Width >= 0) && (aLoc.Height >= 0) )
                    mxShape->addShape( getOoxFilter(), getTheme().getCoreThemePtr(), mxDrawPage, &aLoc );
            }
            mxShape.reset();
            mxAnchor.reset();
        break;
    }
}

// ============================================================================

} // namespace xls
} // namespace oox

