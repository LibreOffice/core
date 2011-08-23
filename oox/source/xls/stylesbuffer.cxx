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

#include "oox/xls/stylesbuffer.hxx"
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/FontFamily.hpp>
#include <com/sun/star/awt/FontPitch.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontType.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/awt/XFont2.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/table/CellVertJustify2.hpp>
#include <com/sun/star/table/CellJustifyMethod.hpp>
#include <com/sun/star/table/TableBorder.hpp>
#include <rtl/tencinfo.h>
#include <rtl/ustrbuf.hxx>
#include "properties.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/propertymap.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/helper/recordinputstream.hxx"
#include "oox/core/filterbase.hxx"
#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/condformatbuffer.hxx"
#include "oox/xls/excelhandlers.hxx"
#include "oox/xls/themebuffer.hxx"
#include "oox/xls/unitconverter.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::UNO_SET_THROW;
using ::com::sun::star::container::XIndexAccess;
using ::com::sun::star::container::XNameAccess;
using ::com::sun::star::container::XNamed;
using ::com::sun::star::awt::FontDescriptor;
using ::com::sun::star::awt::XDevice;
using ::com::sun::star::awt::XFont2;
using ::com::sun::star::table::BorderLine;
using ::com::sun::star::table::BorderLine2;
using ::com::sun::star::table::TableBorder;
using ::com::sun::star::text::XText;
using ::com::sun::star::style::XStyle;
using ::oox::core::FilterBase;

namespace oox {
namespace xls {

// ============================================================================

namespace {

// OOXML constants ------------------------------------------------------------

// OOX predefined color indexes (also used in BIFF3-BIFF8)
const sal_Int32 OOX_COLOR_USEROFFSET        = 0;        /// First user defined color in palette (OOX).
const sal_Int32 BIFF_COLOR_USEROFFSET       = 8;        /// First user defined color in palette (BIFF).

// OOX font family (also used in BIFF)
const sal_Int32 OOX_FONTFAMILY_NONE         = 0;
const sal_Int32 OOX_FONTFAMILY_ROMAN        = 1;
const sal_Int32 OOX_FONTFAMILY_SWISS        = 2;
const sal_Int32 OOX_FONTFAMILY_MODERN       = 3;
const sal_Int32 OOX_FONTFAMILY_SCRIPT       = 4;
const sal_Int32 OOX_FONTFAMILY_DECORATIVE   = 5;

// OOX cell text direction (also used in BIFF)
const sal_Int32 OOX_XF_TEXTDIR_CONTEXT      = 0;
const sal_Int32 OOX_XF_TEXTDIR_LTR          = 1;
const sal_Int32 OOX_XF_TEXTDIR_RTL          = 2;

// OOX cell rotation (also used in BIFF)
const sal_Int32 OOX_XF_ROTATION_NONE        = 0;
const sal_Int32 OOX_XF_ROTATION_90CCW       = 90;
const sal_Int32 OOX_XF_ROTATION_90CW        = 180;
const sal_Int32 OOX_XF_ROTATION_STACKED     = 255;

// OOX cell indentation
const sal_Int32 OOX_XF_INDENT_NONE          = 0;

// OOX built-in cell styles (also used in BIFF)
const sal_Int32 OOX_STYLE_NORMAL            = 0;        /// Default cell style.
const sal_Int32 OOX_STYLE_ROWLEVEL          = 1;        /// RowLevel_x cell style.
const sal_Int32 OOX_STYLE_COLLEVEL          = 2;        /// ColLevel_x cell style.

const sal_Int32 OOX_STYLE_LEVELCOUNT        = 7;        /// Number of outline level styles.

// OOBIN constants ------------------------------------------------------------

// OOBIN color types
const sal_uInt8 OOBIN_COLOR_AUTO            = 0;
const sal_uInt8 OOBIN_COLOR_INDEXED         = 1;
const sal_uInt8 OOBIN_COLOR_RGB             = 2;
const sal_uInt8 OOBIN_COLOR_THEME           = 3;

// OOBIN diagonal borders
const sal_uInt8 OOBIN_BORDER_DIAG_TLBR      = 0x01;     /// Top-left to bottom-right.
const sal_uInt8 OOBIN_BORDER_DIAG_BLTR      = 0x02;     /// Bottom-left to top-right.

// OOBIN gradient fill
const sal_Int32 OOBIN_FILL_GRADIENT         = 40;

// OOBIN XF flags
const sal_uInt32 OOBIN_XF_WRAPTEXT          = 0x00400000;
const sal_uInt32 OOBIN_XF_JUSTLASTLINE      = 0x00800000;
const sal_uInt32 OOBIN_XF_SHRINK            = 0x01000000;
const sal_uInt32 OOBIN_XF_LOCKED            = 0x10000000;
const sal_uInt32 OOBIN_XF_HIDDEN            = 0x20000000;

// OOBIN XF attribute used flags
const sal_uInt16 OOBIN_XF_NUMFMT_USED       = 0x0001;
const sal_uInt16 OOBIN_XF_FONT_USED         = 0x0002;
const sal_uInt16 OOBIN_XF_ALIGN_USED        = 0x0004;
const sal_uInt16 OOBIN_XF_BORDER_USED       = 0x0008;
const sal_uInt16 OOBIN_XF_AREA_USED         = 0x0010;
const sal_uInt16 OOBIN_XF_PROT_USED         = 0x0020;

// OOBIN DXF constants
const sal_uInt16 OOBIN_DXF_FILL_PATTERN     = 0;
const sal_uInt16 OOBIN_DXF_FILL_FGCOLOR     = 1;
const sal_uInt16 OOBIN_DXF_FILL_BGCOLOR     = 2;
const sal_uInt16 OOBIN_DXF_FILL_GRADIENT    = 3;
const sal_uInt16 OOBIN_DXF_FILL_STOP        = 4;
const sal_uInt16 OOBIN_DXF_FONT_COLOR       = 5;
const sal_uInt16 OOBIN_DXF_BORDER_TOP       = 6;
const sal_uInt16 OOBIN_DXF_BORDER_BOTTOM    = 7;
const sal_uInt16 OOBIN_DXF_BORDER_LEFT      = 8;
const sal_uInt16 OOBIN_DXF_BORDER_RIGHT     = 9;
const sal_uInt16 OOBIN_DXF_BORDER_DIAG      = 10;
const sal_uInt16 OOBIN_DXF_BORDER_VERT      = 11;
const sal_uInt16 OOBIN_DXF_BORDER_HOR       = 12;
const sal_uInt16 OOBIN_DXF_BORDER_DIAGUP    = 13;
const sal_uInt16 OOBIN_DXF_BORDER_DIAGDOWN  = 14;
const sal_uInt16 OOBIN_DXF_FONT_NAME        = 24;
const sal_uInt16 OOBIN_DXF_FONT_WEIGHT      = 25;
const sal_uInt16 OOBIN_DXF_FONT_UNDERLINE   = 26;
const sal_uInt16 OOBIN_DXF_FONT_ESCAPEMENT  = 27;
const sal_uInt16 OOBIN_DXF_FONT_ITALIC      = 28;
const sal_uInt16 OOBIN_DXF_FONT_STRIKE      = 29;
const sal_uInt16 OOBIN_DXF_FONT_OUTLINE     = 30;
const sal_uInt16 OOBIN_DXF_FONT_SHADOW      = 31;
const sal_uInt16 OOBIN_DXF_FONT_CONDENSE    = 32;
const sal_uInt16 OOBIN_DXF_FONT_EXTEND      = 33;
const sal_uInt16 OOBIN_DXF_FONT_CHARSET     = 34;
const sal_uInt16 OOBIN_DXF_FONT_FAMILY      = 35;
const sal_uInt16 OOBIN_DXF_FONT_HEIGHT      = 36;
const sal_uInt16 OOBIN_DXF_FONT_SCHEME      = 37;
const sal_uInt16 OOBIN_DXF_NUMFMT_CODE      = 38;
const sal_uInt16 OOBIN_DXF_NUMFMT_ID        = 41;

// OOBIN CELLSTYLE flags
const sal_uInt16 OOBIN_CELLSTYLE_BUILTIN    = 0x0001;
const sal_uInt16 OOBIN_CELLSTYLE_HIDDEN     = 0x0002;
const sal_uInt16 OOBIN_CELLSTYLE_CUSTOM     = 0x0004;

// OOBIN and BIFF constants ---------------------------------------------------

// BIFF predefined color indexes
const sal_uInt16 BIFF2_COLOR_BLACK          = 0;        /// Black (text) in BIFF2.
const sal_uInt16 BIFF2_COLOR_WHITE          = 1;        /// White (background) in BIFF2.

// BIFF font flags, also used in OOBIN
const sal_uInt16 BIFF_FONTFLAG_BOLD         = 0x0001;
const sal_uInt16 BIFF_FONTFLAG_ITALIC       = 0x0002;
const sal_uInt16 BIFF_FONTFLAG_UNDERLINE    = 0x0004;
const sal_uInt16 BIFF_FONTFLAG_STRIKEOUT    = 0x0008;
const sal_uInt16 BIFF_FONTFLAG_OUTLINE      = 0x0010;
const sal_uInt16 BIFF_FONTFLAG_SHADOW       = 0x0020;
const sal_uInt16 BIFF_FONTFLAG_CONDENSE     = 0x0040;

// BIFF font weight
const sal_uInt16 BIFF_FONTWEIGHT_BOLD       = 450;

// BIFF font underline, also used in OOBIN
const sal_uInt8 BIFF_FONTUNDERL_NONE        = 0;
const sal_uInt8 BIFF_FONTUNDERL_SINGLE      = 1;
const sal_uInt8 BIFF_FONTUNDERL_DOUBLE      = 2;
const sal_uInt8 BIFF_FONTUNDERL_SINGLE_ACC  = 33;
const sal_uInt8 BIFF_FONTUNDERL_DOUBLE_ACC  = 34;

// BIFF XF flags
const sal_uInt16 BIFF_XF_LOCKED             = 0x0001;
const sal_uInt16 BIFF_XF_HIDDEN             = 0x0002;
const sal_uInt16 BIFF_XF_STYLE              = 0x0004;
const sal_uInt16 BIFF_XF_STYLEPARENT        = 0x0FFF;   /// Syles don't have a parent.
const sal_uInt16 BIFF_XF_WRAPTEXT           = 0x0008;   /// Automatic line break.
const sal_uInt16 BIFF_XF_JUSTLASTLINE       = 0x0080;
const sal_uInt16 BIFF_XF_SHRINK             = 0x0010;   /// Shrink to fit into cell.
const sal_uInt16 BIFF_XF_MERGE              = 0x0020;

// BIFF XF attribute used flags
const sal_uInt8 BIFF_XF_NUMFMT_USED         = 0x01;
const sal_uInt8 BIFF_XF_FONT_USED           = 0x02;
const sal_uInt8 BIFF_XF_ALIGN_USED          = 0x04;
const sal_uInt8 BIFF_XF_BORDER_USED         = 0x08;
const sal_uInt8 BIFF_XF_AREA_USED           = 0x10;
const sal_uInt8 BIFF_XF_PROT_USED           = 0x20;

// BIFF XF text orientation
const sal_uInt8 BIFF_XF_ORIENT_NONE         = 0;
const sal_uInt8 BIFF_XF_ORIENT_STACKED      = 1;        /// Stacked top to bottom.
const sal_uInt8 BIFF_XF_ORIENT_90CCW        = 2;        /// 90 degr. counterclockwise.
const sal_uInt8 BIFF_XF_ORIENT_90CW         = 3;        /// 90 degr. clockwise.

// BIFF XF line styles
const sal_uInt8 BIFF_LINE_NONE              = 0;
const sal_uInt8 BIFF_LINE_THIN              = 1;

// BIFF XF patterns
const sal_uInt8 BIFF_PATT_NONE              = 0;
const sal_uInt8 BIFF_PATT_125               = 17;

// BIFF2 XF flags
const sal_uInt8 BIFF2_XF_VALFMT_MASK        = 0x3F;
const sal_uInt8 BIFF2_XF_LOCKED             = 0x40;
const sal_uInt8 BIFF2_XF_HIDDEN             = 0x80;
const sal_uInt8 BIFF2_XF_LEFTLINE           = 0x08;
const sal_uInt8 BIFF2_XF_RIGHTLINE          = 0x10;
const sal_uInt8 BIFF2_XF_TOPLINE            = 0x20;
const sal_uInt8 BIFF2_XF_BOTTOMLINE         = 0x40;
const sal_uInt8 BIFF2_XF_BACKGROUND         = 0x80;

// BIFF8 diagonal borders
const sal_uInt32 BIFF_XF_DIAG_TLBR          = 0x40000000;   /// Top-left to bottom-right.
const sal_uInt32 BIFF_XF_DIAG_BLTR          = 0x80000000;   /// Bottom-left to top-right.

// BIFF STYLE flags
const sal_uInt16 BIFF_STYLE_BUILTIN         = 0x8000;
const sal_uInt16 BIFF_STYLE_XFMASK          = 0x0FFF;

// BIFF STYLEEXT flags
const sal_uInt8 BIFF_STYLEEXT_BUILTIN       = 0x01;
const sal_uInt8 BIFF_STYLEEXT_HIDDEN        = 0x02;
const sal_uInt8 BIFF_STYLEEXT_CUSTOM        = 0x04;

// BIFF conditional formatting
const sal_uInt32 BIFF_CFRULE_BORDER_LEFT    = 0x00000400;
const sal_uInt32 BIFF_CFRULE_BORDER_RIGHT   = 0x00000800;
const sal_uInt32 BIFF_CFRULE_BORDER_TOP     = 0x00001000;
const sal_uInt32 BIFF_CFRULE_BORDER_BOTTOM  = 0x00002000;
const sal_uInt32 BIFF_CFRULE_FILL_PATTERN   = 0x00010000;
const sal_uInt32 BIFF_CFRULE_FILL_PATTCOLOR = 0x00020000;
const sal_uInt32 BIFF_CFRULE_FILL_FILLCOLOR = 0x00040000;
const sal_uInt32 BIFF_CFRULE_FONTBLOCK      = 0x04000000;
const sal_uInt32 BIFF_CFRULE_ALIGNBLOCK     = 0x08000000;
const sal_uInt32 BIFF_CFRULE_BORDERBLOCK    = 0x10000000;
const sal_uInt32 BIFF_CFRULE_FILLBLOCK      = 0x20000000;
const sal_uInt32 BIFF_CFRULE_PROTBLOCK      = 0x40000000;

const sal_uInt32 BIFF_CFRULE_FONT_STYLE     = 0x00000002;   /// Font posture or weight modified?
const sal_uInt32 BIFF_CFRULE_FONT_OUTLINE   = 0x00000008;   /// Font outline modified?
const sal_uInt32 BIFF_CFRULE_FONT_SHADOW    = 0x00000010;   /// Font shadow modified?
const sal_uInt32 BIFF_CFRULE_FONT_STRIKEOUT = 0x00000080;   /// Font cancellation modified?
const sal_uInt32 BIFF_CFRULE_FONT_UNDERL    = 0x00000001;   /// Font underline type modified?
const sal_uInt32 BIFF_CFRULE_FONT_ESCAPEM   = 0x00000001;   /// Font escapement type modified?

// ----------------------------------------------------------------------------

sal_Int32 lclReadRgbColor( BinaryInputStream& rStrm )
{
    sal_uInt8 nR, nG, nB, nA;
    rStrm >> nR >> nG >> nB >> nA;
    sal_Int32 nValue = nA;
    nValue <<= 8;
    nValue |= nR;
    nValue <<= 8;
    nValue |= nG;
    nValue <<= 8;
    nValue |= nB;
    return nValue;
}

} // namespace

// ============================================================================

ExcelGraphicHelper::ExcelGraphicHelper( const WorkbookHelper& rHelper ) :
    GraphicHelper( rHelper.getGlobalFactory(), rHelper.getBaseFilter().getTargetFrame(), rHelper.getBaseFilter().getStorage() ),
    WorkbookHelper( rHelper )
{
}

sal_Int32 ExcelGraphicHelper::getSchemeColor( sal_Int32 nToken ) const
{
    if( getFilterType() == FILTER_OOX )
        return getTheme().getColorByToken( nToken );
    return GraphicHelper::getSchemeColor( nToken );
}

sal_Int32 ExcelGraphicHelper::getPaletteColor( sal_Int32 nPaletteIdx ) const
{
    return getStyles().getPaletteColor( nPaletteIdx );
}

// ============================================================================

void Color::setAuto()
{
    clearTransformations();
    setSchemeClr( XML_phClr );
}

void Color::setRgb( sal_Int32 nRgbValue, double fTint )
{
    clearTransformations();
    setSrgbClr( nRgbValue & 0xFFFFFF );
    if( fTint != 0.0 ) addExcelTintTransformation( fTint );
}

void Color::setTheme( sal_Int32 nThemeIdx, double fTint )
{
    clearTransformations();
    static const sal_Int32 spnColorTokens[] = {
        XML_lt1, XML_dk1, XML_lt2, XML_dk2, XML_accent1, XML_accent2,
        XML_accent3, XML_accent4, XML_accent5, XML_accent6, XML_hlink, XML_folHlink };
    setSchemeClr( STATIC_ARRAY_SELECT( spnColorTokens, nThemeIdx, XML_TOKEN_INVALID ) );
    if( fTint != 0.0 ) addExcelTintTransformation( fTint );
}

void Color::setIndexed( sal_Int32 nPaletteIdx, double fTint )
{
    clearTransformations();
    setPaletteClr( nPaletteIdx );
    if( fTint != 0.0 ) addExcelTintTransformation( fTint );
}

void Color::importColor( const AttributeList& rAttribs )
{
    if( rAttribs.getBool( XML_auto, false ) )
        setAuto();
    else if( rAttribs.hasAttribute( XML_rgb ) )
        setRgb( rAttribs.getIntegerHex( XML_rgb, API_RGB_TRANSPARENT ), rAttribs.getDouble( XML_tint, 0.0 ) );
    else if( rAttribs.hasAttribute( XML_theme ) )
        setTheme( rAttribs.getInteger( XML_theme, -1 ), rAttribs.getDouble( XML_tint, 0.0 ) );
    else if( rAttribs.hasAttribute( XML_indexed ) )
        setIndexed( rAttribs.getInteger( XML_indexed, -1 ), rAttribs.getDouble( XML_tint, 0.0 ) );
    else
    {
        OSL_ENSURE( false, "Color::importColor - unknown color type" );
        setAuto();
    }
}

void Color::importColor( RecordInputStream& rStrm )
{
    sal_uInt8 nFlags, nIndex;
    sal_Int16 nTint;
    rStrm >> nFlags >> nIndex >> nTint;

    // scale tint from signed 16-bit to double range -1.0 ... 1.0
    double fTint = nTint;
    if( nTint < 0 )
        fTint /= -SAL_MIN_INT16;
    else if( nTint > 0 )
        fTint /= SAL_MAX_INT16;

    switch( extractValue< sal_uInt8 >( nFlags, 1, 7 ) )
    {
        case OOBIN_COLOR_AUTO:
            setAuto();
            rStrm.skip( 4 );
        break;
        case OOBIN_COLOR_INDEXED:
            setIndexed( nIndex, fTint );
            rStrm.skip( 4 );
        break;
        case OOBIN_COLOR_RGB:
            setRgb( lclReadRgbColor( rStrm ), fTint );
        break;
        case OOBIN_COLOR_THEME:
            setTheme( nIndex, fTint );
            rStrm.skip( 4 );
        break;
        default:
            OSL_ENSURE( false, "Color::importColor - unknown color type" );
            setAuto();
            rStrm.skip( 4 );
    }
}

void Color::importColorId( RecordInputStream& rStrm )
{
    setIndexed( rStrm.readInt32() );
}

void Color::importColorRgb( RecordInputStream& rStrm )
{
    setRgb( lclReadRgbColor( rStrm ) );
}

void Color::importColorId( BiffInputStream& rStrm, bool b16Bit )
{
    setIndexed( b16Bit ? rStrm.readuInt16() : rStrm.readuInt8() );
}

void Color::importColorRgb( BiffInputStream& rStrm )
{
    setRgb( lclReadRgbColor( rStrm ) );
}

RecordInputStream& operator>>( RecordInputStream& rStrm, Color& orColor )
{
    orColor.importColor( rStrm );
    return rStrm;
}

// ============================================================================

namespace {

/** Standard EGA colors, bright. */
#define PALETTE_EGA_COLORS_LIGHT \
            0x000000, 0xFFFFFF, 0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00, 0xFF00FF, 0x00FFFF
/** Standard EGA colors, dark. */
#define PALETTE_EGA_COLORS_DARK \
            0x800000, 0x008000, 0x000080, 0x808000, 0x800080, 0x008080, 0xC0C0C0, 0x808080

/** Default color table for BIFF2. */
static const sal_Int32 spnDefColors2[] =
{
/*  0 */    PALETTE_EGA_COLORS_LIGHT
};

/** Default color table for BIFF3/BIFF4. */
static const sal_Int32 spnDefColors3[] =
{
/*  0 */    PALETTE_EGA_COLORS_LIGHT,
/*  8 */    PALETTE_EGA_COLORS_LIGHT,
/* 16 */    PALETTE_EGA_COLORS_DARK
};

/** Default color table for BIFF5. */
static const sal_Int32 spnDefColors5[] =
{
/*  0 */    PALETTE_EGA_COLORS_LIGHT,
/*  8 */    PALETTE_EGA_COLORS_LIGHT,
/* 16 */    PALETTE_EGA_COLORS_DARK,
/* 24 */    0x8080FF, 0x802060, 0xFFFFC0, 0xA0E0E0, 0x600080, 0xFF8080, 0x0080C0, 0xC0C0FF,
/* 32 */    0x000080, 0xFF00FF, 0xFFFF00, 0x00FFFF, 0x800080, 0x800000, 0x008080, 0x0000FF,
/* 40 */    0x00CFFF, 0x69FFFF, 0xE0FFE0, 0xFFFF80, 0xA6CAF0, 0xDD9CB3, 0xB38FEE, 0xE3E3E3,
/* 48 */    0x2A6FF9, 0x3FB8CD, 0x488436, 0x958C41, 0x8E5E42, 0xA0627A, 0x624FAC, 0x969696,
/* 56 */    0x1D2FBE, 0x286676, 0x004500, 0x453E01, 0x6A2813, 0x85396A, 0x4A3285, 0x424242
};

/** Default color table for BIFF8/OOX. */
static const sal_Int32 spnDefColors8[] =
{
/*  0 */    PALETTE_EGA_COLORS_LIGHT,
/*  8 */    PALETTE_EGA_COLORS_LIGHT,
/* 16 */    PALETTE_EGA_COLORS_DARK,
/* 24 */    0x9999FF, 0x993366, 0xFFFFCC, 0xCCFFFF, 0x660066, 0xFF8080, 0x0066CC, 0xCCCCFF,
/* 32 */    0x000080, 0xFF00FF, 0xFFFF00, 0x00FFFF, 0x800080, 0x800000, 0x008080, 0x0000FF,
/* 40 */    0x00CCFF, 0xCCFFFF, 0xCCFFCC, 0xFFFF99, 0x99CCFF, 0xFF99CC, 0xCC99FF, 0xFFCC99,
/* 48 */    0x3366FF, 0x33CCCC, 0x99CC00, 0xFFCC00, 0xFF9900, 0xFF6600, 0x666699, 0x969696,
/* 56 */    0x003366, 0x339966, 0x003300, 0x333300, 0x993300, 0x993366, 0x333399, 0x333333
};

#undef PALETTE_EGA_COLORS_LIGHT
#undef PALETTE_EGA_COLORS_DARK

} // namespace

// ----------------------------------------------------------------------------

ColorPalette::ColorPalette( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
    // default colors
    switch( getFilterType() )
    {
        case FILTER_OOX:
            maColors.insert( maColors.begin(), spnDefColors8, STATIC_ARRAY_END( spnDefColors8 ) );
            mnAppendIndex = OOX_COLOR_USEROFFSET;
        break;
        case FILTER_BIFF:
            switch( getBiff() )
            {
                case BIFF2: maColors.insert( maColors.begin(), spnDefColors2, STATIC_ARRAY_END( spnDefColors2 ) );  break;
                case BIFF3:
                case BIFF4: maColors.insert( maColors.begin(), spnDefColors3, STATIC_ARRAY_END( spnDefColors3 ) );  break;
                case BIFF5: maColors.insert( maColors.begin(), spnDefColors5, STATIC_ARRAY_END( spnDefColors5 ) );  break;
                case BIFF8: maColors.insert( maColors.begin(), spnDefColors8, STATIC_ARRAY_END( spnDefColors8 ) );  break;
                case BIFF_UNKNOWN: break;
            }
            mnAppendIndex = BIFF_COLOR_USEROFFSET;
        break;
        case FILTER_UNKNOWN: break;
    }
}

void ColorPalette::importPaletteColor( const AttributeList& rAttribs )
{
    appendColor( rAttribs.getIntegerHex( XML_rgb, API_RGB_WHITE ) );
}

void ColorPalette::importPaletteColor( RecordInputStream& rStrm )
{
    sal_Int32 nRgb = lclReadRgbColor( rStrm );
    appendColor( nRgb & 0xFFFFFF );
}

void ColorPalette::importPalette( BiffInputStream& rStrm )
{
    sal_uInt16 nCount;
    rStrm >> nCount;
    OSL_ENSURE( rStrm.getRemaining() == 4 * nCount, "ColorPalette::importPalette - wrong palette size" );

    // fill palette from BIFF_COLOR_USEROFFSET
    mnAppendIndex = BIFF_COLOR_USEROFFSET;
    for( sal_uInt16 nIndex = 0; !rStrm.isEof() && (nIndex < nCount); ++nIndex )
    {
        sal_Int32 nRgb = lclReadRgbColor( rStrm );
        appendColor( nRgb & 0xFFFFFF );
    }
}

sal_Int32 ColorPalette::getColor( sal_Int32 nPaletteIdx ) const
{
    sal_Int32 nColor = API_RGB_TRANSPARENT;
    if( const sal_Int32* pnPaletteColor = ContainerHelper::getVectorElement( maColors, nPaletteIdx ) )
    {
        nColor = *pnPaletteColor;
    }
    else switch( nPaletteIdx )
    {
        case OOX_COLOR_WINDOWTEXT3:
        case OOX_COLOR_WINDOWTEXT:
        case OOX_COLOR_CHWINDOWTEXT:    nColor = getBaseFilter().getGraphicHelper().getSystemColor( XML_windowText );   break;
        case OOX_COLOR_WINDOWBACK3:
        case OOX_COLOR_WINDOWBACK:
        case OOX_COLOR_CHWINDOWBACK:    nColor = getBaseFilter().getGraphicHelper().getSystemColor( XML_window );       break;
        case OOX_COLOR_BUTTONBACK:      nColor = getBaseFilter().getGraphicHelper().getSystemColor( XML_btnFace );      break;
        case OOX_COLOR_CHBORDERAUTO:    nColor = API_RGB_BLACK; /* really always black? */                              break;
        case OOX_COLOR_NOTEBACK:        nColor = getBaseFilter().getGraphicHelper().getSystemColor( XML_infoBk );       break;
        case OOX_COLOR_NOTETEXT:        nColor = getBaseFilter().getGraphicHelper().getSystemColor( XML_infoText );     break;
        case OOX_COLOR_FONTAUTO:        nColor = API_RGB_TRANSPARENT;                                                   break;
        default:                        OSL_ENSURE( false, "ColorPalette::getColor - unknown color index" );
    }
    return nColor;
}

void ColorPalette::appendColor( sal_Int32 nRGBValue )
{
    if( mnAppendIndex < maColors.size() )
        maColors[ mnAppendIndex ] = nRGBValue;
    else
        maColors.push_back( nRGBValue );
    ++mnAppendIndex;
}

// ============================================================================

namespace {

void lclSetFontName( ApiScriptFontName& rFontName, const FontDescriptor& rFontDesc, bool bHasGlyphs )
{
    if( bHasGlyphs )
    {
        rFontName.maName = rFontDesc.Name;
        rFontName.mnFamily = rFontDesc.Family;
        // API font descriptor contains rtl_TextEncoding constants
        rFontName.mnTextEnc = rFontDesc.CharSet;
    }
    else
    {
        rFontName = ApiScriptFontName();
    }
}

} // namespace

// ----------------------------------------------------------------------------

FontModel::FontModel() :
    mnScheme( XML_none ),
    mnFamily( OOX_FONTFAMILY_NONE ),
    mnCharSet( WINDOWS_CHARSET_DEFAULT ),
    mfHeight( 0.0 ),
    mnUnderline( XML_none ),
    mnEscapement( XML_baseline ),
    mbBold( false ),
    mbItalic( false ),
    mbStrikeout( false ),
    mbOutline( false ),
    mbShadow( false )
{
}

void FontModel::setBinScheme( sal_uInt8 nScheme )
{
    static const sal_Int32 spnSchemes[] = { XML_none, XML_major, XML_minor };
    mnScheme = STATIC_ARRAY_SELECT( spnSchemes, nScheme, XML_none );
}

void FontModel::setBiffHeight( sal_uInt16 nHeight )
{
    mfHeight = nHeight / 20.0;  // convert twips to points
}

void FontModel::setBiffWeight( sal_uInt16 nWeight )
{
    mbBold = nWeight >= BIFF_FONTWEIGHT_BOLD;
}

void FontModel::setBiffUnderline( sal_uInt16 nUnderline )
{
    switch( nUnderline )
    {
        case BIFF_FONTUNDERL_NONE:          mnUnderline = XML_none;             break;
        case BIFF_FONTUNDERL_SINGLE:        mnUnderline = XML_single;           break;
        case BIFF_FONTUNDERL_DOUBLE:        mnUnderline = XML_double;           break;
        case BIFF_FONTUNDERL_SINGLE_ACC:    mnUnderline = XML_singleAccounting; break;
        case BIFF_FONTUNDERL_DOUBLE_ACC:    mnUnderline = XML_doubleAccounting; break;
        default:                            mnUnderline = XML_none;
    }
}

void FontModel::setBiffEscapement( sal_uInt16 nEscapement )
{
    static const sal_Int32 spnEscapes[] = { XML_baseline, XML_superscript, XML_subscript };
    mnEscapement = STATIC_ARRAY_SELECT( spnEscapes, nEscapement, XML_baseline );
}

// ----------------------------------------------------------------------------

ApiFontUsedFlags::ApiFontUsedFlags( bool bAllUsed ) :
    mbNameUsed( bAllUsed ),
    mbColorUsed( bAllUsed ),
    mbSchemeUsed( bAllUsed ),
    mbHeightUsed( bAllUsed ),
    mbUnderlineUsed( bAllUsed ),
    mbEscapementUsed( bAllUsed ),
    mbWeightUsed( bAllUsed ),
    mbPostureUsed( bAllUsed ),
    mbStrikeoutUsed( bAllUsed ),
    mbOutlineUsed( bAllUsed ),
    mbShadowUsed( bAllUsed )
{
}

// ----------------------------------------------------------------------------

ApiScriptFontName::ApiScriptFontName() :
    mnFamily( ::com::sun::star::awt::FontFamily::DONTKNOW ),
    mnTextEnc( RTL_TEXTENCODING_DONTKNOW )
{
}

// ----------------------------------------------------------------------------

ApiFontData::ApiFontData() :
    maDesc(
        CREATE_OUSTRING( "Calibri" ),
        220,                                            // height 11 points
        0,
        OUString(),
        ::com::sun::star::awt::FontFamily::DONTKNOW,
        RTL_TEXTENCODING_DONTKNOW,
        ::com::sun::star::awt::FontPitch::DONTKNOW,
        100.0,
        ::com::sun::star::awt::FontWeight::NORMAL,
        ::com::sun::star::awt::FontSlant_NONE,
        ::com::sun::star::awt::FontUnderline::NONE,
        ::com::sun::star::awt::FontStrikeout::NONE,
        0.0,
        sal_False,
        sal_False,
        ::com::sun::star::awt::FontType::DONTKNOW ),
    mnColor( API_RGB_TRANSPARENT ),
    mnEscapement( API_ESCAPE_NONE ),
    mnEscapeHeight( API_ESCAPEHEIGHT_NONE ),
    mbOutline( false ),
    mbShadow( false )
{
    maLatinFont.maName = maDesc.Name;
}

// ============================================================================

Font::Font( const WorkbookHelper& rHelper, bool bDxf ) :
    WorkbookHelper( rHelper ),
    maModel( rHelper.getTheme().getDefaultFontModel() ),
    maUsedFlags( !bDxf ),
    mbDxf( bDxf )
{
}

Font::Font( const WorkbookHelper& rHelper, const FontModel& rModel ) :
    WorkbookHelper( rHelper ),
    maModel( rModel ),
    maUsedFlags( true ),
    mbDxf( false )
{
}

void Font::importAttribs( sal_Int32 nElement, const AttributeList& rAttribs )
{
    const FontModel& rDefModel = getTheme().getDefaultFontModel();
    switch( nElement )
    {
        case XLS_TOKEN( name ):     // when in <font> element
        case XLS_TOKEN( rFont ):    // when in <rPr> element
            if( rAttribs.hasAttribute( XML_val ) )
            {
                maModel.maName = rAttribs.getXString( XML_val, OUString() );
                maUsedFlags.mbNameUsed = true;
            }
        break;
        case XLS_TOKEN( scheme ):
            maModel.mnScheme = rAttribs.getToken( XML_val, rDefModel.mnScheme );
        break;
        case XLS_TOKEN( family ):
            maModel.mnFamily = rAttribs.getInteger( XML_val, rDefModel.mnFamily );
        break;
        case XLS_TOKEN( charset ):
            maModel.mnCharSet = rAttribs.getInteger( XML_val, rDefModel.mnCharSet );
        break;
        case XLS_TOKEN( sz ):
            maModel.mfHeight = rAttribs.getDouble( XML_val, rDefModel.mfHeight );
            maUsedFlags.mbHeightUsed = true;
        break;
        case XLS_TOKEN( color ):
            maModel.maColor.importColor( rAttribs );
            maUsedFlags.mbColorUsed = true;
        break;
        case XLS_TOKEN( u ):
            maModel.mnUnderline = rAttribs.getToken( XML_val, XML_single );
            maUsedFlags.mbUnderlineUsed = true;
        break;
        case XLS_TOKEN( vertAlign ):
            maModel.mnEscapement = rAttribs.getToken( XML_val, XML_baseline );
            maUsedFlags.mbEscapementUsed = true;
        break;
        case XLS_TOKEN( b ):
            maModel.mbBold = rAttribs.getBool( XML_val, true );
            maUsedFlags.mbWeightUsed = true;
        break;
        case XLS_TOKEN( i ):
            maModel.mbItalic = rAttribs.getBool( XML_val, true );
            maUsedFlags.mbPostureUsed = true;
        break;
        case XLS_TOKEN( strike ):
            maModel.mbStrikeout = rAttribs.getBool( XML_val, true );
            maUsedFlags.mbStrikeoutUsed = true;
        break;
        case XLS_TOKEN( outline ):
            maModel.mbOutline = rAttribs.getBool( XML_val, true );
            maUsedFlags.mbOutlineUsed = true;
        break;
        case XLS_TOKEN( shadow ):
            maModel.mbShadow = rAttribs.getBool( XML_val, true );
            maUsedFlags.mbShadowUsed = true;
        break;
    }
}

void Font::importFont( RecordInputStream& rStrm )
{
    OSL_ENSURE( !mbDxf, "Font::importFont - unexpected conditional formatting flag" );

    sal_uInt16 nHeight, nFlags, nWeight, nEscapement;
    sal_uInt8 nUnderline, nFamily, nCharSet, nScheme;
    rStrm >> nHeight >> nFlags >> nWeight >> nEscapement >> nUnderline >> nFamily >> nCharSet;
    rStrm.skip( 1 );
    rStrm >> maModel.maColor >> nScheme >> maModel.maName;

    // equal constants in BIFF and OOBIN for weight, underline, and escapement
    maModel.setBinScheme( nScheme );
    maModel.setBiffHeight( nHeight );
    maModel.setBiffWeight( nWeight );
    maModel.setBiffUnderline( nUnderline );
    maModel.setBiffEscapement( nEscapement );
    maModel.mnFamily    = nFamily;
    maModel.mnCharSet   = nCharSet;
    // equal flags in BIFF and OOBIN
    maModel.mbItalic    = getFlag( nFlags, BIFF_FONTFLAG_ITALIC );
    maModel.mbStrikeout = getFlag( nFlags, BIFF_FONTFLAG_STRIKEOUT );
    maModel.mbOutline   = getFlag( nFlags, BIFF_FONTFLAG_OUTLINE );
    maModel.mbShadow    = getFlag( nFlags, BIFF_FONTFLAG_SHADOW );
}

void Font::importDxfName( RecordInputStream& rStrm )
{
    OSL_ENSURE( mbDxf, "Font::importDxfName - missing conditional formatting flag" );
    maModel.maName = rStrm.readString( false );
    maUsedFlags.mbColorUsed = true;
}

void Font::importDxfColor( RecordInputStream& rStrm )
{
    OSL_ENSURE( mbDxf, "Font::importDxfColor - missing conditional formatting flag" );
    rStrm >> maModel.maColor;
    maUsedFlags.mbColorUsed = true;
}

void Font::importDxfScheme( RecordInputStream& rStrm )
{
    OSL_ENSURE( mbDxf, "Font::importDxfScheme - missing conditional formatting flag" );
    maModel.setBinScheme( rStrm.readuInt8() );
    maUsedFlags.mbSchemeUsed = true;
}

void Font::importDxfHeight( RecordInputStream& rStrm )
{
    OSL_ENSURE( mbDxf, "Font::importDxfHeight - missing conditional formatting flag" );
    maModel.setBiffHeight( rStrm.readuInt16() );
    maUsedFlags.mbHeightUsed = true;
}

void Font::importDxfWeight( RecordInputStream& rStrm )
{
    OSL_ENSURE( mbDxf, "Font::importDxfWeight - missing conditional formatting flag" );
    maModel.setBiffWeight( rStrm.readuInt16() );
    maUsedFlags.mbWeightUsed = true;
}

void Font::importDxfUnderline( RecordInputStream& rStrm )
{
    OSL_ENSURE( mbDxf, "Font::importDxfUnderline - missing conditional formatting flag" );
    maModel.setBiffUnderline( rStrm.readuInt16() );
    maUsedFlags.mbUnderlineUsed = true;
}

void Font::importDxfEscapement( RecordInputStream& rStrm )
{
    OSL_ENSURE( mbDxf, "Font::importDxfEscapement - missing conditional formatting flag" );
    maModel.setBiffEscapement( rStrm.readuInt16() );
    maUsedFlags.mbEscapementUsed = true;
}

void Font::importDxfFlag( sal_Int32 nElement, RecordInputStream& rStrm )
{
    OSL_ENSURE( mbDxf, "Font::importDxfFlag - missing conditional formatting flag" );
    bool bFlag = rStrm.readuInt8() != 0;
    switch( nElement )
    {
        case XML_i:
            maModel.mbItalic = bFlag;
            maUsedFlags.mbPostureUsed = true;
        break;
        case XML_strike:
            maModel.mbStrikeout = bFlag;
            maUsedFlags.mbStrikeoutUsed = true;
        break;
        case XML_outline:
            maModel.mbOutline = bFlag;
            maUsedFlags.mbOutlineUsed = true;
        break;
        case XML_shadow:
            maModel.mbShadow = bFlag;
            maUsedFlags.mbShadowUsed = true;
        break;
        default:
            OSL_ENSURE( false, "Font::importDxfFlag - unexpected element identifier" );
    }
}

void Font::importFont( BiffInputStream& rStrm )
{
    OSL_ENSURE( !mbDxf, "Font::importFont - unexpected conditional formatting flag" );
    switch( getBiff() )
    {
        case BIFF2:
            importFontData2( rStrm );
            importFontName2( rStrm );
        break;
        case BIFF3:
        case BIFF4:
            importFontData2( rStrm );
            importFontColor( rStrm );
            importFontName2( rStrm );
        break;
        case BIFF5:
            importFontData2( rStrm );
            importFontColor( rStrm );
            importFontData5( rStrm );
            importFontName2( rStrm );
        break;
        case BIFF8:
            importFontData2( rStrm );
            importFontColor( rStrm );
            importFontData5( rStrm );
            importFontName8( rStrm );
        break;
        case BIFF_UNKNOWN: break;
    }
}

void Font::importFontColor( BiffInputStream& rStrm )
{
    OSL_ENSURE( !mbDxf, "Font::importFontColor - unexpected conditional formatting flag" );
    maModel.maColor.importColorId( rStrm );
}

void Font::importCfRule( BiffInputStream& rStrm )
{
    OSL_ENSURE( mbDxf, "Font::importCfRule - missing conditional formatting flag" );

    sal_Int32 nHeight, nColor;
    sal_uInt32 nStyle, nFontFlags1, nFontFlags2, nFontFlags3;
    sal_uInt16 nWeight, nEscapement;
    sal_uInt8 nUnderline;

    OSL_ENSURE( rStrm.getRemaining() >= 118, "Font::importCfRule - missing record data" );
    sal_Int64 nRecPos = rStrm.tell();
    maModel.maName = rStrm.readUniStringBody( rStrm.readuInt8() );
    maUsedFlags.mbNameUsed = maModel.maName.getLength() > 0;
    OSL_ENSURE( !rStrm.isEof() && (rStrm.tell() <= nRecPos + 64), "Font::importCfRule - font name too long" );
    rStrm.seek( nRecPos + 64 );
    rStrm >> nHeight >> nStyle >> nWeight >> nEscapement >> nUnderline;
    rStrm.skip( 3 );
    rStrm >> nColor;
    rStrm.skip( 4 );
    rStrm >> nFontFlags1 >> nFontFlags2 >> nFontFlags3;
    rStrm.skip( 18 );

    if( (maUsedFlags.mbColorUsed = (0 <= nColor) && (nColor <= 0x7FFF)) == true )
        maModel.maColor.setIndexed( nColor );
    if( (maUsedFlags.mbHeightUsed = (0 < nHeight) && (nHeight <= 0x7FFF)) == true )
        maModel.setBiffHeight( static_cast< sal_uInt16 >( nHeight ) );
    if( (maUsedFlags.mbUnderlineUsed = !getFlag( nFontFlags3, BIFF_CFRULE_FONT_UNDERL )) == true )
        maModel.setBiffUnderline( nUnderline );
    if( (maUsedFlags.mbEscapementUsed = !getFlag( nFontFlags2, BIFF_CFRULE_FONT_ESCAPEM )) == true )
        maModel.setBiffEscapement( nEscapement );
    if( (maUsedFlags.mbWeightUsed = maUsedFlags.mbPostureUsed = !getFlag( nFontFlags1, BIFF_CFRULE_FONT_STYLE )) == true )
    {
        maModel.setBiffWeight( nWeight );
        maModel.mbItalic = getFlag( nStyle, BIFF_CFRULE_FONT_STYLE );
    }
    if( (maUsedFlags.mbStrikeoutUsed = !getFlag( nFontFlags1, BIFF_CFRULE_FONT_STRIKEOUT )) == true )
        maModel.mbStrikeout = getFlag( nStyle, BIFF_CFRULE_FONT_STRIKEOUT );
    if( (maUsedFlags.mbOutlineUsed = !getFlag( nFontFlags1, BIFF_CFRULE_FONT_OUTLINE )) == true )
        maModel.mbOutline = getFlag( nStyle, BIFF_CFRULE_FONT_OUTLINE );
    if( (maUsedFlags.mbShadowUsed = !getFlag( nFontFlags1, BIFF_CFRULE_FONT_SHADOW )) == true )
        maModel.mbShadow = getFlag( nStyle, BIFF_CFRULE_FONT_SHADOW );
}

rtl_TextEncoding Font::getFontEncoding() const
{
    // #i63105# cells use text encoding from FONT record character set
    // #i67768# BIFF2-BIFF4 FONT records do not contain character set
    // #i71033# do not use maApiData, this function is used before finalizeImport()
    rtl_TextEncoding eFontEnc = RTL_TEXTENCODING_DONTKNOW;
    if( (0 <= maModel.mnCharSet) && (maModel.mnCharSet <= SAL_MAX_UINT8) )
        eFontEnc = rtl_getTextEncodingFromWindowsCharset( static_cast< sal_uInt8 >( maModel.mnCharSet ) );
    return (eFontEnc == RTL_TEXTENCODING_DONTKNOW) ? getTextEncoding() : eFontEnc;
}

void Font::finalizeImport()
{
    namespace cssawt = ::com::sun::star::awt;

    // font name
    maApiData.maDesc.Name = maModel.maName;

    // font family
    switch( maModel.mnFamily )
    {
        case OOX_FONTFAMILY_NONE:           maApiData.maDesc.Family = cssawt::FontFamily::DONTKNOW;     break;
        case OOX_FONTFAMILY_ROMAN:          maApiData.maDesc.Family = cssawt::FontFamily::ROMAN;        break;
        case OOX_FONTFAMILY_SWISS:          maApiData.maDesc.Family = cssawt::FontFamily::SWISS;        break;
        case OOX_FONTFAMILY_MODERN:         maApiData.maDesc.Family = cssawt::FontFamily::MODERN;       break;
        case OOX_FONTFAMILY_SCRIPT:         maApiData.maDesc.Family = cssawt::FontFamily::SCRIPT;       break;
        case OOX_FONTFAMILY_DECORATIVE:     maApiData.maDesc.Family = cssawt::FontFamily::DECORATIVE;   break;
    }

    // character set (API font descriptor uses rtl_TextEncoding in member CharSet!)
    if( (0 <= maModel.mnCharSet) && (maModel.mnCharSet <= SAL_MAX_UINT8) )
        maApiData.maDesc.CharSet = static_cast< sal_Int16 >(
            rtl_getTextEncodingFromWindowsCharset( static_cast< sal_uInt8 >( maModel.mnCharSet ) ) );

    // color, height, weight, slant, strikeout, outline, shadow
    maApiData.mnColor          = maModel.maColor.getColor( getBaseFilter().getGraphicHelper() );
    maApiData.maDesc.Height    = static_cast< sal_Int16 >( maModel.mfHeight * 20.0 );
    maApiData.maDesc.Weight    = maModel.mbBold ? cssawt::FontWeight::BOLD : cssawt::FontWeight::NORMAL;
    maApiData.maDesc.Slant     = maModel.mbItalic ? cssawt::FontSlant_ITALIC : cssawt::FontSlant_NONE;
    maApiData.maDesc.Strikeout = maModel.mbStrikeout ? cssawt::FontStrikeout::SINGLE : cssawt::FontStrikeout::NONE;
    maApiData.mbOutline        = maModel.mbOutline;
    maApiData.mbShadow         = maModel.mbShadow;

    // underline
    switch( maModel.mnUnderline )
    {
        case XML_double:            maApiData.maDesc.Underline = cssawt::FontUnderline::DOUBLE; break;
        case XML_doubleAccounting:  maApiData.maDesc.Underline = cssawt::FontUnderline::DOUBLE; break;
        case XML_none:              maApiData.maDesc.Underline = cssawt::FontUnderline::NONE;   break;
        case XML_single:            maApiData.maDesc.Underline = cssawt::FontUnderline::SINGLE; break;
        case XML_singleAccounting:  maApiData.maDesc.Underline = cssawt::FontUnderline::SINGLE; break;
    }

    // escapement
    switch( maModel.mnEscapement )
    {
        case XML_baseline:
            maApiData.mnEscapement = API_ESCAPE_NONE;
            maApiData.mnEscapeHeight = API_ESCAPEHEIGHT_NONE;
        break;
        case XML_superscript:
            maApiData.mnEscapement = API_ESCAPE_SUPERSCRIPT;
            maApiData.mnEscapeHeight = API_ESCAPEHEIGHT_DEFAULT;
        break;
        case XML_subscript:
            maApiData.mnEscapement = API_ESCAPE_SUBSCRIPT;
            maApiData.mnEscapeHeight = API_ESCAPEHEIGHT_DEFAULT;
        break;
    }

    // supported script types
    if( maUsedFlags.mbNameUsed )
    {
        Reference< XDevice > xDevice = getReferenceDevice();
        if( xDevice.is() )
        {
            Reference< XFont2 > xFont( xDevice->getFont( maApiData.maDesc ), UNO_QUERY );
            if( xFont.is() )
            {
                // #91658# CJK fonts
                bool bHasAsian =
                    xFont->hasGlyphs( OUString( sal_Unicode( 0x3041 ) ) ) ||    // 3040-309F: Hiragana
                    xFont->hasGlyphs( OUString( sal_Unicode( 0x30A1 ) ) ) ||    // 30A0-30FF: Katakana
                    xFont->hasGlyphs( OUString( sal_Unicode( 0x3111 ) ) ) ||    // 3100-312F: Bopomofo
                    xFont->hasGlyphs( OUString( sal_Unicode( 0x3131 ) ) ) ||    // 3130-318F: Hangul Compatibility Jamo
                    xFont->hasGlyphs( OUString( sal_Unicode( 0x3301 ) ) ) ||    // 3300-33FF: CJK Compatibility
                    xFont->hasGlyphs( OUString( sal_Unicode( 0x3401 ) ) ) ||    // 3400-4DBF: CJK Unified Ideographs Extension A
                    xFont->hasGlyphs( OUString( sal_Unicode( 0x4E01 ) ) ) ||    // 4E00-9FAF: CJK Unified Ideographs
                    xFont->hasGlyphs( OUString( sal_Unicode( 0x7E01 ) ) ) ||    // 4E00-9FAF: CJK unified ideographs
                    xFont->hasGlyphs( OUString( sal_Unicode( 0xA001 ) ) ) ||    // A001-A48F: Yi Syllables
                    xFont->hasGlyphs( OUString( sal_Unicode( 0xAC01 ) ) ) ||    // AC00-D7AF: Hangul Syllables
                    xFont->hasGlyphs( OUString( sal_Unicode( 0xCC01 ) ) ) ||    // AC00-D7AF: Hangul Syllables
                    xFont->hasGlyphs( OUString( sal_Unicode( 0xF901 ) ) ) ||    // F900-FAFF: CJK Compatibility Ideographs
                    xFont->hasGlyphs( OUString( sal_Unicode( 0xFF71 ) ) );      // FF00-FFEF: Halfwidth/Fullwidth Forms
                // #113783# CTL fonts
                bool bHasCmplx =
                    xFont->hasGlyphs( OUString( sal_Unicode( 0x05D1 ) ) ) ||    // 0590-05FF: Hebrew
                    xFont->hasGlyphs( OUString( sal_Unicode( 0x0631 ) ) ) ||    // 0600-06FF: Arabic
                    xFont->hasGlyphs( OUString( sal_Unicode( 0x0721 ) ) ) ||    // 0700-074F: Syriac
                    xFont->hasGlyphs( OUString( sal_Unicode( 0x0911 ) ) ) ||    // 0900-0DFF: Indic scripts
                    xFont->hasGlyphs( OUString( sal_Unicode( 0x0E01 ) ) ) ||    // 0E00-0E7F: Thai
                    xFont->hasGlyphs( OUString( sal_Unicode( 0xFB21 ) ) ) ||    // FB1D-FB4F: Hebrew Presentation Forms
                    xFont->hasGlyphs( OUString( sal_Unicode( 0xFB51 ) ) ) ||    // FB50-FDFF: Arabic Presentation Forms-A
                    xFont->hasGlyphs( OUString( sal_Unicode( 0xFE71 ) ) );      // FE70-FEFF: Arabic Presentation Forms-B
                // Western fonts
                bool bHasLatin =
                    (!bHasAsian && !bHasCmplx) ||
                    xFont->hasGlyphs( OUString( sal_Unicode( 'A' ) ) );

                lclSetFontName( maApiData.maLatinFont, maApiData.maDesc, bHasLatin );
                lclSetFontName( maApiData.maAsianFont, maApiData.maDesc, bHasAsian );
                lclSetFontName( maApiData.maCmplxFont, maApiData.maDesc, bHasCmplx );
            }
        }
    }
}

const FontDescriptor& Font::getFontDescriptor() const
{
    return maApiData.maDesc;
}

bool Font::needsRichTextFormat() const
{
    return maApiData.mnEscapement != API_ESCAPE_NONE;
}

void Font::writeToPropertyMap( PropertyMap& rPropMap, FontPropertyType ePropType ) const
{
    // font name properties
    if( maUsedFlags.mbNameUsed )
    {
        if( maApiData.maLatinFont.maName.getLength() > 0 )
        {
            rPropMap[ PROP_CharFontName ]    <<= maApiData.maLatinFont.maName;
            rPropMap[ PROP_CharFontFamily ]  <<= maApiData.maLatinFont.mnFamily;
            rPropMap[ PROP_CharFontCharSet ] <<= maApiData.maLatinFont.mnTextEnc;
        }
        if( maApiData.maAsianFont.maName.getLength() > 0 )
        {
            rPropMap[ PROP_CharFontNameAsian ]    <<= maApiData.maAsianFont.maName;
            rPropMap[ PROP_CharFontFamilyAsian ]  <<= maApiData.maAsianFont.mnFamily;
            rPropMap[ PROP_CharFontCharSetAsian ] <<= maApiData.maAsianFont.mnTextEnc;
        }
        if( maApiData.maCmplxFont.maName.getLength() > 0 )
        {
            rPropMap[ PROP_CharFontNameComplex ]    <<= maApiData.maCmplxFont.maName;
            rPropMap[ PROP_CharFontFamilyComplex ]  <<= maApiData.maCmplxFont.mnFamily;
            rPropMap[ PROP_CharFontCharSetComplex ] <<= maApiData.maCmplxFont.mnTextEnc;
        }
    }
    // font height
    if( maUsedFlags.mbHeightUsed )
    {
        float fHeight = static_cast< float >( maApiData.maDesc.Height / 20.0 ); // twips to points
        rPropMap[ PROP_CharHeight ] <<= fHeight;
        rPropMap[ PROP_CharHeightAsian ] <<= fHeight;
        rPropMap[ PROP_CharHeightComplex ] <<= fHeight;
    }
    // font weight
    if( maUsedFlags.mbWeightUsed )
    {
        float fWeight = maApiData.maDesc.Weight;
        rPropMap[ PROP_CharWeight ] <<= fWeight;
        rPropMap[ PROP_CharWeightAsian ] <<= fWeight;
        rPropMap[ PROP_CharWeightComplex ] <<= fWeight;
    }
    // font posture
    if( maUsedFlags.mbPostureUsed )
    {
        rPropMap[ PROP_CharPosture ] <<= maApiData.maDesc.Slant;
        rPropMap[ PROP_CharPostureAsian ] <<= maApiData.maDesc.Slant;
        rPropMap[ PROP_CharPostureComplex ] <<= maApiData.maDesc.Slant;
    }
    // character color
    if( maUsedFlags.mbColorUsed )
        rPropMap[ PROP_CharColor ] <<= maApiData.mnColor;
    // underline style
    if( maUsedFlags.mbUnderlineUsed )
        rPropMap[ PROP_CharUnderline ] <<= maApiData.maDesc.Underline;
    // strike out style
    if( maUsedFlags.mbStrikeoutUsed )
        rPropMap[ PROP_CharStrikeout ] <<= maApiData.maDesc.Strikeout;
    // outline style
    if( maUsedFlags.mbOutlineUsed )
        rPropMap[ PROP_CharContoured ] <<= maApiData.mbOutline;
    // shadow style
    if( maUsedFlags.mbShadowUsed )
        rPropMap[ PROP_CharShadowed ] <<= maApiData.mbShadow;
    // escapement
    if( maUsedFlags.mbEscapementUsed && (ePropType == FONT_PROPTYPE_TEXT) )
    {
        rPropMap[ PROP_CharEscapement ] <<= maApiData.mnEscapement;
        rPropMap[ PROP_CharEscapementHeight ] <<= maApiData.mnEscapeHeight;
    }
}

void Font::writeToPropertySet( PropertySet& rPropSet, FontPropertyType ePropType ) const
{
    PropertyMap aPropMap;
    writeToPropertyMap( aPropMap, ePropType );
    rPropSet.setProperties( aPropMap );
}

void Font::importFontData2( BiffInputStream& rStrm )
{
    sal_uInt16 nHeight, nFlags;
    rStrm >> nHeight >> nFlags;

    maModel.setBiffHeight( nHeight );
    maModel.mnFamily     = OOX_FONTFAMILY_NONE;
    maModel.mnCharSet    = -1;    // ensure to not use font charset in byte string import
    maModel.mnUnderline  = getFlagValue( nFlags, BIFF_FONTFLAG_UNDERLINE, XML_single, XML_none );
    maModel.mnEscapement = XML_none;
    maModel.mbBold       = getFlag( nFlags, BIFF_FONTFLAG_BOLD );
    maModel.mbItalic     = getFlag( nFlags, BIFF_FONTFLAG_ITALIC );
    maModel.mbStrikeout  = getFlag( nFlags, BIFF_FONTFLAG_STRIKEOUT );
    maModel.mbOutline    = getFlag( nFlags, BIFF_FONTFLAG_OUTLINE );
    maModel.mbShadow     = getFlag( nFlags, BIFF_FONTFLAG_SHADOW );
}

void Font::importFontData5( BiffInputStream& rStrm )
{
    sal_uInt16 nWeight, nEscapement;
    sal_uInt8 nUnderline, nFamily, nCharSet;
    rStrm >> nWeight >> nEscapement >> nUnderline >> nFamily >> nCharSet;
    rStrm.skip( 1 );

    maModel.setBiffWeight( nWeight );
    maModel.setBiffUnderline( nUnderline );
    maModel.setBiffEscapement( nEscapement );
    // equal constants in XML and BIFF for family and charset
    maModel.mnFamily  = nFamily;
    maModel.mnCharSet = nCharSet;
}

void Font::importFontName2( BiffInputStream& rStrm )
{
    maModel.maName = rStrm.readByteStringUC( false, getTextEncoding() );
}

void Font::importFontName8( BiffInputStream& rStrm )
{
    maModel.maName = rStrm.readUniStringBody( rStrm.readuInt8() );
}

// ============================================================================

AlignmentModel::AlignmentModel() :
    mnHorAlign( XML_general ),
    mnVerAlign( XML_bottom ),
    mnTextDir( OOX_XF_TEXTDIR_CONTEXT ),
    mnRotation( OOX_XF_ROTATION_NONE ),
    mnIndent( OOX_XF_INDENT_NONE ),
    mbWrapText( false ),
    mbShrink( false ),
    mbJustLastLine( false )
{
}

void AlignmentModel::setBinHorAlign( sal_uInt8 nHorAlign )
{
    static const sal_Int32 spnHorAligns[] = {
        XML_general, XML_left, XML_center, XML_right,
        XML_fill, XML_justify, XML_centerContinuous, XML_distributed };
    mnHorAlign = STATIC_ARRAY_SELECT( spnHorAligns, nHorAlign, XML_general );
}

void AlignmentModel::setBinVerAlign( sal_uInt8 nVerAlign )
{
    static const sal_Int32 spnVerAligns[] = {
        XML_top, XML_center, XML_bottom, XML_justify, XML_distributed };
    mnVerAlign = STATIC_ARRAY_SELECT( spnVerAligns, nVerAlign, XML_bottom );
}

void AlignmentModel::setBinTextOrient( sal_uInt8 nTextOrient )
{
    static const sal_Int32 spnRotations[] = {
        OOX_XF_ROTATION_NONE, OOX_XF_ROTATION_STACKED,
        OOX_XF_ROTATION_90CCW, OOX_XF_ROTATION_90CW };
    mnRotation = STATIC_ARRAY_SELECT( spnRotations, nTextOrient, OOX_XF_ROTATION_NONE );
}

// ----------------------------------------------------------------------------

ApiAlignmentData::ApiAlignmentData() :
    meHorJustify( ::com::sun::star::table::CellHoriJustify_STANDARD ),
    mnHorJustifyMethod( ::com::sun::star::table::CellJustifyMethod::AUTO ),
    mnVerJustify( ::com::sun::star::table::CellVertJustify2::STANDARD ),
    mnVerJustifyMethod( ::com::sun::star::table::CellJustifyMethod::AUTO ),
    meOrientation( ::com::sun::star::table::CellOrientation_STANDARD ),
    mnRotation( 0 ),
    mnWritingMode( ::com::sun::star::text::WritingMode2::PAGE ),
    mnIndent( 0 ),
    mbWrapText( false ),
    mbShrink( false )
{
}

bool operator==( const ApiAlignmentData& rLeft, const ApiAlignmentData& rRight )
{
    return
        (rLeft.meHorJustify  == rRight.meHorJustify) &&
        (rLeft.mnHorJustifyMethod == rRight.mnHorJustifyMethod) &&
        (rLeft.mnVerJustify  == rRight.mnVerJustify) &&
        (rLeft.mnVerJustifyMethod == rRight.mnVerJustifyMethod) &&
        (rLeft.meOrientation == rRight.meOrientation) &&
        (rLeft.mnRotation    == rRight.mnRotation) &&
        (rLeft.mnWritingMode == rRight.mnWritingMode) &&
        (rLeft.mnIndent      == rRight.mnIndent) &&
        (rLeft.mbWrapText    == rRight.mbWrapText) &&
        (rLeft.mbShrink      == rRight.mbShrink);
}

// ============================================================================

Alignment::Alignment( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
}

void Alignment::importAlignment( const AttributeList& rAttribs )
{
    maModel.mnHorAlign     = rAttribs.getToken( XML_horizontal, XML_general );
    maModel.mnVerAlign     = rAttribs.getToken( XML_vertical, XML_bottom );
    maModel.mnTextDir      = rAttribs.getInteger( XML_readingOrder, OOX_XF_TEXTDIR_CONTEXT );
    maModel.mnRotation     = rAttribs.getInteger( XML_textRotation, OOX_XF_ROTATION_NONE );
    maModel.mnIndent       = rAttribs.getInteger( XML_indent, OOX_XF_INDENT_NONE );
    maModel.mbWrapText     = rAttribs.getBool( XML_wrapText, false );
    maModel.mbShrink       = rAttribs.getBool( XML_shrinkToFit, false );
    maModel.mbJustLastLine = rAttribs.getBool( XML_justifyLastLine, false );
}

void Alignment::setBinData( sal_uInt32 nFlags )
{
    maModel.setBinHorAlign( extractValue< sal_uInt8 >( nFlags, 16, 3 ) );
    maModel.setBinVerAlign( extractValue< sal_uInt8 >( nFlags, 19, 3 ) );
    maModel.mnTextDir      = extractValue< sal_Int32 >( nFlags, 26, 2 );
    maModel.mnRotation     = extractValue< sal_Int32 >( nFlags, 0, 8 );
    maModel.mnIndent       = extractValue< sal_uInt8 >( nFlags, 8, 8 );
    maModel.mbWrapText     = getFlag( nFlags, OOBIN_XF_WRAPTEXT );
    maModel.mbShrink       = getFlag( nFlags, OOBIN_XF_SHRINK );
    maModel.mbJustLastLine = getFlag( nFlags, OOBIN_XF_JUSTLASTLINE );
}

void Alignment::setBiff2Data( sal_uInt8 nFlags )
{
    maModel.setBinHorAlign( extractValue< sal_uInt8 >( nFlags, 0, 3 ) );
}

void Alignment::setBiff3Data( sal_uInt16 nAlign )
{
    maModel.setBinHorAlign( extractValue< sal_uInt8 >( nAlign, 0, 3 ) );
    maModel.mbWrapText = getFlag( nAlign, BIFF_XF_WRAPTEXT ); // new in BIFF3
}

void Alignment::setBiff4Data( sal_uInt16 nAlign )
{
    maModel.setBinHorAlign( extractValue< sal_uInt8 >( nAlign, 0, 3 ) );
    maModel.setBinVerAlign( extractValue< sal_uInt8 >( nAlign, 4, 2 ) ); // new in BIFF4
    maModel.setBinTextOrient( extractValue< sal_uInt8 >( nAlign, 6, 2 ) ); // new in BIFF4
    maModel.mbWrapText = getFlag( nAlign, BIFF_XF_WRAPTEXT );
}

void Alignment::setBiff5Data( sal_uInt16 nAlign )
{
    maModel.setBinHorAlign( extractValue< sal_uInt8 >( nAlign, 0, 3 ) );
    maModel.setBinVerAlign( extractValue< sal_uInt8 >( nAlign, 4, 3 ) );
    maModel.setBinTextOrient( extractValue< sal_uInt8 >( nAlign, 8, 2 ) );
    maModel.mbWrapText = getFlag( nAlign, BIFF_XF_WRAPTEXT );
}

void Alignment::setBiff8Data( sal_uInt16 nAlign, sal_uInt16 nMiscAttrib )
{
    maModel.setBinHorAlign( extractValue< sal_uInt8 >( nAlign, 0, 3 ) );
    maModel.setBinVerAlign( extractValue< sal_uInt8 >( nAlign, 4, 3 ) );
    maModel.mnTextDir      = extractValue< sal_Int32 >( nMiscAttrib, 6, 2 ); // new in BIFF8
    maModel.mnRotation     = extractValue< sal_Int32 >( nAlign, 8, 8 ); // new in BIFF8
    maModel.mnIndent       = extractValue< sal_uInt8 >( nMiscAttrib, 0, 4 ); // new in BIFF8
    maModel.mbWrapText     = getFlag( nAlign, BIFF_XF_WRAPTEXT );
    maModel.mbShrink       = getFlag( nMiscAttrib, BIFF_XF_SHRINK ); // new in BIFF8
    maModel.mbJustLastLine = getFlag( nAlign, BIFF_XF_JUSTLASTLINE ); // new in BIFF8(?)
}

void Alignment::finalizeImport()
{
    namespace csstab = ::com::sun::star::table;
    namespace csstxt = ::com::sun::star::text;

    // horizontal alignment
    switch( maModel.mnHorAlign )
    {
        case XML_center:            maApiData.meHorJustify = csstab::CellHoriJustify_CENTER;    break;
        case XML_centerContinuous:  maApiData.meHorJustify = csstab::CellHoriJustify_CENTER;    break;
        case XML_distributed:       maApiData.meHorJustify = csstab::CellHoriJustify_BLOCK;     break;
        case XML_fill:              maApiData.meHorJustify = csstab::CellHoriJustify_REPEAT;    break;
        case XML_general:           maApiData.meHorJustify = csstab::CellHoriJustify_STANDARD;  break;
        case XML_justify:           maApiData.meHorJustify = csstab::CellHoriJustify_BLOCK;     break;
        case XML_left:              maApiData.meHorJustify = csstab::CellHoriJustify_LEFT;      break;
        case XML_right:             maApiData.meHorJustify = csstab::CellHoriJustify_RIGHT;     break;
    }

    if (maModel.mnHorAlign == XML_distributed)
        maApiData.mnHorJustifyMethod = csstab::CellJustifyMethod::DISTRIBUTE;

    // vertical alignment
    switch( maModel.mnVerAlign )
    {
        case XML_bottom:        maApiData.mnVerJustify = csstab::CellVertJustify2::BOTTOM;    break;
        case XML_center:        maApiData.mnVerJustify = csstab::CellVertJustify2::CENTER;    break;
        case XML_distributed:   maApiData.mnVerJustify = csstab::CellVertJustify2::BLOCK;     break;
        case XML_justify:       maApiData.mnVerJustify = csstab::CellVertJustify2::BLOCK;     break;
        case XML_top:           maApiData.mnVerJustify = csstab::CellVertJustify2::TOP;       break;
    }

    if (maModel.mnVerAlign == XML_distributed)
        maApiData.mnVerJustifyMethod = csstab::CellJustifyMethod::DISTRIBUTE;

    /*  indentation: expressed as number of blocks of 3 space characters in
        OOX, and as multiple of 10 points in BIFF. */
    sal_Int32 nIndent = 0;
    switch( getFilterType() )
    {
        case FILTER_OOX:    nIndent = getUnitConverter().scaleToMm100( 3.0 * maModel.mnIndent, UNIT_SPACE );  break;
        case FILTER_BIFF:   nIndent = getUnitConverter().scaleToMm100( 10.0 * maModel.mnIndent, UNIT_POINT ); break;
        case FILTER_UNKNOWN: break;
    }
    if( (0 <= nIndent) && (nIndent <= SAL_MAX_INT16) )
        maApiData.mnIndent = static_cast< sal_Int16 >( nIndent );

    // complex text direction
    switch( maModel.mnTextDir )
    {
        case OOX_XF_TEXTDIR_CONTEXT:    maApiData.mnWritingMode = csstxt::WritingMode2::PAGE;   break;
        case OOX_XF_TEXTDIR_LTR:        maApiData.mnWritingMode = csstxt::WritingMode2::LR_TB;  break;
        case OOX_XF_TEXTDIR_RTL:        maApiData.mnWritingMode = csstxt::WritingMode2::RL_TB;  break;
    }

    // rotation: 0-90 means 0 to 90 degrees ccw, 91-180 means 1 to 90 degrees cw, 255 means stacked
    sal_Int32 nOoxRot = maModel.mnRotation;
    maApiData.mnRotation = ((0 <= nOoxRot) && (nOoxRot <= 90)) ?
        (100 * nOoxRot) :
        (((91 <= nOoxRot) && (nOoxRot <= 180)) ? (100 * (450 - nOoxRot)) : 0);

    // "Orientation" property used for character stacking
    maApiData.meOrientation = (nOoxRot == OOX_XF_ROTATION_STACKED) ?
        csstab::CellOrientation_STACKED : csstab::CellOrientation_STANDARD;

    // alignment flags (#i84960 automatic line break, if vertically justified/distributed)
    maApiData.mbWrapText = maModel.mbWrapText || (maModel.mnVerAlign == XML_distributed) || (maModel.mnVerAlign == XML_justify);
    maApiData.mbShrink = maModel.mbShrink;

}

void Alignment::writeToPropertyMap( PropertyMap& rPropMap ) const
{
    rPropMap[ PROP_HoriJustify ]     <<= maApiData.meHorJustify;
    rPropMap[ PROP_HoriJustifyMethod ] <<= maApiData.mnHorJustifyMethod;
    rPropMap[ PROP_VertJustify ]     <<= maApiData.mnVerJustify;
    rPropMap[ PROP_VertJustifyMethod ] <<= maApiData.mnVerJustifyMethod;
    rPropMap[ PROP_WritingMode ]     <<= maApiData.mnWritingMode;
    rPropMap[ PROP_RotateAngle ]     <<= maApiData.mnRotation;
    rPropMap[ PROP_Orientation ]     <<= maApiData.meOrientation;
    rPropMap[ PROP_ParaIndent ]      <<= maApiData.mnIndent;
    rPropMap[ PROP_IsTextWrapped ]   <<= maApiData.mbWrapText;
    rPropMap[ PROP_ShrinkToFit ]     <<= maApiData.mbShrink;
}

// ============================================================================

ProtectionModel::ProtectionModel() :
    mbLocked( true ),   // default in Excel and Calc
    mbHidden( false )
{
}

// ----------------------------------------------------------------------------

ApiProtectionData::ApiProtectionData() :
    maCellProt( sal_True, sal_False, sal_False, sal_False )
{
}

bool operator==( const ApiProtectionData& rLeft, const ApiProtectionData& rRight )
{
    return
        (rLeft.maCellProt.IsLocked        == rRight.maCellProt.IsLocked) &&
        (rLeft.maCellProt.IsFormulaHidden == rRight.maCellProt.IsFormulaHidden) &&
        (rLeft.maCellProt.IsHidden        == rRight.maCellProt.IsHidden) &&
        (rLeft.maCellProt.IsPrintHidden   == rRight.maCellProt.IsPrintHidden);
}

// ============================================================================

Protection::Protection( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
}

void Protection::importProtection( const AttributeList& rAttribs )
{
    maModel.mbLocked = rAttribs.getBool( XML_locked, true );
    maModel.mbHidden = rAttribs.getBool( XML_hidden, false );
}

void Protection::setBinData( sal_uInt32 nFlags )
{
    maModel.mbLocked = getFlag( nFlags, OOBIN_XF_LOCKED );
    maModel.mbHidden = getFlag( nFlags, OOBIN_XF_HIDDEN );
}

void Protection::setBiff2Data( sal_uInt8 nNumFmt )
{
    maModel.mbLocked = getFlag( nNumFmt, BIFF2_XF_LOCKED );
    maModel.mbHidden = getFlag( nNumFmt, BIFF2_XF_HIDDEN );
}

void Protection::setBiff3Data( sal_uInt16 nProt )
{
    maModel.mbLocked = getFlag( nProt, BIFF_XF_LOCKED );
    maModel.mbHidden = getFlag( nProt, BIFF_XF_HIDDEN );
}

void Protection::finalizeImport()
{
    maApiData.maCellProt.IsLocked = maModel.mbLocked;
    maApiData.maCellProt.IsFormulaHidden = maModel.mbHidden;
}

void Protection::writeToPropertyMap( PropertyMap& rPropMap ) const
{
    rPropMap[ PROP_CellProtection ] <<= maApiData.maCellProt;
}

// ============================================================================

namespace {

bool lcl_isBorder(const ::com::sun::star::table::BorderLine& rBorder)
{
    return (rBorder.InnerLineWidth > 0) || (rBorder.OuterLineWidth > 0);
}

}

BorderLineModel::BorderLineModel( bool bDxf ) :
    mnStyle( XML_none ),
    mbUsed( !bDxf )
{
    maColor.setIndexed( OOX_COLOR_WINDOWTEXT );
}

void BorderLineModel::setBiffStyle( sal_Int32 nLineStyle )
{
    static const sal_Int32 spnStyleIds[] = {
        XML_none, XML_thin, XML_medium, XML_dashed,
        XML_dotted, XML_thick, XML_double, XML_hair,
        XML_mediumDashed, XML_dashDot, XML_mediumDashDot, XML_dashDotDot,
        XML_mediumDashDotDot, XML_slantDashDot };
    mnStyle = STATIC_ARRAY_SELECT( spnStyleIds, nLineStyle, XML_none );
}

void BorderLineModel::setBiffData( sal_uInt8 nLineStyle, sal_uInt16 nLineColor )
{
    maColor.setIndexed( nLineColor );
    setBiffStyle( nLineStyle );
}

// ----------------------------------------------------------------------------

BorderModel::BorderModel( bool bDxf ) :
    maLeft( bDxf ),
    maRight( bDxf ),
    maTop( bDxf ),
    maBottom( bDxf ),
    maDiagonal( bDxf ),
    mbDiagTLtoBR( false ),
    mbDiagBLtoTR( false )
{
}

// ----------------------------------------------------------------------------

ApiBorderData::ApiBorderData() :
    mbBorderUsed( false ),
    mbDiagUsed( false )
{
}

bool ApiBorderData::hasAnyOuterBorder() const
{
    return
        ( ( lcl_isBorder( maTop ) &&  maTop.OuterLineWidth > 0 ) ) ||
        ( ( lcl_isBorder( maBottom ) && maBottom.OuterLineWidth > 0 ) ) ||
        ( ( lcl_isBorder( maLeft ) && maLeft.OuterLineWidth > 0 ) ) ||
        ( ( lcl_isBorder( maRight ) && maRight.OuterLineWidth > 0 ) );
}

namespace {

bool operator==( const BorderLine& rLeft, const BorderLine& rRight )
{
    return
        (rLeft.Color          == rRight.Color) &&
        (rLeft.InnerLineWidth == rRight.InnerLineWidth) &&
        (rLeft.OuterLineWidth == rRight.OuterLineWidth) &&
        (rLeft.LineDistance   == rRight.LineDistance);
}

bool operator==( const TableBorder& rLeft, const TableBorder& rRight )
{
    return
        (rLeft.TopLine               == rRight.TopLine) &&
        (rLeft.IsTopLineValid        == rRight.IsTopLineValid) &&
        (rLeft.BottomLine            == rRight.BottomLine) &&
        (rLeft.IsBottomLineValid     == rRight.IsBottomLineValid) &&
        (rLeft.LeftLine              == rRight.LeftLine) &&
        (rLeft.IsLeftLineValid       == rRight.IsLeftLineValid) &&
        (rLeft.RightLine             == rRight.RightLine) &&
        (rLeft.IsRightLineValid      == rRight.IsRightLineValid) &&
        (rLeft.HorizontalLine        == rRight.HorizontalLine) &&
        (rLeft.IsHorizontalLineValid == rRight.IsHorizontalLineValid) &&
        (rLeft.VerticalLine          == rRight.VerticalLine) &&
        (rLeft.IsVerticalLineValid   == rRight.IsVerticalLineValid) &&
        (rLeft.Distance              == rRight.Distance) &&
        (rLeft.IsDistanceValid       == rRight.IsDistanceValid);
}

} // namespace

bool operator==( const ApiBorderData& rLeft, const ApiBorderData& rRight )
{
    return
        (rLeft.maLeft       == rRight.maLeft)   &&
        (rLeft.maRight      == rRight.maRight)  &&
        (rLeft.maTop        == rRight.maTop)    &&
        (rLeft.maBottom     == rRight.maBottom) &&
        (rLeft.maTLtoBR     == rRight.maTLtoBR) &&
        (rLeft.maBLtoTR     == rRight.maBLtoTR) &&
        (rLeft.mbBorderUsed == rRight.mbBorderUsed) &&
        (rLeft.mbDiagUsed   == rRight.mbDiagUsed);
}

// ============================================================================

namespace {

inline void lclSetBorderLineWidth( BorderLine& rBorderLine,
        sal_Int16 nOuter, sal_Int16 nDist = API_LINE_NONE, sal_Int16 nInner = API_LINE_NONE )
{
    rBorderLine.OuterLineWidth = nOuter;
    rBorderLine.LineDistance = nDist;
    rBorderLine.InnerLineWidth = nInner;
}

inline sal_Int32 lclGetBorderLineWidth( const BorderLine& rBorderLine )
{
    return rBorderLine.OuterLineWidth + rBorderLine.LineDistance + rBorderLine.InnerLineWidth;
}

const BorderLine2* lclGetThickerLine( const BorderLine2& rBorderLine1, sal_Bool bValid1, const BorderLine2& rBorderLine2, sal_Bool bValid2 )
{
    if( bValid1 && bValid2 )
        return (lclGetBorderLineWidth( rBorderLine1 ) < lclGetBorderLineWidth( rBorderLine2 )) ? &rBorderLine2 : &rBorderLine1;
    if( bValid1 )
        return &rBorderLine1;
    if( bValid2 )
        return &rBorderLine2;
    return 0;
}

} // namespace

// ----------------------------------------------------------------------------

Border::Border( const WorkbookHelper& rHelper, bool bDxf ) :
    WorkbookHelper( rHelper ),
    maModel( bDxf ),
    mbDxf( bDxf )
{
}

void Border::importBorder( const AttributeList& rAttribs )
{
    maModel.mbDiagTLtoBR = rAttribs.getBool( XML_diagonalDown, false );
    maModel.mbDiagBLtoTR = rAttribs.getBool( XML_diagonalUp, false );
}

void Border::importStyle( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( BorderLineModel* pBorderLine = getBorderLine( nElement ) )
    {
        pBorderLine->mnStyle = rAttribs.getToken( XML_style, XML_none );
        pBorderLine->mbUsed = true;
    }
}

void Border::importColor( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( BorderLineModel* pBorderLine = getBorderLine( nElement ) )
        pBorderLine->maColor.importColor( rAttribs );
}

void Border::importBorder( RecordInputStream& rStrm )
{
    sal_uInt8 nFlags = rStrm.readuInt8();
    maModel.mbDiagTLtoBR = getFlag( nFlags, OOBIN_BORDER_DIAG_TLBR );
    maModel.mbDiagBLtoTR = getFlag( nFlags, OOBIN_BORDER_DIAG_BLTR );
    maModel.maTop.setBiffStyle( rStrm.readuInt16() );
    rStrm >> maModel.maTop.maColor;
    maModel.maBottom.setBiffStyle( rStrm.readuInt16() );
    rStrm >> maModel.maBottom.maColor;
    maModel.maLeft.setBiffStyle( rStrm.readuInt16() );
    rStrm >> maModel.maLeft.maColor;
    maModel.maRight.setBiffStyle( rStrm.readuInt16() );
    rStrm >> maModel.maRight.maColor;
    maModel.maDiagonal.setBiffStyle( rStrm.readuInt16() );
    rStrm >> maModel.maDiagonal.maColor;
}

void Border::importDxfBorder( sal_Int32 nElement, RecordInputStream& rStrm )
{
    OSL_ENSURE( mbDxf, "Border::importDxfBorder - missing conditional formatting flag" );
    if( BorderLineModel* pBorderLine = getBorderLine( nElement ) )
    {
        sal_uInt16 nStyle;
        rStrm >> pBorderLine->maColor >> nStyle;
        pBorderLine->setBiffStyle( nStyle );
        pBorderLine->mbUsed = true;
    }
}

void Border::setBiff2Data( sal_uInt8 nFlags )
{
    OSL_ENSURE( !mbDxf, "Border::setBiff2Data - unexpected conditional formatting flag" );
    maModel.maLeft.setBiffData(   getFlagValue( nFlags, BIFF2_XF_LEFTLINE,   BIFF_LINE_THIN, BIFF_LINE_NONE ), BIFF2_COLOR_BLACK );
    maModel.maRight.setBiffData(  getFlagValue( nFlags, BIFF2_XF_RIGHTLINE,  BIFF_LINE_THIN, BIFF_LINE_NONE ), BIFF2_COLOR_BLACK );
    maModel.maTop.setBiffData(    getFlagValue( nFlags, BIFF2_XF_TOPLINE,    BIFF_LINE_THIN, BIFF_LINE_NONE ), BIFF2_COLOR_BLACK );
    maModel.maBottom.setBiffData( getFlagValue( nFlags, BIFF2_XF_BOTTOMLINE, BIFF_LINE_THIN, BIFF_LINE_NONE ), BIFF2_COLOR_BLACK );
    maModel.maDiagonal.mbUsed = false;
}

void Border::setBiff3Data( sal_uInt32 nBorder )
{
    OSL_ENSURE( !mbDxf, "Border::setBiff3Data - unexpected conditional formatting flag" );
    maModel.maLeft.setBiffData(   extractValue< sal_uInt8 >( nBorder,  8, 3 ), extractValue< sal_uInt16 >( nBorder, 11, 5 ) );
    maModel.maRight.setBiffData(  extractValue< sal_uInt8 >( nBorder, 24, 3 ), extractValue< sal_uInt16 >( nBorder, 27, 5 ) );
    maModel.maTop.setBiffData(    extractValue< sal_uInt8 >( nBorder,  0, 3 ), extractValue< sal_uInt16 >( nBorder,  3, 5 ) );
    maModel.maBottom.setBiffData( extractValue< sal_uInt8 >( nBorder, 16, 3 ), extractValue< sal_uInt16 >( nBorder, 19, 5 ) );
    maModel.maDiagonal.mbUsed = false;
}

void Border::setBiff5Data( sal_uInt32 nBorder, sal_uInt32 nArea )
{
    OSL_ENSURE( !mbDxf, "Border::setBiff5Data - unexpected conditional formatting flag" );
    maModel.maLeft.setBiffData(   extractValue< sal_uInt8 >( nBorder,  3, 3 ), extractValue< sal_uInt16 >( nBorder, 16, 7 ) );
    maModel.maRight.setBiffData(  extractValue< sal_uInt8 >( nBorder,  6, 3 ), extractValue< sal_uInt16 >( nBorder, 23, 7 ) );
    maModel.maTop.setBiffData(    extractValue< sal_uInt8 >( nBorder,  0, 3 ), extractValue< sal_uInt16 >( nBorder,  9, 7 ) );
    maModel.maBottom.setBiffData( extractValue< sal_uInt8 >( nArea,   22, 3 ), extractValue< sal_uInt16 >( nArea,   25, 7 ) );
    maModel.maDiagonal.mbUsed = false;
}

void Border::setBiff8Data( sal_uInt32 nBorder1, sal_uInt32 nBorder2 )
{
    OSL_ENSURE( !mbDxf, "Border::setBiff8Data - unexpected conditional formatting flag" );
    maModel.maLeft.setBiffData(   extractValue< sal_uInt8 >( nBorder1,  0, 4 ), extractValue< sal_uInt16 >( nBorder1, 16, 7 ) );
    maModel.maRight.setBiffData(  extractValue< sal_uInt8 >( nBorder1,  4, 4 ), extractValue< sal_uInt16 >( nBorder1, 23, 7 ) );
    maModel.maTop.setBiffData(    extractValue< sal_uInt8 >( nBorder1,  8, 4 ), extractValue< sal_uInt16 >( nBorder2,  0, 7 ) );
    maModel.maBottom.setBiffData( extractValue< sal_uInt8 >( nBorder1, 12, 4 ), extractValue< sal_uInt16 >( nBorder2,  7, 7 ) );
    maModel.mbDiagTLtoBR = getFlag( nBorder1, BIFF_XF_DIAG_TLBR );
    maModel.mbDiagBLtoTR = getFlag( nBorder1, BIFF_XF_DIAG_BLTR );
    if( maModel.mbDiagTLtoBR || maModel.mbDiagBLtoTR )
        maModel.maDiagonal.setBiffData( extractValue< sal_uInt8 >( nBorder2, 21, 4 ), extractValue< sal_uInt16 >( nBorder2, 14, 7 ) );
}

void Border::importCfRule( BiffInputStream& rStrm, sal_uInt32 nFlags )
{
    OSL_ENSURE( mbDxf, "Border::importCfRule - missing conditional formatting flag" );
    OSL_ENSURE( getFlag( nFlags, BIFF_CFRULE_BORDERBLOCK ), "Border::importCfRule - missing border block flag" );
    sal_uInt16 nStyle;
    sal_uInt32 nColor;
    rStrm >> nStyle >> nColor;
    rStrm.skip( 2 );
    maModel.maLeft.setBiffData(   extractValue< sal_uInt8 >( nStyle,  0, 4 ), extractValue< sal_uInt16 >( nColor,  0, 7 ) );
    maModel.maRight.setBiffData(  extractValue< sal_uInt8 >( nStyle,  4, 4 ), extractValue< sal_uInt16 >( nColor,  7, 7 ) );
    maModel.maTop.setBiffData(    extractValue< sal_uInt8 >( nStyle,  8, 4 ), extractValue< sal_uInt16 >( nColor, 16, 7 ) );
    maModel.maBottom.setBiffData( extractValue< sal_uInt8 >( nStyle, 12, 4 ), extractValue< sal_uInt16 >( nColor, 23, 7 ) );
    maModel.maLeft.mbUsed   = !getFlag( nFlags, BIFF_CFRULE_BORDER_LEFT );
    maModel.maRight.mbUsed  = !getFlag( nFlags, BIFF_CFRULE_BORDER_RIGHT );
    maModel.maTop.mbUsed    = !getFlag( nFlags, BIFF_CFRULE_BORDER_TOP );
    maModel.maBottom.mbUsed = !getFlag( nFlags, BIFF_CFRULE_BORDER_BOTTOM );
}

void Border::finalizeImport()
{
    maApiData.mbBorderUsed = maModel.maLeft.mbUsed || maModel.maRight.mbUsed || maModel.maTop.mbUsed || maModel.maBottom.mbUsed;
    maApiData.mbDiagUsed   = maModel.maDiagonal.mbUsed;

    convertBorderLine( maApiData.maLeft,   maModel.maLeft );
    convertBorderLine( maApiData.maRight,  maModel.maRight );
    convertBorderLine( maApiData.maTop,    maModel.maTop );
    convertBorderLine( maApiData.maBottom, maModel.maBottom );

    if( maModel.mbDiagTLtoBR )
        convertBorderLine( maApiData.maTLtoBR, maModel.maDiagonal );
    if( maModel.mbDiagBLtoTR )
        convertBorderLine( maApiData.maBLtoTR, maModel.maDiagonal );
}

void Border::writeToPropertyMap( PropertyMap& rPropMap ) const
{
    if( maApiData.mbBorderUsed )
    {
        rPropMap[ PROP_LeftBorder ]   <<= maApiData.maLeft;
        rPropMap[ PROP_RightBorder ]  <<= maApiData.maRight;
        rPropMap[ PROP_TopBorder ]    <<= maApiData.maTop;
        rPropMap[ PROP_BottomBorder ] <<= maApiData.maBottom;
    }
    if( maApiData.mbDiagUsed )
    {
        rPropMap[ PROP_DiagonalTLBR ] <<= maApiData.maTLtoBR;
        rPropMap[ PROP_DiagonalBLTR ] <<= maApiData.maBLtoTR;
    }
}

bool Border::hasBorder() const
{
    if (lcl_isBorder(maApiData.maBottom))
        return true;

    if (lcl_isBorder(maApiData.maTop))
        return true;

    if (lcl_isBorder(maApiData.maLeft))
        return true;

    if (lcl_isBorder(maApiData.maRight))
        return true;

    return false;
}

BorderLineModel* Border::getBorderLine( sal_Int32 nElement )
{
    switch( nElement )
    {
        case XLS_TOKEN( left ):     return &maModel.maLeft;
        case XLS_TOKEN( right ):    return &maModel.maRight;
        case XLS_TOKEN( top ):      return &maModel.maTop;
        case XLS_TOKEN( bottom ):   return &maModel.maBottom;
        case XLS_TOKEN( diagonal ): return &maModel.maDiagonal;
    }
    return 0;
}

bool Border::convertBorderLine( BorderLine2& rBorderLine, const BorderLineModel& rModel )
{
    rBorderLine.Color = rModel.maColor.getColor( getBaseFilter().getGraphicHelper(), API_RGB_BLACK );
    switch( rModel.mnStyle )
    {
        case XML_dashDot:           lclSetBorderLineWidth( rBorderLine, API_LINE_THIN );    break;
        case XML_dashDotDot:        lclSetBorderLineWidth( rBorderLine, API_LINE_THIN );    break;
        case XML_dashed:
        {
                                    lclSetBorderLineWidth( rBorderLine, API_LINE_THIN );
                                    rBorderLine.LineStyle = API_LINE_DASHED;
                                    break;
        }
        case XML_dotted:
        {
                                    lclSetBorderLineWidth( rBorderLine, API_LINE_THIN );
                                    rBorderLine.LineStyle = API_LINE_DOTTED;
                                    break;
        }
        case XML_double:            lclSetBorderLineWidth( rBorderLine, API_LINE_THIN, API_LINE_THIN, API_LINE_THIN ); break;
        case XML_hair:              lclSetBorderLineWidth( rBorderLine, API_LINE_HAIR );    break;
        case XML_medium:            lclSetBorderLineWidth( rBorderLine, API_LINE_MEDIUM );  break;
        case XML_mediumDashDot:     lclSetBorderLineWidth( rBorderLine, API_LINE_MEDIUM );  break;
        case XML_mediumDashDotDot:  lclSetBorderLineWidth( rBorderLine, API_LINE_MEDIUM );  break;
        case XML_mediumDashed:      lclSetBorderLineWidth( rBorderLine, API_LINE_MEDIUM );  break;
        case XML_none:              lclSetBorderLineWidth( rBorderLine, API_LINE_NONE );    break;
        case XML_slantDashDot:      lclSetBorderLineWidth( rBorderLine, API_LINE_MEDIUM );  break;
        case XML_thick:             lclSetBorderLineWidth( rBorderLine, API_LINE_THICK );   break;
        case XML_thin:              lclSetBorderLineWidth( rBorderLine, API_LINE_THIN );    break;
        default:                    lclSetBorderLineWidth( rBorderLine, API_LINE_NONE );    break;
    }
    return rModel.mbUsed;
}


// ============================================================================

PatternFillModel::PatternFillModel( bool bDxf ) :
    mnPattern( XML_none ),
    mbPattColorUsed( !bDxf ),
    mbFillColorUsed( !bDxf ),
    mbPatternUsed( !bDxf )
{
    maPatternColor.setIndexed( OOX_COLOR_WINDOWTEXT );
    maFillColor.setIndexed( OOX_COLOR_WINDOWBACK );
}

void PatternFillModel::setBinPattern( sal_Int32 nPattern )
{
    static const sal_Int32 spnPatternIds[] = {
        XML_none, XML_solid, XML_mediumGray, XML_darkGray,
        XML_lightGray, XML_darkHorizontal, XML_darkVertical, XML_darkDown,
        XML_darkUp, XML_darkGrid, XML_darkTrellis, XML_lightHorizontal,
        XML_lightVertical, XML_lightDown, XML_lightUp, XML_lightGrid,
        XML_lightTrellis, XML_gray125, XML_gray0625 };
    mnPattern = STATIC_ARRAY_SELECT( spnPatternIds, nPattern, XML_none );
}

void PatternFillModel::setBiffData( sal_uInt16 nPatternColor, sal_uInt16 nFillColor, sal_uInt8 nPattern )
{
    maPatternColor.setIndexed( nPatternColor );
    maFillColor.setIndexed( nFillColor );
    // patterns equal in BIFF and OOBIN
    setBinPattern( nPattern );
}

// ----------------------------------------------------------------------------

GradientFillModel::GradientFillModel() :
    mnType( XML_linear ),
    mfAngle( 0.0 ),
    mfLeft( 0.0 ),
    mfRight( 0.0 ),
    mfTop( 0.0 ),
    mfBottom( 0.0 )
{
}

void GradientFillModel::readGradient( RecordInputStream& rStrm )
{
    sal_Int32 nType;
    rStrm >> nType >> mfAngle >> mfLeft >> mfRight >> mfTop >> mfBottom;
    static const sal_Int32 spnTypes[] = { XML_linear, XML_path };
    mnType = STATIC_ARRAY_SELECT( spnTypes, nType, XML_TOKEN_INVALID );
}

void GradientFillModel::readGradientStop( RecordInputStream& rStrm, bool bDxf )
{
    Color aColor;
    double fPosition;
    if( bDxf )
    {
        rStrm.skip( 2 );
        rStrm >> fPosition >> aColor;
    }
    else
    {
        rStrm >> aColor >> fPosition;
    }
    if( !rStrm.isEof() && (fPosition >= 0.0) )
        maColors[ fPosition ] = aColor;
}

// ----------------------------------------------------------------------------

ApiSolidFillData::ApiSolidFillData() :
    mnColor( API_RGB_TRANSPARENT ),
    mbTransparent( true ),
    mbUsed( false )
{
}

bool operator==( const ApiSolidFillData& rLeft, const ApiSolidFillData& rRight )
{
    return
        (rLeft.mnColor       == rRight.mnColor) &&
        (rLeft.mbTransparent == rRight.mbTransparent) &&
        (rLeft.mbUsed        == rRight.mbUsed);
}

// ============================================================================

namespace {

inline sal_Int32 lclGetMixedColorComp( sal_Int32 nPatt, sal_Int32 nFill, sal_Int32 nAlpha )
{
    return ((nPatt - nFill) * nAlpha) / 0x80 + nFill;
}

sal_Int32 lclGetMixedColor( sal_Int32 nPattColor, sal_Int32 nFillColor, sal_Int32 nAlpha )
{
    return
        (lclGetMixedColorComp( nPattColor & 0xFF0000, nFillColor & 0xFF0000, nAlpha ) & 0xFF0000) |
        (lclGetMixedColorComp( nPattColor & 0x00FF00, nFillColor & 0x00FF00, nAlpha ) & 0x00FF00) |
        (lclGetMixedColorComp( nPattColor & 0x0000FF, nFillColor & 0x0000FF, nAlpha ) & 0x0000FF);
}

} // namespace

// ----------------------------------------------------------------------------

Fill::Fill( const WorkbookHelper& rHelper, bool bDxf ) :
    WorkbookHelper( rHelper ),
    mbDxf( bDxf )
{
}

void Fill::importPatternFill( const AttributeList& rAttribs )
{
    mxPatternModel.reset( new PatternFillModel( mbDxf ) );
    mxPatternModel->mnPattern = rAttribs.getToken( XML_patternType, XML_none );
    if( mbDxf )
        mxPatternModel->mbPatternUsed = rAttribs.hasAttribute( XML_patternType );
}

void Fill::importFgColor( const AttributeList& rAttribs )
{
    OSL_ENSURE( mxPatternModel.get(), "Fill::importFgColor - missing pattern data" );
    if( mxPatternModel.get() )
    {
        mxPatternModel->maPatternColor.importColor( rAttribs );
        mxPatternModel->mbPattColorUsed = true;
    }
}

void Fill::importBgColor( const AttributeList& rAttribs )
{
    OSL_ENSURE( mxPatternModel.get(), "Fill::importBgColor - missing pattern data" );
    if( mxPatternModel.get() )
    {
        mxPatternModel->maFillColor.importColor( rAttribs );
        mxPatternModel->mbFillColorUsed = true;
    }
}

void Fill::importGradientFill( const AttributeList& rAttribs )
{
    mxGradientModel.reset( new GradientFillModel );
    mxGradientModel->mnType = rAttribs.getToken( XML_type, XML_linear );
    mxGradientModel->mfAngle = rAttribs.getDouble( XML_degree, 0.0 );
    mxGradientModel->mfLeft = rAttribs.getDouble( XML_left, 0.0 );
    mxGradientModel->mfRight = rAttribs.getDouble( XML_right, 0.0 );
    mxGradientModel->mfTop = rAttribs.getDouble( XML_top, 0.0 );
    mxGradientModel->mfBottom = rAttribs.getDouble( XML_bottom, 0.0 );
}

void Fill::importColor( const AttributeList& rAttribs, double fPosition )
{
    OSL_ENSURE( mxGradientModel.get(), "Fill::importColor - missing gradient data" );
    if( mxGradientModel.get() && (fPosition >= 0.0) )
        mxGradientModel->maColors[ fPosition ].importColor( rAttribs );
}

void Fill::importFill( RecordInputStream& rStrm )
{
    OSL_ENSURE( !mbDxf, "Fill::importFill - unexpected conditional formatting flag" );
    sal_Int32 nPattern = rStrm.readInt32();
    if( nPattern == OOBIN_FILL_GRADIENT )
    {
        mxGradientModel.reset( new GradientFillModel );
        sal_Int32 nStopCount;
        rStrm.skip( 16 );
        mxGradientModel->readGradient( rStrm );
        rStrm >> nStopCount;
        for( sal_Int32 nStop = 0; (nStop < nStopCount) && !rStrm.isEof(); ++nStop )
            mxGradientModel->readGradientStop( rStrm, false );
    }
    else
    {
        mxPatternModel.reset( new PatternFillModel( mbDxf ) );
        mxPatternModel->setBinPattern( nPattern );
        rStrm >> mxPatternModel->maPatternColor >> mxPatternModel->maFillColor;
    }
}

void Fill::importDxfPattern( RecordInputStream& rStrm )
{
    OSL_ENSURE( mbDxf, "Fill::importDxfPattern - missing conditional formatting flag" );
    if( !mxPatternModel )
        mxPatternModel.reset( new PatternFillModel( mbDxf ) );
    mxPatternModel->setBinPattern( rStrm.readuInt8() );
    mxPatternModel->mbPatternUsed = true;
}

void Fill::importDxfFgColor( RecordInputStream& rStrm )
{
    OSL_ENSURE( mbDxf, "Fill::importDxfFgColor - missing conditional formatting flag" );
    if( !mxPatternModel )
        mxPatternModel.reset( new PatternFillModel( mbDxf ) );
    mxPatternModel->maPatternColor.importColor( rStrm );
    mxPatternModel->mbPattColorUsed = true;
}

void Fill::importDxfBgColor( RecordInputStream& rStrm )
{
    OSL_ENSURE( mbDxf, "Fill::importDxfBgColor - missing conditional formatting flag" );
    if( !mxPatternModel )
        mxPatternModel.reset( new PatternFillModel( mbDxf ) );
    mxPatternModel->maFillColor.importColor( rStrm );
    mxPatternModel->mbFillColorUsed = true;
}

void Fill::importDxfGradient( RecordInputStream& rStrm )
{
    OSL_ENSURE( mbDxf, "Fill::importDxfGradient - missing conditional formatting flag" );
    if( !mxGradientModel )
        mxGradientModel.reset( new GradientFillModel );
    mxGradientModel->readGradient( rStrm );
}

void Fill::importDxfStop( RecordInputStream& rStrm )
{
    OSL_ENSURE( mbDxf, "Fill::importDxfStop - missing conditional formatting flag" );
    if( !mxGradientModel )
        mxGradientModel.reset( new GradientFillModel );
    mxGradientModel->readGradientStop( rStrm, true );
}

void Fill::setBiff2Data( sal_uInt8 nFlags )
{
    OSL_ENSURE( !mbDxf, "Fill::setBiff2Data - unexpected conditional formatting flag" );
    mxPatternModel.reset( new PatternFillModel( mbDxf ) );
    mxPatternModel->setBiffData(
        BIFF2_COLOR_BLACK,
        BIFF2_COLOR_WHITE,
        getFlagValue( nFlags, BIFF2_XF_BACKGROUND, BIFF_PATT_125, BIFF_PATT_NONE ) );
}

void Fill::setBiff3Data( sal_uInt16 nArea )
{
    OSL_ENSURE( !mbDxf, "Fill::setBiff3Data - unexpected conditional formatting flag" );
    mxPatternModel.reset( new PatternFillModel( mbDxf ) );
    mxPatternModel->setBiffData(
        extractValue< sal_uInt16 >( nArea, 6, 5 ),
        extractValue< sal_uInt16 >( nArea, 11, 5 ),
        extractValue< sal_uInt8 >( nArea, 0, 6 ) );
}

void Fill::setBiff5Data( sal_uInt32 nArea )
{
    OSL_ENSURE( !mbDxf, "Fill::setBiff5Data - unexpected conditional formatting flag" );
    mxPatternModel.reset( new PatternFillModel( mbDxf ) );
    mxPatternModel->setBiffData(
        extractValue< sal_uInt16 >( nArea, 0, 7 ),
        extractValue< sal_uInt16 >( nArea, 7, 7 ),
        extractValue< sal_uInt8 >( nArea, 16, 6 ) );
}

void Fill::setBiff8Data( sal_uInt32 nBorder2, sal_uInt16 nArea )
{
    OSL_ENSURE( !mbDxf, "Fill::setBiff8Data - unexpected conditional formatting flag" );
    mxPatternModel.reset( new PatternFillModel( mbDxf ) );
    mxPatternModel->setBiffData(
        extractValue< sal_uInt16 >( nArea, 0, 7 ),
        extractValue< sal_uInt16 >( nArea, 7, 7 ),
        extractValue< sal_uInt8 >( nBorder2, 26, 6 ) );
}

void Fill::importCfRule( BiffInputStream& rStrm, sal_uInt32 nFlags )
{
    OSL_ENSURE( mbDxf, "Fill::importCfRule - missing conditional formatting flag" );
    OSL_ENSURE( getFlag( nFlags, BIFF_CFRULE_FILLBLOCK ), "Fill::importCfRule - missing fill block flag" );
    mxPatternModel.reset( new PatternFillModel( mbDxf ) );
    sal_uInt32 nFillData;
    rStrm >> nFillData;
    mxPatternModel->setBiffData(
        extractValue< sal_uInt16 >( nFillData, 16, 7 ),
        extractValue< sal_uInt16 >( nFillData, 23, 7 ),
        extractValue< sal_uInt8 >( nFillData, 10, 6 ) );
    mxPatternModel->mbPattColorUsed = !getFlag( nFlags, BIFF_CFRULE_FILL_PATTCOLOR );
    mxPatternModel->mbFillColorUsed = !getFlag( nFlags, BIFF_CFRULE_FILL_FILLCOLOR );
    mxPatternModel->mbPatternUsed   = !getFlag( nFlags, BIFF_CFRULE_FILL_PATTERN );
}

void Fill::finalizeImport()
{
    const GraphicHelper& rGraphicHelper = getBaseFilter().getGraphicHelper();

    if( mxPatternModel.get() )
    {
        // finalize the OOX data struct
        PatternFillModel& rModel = *mxPatternModel;
        if( mbDxf )
        {
            if( rModel.mbFillColorUsed && (!rModel.mbPatternUsed || (rModel.mnPattern == XML_solid)) )
            {
                rModel.maPatternColor = rModel.maFillColor;
                rModel.mnPattern = XML_solid;
                rModel.mbPattColorUsed = rModel.mbPatternUsed = true;
            }
            else if( !rModel.mbFillColorUsed && rModel.mbPatternUsed && (rModel.mnPattern == XML_solid) )
            {
                rModel.mbPatternUsed = false;
            }
        }

        // convert to API fill settings
        maApiData.mbUsed = rModel.mbPatternUsed;
        if( rModel.mnPattern == XML_none )
        {
            maApiData.mnColor = API_RGB_TRANSPARENT;
            maApiData.mbTransparent = true;
        }
        else
        {
            sal_Int32 nAlpha = 0x80;
            switch( rModel.mnPattern )
            {
                case XML_darkDown:          nAlpha = 0x40;  break;
                case XML_darkGray:          nAlpha = 0x60;  break;
                case XML_darkGrid:          nAlpha = 0x40;  break;
                case XML_darkHorizontal:    nAlpha = 0x40;  break;
                case XML_darkTrellis:       nAlpha = 0x60;  break;
                case XML_darkUp:            nAlpha = 0x40;  break;
                case XML_darkVertical:      nAlpha = 0x40;  break;
                case XML_gray0625:          nAlpha = 0x08;  break;
                case XML_gray125:           nAlpha = 0x10;  break;
                case XML_lightDown:         nAlpha = 0x20;  break;
                case XML_lightGray:         nAlpha = 0x20;  break;
                case XML_lightGrid:         nAlpha = 0x38;  break;
                case XML_lightHorizontal:   nAlpha = 0x20;  break;
                case XML_lightTrellis:      nAlpha = 0x30;  break;
                case XML_lightUp:           nAlpha = 0x20;  break;
                case XML_lightVertical:     nAlpha = 0x20;  break;
                case XML_mediumGray:        nAlpha = 0x40;  break;
                case XML_solid:             nAlpha = 0x80;  break;
            }

            sal_Int32 nWinTextColor = rGraphicHelper.getSystemColor( XML_windowText );
            sal_Int32 nWinColor = rGraphicHelper.getSystemColor( XML_window );

            if( !rModel.mbPattColorUsed )
                rModel.maPatternColor.setAuto();
            sal_Int32 nPattColor = rModel.maPatternColor.getColor( rGraphicHelper, nWinTextColor );

            if( !rModel.mbFillColorUsed )
                rModel.maFillColor.setAuto();
            sal_Int32 nFillColor = rModel.maFillColor.getColor( rGraphicHelper, nWinColor );

            maApiData.mnColor = lclGetMixedColor( nPattColor, nFillColor, nAlpha );
            maApiData.mbTransparent = false;
        }
    }
    else if( mxGradientModel.get() && !mxGradientModel->maColors.empty() )
    {
        GradientFillModel& rModel = *mxGradientModel;
        maApiData.mbUsed = true;    // no support for differential attributes
        GradientFillModel::ColorMap::const_iterator aIt = rModel.maColors.begin();
        OSL_ENSURE( !aIt->second.isAuto(), "Fill::finalizeImport - automatic gradient color" );
        maApiData.mnColor = aIt->second.getColor( rGraphicHelper, API_RGB_WHITE );
        if( ++aIt != rModel.maColors.end() )
        {
            OSL_ENSURE( !aIt->second.isAuto(), "Fill::finalizeImport - automatic gradient color" );
            sal_Int32 nEndColor = aIt->second.getColor( rGraphicHelper, API_RGB_WHITE );
            maApiData.mnColor = lclGetMixedColor( maApiData.mnColor, nEndColor, 0x40 );
            maApiData.mbTransparent = false;
        }
    }
}

void Fill::writeToPropertyMap( PropertyMap& rPropMap ) const
{
    if( maApiData.mbUsed )
    {
        rPropMap[ PROP_CellBackColor ] <<= maApiData.mnColor;
        rPropMap[ PROP_IsCellBackgroundTransparent ] <<= maApiData.mbTransparent;
    }
}

// ============================================================================

XfModel::XfModel() :
    mnStyleXfId( -1 ),
    mnFontId( -1 ),
    mnNumFmtId( -1 ),
    mnBorderId( -1 ),
    mnFillId( -1 ),
    mbCellXf( true ),
    mbFontUsed( false ),
    mbNumFmtUsed( false ),
    mbAlignUsed( false ),
    mbProtUsed( false ),
    mbBorderUsed( false ),
    mbAreaUsed( false )
{
}

// ============================================================================

Xf::Xf( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    maAlignment( rHelper ),
    maProtection( rHelper ),
    meRotationRef( ::com::sun::star::table::CellVertJustify_STANDARD )
{
}

void Xf::setAllUsedFlags( bool bUsed )
{
    maModel.mbAlignUsed = maModel.mbProtUsed = maModel.mbFontUsed =
        maModel.mbNumFmtUsed = maModel.mbBorderUsed = maModel.mbAreaUsed = bUsed;
}

void Xf::importXf( const AttributeList& rAttribs, bool bCellXf )
{
    maModel.mbCellXf = bCellXf;
    maModel.mnStyleXfId = rAttribs.getInteger( XML_xfId, -1 );
    maModel.mnFontId = rAttribs.getInteger( XML_fontId, -1 );
    maModel.mnNumFmtId = rAttribs.getInteger( XML_numFmtId, -1 );
    maModel.mnBorderId = rAttribs.getInteger( XML_borderId, -1 );
    maModel.mnFillId = rAttribs.getInteger( XML_fillId, -1 );

    /*  Default value of the apply*** attributes is dependent on context:
        true in cellStyleXfs element, false in cellXfs element... */
    maModel.mbAlignUsed  = rAttribs.getBool( XML_applyAlignment,    !maModel.mbCellXf );
    maModel.mbProtUsed   = rAttribs.getBool( XML_applyProtection,   !maModel.mbCellXf );
    maModel.mbFontUsed   = rAttribs.getBool( XML_applyFont,         !maModel.mbCellXf );
    maModel.mbNumFmtUsed = rAttribs.getBool( XML_applyNumberFormat, !maModel.mbCellXf );
    maModel.mbBorderUsed = rAttribs.getBool( XML_applyBorder,       !maModel.mbCellXf );
    maModel.mbAreaUsed   = rAttribs.getBool( XML_applyFill,         !maModel.mbCellXf );
}

void Xf::importAlignment( const AttributeList& rAttribs )
{
    maAlignment.importAlignment( rAttribs );
}

void Xf::importProtection( const AttributeList& rAttribs )
{
    maProtection.importProtection( rAttribs );
}

void Xf::importXf( RecordInputStream& rStrm, bool bCellXf )
{
    maModel.mbCellXf = bCellXf;
    maModel.mnStyleXfId = rStrm.readuInt16();
    maModel.mnNumFmtId = rStrm.readuInt16();
    maModel.mnFontId = rStrm.readuInt16();
    maModel.mnFillId = rStrm.readuInt16();
    maModel.mnBorderId = rStrm.readuInt16();
    sal_uInt32 nFlags = rStrm.readuInt32();
    maAlignment.setBinData( nFlags );
    maProtection.setBinData( nFlags );
    // used flags, see comments in Xf::setBiffUsedFlags()
    sal_uInt16 nUsedFlags = rStrm.readuInt16();
    maModel.mbFontUsed   = maModel.mbCellXf == getFlag( nUsedFlags, OOBIN_XF_FONT_USED );
    maModel.mbNumFmtUsed = maModel.mbCellXf == getFlag( nUsedFlags, OOBIN_XF_NUMFMT_USED );
    maModel.mbAlignUsed  = maModel.mbCellXf == getFlag( nUsedFlags, OOBIN_XF_ALIGN_USED );
    maModel.mbProtUsed   = maModel.mbCellXf == getFlag( nUsedFlags, OOBIN_XF_PROT_USED );
    maModel.mbBorderUsed = maModel.mbCellXf == getFlag( nUsedFlags, OOBIN_XF_BORDER_USED );
    maModel.mbAreaUsed   = maModel.mbCellXf == getFlag( nUsedFlags, OOBIN_XF_AREA_USED );
}

void Xf::importXf( BiffInputStream& rStrm )
{
    BorderRef xBorder = getStyles().createBorder( &maModel.mnBorderId );
    FillRef xFill = getStyles().createFill( &maModel.mnFillId );

    switch( getBiff() )
    {
        case BIFF2:
        {
            sal_uInt8 nFontId, nNumFmtId, nFlags;
            rStrm >> nFontId;
            rStrm.skip( 1 );
            rStrm >> nNumFmtId >> nFlags;

            // only cell XFs in BIFF2, no parent style, used flags always true
            setAllUsedFlags( true );

            // attributes
            maAlignment.setBiff2Data( nFlags );
            maProtection.setBiff2Data( nNumFmtId );
            xBorder->setBiff2Data( nFlags );
            xFill->setBiff2Data( nFlags );
            maModel.mnFontId = static_cast< sal_Int32 >( nFontId );
            maModel.mnNumFmtId = static_cast< sal_Int32 >( nNumFmtId & BIFF2_XF_VALFMT_MASK );
        }
        break;

        case BIFF3:
        {
            sal_uInt32 nBorder;
            sal_uInt16 nTypeProt, nAlign, nArea;
            sal_uInt8 nFontId, nNumFmtId;
            rStrm >> nFontId >> nNumFmtId >> nTypeProt >> nAlign >> nArea >> nBorder;

            // XF type/parent
            maModel.mbCellXf = !getFlag( nTypeProt, BIFF_XF_STYLE ); // new in BIFF3
            maModel.mnStyleXfId = extractValue< sal_Int32 >( nAlign, 4, 12 ); // new in BIFF3
            // attribute used flags
            setBiffUsedFlags( extractValue< sal_uInt8 >( nTypeProt, 10, 6 ) ); // new in BIFF3

            // attributes
            maAlignment.setBiff3Data( nAlign );
            maProtection.setBiff3Data( nTypeProt );
            xBorder->setBiff3Data( nBorder );
            xFill->setBiff3Data( nArea );
            maModel.mnFontId = static_cast< sal_Int32 >( nFontId );
            maModel.mnNumFmtId = static_cast< sal_Int32 >( nNumFmtId );
        }
        break;

        case BIFF4:
        {
            sal_uInt32 nBorder;
            sal_uInt16 nTypeProt, nAlign, nArea;
            sal_uInt8 nFontId, nNumFmtId;
            rStrm >> nFontId >> nNumFmtId >> nTypeProt >> nAlign >> nArea >> nBorder;

            // XF type/parent
            maModel.mbCellXf = !getFlag( nTypeProt, BIFF_XF_STYLE );
            maModel.mnStyleXfId = extractValue< sal_Int32 >( nTypeProt, 4, 12 );
            // attribute used flags
            setBiffUsedFlags( extractValue< sal_uInt8 >( nAlign, 10, 6 ) );

            // attributes
            maAlignment.setBiff4Data( nAlign );
            maProtection.setBiff3Data( nTypeProt );
            xBorder->setBiff3Data( nBorder );
            xFill->setBiff3Data( nArea );
            maModel.mnFontId = static_cast< sal_Int32 >( nFontId );
            maModel.mnNumFmtId = static_cast< sal_Int32 >( nNumFmtId );
        }
        break;

        case BIFF5:
        {
            sal_uInt32 nArea, nBorder;
            sal_uInt16 nFontId, nNumFmtId, nTypeProt, nAlign;
            rStrm >> nFontId >> nNumFmtId >> nTypeProt >> nAlign >> nArea >> nBorder;

            // XF type/parent
            maModel.mbCellXf = !getFlag( nTypeProt, BIFF_XF_STYLE );
            maModel.mnStyleXfId = extractValue< sal_Int32 >( nTypeProt, 4, 12 );
            // attribute used flags
            setBiffUsedFlags( extractValue< sal_uInt8 >( nAlign, 10, 6 ) );

            // attributes
            maAlignment.setBiff5Data( nAlign );
            maProtection.setBiff3Data( nTypeProt );
            xBorder->setBiff5Data( nBorder, nArea );
            xFill->setBiff5Data( nArea );
            maModel.mnFontId = static_cast< sal_Int32 >( nFontId );
            maModel.mnNumFmtId = static_cast< sal_Int32 >( nNumFmtId );
        }
        break;

        case BIFF8:
        {
            sal_uInt32 nBorder1, nBorder2;
            sal_uInt16 nFontId, nNumFmtId, nTypeProt, nAlign, nMiscAttrib, nArea;
            rStrm >> nFontId >> nNumFmtId >> nTypeProt >> nAlign >> nMiscAttrib >> nBorder1 >> nBorder2 >> nArea;

            // XF type/parent
            maModel.mbCellXf = !getFlag( nTypeProt, BIFF_XF_STYLE );
            maModel.mnStyleXfId = extractValue< sal_Int32 >( nTypeProt, 4, 12 );
            // attribute used flags
            setBiffUsedFlags( extractValue< sal_uInt8 >( nMiscAttrib, 10, 6 ) );

            // attributes
            maAlignment.setBiff8Data( nAlign, nMiscAttrib );
            maProtection.setBiff3Data( nTypeProt );
            xBorder->setBiff8Data( nBorder1, nBorder2 );
            xFill->setBiff8Data( nBorder2, nArea );
            maModel.mnFontId = static_cast< sal_Int32 >( nFontId );
            maModel.mnNumFmtId = static_cast< sal_Int32 >( nNumFmtId );
        }
        break;

        case BIFF_UNKNOWN: break;
    }
}

void Xf::finalizeImport()
{
    StylesBuffer& rStyles = getStyles();

    // alignment and protection
    maAlignment.finalizeImport();
    maProtection.finalizeImport();

    /*  Enables the used flags, if the formatting attributes differ from the
        style XF. In cell XFs Excel uses the cell attributes, if they differ
        from the parent style XF (even if the used flag is switched off).
        #109899# ...or if the respective flag is not set in parent style XF.
     */
    const Xf* pStyleXf = isCellXf() ? rStyles.getStyleXf( maModel.mnStyleXfId ).get() : 0;
    if( pStyleXf )
    {
        const XfModel& rStyleData = pStyleXf->maModel;
        if( !maModel.mbFontUsed )
            maModel.mbFontUsed = !rStyleData.mbFontUsed || (maModel.mnFontId != rStyleData.mnFontId);
        if( !maModel.mbNumFmtUsed )
            maModel.mbNumFmtUsed = !rStyleData.mbNumFmtUsed || (maModel.mnNumFmtId != rStyleData.mnNumFmtId);
        if( !maModel.mbAlignUsed )
            maModel.mbAlignUsed = !rStyleData.mbAlignUsed || !(maAlignment.getApiData() == pStyleXf->maAlignment.getApiData());
        if( !maModel.mbProtUsed )
            maModel.mbProtUsed = !rStyleData.mbProtUsed || !(maProtection.getApiData() == pStyleXf->maProtection.getApiData());
        if( !maModel.mbBorderUsed )
            maModel.mbBorderUsed = !rStyleData.mbBorderUsed || !rStyles.equalBorders( maModel.mnBorderId, rStyleData.mnBorderId );
        if( !maModel.mbAreaUsed )
            maModel.mbAreaUsed = !rStyleData.mbAreaUsed || !rStyles.equalFills( maModel.mnFillId, rStyleData.mnFillId );
    }

    /*  #i38709# Decide which rotation reference mode to use. If any outer
        border line of the cell is set (either explicitly or via cell style),
        and the cell contents are rotated, set rotation reference to bottom of
        cell. This causes the borders to be painted rotated with the text. */
    if( const Alignment* pAlignment = maModel.mbAlignUsed ? &maAlignment : (pStyleXf ? &pStyleXf->maAlignment : 0) )
    {
        sal_Int32 nBorderId = maModel.mbBorderUsed ? maModel.mnBorderId : (pStyleXf ? pStyleXf->maModel.mnBorderId : -1);
        if( const Border* pBorder = rStyles.getBorder( nBorderId ).get() )
            if( (pAlignment->getApiData().mnRotation != 0) && pBorder->getApiData().hasAnyOuterBorder() )
                meRotationRef = ::com::sun::star::table::CellVertJustify_BOTTOM;
    }
}

FontRef Xf::getFont() const
{
    return getStyles().getFont( maModel.mnFontId );
}

bool Xf::hasAnyUsedFlags() const
{
    return
        maModel.mbAlignUsed || maModel.mbProtUsed || maModel.mbFontUsed ||
        maModel.mbNumFmtUsed || maModel.mbBorderUsed || maModel.mbAreaUsed;
}

void Xf::writeToPropertyMap( PropertyMap& rPropMap ) const
{
    StylesBuffer& rStyles = getStyles();

    // create and set cell style
    if( isCellXf() )
        rPropMap[ PROP_CellStyle ] <<= rStyles.createCellStyle( maModel.mnStyleXfId );

    if( maModel.mbFontUsed )
        rStyles.writeFontToPropertyMap( rPropMap, maModel.mnFontId );
    if( maModel.mbNumFmtUsed )
        rStyles.writeNumFmtToPropertyMap( rPropMap, maModel.mnNumFmtId );
    if( maModel.mbAlignUsed )
        maAlignment.writeToPropertyMap( rPropMap );
    if( maModel.mbProtUsed )
        maProtection.writeToPropertyMap( rPropMap );
    if( maModel.mbBorderUsed )
        rStyles.writeBorderToPropertyMap( rPropMap, maModel.mnBorderId );
    if( maModel.mbAreaUsed )
        rStyles.writeFillToPropertyMap( rPropMap, maModel.mnFillId );
    if( maModel.mbAlignUsed || maModel.mbBorderUsed )
        rPropMap[ PROP_RotateReference ] <<= meRotationRef;

    ::com::sun::star::table::CellVertJustify eRotRef = ::com::sun::star::table::CellVertJustify_STANDARD;
    if (maModel.mbBorderUsed && rStyles.hasBorder(maModel.mnBorderId) && maAlignment.getApiData().mnRotation)
        eRotRef = ::com::sun::star::table::CellVertJustify_BOTTOM;
    rPropMap[ PROP_RotateReference ] <<= eRotRef;
}

void Xf::writeToPropertySet( PropertySet& rPropSet ) const
{
    PropertyMap aPropMap;
    writeToPropertyMap( aPropMap );
    rPropSet.setProperties( aPropMap );
}

void Xf::setBiffUsedFlags( sal_uInt8 nUsedFlags )
{
    /*  Notes about finding the used flags:
        - In cell XFs a *set* bit means a used attribute.
        - In style XFs a *cleared* bit means a used attribute.
        The boolean flags always store true, if the attribute is used.
        The "isCellXf() == getFlag(...)" construct evaluates to true in both
        mentioned cases: cell XF and set bit; or style XF and cleared bit.
     */
    maModel.mbFontUsed   = isCellXf() == getFlag( nUsedFlags, BIFF_XF_FONT_USED );
    maModel.mbNumFmtUsed = isCellXf() == getFlag( nUsedFlags, BIFF_XF_NUMFMT_USED );
    maModel.mbAlignUsed  = isCellXf() == getFlag( nUsedFlags, BIFF_XF_ALIGN_USED );
    maModel.mbProtUsed   = isCellXf() == getFlag( nUsedFlags, BIFF_XF_PROT_USED );
    maModel.mbBorderUsed = isCellXf() == getFlag( nUsedFlags, BIFF_XF_BORDER_USED );
    maModel.mbAreaUsed   = isCellXf() == getFlag( nUsedFlags, BIFF_XF_AREA_USED );
}

// ============================================================================

Dxf::Dxf( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
}

FontRef Dxf::createFont( bool bAlwaysNew )
{
    if( bAlwaysNew || !mxFont )
        mxFont.reset( new Font( *this, true ) );
    return mxFont;
}

BorderRef Dxf::createBorder( bool bAlwaysNew )
{
    if( bAlwaysNew || !mxBorder )
        mxBorder.reset( new Border( *this, true ) );
    return mxBorder;
}

FillRef Dxf::createFill( bool bAlwaysNew )
{
    if( bAlwaysNew || !mxFill )
        mxFill.reset( new Fill( *this, true ) );
    return mxFill;
}

void Dxf::importNumFmt( const AttributeList& rAttribs )
{
    mxNumFmt = getStyles().importNumFmt( rAttribs );
}

void Dxf::importAlignment( const AttributeList& rAttribs )
{
    mxAlignment.reset( new Alignment( *this ) );
    mxAlignment->importAlignment( rAttribs );
}

void Dxf::importProtection( const AttributeList& rAttribs )
{
    mxProtection.reset( new Protection( *this ) );
    mxProtection->importProtection( rAttribs );
}

void Dxf::importDxf( RecordInputStream& rStrm )
{
    sal_Int32 nNumFmtId = -1;
    OUString aFmtCode;
    sal_uInt16 nRecCount;
    rStrm.skip( 4 );    // flags
    rStrm >> nRecCount;
    for( sal_uInt16 nRec = 0; !rStrm.isEof() && (nRec < nRecCount); ++nRec )
    {
        sal_uInt16 nSubRecId, nSubRecSize;
        sal_Int64 nRecEnd = rStrm.tell();
        rStrm >> nSubRecId >> nSubRecSize;
        nRecEnd += nSubRecSize;
        switch( nSubRecId )
        {
            case OOBIN_DXF_FILL_PATTERN:    createFill( false )->importDxfPattern( rStrm );                         break;
            case OOBIN_DXF_FILL_FGCOLOR:    createFill( false )->importDxfFgColor( rStrm );                         break;
            case OOBIN_DXF_FILL_BGCOLOR:    createFill( false )->importDxfBgColor( rStrm );                         break;
            case OOBIN_DXF_FILL_GRADIENT:   createFill( false )->importDxfGradient( rStrm );                        break;
            case OOBIN_DXF_FILL_STOP:       createFill( false )->importDxfStop( rStrm );                            break;
            case OOBIN_DXF_FONT_COLOR:      createFont( false )->importDxfColor( rStrm );                           break;
            case OOBIN_DXF_BORDER_TOP:      createBorder( false )->importDxfBorder( XLS_TOKEN( top ), rStrm );      break;
            case OOBIN_DXF_BORDER_BOTTOM:   createBorder( false )->importDxfBorder( XLS_TOKEN( bottom ), rStrm );   break;
            case OOBIN_DXF_BORDER_LEFT:     createBorder( false )->importDxfBorder( XLS_TOKEN( left ), rStrm );     break;
            case OOBIN_DXF_BORDER_RIGHT:    createBorder( false )->importDxfBorder( XLS_TOKEN( right ), rStrm );    break;
            case OOBIN_DXF_FONT_NAME:       createFont( false )->importDxfName( rStrm );                            break;
            case OOBIN_DXF_FONT_WEIGHT:     createFont( false )->importDxfWeight( rStrm );                          break;
            case OOBIN_DXF_FONT_UNDERLINE:  createFont( false )->importDxfUnderline( rStrm );                       break;
            case OOBIN_DXF_FONT_ESCAPEMENT: createFont( false )->importDxfEscapement( rStrm );                      break;
            case OOBIN_DXF_FONT_ITALIC:     createFont( false )->importDxfFlag( XML_i, rStrm );                     break;
            case OOBIN_DXF_FONT_STRIKE:     createFont( false )->importDxfFlag( XML_strike, rStrm );                break;
            case OOBIN_DXF_FONT_OUTLINE:    createFont( false )->importDxfFlag( XML_outline, rStrm );               break;
            case OOBIN_DXF_FONT_SHADOW:     createFont( false )->importDxfFlag( XML_shadow, rStrm );                break;
            case OOBIN_DXF_FONT_HEIGHT:     createFont( false )->importDxfHeight( rStrm );                          break;
            case OOBIN_DXF_FONT_SCHEME:     createFont( false )->importDxfScheme( rStrm );                          break;
            case OOBIN_DXF_NUMFMT_CODE:     aFmtCode = rStrm.readString( false );                                   break;
            case OOBIN_DXF_NUMFMT_ID:       nNumFmtId = rStrm.readuInt16();                                         break;
        }
        rStrm.seek( nRecEnd );
    }
    OSL_ENSURE( !rStrm.isEof() && (rStrm.getRemaining() == 0), "Dxf::importDxf - unexpected remaining data" );
    mxNumFmt = getStyles().createNumFmt( nNumFmtId, aFmtCode );
}

void Dxf::importCfRule( BiffInputStream& rStrm, sal_uInt32 nFlags )
{
    if( getFlag( nFlags, BIFF_CFRULE_FONTBLOCK ) )
        createFont()->importCfRule( rStrm );
    if( getFlag( nFlags, BIFF_CFRULE_ALIGNBLOCK ) )
        rStrm.skip( 8 );
    if( getFlag( nFlags, BIFF_CFRULE_BORDERBLOCK ) )
        createBorder()->importCfRule( rStrm, nFlags );
    if( getFlag( nFlags, BIFF_CFRULE_FILLBLOCK ) )
        createFill()->importCfRule( rStrm, nFlags );
    if( getFlag( nFlags, BIFF_CFRULE_PROTBLOCK ) )
        rStrm.skip( 2 );
}

void Dxf::finalizeImport()
{
    if( mxFont.get() )
        mxFont->finalizeImport();
    // number format already finalized by the number formats buffer
    if( mxAlignment.get() )
        mxAlignment->finalizeImport();
    if( mxProtection.get() )
        mxProtection->finalizeImport();
    if( mxBorder.get() )
        mxBorder->finalizeImport();
    if( mxFill.get() )
        mxFill->finalizeImport();
}

void Dxf::writeToPropertyMap( PropertyMap& rPropMap ) const
{
    if( mxFont.get() )
        mxFont->writeToPropertyMap( rPropMap, FONT_PROPTYPE_CELL );
    if( mxNumFmt.get() )
        mxNumFmt->writeToPropertyMap( rPropMap );
    if( mxAlignment.get() )
        mxAlignment->writeToPropertyMap( rPropMap );
    if( mxProtection.get() )
        mxProtection->writeToPropertyMap( rPropMap );
    if( mxBorder.get() )
        mxBorder->writeToPropertyMap( rPropMap );
    if( mxFill.get() )
        mxFill->writeToPropertyMap( rPropMap );
}

void Dxf::writeToPropertySet( PropertySet& rPropSet ) const
{
    PropertyMap aPropMap;
    writeToPropertyMap( aPropMap );
    rPropSet.setProperties( aPropMap );
}

// ============================================================================

namespace {

const sal_Char* const spcLegacyStyleNamePrefix = "Excel_BuiltIn_";
const sal_Char* const sppcLegacyStyleNames[] =
{
    "Normal",
    "RowLevel_",            // outline level will be appended
    "ColumnLevel_",         // outline level will be appended
    "Comma",
    "Currency",
    "Percent",
    "Comma_0",              // new in BIFF4
    "Currency_0",
    "Hyperlink",            // new in BIFF8
    "Followed_Hyperlink"
};
const sal_Int32 snLegacyStyleNamesCount = static_cast< sal_Int32 >( STATIC_ARRAY_SIZE( sppcLegacyStyleNames ) );

const sal_Char* const spcStyleNamePrefix = "Excel Built-in ";
const sal_Char* const sppcStyleNames[] =
{
    "Normal",
    "RowLevel_",            // outline level will be appended
    "ColLevel_",            // outline level will be appended
    "Comma",
    "Currency",
    "Percent",
    "Comma [0]",            // new in BIFF4
    "Currency [0]",
    "Hyperlink",            // new in BIFF8
    "Followed Hyperlink",
    "Note",                 // new in OOX
    "Warning Text",
    "",
    "",
    "",
    "Title",
    "Heading 1",
    "Heading 2",
    "Heading 3",
    "Heading 4",
    "Input",
    "Output",
    "Calculation",
    "Check Cell",
    "Linked Cell",
    "Total",
    "Good",
    "Bad",
    "Neutral",
    "Accent1",
    "20% - Accent1",
    "40% - Accent1",
    "60% - Accent1",
    "Accent2",
    "20% - Accent2",
    "40% - Accent2",
    "60% - Accent2",
    "Accent3",
    "20% - Accent3",
    "40% - Accent3",
    "60% - Accent3",
    "Accent4",
    "20% - Accent4",
    "40% - Accent4",
    "60% - Accent4",
    "Accent5",
    "20% - Accent5",
    "40% - Accent5",
    "60% - Accent5",
    "Accent6",
    "20% - Accent6",
    "40% - Accent6",
    "60% - Accent6",
    "Explanatory Text"
};
const sal_Int32 snStyleNamesCount = static_cast< sal_Int32 >( STATIC_ARRAY_SIZE( sppcStyleNames ) );

OUString lclGetBuiltinStyleName( sal_Int32 nBuiltinId, const OUString& rName, sal_Int32 nLevel = 0 )
{
    OSL_ENSURE( (0 <= nBuiltinId) && (nBuiltinId < snStyleNamesCount), "lclGetBuiltinStyleName - unknown built-in style" );
    OUStringBuffer aStyleName;
    aStyleName.appendAscii( spcStyleNamePrefix );
    if( (0 <= nBuiltinId) && (nBuiltinId < snStyleNamesCount) && (sppcStyleNames[ nBuiltinId ][ 0 ] != 0) )
        aStyleName.appendAscii( sppcStyleNames[ nBuiltinId ] );
    else if( rName.getLength() > 0 )
        aStyleName.append( rName );
    else
        aStyleName.append( nBuiltinId );
    if( (nBuiltinId == OOX_STYLE_ROWLEVEL) || (nBuiltinId == OOX_STYLE_COLLEVEL) )
        aStyleName.append( nLevel );
    return aStyleName.makeStringAndClear();
}

OUString lclGetBuiltInStyleName( const OUString& rName )
{
    OUStringBuffer aStyleName;
    aStyleName.appendAscii( spcStyleNamePrefix ).append( rName );
    return aStyleName.makeStringAndClear();
}

bool lclIsBuiltinStyleName( const OUString& rStyleName, sal_Int32* pnBuiltinId, sal_Int32* pnNextChar )
{
    // try the other built-in styles
    OUString aPrefix = OUString::createFromAscii( spcStyleNamePrefix );
    sal_Int32 nPrefixLen = aPrefix.getLength();
    sal_Int32 nFoundId = 0;
    sal_Int32 nNextChar = 0;
    if( rStyleName.matchIgnoreAsciiCase( aPrefix ) )
    {
        OUString aShortName;
        for( sal_Int32 nId = 0; nId < snStyleNamesCount; ++nId )
        {
            aShortName = OUString::createFromAscii( sppcStyleNames[ nId ] );
            if( rStyleName.matchIgnoreAsciiCase( aShortName, nPrefixLen ) &&
                    (nNextChar < nPrefixLen + aShortName.getLength()) )
            {
                nFoundId = nId;
                nNextChar = nPrefixLen + aShortName.getLength();
            }
        }
    }

    if( nNextChar > 0 )
    {
        if( pnBuiltinId ) *pnBuiltinId = nFoundId;
        if( pnNextChar ) *pnNextChar = nNextChar;
        return true;
    }

    if( pnBuiltinId ) *pnBuiltinId = -1;
    if( pnNextChar ) *pnNextChar = 0;
    return false;
}

bool lclGetBuiltinStyleId( sal_Int32& rnBuiltinId, sal_Int32& rnLevel, const OUString& rStyleName )
{
    sal_Int32 nBuiltinId;
    sal_Int32 nNextChar;
    if( lclIsBuiltinStyleName( rStyleName, &nBuiltinId, &nNextChar ) )
    {
        if( (nBuiltinId == OOX_STYLE_ROWLEVEL) || (nBuiltinId == OOX_STYLE_COLLEVEL) )
        {
            OUString aLevel = rStyleName.copy( nNextChar );
            sal_Int32 nLevel = aLevel.toInt32();
            if( (0 < nLevel) && (nLevel <= OOX_STYLE_LEVELCOUNT) )
            {
                rnBuiltinId = nBuiltinId;
                rnLevel = nLevel;
                return true;
            }
        }
        else if( rStyleName.getLength() == nNextChar )
        {
            rnBuiltinId = nBuiltinId;
            rnLevel = 0;
            return true;
        }
    }
    rnBuiltinId = -1;
    rnLevel = 0;
    return false;
}

} // namespace

// ----------------------------------------------------------------------------

CellStyleModel::CellStyleModel() :
    mnXfId( -1 ),
    mnBuiltinId( -1 ),
    mnLevel( 0 ),
    mbBuiltin( false ),
    mbCustom( false ),
    mbHidden( false )
{
}

bool CellStyleModel::isBuiltin() const
{
    return mbBuiltin && (mnBuiltinId >= 0);
}

bool CellStyleModel::isDefaultStyle() const
{
    return mbBuiltin && (mnBuiltinId == OOX_STYLE_NORMAL);
}

// ============================================================================

CellStyle::CellStyle( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    mbCreated( false )
{
}

void CellStyle::importCellStyle( const AttributeList& rAttribs )
{
    maModel.maName      = rAttribs.getXString( XML_name, OUString() );
    maModel.mnXfId      = rAttribs.getInteger( XML_xfId, -1 );
    maModel.mnBuiltinId = rAttribs.getInteger( XML_builtinId, -1 );
    maModel.mnLevel     = rAttribs.getInteger( XML_iLevel, 0 );
    maModel.mbBuiltin   = rAttribs.hasAttribute( XML_builtinId );
    maModel.mbCustom    = rAttribs.getBool( XML_customBuiltin, false );
    maModel.mbHidden    = rAttribs.getBool( XML_hidden, false );
}

void CellStyle::importCellStyle( RecordInputStream& rStrm )
{
    sal_uInt16 nFlags;
    rStrm >> maModel.mnXfId >> nFlags;
    maModel.mnBuiltinId = rStrm.readInt8();
    maModel.mnLevel = rStrm.readInt8();
    rStrm >> maModel.maName;
    maModel.mbBuiltin = getFlag( nFlags, OOBIN_CELLSTYLE_BUILTIN );
    maModel.mbCustom = getFlag( nFlags, OOBIN_CELLSTYLE_CUSTOM );
    maModel.mbHidden = getFlag( nFlags, OOBIN_CELLSTYLE_HIDDEN );
}

void CellStyle::importStyle( BiffInputStream& rStrm )
{
    sal_uInt16 nStyleXf;
    rStrm >> nStyleXf;
    maModel.mnXfId = static_cast< sal_Int32 >( nStyleXf & BIFF_STYLE_XFMASK );
    maModel.mbBuiltin = getFlag( nStyleXf, BIFF_STYLE_BUILTIN );
    if( maModel.mbBuiltin )
    {
        maModel.mnBuiltinId = rStrm.readInt8();
        maModel.mnLevel = rStrm.readInt8();
    }
    else
    {
        maModel.maName = (getBiff() == BIFF8) ?
            rStrm.readUniString() : rStrm.readByteStringUC( false, getTextEncoding() );
        // #i103281# check if this is a new built-in style introduced in XL2007
        if( (getBiff() == BIFF8) && (rStrm.getNextRecId() == BIFF_ID_STYLEEXT) && rStrm.startNextRecord() )
        {
            sal_uInt8 nExtFlags;
            rStrm.skip( 12 );
            rStrm >> nExtFlags;
            maModel.mbBuiltin = getFlag( nExtFlags, BIFF_STYLEEXT_BUILTIN );
            maModel.mbCustom = getFlag( nExtFlags, BIFF_STYLEEXT_CUSTOM );
            maModel.mbHidden = getFlag( nExtFlags, BIFF_STYLEEXT_HIDDEN );
            if( maModel.mbBuiltin )
            {
                maModel.mnBuiltinId = rStrm.readInt8();
                maModel.mnLevel = rStrm.readInt8();
            }
        }
    }
}

void CellStyle::createCellStyle()
{
    // #i1624# #i1768# ignore unnamed user styles
    if( !mbCreated )
        mbCreated = maFinalName.getLength() == 0;

    /*  #i103281# do not create another style of the same name, if it exists
        already. This is needed to prevent that styles pasted from clipboard
        get duplicated over and over. */
    if( !mbCreated ) try
    {
        Reference< XNameAccess > xCellStylesNA( getStyleFamily( false ), UNO_QUERY_THROW );
        mbCreated = xCellStylesNA->hasByName( maFinalName );
    }
    catch( Exception& )
    {
    }

    // create the style object in the document
    if( !mbCreated ) try
    {
        mbCreated = true;
        Reference< XStyle > xStyle( createStyleObject( maFinalName, false ), UNO_SET_THROW );
        // write style formatting properties
        PropertySet aPropSet( xStyle );
        getStyles().writeStyleXfToPropertySet( aPropSet, maModel.mnXfId );
        if( !maModel.isDefaultStyle() )
            xStyle->setParentStyle( getStyles().getDefaultStyleName() );
    }
    catch( Exception& )
    {
    }
}

void CellStyle::finalizeImport( const OUString& rFinalName )
{
    maFinalName = rFinalName;
    if( !maModel.isBuiltin() || maModel.mbCustom )
        createCellStyle();
}

// ============================================================================

CellStyleBuffer::CellStyleBuffer( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
}

CellStyleRef CellStyleBuffer::importCellStyle( const AttributeList& rAttribs )
{
    CellStyleRef xCellStyle( new CellStyle( *this ) );
    xCellStyle->importCellStyle( rAttribs );
    insertCellStyle( xCellStyle );
    return xCellStyle;
}

CellStyleRef CellStyleBuffer::importCellStyle( RecordInputStream& rStrm )
{
    CellStyleRef xCellStyle( new CellStyle( *this ) );
    xCellStyle->importCellStyle( rStrm );
    insertCellStyle( xCellStyle );
    return xCellStyle;
}

CellStyleRef CellStyleBuffer::importStyle( BiffInputStream& rStrm )
{
    CellStyleRef xCellStyle( new CellStyle( *this ) );
    xCellStyle->importStyle( rStrm );
    insertCellStyle( xCellStyle );
    return xCellStyle;
}

void CellStyleBuffer::finalizeImport()
{
    // calculate final names of all styles
    typedef RefMap< OUString, CellStyle, IgnoreCaseCompare > CellStyleNameMap;
    CellStyleNameMap aCellStyles;
    CellStyleVector aConflictNameStyles;

    /*  First, reserve style names that are built-in in Calc. This causes that
        imported cell styles get different unused names and thus do not try to
        overwrite these built-in styles. For BIFF4 workbooks (which contain a
        separate list of cell styles per sheet), reserve all existing styles if
        current sheet is not the first sheet (this styles buffer will be
        constructed again for every new sheet). This will create unique names
        for styles in different sheets with the same name. Assuming that the
        BIFF4W import filter is never used to import from clipboard... */
    bool bReserveAll = (getFilterType() == FILTER_BIFF) && (getBiff() == BIFF4) && isWorkbookFile() && (getCurrentSheetIndex() > 0);
    try
    {
        // unfortunately, com.sun.star.style.StyleFamily does not implement XEnumerationAccess...
        Reference< XIndexAccess > xStyleFamilyIA( getStyleFamily( false ), UNO_QUERY_THROW );
        for( sal_Int32 nIndex = 0, nCount = xStyleFamilyIA->getCount(); nIndex < nCount; ++nIndex )
        {
            Reference< XStyle > xStyle( xStyleFamilyIA->getByIndex( nIndex ), UNO_QUERY_THROW );
            if( bReserveAll || !xStyle->isUserDefined() )
            {
                Reference< XNamed > xStyleName( xStyle, UNO_QUERY_THROW );
                // create an empty entry by using ::std::map<>::operator[]
                aCellStyles[ xStyleName->getName() ];
            }
        }
    }
    catch( Exception& )
    {
    }

    /*  Calculate names of built-in styles. Store styles with reserved names
        in the aConflictNameStyles list. */
    for( CellStyleVector::iterator aIt = maBuiltinStyles.begin(), aEnd = maBuiltinStyles.end(); aIt != aEnd; ++aIt )
    {
        const CellStyleModel& rModel = (*aIt)->getModel();
        if (rModel.isDefaultStyle())
            continue;

        OUString aStyleName = lclGetBuiltinStyleName( rModel.mnBuiltinId, rModel.maName, rModel.mnLevel );
        OSL_ENSURE( bReserveAll || (aCellStyles.count( aStyleName ) == 0),
            "CellStyleBuffer::finalizeImport - multiple styles with equal built-in identifier" );
        if( aCellStyles.count( aStyleName ) > 0 )
            aConflictNameStyles.push_back( *aIt );
        else
            aCellStyles[ aStyleName ] = *aIt;
    }

    /*  Calculate names of user defined styles. Store styles with reserved
        names in the aConflictNameStyles list. */
    for( CellStyleVector::iterator aIt = maUserStyles.begin(), aEnd = maUserStyles.end(); aIt != aEnd; ++aIt )
    {
        const CellStyleModel& rModel = (*aIt)->getModel();
        // #i1624# #i1768# ignore unnamed user styles
        if( rModel.maName.getLength() > 0 )
        {
            if( aCellStyles.count( rModel.maName ) > 0 )
                aConflictNameStyles.push_back( *aIt );
            else
                aCellStyles[ rModel.maName ] = *aIt;
        }
    }

    // find unused names for all styles with conflicting names
    for( CellStyleVector::iterator aIt = aConflictNameStyles.begin(), aEnd = aConflictNameStyles.end(); aIt != aEnd; ++aIt )
    {
        const CellStyleModel& rModel = (*aIt)->getModel();
        OUString aUnusedName;
        sal_Int32 nIndex = 0;
        do
        {
            aUnusedName = OUStringBuffer( rModel.maName ).append( sal_Unicode( ' ' ) ).append( ++nIndex ).makeStringAndClear();
        }
        while( aCellStyles.count( aUnusedName ) > 0 );
        aCellStyles[ aUnusedName ] = *aIt;
    }

    // set final names and create user-defined and modified built-in cell styles
    aCellStyles.forEachMemWithKey( &CellStyle::finalizeImport );

    if (mxDefStyle)
    {
        Reference<XNameAccess> xNA(getStyleFamily(false), UNO_QUERY_THROW);
        if (xNA->hasByName(CREATE_OUSTRING("Default")))
        {
            PropertySet aPropSet(xNA->getByName(CREATE_OUSTRING("Default")));
            getStyles().writeStyleXfToPropertySet(aPropSet, mxDefStyle->getModel().mnXfId);
        }
    }
}

sal_Int32 CellStyleBuffer::getDefaultXfId() const
{
    return mxDefStyle.get() ? mxDefStyle->getModel().mnXfId : -1;
}

OUString CellStyleBuffer::getDefaultStyleName() const
{
    return createCellStyle( mxDefStyle );
}

OUString CellStyleBuffer::createCellStyle( sal_Int32 nXfId ) const
{
    return createCellStyle( maStylesByXf.get( nXfId ) );
}

// private --------------------------------------------------------------------

void CellStyleBuffer::insertCellStyle( CellStyleRef xCellStyle )
{
    const CellStyleModel& rModel = xCellStyle->getModel();
    if( rModel.mnXfId >= 0 )
    {
        // insert into the built-in map or user defined map
        (rModel.isBuiltin() ? maBuiltinStyles : maUserStyles).push_back( xCellStyle );

        // insert into the XF identifier map
        OSL_ENSURE( maStylesByXf.count( rModel.mnXfId ) == 0, "CellStyleBuffer::insertCellStyle - multiple styles with equal XF identifier" );
        maStylesByXf[ rModel.mnXfId ] = xCellStyle;

        // remember default cell style
        if( rModel.isDefaultStyle() )
            mxDefStyle = xCellStyle;
    }
}

OUString CellStyleBuffer::createCellStyle( const CellStyleRef& rxCellStyle ) const
{
    if( rxCellStyle.get() )
    {
        rxCellStyle->createCellStyle();
        const OUString& rStyleName = rxCellStyle->getFinalStyleName();
        if( rStyleName.getLength() > 0 )
            return rStyleName;
    }
    // on error: fallback to default style
    return lclGetBuiltinStyleName( OOX_STYLE_NORMAL, OUString() );
}

// ============================================================================

StylesBuffer::StylesBuffer( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    maPalette( rHelper ),
    maNumFmts( rHelper ),
    maCellStyles( rHelper )
{
}

FontRef StylesBuffer::createFont( sal_Int32* opnFontId )
{
    if( opnFontId ) *opnFontId = static_cast< sal_Int32 >( maFonts.size() );
    FontRef xFont( new Font( *this, false ) );
    maFonts.push_back( xFont );
    return xFont;
}

NumberFormatRef StylesBuffer::createNumFmt( sal_Int32 nNumFmtId, const OUString& rFmtCode )
{
    return maNumFmts.createNumFmt( nNumFmtId, rFmtCode );
}

BorderRef StylesBuffer::createBorder( sal_Int32* opnBorderId )
{
    if( opnBorderId ) *opnBorderId = static_cast< sal_Int32 >( maBorders.size() );
    BorderRef xBorder( new Border( *this, false ) );
    maBorders.push_back( xBorder );
    return xBorder;
}

FillRef StylesBuffer::createFill( sal_Int32* opnFillId )
{
    if( opnFillId ) *opnFillId = static_cast< sal_Int32 >( maFills.size() );
    FillRef xFill( new Fill( *this, false ) );
    maFills.push_back( xFill );
    return xFill;
}

XfRef StylesBuffer::createCellXf( sal_Int32* opnXfId )
{
    if( opnXfId ) *opnXfId = static_cast< sal_Int32 >( maCellXfs.size() );
    XfRef xXf( new Xf( *this ) );
    maCellXfs.push_back( xXf );
    return xXf;
}

XfRef StylesBuffer::createStyleXf( sal_Int32* opnXfId )
{
    if( opnXfId ) *opnXfId = static_cast< sal_Int32 >( maStyleXfs.size() );
    XfRef xXf( new Xf( *this ) );
    maStyleXfs.push_back( xXf );
    return xXf;
}

DxfRef StylesBuffer::createDxf( sal_Int32* opnDxfId )
{
    if( opnDxfId ) *opnDxfId = static_cast< sal_Int32 >( maDxfs.size() );
    DxfRef xDxf( new Dxf( *this ) );
    maDxfs.push_back( xDxf );
    return xDxf;
}

void StylesBuffer::importPaletteColor( const AttributeList& rAttribs )
{
    maPalette.importPaletteColor( rAttribs );
}

NumberFormatRef StylesBuffer::importNumFmt( const AttributeList& rAttribs )
{
    return maNumFmts.importNumFmt( rAttribs );
}

CellStyleRef StylesBuffer::importCellStyle( const AttributeList& rAttribs )
{
    return maCellStyles.importCellStyle( rAttribs );
}

void StylesBuffer::importPaletteColor( RecordInputStream& rStrm )
{
    maPalette.importPaletteColor( rStrm );
}

void StylesBuffer::importNumFmt( RecordInputStream& rStrm )
{
    maNumFmts.importNumFmt( rStrm );
}

void StylesBuffer::importCellStyle( RecordInputStream& rStrm )
{
    maCellStyles.importCellStyle( rStrm );
}

void StylesBuffer::importPalette( BiffInputStream& rStrm )
{
    maPalette.importPalette( rStrm );
}

void StylesBuffer::importFont( BiffInputStream& rStrm )
{
    /* Font with index 4 is not stored in BIFF. This means effectively, first
        font in the BIFF file has index 0, fourth font has index 3, and fifth
        font has index 5. Insert a dummy font to correctly map passed font
        identifiers. */
    if( maFonts.size() == 4 )
        maFonts.push_back( maFonts.front() );

    FontRef xFont = createFont();
    xFont->importFont( rStrm );

    /*  #i71033# Set stream text encoding from application font, if CODEPAGE
        record is missing. Must be done now (not while finalizeImport() runs),
        to be able to read all following byte strings correctly (e.g. cell
        style names). */
    if( maFonts.size() == 1 )
        setAppFontEncoding( xFont->getFontEncoding() );
}

void StylesBuffer::importFontColor( BiffInputStream& rStrm )
{
    if( !maFonts.empty() )
        maFonts.back()->importFontColor( rStrm );
}

void StylesBuffer::importFormat( BiffInputStream& rStrm )
{
    maNumFmts.importFormat( rStrm );
}

void StylesBuffer::importXf( BiffInputStream& rStrm )
{
    XfRef xXf( new Xf( *this ) );
    xXf->importXf( rStrm );

    XfRef xCellXf, xStyleXf;
    (xXf->isCellXf() ? xCellXf : xStyleXf) = xXf;
    maCellXfs.push_back( xCellXf );
    maStyleXfs.push_back( xStyleXf );
}

void StylesBuffer::importStyle( BiffInputStream& rStrm )
{
    maCellStyles.importStyle( rStrm );
}

void StylesBuffer::finalizeImport()
{
    // fonts first, are needed to finalize unit converter and XFs below
    maFonts.forEachMem( &Font::finalizeImport );
    // finalize unit coefficients after default font is known
    getUnitConverter().finalizeImport();
    // number formats
    maNumFmts.finalizeImport();
    // borders and fills
    maBorders.forEachMem( &Border::finalizeImport );
    maFills.forEachMem( &Fill::finalizeImport );
    // style XFs and cell XFs
    maStyleXfs.forEachMem( &Xf::finalizeImport );
    maCellXfs.forEachMem( &Xf::finalizeImport );
    // built-in and user defined cell styles
    maCellStyles.finalizeImport();
    // differential formatting (for conditional formatting)
    maDxfs.forEachMem( &Dxf::finalizeImport );
}

sal_Int32 StylesBuffer::getPaletteColor( sal_Int32 nPaletteIdx ) const
{
    return maPalette.getColor( nPaletteIdx );
}

FontRef StylesBuffer::getFont( sal_Int32 nFontId ) const
{
    return maFonts.get( nFontId );
}

BorderRef StylesBuffer::getBorder( sal_Int32 nBorderId ) const
{
    return maBorders.get( nBorderId );
}

XfRef StylesBuffer::getCellXf( sal_Int32 nXfId ) const
{
    return maCellXfs.get( nXfId );
}

XfRef StylesBuffer::getStyleXf( sal_Int32 nXfId ) const
{
    return maStyleXfs.get( nXfId );
}

DxfRef StylesBuffer::getDxf( sal_Int32 nDxfId ) const
{
    return maDxfs.get( nDxfId );
}

FontRef StylesBuffer::getFontFromCellXf( sal_Int32 nXfId ) const
{
    FontRef xFont;
    if( const Xf* pXf = getCellXf( nXfId ).get() )
        xFont = pXf->getFont();
    return xFont;
}

FontRef StylesBuffer::getDefaultFont() const
{
    FontRef xDefFont;
    if( const Xf* pXf = getStyleXf( maCellStyles.getDefaultXfId() ).get() )
        xDefFont = pXf->getFont();
    // no font from styles - try first loaded font (e.g. BIFF2)
    if( !xDefFont )
        xDefFont = maFonts.get( 0 );
    OSL_ENSURE( xDefFont.get(), "StylesBuffer::getDefaultFont - no default font found" );
    return xDefFont;
}

const FontModel& StylesBuffer::getDefaultFontModel() const
{
    FontRef xDefFont = getDefaultFont();
    return xDefFont.get() ? xDefFont->getModel() : getTheme().getDefaultFontModel();
}

bool StylesBuffer::equalBorders( sal_Int32 nBorderId1, sal_Int32 nBorderId2 ) const
{
    if( nBorderId1 == nBorderId2 )
        return true;

    switch( getFilterType() )
    {
        case FILTER_OOX:
            // in OOXML, borders are assumed to be unique
            return false;

        case FILTER_BIFF:
        {
            // in BIFF, a new border entry has been created for every XF
            const Border* pBorder1 = maBorders.get( nBorderId1 ).get();
            const Border* pBorder2 = maBorders.get( nBorderId2 ).get();
            return pBorder1 && pBorder2 && (pBorder1->getApiData() == pBorder2->getApiData());
        }

        case FILTER_UNKNOWN:
        break;
    }
    return false;
}

bool StylesBuffer::equalFills( sal_Int32 nFillId1, sal_Int32 nFillId2 ) const
{
    if( nFillId1 == nFillId2 )
        return true;

    switch( getFilterType() )
    {
        case FILTER_OOX:
            // in OOXML, fills are assumed to be unique
            return false;

        case FILTER_BIFF:
        {
            // in BIFF, a new fill entry has been created for every XF
            const Fill* pFill1 = maFills.get( nFillId1 ).get();
            const Fill* pFill2 = maFills.get( nFillId2 ).get();
            return pFill1 && pFill2 && (pFill1->getApiData() == pFill2->getApiData());
        }

        case FILTER_UNKNOWN:
        break;
    }
    return false;
}

OUString StylesBuffer::getDefaultStyleName() const
{
    return maCellStyles.getDefaultStyleName();
}

OUString StylesBuffer::createCellStyle( sal_Int32 nXfId ) const
{
    return maCellStyles.createCellStyle( nXfId );
}

OUString StylesBuffer::createDxfStyle( sal_Int32 nDxfId ) const
{
    OUString& rStyleName = maDxfStyles[ nDxfId ];
    if( rStyleName.getLength() == 0 )
    {
        if( Dxf* pDxf = maDxfs.get( nDxfId ).get() )
        {
            rStyleName = OUStringBuffer( CREATE_OUSTRING( "ConditionalStyle_" ) ).append( nDxfId + 1 ).makeStringAndClear();
            // create the style sheet (this may change rStyleName if such a style already exists)
            Reference< XStyle > xStyle = createStyleObject( rStyleName, false );
            // write style formatting properties
            PropertySet aPropSet( xStyle );
            pDxf->writeToPropertySet( aPropSet );
        }
        // on error: fallback to default style
        if( rStyleName.getLength() == 0 )
            rStyleName = maCellStyles.getDefaultStyleName();
    }
    return rStyleName;
}

void StylesBuffer::writeFontToPropertyMap( PropertyMap& rPropMap, sal_Int32 nFontId ) const
{
    if( Font* pFont = maFonts.get( nFontId ).get() )
        pFont->writeToPropertyMap( rPropMap, FONT_PROPTYPE_CELL );
}

void StylesBuffer::writeNumFmtToPropertyMap( PropertyMap& rPropMap, sal_Int32 nNumFmtId ) const
{
    maNumFmts.writeToPropertyMap( rPropMap, nNumFmtId );
}

void StylesBuffer::writeBorderToPropertyMap( PropertyMap& rPropMap, sal_Int32 nBorderId ) const
{
    if( Border* pBorder = maBorders.get( nBorderId ).get() )
        pBorder->writeToPropertyMap( rPropMap );
}

void StylesBuffer::writeFillToPropertyMap( PropertyMap& rPropMap, sal_Int32 nFillId ) const
{
    if( Fill* pFill = maFills.get( nFillId ).get() )
        pFill->writeToPropertyMap( rPropMap );
}

void StylesBuffer::writeCellXfToPropertyMap( PropertyMap& rPropMap, sal_Int32 nXfId ) const
{
    if( Xf* pXf = maCellXfs.get( nXfId ).get() )
        pXf->writeToPropertyMap( rPropMap );
}

void StylesBuffer::writeStyleXfToPropertyMap( PropertyMap& rPropMap, sal_Int32 nXfId ) const
{
    if( Xf* pXf = maStyleXfs.get( nXfId ).get() )
        pXf->writeToPropertyMap( rPropMap );
}

void StylesBuffer::writeCellXfToPropertySet( PropertySet& rPropSet, sal_Int32 nXfId ) const
{
    if( Xf* pXf = maCellXfs.get( nXfId ).get() )
        pXf->writeToPropertySet( rPropSet );
}

bool StylesBuffer::hasBorder( sal_Int32 nBorderId ) const
{
    Border* pBorder = maBorders.get( nBorderId ).get();
    return pBorder && pBorder->hasBorder();
}

void StylesBuffer::writeStyleXfToPropertySet( PropertySet& rPropSet, sal_Int32 nXfId ) const
{
    if( Xf* pXf = maStyleXfs.get( nXfId ).get() )
        pXf->writeToPropertySet( rPropSet );
}

// ============================================================================

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
