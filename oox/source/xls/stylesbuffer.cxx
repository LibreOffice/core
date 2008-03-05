/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stylesbuffer.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 19:06:36 $
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

#include "oox/xls/stylesbuffer.hxx"
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/FontFamily.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/awt/XFont2.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/XText.hpp>
#include <rtl/tencinfo.h>
#include <rtl/ustrbuf.hxx>
#include "oox/helper/attributelist.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/helper/recordinputstream.hxx"
#include "oox/xls/biffinputstream.hxx"
#include "oox/xls/condformatbuffer.hxx"
#include "oox/xls/excelhandlers.hxx"
#include "oox/xls/themebuffer.hxx"
#include "oox/xls/unitconverter.hxx"

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::awt::FontDescriptor;
using ::com::sun::star::awt::XDevice;
using ::com::sun::star::awt::XFont2;
using ::com::sun::star::table::BorderLine;
using ::com::sun::star::text::XText;
using ::com::sun::star::style::XStyle;

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

// OOX font charset (also used in BIFF)
const sal_Int32 OOX_FONTCHARSET_UNUSED      = -1;
const sal_Int32 OOX_FONTCHARSET_ANSI        = 0;

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

sal_Int32 lclGetRgbColor( sal_uInt8 nR, sal_uInt8 nG, sal_uInt8 nB, sal_uInt8 nA )
{
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

// ----------------------------------------------------------------------------

OoxColor::OoxColor() :
    mfTint( 0.0 ),
    mnType( XML_auto ),
    mnValue( 0 )
{
}

OoxColor::OoxColor( sal_Int32 nType, sal_Int32 nValue, double fTint ) :
    mfTint( fTint ),
    mnType( nType ),
    mnValue( nValue )
{
}

bool OoxColor::isAuto() const
{
    return mnType == XML_auto;
}

void OoxColor::set( sal_Int32 nType, sal_Int32 nValue, double fTint )
{
    mfTint = fTint;
    mnType = nType;
    mnValue = nValue;
}

void OoxColor::importColor( const AttributeList& rAttribs )
{
    mfTint = rAttribs.getDouble( XML_tint, 0.0 );
    if( rAttribs.getBool( XML_auto, false ) )
    {
        mnType = XML_auto;
    }
    else if( rAttribs.hasAttribute( XML_rgb ) )
    {
        mnType = XML_rgb;
        mnValue = rAttribs.getHex( XML_rgb, API_RGB_TRANSPARENT );
    }
    else if( rAttribs.hasAttribute( XML_theme ) )
    {
        mnType = XML_theme;
        mnValue = rAttribs.getInteger( XML_theme, -1 );
    }
    else if( rAttribs.hasAttribute( XML_indexed ) )
    {
        mnType = XML_indexed;
        mnValue = rAttribs.getInteger( XML_indexed, -1 );
    }
    else
    {
        mnType = XML_auto;
        OSL_ENSURE( false, "OoxColor::importColor - unknown color type" );
    }
}

void OoxColor::importColor( RecordInputStream& rStrm )
{
    sal_uInt8 nFlags, nIndex;
    sal_Int16 nTint;
    rStrm >> nFlags >> nIndex >> nTint;
    switch( extractValue< sal_uInt8 >( nFlags, 1, 7 ) )
    {
        case OOBIN_COLOR_AUTO:
            mnType = XML_auto;
            rStrm.skip( 4 );
        break;
        case OOBIN_COLOR_INDEXED:
            mnType = XML_indexed;
            mnValue = nIndex;
            rStrm.skip( 4 );
        break;
        case OOBIN_COLOR_RGB:
            importColorRgb( rStrm );
        break;
        case OOBIN_COLOR_THEME:
            mnType = XML_theme;
            mnValue = nIndex;
            rStrm.skip( 4 );
        break;
        default:
            OSL_ENSURE( false, "OoxColor::importColor - unknown color type" );
            mnType = XML_auto;
            rStrm.skip( 4 );
    }
    // scale tint from signed 16-bit to double range -1.0 ... 1.0
    mfTint = static_cast< double >( nTint ) / 0x8000;
}

void OoxColor::importColorId( RecordInputStream& rStrm )
{
    mfTint = 0.0;
    mnType = XML_indexed;
    rStrm >> mnValue;
}

void OoxColor::importColorRgb( RecordInputStream& rStrm )
{
    mfTint = 0.0;
    mnType = XML_rgb;
    sal_uInt8 nR, nG, nB, nA;
    rStrm >> nR >> nG >> nB >> nA;
    mnValue = lclGetRgbColor( nR, nG, nB, nA );
}

void OoxColor::importColorId( BiffInputStream& rStrm, bool b16Bit )
{
    mfTint = 0.0;
    mnType = XML_indexed;
    mnValue = b16Bit ? rStrm.readuInt16() : rStrm.readuInt8();
}

void OoxColor::importColorRgb( BiffInputStream& rStrm )
{
    mfTint = 0.0;
    mnType = XML_rgb;
    sal_uInt8 nR, nG, nB, nA;
    rStrm >> nR >> nG >> nB >> nA;
    mnValue = lclGetRgbColor( nR, nG, nB, nA );
}

RecordInputStream& operator>>( RecordInputStream& rStrm, OoxColor& orColor )
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
    WorkbookHelper( rHelper ),
    mnWindowColor( ThemeBuffer::getSystemWindowColor() ),
    mnWinTextColor( ThemeBuffer::getSystemWindowTextColor() )
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
    appendColor( rAttribs.getHex( XML_rgb, API_RGB_TRANSPARENT ) );
}

void ColorPalette::importPaletteColor( RecordInputStream& rStrm )
{
    OoxColor aColor;
    aColor.importColorRgb( rStrm );
    appendColor( aColor.mnValue );
}

void ColorPalette::importPalette( BiffInputStream& rStrm )
{
    sal_uInt16 nCount;
    rStrm >> nCount;
    OSL_ENSURE( rStrm.getRecLeft() == static_cast< sal_uInt32 >( 4 * nCount ),
        "ColorPalette::importPalette - wrong palette size" );

    // fill palette from BIFF_COLOR_USEROFFSET
    mnAppendIndex = BIFF_COLOR_USEROFFSET;
    OoxColor aColor;
    for( sal_uInt16 nIndex = 0; rStrm.isValid() && (nIndex < nCount); ++nIndex )
    {
        aColor.importColorRgb( rStrm );
        appendColor( aColor.mnValue );
    }
}

sal_Int32 ColorPalette::getColor( sal_Int32 nIndex ) const
{
    sal_Int32 nColor = API_RGB_TRANSPARENT;
    if( (0 <= nIndex) && (static_cast< size_t >( nIndex ) < maColors.size()) )
    {
        nColor = maColors[ nIndex ];
    }
    else switch( nIndex )
    {
        case OOX_COLOR_WINDOWTEXT3:
        case OOX_COLOR_WINDOWTEXT:
        case OOX_COLOR_CHWINDOWTEXT:    nColor = mnWinTextColor;        break;
        case OOX_COLOR_WINDOWBACK3:
        case OOX_COLOR_WINDOWBACK:
        case OOX_COLOR_CHWINDOWBACK:    nColor = mnWindowColor;         break;
//        case OOX_COLOR_BUTTONBACK:
//        case OOX_COLOR_CHBORDERAUTO:
//        case OOX_COLOR_NOTEBACK:
//        case OOX_COLOR_NOTETEXT:
        case OOX_COLOR_FONTAUTO:        nColor = API_RGB_TRANSPARENT;   break;
        default:
            OSL_ENSURE( false, "ColorPalette::getColor - unknown color index" );
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

OoxFontData::OoxFontData() :
    mnScheme( XML_none ),
    mnFamily( OOX_FONTFAMILY_NONE ),
    mnCharSet( OOX_FONTCHARSET_ANSI ),
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

void OoxFontData::setBinScheme( sal_uInt8 nScheme )
{
    static const sal_Int32 spnSchemes[] = { XML_none, XML_major, XML_minor };
    mnScheme = STATIC_ARRAY_SELECT( spnSchemes, nScheme, XML_none );
}

void OoxFontData::setBiffHeight( sal_uInt16 nHeight )
{
    mfHeight = nHeight / 20.0;  // convert twips to points
}

void OoxFontData::setBiffWeight( sal_uInt16 nWeight )
{
    mbBold = nWeight >= BIFF_FONTWEIGHT_BOLD;
}

void OoxFontData::setBiffUnderline( sal_uInt16 nUnderline )
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

void OoxFontData::setBiffEscapement( sal_uInt16 nEscapement )
{
    static const sal_Int32 spnEscapes[] = { XML_baseline, XML_superscript, XML_subscript };
    mnEscapement = STATIC_ARRAY_SELECT( spnEscapes, nEscapement, XML_baseline );
}

// ============================================================================

Font::Font( const WorkbookHelper& rHelper, bool bDxf ) :
    WorkbookHelper( rHelper ),
    maOoxData( rHelper.getTheme().getDefaultFontData() ),
    maUsedFlags( !bDxf ),
    mbDxf( bDxf )
{
}

Font::Font( const WorkbookHelper& rHelper, const OoxFontData& rFontData ) :
    WorkbookHelper( rHelper ),
    maOoxData( rFontData ),
    maUsedFlags( true ),
    mbDxf( false )
{
}

bool Font::isSupportedContext( sal_Int32 nElement, sal_Int32 nParentContext )
{
    switch( nParentContext )
    {
        case XLS_TOKEN( font ):
            return  (nElement == XLS_TOKEN( name )) ||
                    (nElement == XLS_TOKEN( scheme )) ||
                    (nElement == XLS_TOKEN( charset )) ||
                    (nElement == XLS_TOKEN( family )) ||
                    (nElement == XLS_TOKEN( sz )) ||
                    (nElement == XLS_TOKEN( color )) ||
                    (nElement == XLS_TOKEN( u )) ||
                    (nElement == XLS_TOKEN( vertAlign )) ||
                    (nElement == XLS_TOKEN( b )) ||
                    (nElement == XLS_TOKEN( i )) ||
                    (nElement == XLS_TOKEN( outline )) ||
                    (nElement == XLS_TOKEN( shadow )) ||
                    (nElement == XLS_TOKEN( strike ));

        case XLS_TOKEN( rPr ):
            return  (nElement == XLS_TOKEN( rFont )) ||
                    (nElement == XLS_TOKEN( scheme )) ||
                    (nElement == XLS_TOKEN( charset )) ||
                    (nElement == XLS_TOKEN( family )) ||
                    (nElement == XLS_TOKEN( sz )) ||
                    (nElement == XLS_TOKEN( color )) ||
                    (nElement == XLS_TOKEN( u )) ||
                    (nElement == XLS_TOKEN( vertAlign )) ||
                    (nElement == XLS_TOKEN( b )) ||
                    (nElement == XLS_TOKEN( i )) ||
                    (nElement == XLS_TOKEN( outline )) ||
                    (nElement == XLS_TOKEN( shadow )) ||
                    (nElement == XLS_TOKEN( strike )) ||
                    (nElement == XLS_TOKEN( vertAlign ));
    }
    return false;
}

void Font::importAttribs( sal_Int32 nElement, const AttributeList& rAttribs )
{
    const OoxFontData& rDefFontData = getTheme().getDefaultFontData();
    switch( nElement )
    {
        case XLS_TOKEN( name ):
        case XLS_TOKEN( rFont ):
            if( rAttribs.hasAttribute( XML_val ) )
            {
                maOoxData.maName = rAttribs.getString( XML_val );
                maUsedFlags.mbNameUsed = true;
            }
        break;
        case XLS_TOKEN( scheme ):
            maOoxData.mnScheme = rAttribs.getToken( XML_val, rDefFontData.mnScheme );
        break;
        case XLS_TOKEN( family ):
            maOoxData.mnFamily = rAttribs.getInteger( XML_val, rDefFontData.mnFamily );
        break;
        case XLS_TOKEN( charset ):
            maOoxData.mnCharSet = rAttribs.getInteger( XML_val, rDefFontData.mnCharSet );
        break;
        case XLS_TOKEN( sz ):
            maOoxData.mfHeight = rAttribs.getDouble( XML_val, rDefFontData.mfHeight );
            maUsedFlags.mbHeightUsed = true;
        break;
        case XLS_TOKEN( color ):
            maOoxData.maColor.importColor( rAttribs );
            maUsedFlags.mbColorUsed = true;
        break;
        case XLS_TOKEN( u ):
            maOoxData.mnUnderline = rAttribs.getToken( XML_val, XML_single );
            maUsedFlags.mbUnderlineUsed = true;
        break;
        case XLS_TOKEN( vertAlign ):
            maOoxData.mnEscapement = rAttribs.getToken( XML_val, XML_baseline );
            maUsedFlags.mbEscapementUsed = true;
        break;
        case XLS_TOKEN( b ):
            maOoxData.mbBold = rAttribs.getBool( XML_val, true );
            maUsedFlags.mbWeightUsed = true;
        break;
        case XLS_TOKEN( i ):
            maOoxData.mbItalic = rAttribs.getBool( XML_val, true );
            maUsedFlags.mbPostureUsed = true;
        break;
        case XLS_TOKEN( strike ):
            maOoxData.mbStrikeout = rAttribs.getBool( XML_val, true );
            maUsedFlags.mbStrikeoutUsed = true;
        break;
        case XLS_TOKEN( outline ):
            maOoxData.mbOutline = rAttribs.getBool( XML_val, true );
            maUsedFlags.mbOutlineUsed = true;
        break;
        case XLS_TOKEN( shadow ):
            maOoxData.mbShadow = rAttribs.getBool( XML_val, true );
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
    rStrm >> maOoxData.maColor >> nScheme >> maOoxData.maName;

    // equal constants in BIFF and OOBIN for weight, underline, and escapement
    maOoxData.setBinScheme( nScheme );
    maOoxData.setBiffHeight( nHeight );
    maOoxData.setBiffWeight( nWeight );
    maOoxData.setBiffUnderline( nUnderline );
    maOoxData.setBiffEscapement( nEscapement );
    maOoxData.mnFamily    = nFamily;
    maOoxData.mnCharSet   = nCharSet;
    // equal flags in BIFF and OOBIN
    maOoxData.mbItalic    = getFlag( nFlags, BIFF_FONTFLAG_ITALIC );
    maOoxData.mbStrikeout = getFlag( nFlags, BIFF_FONTFLAG_STRIKEOUT );
    maOoxData.mbOutline   = getFlag( nFlags, BIFF_FONTFLAG_OUTLINE );
    maOoxData.mbShadow    = getFlag( nFlags, BIFF_FONTFLAG_SHADOW );
}

void Font::importDxfName( RecordInputStream& rStrm )
{
    OSL_ENSURE( mbDxf, "Font::importDxfName - missing conditional formatting flag" );
    maOoxData.maName = rStrm.readString( false );
    maUsedFlags.mbColorUsed = true;
}

void Font::importDxfColor( RecordInputStream& rStrm )
{
    OSL_ENSURE( mbDxf, "Font::importDxfColor - missing conditional formatting flag" );
    rStrm >> maOoxData.maColor;
    maUsedFlags.mbColorUsed = true;
}

void Font::importDxfScheme( RecordInputStream& rStrm )
{
    OSL_ENSURE( mbDxf, "Font::importDxfScheme - missing conditional formatting flag" );
    maOoxData.setBinScheme( rStrm.readuInt8() );
    maUsedFlags.mbSchemeUsed = true;
}

void Font::importDxfHeight( RecordInputStream& rStrm )
{
    OSL_ENSURE( mbDxf, "Font::importDxfHeight - missing conditional formatting flag" );
    maOoxData.setBiffHeight( rStrm.readuInt16() );
    maUsedFlags.mbHeightUsed = true;
}

void Font::importDxfWeight( RecordInputStream& rStrm )
{
    OSL_ENSURE( mbDxf, "Font::importDxfWeight - missing conditional formatting flag" );
    maOoxData.setBiffWeight( rStrm.readuInt16() );
    maUsedFlags.mbWeightUsed = true;
}

void Font::importDxfUnderline( RecordInputStream& rStrm )
{
    OSL_ENSURE( mbDxf, "Font::importDxfUnderline - missing conditional formatting flag" );
    maOoxData.setBiffUnderline( rStrm.readuInt16() );
    maUsedFlags.mbUnderlineUsed = true;
}

void Font::importDxfEscapement( RecordInputStream& rStrm )
{
    OSL_ENSURE( mbDxf, "Font::importDxfEscapement - missing conditional formatting flag" );
    maOoxData.setBiffEscapement( rStrm.readuInt16() );
    maUsedFlags.mbEscapementUsed = true;
}

void Font::importDxfFlag( sal_Int32 nElement, RecordInputStream& rStrm )
{
    OSL_ENSURE( mbDxf, "Font::importDxfFlag - missing conditional formatting flag" );
    bool bFlag = rStrm.readuInt8() != 0;
    switch( nElement )
    {
        case XML_i:
            maOoxData.mbItalic = bFlag;
            maUsedFlags.mbPostureUsed = true;
        break;
        case XML_strike:
            maOoxData.mbStrikeout = bFlag;
            maUsedFlags.mbStrikeoutUsed = true;
        break;
        case XML_outline:
            maOoxData.mbOutline = bFlag;
            maUsedFlags.mbOutlineUsed = true;
        break;
        case XML_shadow:
            maOoxData.mbShadow = bFlag;
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
    maOoxData.maColor.importColorId( rStrm );
}

void Font::importCfRule( BiffInputStream& rStrm )
{
    OSL_ENSURE( mbDxf, "Font::importCfRule - missing conditional formatting flag" );

    sal_Int32 nHeight, nColor;
    sal_uInt32 nStyle, nFontFlags1, nFontFlags2, nFontFlags3;
    sal_uInt16 nWeight, nEscapement;
    sal_uInt8 nUnderline;

    OSL_ENSURE( rStrm.getRecLeft() >= 118, "Font::importCfRule - missing record data" );
    sal_uInt32 nRecPos = rStrm.getRecPos();
    maOoxData.maName = rStrm.readUniString( rStrm.readuInt8() );
    maUsedFlags.mbNameUsed = maOoxData.maName.getLength() > 0;
    OSL_ENSURE( rStrm.isValid() && (rStrm.getRecPos() <= nRecPos + 64), "Font::importCfRule - font name too long" );
    rStrm.seek( nRecPos + 64 );
    rStrm >> nHeight >> nStyle >> nWeight >> nEscapement >> nUnderline;
    rStrm.skip( 3 );
    rStrm >> nColor;
    rStrm.skip( 4 );
    rStrm >> nFontFlags1 >> nFontFlags2 >> nFontFlags3;
    rStrm.skip( 18 );

    if( (maUsedFlags.mbColorUsed = (0 <= nColor) && (nColor <= 0x7FFF)) == true )
        maOoxData.maColor.set( XML_indexed, nColor );
    if( (maUsedFlags.mbHeightUsed = (0 < nHeight) && (nHeight <= 0x7FFF)) == true )
        maOoxData.setBiffHeight( static_cast< sal_uInt16 >( nHeight ) );
    if( (maUsedFlags.mbUnderlineUsed = !getFlag( nFontFlags3, BIFF_CFRULE_FONT_UNDERL )) == true )
        maOoxData.setBiffUnderline( nUnderline );
    if( (maUsedFlags.mbEscapementUsed = !getFlag( nFontFlags2, BIFF_CFRULE_FONT_ESCAPEM )) == true )
        maOoxData.setBiffEscapement( nEscapement );
    if( (maUsedFlags.mbWeightUsed = maUsedFlags.mbPostureUsed = !getFlag( nFontFlags1, BIFF_CFRULE_FONT_STYLE )) == true )
    {
        maOoxData.setBiffWeight( nWeight );
        maOoxData.mbItalic = getFlag( nStyle, BIFF_CFRULE_FONT_STYLE );
    }
    if( (maUsedFlags.mbStrikeoutUsed = !getFlag( nFontFlags1, BIFF_CFRULE_FONT_STRIKEOUT )) == true )
        maOoxData.mbStrikeout = getFlag( nStyle, BIFF_CFRULE_FONT_STRIKEOUT );
    if( (maUsedFlags.mbOutlineUsed = !getFlag( nFontFlags1, BIFF_CFRULE_FONT_OUTLINE )) == true )
        maOoxData.mbOutline = getFlag( nStyle, BIFF_CFRULE_FONT_OUTLINE );
    if( (maUsedFlags.mbShadowUsed = !getFlag( nFontFlags1, BIFF_CFRULE_FONT_SHADOW )) == true )
        maOoxData.mbShadow = getFlag( nStyle, BIFF_CFRULE_FONT_SHADOW );
}

rtl_TextEncoding Font::getFontEncoding() const
{
    // #i63105# cells use text encoding from FONT record character set
    // #i67768# BIFF2-BIFF4 FONT records do not contain character set
    // #i71033# do not use maApiData, this function is used before finalizeImport()
    rtl_TextEncoding eFontEnc = RTL_TEXTENCODING_DONTKNOW;
    if( (0 <= maOoxData.mnCharSet) && (maOoxData.mnCharSet <= SAL_MAX_UINT8) )
        eFontEnc = rtl_getTextEncodingFromWindowsCharset( static_cast< sal_uInt8 >( maOoxData.mnCharSet ) );
    return (eFontEnc == RTL_TEXTENCODING_DONTKNOW) ? getTextEncoding() : eFontEnc;
}

void Font::finalizeImport()
{
    namespace cssawt = ::com::sun::star::awt;

    // font name
    maApiData.maDesc.Name = maOoxData.maName;

    // font family
    switch( maOoxData.mnFamily )
    {
        case OOX_FONTFAMILY_NONE:           maApiData.maDesc.Family = cssawt::FontFamily::DONTKNOW;     break;
        case OOX_FONTFAMILY_ROMAN:          maApiData.maDesc.Family = cssawt::FontFamily::ROMAN;        break;
        case OOX_FONTFAMILY_SWISS:          maApiData.maDesc.Family = cssawt::FontFamily::SWISS;        break;
        case OOX_FONTFAMILY_MODERN:         maApiData.maDesc.Family = cssawt::FontFamily::MODERN;       break;
        case OOX_FONTFAMILY_SCRIPT:         maApiData.maDesc.Family = cssawt::FontFamily::SCRIPT;       break;
        case OOX_FONTFAMILY_DECORATIVE:     maApiData.maDesc.Family = cssawt::FontFamily::DECORATIVE;   break;
    }

    // character set
    if( (0 <= maOoxData.mnCharSet) && (maOoxData.mnCharSet <= 255) )
        maApiData.maDesc.CharSet = static_cast< sal_Int16 >(
            rtl_getTextEncodingFromWindowsCharset( static_cast< sal_uInt8 >( maOoxData.mnCharSet ) ) );

    // color, height, weight, slant, strikeout, outline, shadow
    maApiData.mnColor          = getStyles().getColor( maOoxData.maColor, API_RGB_TRANSPARENT );
    maApiData.maDesc.Height    = static_cast< sal_Int16 >( maOoxData.mfHeight * 20.0 );
    maApiData.maDesc.Weight    = maOoxData.mbBold ? cssawt::FontWeight::BOLD : cssawt::FontWeight::NORMAL;
    maApiData.maDesc.Slant     = maOoxData.mbItalic ? cssawt::FontSlant_ITALIC : cssawt::FontSlant_NONE;
    maApiData.maDesc.Strikeout = maOoxData.mbStrikeout ? cssawt::FontStrikeout::SINGLE : cssawt::FontStrikeout::NONE;
    maApiData.mbOutline        = maOoxData.mbOutline;
    maApiData.mbShadow         = maOoxData.mbShadow;

    // underline
    switch( maOoxData.mnUnderline )
    {
        case XML_double:            maApiData.maDesc.Underline = cssawt::FontUnderline::DOUBLE; break;
        case XML_doubleAccounting:  maApiData.maDesc.Underline = cssawt::FontUnderline::DOUBLE; break;
        case XML_none:              maApiData.maDesc.Underline = cssawt::FontUnderline::NONE;   break;
        case XML_single:            maApiData.maDesc.Underline = cssawt::FontUnderline::SINGLE; break;
        case XML_singleAccounting:  maApiData.maDesc.Underline = cssawt::FontUnderline::SINGLE; break;
    }

    // escapement
    switch( maOoxData.mnEscapement )
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
                maApiData.mbHasAsian =
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
                maApiData.mbHasCmplx =
                    xFont->hasGlyphs( OUString( sal_Unicode( 0x05D1 ) ) ) ||    // 0590-05FF: Hebrew
                    xFont->hasGlyphs( OUString( sal_Unicode( 0x0631 ) ) ) ||    // 0600-06FF: Arabic
                    xFont->hasGlyphs( OUString( sal_Unicode( 0x0721 ) ) ) ||    // 0700-074F: Syriac
                    xFont->hasGlyphs( OUString( sal_Unicode( 0x0911 ) ) ) ||    // 0900-0DFF: Indic scripts
                    xFont->hasGlyphs( OUString( sal_Unicode( 0x0E01 ) ) ) ||    // 0E00-0E7F: Thai
                    xFont->hasGlyphs( OUString( sal_Unicode( 0xFB21 ) ) ) ||    // FB1D-FB4F: Hebrew Presentation Forms
                    xFont->hasGlyphs( OUString( sal_Unicode( 0xFB51 ) ) ) ||    // FB50-FDFF: Arabic Presentation Forms-A
                    xFont->hasGlyphs( OUString( sal_Unicode( 0xFE71 ) ) );      // FE70-FEFF: Arabic Presentation Forms-B
                // Western fonts
                maApiData.mbHasWstrn =
                    (!maApiData.mbHasAsian && !maApiData.mbHasCmplx) ||
                    xFont->hasGlyphs( OUString( sal_Unicode( 'A' ) ) );
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

void Font::writeToPropertySet( PropertySet& rPropSet, FontPropertyType ePropType ) const
{
    getStylesPropertyHelper().writeFontProperties( rPropSet, maApiData, maUsedFlags, ePropType );
}

void Font::importFontData2( BiffInputStream& rStrm )
{
    sal_uInt16 nHeight, nFlags;
    rStrm >> nHeight >> nFlags;

    maOoxData.setBiffHeight( nHeight );
    maOoxData.mnFamily     = OOX_FONTFAMILY_NONE;
    maOoxData.mnCharSet    = OOX_FONTCHARSET_UNUSED;    // ensure to not use font charset in byte string import
    maOoxData.mnUnderline  = getFlagValue( nFlags, BIFF_FONTFLAG_UNDERLINE, XML_single, XML_none );
    maOoxData.mnEscapement = XML_none;
    maOoxData.mbBold       = getFlag( nFlags, BIFF_FONTFLAG_BOLD );
    maOoxData.mbItalic     = getFlag( nFlags, BIFF_FONTFLAG_ITALIC );
    maOoxData.mbStrikeout  = getFlag( nFlags, BIFF_FONTFLAG_STRIKEOUT );
    maOoxData.mbOutline    = getFlag( nFlags, BIFF_FONTFLAG_OUTLINE );
    maOoxData.mbShadow     = getFlag( nFlags, BIFF_FONTFLAG_SHADOW );
}

void Font::importFontData5( BiffInputStream& rStrm )
{
    sal_uInt16 nWeight, nEscapement;
    sal_uInt8 nUnderline, nFamily, nCharSet;
    rStrm >> nWeight >> nEscapement >> nUnderline >> nFamily >> nCharSet;
    rStrm.skip( 1 );

    maOoxData.setBiffWeight( nWeight );
    maOoxData.setBiffUnderline( nUnderline );
    maOoxData.setBiffEscapement( nEscapement );
    // equal constants in XML and BIFF for family and charset
    maOoxData.mnFamily  = nFamily;
    maOoxData.mnCharSet = nCharSet;
}

void Font::importFontName2( BiffInputStream& rStrm )
{
    maOoxData.maName = rStrm.readByteString( false, getTextEncoding() );
}

void Font::importFontName8( BiffInputStream& rStrm )
{
    maOoxData.maName = rStrm.readUniString( rStrm.readuInt8() );
}

// ============================================================================

OoxAlignmentData::OoxAlignmentData() :
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

void OoxAlignmentData::setBinHorAlign( sal_uInt8 nHorAlign )
{
    static const sal_Int32 spnHorAligns[] = {
        XML_general, XML_left, XML_center, XML_right,
        XML_fill, XML_justify, XML_centerContinuous, XML_distributed };
    mnHorAlign = STATIC_ARRAY_SELECT( spnHorAligns, nHorAlign, XML_general );
}

void OoxAlignmentData::setBinVerAlign( sal_uInt8 nVerAlign )
{
    static const sal_Int32 spnVerAligns[] = {
        XML_top, XML_center, XML_bottom, XML_justify, XML_distributed };
    mnVerAlign = STATIC_ARRAY_SELECT( spnVerAligns, nVerAlign, XML_bottom );
}

void OoxAlignmentData::setBinTextOrient( sal_uInt8 nTextOrient )
{
    static const sal_Int32 spnRotations[] = {
        OOX_XF_ROTATION_NONE, OOX_XF_ROTATION_STACKED,
        OOX_XF_ROTATION_90CCW, OOX_XF_ROTATION_90CW };
    mnRotation = STATIC_ARRAY_SELECT( spnRotations, nTextOrient, OOX_XF_ROTATION_NONE );
}

// ============================================================================

Alignment::Alignment( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
}

void Alignment::importAlignment( const AttributeList& rAttribs )
{
    maOoxData.mnHorAlign     = rAttribs.getToken( XML_horizontal, XML_general );
    maOoxData.mnVerAlign     = rAttribs.getToken( XML_vertical, XML_bottom );
    maOoxData.mnTextDir      = rAttribs.getInteger( XML_readingOrder, OOX_XF_TEXTDIR_CONTEXT );
    maOoxData.mnRotation     = rAttribs.getInteger( XML_textRotation, OOX_XF_ROTATION_NONE );
    maOoxData.mnIndent       = rAttribs.getInteger( XML_indent, OOX_XF_INDENT_NONE );
    maOoxData.mbWrapText     = rAttribs.getBool( XML_wrapText, false );
    maOoxData.mbShrink       = rAttribs.getBool( XML_shrinkToFit, false );
    maOoxData.mbJustLastLine = rAttribs.getBool( XML_justifyLastLine, false );
}

void Alignment::setBinData( sal_uInt32 nFlags )
{
    maOoxData.setBinHorAlign( extractValue< sal_uInt8 >( nFlags, 16, 3 ) );
    maOoxData.setBinVerAlign( extractValue< sal_uInt8 >( nFlags, 19, 3 ) );
    maOoxData.mnTextDir      = extractValue< sal_Int32 >( nFlags, 26, 2 );
    maOoxData.mnRotation     = extractValue< sal_Int32 >( nFlags, 0, 8 );
    maOoxData.mnIndent       = extractValue< sal_uInt8 >( nFlags, 8, 8 );
    maOoxData.mbWrapText     = getFlag( nFlags, OOBIN_XF_WRAPTEXT );
    maOoxData.mbShrink       = getFlag( nFlags, OOBIN_XF_SHRINK );
    maOoxData.mbJustLastLine = getFlag( nFlags, OOBIN_XF_JUSTLASTLINE );
}

void Alignment::setBiff2Data( sal_uInt8 nFlags )
{
    maOoxData.setBinHorAlign( extractValue< sal_uInt8 >( nFlags, 0, 3 ) );
}

void Alignment::setBiff3Data( sal_uInt16 nAlign )
{
    maOoxData.setBinHorAlign( extractValue< sal_uInt8 >( nAlign, 0, 3 ) );
    maOoxData.mbWrapText = getFlag( nAlign, BIFF_XF_WRAPTEXT ); // new in BIFF3
}

void Alignment::setBiff4Data( sal_uInt16 nAlign )
{
    maOoxData.setBinHorAlign( extractValue< sal_uInt8 >( nAlign, 0, 3 ) );
    maOoxData.setBinVerAlign( extractValue< sal_uInt8 >( nAlign, 4, 2 ) ); // new in BIFF4
    maOoxData.setBinTextOrient( extractValue< sal_uInt8 >( nAlign, 6, 2 ) ); // new in BIFF4
    maOoxData.mbWrapText = getFlag( nAlign, BIFF_XF_WRAPTEXT );
}

void Alignment::setBiff5Data( sal_uInt16 nAlign )
{
    maOoxData.setBinHorAlign( extractValue< sal_uInt8 >( nAlign, 0, 3 ) );
    maOoxData.setBinVerAlign( extractValue< sal_uInt8 >( nAlign, 4, 3 ) );
    maOoxData.setBinTextOrient( extractValue< sal_uInt8 >( nAlign, 8, 2 ) );
    maOoxData.mbWrapText = getFlag( nAlign, BIFF_XF_WRAPTEXT );
}

void Alignment::setBiff8Data( sal_uInt16 nAlign, sal_uInt16 nMiscAttrib )
{
    maOoxData.setBinHorAlign( extractValue< sal_uInt8 >( nAlign, 0, 3 ) );
    maOoxData.setBinVerAlign( extractValue< sal_uInt8 >( nAlign, 4, 3 ) );
    maOoxData.mnTextDir      = extractValue< sal_Int32 >( nMiscAttrib, 6, 2 ); // new in BIFF8
    maOoxData.mnRotation     = extractValue< sal_Int32 >( nAlign, 8, 8 ); // new in BIFF8
    maOoxData.mnIndent       = extractValue< sal_uInt8 >( nMiscAttrib, 0, 4 ); // new in BIFF8
    maOoxData.mbWrapText     = getFlag( nAlign, BIFF_XF_WRAPTEXT );
    maOoxData.mbShrink       = getFlag( nMiscAttrib, BIFF_XF_SHRINK ); // new in BIFF8
    maOoxData.mbJustLastLine = getFlag( nAlign, BIFF_XF_JUSTLASTLINE ); // new in BIFF8(?)
}

void Alignment::finalizeImport()
{
    namespace csstab = ::com::sun::star::table;
    namespace csstxt = ::com::sun::star::text;

    // horizontal alignment
    switch( maOoxData.mnHorAlign )
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

    // vertical alignment
    switch( maOoxData.mnVerAlign )
    {
        case XML_bottom:        maApiData.meVerJustify = csstab::CellVertJustify_BOTTOM;    break;
        case XML_center:        maApiData.meVerJustify = csstab::CellVertJustify_CENTER;    break;
        case XML_distributed:   maApiData.meVerJustify = csstab::CellVertJustify_TOP;       break;
        case XML_justify:       maApiData.meVerJustify = csstab::CellVertJustify_TOP;       break;
        case XML_top:           maApiData.meVerJustify = csstab::CellVertJustify_TOP;       break;
    }

    /*  indentation: expressed as number of blocks of 3 space characters in
        OOX, and as multiple of 10 points in BIFF. */
    sal_Int32 nIndent = 0;
    switch( getFilterType() )
    {
        case FILTER_OOX:    nIndent = getUnitConverter().scaleToMm100( 3.0 * maOoxData.mnIndent, UNIT_SPACE );  break;
        case FILTER_BIFF:   nIndent = getUnitConverter().scaleToMm100( 10.0 * maOoxData.mnIndent, UNIT_POINT ); break;
        case FILTER_UNKNOWN: break;
    }
    if( (0 <= nIndent) && (nIndent <= SAL_MAX_INT16) )
        maApiData.mnIndent = static_cast< sal_Int16 >( nIndent );

    // complex text direction
    switch( maOoxData.mnTextDir )
    {
        case OOX_XF_TEXTDIR_CONTEXT:    maApiData.mnWritingMode = csstxt::WritingMode2::PAGE;   break;
        case OOX_XF_TEXTDIR_LTR:        maApiData.mnWritingMode = csstxt::WritingMode2::LR_TB;  break;
        case OOX_XF_TEXTDIR_RTL:        maApiData.mnWritingMode = csstxt::WritingMode2::RL_TB;  break;
    }

    // rotation: 0-90 means 0 to 90 degrees ccw, 91-180 means 1 to 90 degrees cw, 255 means stacked
    sal_Int32 nOoxRot = maOoxData.mnRotation;
    maApiData.mnRotation = ((0 <= nOoxRot) && (nOoxRot <= 90)) ?
        (100 * nOoxRot) :
        (((91 <= nOoxRot) && (nOoxRot <= 180)) ? (100 * (450 - nOoxRot)) : 0);

    // "Orientation" property used for character stacking
    maApiData.meOrientation = (nOoxRot == OOX_XF_ROTATION_STACKED) ?
        csstab::CellOrientation_STACKED : csstab::CellOrientation_STANDARD;

    // alignment flags (#i84960 automatic line break, if vertically justified/distributed)
    maApiData.mbWrapText = maOoxData.mbWrapText || (maOoxData.mnVerAlign == XML_distributed) || (maOoxData.mnVerAlign == XML_justify);
    maApiData.mbShrink = maOoxData.mbShrink;

}

void Alignment::writeToPropertySet( PropertySet& rPropSet ) const
{
    getStylesPropertyHelper().writeAlignmentProperties( rPropSet, maApiData );
}

// ============================================================================

OoxProtectionData::OoxProtectionData() :
    mbLocked( true ),   // default in Excel and Calc
    mbHidden( false )
{
}

// ============================================================================

Protection::Protection( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
}

void Protection::importProtection( const AttributeList& rAttribs )
{
    maOoxData.mbLocked = rAttribs.getBool( XML_locked, true );
    maOoxData.mbHidden = rAttribs.getBool( XML_hidden, false );
}

void Protection::setBinData( sal_uInt32 nFlags )
{
    maOoxData.mbLocked = getFlag( nFlags, OOBIN_XF_LOCKED );
    maOoxData.mbHidden = getFlag( nFlags, OOBIN_XF_HIDDEN );
}

void Protection::setBiff2Data( sal_uInt8 nNumFmt )
{
    maOoxData.mbLocked = getFlag( nNumFmt, BIFF2_XF_LOCKED );
    maOoxData.mbHidden = getFlag( nNumFmt, BIFF2_XF_HIDDEN );
}

void Protection::setBiff3Data( sal_uInt16 nProt )
{
    maOoxData.mbLocked = getFlag( nProt, BIFF_XF_LOCKED );
    maOoxData.mbHidden = getFlag( nProt, BIFF_XF_HIDDEN );
}

void Protection::finalizeImport()
{
    maApiData.maCellProt.IsLocked = maOoxData.mbLocked;
    maApiData.maCellProt.IsFormulaHidden = maOoxData.mbHidden;
}

void Protection::writeToPropertySet( PropertySet& rPropSet ) const
{
    getStylesPropertyHelper().writeProtectionProperties( rPropSet, maApiData );
}

// ============================================================================

OoxBorderLineData::OoxBorderLineData( bool bDxf ) :
    maColor( XML_indexed, OOX_COLOR_WINDOWTEXT ),
    mnStyle( XML_none ),
    mbUsed( !bDxf )
{
}

void OoxBorderLineData::setBiffStyle( sal_Int32 nLineStyle )
{
    static const sal_Int32 spnStyleIds[] = {
        XML_none, XML_thin, XML_medium, XML_dashed,
        XML_dotted, XML_thick, XML_double, XML_hair,
        XML_mediumDashed, XML_dashDot, XML_mediumDashDot, XML_dashDotDot,
        XML_mediumDashDotDot, XML_slantDashDot };
    mnStyle = STATIC_ARRAY_SELECT( spnStyleIds, nLineStyle, XML_none );
}

void OoxBorderLineData::setBiffData( sal_uInt8 nLineStyle, sal_uInt16 nLineColor )
{
    maColor.set( XML_indexed, nLineColor );
    setBiffStyle( nLineStyle );
}

// ============================================================================

OoxBorderData::OoxBorderData( bool bDxf ) :
    maLeft( bDxf ),
    maRight( bDxf ),
    maTop( bDxf ),
    maBottom( bDxf ),
    maDiagonal( bDxf ),
    mbDiagTLtoBR( false ),
    mbDiagBLtoTR( false )
{
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

const BorderLine* lclGetThickerLine( const BorderLine& rBorderLine1, sal_Bool bValid1, const BorderLine& rBorderLine2, sal_Bool bValid2 )
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
    maOoxData( bDxf ),
    mbDxf( bDxf )
{
}

bool Border::isSupportedContext( sal_Int32 nElement, sal_Int32 nParentContext )
{
    switch( nParentContext )
    {
        case XLS_TOKEN( border ):
            return  (nElement == XLS_TOKEN( left )) ||
                    (nElement == XLS_TOKEN( right )) ||
                    (nElement == XLS_TOKEN( top )) ||
                    (nElement == XLS_TOKEN( bottom )) ||
                    (nElement == XLS_TOKEN( diagonal ));
        case XLS_TOKEN( left ):
        case XLS_TOKEN( right ):
        case XLS_TOKEN( top ):
        case XLS_TOKEN( bottom ):
        case XLS_TOKEN( diagonal ):
            return  (nElement == XLS_TOKEN( color ));
    }
    return false;
}

void Border::importBorder( const AttributeList& rAttribs )
{
    maOoxData.mbDiagTLtoBR = rAttribs.getBool( XML_diagonalDown, false );
    maOoxData.mbDiagBLtoTR = rAttribs.getBool( XML_diagonalUp, false );
}

void Border::importStyle( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( OoxBorderLineData* pBorderLine = getBorderLine( nElement ) )
    {
        pBorderLine->mnStyle = rAttribs.getToken( XML_style, XML_none );
        pBorderLine->mbUsed = true;
    }
}

void Border::importColor( sal_Int32 nElement, const AttributeList& rAttribs )
{
    if( OoxBorderLineData* pBorderLine = getBorderLine( nElement ) )
        pBorderLine->maColor.importColor( rAttribs );
}

void Border::importBorder( RecordInputStream& rStrm )
{
    sal_uInt8 nFlags = rStrm.readuInt8();
    maOoxData.mbDiagTLtoBR = getFlag( nFlags, OOBIN_BORDER_DIAG_TLBR );
    maOoxData.mbDiagBLtoTR = getFlag( nFlags, OOBIN_BORDER_DIAG_BLTR );
    maOoxData.maTop.setBiffStyle( rStrm.readuInt16() );
    rStrm >> maOoxData.maTop.maColor;
    maOoxData.maBottom.setBiffStyle( rStrm.readuInt16() );
    rStrm >> maOoxData.maBottom.maColor;
    maOoxData.maLeft.setBiffStyle( rStrm.readuInt16() );
    rStrm >> maOoxData.maLeft.maColor;
    maOoxData.maRight.setBiffStyle( rStrm.readuInt16() );
    rStrm >> maOoxData.maRight.maColor;
    maOoxData.maDiagonal.setBiffStyle( rStrm.readuInt16() );
    rStrm >> maOoxData.maDiagonal.maColor;
}

void Border::importDxfBorder( sal_Int32 nElement, RecordInputStream& rStrm )
{
    OSL_ENSURE( mbDxf, "Border::importDxfBorder - missing conditional formatting flag" );
    if( OoxBorderLineData* pBorderLine = getBorderLine( nElement ) )
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
    maOoxData.maLeft.setBiffData(   getFlagValue( nFlags, BIFF2_XF_LEFTLINE,   BIFF_LINE_THIN, BIFF_LINE_NONE ), BIFF2_COLOR_BLACK );
    maOoxData.maRight.setBiffData(  getFlagValue( nFlags, BIFF2_XF_RIGHTLINE,  BIFF_LINE_THIN, BIFF_LINE_NONE ), BIFF2_COLOR_BLACK );
    maOoxData.maTop.setBiffData(    getFlagValue( nFlags, BIFF2_XF_TOPLINE,    BIFF_LINE_THIN, BIFF_LINE_NONE ), BIFF2_COLOR_BLACK );
    maOoxData.maBottom.setBiffData( getFlagValue( nFlags, BIFF2_XF_BOTTOMLINE, BIFF_LINE_THIN, BIFF_LINE_NONE ), BIFF2_COLOR_BLACK );
    maOoxData.maDiagonal.mbUsed = false;
}

void Border::setBiff3Data( sal_uInt32 nBorder )
{
    OSL_ENSURE( !mbDxf, "Border::setBiff3Data - unexpected conditional formatting flag" );
    maOoxData.maLeft.setBiffData(   extractValue< sal_uInt8 >( nBorder,  8, 3 ), extractValue< sal_uInt16 >( nBorder, 11, 5 ) );
    maOoxData.maRight.setBiffData(  extractValue< sal_uInt8 >( nBorder, 24, 3 ), extractValue< sal_uInt16 >( nBorder, 27, 5 ) );
    maOoxData.maTop.setBiffData(    extractValue< sal_uInt8 >( nBorder,  0, 3 ), extractValue< sal_uInt16 >( nBorder,  3, 5 ) );
    maOoxData.maBottom.setBiffData( extractValue< sal_uInt8 >( nBorder, 16, 3 ), extractValue< sal_uInt16 >( nBorder, 19, 5 ) );
    maOoxData.maDiagonal.mbUsed = false;
}

void Border::setBiff5Data( sal_uInt32 nBorder, sal_uInt32 nArea )
{
    OSL_ENSURE( !mbDxf, "Border::setBiff5Data - unexpected conditional formatting flag" );
    maOoxData.maLeft.setBiffData(   extractValue< sal_uInt8 >( nBorder,  3, 3 ), extractValue< sal_uInt16 >( nBorder, 16, 7 ) );
    maOoxData.maRight.setBiffData(  extractValue< sal_uInt8 >( nBorder,  6, 3 ), extractValue< sal_uInt16 >( nBorder, 23, 7 ) );
    maOoxData.maTop.setBiffData(    extractValue< sal_uInt8 >( nBorder,  0, 3 ), extractValue< sal_uInt16 >( nBorder,  9, 7 ) );
    maOoxData.maBottom.setBiffData( extractValue< sal_uInt8 >( nArea,   22, 3 ), extractValue< sal_uInt16 >( nArea,   25, 7 ) );
    maOoxData.maDiagonal.mbUsed = false;
}

void Border::setBiff8Data( sal_uInt32 nBorder1, sal_uInt32 nBorder2 )
{
    OSL_ENSURE( !mbDxf, "Border::setBiff8Data - unexpected conditional formatting flag" );
    maOoxData.maLeft.setBiffData(   extractValue< sal_uInt8 >( nBorder1,  0, 4 ), extractValue< sal_uInt16 >( nBorder1, 16, 7 ) );
    maOoxData.maRight.setBiffData(  extractValue< sal_uInt8 >( nBorder1,  4, 4 ), extractValue< sal_uInt16 >( nBorder1, 23, 7 ) );
    maOoxData.maTop.setBiffData(    extractValue< sal_uInt8 >( nBorder1,  8, 4 ), extractValue< sal_uInt16 >( nBorder2,  0, 7 ) );
    maOoxData.maBottom.setBiffData( extractValue< sal_uInt8 >( nBorder1, 12, 4 ), extractValue< sal_uInt16 >( nBorder2,  7, 7 ) );
    maOoxData.mbDiagTLtoBR = getFlag( nBorder1, BIFF_XF_DIAG_TLBR );
    maOoxData.mbDiagBLtoTR = getFlag( nBorder1, BIFF_XF_DIAG_BLTR );
    if( maOoxData.mbDiagTLtoBR || maOoxData.mbDiagBLtoTR )
        maOoxData.maDiagonal.setBiffData( extractValue< sal_uInt8 >( nBorder2, 21, 4 ), extractValue< sal_uInt16 >( nBorder2, 14, 7 ) );
}

void Border::importCfRule( BiffInputStream& rStrm, sal_uInt32 nFlags )
{
    OSL_ENSURE( mbDxf, "Border::importCfRule - missing conditional formatting flag" );
    OSL_ENSURE( getFlag( nFlags, BIFF_CFRULE_BORDERBLOCK ), "Border::importCfRule - missing border block flag" );
    sal_uInt16 nStyle;
    sal_uInt32 nColor;
    rStrm >> nStyle >> nColor;
    rStrm.skip( 2 );
    maOoxData.maLeft.setBiffData(   extractValue< sal_uInt8 >( nStyle,  0, 4 ), extractValue< sal_uInt16 >( nColor,  0, 7 ) );
    maOoxData.maRight.setBiffData(  extractValue< sal_uInt8 >( nStyle,  4, 4 ), extractValue< sal_uInt16 >( nColor,  7, 7 ) );
    maOoxData.maTop.setBiffData(    extractValue< sal_uInt8 >( nStyle,  8, 4 ), extractValue< sal_uInt16 >( nColor, 16, 7 ) );
    maOoxData.maBottom.setBiffData( extractValue< sal_uInt8 >( nStyle, 12, 4 ), extractValue< sal_uInt16 >( nColor, 23, 7 ) );
    maOoxData.maLeft.mbUsed   = !getFlag( nFlags, BIFF_CFRULE_BORDER_LEFT );
    maOoxData.maRight.mbUsed  = !getFlag( nFlags, BIFF_CFRULE_BORDER_RIGHT );
    maOoxData.maTop.mbUsed    = !getFlag( nFlags, BIFF_CFRULE_BORDER_TOP );
    maOoxData.maBottom.mbUsed = !getFlag( nFlags, BIFF_CFRULE_BORDER_BOTTOM );
}

void Border::finalizeImport()
{
    maApiData.mbBorderUsed = maOoxData.maLeft.mbUsed || maOoxData.maRight.mbUsed || maOoxData.maTop.mbUsed || maOoxData.maBottom.mbUsed;
    maApiData.mbDiagUsed   = maOoxData.maDiagonal.mbUsed;

    maApiData.maBorder.IsLeftLineValid   = convertBorderLine( maApiData.maBorder.LeftLine,   maOoxData.maLeft );
    maApiData.maBorder.IsRightLineValid  = convertBorderLine( maApiData.maBorder.RightLine,  maOoxData.maRight );
    maApiData.maBorder.IsTopLineValid    = convertBorderLine( maApiData.maBorder.TopLine,    maOoxData.maTop );
    maApiData.maBorder.IsBottomLineValid = convertBorderLine( maApiData.maBorder.BottomLine, maOoxData.maBottom );

    if( !mbDxf )
    {
        maApiData.maBorder.IsVerticalLineValid = maApiData.maBorder.IsLeftLineValid || maApiData.maBorder.IsRightLineValid;
        if( const BorderLine* pVertLine = lclGetThickerLine( maApiData.maBorder.LeftLine, maApiData.maBorder.IsLeftLineValid, maApiData.maBorder.RightLine, maApiData.maBorder.IsRightLineValid ) )
            maApiData.maBorder.VerticalLine = *pVertLine;

        maApiData.maBorder.IsHorizontalLineValid = maApiData.maBorder.IsTopLineValid || maApiData.maBorder.IsBottomLineValid;
        if( const BorderLine* pHorLine = lclGetThickerLine( maApiData.maBorder.TopLine, maApiData.maBorder.IsTopLineValid, maApiData.maBorder.BottomLine, maApiData.maBorder.IsBottomLineValid ) )
            maApiData.maBorder.HorizontalLine = *pHorLine;
    }

    if( maOoxData.mbDiagTLtoBR )
        convertBorderLine( maApiData.maTLtoBR, maOoxData.maDiagonal );
    if( maOoxData.mbDiagBLtoTR )
        convertBorderLine( maApiData.maBLtoTR, maOoxData.maDiagonal );
}

void Border::writeToPropertySet( PropertySet& rPropSet ) const
{
    getStylesPropertyHelper().writeBorderProperties( rPropSet, maApiData );
}

OoxBorderLineData* Border::getBorderLine( sal_Int32 nElement )
{
    switch( nElement )
    {
        case XLS_TOKEN( left ):     return &maOoxData.maLeft;
        case XLS_TOKEN( right ):    return &maOoxData.maRight;
        case XLS_TOKEN( top ):      return &maOoxData.maTop;
        case XLS_TOKEN( bottom ):   return &maOoxData.maBottom;
        case XLS_TOKEN( diagonal ): return &maOoxData.maDiagonal;
    }
    return 0;
}

bool Border::convertBorderLine( BorderLine& rBorderLine, const OoxBorderLineData& rLineData )
{
    rBorderLine.Color = getStyles().getColor( rLineData.maColor, API_RGB_BLACK );
    switch( rLineData.mnStyle )
    {
        case XML_dashDot:           lclSetBorderLineWidth( rBorderLine, API_LINE_THIN );    break;
        case XML_dashDotDot:        lclSetBorderLineWidth( rBorderLine, API_LINE_THIN );    break;
        case XML_dashed:            lclSetBorderLineWidth( rBorderLine, API_LINE_THIN );    break;
        case XML_dotted:            lclSetBorderLineWidth( rBorderLine, API_LINE_THIN );    break;
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
    return rLineData.mbUsed;
}


// ============================================================================

OoxPatternFillData::OoxPatternFillData( bool bDxf ) :
    maPatternColor( XML_indexed, OOX_COLOR_WINDOWTEXT ),
    maFillColor( XML_indexed, OOX_COLOR_WINDOWBACK ),
    mnPattern( XML_none ),
    mbPattColorUsed( !bDxf ),
    mbFillColorUsed( !bDxf ),
    mbPatternUsed( !bDxf )
{
}

void OoxPatternFillData::setBinPattern( sal_Int32 nPattern )
{
    static const sal_Int32 spnPatternIds[] = {
        XML_none, XML_solid, XML_mediumGray, XML_darkGray,
        XML_lightGray, XML_darkHorizontal, XML_darkVertical, XML_darkDown,
        XML_darkUp, XML_darkGrid, XML_darkTrellis, XML_lightHorizontal,
        XML_lightVertical, XML_lightDown, XML_lightUp, XML_lightGrid,
        XML_lightTrellis, XML_gray125, XML_gray0625 };
    mnPattern = STATIC_ARRAY_SELECT( spnPatternIds, nPattern, XML_none );
}

void OoxPatternFillData::setBiffData( sal_uInt16 nPatternColor, sal_uInt16 nFillColor, sal_uInt8 nPattern )
{
    maPatternColor.set( XML_indexed, static_cast< sal_Int32 >( nPatternColor ) );
    maFillColor.set( XML_indexed, static_cast< sal_Int32 >( nFillColor ) );
    // patterns equal in BIFF and OOBIN
    setBinPattern( nPattern );
}

// ----------------------------------------------------------------------------

OoxGradientFillData::OoxGradientFillData() :
    mnType( XML_linear ),
    mfAngle( 0.0 ),
    mfLeft( 0.0 ),
    mfRight( 0.0 ),
    mfTop( 0.0 ),
    mfBottom( 0.0 )
{
}

void OoxGradientFillData::readGradient( RecordInputStream& rStrm )
{
    sal_Int32 nType;
    rStrm >> nType >> mfAngle >> mfLeft >> mfRight >> mfTop >> mfBottom;
    static const sal_Int32 spnTypes[] = { XML_linear, XML_path };
    mnType = STATIC_ARRAY_SELECT( spnTypes, nType, XML_TOKEN_INVALID );
}

void OoxGradientFillData::readGradientStop( RecordInputStream& rStrm, bool bDxf )
{
    OoxColor aColor;
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
    if( rStrm.isValid() && (fPosition >= 0.0) )
        maColors[ fPosition ] = aColor;
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

bool Fill::isSupportedContext( sal_Int32 nElement, sal_Int32 nParentContext )
{
    switch( nParentContext )
    {
        case XLS_TOKEN( fill ):
            return  (nElement == XLS_TOKEN( patternFill )) ||
                    (nElement == XLS_TOKEN( gradientFill ));
        case XLS_TOKEN( patternFill ):
            return  (nElement == XLS_TOKEN( fgColor )) ||
                    (nElement == XLS_TOKEN( bgColor ));
        case XLS_TOKEN( gradientFill ):
            return  (nElement == XLS_TOKEN( stop ));
        case XLS_TOKEN( stop ):
            return  (nElement == XLS_TOKEN( color ));
    }
    return false;
}

void Fill::importPatternFill( const AttributeList& rAttribs )
{
    mxOoxPattData.reset( new OoxPatternFillData( mbDxf ) );
    mxOoxPattData->mnPattern = rAttribs.getToken( XML_patternType, XML_none );
    if( mbDxf )
        mxOoxPattData->mbPatternUsed = rAttribs.hasAttribute( XML_patternType );
}

void Fill::importFgColor( const AttributeList& rAttribs )
{
    OSL_ENSURE( mxOoxPattData.get(), "Fill::importFgColor - missing pattern data" );
    if( mxOoxPattData.get() )
    {
        mxOoxPattData->maPatternColor.importColor( rAttribs );
        mxOoxPattData->mbPattColorUsed = true;
    }
}

void Fill::importBgColor( const AttributeList& rAttribs )
{
    OSL_ENSURE( mxOoxPattData.get(), "Fill::importBgColor - missing pattern data" );
    if( mxOoxPattData.get() )
    {
        mxOoxPattData->maFillColor.importColor( rAttribs );
        mxOoxPattData->mbFillColorUsed = true;
    }
}

void Fill::importGradientFill( const AttributeList& rAttribs )
{
    mxOoxGradData.reset( new OoxGradientFillData );
    mxOoxGradData->mnType = rAttribs.getToken( XML_type, XML_linear );
    mxOoxGradData->mfAngle = rAttribs.getDouble( XML_degree, 0.0 );
    mxOoxGradData->mfLeft = rAttribs.getDouble( XML_left, 0.0 );
    mxOoxGradData->mfRight = rAttribs.getDouble( XML_right, 0.0 );
    mxOoxGradData->mfTop = rAttribs.getDouble( XML_top, 0.0 );
    mxOoxGradData->mfBottom = rAttribs.getDouble( XML_bottom, 0.0 );
}

void Fill::importColor( const AttributeList& rAttribs, double fPosition )
{
    OSL_ENSURE( mxOoxGradData.get(), "Fill::importColor - missing gradient data" );
    if( mxOoxGradData.get() && (fPosition >= 0.0) )
        mxOoxGradData->maColors[ fPosition ].importColor( rAttribs );
}

void Fill::importFill( RecordInputStream& rStrm )
{
    OSL_ENSURE( !mbDxf, "Fill::importFill - unexpected conditional formatting flag" );
    sal_Int32 nPattern = rStrm.readInt32();
    if( nPattern == OOBIN_FILL_GRADIENT )
    {
        mxOoxGradData.reset( new OoxGradientFillData );
        sal_Int32 nStopCount;
        rStrm.skip( 16 );
        mxOoxGradData->readGradient( rStrm );
        rStrm >> nStopCount;
        for( sal_Int32 nStop = 0; (nStop < nStopCount) && rStrm.isValid(); ++nStop )
            mxOoxGradData->readGradientStop( rStrm, false );
    }
    else
    {
        mxOoxPattData.reset( new OoxPatternFillData( mbDxf ) );
        mxOoxPattData->setBinPattern( nPattern );
        rStrm >> mxOoxPattData->maPatternColor >> mxOoxPattData->maFillColor;
    }
}

void Fill::importDxfPattern( RecordInputStream& rStrm )
{
    OSL_ENSURE( mbDxf, "Fill::importDxfPattern - missing conditional formatting flag" );
    if( !mxOoxPattData )
        mxOoxPattData.reset( new OoxPatternFillData( mbDxf ) );
    mxOoxPattData->setBinPattern( rStrm.readuInt8() );
    mxOoxPattData->mbPatternUsed = true;
}

void Fill::importDxfFgColor( RecordInputStream& rStrm )
{
    OSL_ENSURE( mbDxf, "Fill::importDxfFgColor - missing conditional formatting flag" );
    if( !mxOoxPattData )
        mxOoxPattData.reset( new OoxPatternFillData( mbDxf ) );
    mxOoxPattData->maPatternColor.importColor( rStrm );
    mxOoxPattData->mbPattColorUsed = true;
}

void Fill::importDxfBgColor( RecordInputStream& rStrm )
{
    OSL_ENSURE( mbDxf, "Fill::importDxfBgColor - missing conditional formatting flag" );
    if( !mxOoxPattData )
        mxOoxPattData.reset( new OoxPatternFillData( mbDxf ) );
    mxOoxPattData->maFillColor.importColor( rStrm );
    mxOoxPattData->mbFillColorUsed = true;
}

void Fill::importDxfGradient( RecordInputStream& rStrm )
{
    OSL_ENSURE( mbDxf, "Fill::importDxfGradient - missing conditional formatting flag" );
    if( !mxOoxGradData )
        mxOoxGradData.reset( new OoxGradientFillData );
    mxOoxGradData->readGradient( rStrm );
}

void Fill::importDxfStop( RecordInputStream& rStrm )
{
    OSL_ENSURE( mbDxf, "Fill::importDxfStop - missing conditional formatting flag" );
    if( !mxOoxGradData )
        mxOoxGradData.reset( new OoxGradientFillData );
    mxOoxGradData->readGradientStop( rStrm, true );
}

void Fill::setBiff2Data( sal_uInt8 nFlags )
{
    OSL_ENSURE( !mbDxf, "Fill::setBiff2Data - unexpected conditional formatting flag" );
    mxOoxPattData.reset( new OoxPatternFillData( mbDxf ) );
    mxOoxPattData->setBiffData(
        BIFF2_COLOR_BLACK,
        BIFF2_COLOR_WHITE,
        getFlagValue( nFlags, BIFF2_XF_BACKGROUND, BIFF_PATT_125, BIFF_PATT_NONE ) );
}

void Fill::setBiff3Data( sal_uInt16 nArea )
{
    OSL_ENSURE( !mbDxf, "Fill::setBiff3Data - unexpected conditional formatting flag" );
    mxOoxPattData.reset( new OoxPatternFillData( mbDxf ) );
    mxOoxPattData->setBiffData(
        extractValue< sal_uInt16 >( nArea, 6, 5 ),
        extractValue< sal_uInt16 >( nArea, 11, 5 ),
        extractValue< sal_uInt8 >( nArea, 0, 6 ) );
}

void Fill::setBiff5Data( sal_uInt32 nArea )
{
    OSL_ENSURE( !mbDxf, "Fill::setBiff5Data - unexpected conditional formatting flag" );
    mxOoxPattData.reset( new OoxPatternFillData( mbDxf ) );
    mxOoxPattData->setBiffData(
        extractValue< sal_uInt16 >( nArea, 0, 7 ),
        extractValue< sal_uInt16 >( nArea, 7, 7 ),
        extractValue< sal_uInt8 >( nArea, 16, 6 ) );
}

void Fill::setBiff8Data( sal_uInt32 nBorder2, sal_uInt16 nArea )
{
    OSL_ENSURE( !mbDxf, "Fill::setBiff8Data - unexpected conditional formatting flag" );
    mxOoxPattData.reset( new OoxPatternFillData( mbDxf ) );
    mxOoxPattData->setBiffData(
        extractValue< sal_uInt16 >( nArea, 0, 7 ),
        extractValue< sal_uInt16 >( nArea, 7, 7 ),
        extractValue< sal_uInt8 >( nBorder2, 26, 6 ) );
}

void Fill::importCfRule( BiffInputStream& rStrm, sal_uInt32 nFlags )
{
    OSL_ENSURE( mbDxf, "Fill::importCfRule - missing conditional formatting flag" );
    OSL_ENSURE( getFlag( nFlags, BIFF_CFRULE_FILLBLOCK ), "Fill::importCfRule - missing fill block flag" );
    mxOoxPattData.reset( new OoxPatternFillData( mbDxf ) );
    sal_uInt32 nFillData;
    rStrm >> nFillData;
    mxOoxPattData->setBiffData(
        extractValue< sal_uInt16 >( nFillData, 16, 7 ),
        extractValue< sal_uInt16 >( nFillData, 23, 7 ),
        extractValue< sal_uInt8 >( nFillData, 10, 6 ) );
    mxOoxPattData->mbPattColorUsed = !getFlag( nFlags, BIFF_CFRULE_FILL_PATTCOLOR );
    mxOoxPattData->mbFillColorUsed = !getFlag( nFlags, BIFF_CFRULE_FILL_FILLCOLOR );
    mxOoxPattData->mbPatternUsed   = !getFlag( nFlags, BIFF_CFRULE_FILL_PATTERN );
}

void Fill::finalizeImport()
{
    if( mxOoxPattData.get() )
    {
        // finalize the OOX data struct
        OoxPatternFillData& rOoxData = *mxOoxPattData;
        if( mbDxf )
        {
            if( rOoxData.mbFillColorUsed && (!rOoxData.mbPatternUsed || (rOoxData.mnPattern == XML_solid)) )
            {
                rOoxData.maPatternColor = rOoxData.maFillColor;
                rOoxData.mnPattern = XML_solid;
                rOoxData.mbPattColorUsed = rOoxData.mbPatternUsed = true;
            }
            else if( !rOoxData.mbFillColorUsed && rOoxData.mbPatternUsed && (rOoxData.mnPattern == XML_solid) )
            {
                rOoxData.mbPatternUsed = false;
            }
        }

        // convert to API fill settings
        maApiData.mbUsed = rOoxData.mbPatternUsed;
        if( rOoxData.mnPattern == XML_none )
        {
            maApiData.mnColor = API_RGB_TRANSPARENT;
            maApiData.mbTransparent = true;
        }
        else
        {
            sal_Int32 nAlpha = 0x80;
            switch( rOoxData.mnPattern )
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

            if( !rOoxData.mbPattColorUsed )
                rOoxData.maPatternColor.set( XML_auto, 0 );
            sal_Int32 nPattColor = getStyles().getColor(
                rOoxData.maPatternColor, ThemeBuffer::getSystemWindowTextColor() );

            if( !rOoxData.mbFillColorUsed )
                rOoxData.maFillColor.set( XML_auto, 0 );
            sal_Int32 nFillColor = getStyles().getColor(
                rOoxData.maFillColor, ThemeBuffer::getSystemWindowColor() );

            maApiData.mnColor = lclGetMixedColor( nPattColor, nFillColor, nAlpha );
            maApiData.mbTransparent = false;
        }
    }
    else if( mxOoxGradData.get() && !mxOoxGradData->maColors.empty() )
    {
        OoxGradientFillData& rOoxData = *mxOoxGradData;
        maApiData.mbUsed = true;    // no support for differential attributes
        OoxGradientFillData::OoxColorMap::const_iterator aIt = rOoxData.maColors.begin();
        OSL_ENSURE( !aIt->second.isAuto(), "Fill::finalizeImport - automatic gradient color" );
        maApiData.mnColor = getStyles().getColor( aIt->second, API_RGB_TRANSPARENT );
        if( ++aIt != rOoxData.maColors.end() )
        {
            OSL_ENSURE( !aIt->second.isAuto(), "Fill::finalizeImport - automatic gradient color" );
            sal_Int32 nEndColor = getStyles().getColor( aIt->second, API_RGB_TRANSPARENT );
            maApiData.mnColor = lclGetMixedColor( maApiData.mnColor, nEndColor, 0x40 );
            maApiData.mbTransparent = false;
        }
    }
}

void Fill::writeToPropertySet( PropertySet& rPropSet ) const
{
    getStylesPropertyHelper().writeSolidFillProperties( rPropSet, maApiData );
}

// ============================================================================

OoxXfData::OoxXfData() :
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
    maProtection( rHelper )
{
}

void Xf::setAllUsedFlags( bool bUsed )
{
    maOoxData.mbAlignUsed = maOoxData.mbProtUsed = maOoxData.mbFontUsed =
        maOoxData.mbNumFmtUsed = maOoxData.mbBorderUsed = maOoxData.mbAreaUsed = bUsed;
}

void Xf::importXf( const AttributeList& rAttribs, bool bCellXf )
{
    maOoxData.mbCellXf = bCellXf;
    maOoxData.mnStyleXfId = rAttribs.getInteger( XML_xfId, -1 );
    maOoxData.mnFontId = rAttribs.getInteger( XML_fontId, -1 );
    maOoxData.mnNumFmtId = rAttribs.getInteger( XML_numFmtId, -1 );
    maOoxData.mnBorderId = rAttribs.getInteger( XML_borderId, -1 );
    maOoxData.mnFillId = rAttribs.getInteger( XML_fillId, -1 );

    /*  Default value of the apply*** attributes is dependent on context:
        true in cellStyleXfs element, false in cellXfs element... */
    maOoxData.mbAlignUsed  = rAttribs.getBool( XML_applyAlignment,    !maOoxData.mbCellXf );
    maOoxData.mbProtUsed   = rAttribs.getBool( XML_applyProtection,   !maOoxData.mbCellXf );
    maOoxData.mbFontUsed   = rAttribs.getBool( XML_applyFont,         !maOoxData.mbCellXf );
    maOoxData.mbNumFmtUsed = rAttribs.getBool( XML_applyNumberFormat, !maOoxData.mbCellXf );
    maOoxData.mbBorderUsed = rAttribs.getBool( XML_applyBorder,       !maOoxData.mbCellXf );
    maOoxData.mbAreaUsed   = rAttribs.getBool( XML_applyFill,         !maOoxData.mbCellXf );
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
    maOoxData.mbCellXf = bCellXf;
    maOoxData.mnStyleXfId = rStrm.readuInt16();
    maOoxData.mnNumFmtId = rStrm.readuInt16();
    maOoxData.mnFontId = rStrm.readuInt16();
    maOoxData.mnFillId = rStrm.readuInt16();
    maOoxData.mnBorderId = rStrm.readuInt16();
    sal_uInt32 nFlags = rStrm.readuInt32();
    maAlignment.setBinData( nFlags );
    maProtection.setBinData( nFlags );
    // used flags, see comments in Xf::setBiffUsedFlags()
    sal_uInt16 nUsedFlags = rStrm.readuInt16();
    maOoxData.mbFontUsed   = maOoxData.mbCellXf == getFlag( nUsedFlags, OOBIN_XF_FONT_USED );
    maOoxData.mbNumFmtUsed = maOoxData.mbCellXf == getFlag( nUsedFlags, OOBIN_XF_NUMFMT_USED );
    maOoxData.mbAlignUsed  = maOoxData.mbCellXf == getFlag( nUsedFlags, OOBIN_XF_ALIGN_USED );
    maOoxData.mbProtUsed   = maOoxData.mbCellXf == getFlag( nUsedFlags, OOBIN_XF_PROT_USED );
    maOoxData.mbBorderUsed = maOoxData.mbCellXf == getFlag( nUsedFlags, OOBIN_XF_BORDER_USED );
    maOoxData.mbAreaUsed   = maOoxData.mbCellXf == getFlag( nUsedFlags, OOBIN_XF_AREA_USED );
}

void Xf::importXf( BiffInputStream& rStrm )
{
    BorderRef xBorder = getStyles().createBorder( &maOoxData.mnBorderId );
    FillRef xFill = getStyles().createFill( &maOoxData.mnFillId );

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
            maOoxData.mnFontId = static_cast< sal_Int32 >( nFontId );
            maOoxData.mnNumFmtId = static_cast< sal_Int32 >( nNumFmtId & BIFF2_XF_VALFMT_MASK );
        }
        break;

        case BIFF3:
        {
            sal_uInt32 nBorder;
            sal_uInt16 nTypeProt, nAlign, nArea;
            sal_uInt8 nFontId, nNumFmtId;
            rStrm >> nFontId >> nNumFmtId >> nTypeProt >> nAlign >> nArea >> nBorder;

            // XF type/parent
            maOoxData.mbCellXf = !getFlag( nTypeProt, BIFF_XF_STYLE ); // new in BIFF3
            maOoxData.mnStyleXfId = extractValue< sal_Int32 >( nAlign, 4, 12 ); // new in BIFF3
            // attribute used flags
            setBiffUsedFlags( extractValue< sal_uInt8 >( nTypeProt, 10, 6 ) ); // new in BIFF3

            // attributes
            maAlignment.setBiff3Data( nAlign );
            maProtection.setBiff3Data( nTypeProt );
            xBorder->setBiff3Data( nBorder );
            xFill->setBiff3Data( nArea );
            maOoxData.mnFontId = static_cast< sal_Int32 >( nFontId );
            maOoxData.mnNumFmtId = static_cast< sal_Int32 >( nNumFmtId );
        }
        break;

        case BIFF4:
        {
            sal_uInt32 nBorder;
            sal_uInt16 nTypeProt, nAlign, nArea;
            sal_uInt8 nFontId, nNumFmtId;
            rStrm >> nFontId >> nNumFmtId >> nTypeProt >> nAlign >> nArea >> nBorder;

            // XF type/parent
            maOoxData.mbCellXf = !getFlag( nTypeProt, BIFF_XF_STYLE );
            maOoxData.mnStyleXfId = extractValue< sal_Int32 >( nTypeProt, 4, 12 );
            // attribute used flags
            setBiffUsedFlags( extractValue< sal_uInt8 >( nAlign, 10, 6 ) );

            // attributes
            maAlignment.setBiff4Data( nAlign );
            maProtection.setBiff3Data( nTypeProt );
            xBorder->setBiff3Data( nBorder );
            xFill->setBiff3Data( nArea );
            maOoxData.mnFontId = static_cast< sal_Int32 >( nFontId );
            maOoxData.mnNumFmtId = static_cast< sal_Int32 >( nNumFmtId );
        }
        break;

        case BIFF5:
        {
            sal_uInt32 nArea, nBorder;
            sal_uInt16 nFontId, nNumFmtId, nTypeProt, nAlign;
            rStrm >> nFontId >> nNumFmtId >> nTypeProt >> nAlign >> nArea >> nBorder;

            // XF type/parent
            maOoxData.mbCellXf = !getFlag( nTypeProt, BIFF_XF_STYLE );
            maOoxData.mnStyleXfId = extractValue< sal_Int32 >( nTypeProt, 4, 12 );
            // attribute used flags
            setBiffUsedFlags( extractValue< sal_uInt8 >( nAlign, 10, 6 ) );

            // attributes
            maAlignment.setBiff5Data( nAlign );
            maProtection.setBiff3Data( nTypeProt );
            xBorder->setBiff5Data( nBorder, nArea );
            xFill->setBiff5Data( nArea );
            maOoxData.mnFontId = static_cast< sal_Int32 >( nFontId );
            maOoxData.mnNumFmtId = static_cast< sal_Int32 >( nNumFmtId );
        }
        break;

        case BIFF8:
        {
            sal_uInt32 nBorder1, nBorder2;
            sal_uInt16 nFontId, nNumFmtId, nTypeProt, nAlign, nMiscAttrib, nArea;
            rStrm >> nFontId >> nNumFmtId >> nTypeProt >> nAlign >> nMiscAttrib >> nBorder1 >> nBorder2 >> nArea;

            // XF type/parent
            maOoxData.mbCellXf = !getFlag( nTypeProt, BIFF_XF_STYLE );
            maOoxData.mnStyleXfId = extractValue< sal_Int32 >( nTypeProt, 4, 12 );
            // attribute used flags
            setBiffUsedFlags( extractValue< sal_uInt8 >( nMiscAttrib, 10, 6 ) );

            // attributes
            maAlignment.setBiff8Data( nAlign, nMiscAttrib );
            maProtection.setBiff3Data( nTypeProt );
            xBorder->setBiff8Data( nBorder1, nBorder2 );
            xFill->setBiff8Data( nBorder2, nArea );
            maOoxData.mnFontId = static_cast< sal_Int32 >( nFontId );
            maOoxData.mnNumFmtId = static_cast< sal_Int32 >( nNumFmtId );
        }
        break;

        case BIFF_UNKNOWN: break;
    }
}

void Xf::finalizeImport()
{
    // alignment and protection
    maAlignment.finalizeImport();
    maProtection.finalizeImport();
    // update used flags from cell style
    if( maOoxData.mbCellXf )
        if( const Xf* pStyleXf = getStyles().getStyleXf( maOoxData.mnStyleXfId ).get() )
            updateUsedFlags( *pStyleXf );
}

FontRef Xf::getFont() const
{
    return getStyles().getFont( maOoxData.mnFontId );
}

bool Xf::hasAnyUsedFlags() const
{
    return
        maOoxData.mbAlignUsed || maOoxData.mbProtUsed || maOoxData.mbFontUsed ||
        maOoxData.mbNumFmtUsed || maOoxData.mbBorderUsed || maOoxData.mbAreaUsed;
}

void Xf::writeToPropertySet( PropertySet& rPropSet ) const
{
    StylesBuffer& rStyles = getStyles();

    // create and set cell style
    if( maOoxData.mbCellXf )
    {
        const OUString& rStyleName = rStyles.createCellStyle( maOoxData.mnStyleXfId );
        rPropSet.setProperty( CREATE_OUSTRING( "CellStyle" ), rStyleName );
    }

    if( maOoxData.mbFontUsed )
        rStyles.writeFontToPropertySet( rPropSet, maOoxData.mnFontId );
    if( maOoxData.mbNumFmtUsed )
        rStyles.writeNumFmtToPropertySet( rPropSet, maOoxData.mnNumFmtId );
    if( maOoxData.mbAlignUsed )
        maAlignment.writeToPropertySet( rPropSet );
    if( maOoxData.mbProtUsed )
        maProtection.writeToPropertySet( rPropSet );
    if( maOoxData.mbBorderUsed )
        rStyles.writeBorderToPropertySet( rPropSet, maOoxData.mnBorderId );
    if( maOoxData.mbAreaUsed )
        rStyles.writeFillToPropertySet( rPropSet, maOoxData.mnFillId );
}

void Xf::setBiffUsedFlags( sal_uInt8 nUsedFlags )
{
    /*  Notes about finding the used flags:
        - In cell XFs a *set* bit means a used attribute.
        - In style XFs a *cleared* bit means a used attribute.
        The boolean flags always store true, if the attribute is used.
        The "maOoxData.mbCellXf == getFlag(...)" construct evaluates to true in
        both mentioned cases: cell XF and set bit; or style XF and cleared bit.
     */
    maOoxData.mbFontUsed   = maOoxData.mbCellXf == getFlag( nUsedFlags, BIFF_XF_FONT_USED );
    maOoxData.mbNumFmtUsed = maOoxData.mbCellXf == getFlag( nUsedFlags, BIFF_XF_NUMFMT_USED );
    maOoxData.mbAlignUsed  = maOoxData.mbCellXf == getFlag( nUsedFlags, BIFF_XF_ALIGN_USED );
    maOoxData.mbProtUsed   = maOoxData.mbCellXf == getFlag( nUsedFlags, BIFF_XF_PROT_USED );
    maOoxData.mbBorderUsed = maOoxData.mbCellXf == getFlag( nUsedFlags, BIFF_XF_BORDER_USED );
    maOoxData.mbAreaUsed   = maOoxData.mbCellXf == getFlag( nUsedFlags, BIFF_XF_AREA_USED );
}

void Xf::updateUsedFlags( const Xf& rStyleXf )
{
    /*  Enables the used flags, if the formatting attributes differ from the
        passed style XF. In cell XFs Excel uses the cell attributes, if they
        differ from the parent style XF.
        #109899# ...or if the respective flag is not set in parent style XF.
     */
    const OoxXfData& rStyleData = rStyleXf.maOoxData;
    if( !maOoxData.mbFontUsed )
        maOoxData.mbFontUsed = !rStyleData.mbFontUsed || (maOoxData.mnFontId != rStyleData.mnFontId);
    if( !maOoxData.mbNumFmtUsed )
        maOoxData.mbNumFmtUsed = !rStyleData.mbNumFmtUsed || (maOoxData.mnNumFmtId != rStyleData.mnNumFmtId);
    if( !maOoxData.mbAlignUsed )
        maOoxData.mbAlignUsed = !rStyleData.mbAlignUsed || !(maAlignment.getApiData() == rStyleXf.maAlignment.getApiData());
    if( !maOoxData.mbProtUsed )
        maOoxData.mbProtUsed = !rStyleData.mbProtUsed || !(maProtection.getApiData() == rStyleXf.maProtection.getApiData());
    if( !maOoxData.mbBorderUsed )
        maOoxData.mbBorderUsed = !rStyleData.mbBorderUsed || (maOoxData.mnBorderId != rStyleData.mnBorderId);
    if( !maOoxData.mbAreaUsed )
        maOoxData.mbAreaUsed = !rStyleData.mbAreaUsed || (maOoxData.mnFillId != rStyleData.mnFillId);
}

// ============================================================================

Dxf::Dxf( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
}

FontRef Dxf::importFont( const AttributeList& )
{
    createFont( true );
    return mxFont;
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

BorderRef Dxf::importBorder( const AttributeList& rAttribs )
{
    createBorder( true );
    mxBorder->importBorder( rAttribs );
    return mxBorder;
}

FillRef Dxf::importFill( const AttributeList& )
{
    createFill( true );
    return mxFill;
}

void Dxf::importDxf( RecordInputStream& rStrm )
{
    sal_Int32 nNumFmtId = -1;
    OUString aFmtCode;
    sal_uInt16 nRecCount;
    rStrm.skip( 4 );    // flags
    rStrm >> nRecCount;
    for( sal_uInt16 nRec = 0; rStrm.isValid() && (nRec < nRecCount); ++nRec )
    {
        sal_uInt16 nSubRecId, nSubRecSize;
        sal_Int32 nRecEnd = rStrm.getRecPos();
        rStrm >> nSubRecId >> nSubRecSize;
        nRecEnd += nSubRecSize;
        switch( nSubRecId )
        {
            case OOBIN_DXF_FILL_PATTERN:    createFill( false ); mxFill->importDxfPattern( rStrm );                         break;
            case OOBIN_DXF_FILL_FGCOLOR:    createFill( false ); mxFill->importDxfFgColor( rStrm );                         break;
            case OOBIN_DXF_FILL_BGCOLOR:    createFill( false ); mxFill->importDxfBgColor( rStrm );                         break;
            case OOBIN_DXF_FILL_GRADIENT:   createFill( false ); mxFill->importDxfGradient( rStrm );                        break;
            case OOBIN_DXF_FILL_STOP:       createFill( false ); mxFill->importDxfStop( rStrm );                            break;
            case OOBIN_DXF_FONT_COLOR:      createFont( false ); mxFont->importDxfColor( rStrm );                           break;
            case OOBIN_DXF_BORDER_TOP:      createBorder( false ); mxBorder->importDxfBorder( XLS_TOKEN( top ), rStrm );    break;
            case OOBIN_DXF_BORDER_BOTTOM:   createBorder( false ); mxBorder->importDxfBorder( XLS_TOKEN( bottom ), rStrm ); break;
            case OOBIN_DXF_BORDER_LEFT:     createBorder( false ); mxBorder->importDxfBorder( XLS_TOKEN( left ), rStrm );   break;
            case OOBIN_DXF_BORDER_RIGHT:    createBorder( false ); mxBorder->importDxfBorder( XLS_TOKEN( right ), rStrm );  break;
            case OOBIN_DXF_FONT_NAME:       createFont( false ); mxFont->importDxfName( rStrm );                            break;
            case OOBIN_DXF_FONT_WEIGHT:     createFont( false ); mxFont->importDxfWeight( rStrm );                          break;
            case OOBIN_DXF_FONT_UNDERLINE:  createFont( false ); mxFont->importDxfUnderline( rStrm );                       break;
            case OOBIN_DXF_FONT_ESCAPEMENT: createFont( false ); mxFont->importDxfEscapement( rStrm );                      break;
            case OOBIN_DXF_FONT_ITALIC:     createFont( false ); mxFont->importDxfFlag( XML_i, rStrm );                     break;
            case OOBIN_DXF_FONT_STRIKE:     createFont( false ); mxFont->importDxfFlag( XML_strike, rStrm );                break;
            case OOBIN_DXF_FONT_OUTLINE:    createFont( false ); mxFont->importDxfFlag( XML_outline, rStrm );               break;
            case OOBIN_DXF_FONT_SHADOW:     createFont( false ); mxFont->importDxfFlag( XML_shadow, rStrm );                break;
            case OOBIN_DXF_FONT_HEIGHT:     createFont( false ); mxFont->importDxfHeight( rStrm );                          break;
            case OOBIN_DXF_FONT_SCHEME:     createFont( false ); mxFont->importDxfScheme( rStrm );                          break;
            case OOBIN_DXF_NUMFMT_CODE:     aFmtCode = rStrm.readString( false );                                           break;
            case OOBIN_DXF_NUMFMT_ID:       nNumFmtId = rStrm.readuInt16();                                                 break;
        }
        rStrm.seek( nRecEnd );
    }
    OSL_ENSURE( rStrm.isValid() && (rStrm.getRecLeft() == 0), "Dxf::importDxf - unexpected remaining data" );
    mxNumFmt = getStyles().createNumFmt( nNumFmtId, aFmtCode );
}

void Dxf::importCfRule( BiffInputStream& rStrm, sal_uInt32 nFlags )
{
    if( getFlag( nFlags, BIFF_CFRULE_FONTBLOCK ) )
    {
        createFont( true );
        mxFont->importCfRule( rStrm );
    }
    if( getFlag( nFlags, BIFF_CFRULE_ALIGNBLOCK ) )
    {
        rStrm.skip( 8 );
    }
    if( getFlag( nFlags, BIFF_CFRULE_BORDERBLOCK ) )
    {
        createBorder( true );
        mxBorder->importCfRule( rStrm, nFlags );
    }
    if( getFlag( nFlags, BIFF_CFRULE_FILLBLOCK ) )
    {
        createFill( true );
        mxFill->importCfRule( rStrm, nFlags );
    }
    if( getFlag( nFlags, BIFF_CFRULE_PROTBLOCK ) )
    {
        rStrm.skip( 2 );
    }
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

const OUString& Dxf::createDxfStyle( sal_Int32 nDxfId )
{
    if( maFinalName.getLength() == 0 )
    {
        maFinalName = OUStringBuffer( CREATE_OUSTRING( "ConditionalStyle_" ) ).append( nDxfId + 1 ).makeStringAndClear();
        Reference< XStyle > xStyle = createStyleObject( maFinalName, false );
        // write style formatting properties
        PropertySet aPropSet( xStyle );
        if( mxFont.get() )
            mxFont->writeToPropertySet( aPropSet, FONT_PROPTYPE_CELL );
        if( mxNumFmt.get() )
            mxNumFmt->writeToPropertySet( aPropSet );
        if( mxAlignment.get() )
            mxAlignment->writeToPropertySet( aPropSet );
        if( mxProtection.get() )
            mxProtection->writeToPropertySet( aPropSet );
        if( mxBorder.get() )
            mxBorder->writeToPropertySet( aPropSet );
        if( mxFill.get() )
            mxFill->writeToPropertySet( aPropSet );
    }
    return maFinalName;
}

void Dxf::createFont( bool bAlwaysNew )
{
    if( bAlwaysNew || !mxFont )
        mxFont.reset( new Font( *this, true ) );
}

void Dxf::createBorder( bool bAlwaysNew )
{
    if( bAlwaysNew || !mxBorder )
        mxBorder.reset( new Border( *this, true ) );
}

void Dxf::createFill( bool bAlwaysNew )
{
    if( bAlwaysNew || !mxFill )
        mxFill.reset( new Fill( *this, true ) );
}

// ============================================================================

namespace {

const sal_Char* const spcLegacyStyleNamePrefix = "Excel_BuiltIn_";
const sal_Char* const sppcLegacyStyleNames[] =
{
#if OOX_XLS_USE_DEFAULT_STYLE
    "",                     // use existing "Default" style
#else
    "Normal",
#endif
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
#if OOX_XLS_USE_DEFAULT_STYLE
    "",                     // use existing "Default" style
#else
    "Normal",
#endif
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

#if OOX_XLS_USE_DEFAULT_STYLE
const sal_Char* const spcDefaultStyleName = "Default";
#endif

OUString lclGetBuiltinStyleName( sal_Int32 nBuiltinId, const OUString& rName, sal_Int32 nLevel = 0 )
{
    OUStringBuffer aStyleName;
    OSL_ENSURE( (0 <= nBuiltinId) && (nBuiltinId < snStyleNamesCount), "lclGetBuiltinStyleName - unknown builtin style" );
#if OOX_XLS_USE_DEFAULT_STYLE
    if( nBuiltinId == OOX_STYLE_NORMAL )    // "Normal" becomes "Default" style
    {
        aStyleName.appendAscii( spcDefaultStyleName );
    }
    else
    {
#endif
        aStyleName.appendAscii( spcStyleNamePrefix );
        if( (0 <= nBuiltinId) && (nBuiltinId < snStyleNamesCount) && (sppcStyleNames[ nBuiltinId ][ 0 ] != 0) )
            aStyleName.appendAscii( sppcStyleNames[ nBuiltinId ] );
        else if( rName.getLength() > 0 )
            aStyleName.append( rName );
        else
            aStyleName.append( nBuiltinId );
        if( (nBuiltinId == OOX_STYLE_ROWLEVEL) || (nBuiltinId == OOX_STYLE_COLLEVEL) )
            aStyleName.append( nLevel );
#if OOX_XLS_USE_DEFAULT_STYLE
    }
#endif
    return aStyleName.makeStringAndClear();
}

bool lclIsBuiltinStyleName( const OUString& rStyleName, sal_Int32* pnBuiltinId, sal_Int32* pnNextChar )
{
#if OOX_XLS_USE_DEFAULT_STYLE
    // "Default" becomes "Normal"
    if( rStyleName.equalsIgnoreAsciiCaseAscii( spcDefaultStyleName ) )
    {
        if( pnBuiltinId ) *pnBuiltinId = OOX_STYLE_NORMAL;
        if( pnNextChar ) *pnNextChar = rStyleName.getLength();
        return true;
    }
#endif

    // try the other builtin styles
    OUString aPrefix = OUString::createFromAscii( spcStyleNamePrefix );
    sal_Int32 nPrefixLen = aPrefix.getLength();
    sal_Int32 nFoundId = 0;
    sal_Int32 nNextChar = 0;
    if( rStyleName.matchIgnoreAsciiCase( aPrefix ) )
    {
        OUString aShortName;
        for( sal_Int32 nId = 0; nId < snStyleNamesCount; ++nId )
        {
#if OOX_XLS_USE_DEFAULT_STYLE
            if( nId != OOX_STYLE_NORMAL )
            {
#endif
                aShortName = OUString::createFromAscii( sppcStyleNames[ nId ] );
                if( rStyleName.matchIgnoreAsciiCase( aShortName, nPrefixLen ) &&
                        (nNextChar < nPrefixLen + aShortName.getLength()) )
                {
                    nFoundId = nId;
                    nNextChar = nPrefixLen + aShortName.getLength();
                }
#if OOX_XLS_USE_DEFAULT_STYLE
            }
#endif
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

OoxCellStyleData::OoxCellStyleData() :
    mnXfId( -1 ),
    mnBuiltinId( -1 ),
    mnLevel( 0 ),
    mbBuiltin( false ),
    mbCustom( false ),
    mbHidden( false )
{
}

bool OoxCellStyleData::isDefaultStyle() const
{
    return mbBuiltin && (mnBuiltinId == OOX_STYLE_NORMAL);
}

OUString OoxCellStyleData::createStyleName() const
{
    return isBuiltin() ? lclGetBuiltinStyleName( mnBuiltinId, maName, mnLevel ) : maName;
}

// ============================================================================

CellStyle::CellStyle( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
}

void CellStyle::importCellStyle( const AttributeList& rAttribs )
{
    maOoxData.maName = rAttribs.getString( XML_name );
    maOoxData.mnXfId = rAttribs.getInteger( XML_xfId, -1 );
    maOoxData.mnBuiltinId = rAttribs.getInteger( XML_builtinId, -1 );
    maOoxData.mnLevel = rAttribs.getInteger( XML_iLevel, 0 );
    maOoxData.mbBuiltin = rAttribs.hasAttribute( XML_builtinId );
    maOoxData.mbCustom = rAttribs.getBool( XML_customBuiltin, false );
    maOoxData.mbHidden = rAttribs.getBool( XML_hidden, false );
}

void CellStyle::importCellStyle( RecordInputStream& rStrm )
{
    sal_uInt16 nFlags;
    rStrm >> maOoxData.mnXfId >> nFlags;
    maOoxData.mnBuiltinId = rStrm.readuInt8();
    maOoxData.mnLevel = rStrm.readuInt8();
    rStrm >> maOoxData.maName;
    maOoxData.mbBuiltin = getFlag( nFlags, OOBIN_CELLSTYLE_BUILTIN );
    maOoxData.mbCustom = getFlag( nFlags, OOBIN_CELLSTYLE_CUSTOM );
    maOoxData.mbHidden = getFlag( nFlags, OOBIN_CELLSTYLE_HIDDEN );
}

void CellStyle::importStyle( BiffInputStream& rStrm )
{
    sal_uInt16 nStyleXf;
    rStrm >> nStyleXf;
    maOoxData.mnXfId = static_cast< sal_Int32 >( nStyleXf & BIFF_STYLE_XFMASK );
    maOoxData.mbBuiltin = getFlag( nStyleXf, BIFF_STYLE_BUILTIN );
    if( maOoxData.mbBuiltin )
    {
        maOoxData.mnBuiltinId = rStrm.readuInt8();
        maOoxData.mnLevel = rStrm.readuInt8();
    }
    else
    {
        maOoxData.maName = (getBiff() == BIFF8) ?
            rStrm.readUniString() : rStrm.readByteString( false, getTextEncoding() );
    }
}

const OUString& CellStyle::createCellStyle( sal_Int32 nXfId, bool bSkipDefaultBuiltin )
{
    if( maFinalName.getLength() == 0 )
    {
        bool bBuiltin = maOoxData.isBuiltin();
        if( !bSkipDefaultBuiltin || !bBuiltin || maOoxData.mbCustom )
        {
            // name of the style (generate unique name for builtin styles)
            maFinalName = maOoxData.createStyleName();
            // #i1624# #i1768# ignore unnamed user styles
            if( maFinalName.getLength() > 0 )
            {
                Reference< XStyle > xStyle;
#if OOX_XLS_USE_DEFAULT_STYLE
                // special handling for default style (do not create, but use existing)
                if( isDefaultStyle() )
                {
                    /*  Set all flags to true to have all properties in the style,
                        even if the used flags are not set (that's what Excel does). */
                    if( Xf* pXf = getStyles().getStyleXf( nXfId ).get() )
                        pXf->setAllUsedFlags( true );
                    // use existing built-in style
                    xStyle = getStyleObject( maFinalName, false );
                }
                else
                {
#endif
                    /*  Insert into cell styles collection, rename existing user styles,
                        if this is a built-in style, but do not do this in BIFF4 workspace
                        files, where built-in styles occur repeatedly. */
                    bool bRenameExisting = bBuiltin && (getBiff() != BIFF4);
                    xStyle = createStyleObject( maFinalName, false, bRenameExisting );
#if OOX_XLS_USE_DEFAULT_STYLE
                }
#endif

                // write style formatting properties
                PropertySet aPropSet( xStyle );
                getStyles().writeStyleXfToPropertySet( aPropSet, nXfId );
#if OOX_XLS_USE_DEFAULT_STYLE
#else
                if( !isDefaultStyle() && xStyle.is() )
                    xStyle->setParentStyle( getStyles().getDefaultStyleName() );
#endif
            }
        }
    }
    return maFinalName;
}

// ============================================================================

namespace {

sal_Int32 lclTintToColor( sal_Int32 nColor, double fTint )
{
    if( nColor == 0x000000 )
        return 0x010101 * static_cast< sal_Int32 >( ::std::max( fTint, 0.0 ) * 255.0 );
    if( nColor == 0xFFFFFF )
        return 0x010101 * static_cast< sal_Int32 >( ::std::min( fTint + 1.0, 1.0 ) * 255.0 );

    sal_Int32 nR = (nColor >> 16) & 0xFF;
    sal_Int32 nG = (nColor >> 8) & 0xFF;
    sal_Int32 nB = nColor & 0xFF;

    double fMean = (::std::min( ::std::min( nR, nG ), nB ) + ::std::max( ::std::max( nR, nG ), nB )) / 2.0;
    double fTintTh = (fMean <= 127.5) ? ((127.5 - fMean) / (255.0 - fMean)) : (127.5 / fMean - 1.0);
    if( (fTintTh < 0.0) || ((fTintTh == 0.0) && (fTint <= 0.0)) )
    {
        double fTintMax = 255.0 / fMean - 1.0;
        double fRTh = fTintTh / fTintMax * (255.0 - nR) + nR;
        double fGTh = fTintTh / fTintMax * (255.0 - nG) + nG;
        double fBTh = fTintTh / fTintMax * (255.0 - nB) + nB;
        if( fTint <= fTintTh )
        {
            double fFactor = (fTint + 1.0) / (fTintTh + 1.0);
            nR = static_cast< sal_Int32 >( fFactor * fRTh + 0.5 );
            nG = static_cast< sal_Int32 >( fFactor * fGTh + 0.5 );
            nB = static_cast< sal_Int32 >( fFactor * fBTh + 0.5 );
        }
        else
        {
            double fFactor = (fTint > 0.0) ? (fTint * fTintMax / fTintTh) : (fTint / fTintTh);
            nR = static_cast< sal_Int32 >( fFactor * fRTh + (1.0 - fFactor) * nR + 0.5 );
            nG = static_cast< sal_Int32 >( fFactor * fGTh + (1.0 - fFactor) * nG + 0.5 );
            nB = static_cast< sal_Int32 >( fFactor * fBTh + (1.0 - fFactor) * nB + 0.5 );
        }
    }
    else
    {
        double fTintMin = fMean / (fMean - 255.0);
        double fRTh = (1.0 - fTintTh / fTintMin) * nR;
        double fGTh = (1.0 - fTintTh / fTintMin) * nG;
        double fBTh = (1.0 - fTintTh / fTintMin) * nB;
        if( fTint <= fTintTh )
        {
            double fFactor = (fTint < 0.0) ? (fTint * -fTintMin / fTintTh) : (fTint / fTintTh);
            nR = static_cast< sal_Int32 >( fFactor * fRTh + (1.0 - fFactor) * nR + 0.5 );
            nG = static_cast< sal_Int32 >( fFactor * fGTh + (1.0 - fFactor) * nG + 0.5 );
            nB = static_cast< sal_Int32 >( fFactor * fBTh + (1.0 - fFactor) * nB + 0.5 );
        }
        else
        {
            double fFactor = (1.0 - fTint) / (1.0 - fTintTh);
            nR = static_cast< sal_Int32 >( 255.5 - fFactor * (255.0 - fRTh) );
            nG = static_cast< sal_Int32 >( 255.5 - fFactor * (255.0 - fGTh) );
            nB = static_cast< sal_Int32 >( 255.5 - fFactor * (255.0 - fBTh) );
        }
    }

    return (nR << 16) | (nG << 8) | nB;
}

} // namespace

// ----------------------------------------------------------------------------

StylesBuffer::StylesBuffer( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    maPalette( rHelper ),
    maNumFmts( rHelper ),
    maDefStyleName( lclGetBuiltinStyleName( OOX_STYLE_NORMAL, OUString() ) ),
    mnDefStyleXf( -1 )
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

FontRef StylesBuffer::importFont( const AttributeList& )
{
    return createFont();
}

NumberFormatRef StylesBuffer::importNumFmt( const AttributeList& rAttribs )
{
    return maNumFmts.importNumFmt( rAttribs );
}

BorderRef StylesBuffer::importBorder( const AttributeList& rAttribs )
{
    BorderRef xBorder = createBorder();
    xBorder->importBorder( rAttribs );
    return xBorder;
}

FillRef StylesBuffer::importFill( const AttributeList& )
{
    return createFill();
}

XfRef StylesBuffer::importXf( sal_Int32 nContext, const AttributeList& rAttribs )
{
    XfRef xXf;
    switch( nContext )
    {
        case XLS_TOKEN( cellXfs ):
            xXf = createCellXf();
            xXf->importXf( rAttribs, true );
        break;
        case XLS_TOKEN( cellStyleXfs ):
            xXf = createStyleXf();
            xXf->importXf( rAttribs, false );
        break;
    }
    return xXf;
}

DxfRef StylesBuffer::importDxf( const AttributeList& )
{
    return createDxf();
}

CellStyleRef StylesBuffer::importCellStyle( const AttributeList& rAttribs )
{
    CellStyleRef xCellStyle( new CellStyle( *this ) );
    xCellStyle->importCellStyle( rAttribs );
    insertCellStyle( xCellStyle );
    return xCellStyle;
}

void StylesBuffer::importPaletteColor( RecordInputStream& rStrm )
{
    maPalette.importPaletteColor( rStrm );
}

void StylesBuffer::importFont( RecordInputStream& rStrm )
{
    createFont()->importFont( rStrm );
}

void StylesBuffer::importNumFmt( RecordInputStream& rStrm )
{
    maNumFmts.importNumFmt( rStrm );
}

void StylesBuffer::importBorder( RecordInputStream& rStrm )
{
    createBorder()->importBorder( rStrm );
}

void StylesBuffer::importFill( RecordInputStream& rStrm )
{
    createFill()->importFill( rStrm );
}

void StylesBuffer::importXf( sal_Int32 nContext, RecordInputStream& rStrm )
{
    switch( nContext )
    {
        case OOBIN_ID_CELLXFS:
            createCellXf()->importXf( rStrm, true );
        break;
        case OOBIN_ID_CELLSTYLEXFS:
            createStyleXf()->importXf( rStrm, false );
        break;
    }
}

void StylesBuffer::importDxf( RecordInputStream& rStrm )
{
    createDxf()->importDxf( rStrm );
}

void StylesBuffer::importCellStyle( RecordInputStream& rStrm )
{
    CellStyleRef xCellStyle( new CellStyle( *this ) );
    xCellStyle->importCellStyle( rStrm );
    insertCellStyle( xCellStyle );
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
    // store XF in both lists (except BIFF2 which does not support cell styles)
    maCellXfs.push_back( xXf );
    if( getBiff() != BIFF2 )
        maStyleXfs.push_back( xXf );
    xXf->importXf( rStrm );
}

void StylesBuffer::importStyle( BiffInputStream& rStrm )
{
    CellStyleRef xCellStyle( new CellStyle( *this ) );
    xCellStyle->importStyle( rStrm );
    insertCellStyle( xCellStyle );
}

void StylesBuffer::finalizeImport()
{
    // fonts first, are needed to finalize unit converter and XFs below
    maFonts.forEachMem( &Font::finalizeImport );
    // finalize unit converter after default font is known
    getUnitConverter().finalizeImport();
    // number formats
    maNumFmts.finalizeImport();
    // borders and fills
    maBorders.forEachMem( &Border::finalizeImport );
    maFills.forEachMem( &Fill::finalizeImport );

    /*  Style XFs and cell XFs. The BIFF format stores cell XFs and style XFs
        mixed in a single list. The import filter has stored the XFs in both
        lists to make the getStyleXf() function working correctly (e.g. for
        retrieving the default font, see getDefaultFont() function), except for
        BIFF2 which does not support cell styles at all. Therefore, if in BIFF
        filter mode, we do not need to finalize the cell styles list. */
    if( getFilterType() == FILTER_OOX )
        maStyleXfs.forEachMem( &Xf::finalizeImport );
    maCellXfs.forEachMem( &Xf::finalizeImport );

    // conditional formatting
    maDxfs.forEachMem( &Dxf::finalizeImport );

    // create the default cell style first
    if( CellStyle* pDefStyle = maCellStyles.get( mnDefStyleXf ).get() )
        pDefStyle->createCellStyle( mnDefStyleXf );
    /*  Create user-defined and modified builtin cell styles, passing true to
        createStyleSheet() skips unchanged builtin styles. */
    for( CellStyleMap::iterator aIt = maCellStyles.begin(), aEnd = maCellStyles.end(); aIt != aEnd; ++aIt )
        aIt->second->createCellStyle( aIt->first, true );
}

sal_Int32 StylesBuffer::getColor( const OoxColor& rColor, sal_Int32 nAuto ) const
{
    sal_Int32 nColor = API_RGB_TRANSPARENT;
    switch( rColor.mnType )
    {
        case XML_auto:
            nColor = nAuto;
        break;
        case XML_rgb:
            nColor = rColor.mnValue & 0xFFFFFF;
        break;
        case XML_theme:
            nColor = getTheme().getColorByIndex( rColor.mnValue );
        break;
        case XML_indexed:
            nColor = maPalette.getColor( rColor.mnValue );
        break;
        default:
            OSL_ENSURE( false, "StylesBuffer::getColor - unknown color type" );
    }
    if( (rColor.mnType != XML_auto) && (nColor != API_RGB_TRANSPARENT) && (rColor.mfTint >= -1.0) && (rColor.mfTint != 0.0) && (rColor.mfTint <= 1.0) )
        nColor = lclTintToColor( nColor, rColor.mfTint );
    return nColor;
}

FontRef StylesBuffer::getFont( sal_Int32 nFontId ) const
{
    return maFonts.get( nFontId );
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
    if( const Xf* pXf = getStyleXf( mnDefStyleXf ).get() )
        xDefFont = pXf->getFont();
    // no font from styles - try first loaded font (e.g. BIFF2)
    if( !xDefFont )
        xDefFont = maFonts.get( 0 );
    OSL_ENSURE( xDefFont.get(), "StylesBuffer::getDefaultFont - no default font found" );
    return xDefFont;
}

const OoxFontData& StylesBuffer::getDefaultFontData() const
{
    FontRef xDefFont = getDefaultFont();
    return xDefFont.get() ? xDefFont->getFontData() : getTheme().getDefaultFontData();
}

const OUString& StylesBuffer::createCellStyle( sal_Int32 nXfId ) const
{
    if( CellStyle* pCellStyle = maCellStyles.get( nXfId ).get() )
        return pCellStyle->createCellStyle( nXfId );
    // on error: fallback to default style
    return maDefStyleName;
}

const OUString& StylesBuffer::createDxfStyle( sal_Int32 nDxfId ) const
{
    if( Dxf* pDxf = maDxfs.get( nDxfId ).get() )
        return pDxf->createDxfStyle( nDxfId );
    // on error: fallback to default style
    return maDefStyleName;
}

#if OOX_XLS_USE_DEFAULT_STYLE
#else
const OUString& StylesBuffer::getDefaultStyleName() const
{
    return createCellStyle( mnDefStyleXf );
}
#endif

void StylesBuffer::writeFontToPropertySet( PropertySet& rPropSet, sal_Int32 nFontId ) const
{
    if( Font* pFont = maFonts.get( nFontId ).get() )
        pFont->writeToPropertySet( rPropSet, FONT_PROPTYPE_CELL );
}

void StylesBuffer::writeNumFmtToPropertySet( PropertySet& rPropSet, sal_Int32 nNumFmtId ) const
{
    maNumFmts.writeToPropertySet( rPropSet, nNumFmtId );
}

void StylesBuffer::writeBorderToPropertySet( PropertySet& rPropSet, sal_Int32 nBorderId ) const
{
    if( Border* pBorder = maBorders.get( nBorderId ).get() )
        pBorder->writeToPropertySet( rPropSet );
}

void StylesBuffer::writeFillToPropertySet( PropertySet& rPropSet, sal_Int32 nFillId ) const
{
    if( Fill* pFill = maFills.get( nFillId ).get() )
        pFill->writeToPropertySet( rPropSet );
}

void StylesBuffer::writeCellXfToPropertySet( PropertySet& rPropSet, sal_Int32 nXfId ) const
{
    if( Xf* pXf = maCellXfs.get( nXfId ).get() )
        pXf->writeToPropertySet( rPropSet );
}

void StylesBuffer::writeStyleXfToPropertySet( PropertySet& rPropSet, sal_Int32 nXfId ) const
{
    if( Xf* pXf = maStyleXfs.get( nXfId ).get() )
        pXf->writeToPropertySet( rPropSet );
}

void StylesBuffer::insertCellStyle( CellStyleRef xCellStyle )
{
    if( xCellStyle->getXfId() >= 0 )
    {
        maCellStyles[ xCellStyle->getXfId() ] = xCellStyle;
        if( xCellStyle->isDefaultStyle() )
            mnDefStyleXf = xCellStyle->getXfId();
    }
}

// ============================================================================

} // namespace xls
} // namespace oox

