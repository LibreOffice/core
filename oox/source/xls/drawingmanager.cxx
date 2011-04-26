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

#include "oox/xls/drawingmanager.hxx"

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
#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/unitconverter.hxx"

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

inline BiffInputStream& operator>>( BiffInputStream& rStrm, ShapeAnchor& rAnchor )
{
    rAnchor.importBiffAnchor( rStrm );
    return rStrm;
}

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

bool BiffObjLineModel::isVisible() const
{
    return mbAuto || (mnStyle != BIFF_OBJ_LINE_NONE);
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

bool BiffObjFillModel::isFilled() const
{
    return mbAuto || (mnPattern != BIFF_OBJ_PATT_NONE);
}

BiffInputStream& operator>>( BiffInputStream& rStrm, BiffObjFillModel& rModel )
{
    sal_uInt8 nFlags;
    rStrm >> rModel.mnBackColorIdx >> rModel.mnPattColorIdx >> rModel.mnPattern >> nFlags;
    rModel.mbAuto = getFlag( nFlags, BIFF_OBJ_FILL_AUTO );
    return rStrm;
}

// ============================================================================

BiffObjTextModel::BiffObjTextModel() :
    mnTextLen( 0 ),
    mnFormatSize( 0 ),
    mnLinkSize( 0 ),
    mnDefFontId( 0 ),
    mnFlags( 0 ),
    mnOrientation( BIFF_OBJ_ORIENT_NONE ),
    mnButtonFlags( 0 ),
    mnShortcut( 0 ),
    mnShortcutEA( 0 )
{
}

void BiffObjTextModel::readObj3( BiffInputStream& rStrm )
{
    rStrm >> mnTextLen;
    rStrm.skip( 2 );
    rStrm >> mnFormatSize >> mnDefFontId;
    rStrm.skip( 2 );
    rStrm >> mnFlags >> mnOrientation;
    rStrm.skip( 8 );
}

void BiffObjTextModel::readObj5( BiffInputStream& rStrm )
{
    rStrm >> mnTextLen;
    rStrm.skip( 2 );
    rStrm >> mnFormatSize >> mnDefFontId;
    rStrm.skip( 2 );
    rStrm >> mnFlags >> mnOrientation;
    rStrm.skip( 2 );
    rStrm >> mnLinkSize;
    rStrm.skip( 2 );
    rStrm >> mnButtonFlags >> mnShortcut >> mnShortcutEA;
}

void BiffObjTextModel::readTxo8( BiffInputStream& rStrm )
{
    rStrm >> mnFlags >> mnOrientation >> mnButtonFlags >> mnShortcut >> mnShortcutEA >> mnTextLen >> mnFormatSize;
}

sal_uInt8 BiffObjTextModel::getHorAlign() const
{
    return extractValue< sal_uInt8 >( mnFlags, 1, 3 );
}

sal_uInt8 BiffObjTextModel::getVerAlign() const
{
    return extractValue< sal_uInt8 >( mnFlags, 4, 3 );
}

// ============================================================================
// BIFF drawing objects
// ============================================================================

BiffDrawingObjectContainer::BiffDrawingObjectContainer()
{
}

void BiffDrawingObjectContainer::append( const BiffDrawingObjectRef& rxDrawingObj )
{
    maObjects.push_back( rxDrawingObj );
}

void BiffDrawingObjectContainer::insertGrouped( const BiffDrawingObjectRef& rxDrawingObj )
{
    if( !maObjects.empty() )
        if( BiffGroupObject* pGroupObj = dynamic_cast< BiffGroupObject* >( maObjects.back().get() ) )
            if( pGroupObj->tryInsert( rxDrawingObj ) )
                return;
    maObjects.push_back( rxDrawingObj );
}

void BiffDrawingObjectContainer::convertAndInsert( BiffDrawingBase& rDrawing, const Reference< XShapes >& rxShapes, const Rectangle* pParentRect ) const
{
    maObjects.forEachMem( &BiffDrawingObjectBase::convertAndInsert, ::boost::ref( rDrawing ), ::boost::cref( rxShapes ), pParentRect );
}

// ============================================================================

BiffDrawingObjectBase::BiffDrawingObjectBase( const WorksheetHelper& rHelper ) :
    WorksheetHelper( rHelper ),
    maAnchor( rHelper ),
    mnDffShapeId( 0 ),
    mnDffFlags( 0 ),
    mnObjId( BIFF_OBJ_INVALID_ID ),
    mnObjType( BIFF_OBJTYPE_UNKNOWN ),
    mbHasAnchor( false ),
    mbHidden( false ),
    mbVisible( true ),
    mbPrintable( true ),
    mbAreaObj( false ),
    mbAutoMargin( true ),
    mbSimpleMacro( true ),
    mbProcessShape( true ),
    mbInsertShape( true ),
    mbCustomDff( false )
{
}

BiffDrawingObjectBase::~BiffDrawingObjectBase()
{
}

/*static*/ BiffDrawingObjectRef BiffDrawingObjectBase::importObjBiff3( const WorksheetHelper& rHelper, BiffInputStream& rStrm )
{
    BiffDrawingObjectRef xDrawingObj;

    if( rStrm.getRemaining() >= 30 )
    {
        sal_uInt16 nObjType;
        rStrm.skip( 4 );
        rStrm >> nObjType;
        switch( nObjType )
        {
            case BIFF_OBJTYPE_GROUP:        xDrawingObj.reset( new BiffGroupObject( rHelper ) );    break;
            case BIFF_OBJTYPE_LINE:         xDrawingObj.reset( new BiffLineObject( rHelper ) );     break;
            case BIFF_OBJTYPE_RECTANGLE:    xDrawingObj.reset( new BiffRectObject( rHelper ) );     break;
            case BIFF_OBJTYPE_OVAL:         xDrawingObj.reset( new BiffOvalObject( rHelper ) );     break;
            case BIFF_OBJTYPE_ARC:          xDrawingObj.reset( new BiffArcObject( rHelper ) );      break;
#if 0
            case BIFF_OBJTYPE_CHART:        xDrawingObj.reset( new XclImpChartObj( rHelper ) );     break;
            case BIFF_OBJTYPE_TEXT:         xDrawingObj.reset( new XclImpTextObj( rHelper ) );      break;
            case BIFF_OBJTYPE_BUTTON:       xDrawingObj.reset( new XclImpButtonObj( rHelper ) );    break;
            case BIFF_OBJTYPE_PICTURE:      xDrawingObj.reset( new XclImpPictureObj( rHelper ) );   break;
#endif
            default:
#if 0
                OSL_ENSURE( false, "BiffDrawingObjectBase::importObjBiff3 - unknown object type" );
#endif
                xDrawingObj.reset( new BiffPlaceholderObject( rHelper ) );
        }
    }

    xDrawingObj->importObjBiff3( rStrm );
    return xDrawingObj;
}

/*static*/ BiffDrawingObjectRef BiffDrawingObjectBase::importObjBiff4( const WorksheetHelper& rHelper, BiffInputStream& rStrm )
{
    BiffDrawingObjectRef xDrawingObj;

    if( rStrm.getRemaining() >= 30 )
    {
        sal_uInt16 nObjType;
        rStrm.skip( 4 );
        rStrm >> nObjType;
        switch( nObjType )
        {
            case BIFF_OBJTYPE_GROUP:        xDrawingObj.reset( new BiffGroupObject( rHelper ) );    break;
            case BIFF_OBJTYPE_LINE:         xDrawingObj.reset( new BiffLineObject( rHelper ) );     break;
            case BIFF_OBJTYPE_RECTANGLE:    xDrawingObj.reset( new BiffRectObject( rHelper ) );     break;
            case BIFF_OBJTYPE_OVAL:         xDrawingObj.reset( new BiffOvalObject( rHelper ) );     break;
            case BIFF_OBJTYPE_ARC:          xDrawingObj.reset( new BiffArcObject( rHelper ) );      break;
            case BIFF_OBJTYPE_POLYGON:      xDrawingObj.reset( new BiffPolygonObject( rHelper ) );  break;
#if 0
            case BIFF_OBJTYPE_CHART:        xDrawingObj.reset( new XclImpChartObj( rHelper ) );     break;
            case BIFF_OBJTYPE_TEXT:         xDrawingObj.reset( new XclImpTextObj( rHelper ) );      break;
            case BIFF_OBJTYPE_BUTTON:       xDrawingObj.reset( new XclImpButtonObj( rHelper ) );    break;
            case BIFF_OBJTYPE_PICTURE:      xDrawingObj.reset( new XclImpPictureObj( rHelper ) );   break;
#endif
            default:
#if 0
                OSL_ENSURE( false, "BiffDrawingObjectBase::importObjBiff4 - unknown object type" );
#endif
                xDrawingObj.reset( new BiffPlaceholderObject( rHelper ) );
        }
    }

    xDrawingObj->importObjBiff4( rStrm );
    return xDrawingObj;
}

/*static*/ BiffDrawingObjectRef BiffDrawingObjectBase::importObjBiff5( const WorksheetHelper& rHelper, BiffInputStream& rStrm )
{
    BiffDrawingObjectRef xDrawingObj;

    if( rStrm.getRemaining() >= 34 )
    {
        sal_uInt16 nObjType;
        rStrm.skip( 4 );
        rStrm >> nObjType;
        switch( nObjType )
        {
            case BIFF_OBJTYPE_GROUP:        xDrawingObj.reset( new BiffGroupObject( rHelper ) );        break;
            case BIFF_OBJTYPE_LINE:         xDrawingObj.reset( new BiffLineObject( rHelper ) );         break;
            case BIFF_OBJTYPE_RECTANGLE:    xDrawingObj.reset( new BiffRectObject( rHelper ) );         break;
            case BIFF_OBJTYPE_OVAL:         xDrawingObj.reset( new BiffOvalObject( rHelper ) );         break;
            case BIFF_OBJTYPE_ARC:          xDrawingObj.reset( new BiffArcObject( rHelper ) );           break;
            case BIFF_OBJTYPE_POLYGON:      xDrawingObj.reset( new BiffPolygonObject( rHelper ) );      break;
#if 0
            case BIFF_OBJTYPE_CHART:        xDrawingObj.reset( new XclImpChartObj( rHelper ) );         break;
            case BIFF_OBJTYPE_TEXT:         xDrawingObj.reset( new XclImpTextObj( rHelper ) );          break;
            case BIFF_OBJTYPE_BUTTON:       xDrawingObj.reset( new XclImpButtonObj( rHelper ) );        break;
            case BIFF_OBJTYPE_PICTURE:      xDrawingObj.reset( new XclImpPictureObj( rHelper ) );       break;
            case BIFF_OBJTYPE_CHECKBOX:     xDrawingObj.reset( new XclImpCheckBoxObj( rHelper ) );      break;
            case BIFF_OBJTYPE_OPTIONBUTTON: xDrawingObj.reset( new XclImpOptionButtonObj( rHelper ) );  break;
            case BIFF_OBJTYPE_EDIT:         xDrawingObj.reset( new XclImpEditObj( rHelper ) );          break;
            case BIFF_OBJTYPE_LABEL:        xDrawingObj.reset( new XclImpLabelObj( rHelper ) );         break;
            case BIFF_OBJTYPE_DIALOG:       xDrawingObj.reset( new XclImpDialogObj( rHelper ) );        break;
            case BIFF_OBJTYPE_SPIN:         xDrawingObj.reset( new XclImpSpinButtonObj( rHelper ) );    break;
            case BIFF_OBJTYPE_SCROLLBAR:    xDrawingObj.reset( new XclImpScrollBarObj( rHelper ) );     break;
            case BIFF_OBJTYPE_LISTBOX:      xDrawingObj.reset( new XclImpListBoxObj( rHelper ) );       break;
            case BIFF_OBJTYPE_GROUPBOX:     xDrawingObj.reset( new XclImpGroupBoxObj( rHelper ) );      break;
            case BIFF_OBJTYPE_DROPDOWN:     xDrawingObj.reset( new XclImpDropDownObj( rHelper ) );      break;
#endif
            default:
#if 0
                OSL_ENSURE( false, "BiffDrawingObjectBase::importObjBiff5 - unknown object type" );
#endif
                xDrawingObj.reset( new BiffPlaceholderObject( rHelper ) );
        }
    }

    xDrawingObj->importObjBiff5( rStrm );
    return xDrawingObj;
}

/*static*/ BiffDrawingObjectRef BiffDrawingObjectBase::importObjBiff8( const WorksheetHelper& rHelper, BiffInputStream& rStrm )
{
    BiffDrawingObjectRef xDrawingObj;

    if( rStrm.getRemaining() >= 10 )
    {
        sal_uInt16 nSubRecId, nSubRecSize, nObjType;
        rStrm >> nSubRecId >> nSubRecSize >> nObjType;
        OSL_ENSURE( nSubRecId == BIFF_ID_OBJCMO, "BiffDrawingObjectBase::importObjBiff8 - OBJCMO subrecord expected" );
        if( (nSubRecId == BIFF_ID_OBJCMO) && (nSubRecSize >= 6) )
        {
            switch( nObjType )
            {
#if 0
                // in BIFF8, all simple objects support text
                case BIFF_OBJTYPE_LINE:
                case BIFF_OBJTYPE_ARC:
                    xDrawingObj.reset( new XclImpTextObj( rHelper ) );
                    // lines and arcs may be 2-dimensional
                    xDrawingObj->setAreaObj( false );
                break;

                // in BIFF8, all simple objects support text
                case BIFF_OBJTYPE_RECTANGLE:
                case BIFF_OBJTYPE_OVAL:
                case BIFF_OBJTYPE_POLYGON:
                case BIFF_OBJTYPE_DRAWING:
                case BIFF_OBJTYPE_TEXT:
                    xDrawingObj.reset( new XclImpTextObj( rHelper ) );
                break;
#endif

                case BIFF_OBJTYPE_GROUP:        xDrawingObj.reset( new BiffGroupObject( rHelper ) );        break;
#if 0
                case BIFF_OBJTYPE_CHART:        xDrawingObj.reset( new XclImpChartObj( rHelper ) );         break;
                case BIFF_OBJTYPE_BUTTON:       xDrawingObj.reset( new XclImpButtonObj( rHelper ) );        break;
                case BIFF_OBJTYPE_PICTURE:      xDrawingObj.reset( new XclImpPictureObj( rHelper ) );       break;
                case BIFF_OBJTYPE_CHECKBOX:     xDrawingObj.reset( new XclImpCheckBoxObj( rHelper ) );      break;
                case BIFF_OBJTYPE_OPTIONBUTTON: xDrawingObj.reset( new XclImpOptionButtonObj( rHelper ) );  break;
                case BIFF_OBJTYPE_EDIT:         xDrawingObj.reset( new XclImpEditObj( rHelper ) );          break;
                case BIFF_OBJTYPE_LABEL:        xDrawingObj.reset( new XclImpLabelObj( rHelper ) );         break;
                case BIFF_OBJTYPE_DIALOG:       xDrawingObj.reset( new XclImpDialogObj( rHelper ) );        break;
                case BIFF_OBJTYPE_SPIN:         xDrawingObj.reset( new XclImpSpinButtonObj( rHelper ) );    break;
                case BIFF_OBJTYPE_SCROLLBAR:    xDrawingObj.reset( new XclImpScrollBarObj( rHelper ) );     break;
                case BIFF_OBJTYPE_LISTBOX:      xDrawingObj.reset( new XclImpListBoxObj( rHelper ) );       break;
                case BIFF_OBJTYPE_GROUPBOX:     xDrawingObj.reset( new XclImpGroupBoxObj( rHelper ) );      break;
                case BIFF_OBJTYPE_DROPDOWN:     xDrawingObj.reset( new XclImpDropDownObj( rHelper ) );      break;
                case BIFF_OBJTYPE_NOTE:         xDrawingObj.reset( new XclImpNoteObj( rHelper ) );          break;
#endif

                default:
#if 0
                    OSL_ENSURE( false, "BiffDrawingObjectBase::importObjBiff8 - unknown object type" );
#endif
                    xDrawingObj.reset( new BiffPlaceholderObject( rHelper ) );
            }
        }
    }

    xDrawingObj->importObjBiff8( rStrm );
    return xDrawingObj;
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

// protected ------------------------------------------------------------------

void BiffDrawingObjectBase::readNameBiff5( BiffInputStream& rStrm, sal_uInt16 nNameLen )
{
    maObjName = OUString();
    if( nNameLen > 0 )
    {
        // name length field is repeated before the name
        maObjName = rStrm.readByteStringUC( false, getTextEncoding() );
        // skip padding byte for word boundaries
        rStrm.alignToBlock( 2 );
    }
}

void BiffDrawingObjectBase::readMacroBiff3( BiffInputStream& rStrm, sal_uInt16 nMacroSize )
{
    maMacroName = OUString();
    rStrm.skip( nMacroSize );
    // skip padding byte for word boundaries, not contained in nMacroSize
    rStrm.alignToBlock( 2 );
}

void BiffDrawingObjectBase::readMacroBiff4( BiffInputStream& rStrm, sal_uInt16 nMacroSize )
{
    maMacroName = OUString();
    rStrm.skip( nMacroSize );
}

void BiffDrawingObjectBase::readMacroBiff5( BiffInputStream& rStrm, sal_uInt16 nMacroSize )
{
    maMacroName = OUString();
    rStrm.skip( nMacroSize );
}

void BiffDrawingObjectBase::readMacroBiff8( BiffInputStream& rStrm )
{
    maMacroName = OUString();
    if( rStrm.getRemaining() > 6 )
    {
        // macro is stored in a tNameXR token containing a link to a defined name
        sal_uInt16 nFmlaSize;
        rStrm >> nFmlaSize;
        rStrm.skip( 4 );
        OSL_ENSURE( nFmlaSize == 7, "BiffDrawingObjectBase::readMacroBiff8 - unexpected formula size" );
        if( nFmlaSize == 7 )
        {
            sal_uInt8 nTokenId;
            sal_uInt16 nExtLinkId, nExtNameId;
            rStrm >> nTokenId >> nExtLinkId >> nExtNameId;
#if 0
            OSL_ENSURE( nTokenId == XclTokenArrayHelper::GetTokenId( EXC_TOKID_NAMEX, EXC_TOKCLASS_REF ),
                "BiffDrawingObjectBase::readMacroBiff8 - tNameXR token expected" );
            if( nTokenId == XclTokenArrayHelper::GetTokenId( EXC_TOKID_NAMEX, EXC_TOKCLASS_REF ) )
                maMacroName = GetLinkManager().GetMacroName( nExtLinkId, nExtNameId );
#endif
        }
    }
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

void BiffDrawingObjectBase::convertFrameProperties( ShapePropertyMap& /*rPropMap*/, sal_uInt16 /*nFrameFlags*/ ) const
{
}

void BiffDrawingObjectBase::implReadObjBiff3( BiffInputStream& /*rStrm*/, sal_uInt16 /*nMacroSize*/ )
{
}

void BiffDrawingObjectBase::implReadObjBiff4( BiffInputStream& /*rStrm*/, sal_uInt16 /*nMacroSize*/ )
{
}

void BiffDrawingObjectBase::implReadObjBiff5( BiffInputStream& /*rStrm*/, sal_uInt16 /*nNameLen*/, sal_uInt16 /*nMacroSize*/ )
{
}

void BiffDrawingObjectBase::implReadObjBiff8SubRec( BiffInputStream& /*rStrm*/, sal_uInt16 /*nSubRecId*/, sal_uInt16 /*nSubRecSize*/ )
{
}

// private --------------------------------------------------------------------

void BiffDrawingObjectBase::importObjBiff3( BiffInputStream& rStrm )
{
    // back to offset 4 (ignore object count field)
    rStrm.seek( 4 );

    sal_uInt16 nObjFlags, nMacroSize;
    rStrm >> mnObjType >> mnObjId >> nObjFlags >> maAnchor >> nMacroSize;
    rStrm.skip( 2 );

    mbHasAnchor = true;
    mbHidden = getFlag( nObjFlags, BIFF_OBJ_HIDDEN );
    mbVisible = getFlag( nObjFlags, BIFF_OBJ_VISIBLE );
    implReadObjBiff3( rStrm, nMacroSize );
}

void BiffDrawingObjectBase::importObjBiff4( BiffInputStream& rStrm )
{
    // back to offset 4 (ignore object count field)
    rStrm.seek( 4 );

    sal_uInt16 nObjFlags, nMacroSize;
    rStrm >> mnObjType >> mnObjId >> nObjFlags >> maAnchor >> nMacroSize;
    rStrm.skip( 2 );

    mbHasAnchor = true;
    mbHidden = getFlag( nObjFlags, BIFF_OBJ_HIDDEN );
    mbVisible = getFlag( nObjFlags, BIFF_OBJ_VISIBLE );
    mbPrintable = getFlag( nObjFlags, BIFF_OBJ_PRINTABLE );
    implReadObjBiff4( rStrm, nMacroSize );
}

void BiffDrawingObjectBase::importObjBiff5( BiffInputStream& rStrm )
{
    // back to offset 4 (ignore object count field)
    rStrm.seek( 4 );

    sal_uInt16 nObjFlags, nMacroSize, nNameLen;
    rStrm >> mnObjType >> mnObjId >> nObjFlags >> maAnchor >> nMacroSize;
    rStrm.skip( 2 );
    rStrm >> nNameLen;
    rStrm.skip( 2 );

    mbHasAnchor = true;
    mbHidden = getFlag( nObjFlags, BIFF_OBJ_HIDDEN );
    mbVisible = getFlag( nObjFlags, BIFF_OBJ_VISIBLE );
    mbPrintable = getFlag( nObjFlags, BIFF_OBJ_PRINTABLE );
    implReadObjBiff5( rStrm, nNameLen, nMacroSize );
}

void BiffDrawingObjectBase::importObjBiff8( BiffInputStream& rStrm )
{
    // back to beginning
    rStrm.seekToStart();

    bool bLoop = true;
    while( bLoop && (rStrm.getRemaining() >= 4) )
    {
        sal_uInt16 nSubRecId, nSubRecSize;
        rStrm >> nSubRecId >> nSubRecSize;
        sal_Int64 nStrmPos = rStrm.tell();
        // sometimes the last subrecord has an invalid length (OBJLBSDATA) -> min()
        nSubRecSize = static_cast< sal_uInt16 >( ::std::min< sal_Int64 >( nSubRecSize, rStrm.getRemaining() ) );

        switch( nSubRecId )
        {
            case BIFF_ID_OBJCMO:
                OSL_ENSURE( rStrm.tell() == 4, "BiffDrawingObjectBase::importObjBiff8 - unexpected OBJCMO subrecord" );
                if( (rStrm.tell() == 4) && (nSubRecSize >= 6) )
                {
                    sal_uInt16 nObjFlags;
                    rStrm >> mnObjType >> mnObjId >> nObjFlags;
                    mbPrintable = getFlag( nObjFlags, BIFF_OBJCMO_PRINTABLE );
                }
            break;
            case BIFF_ID_OBJMACRO:
                readMacroBiff8( rStrm );
            break;
            case BIFF_ID_OBJEND:
                bLoop = false;
            break;
            default:
                implReadObjBiff8SubRec( rStrm, nSubRecId, nSubRecSize );
        }

        // seek to end of subrecord
        rStrm.seek( nStrmPos + nSubRecSize );
    }

    /*  Call doReadObj8SubRec() with BIFF_ID_OBJEND for further stream
        processing (e.g. charts), even if the OBJEND subrecord is missing. */
    implReadObjBiff8SubRec( rStrm, BIFF_ID_OBJEND, 0 );

    /*  Pictures that Excel reads from BIFF5 and writes to BIFF8 still have the
        IMGDATA record following the OBJ record (but they use the image data
        stored in DFF). The IMGDATA record may be continued by several CONTINUE
        records. But the last CONTINUE record may be in fact an MSODRAWING
        record that contains the DFF data of the next drawing object! So we
        have to skip just enough CONTINUE records to look at the next
        MSODRAWING/CONTINUE record. */
    if( (rStrm.getNextRecId() == BIFF3_ID_IMGDATA) && rStrm.startNextRecord() )
    {
        rStrm.skip( 4 );
        sal_Int64 nDataSize = rStrm.readuInt32();
        nDataSize -= rStrm.getRemaining();
        // skip following CONTINUE records until IMGDATA ends
        while( (nDataSize > 0) && (rStrm.getNextRecId() == BIFF_ID_CONT) && rStrm.startNextRecord() )
        {
            OSL_ENSURE( nDataSize >= rStrm.getRemaining(), "BiffDrawingObjectBase::importObjBiff8 - CONTINUE too long" );
            nDataSize -= ::std::min( rStrm.getRemaining(), nDataSize );
        }
        OSL_ENSURE( nDataSize == 0, "BiffDrawingObjectBase::importObjBiff8 - missing CONTINUE records" );
        // next record may be MSODRAWING or CONTINUE or anything else
    }
}

// ============================================================================

BiffPlaceholderObject::BiffPlaceholderObject( const WorksheetHelper& rHelper ) :
    BiffDrawingObjectBase( rHelper )
{
    setProcessShape( false );
}

Reference< XShape > BiffPlaceholderObject::implConvertAndInsert( BiffDrawingBase& /*rDrawing*/,
        const Reference< XShapes >& /*rxShapes*/, const Rectangle& /*rShapeRect*/ ) const
{
    return Reference< XShape >();
}

// ============================================================================

BiffGroupObject::BiffGroupObject( const WorksheetHelper& rHelper ) :
    BiffDrawingObjectBase( rHelper ),
    mnFirstUngrouped( BIFF_OBJ_INVALID_ID )
{
}

bool BiffGroupObject::tryInsert( const BiffDrawingObjectRef& rxDrawingObj )
{
    if( rxDrawingObj->getObjId() == mnFirstUngrouped )
        return false;
    // insert into own list or into nested group
    maChildren.insertGrouped( rxDrawingObj );
    return true;
}

void BiffGroupObject::implReadObjBiff3( BiffInputStream& rStrm, sal_uInt16 nMacroSize )
{
    rStrm.skip( 4 );
    rStrm >> mnFirstUngrouped;
    rStrm.skip( 16 );
    readMacroBiff3( rStrm, nMacroSize );
}

void BiffGroupObject::implReadObjBiff4( BiffInputStream& rStrm, sal_uInt16 nMacroSize )
{
    rStrm.skip( 4 );
    rStrm >> mnFirstUngrouped;
    rStrm.skip( 16 );
    readMacroBiff4( rStrm, nMacroSize );
}

void BiffGroupObject::implReadObjBiff5( BiffInputStream& rStrm, sal_uInt16 nNameLen, sal_uInt16 nMacroSize )
{
    rStrm.skip( 4 );
    rStrm >> mnFirstUngrouped;
    rStrm.skip( 16 );
    readNameBiff5( rStrm, nNameLen );
    readMacroBiff5( rStrm, nMacroSize );
}

Reference< XShape > BiffGroupObject::implConvertAndInsert( BiffDrawingBase& rDrawing,
        const Reference< XShapes >& rxShapes, const Rectangle& rShapeRect ) const
{
    Reference< XShape > xGroupShape;
    if( !maChildren.empty() ) try
    {
        xGroupShape = rDrawing.createAndInsertXShape( CREATE_OUSTRING( "com.sun.star.drawing.GroupShape" ), rxShapes, rShapeRect );
        Reference< XShapes > xChildShapes( xGroupShape, UNO_QUERY_THROW );
        maChildren.convertAndInsert( rDrawing, xChildShapes, &rShapeRect );
        // no child shape has been created - delete the group shape
        if( !xChildShapes->hasElements() )
        {
            rxShapes->remove( xGroupShape );
            xGroupShape.clear();
        }
    }
    catch( Exception& )
    {
    }
    return xGroupShape;
}

// ============================================================================

BiffLineObject::BiffLineObject( const WorksheetHelper& rHelper ) :
    BiffDrawingObjectBase( rHelper ),
    mnArrows( 0 ),
    mnStartPoint( BIFF_OBJ_LINE_TL )
{
    setAreaObj( false );
}

void BiffLineObject::implReadObjBiff3( BiffInputStream& rStrm, sal_uInt16 nMacroSize )
{
    rStrm >> maLineModel >> mnArrows >> mnStartPoint;
    rStrm.skip( 1 );
    readMacroBiff3( rStrm, nMacroSize );
}

void BiffLineObject::implReadObjBiff4( BiffInputStream& rStrm, sal_uInt16 nMacroSize )
{
    rStrm >> maLineModel >> mnArrows >> mnStartPoint;
    rStrm.skip( 1 );
    readMacroBiff4( rStrm, nMacroSize );
}

void BiffLineObject::implReadObjBiff5( BiffInputStream& rStrm, sal_uInt16 nNameLen, sal_uInt16 nMacroSize )
{
    rStrm >> maLineModel >> mnArrows >> mnStartPoint;
    rStrm.skip( 1 );
    readNameBiff5( rStrm, nNameLen );
    readMacroBiff5( rStrm, nMacroSize );
}

Reference< XShape > BiffLineObject::implConvertAndInsert( BiffDrawingBase& rDrawing,
        const Reference< XShapes >& rxShapes, const Rectangle& rShapeRect ) const
{
    ShapePropertyMap aPropMap( getBaseFilter().getModelObjectHelper() );
    convertLineProperties( aPropMap, maLineModel, mnArrows );

    // create the line polygon
    PointSequenceSequence aPoints( 1 );
    aPoints[ 0 ].realloc( 2 );
    Point& rBeg = aPoints[ 0 ][ 0 ];
    Point& rEnd = aPoints[ 0 ][ 1 ];
    sal_Int32 nL = rShapeRect.X;
    sal_Int32 nT = rShapeRect.Y;
    sal_Int32 nR = rShapeRect.X + ::std::max< sal_Int32 >( rShapeRect.Width - 1, 0 );
    sal_Int32 nB = rShapeRect.Y + ::std::max< sal_Int32 >( rShapeRect.Height - 1, 0 );
    switch( mnStartPoint )
    {
        default:
        case BIFF_OBJ_LINE_TL: rBeg.X = nL; rBeg.Y = nT; rEnd.X = nR; rEnd.Y = nB; break;
        case BIFF_OBJ_LINE_TR: rBeg.X = nR; rBeg.Y = nT; rEnd.X = nL; rEnd.Y = nB; break;
        case BIFF_OBJ_LINE_BR: rBeg.X = nR; rBeg.Y = nB; rEnd.X = nL; rEnd.Y = nT; break;
        case BIFF_OBJ_LINE_BL: rBeg.X = nL; rBeg.Y = nB; rEnd.X = nR; rEnd.Y = nT; break;
    }
    aPropMap.setProperty( PROP_PolyPolygon, aPoints );
    aPropMap.setProperty( PROP_PolygonKind, PolygonKind_LINE );

    // create the shape
    Reference< XShape > xShape = rDrawing.createAndInsertXShape( CREATE_OUSTRING( "com.sun.star.drawing.LineShape" ), rxShapes, rShapeRect );
    PropertySet( xShape ).setProperties( aPropMap );
    return xShape;
}

// ============================================================================

BiffRectObject::BiffRectObject( const WorksheetHelper& rHelper ) :
    BiffDrawingObjectBase( rHelper ),
    mnFrameFlags( 0 )
{
    setAreaObj( true );
}

void BiffRectObject::readFrameData( BiffInputStream& rStrm )
{
    rStrm >> maFillModel >> maLineModel >> mnFrameFlags;
}

void BiffRectObject::convertRectProperties( ShapePropertyMap& rPropMap ) const
{
    convertLineProperties( rPropMap, maLineModel );
    convertFillProperties( rPropMap, maFillModel );
    convertFrameProperties( rPropMap, mnFrameFlags );
}

void BiffRectObject::implReadObjBiff3( BiffInputStream& rStrm, sal_uInt16 nMacroSize )
{
    readFrameData( rStrm );
    readMacroBiff3( rStrm, nMacroSize );
}

void BiffRectObject::implReadObjBiff4( BiffInputStream& rStrm, sal_uInt16 nMacroSize )
{
    readFrameData( rStrm );
    readMacroBiff4( rStrm, nMacroSize );
}

void BiffRectObject::implReadObjBiff5( BiffInputStream& rStrm, sal_uInt16 nNameLen, sal_uInt16 nMacroSize )
{
    readFrameData( rStrm );
    readNameBiff5( rStrm, nNameLen );
    readMacroBiff5( rStrm, nMacroSize );
}

Reference< XShape > BiffRectObject::implConvertAndInsert( BiffDrawingBase& rDrawing,
        const Reference< XShapes >& rxShapes, const Rectangle& rShapeRect ) const
{
    ShapePropertyMap aPropMap( getBaseFilter().getModelObjectHelper() );
    convertRectProperties( aPropMap );

    Reference< XShape > xShape = rDrawing.createAndInsertXShape( CREATE_OUSTRING( "com.sun.star.drawing.RectangleShape" ), rxShapes, rShapeRect );
    PropertySet( xShape ).setProperties( aPropMap );
    return xShape;
}

// ============================================================================

BiffOvalObject::BiffOvalObject( const WorksheetHelper& rHelper ) :
    BiffRectObject( rHelper )
{
}

Reference< XShape > BiffOvalObject::implConvertAndInsert( BiffDrawingBase& rDrawing,
        const Reference< XShapes >& rxShapes, const Rectangle& rShapeRect ) const
{
    ShapePropertyMap aPropMap( getBaseFilter().getModelObjectHelper() );
    convertRectProperties( aPropMap );

    Reference< XShape > xShape = rDrawing.createAndInsertXShape( CREATE_OUSTRING( "com.sun.star.drawing.EllipseShape" ), rxShapes, rShapeRect );
    PropertySet( xShape ).setProperties( aPropMap );
    return xShape;
}

// ============================================================================

BiffArcObject::BiffArcObject( const WorksheetHelper& rHelper ) :
    BiffDrawingObjectBase( rHelper ),
    mnQuadrant( BIFF_OBJ_ARC_TR )
{
    setAreaObj( false );    // arc may be 2-dimensional
}

void BiffArcObject::implReadObjBiff3( BiffInputStream& rStrm, sal_uInt16 nMacroSize )
{
    rStrm >> maFillModel >> maLineModel >> mnQuadrant;
    rStrm.skip( 1 );
    readMacroBiff3( rStrm, nMacroSize );
}

void BiffArcObject::implReadObjBiff4( BiffInputStream& rStrm, sal_uInt16 nMacroSize )
{
    rStrm >> maFillModel >> maLineModel >> mnQuadrant;
    rStrm.skip( 1 );
    readMacroBiff4( rStrm, nMacroSize );
}

void BiffArcObject::implReadObjBiff5( BiffInputStream& rStrm, sal_uInt16 nNameLen, sal_uInt16 nMacroSize )
{
    rStrm >> maFillModel >> maLineModel >> mnQuadrant;
    rStrm.skip( 1 );
    readNameBiff5( rStrm, nNameLen );
    readMacroBiff5( rStrm, nMacroSize );
}

Reference< XShape > BiffArcObject::implConvertAndInsert( BiffDrawingBase& rDrawing,
        const Reference< XShapes >& rxShapes, const Rectangle& rShapeRect ) const
{
    ShapePropertyMap aPropMap( getBaseFilter().getModelObjectHelper() );
    convertLineProperties( aPropMap, maLineModel );
    convertFillProperties( aPropMap, maFillModel );

    /*  Simulate arc objects with ellipse sections. While the original arc
        object uses the entire object rectangle, only one quarter of the
        ellipse shape will be visible. Thus, the size of the ellipse shape
        needs to be extended and its position adjusted according to the visible
        quadrant. */
    Rectangle aNewRect( rShapeRect.X, rShapeRect.Y, rShapeRect.Width * 2, rShapeRect.Height * 2 );
    long nStartAngle = 0;
    switch( mnQuadrant )
    {
        default:
        case BIFF_OBJ_ARC_TR: nStartAngle =     0; aNewRect.X -= rShapeRect.Width;                                  break;
        case BIFF_OBJ_ARC_TL: nStartAngle =  9000;                                                                  break;
        case BIFF_OBJ_ARC_BL: nStartAngle = 18000;                                 aNewRect.Y -= rShapeRect.Height; break;
        case BIFF_OBJ_ARC_BR: nStartAngle = 27000; aNewRect.X -= rShapeRect.Width; aNewRect.Y -= rShapeRect.Height; break;
    }
    long nEndAngle = (nStartAngle + 9000) % 36000;
    aPropMap.setProperty( PROP_CircleKind, maFillModel.isFilled() ? CircleKind_SECTION : CircleKind_ARC );
    aPropMap.setProperty( PROP_CircleStartAngle, nStartAngle );
    aPropMap.setProperty( PROP_CircleEndAngle, nEndAngle );

    // create the shape
    Reference< XShape > xShape = rDrawing.createAndInsertXShape( CREATE_OUSTRING( "com.sun.star.drawing.EllipseShape" ), rxShapes, aNewRect );
    PropertySet( xShape ).setProperties( aPropMap );
    return xShape;
}

// ============================================================================

BiffPolygonObject::BiffPolygonObject( const WorksheetHelper& rHelper ) :
    BiffRectObject( rHelper ),
    mnPolyFlags( 0 ),
    mnPointCount( 0 )
{
    setAreaObj( false );    // polygon may be 2-dimensional
}

void BiffPolygonObject::implReadObjBiff4( BiffInputStream& rStrm, sal_uInt16 nMacroSize )
{
    readFrameData( rStrm );
    rStrm >> mnPolyFlags;
    rStrm.skip( 10 );
    rStrm >> mnPointCount;
    rStrm.skip( 8 );
    readMacroBiff4( rStrm, nMacroSize );
    importCoordList( rStrm );
}

void BiffPolygonObject::implReadObjBiff5( BiffInputStream& rStrm, sal_uInt16 nNameLen, sal_uInt16 nMacroSize )
{
    readFrameData( rStrm );
    rStrm >> mnPolyFlags;
    rStrm.skip( 10 );
    rStrm >> mnPointCount;
    rStrm.skip( 8 );
    readNameBiff5( rStrm, nNameLen );
    readMacroBiff5( rStrm, nMacroSize );
    importCoordList( rStrm );
}

namespace {

Point lclGetPolyPoint( const Rectangle& rAnchorRect, const Point& rPoint )
{
    // polygon coordinates are given in 1/16384 of shape size
    return Point(
        rAnchorRect.X + static_cast< sal_Int32 >( rAnchorRect.Width * getLimitedValue< double >( static_cast< double >( rPoint.X ) / 16384.0, 0.0, 1.0 ) + 0.5 ),
        rAnchorRect.Y + static_cast< sal_Int32 >( rAnchorRect.Height * getLimitedValue< double >( static_cast< double >( rPoint.Y ) / 16384.0, 0.0, 1.0 ) + 0.5 ) );
}

} // namespace

Reference< XShape > BiffPolygonObject::implConvertAndInsert( BiffDrawingBase& rDrawing,
        const Reference< XShapes >& rxShapes, const Rectangle& rShapeRect ) const
{
    Reference< XShape > xShape;
    if( maCoords.size() >= 2 )
    {
        ShapePropertyMap aPropMap( getBaseFilter().getModelObjectHelper() );
        convertRectProperties( aPropMap );

        // create the polygon
        PointVector aPolygon;
        for( PointVector::const_iterator aIt = maCoords.begin(), aEnd = maCoords.end(); aIt != aEnd; ++aIt )
            aPolygon.push_back( lclGetPolyPoint( rShapeRect, *aIt ) );
        // close polygon if specified
        if( getFlag( mnPolyFlags, BIFF_OBJ_POLY_CLOSED ) && ((maCoords.front().X != maCoords.back().X) || (maCoords.front().Y != maCoords.back().Y)) )
            aPolygon.push_back( aPolygon.front() );
        PointSequenceSequence aPoints( 1 );
        aPoints[ 0 ] = ContainerHelper::vectorToSequence( aPolygon );
        aPropMap.setProperty( PROP_PolyPolygon, aPoints );

        // create the shape
        OUString aService = maFillModel.isFilled() ?
            CREATE_OUSTRING( "com.sun.star.drawing.PolyPolygonShape" ) :
            CREATE_OUSTRING( "com.sun.star.drawing.PolyLineShape" );
        xShape = rDrawing.createAndInsertXShape( aService, rxShapes, rShapeRect );
        PropertySet( xShape ).setProperties( aPropMap );
    }
    return xShape;
}

void BiffPolygonObject::importCoordList( BiffInputStream& rStrm )
{
    if( (rStrm.getNextRecId() == BIFF_ID_COORDLIST) && rStrm.startNextRecord() )
    {
        OSL_ENSURE( rStrm.getRemaining() / 4 == mnPointCount, "BiffPolygonObject::importCoordList - wrong polygon point count" );
        while( rStrm.getRemaining() >= 4 )
        {
            sal_uInt16 nX, nY;
            rStrm >> nX >> nY;
            maCoords.push_back( Point( nX, nY ) );
        }
    }
}

// ============================================================================
// BIFF drawing page
// ============================================================================

BiffDrawingBase::BiffDrawingBase( const WorksheetHelper& rHelper, const Reference< XDrawPage >& rxDrawPage ) :
    WorksheetHelper( rHelper ),
    mxDrawPage( rxDrawPage )
{
}

void BiffDrawingBase::importObj( BiffInputStream& rStrm )
{
    BiffDrawingObjectRef xDrawingObj;

#if 0
    /*  #i61786# In BIFF8 streams, OBJ records may occur without MSODRAWING
        records. In this case, the OBJ records are in BIFF5 format. Do a sanity
        check here that there is no DFF data loaded before. */
    DBG_ASSERT( maDffStrm.Tell() == 0, "BiffDrawingBase::importObj - unexpected DFF stream data, OBJ will be ignored" );
    if( maDffStrm.Tell() == 0 ) switch( GetBiff() )
#else
    switch( getBiff() )
#endif
    {
        case BIFF3:
            xDrawingObj = BiffDrawingObjectBase::importObjBiff3( *this, rStrm );
        break;
        case BIFF4:
            xDrawingObj = BiffDrawingObjectBase::importObjBiff4( *this, rStrm );
        break;
        case BIFF5:
// TODO: add BIFF8 when DFF is supported
//        case BIFF8:
            xDrawingObj = BiffDrawingObjectBase::importObjBiff5( *this, rStrm );
        break;
        default:;
    }

    if( xDrawingObj.get() )
    {
        // insert into maRawObjs or into the last open group object
        maRawObjs.insertGrouped( xDrawingObj );
        // to be able to find objects by ID
        maObjMapId[ xDrawingObj->getObjId() ] = xDrawingObj;
    }
}

void BiffDrawingBase::setSkipObj( sal_uInt16 nObjId )
{
    /*  Store identifiers of objects to be skipped in a separate list (the OBJ
        record may not be read yet). In the finalization phase, all objects
        registered here will be skipped. */
    maSkipObjs.push_back( nObjId );
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

Reference< XShape > BiffDrawingBase::createAndInsertXShape( const OUString& rService,
        const Reference< XShapes >& rxShapes, const Rectangle& rShapeRect ) const
{
    OSL_ENSURE( rService.getLength() > 0, "BiffDrawingBase::createAndInsertXShape - missing UNO shape service name" );
    OSL_ENSURE( rxShapes.is(), "BiffDrawingBase::createAndInsertXShape - missing XShapes container" );
    Reference< XShape > xShape;
    if( (rService.getLength() > 0) && rxShapes.is() ) try
    {
        xShape.set( getBaseFilter().getModelFactory()->createInstance( rService ), UNO_QUERY_THROW );
        // insert shape into passed shape collection (maybe drawpage or group shape)
        rxShapes->add( xShape );
        xShape->setPosition( Point( rShapeRect.X, rShapeRect.Y ) );
        xShape->setSize( Size( rShapeRect.Width, rShapeRect.Height ) );
    }
    catch( Exception& )
    {
    }
    OSL_ENSURE( xShape.is(), "BiffDrawingBase::createAndInsertXShape - cannot instanciate shape object" );
    return xShape;
}

// protected ------------------------------------------------------------------

void BiffDrawingBase::appendRawObject( const BiffDrawingObjectRef& rxDrawingObj )
{
    OSL_ENSURE( rxDrawingObj.get(), "BiffDrawingBase::appendRawObject - unexpected empty object reference" );
    maRawObjs.append( rxDrawingObj );
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
