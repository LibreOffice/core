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

#include "drawingmanager.hxx"

#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/drawing/CircleKind.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/drawing/PolygonKind.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include "oox/core/filterbase.hxx"
#include "oox/drawingml/fillproperties.hxx"
#include "oox/drawingml/lineproperties.hxx"
#include "oox/drawingml/shapepropertymap.hxx"
#include "oox/helper/containerhelper.hxx"
#include "oox/token/tokens.hxx"
#include "biffinputstream.hxx"
#include "unitconverter.hxx"

namespace oox {
namespace xls {

// ============================================================================

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::oox::drawingml;

using ::rtl::OUString;

// ============================================================================

namespace {

// OBJ record -----------------------------------------------------------------

const sal_uInt16 BIFF_OBJTYPE_GROUP         = 0;
const sal_uInt16 BIFF_OBJTYPE_LINE          = 1;
const sal_uInt16 BIFF_OBJTYPE_RECTANGLE     = 2;
const sal_uInt16 BIFF_OBJTYPE_OVAL          = 3;
const sal_uInt16 BIFF_OBJTYPE_ARC           = 4;
const sal_uInt16 BIFF_OBJTYPE_CHART         = 5;
const sal_uInt16 BIFF_OBJTYPE_TEXT          = 6;
const sal_uInt16 BIFF_OBJTYPE_BUTTON        = 7;
const sal_uInt16 BIFF_OBJTYPE_PICTURE       = 8;
const sal_uInt16 BIFF_OBJTYPE_POLYGON       = 9;        // new in BIFF4
const sal_uInt16 BIFF_OBJTYPE_CHECKBOX      = 11;       // new in BIFF5
const sal_uInt16 BIFF_OBJTYPE_OPTIONBUTTON  = 12;
const sal_uInt16 BIFF_OBJTYPE_EDIT          = 13;
const sal_uInt16 BIFF_OBJTYPE_LABEL         = 14;
const sal_uInt16 BIFF_OBJTYPE_DIALOG        = 15;
const sal_uInt16 BIFF_OBJTYPE_SPIN          = 16;
const sal_uInt16 BIFF_OBJTYPE_SCROLLBAR     = 17;
const sal_uInt16 BIFF_OBJTYPE_LISTBOX       = 18;
const sal_uInt16 BIFF_OBJTYPE_GROUPBOX      = 19;
const sal_uInt16 BIFF_OBJTYPE_DROPDOWN      = 20;
const sal_uInt16 BIFF_OBJTYPE_NOTE          = 25;       // new in BIFF8
const sal_uInt16 BIFF_OBJTYPE_DRAWING       = 30;
const sal_uInt16 BIFF_OBJTYPE_UNKNOWN       = 0xFFFF;   // for internal use only

const sal_uInt16 BIFF_OBJ_HIDDEN            = 0x0100;
const sal_uInt16 BIFF_OBJ_VISIBLE           = 0x0200;
const sal_uInt16 BIFF_OBJ_PRINTABLE         = 0x0400;

// line formatting ------------------------------------------------------------

const sal_uInt8 BIFF_OBJ_LINE_AUTOCOLOR     = 64;

const sal_uInt8 BIFF_OBJ_LINE_SOLID         = 0;
const sal_uInt8 BIFF_OBJ_LINE_DASH          = 1;
const sal_uInt8 BIFF_OBJ_LINE_DOT           = 2;
const sal_uInt8 BIFF_OBJ_LINE_DASHDOT       = 3;
const sal_uInt8 BIFF_OBJ_LINE_DASHDOTDOT    = 4;
const sal_uInt8 BIFF_OBJ_LINE_MEDTRANS      = 5;
const sal_uInt8 BIFF_OBJ_LINE_DARKTRANS     = 6;
const sal_uInt8 BIFF_OBJ_LINE_LIGHTTRANS    = 7;
const sal_uInt8 BIFF_OBJ_LINE_NONE          = 255;

const sal_uInt8 BIFF_OBJ_LINE_HAIR          = 0;
const sal_uInt8 BIFF_OBJ_LINE_THIN          = 1;
const sal_uInt8 BIFF_OBJ_LINE_MEDIUM        = 2;
const sal_uInt8 BIFF_OBJ_LINE_THICK         = 3;

const sal_uInt8 BIFF_OBJ_LINE_AUTO          = 0x01;

const sal_uInt8 BIFF_OBJ_ARROW_NONE         = 0;
const sal_uInt8 BIFF_OBJ_ARROW_OPEN         = 1;
const sal_uInt8 BIFF_OBJ_ARROW_FILLED       = 2;
const sal_uInt8 BIFF_OBJ_ARROW_OPENBOTH     = 3;
const sal_uInt8 BIFF_OBJ_ARROW_FILLEDBOTH   = 4;

const sal_uInt8 BIFF_OBJ_ARROW_NARROW       = 0;
const sal_uInt8 BIFF_OBJ_ARROW_MEDIUM       = 1;
const sal_uInt8 BIFF_OBJ_ARROW_WIDE         = 2;

const sal_uInt8 BIFF_OBJ_LINE_TL            = 0;
const sal_uInt8 BIFF_OBJ_LINE_TR            = 1;
const sal_uInt8 BIFF_OBJ_LINE_BR            = 2;
const sal_uInt8 BIFF_OBJ_LINE_BL            = 3;

const sal_uInt8 BIFF_OBJ_ARC_TR             = 0;
const sal_uInt8 BIFF_OBJ_ARC_TL             = 1;
const sal_uInt8 BIFF_OBJ_ARC_BL             = 2;
const sal_uInt8 BIFF_OBJ_ARC_BR             = 3;

const sal_uInt16 BIFF_OBJ_POLY_CLOSED       = 0x0100;

// fill formatting ------------------------------------------------------------

const sal_uInt8 BIFF_OBJ_FILL_AUTOCOLOR     = 65;

const sal_uInt8 BIFF_OBJ_PATT_NONE          = 0;
const sal_uInt8 BIFF_OBJ_PATT_SOLID         = 1;

const sal_uInt8 BIFF_OBJ_FILL_AUTO          = 0x01;

// text formatting ------------------------------------------------------------

const sal_uInt8 BIFF_OBJ_HOR_LEFT           = 1;
const sal_uInt8 BIFF_OBJ_HOR_CENTER         = 2;
const sal_uInt8 BIFF_OBJ_HOR_RIGHT          = 3;
const sal_uInt8 BIFF_OBJ_HOR_JUSTIFY        = 4;

const sal_uInt8 BIFF_OBJ_VER_TOP            = 1;
const sal_uInt8 BIFF_OBJ_VER_CENTER         = 2;
const sal_uInt8 BIFF_OBJ_VER_BOTTOM         = 3;
const sal_uInt8 BIFF_OBJ_VER_JUSTIFY        = 4;

const sal_uInt16 BIFF_OBJ_ORIENT_NONE       = 0;
const sal_uInt16 BIFF_OBJ_ORIENT_STACKED    = 1;        /// Stacked top to bottom.
const sal_uInt16 BIFF_OBJ_ORIENT_90CCW      = 2;        /// 90 degr. counterclockwise.
const sal_uInt16 BIFF_OBJ_ORIENT_90CW       = 3;        /// 90 degr. clockwise.

const sal_uInt16 BIFF_OBJ_TEXT_AUTOSIZE     = 0x0080;
const sal_uInt16 BIFF_OBJ_TEXT_LOCKED       = 0x0200;

const sal_Int32 BIFF_OBJ_TEXT_MARGIN        = 20000;    /// Automatic text margin (EMUs).

// BIFF8 OBJ sub records ------------------------------------------------------

const sal_uInt16 BIFF_OBJCMO_PRINTABLE      = 0x0010;   /// Object printable.
const sal_uInt16 BIFF_OBJCMO_AUTOLINE       = 0x2000;   /// Automatic line formatting.
const sal_uInt16 BIFF_OBJCMO_AUTOFILL       = 0x4000;   /// Automatic fill formatting.

// ----------------------------------------------------------------------------

} // namespace

// ============================================================================
// Model structures for BIFF OBJ record data
// ============================================================================

BiffObjLineModel::BiffObjLineModel() :
    mnColorIdx( BIFF_OBJ_LINE_AUTOCOLOR ),
    mnStyle( BIFF_OBJ_LINE_SOLID ),
    mnWidth( BIFF_OBJ_LINE_HAIR ),
    mbAuto( true )
{
}

BiffInputStream& operator>>( BiffInputStream& rStrm, BiffObjLineModel& rModel )
{
    sal_uInt8 nFlags;
    rStrm >> rModel.mnColorIdx >> rModel.mnStyle >> rModel.mnWidth >> nFlags;
    rModel.mbAuto = getFlag( nFlags, BIFF_OBJ_LINE_AUTO );
    return rStrm;
}

// ============================================================================

BiffObjFillModel::BiffObjFillModel() :
    mnBackColorIdx( BIFF_OBJ_LINE_AUTOCOLOR ),
    mnPattColorIdx( BIFF_OBJ_FILL_AUTOCOLOR ),
    mnPattern( BIFF_OBJ_PATT_SOLID ),
    mbAuto( true )
{
}

BiffInputStream& operator>>( BiffInputStream& rStrm, BiffObjFillModel& rModel )
{
    sal_uInt8 nFlags;
    rStrm >> rModel.mnBackColorIdx >> rModel.mnPattColorIdx >> rModel.mnPattern >> nFlags;
    rModel.mbAuto = getFlag( nFlags, BIFF_OBJ_FILL_AUTO );
    return rStrm;
}

// ============================================================================
// BIFF drawing objects
// ============================================================================

BiffDrawingObjectContainer::BiffDrawingObjectContainer()
{
}

void BiffDrawingObjectContainer::convertAndInsert( BiffDrawingBase& rDrawing, const Reference< XShapes >& rxShapes, const Rectangle* pParentRect ) const
{
    maObjects.forEachMem( &BiffDrawingObjectBase::convertAndInsert, ::boost::ref( rDrawing ), ::boost::cref( rxShapes ), pParentRect );
}

// ============================================================================

BiffDrawingObjectBase::BiffDrawingObjectBase( const WorksheetHelper& rHelper ) :
    WorksheetHelper( rHelper ),
    maAnchor( rHelper ),
    mnObjId( BIFF_OBJ_INVALID_ID ),
    mnObjType( BIFF_OBJTYPE_UNKNOWN ),
    mbHidden( false ),
    mbVisible( true ),
    mbPrintable( true ),
    mbAreaObj( false ),
    mbSimpleMacro( true ),
    mbProcessShape( true ),
    mbInsertShape( true ),
    mbCustomDff( false )
{
}

BiffDrawingObjectBase::~BiffDrawingObjectBase()
{
}

Reference< XShape > BiffDrawingObjectBase::convertAndInsert( BiffDrawingBase& rDrawing,
        const Reference< XShapes >& rxShapes, const Rectangle* pParentRect ) const
{
    Reference< XShape > xShape;
    if( rxShapes.is() && mbProcessShape && !mbHidden )  // TODO: support for hidden objects?
    {
        // base class 'ShapeAnchor' calculates the shape rectangle in 1/100 mm
        // in BIFF3-BIFF5, all shapes have absolute anchor (also children of group shapes)
        Rectangle aShapeRect = maAnchor.calcAnchorRectHmm( getDrawPageSize() );

        // convert the shape, if the calculated rectangle is not empty
        bool bHasWidth = aShapeRect.Width > 0;
        bool bHasHeight = aShapeRect.Height > 0;
        if( mbAreaObj ? (bHasWidth && bHasHeight) : (bHasWidth || bHasHeight) )
        {
            xShape = implConvertAndInsert( rDrawing, rxShapes, aShapeRect );
            /*  Notify the drawing that a new shape has been inserted (but not
                for children of group shapes). For convenience, pass the
                rectangle that contains position and size of the shape. */
            if( !pParentRect && xShape.is() )
                rDrawing.notifyShapeInserted( xShape, aShapeRect );
        }
    }
    return xShape;
}

void BiffDrawingObjectBase::convertLineProperties( ShapePropertyMap& rPropMap, const BiffObjLineModel& rLineModel, sal_uInt16 nArrows ) const
{
    if( rLineModel.mbAuto )
    {
        BiffObjLineModel aAutoModel;
        aAutoModel.mbAuto = false;
        convertLineProperties( rPropMap, aAutoModel, nArrows );
        return;
    }

    /*  Convert line formatting to DrawingML line formatting and let the
        DrawingML code do the hard work. */
    LineProperties aLineProps;

    if( rLineModel.mnStyle == BIFF_OBJ_LINE_NONE )
    {
        aLineProps.maLineFill.moFillType = XML_noFill;
    }
    else
    {
        aLineProps.maLineFill.moFillType = XML_solidFill;
        aLineProps.maLineFill.maFillColor.setPaletteClr( rLineModel.mnColorIdx );
        aLineProps.moLineCompound = XML_sng;
        aLineProps.moLineCap = XML_flat;
        aLineProps.moLineJoint = XML_round;

        // line width: use 0.35 mm per BIFF line width step
        sal_Int32 nLineWidth = 0;
        switch( rLineModel.mnWidth )
        {
            default:
            case BIFF_OBJ_LINE_HAIR:    nLineWidth = 0;     break;
            case BIFF_OBJ_LINE_THIN:    nLineWidth = 20;    break;
            case BIFF_OBJ_LINE_MEDIUM:  nLineWidth = 40;    break;
            case BIFF_OBJ_LINE_THICK:   nLineWidth = 60;    break;
        }
        aLineProps.moLineWidth = getLimitedValue< sal_Int32, sal_Int64 >( convertHmmToEmu( nLineWidth ), 0, SAL_MAX_INT32 );

        // dash style and transparency
        switch( rLineModel.mnStyle )
        {
            default:
            case BIFF_OBJ_LINE_SOLID:
                aLineProps.moPresetDash = XML_solid;
            break;
            case BIFF_OBJ_LINE_DASH:
                aLineProps.moPresetDash = XML_lgDash;
            break;
            case BIFF_OBJ_LINE_DOT:
                aLineProps.moPresetDash = XML_dot;
            break;
            case BIFF_OBJ_LINE_DASHDOT:
                aLineProps.moPresetDash = XML_lgDashDot;
            break;
            case BIFF_OBJ_LINE_DASHDOTDOT:
                aLineProps.moPresetDash = XML_lgDashDotDot;
            break;
            case BIFF_OBJ_LINE_MEDTRANS:
                aLineProps.moPresetDash = XML_solid;
                aLineProps.maLineFill.maFillColor.addTransformation( XML_alpha, 50 * PER_PERCENT );
            break;
            case BIFF_OBJ_LINE_DARKTRANS:
                aLineProps.moPresetDash = XML_solid;
                aLineProps.maLineFill.maFillColor.addTransformation( XML_alpha, 75 * PER_PERCENT );
            break;
            case BIFF_OBJ_LINE_LIGHTTRANS:
                aLineProps.moPresetDash = XML_solid;
                aLineProps.maLineFill.maFillColor.addTransformation( XML_alpha, 25 * PER_PERCENT );
            break;
        }

        // line ends
        bool bLineStart = false;
        bool bLineEnd = false;
        bool bFilled = false;
        switch( extractValue< sal_uInt8 >( nArrows, 0, 4 ) )
        {
            case BIFF_OBJ_ARROW_OPEN:       bLineStart = false; bLineEnd = true;  bFilled = false;  break;
            case BIFF_OBJ_ARROW_OPENBOTH:   bLineStart = true;  bLineEnd = true;  bFilled = false;  break;
            case BIFF_OBJ_ARROW_FILLED:     bLineStart = false; bLineEnd = true;  bFilled = true;   break;
            case BIFF_OBJ_ARROW_FILLEDBOTH: bLineStart = true;  bLineEnd = true;  bFilled = true;   break;
        }
        if( bLineStart || bLineEnd )
        {
            // arrow type (open or closed)
            sal_Int32 nArrowType = bFilled ? XML_triangle : XML_arrow;
            aLineProps.maStartArrow.moArrowType = bLineStart ? nArrowType : XML_none;
            aLineProps.maEndArrow.moArrowType   = bLineEnd   ? nArrowType : XML_none;

            // arrow width
            sal_Int32 nArrowWidth = XML_med;
            switch( extractValue< sal_uInt8 >( nArrows, 4, 4 ) )
            {
                case BIFF_OBJ_ARROW_NARROW: nArrowWidth = XML_sm;   break;
                case BIFF_OBJ_ARROW_MEDIUM: nArrowWidth = XML_med;  break;
                case BIFF_OBJ_ARROW_WIDE:   nArrowWidth = XML_lg;   break;
            }
            aLineProps.maStartArrow.moArrowWidth = aLineProps.maEndArrow.moArrowWidth = nArrowWidth;

            // arrow length
            sal_Int32 nArrowLength = XML_med;
            switch( extractValue< sal_uInt8 >( nArrows, 8, 4 ) )
            {
                case BIFF_OBJ_ARROW_NARROW: nArrowLength = XML_sm;  break;
                case BIFF_OBJ_ARROW_MEDIUM: nArrowLength = XML_med; break;
                case BIFF_OBJ_ARROW_WIDE:   nArrowLength = XML_lg;  break;
            }
            aLineProps.maStartArrow.moArrowLength = aLineProps.maEndArrow.moArrowLength = nArrowLength;
        }
    }

    aLineProps.pushToPropMap( rPropMap, getBaseFilter().getGraphicHelper() );
}

void BiffDrawingObjectBase::convertFillProperties( ShapePropertyMap& rPropMap, const BiffObjFillModel& rFillModel ) const
{
    if( rFillModel.mbAuto )
    {
        BiffObjFillModel aAutoModel;
        aAutoModel.mbAuto = false;
        convertFillProperties( rPropMap, aAutoModel );
        return;
    }

    /*  Convert fill formatting to DrawingML fill formatting and let the
        DrawingML code do the hard work. */
    FillProperties aFillProps;

    if( rFillModel.mnPattern == BIFF_OBJ_PATT_NONE )
    {
        aFillProps.moFillType = XML_noFill;
    }
    else
    {
        const sal_Int32 spnPatternPresets[] = {
            XML_TOKEN_INVALID, XML_TOKEN_INVALID, XML_pct50, XML_pct50, XML_pct25,
            XML_dkHorz, XML_dkVert, XML_dkDnDiag, XML_dkUpDiag, XML_smCheck, XML_trellis,
            XML_ltHorz, XML_ltVert, XML_ltDnDiag, XML_ltUpDiag, XML_smGrid, XML_diagCross,
            XML_pct20, XML_pct10 };
        sal_Int32 nPatternPreset = STATIC_ARRAY_SELECT( spnPatternPresets, rFillModel.mnPattern, XML_TOKEN_INVALID );
        if( nPatternPreset == XML_TOKEN_INVALID )
        {
            aFillProps.moFillType = XML_solidFill;
            aFillProps.maFillColor.setPaletteClr( rFillModel.mnPattColorIdx );
        }
        else
        {
            aFillProps.moFillType = XML_pattFill;
            aFillProps.maPatternProps.maPattFgColor.setPaletteClr( rFillModel.mnPattColorIdx );
            aFillProps.maPatternProps.maPattBgColor.setPaletteClr( rFillModel.mnBackColorIdx );
            aFillProps.maPatternProps.moPattPreset = nPatternPreset;
        }
#if 0
        static const sal_uInt8 sppnPatterns[][ 8 ] =
        {
            { 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55 },
            { 0x77, 0xDD, 0x77, 0xDD, 0x77, 0xDD, 0x77, 0xDD },
            { 0x88, 0x22, 0x88, 0x22, 0x88, 0x22, 0x88, 0x22 },
            { 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00 },
            { 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC },
            { 0x33, 0x66, 0xCC, 0x99, 0x33, 0x66, 0xCC, 0x99 },
            { 0xCC, 0x66, 0x33, 0x99, 0xCC, 0x66, 0x33, 0x99 },
            { 0xCC, 0xCC, 0x33, 0x33, 0xCC, 0xCC, 0x33, 0x33 },
            { 0xCC, 0xFF, 0x33, 0xFF, 0xCC, 0xFF, 0x33, 0xFF },
            { 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00 },
            { 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88 },
            { 0x11, 0x22, 0x44, 0x88, 0x11, 0x22, 0x44, 0x88 },
            { 0x88, 0x44, 0x22, 0x11, 0x88, 0x44, 0x22, 0x11 },
            { 0xFF, 0x11, 0x11, 0x11, 0xFF, 0x11, 0x11, 0x11 },
            { 0xAA, 0x44, 0xAA, 0x11, 0xAA, 0x44, 0xAA, 0x11 },
            { 0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00 },
            { 0x80, 0x00, 0x08, 0x00, 0x80, 0x00, 0x08, 0x00 }
        };
        const sal_uInt8* const pnPattern = sppnPatterns[ ::std::min< size_t >( rFillData.mnPattern - 2, STATIC_ARRAY_SIZE( sppnPatterns ) ) ];
        // create 2-colored 8x8 DIB
        SvMemoryStream aMemStrm;
//      { 0x0C, 0x00, 0x00, 0x00, 0x08, 0x00, 0x08, 0x00, 0x01, 0x00, 0x01, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00 }
        aMemStrm << sal_uInt32( 12 ) << sal_Int16( 8 ) << sal_Int16( 8 ) << sal_uInt16( 1 ) << sal_uInt16( 1 );
        aMemStrm << sal_uInt8( 0xFF ) << sal_uInt8( 0xFF ) << sal_uInt8( 0xFF );
        aMemStrm << sal_uInt8( 0x00 ) << sal_uInt8( 0x00 ) << sal_uInt8( 0x00 );
        for( size_t nIdx = 0; nIdx < 8; ++nIdx )
            aMemStrm << sal_uInt32( pnPattern[ nIdx ] ); // 32-bit little-endian
        aMemStrm.Seek( STREAM_SEEK_TO_BEGIN );
        Bitmap aBitmap;
        aBitmap.Read( aMemStrm, FALSE );
        XOBitmap aXOBitmap( aBitmap );
        aXOBitmap.Bitmap2Array();
        aXOBitmap.SetBitmapType( XBITMAP_8X8 );
        if( aXOBitmap.GetBackgroundColor().GetColor() == COL_BLACK )
            ::std::swap( aPattColor, aBackColor );
        aXOBitmap.SetPixelColor( aPattColor );
        aXOBitmap.SetBackgroundColor( aBackColor );
        rSdrObj.SetMergedItem( XFillStyleItem( XFILL_BITMAP ) );
        rSdrObj.SetMergedItem( XFillBitmapItem( EMPTY_STRING, aXOBitmap ) );
#endif
    }

    aFillProps.pushToPropMap( rPropMap, getBaseFilter().getGraphicHelper() );
}

// ============================================================================
// BIFF drawing page
// ============================================================================

BiffDrawingBase::BiffDrawingBase( const WorksheetHelper& rHelper, const Reference< XDrawPage >& rxDrawPage ) :
    WorksheetHelper( rHelper ),
    mxDrawPage( rxDrawPage )
{
}

void BiffDrawingBase::finalizeImport()
{
    Reference< XShapes > xShapes( mxDrawPage, UNO_QUERY );
    OSL_ENSURE( xShapes.is(), "BiffDrawingBase::finalizeImport - no shapes container" );
    if( !xShapes.is() )
        return;

    // process list of objects to be skipped
    for( BiffObjIdVector::const_iterator aIt = maSkipObjs.begin(), aEnd = maSkipObjs.end(); aIt != aEnd; ++aIt )
        if( BiffDrawingObjectBase* pDrawingObj = maObjMapId.get( *aIt ).get() )
            pDrawingObj->setProcessShape( false );

    // process drawing objects without DFF data
    maRawObjs.convertAndInsert( *this, xShapes );
}

// ============================================================================

BiffSheetDrawing::BiffSheetDrawing( const WorksheetHelper& rHelper ) :
    BiffDrawingBase( rHelper, rHelper.getDrawPage() )
{
}

void BiffSheetDrawing::notifyShapeInserted( const Reference< XShape >& /*rxShape*/, const Rectangle& rShapeRect )
{
    // collect all shape positions in the WorksheetHelper base class
    extendShapeBoundingBox( rShapeRect );
}

// ============================================================================

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
