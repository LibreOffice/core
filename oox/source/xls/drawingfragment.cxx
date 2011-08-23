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

#include "oox/xls/drawingfragment.hxx"
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/form/binding/XBindableValue.hpp>
#include <com/sun/star/form/binding/XListEntrySink.hpp>
#include <com/sun/star/form/binding/XListEntrySource.hpp>
#include <com/sun/star/form/binding/XValueBinding.hpp>
#include "properties.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/drawingml/connectorshapecontext.hxx"
#include "oox/drawingml/graphicshapecontext.hxx"
#include "oox/drawingml/shapecontext.hxx"
#include "oox/drawingml/shapegroupcontext.hxx"
#include "oox/vml/vmlshape.hxx"
#include "oox/vml/vmlshapecontainer.hxx"
#include "oox/xls/formulaparser.hxx"
#include "oox/xls/stylesbuffer.hxx"
#include "oox/xls/themebuffer.hxx"
#include "oox/xls/unitconverter.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::beans::NamedValue;
using ::com::sun::star::awt::Point;
using ::com::sun::star::awt::Rectangle;
using ::com::sun::star::awt::Size;
using ::com::sun::star::awt::XControlModel;
using ::com::sun::star::form::binding::XBindableValue;
using ::com::sun::star::form::binding::XListEntrySink;
using ::com::sun::star::form::binding::XListEntrySource;
using ::com::sun::star::form::binding::XValueBinding;
using ::com::sun::star::drawing::XShape;
using ::com::sun::star::table::CellAddress;
using ::com::sun::star::table::CellRangeAddress;
using ::oox::core::ContextHandlerRef;
using ::oox::drawingml::ConnectorShapeContext;
using ::oox::drawingml::GraphicalObjectFrameContext;
using ::oox::drawingml::GraphicShapeContext;
using ::oox::drawingml::Shape;
using ::oox::drawingml::ShapePtr;
using ::oox::drawingml::ShapeContext;
using ::oox::drawingml::ShapeGroupContext;
// no using's for ::oox::vml, that may clash with ::oox::drawingml types

namespace oox {
namespace xls {

// ============================================================================

namespace {

/** Converts the passed 64-bit integer value from the passed unit to EMUs. */
sal_Int64 lclCalcEmu( const UnitConverter& rUnitConv, sal_Int64 nValue, Unit eFromUnit )
{
    return (eFromUnit == UNIT_EMU) ? nValue :
        static_cast< sal_Int64 >( rUnitConv.scaleValue( static_cast< double >( nValue ), eFromUnit, UNIT_EMU ) + 0.5 );
}

} // namespace

// ============================================================================

AnchorCellModel::AnchorCellModel() :
    mnCol( -1 ),
    mnRow( -1 ),
    mnColOffset( 0 ),
    mnRowOffset( 0 )
{
}

// ----------------------------------------------------------------------------

AnchorClientDataModel::AnchorClientDataModel() :
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

void ShapeAnchor::importAnchor( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( nElement )
    {
        case XDR_TOKEN( absoluteAnchor ):
            meType = ANCHOR_ABSOLUTE;
        break;
        case XDR_TOKEN( oneCellAnchor ):
            meType = ANCHOR_ONECELL;
        break;
        case XDR_TOKEN( twoCellAnchor ):
            meType = ANCHOR_TWOCELL;
            mnEditAs = rAttribs.getToken( XML_editAs, XML_twoCell );
        break;
        default:
            OSL_ENSURE( false, "ShapeAnchor::importAnchor - unexpected element" );
    }
}

void ShapeAnchor::importPos( const AttributeList& rAttribs )
{
    OSL_ENSURE( meType == ANCHOR_ABSOLUTE, "ShapeAnchor::importPos - unexpected 'xdr:pos' element" );
    maPos.X = rAttribs.getHyper( XML_x, 0 );
    maPos.Y = rAttribs.getHyper( XML_y, 0 );
}

void ShapeAnchor::importExt( const AttributeList& rAttribs )
{
    OSL_ENSURE( (meType == ANCHOR_ABSOLUTE) || (meType == ANCHOR_ONECELL), "ShapeAnchor::importExt - unexpected 'xdr:ext' element" );
    maSize.Width = rAttribs.getHyper( XML_cx, 0 );
    maSize.Height = rAttribs.getHyper( XML_cy, 0 );
}

void ShapeAnchor::importClientData( const AttributeList& rAttribs )
{
    maClientData.mbLocksWithSheet  = rAttribs.getBool( XML_fLocksWithSheet, true );
    maClientData.mbPrintsWithSheet = rAttribs.getBool( XML_fPrintsWithSheet, true );
}

void ShapeAnchor::setCellPos( sal_Int32 nElement, sal_Int32 nParentContext, const OUString& rValue )
{
    AnchorCellModel* pAnchorCell = 0;
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

void ShapeAnchor::importVmlAnchor( const OUString& rAnchor )
{
    meType = ANCHOR_VML;

    ::std::vector< OUString > aTokens;
    sal_Int32 nIndex = 0;
    while( nIndex >= 0 )
        aTokens.push_back( rAnchor.getToken( 0, ',', nIndex ).trim() );

    OSL_ENSURE( aTokens.size() >= 8, "ShapeAnchor::importVmlAnchor - missing anchor tokens" );
    if( aTokens.size() >= 8 )
    {
        maFrom.mnCol       = aTokens[ 0 ].toInt32();
        maFrom.mnColOffset = aTokens[ 1 ].toInt32();
        maFrom.mnRow       = aTokens[ 2 ].toInt32();
        maFrom.mnRowOffset = aTokens[ 3 ].toInt32();
        maTo.mnCol         = aTokens[ 4 ].toInt32();
        maTo.mnColOffset   = aTokens[ 5 ].toInt32();
        maTo.mnRow         = aTokens[ 6 ].toInt32();
        maTo.mnRowOffset   = aTokens[ 7 ].toInt32();
    }
}

bool ShapeAnchor::isValidAnchor() const
{
    bool bValid = false;
    switch( meType )
    {
        case ANCHOR_ABSOLUTE:
            OSL_ENSURE( maPos.isValid(), "ShapeAnchor::isValidAnchor - invalid position" );
            OSL_ENSURE( maSize.isValid(), "ShapeAnchor::isValidAnchor - invalid size" );
            bValid = maPos.isValid() && maSize.isValid() && (maSize.Width > 0) && (maSize.Height > 0);
        break;
        case ANCHOR_ONECELL:
            OSL_ENSURE( maFrom.isValid(), "ShapeAnchor::isValidAnchor - invalid from position" );
            OSL_ENSURE( maSize.isValid(), "ShapeAnchor::isValidAnchor - invalid size" );
            bValid = maFrom.isValid() && maSize.isValid() && (maSize.Width > 0) && (maSize.Height > 0);
        break;
        case ANCHOR_TWOCELL:
        case ANCHOR_VML:
            OSL_ENSURE( maFrom.isValid(), "ShapeAnchor::isValidAnchor - invalid from position" );
            OSL_ENSURE( maTo.isValid(), "ShapeAnchor::isValidAnchor - invalid to position" );
            bValid = maFrom.isValid() && maTo.isValid() &&
                ((maFrom.mnCol < maTo.mnCol) || ((maFrom.mnCol == maTo.mnCol) && (maFrom.mnColOffset < maTo.mnColOffset))) &&
                ((maFrom.mnRow < maTo.mnRow) || ((maFrom.mnRow == maTo.mnRow) && (maFrom.mnRowOffset < maTo.mnRowOffset)));
        break;
        case ANCHOR_INVALID:
            OSL_ENSURE( false, "ShapeAnchor::isValidAnchor - invalid anchor" );
        break;
    }
    return bValid;
}

Rectangle ShapeAnchor::calcApiLocation( const Size& rApiSheetSize, const AnchorSizeModel& rEmuSheetSize ) const
{
    AddressConverter& rAddrConv = getAddressConverter();
    UnitConverter& rUnitConv = getUnitConverter();
    Rectangle aApiLoc( -1, -1, -1, -1 );
    Unit eUnitX = (meType == ANCHOR_VML) ? UNIT_SCREENX : UNIT_EMU;
    Unit eUnitY = (meType == ANCHOR_VML) ? UNIT_SCREENY : UNIT_EMU;

    // calculate shape position
    switch( meType )
    {
        case ANCHOR_ABSOLUTE:
            OSL_ENSURE( maPos.isValid(), "ShapeAnchor::calcApiLocation - invalid position" );
            if( maPos.isValid() && (maPos.X < rEmuSheetSize.Width) && (maPos.Y < rEmuSheetSize.Height) )
            {
                aApiLoc.X = rUnitConv.scaleToMm100( static_cast< double >( maPos.X ), UNIT_EMU );
                aApiLoc.Y = rUnitConv.scaleToMm100( static_cast< double >( maPos.Y ), UNIT_EMU );
            }
        break;
        case ANCHOR_ONECELL:
        case ANCHOR_TWOCELL:
        case ANCHOR_VML:
            OSL_ENSURE( maFrom.isValid(), "ShapeAnchor::calcApiLocation - invalid position" );
            if( maFrom.isValid() && rAddrConv.checkCol( maFrom.mnCol, true ) && rAddrConv.checkRow( maFrom.mnRow, true ) )
            {
                Point aPoint = getCellPosition( maFrom.mnCol, maFrom.mnRow );
                aApiLoc.X = aPoint.X + rUnitConv.scaleToMm100( static_cast< double >( maFrom.mnColOffset ), eUnitX );
                aApiLoc.Y = aPoint.Y + rUnitConv.scaleToMm100( static_cast< double >( maFrom.mnRowOffset ), eUnitY );
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
                    rUnitConv.scaleToMm100( static_cast< double >( maSize.Width ), UNIT_EMU ),
                    rApiSheetSize.Width - aApiLoc.X );
                aApiLoc.Height = ::std::min< sal_Int32 >(
                    rUnitConv.scaleToMm100( static_cast< double >( maSize.Height ), UNIT_EMU ),
                    rApiSheetSize.Height - aApiLoc.Y );
            }
        break;
        case ANCHOR_TWOCELL:
        case ANCHOR_VML:
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
                    aPoint.X += rUnitConv.scaleToMm100( static_cast< double >( maTo.mnColOffset ), eUnitX );
                    aApiLoc.Width = ::std::min< sal_Int32 >( aPoint.X - aApiLoc.X + 1, aApiLoc.Width );
                }
                // height
                aApiLoc.Height = rApiSheetSize.Height - aApiLoc.Y;
                if( aToCell.Row == maTo.mnRow )
                {
                    aPoint.Y += rUnitConv.scaleToMm100( static_cast< double >( maTo.mnRowOffset ), eUnitY );
                    aApiLoc.Height = ::std::min< sal_Int32 >( aPoint.Y - aApiLoc.Y + 1, aApiLoc.Height );
                }
            }
        break;
        case ANCHOR_INVALID:
        break;
    }

    return aApiLoc;
}

Rectangle ShapeAnchor::calcEmuLocation( const AnchorSizeModel& rEmuSheetSize ) const
{
    AddressConverter& rAddrConv = getAddressConverter();
    UnitConverter& rUnitConv = getUnitConverter();

    Size aSheetSize(
        getLimitedValue< sal_Int32, sal_Int64 >( rEmuSheetSize.Width, 0, SAL_MAX_INT32 ),
        getLimitedValue< sal_Int32, sal_Int64 >( rEmuSheetSize.Height, 0, SAL_MAX_INT32 ) );
    Rectangle aLoc( -1, -1, -1, -1 );
    Unit eUnitX = (meType == ANCHOR_VML) ? UNIT_SCREENX : UNIT_EMU;
    Unit eUnitY = (meType == ANCHOR_VML) ? UNIT_SCREENY : UNIT_EMU;

    // calculate shape position
    switch( meType )
    {
        case ANCHOR_ABSOLUTE:
            OSL_ENSURE( maPos.isValid(), "ShapeAnchor::calcEmuLocation - invalid position" );
            if( maPos.isValid() && (maPos.X < aSheetSize.Width) && (maPos.Y < aSheetSize.Height) )
            {
                aLoc.X = static_cast< sal_Int32 >( maPos.X );
                aLoc.Y = static_cast< sal_Int32 >( maPos.Y );
            }
        break;
        case ANCHOR_ONECELL:
        case ANCHOR_TWOCELL:
        case ANCHOR_VML:
            OSL_ENSURE( maFrom.isValid(), "ShapeAnchor::calcEmuLocation - invalid position" );
            if( maFrom.isValid() && rAddrConv.checkCol( maFrom.mnCol, true ) && rAddrConv.checkRow( maFrom.mnRow, true ) )
            {
                Point aPoint = getCellPosition( maFrom.mnCol, maFrom.mnRow );
                sal_Int64 nX = static_cast< sal_Int64 >( rUnitConv.scaleFromMm100( aPoint.X, UNIT_EMU ) ) + lclCalcEmu( rUnitConv, maFrom.mnColOffset, eUnitX );
                sal_Int64 nY = static_cast< sal_Int64 >( rUnitConv.scaleFromMm100( aPoint.Y, UNIT_EMU ) ) + lclCalcEmu( rUnitConv, maFrom.mnRowOffset, eUnitY );
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
                aLoc.Width = static_cast< sal_Int32 >( ::std::min< sal_Int64 >( maSize.Width, aSheetSize.Width - aLoc.X ) );
                aLoc.Height = static_cast< sal_Int32 >( ::std::min< sal_Int64 >( maSize.Height, aSheetSize.Height - aLoc.Y ) );
            }
        break;
        case ANCHOR_TWOCELL:
        case ANCHOR_VML:
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
                    nX += lclCalcEmu( rUnitConv, maTo.mnColOffset, eUnitX );
                    aLoc.Width = static_cast< sal_Int32 >( ::std::min< sal_Int64 >( nX - aLoc.X + 1, aLoc.Width ) );
                }
                // height
                aLoc.Height = aSheetSize.Height - aLoc.Y;
                if( aToCell.Row == maTo.mnRow )
                {
                    nY += lclCalcEmu( rUnitConv, maTo.mnRowOffset, eUnitY );
                    aLoc.Height = static_cast< sal_Int32 >( ::std::min< sal_Int64 >( nY - aLoc.Y + 1, aLoc.Height ) );
                }
            }
        break;
        case ANCHOR_INVALID:
        break;
    }

    // add 0.75 mm (27,000 EMUs) in X direction to correct display error
    if( aLoc.X >= 0 )
        aLoc.X += 27000;
    // remove 0.25 mm (9,000 EMUs) in Y direction to correct display error
    if( aLoc.Y >= 9000 )
        aLoc.Y -= 9000;

    return aLoc;
}

// ============================================================================

OoxDrawingFragment::OoxDrawingFragment( const WorksheetHelper& rHelper, const OUString& rFragmentPath ) :
    OoxWorksheetFragmentBase( rHelper, rFragmentPath ),
    mxDrawPage( rHelper.getDrawPage(), UNO_QUERY )
{
    OSL_ENSURE( mxDrawPage.is(), "OoxDrawingFragment::OoxDrawingFragment - missing drawing page" );
    maApiSheetSize = getDrawPageSize();
    maEmuSheetSize.Width = static_cast< sal_Int64 >( getUnitConverter().scaleFromMm100( maApiSheetSize.Width, UNIT_EMU ) );
    maEmuSheetSize.Height = static_cast< sal_Int64 >( getUnitConverter().scaleFromMm100( maApiSheetSize.Height, UNIT_EMU ) );
}

// oox.core.ContextHandler2Helper interface -----------------------------------

ContextHandlerRef OoxDrawingFragment::onCreateContext( sal_Int32 nElement, const AttributeList& rAttribs )
{
    switch( getCurrentElement() )
    {
        case XML_ROOT_CONTEXT:
            if( nElement == XDR_TOKEN( wsDr ) ) return this;
        break;

        case XDR_TOKEN( wsDr ):
            switch( nElement )
            {
                case XDR_TOKEN( absoluteAnchor ):
                case XDR_TOKEN( oneCellAnchor ):
                case XDR_TOKEN( twoCellAnchor ):
                    mxAnchor.reset( new ShapeAnchor( *this ) );
                    mxAnchor->importAnchor( nElement, rAttribs );
                    return this;
            }
        break;

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
                    return new GraphicalObjectFrameContext( *this, ShapePtr(), mxShape, getSheetType() != SHEETTYPE_CHARTSHEET );
                case XDR_TOKEN( grpSp ):
                    mxShape.reset( new Shape( "com.sun.star.drawing.GroupShape" ) );
                    return new ShapeGroupContext( *this, ShapePtr(), mxShape );

                case XDR_TOKEN( from ):
                case XDR_TOKEN( to ):           return this;

                case XDR_TOKEN( pos ):          if( mxAnchor.get() ) mxAnchor->importPos( rAttribs );           break;
                case XDR_TOKEN( ext ):          if( mxAnchor.get() ) mxAnchor->importExt( rAttribs );           break;
                case XDR_TOKEN( clientData ):   if( mxAnchor.get() ) mxAnchor->importClientData( rAttribs );    break;
            }
        break;

        case XDR_TOKEN( from ):
        case XDR_TOKEN( to ):
            switch( nElement )
            {
                case XDR_TOKEN( col ):
                case XDR_TOKEN( row ):
                case XDR_TOKEN( colOff ):
                case XDR_TOKEN( rowOff ):       return this;    // collect index in onEndElement()
            }
        break;
    }
    return 0;
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
            if( mxDrawPage.is() && mxShape.get() && mxAnchor.get() && mxAnchor->isValidAnchor() )
            {
                Rectangle aShapeRect = mxAnchor->calcEmuLocation( maEmuSheetSize );
                if( (aShapeRect.X >= 0) && (aShapeRect.Y >= 0) && (aShapeRect.Width >= 0) && (aShapeRect.Height >= 0) )
                {
                    mxShape->addShape( getOoxFilter(), &getTheme(), mxDrawPage, &aShapeRect );
                    // collect all shape positions in the WorksheetHelper base class
                    extendShapeBoundingBox( aShapeRect );
                }
            }
            mxShape.reset();
            mxAnchor.reset();
        break;
    }
}

// ============================================================================

namespace {

class VmlFindNoteFunc
{
public:
    explicit            VmlFindNoteFunc( const CellAddress& rPos );
    bool                operator()( const ::oox::vml::ShapeBase& rShape ) const;

private:
    sal_Int32           mnCol;
    sal_Int32           mnRow;
};

VmlFindNoteFunc::VmlFindNoteFunc( const CellAddress& rPos ) :
    mnCol( rPos.Column ),
    mnRow( rPos.Row )
{
}

bool VmlFindNoteFunc::operator()( const ::oox::vml::ShapeBase& rShape ) const
{
    const ::oox::vml::ShapeModel::ShapeClientDataPtr& rxClientData = rShape.getShapeModel().mxClientData;
    return rxClientData.get() && (rxClientData->mnCol == mnCol) && (rxClientData->mnRow == mnRow);
}

} // namespace

// ----------------------------------------------------------------------------

VmlDrawing::VmlDrawing( const WorksheetHelper& rHelper ) :
    ::oox::vml::Drawing( rHelper.getOoxFilter(), rHelper.getDrawPage(), ::oox::vml::VMLDRAWING_EXCEL ),
    WorksheetHelper( rHelper )
{
}

const ::oox::vml::ShapeBase* VmlDrawing::getNoteShape( const CellAddress& rPos ) const
{
    return getShapes().findShape( VmlFindNoteFunc( rPos ) );
}

bool VmlDrawing::isShapeSupported( const ::oox::vml::ShapeBase& rShape ) const
{
    const ::oox::vml::ShapeModel::ShapeClientDataPtr& rxClientData = rShape.getShapeModel().mxClientData;
    return !rxClientData.get() || (rxClientData->mnObjType != XML_Note);
}

bool VmlDrawing::convertShapeClientAnchor( Rectangle& orShapeRect, const OUString& rShapeAnchor ) const
{
    if( rShapeAnchor.getLength() == 0 )
        return false;
    ShapeAnchor aAnchor( *this );
    aAnchor.importVmlAnchor( rShapeAnchor );
    orShapeRect = aAnchor.calcApiLocation( getDrawPageSize(), AnchorSizeModel() );
    return (orShapeRect.Width >= 0) && (orShapeRect.Height >= 0);
}

void VmlDrawing::convertControlClientData( const Reference< XControlModel >& rxCtrlModel,
        const ::oox::vml::ShapeClientData& rClientData ) const
{
    if( rxCtrlModel.is() )
    {
        PropertySet aPropSet( rxCtrlModel );

        // printable
        aPropSet.setProperty( PROP_Printable, rClientData.mbPrintObject );

        // linked cell
        if( rClientData.maLinkedCell.getLength() > 0 ) try
        {
            Reference< XBindableValue > xBindable( rxCtrlModel, UNO_QUERY_THROW );

            // convert formula string to cell address
            FormulaParser& rParser = getFormulaParser();
            TokensFormulaContext aContext( true, false );
            aContext.setBaseAddress( CellAddress( getSheetIndex(), 0, 0 ) );
            rParser.importFormula( aContext, rClientData.maLinkedCell );
            CellAddress aAddress;
            if( rParser.extractCellAddress( aAddress, aContext.getTokens(), true ) )
            {
                // create argument sequence for createInstanceWithArguments()
                NamedValue aValue;
                aValue.Name = CREATE_OUSTRING( "BoundCell" );
                aValue.Value <<= aAddress;
                Sequence< Any > aArgs( 1 );
                aArgs[ 0 ] <<= aValue;

                // create the CellValueBinding instance and set at the control model
                Reference< XValueBinding > xBinding( getDocumentFactory()->createInstanceWithArguments(
                    CREATE_OUSTRING( "com.sun.star.table.CellValueBinding" ), aArgs ), UNO_QUERY_THROW );
                xBindable->setValueBinding( xBinding );
            }
        }
        catch( Exception& )
        {
        }

        // source range
        if( rClientData.maSourceRange.getLength() > 0 ) try
        {
            Reference< XListEntrySink > xEntrySink( rxCtrlModel, UNO_QUERY_THROW );

            // convert formula string to cell range
            FormulaParser& rParser = getFormulaParser();
            TokensFormulaContext aContext( true, false );
            aContext.setBaseAddress( CellAddress( getSheetIndex(), 0, 0 ) );
            rParser.importFormula( aContext, rClientData.maSourceRange );
            CellRangeAddress aRange;
            if( rParser.extractCellRange( aRange, aContext.getTokens(), true ) )
            {
                // create argument sequence for createInstanceWithArguments()
                NamedValue aValue;
                aValue.Name = CREATE_OUSTRING( "CellRange" );
                aValue.Value <<= aRange;
                Sequence< Any > aArgs( 1 );
                aArgs[ 0 ] <<= aValue;

                // create the EntrySource instance and set at the control model
                Reference< XListEntrySource > xEntrySource( getDocumentFactory()->createInstanceWithArguments(
                    CREATE_OUSTRING( "com.sun.star.table.CellRangeListSource"  ), aArgs ), UNO_QUERY_THROW );
                xEntrySink->setListEntrySource( xEntrySource );
            }
        }
        catch( Exception& )
        {
        }
    }
}

void VmlDrawing::notifyShapeInserted( const Reference< XShape >& /*rxShape*/, const Rectangle& rShapeRect )
{
    // collect all shape positions in the WorksheetHelper base class
    extendShapeBoundingBox( rShapeRect );
}

// ============================================================================

OoxVmlDrawingFragment::OoxVmlDrawingFragment( const WorksheetHelper& rHelper, const OUString& rFragmentPath ) :
    ::oox::vml::DrawingFragment( rHelper.getOoxFilter(), rFragmentPath, rHelper.getVmlDrawing() ),
    WorksheetHelper( rHelper )
{
}

void OoxVmlDrawingFragment::finalizeImport()
{
    ::oox::vml::DrawingFragment::finalizeImport();
    getVmlDrawing().convertAndInsert();
}

// ============================================================================

} // namespace xls
} // namespace oox

