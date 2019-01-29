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

#include <stylesbuffer.hxx>

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
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/table/BorderLineStyle.hpp>
#include <com/sun/star/table/CellVertJustify2.hpp>
#include <com/sun/star/table/CellJustifyMethod.hpp>
#include <editeng/justifyitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/lineitem.hxx>
#include <editeng/brushitem.hxx>
#include <svx/rotmodit.hxx>
#include <tools/fontenum.hxx>
#include <vcl/unohelp.hxx>
#include <rtl/tencinfo.h>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <oox/core/filterbase.hxx>
#include <oox/helper/attributelist.hxx>
#include <oox/helper/binaryinputstream.hxx>
#include <oox/helper/containerhelper.hxx>
#include <oox/helper/propertymap.hxx>
#include <oox/helper/propertyset.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>
#include <themebuffer.hxx>
#include <unitconverter.hxx>
#include <document.hxx>
#include <stlpool.hxx>
#include <docpool.hxx>
#include <ftools.hxx>
#include <scitems.hxx>
#include <attrib.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <xlconst.hxx>
#include <documentimport.hxx>
#include <numformat.hxx>
#include <patattr.hxx>
#include <stlsheet.hxx>
#include <biffhelper.hxx>

namespace oox {
namespace xls {

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::table;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;


namespace {

// OOXML constants ------------------------------------------------------------

// OOXML predefined color indexes (also used in BIFF3-BIFF8)
const sal_Int32 OOX_COLOR_USEROFFSET        = 0;        /// First user defined color in palette (OOXML/BIFF12).

// OOXML font family (also used in BIFF)
const sal_Int32 OOX_FONTFAMILY_NONE         = 0;
const sal_Int32 OOX_FONTFAMILY_ROMAN        = 1;
const sal_Int32 OOX_FONTFAMILY_SWISS        = 2;
const sal_Int32 OOX_FONTFAMILY_MODERN       = 3;
const sal_Int32 OOX_FONTFAMILY_SCRIPT       = 4;
const sal_Int32 OOX_FONTFAMILY_DECORATIVE   = 5;

// OOXML cell text direction (also used in BIFF)
const sal_Int32 OOX_XF_TEXTDIR_CONTEXT      = 0;
const sal_Int32 OOX_XF_TEXTDIR_LTR          = 1;
const sal_Int32 OOX_XF_TEXTDIR_RTL          = 2;

// OOXML cell rotation (also used in BIFF)
const sal_Int32 OOX_XF_ROTATION_NONE        = 0;
const sal_Int32 OOX_XF_ROTATION_STACKED     = 255;

// OOXML cell indentation
const sal_Int32 OOX_XF_INDENT_NONE          = 0;

// OOXML built-in cell styles (also used in BIFF)
const sal_Int32 OOX_STYLE_NORMAL            = 0;        /// Default cell style.
const sal_Int32 OOX_STYLE_ROWLEVEL          = 1;        /// RowLevel_x cell style.
const sal_Int32 OOX_STYLE_COLLEVEL          = 2;        /// ColLevel_x cell style.

// BIFF12 constants -----------------------------------------------------------

// BIFF12 color types
const sal_uInt8 BIFF12_COLOR_AUTO           = 0;
const sal_uInt8 BIFF12_COLOR_INDEXED        = 1;
const sal_uInt8 BIFF12_COLOR_RGB            = 2;
const sal_uInt8 BIFF12_COLOR_THEME          = 3;

// BIFF12 diagonal borders
const sal_uInt8 BIFF12_BORDER_DIAG_TLBR     = 0x01;     /// Top-left to bottom-right.
const sal_uInt8 BIFF12_BORDER_DIAG_BLTR     = 0x02;     /// Bottom-left to top-right.

// BIFF12 gradient fill
const sal_Int32 BIFF12_FILL_GRADIENT        = 40;

// BIFF12 XF flags
const sal_uInt32 BIFF12_XF_WRAPTEXT         = 0x00400000;
const sal_uInt32 BIFF12_XF_JUSTLASTLINE     = 0x00800000;
const sal_uInt32 BIFF12_XF_SHRINK           = 0x01000000;
const sal_uInt32 BIFF12_XF_LOCKED           = 0x10000000;
const sal_uInt32 BIFF12_XF_HIDDEN           = 0x20000000;

// BIFF12 XF attribute used flags
const sal_uInt16 BIFF12_XF_NUMFMT_USED      = 0x0001;
const sal_uInt16 BIFF12_XF_FONT_USED        = 0x0002;
const sal_uInt16 BIFF12_XF_ALIGN_USED       = 0x0004;
const sal_uInt16 BIFF12_XF_BORDER_USED      = 0x0008;
const sal_uInt16 BIFF12_XF_AREA_USED        = 0x0010;
const sal_uInt16 BIFF12_XF_PROT_USED        = 0x0020;

// BIFF12 DXF constants
const sal_uInt16 BIFF12_DXF_FILL_PATTERN    = 0;
const sal_uInt16 BIFF12_DXF_FILL_FGCOLOR    = 1;
const sal_uInt16 BIFF12_DXF_FILL_BGCOLOR    = 2;
const sal_uInt16 BIFF12_DXF_FILL_GRADIENT   = 3;
const sal_uInt16 BIFF12_DXF_FILL_STOP       = 4;
const sal_uInt16 BIFF12_DXF_FONT_COLOR      = 5;
const sal_uInt16 BIFF12_DXF_BORDER_TOP      = 6;
const sal_uInt16 BIFF12_DXF_BORDER_BOTTOM   = 7;
const sal_uInt16 BIFF12_DXF_BORDER_LEFT     = 8;
const sal_uInt16 BIFF12_DXF_BORDER_RIGHT    = 9;
const sal_uInt16 BIFF12_DXF_FONT_NAME       = 24;
const sal_uInt16 BIFF12_DXF_FONT_WEIGHT     = 25;
const sal_uInt16 BIFF12_DXF_FONT_UNDERLINE  = 26;
const sal_uInt16 BIFF12_DXF_FONT_ESCAPEMENT = 27;
const sal_uInt16 BIFF12_DXF_FONT_ITALIC     = 28;
const sal_uInt16 BIFF12_DXF_FONT_STRIKE     = 29;
const sal_uInt16 BIFF12_DXF_FONT_OUTLINE    = 30;
const sal_uInt16 BIFF12_DXF_FONT_SHADOW     = 31;
const sal_uInt16 BIFF12_DXF_FONT_HEIGHT     = 36;
const sal_uInt16 BIFF12_DXF_FONT_SCHEME     = 37;
const sal_uInt16 BIFF12_DXF_NUMFMT_CODE     = 38;
const sal_uInt16 BIFF12_DXF_NUMFMT_ID       = 41;

// BIFF12 CELLSTYLE flags
const sal_uInt16 BIFF12_CELLSTYLE_BUILTIN   = 0x0001;
const sal_uInt16 BIFF12_CELLSTYLE_HIDDEN    = 0x0002;
const sal_uInt16 BIFF12_CELLSTYLE_CUSTOM    = 0x0004;

// BIFF constants -------------------------------------------------------------

// BIFF font flags, also used in BIFF12
const sal_uInt16 BIFF_FONTFLAG_ITALIC       = 0x0002;
const sal_uInt16 BIFF_FONTFLAG_STRIKEOUT    = 0x0008;
const sal_uInt16 BIFF_FONTFLAG_OUTLINE      = 0x0010;
const sal_uInt16 BIFF_FONTFLAG_SHADOW       = 0x0020;

// BIFF font weight
const sal_uInt16 BIFF_FONTWEIGHT_BOLD       = 450;

// BIFF font underline, also used in BIFF12
const sal_uInt8 BIFF_FONTUNDERL_NONE        = 0;
const sal_uInt8 BIFF_FONTUNDERL_SINGLE      = 1;
const sal_uInt8 BIFF_FONTUNDERL_DOUBLE      = 2;
const sal_uInt8 BIFF_FONTUNDERL_SINGLE_ACC  = 33;
const sal_uInt8 BIFF_FONTUNDERL_DOUBLE_ACC  = 34;

::Color lclReadRgbColor( BinaryInputStream& rStrm )
{
    sal_uInt8 nR, nG, nB, nA;
    nR = rStrm.readuChar();
    nG = rStrm.readuChar();
    nB = rStrm.readuChar();
    nA = rStrm.readuChar();
    sal_Int32 nValue = nA;
    nValue <<= 8;
    nValue |= nR;
    nValue <<= 8;
    nValue |= nG;
    nValue <<= 8;
    nValue |= nB;
    return ::Color(nValue);
}

} // namespace

ExcelGraphicHelper::ExcelGraphicHelper( const WorkbookHelper& rHelper ) :
    GraphicHelper( rHelper.getBaseFilter().getComponentContext(), rHelper.getBaseFilter().getTargetFrame(), rHelper.getBaseFilter().getStorage() ),
    WorkbookHelper( rHelper )
{
}

::Color ExcelGraphicHelper::getSchemeColor( sal_Int32 nToken ) const
{
    return getTheme().getColorByToken( nToken );
}

::Color ExcelGraphicHelper::getPaletteColor( sal_Int32 nPaletteIdx ) const
{
    return getStyles().getPaletteColor( nPaletteIdx );
}

void Color::setAuto()
{
    clearTransformations();
    setSchemeClr( XML_phClr );
}

void Color::setRgb( ::Color nRgbValue, double fTint )
{
    clearTransformations();
    setSrgbClr( sal_uInt32(nRgbValue) & 0xFFFFFF );
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
    // tdf#113271 The order of import color is very important in case of more than one color attributes was provided.
    // This order (theme -> rgb -> indexed -> auto) is not documented and was gathered experimentally based on MS Excel 2013.
    if( rAttribs.hasAttribute( XML_theme ) )
        setTheme( rAttribs.getInteger( XML_theme, -1 ), rAttribs.getDouble( XML_tint, 0.0 ) );
    else if( rAttribs.hasAttribute( XML_rgb ) )
        setRgb( rAttribs.getIntegerHex( XML_rgb, sal_Int32(API_RGB_TRANSPARENT) ), rAttribs.getDouble( XML_tint, 0.0 ) );
    else if( rAttribs.hasAttribute( XML_indexed ) )
        setIndexed( rAttribs.getInteger( XML_indexed, -1 ), rAttribs.getDouble( XML_tint, 0.0 ) );
    else if( rAttribs.getBool( XML_auto, false ) )
        setAuto();
    else
    {
        OSL_FAIL( "Color::importColor - unknown color type" );
        setAuto();
    }
}

void Color::importColor( SequenceInputStream& rStrm )
{
    sal_uInt8 nFlags, nIndex;
    sal_Int16 nTint;
    nFlags = rStrm.readuChar();
    nIndex = rStrm.readuChar();
    nTint = rStrm.readInt16();

    // scale tint from signed 16-bit to double range -1.0 ... 1.0
    double fTint = nTint;
    if( nTint < 0 )
        fTint /= -SAL_MIN_INT16;
    else if( nTint > 0 )
        fTint /= SAL_MAX_INT16;

    switch( extractValue< sal_uInt8 >( nFlags, 1, 7 ) )
    {
        case BIFF12_COLOR_AUTO:
            setAuto();
            rStrm.skip( 4 );
        break;
        case BIFF12_COLOR_INDEXED:
            setIndexed( nIndex, fTint );
            rStrm.skip( 4 );
        break;
        case BIFF12_COLOR_RGB:
            setRgb( lclReadRgbColor( rStrm ), fTint );
        break;
        case BIFF12_COLOR_THEME:
            setTheme( nIndex, fTint );
            rStrm.skip( 4 );
        break;
        default:
            OSL_FAIL( "Color::importColor - unknown color type" );
            setAuto();
            rStrm.skip( 4 );
    }
}

void Color::importColorId( SequenceInputStream& rStrm )
{
    setIndexed( rStrm.readInt32() );
}

SequenceInputStream& operator>>( SequenceInputStream& rStrm, Color& orColor )
{
    orColor.importColor( rStrm );
    return rStrm;
}

namespace {

/** Standard EGA colors, bright. */
#define PALETTE_EGA_COLORS_LIGHT \
            ::Color(0x000000), ::Color(0xFFFFFF), ::Color(0xFF0000), ::Color(0x00FF00), ::Color(0x0000FF), ::Color(0xFFFF00), ::Color(0xFF00FF), ::Color(0x00FFFF)
/** Standard EGA colors), dark. */
#define PALETTE_EGA_COLORS_DARK \
            ::Color(0x800000), ::Color(0x008000), ::Color(0x000080), ::Color(0x808000), ::Color(0x800080), ::Color(0x008080), ::Color(0xC0C0C0), ::Color(0x808080)

/** Default color table for BIFF8/BIFF12/OOXML. */
static const ::Color spnDefColors8[] =
{
/*  0 */    PALETTE_EGA_COLORS_LIGHT,
/*  8 */    PALETTE_EGA_COLORS_LIGHT,
/* 16 */    PALETTE_EGA_COLORS_DARK,
/* 24 */    ::Color(0x9999FF), ::Color(0x993366), ::Color(0xFFFFCC), ::Color(0xCCFFFF), ::Color(0x660066), ::Color(0xFF8080), ::Color(0x0066CC), ::Color(0xCCCCFF),
/* 32 */    ::Color(0x000080), ::Color(0xFF00FF), ::Color(0xFFFF00), ::Color(0x00FFFF), ::Color(0x800080), ::Color(0x800000), ::Color(0x008080), ::Color(0x0000FF),
/* 40 */    ::Color(0x00CCFF), ::Color(0xCCFFFF), ::Color(0xCCFFCC), ::Color(0xFFFF99), ::Color(0x99CCFF), ::Color(0xFF99CC), ::Color(0xCC99FF), ::Color(0xFFCC99),
/* 48 */    ::Color(0x3366FF), ::Color(0x33CCCC), ::Color(0x99CC00), ::Color(0xFFCC00), ::Color(0xFF9900), ::Color(0xFF6600), ::Color(0x666699), ::Color(0x969696),
/* 56 */    ::Color(0x003366), ::Color(0x339966), ::Color(0x003300), ::Color(0x333300), ::Color(0x993300), ::Color(0x993366), ::Color(0x333399), ::Color(0x333333)
};

#undef PALETTE_EGA_COLORS_LIGHT
#undef PALETTE_EGA_COLORS_DARK

} // namespace

ColorPalette::ColorPalette( const WorkbookHelper& rHelper )
    : WorkbookHelper(rHelper)
    , mnAppendIndex(0)
{
    // default colors
    maColors.insert( maColors.begin(), spnDefColors8, spnDefColors8 + SAL_N_ELEMENTS(spnDefColors8) );
    mnAppendIndex = OOX_COLOR_USEROFFSET;
}

void ColorPalette::importPaletteColor( const AttributeList& rAttribs )
{
    appendColor( rAttribs.getIntegerHex( XML_rgb, sal_Int32(API_RGB_WHITE) ) );
}

void ColorPalette::importPaletteColor( SequenceInputStream& rStrm )
{
    ::Color nRgb = lclReadRgbColor( rStrm );
    appendColor( nRgb );
}

::Color ColorPalette::getColor( sal_Int32 nPaletteIdx ) const
{
    ::Color nColor = API_RGB_TRANSPARENT;
    if( const ::Color* pnPaletteColor = ContainerHelper::getVectorElement( maColors, nPaletteIdx ) )
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
        default:                        OSL_FAIL( "ColorPalette::getColor - unknown color index" );
    }
    return nColor;
}

void ColorPalette::appendColor( ::Color nRGBValue )
{
    if( mnAppendIndex < maColors.size() )
        maColors[ mnAppendIndex ] = nRGBValue;
    else
        maColors.push_back( nRGBValue );
    ++mnAppendIndex;
}

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

void FontModel::setBiff12Scheme( sal_uInt8 nScheme )
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

ApiScriptFontName::ApiScriptFontName() :
    mnFamily( css::awt::FontFamily::DONTKNOW ),
    mnTextEnc( RTL_TEXTENCODING_DONTKNOW )
{
}

ApiFontData::ApiFontData() :
    maDesc(
        "Calibri",
        220,                                            // height 11 points
        0,
        OUString(),
        css::awt::FontFamily::DONTKNOW,
        RTL_TEXTENCODING_DONTKNOW,
        css::awt::FontPitch::DONTKNOW,
        100.0,
        css::awt::FontWeight::NORMAL,
        css::awt::FontSlant_NONE,
        css::awt::FontUnderline::NONE,
        css::awt::FontStrikeout::NONE,
        0.0,
        false,
        false,
        css::awt::FontType::DONTKNOW ),
    mnColor( API_RGB_TRANSPARENT ),
    mnEscapement( API_ESCAPE_NONE ),
    mnEscapeHeight( API_ESCAPEHEIGHT_NONE ),
    mbOutline( false ),
    mbShadow( false )
{
    maLatinFont.maName = maDesc.Name;
}

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

void Font::importFont( SequenceInputStream& rStrm )
{
    SAL_WARN_IF( mbDxf, "sc", "Font::importFont - unexpected conditional formatting flag" );

    sal_uInt16 nHeight, nFlags, nWeight, nEscapement;
    sal_uInt8 nUnderline, nFamily, nCharSet, nScheme;
    nHeight = rStrm.readuInt16();
    nFlags = rStrm.readuInt16();
    nWeight = rStrm.readuInt16();
    nEscapement = rStrm.readuInt16();
    nUnderline = rStrm.readuChar();
    nFamily = rStrm.readuChar();
    nCharSet = rStrm.readuChar();
    rStrm.skip( 1 );
    rStrm >> maModel.maColor;
    nScheme = rStrm.readuChar();
    rStrm >> maModel.maName;

    // equal constants in all BIFFs for weight, underline, and escapement
    maModel.setBiff12Scheme( nScheme );
    maModel.setBiffHeight( nHeight );
    maModel.setBiffWeight( nWeight );
    maModel.setBiffUnderline( nUnderline );
    maModel.setBiffEscapement( nEscapement );
    maModel.mnFamily    = nFamily;
    maModel.mnCharSet   = nCharSet;
    // equal flags in all BIFFs
    maModel.mbItalic    = getFlag( nFlags, BIFF_FONTFLAG_ITALIC );
    maModel.mbStrikeout = getFlag( nFlags, BIFF_FONTFLAG_STRIKEOUT );
    maModel.mbOutline   = getFlag( nFlags, BIFF_FONTFLAG_OUTLINE );
    maModel.mbShadow    = getFlag( nFlags, BIFF_FONTFLAG_SHADOW );
}

void Font::importDxfName( SequenceInputStream& rStrm )
{
    SAL_WARN_IF( !mbDxf, "sc", "Font::importDxfName - missing conditional formatting flag" );
    maModel.maName = BiffHelper::readString( rStrm, false );
    maUsedFlags.mbColorUsed = true;
}

void Font::importDxfColor( SequenceInputStream& rStrm )
{
    SAL_WARN_IF( !mbDxf, "sc", "Font::importDxfColor - missing conditional formatting flag" );
    rStrm >> maModel.maColor;
    maUsedFlags.mbColorUsed = true;
}

void Font::importDxfScheme( SequenceInputStream& rStrm )
{
    SAL_WARN_IF( !mbDxf, "sc", "Font::importDxfScheme - missing conditional formatting flag" );
    maModel.setBiff12Scheme( rStrm.readuInt8() );
    maUsedFlags.mbSchemeUsed = true;
}

void Font::importDxfHeight( SequenceInputStream& rStrm )
{
    SAL_WARN_IF( !mbDxf, "sc", "Font::importDxfHeight - missing conditional formatting flag" );
    maModel.setBiffHeight( rStrm.readuInt16() );
    maUsedFlags.mbHeightUsed = true;
}

void Font::importDxfWeight( SequenceInputStream& rStrm )
{
    SAL_WARN_IF( !mbDxf, "sc", "Font::importDxfWeight - missing conditional formatting flag" );
    maModel.setBiffWeight( rStrm.readuInt16() );
    maUsedFlags.mbWeightUsed = true;
}

void Font::importDxfUnderline( SequenceInputStream& rStrm )
{
    SAL_WARN_IF( !mbDxf, "sc", "Font::importDxfUnderline - missing conditional formatting flag" );
    maModel.setBiffUnderline( rStrm.readuInt16() );
    maUsedFlags.mbUnderlineUsed = true;
}

void Font::importDxfEscapement( SequenceInputStream& rStrm )
{
    SAL_WARN_IF( !mbDxf, "sc", "Font::importDxfEscapement - missing conditional formatting flag" );
    maModel.setBiffEscapement( rStrm.readuInt16() );
    maUsedFlags.mbEscapementUsed = true;
}

void Font::importDxfFlag( sal_Int32 nElement, SequenceInputStream& rStrm )
{
    SAL_WARN_IF( !mbDxf, "sc", "Font::importDxfFlag - missing conditional formatting flag" );
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
            OSL_FAIL( "Font::importDxfFlag - unexpected element identifier" );
    }
}

void Font::finalizeImport()
{
    // font name
    maApiData.maDesc.Name = maModel.maName;

    // font family
    switch( maModel.mnFamily )
    {
        case OOX_FONTFAMILY_NONE:           maApiData.maDesc.Family = css::awt::FontFamily::DONTKNOW;     break;
        case OOX_FONTFAMILY_ROMAN:          maApiData.maDesc.Family = css::awt::FontFamily::ROMAN;        break;
        case OOX_FONTFAMILY_SWISS:          maApiData.maDesc.Family = css::awt::FontFamily::SWISS;        break;
        case OOX_FONTFAMILY_MODERN:         maApiData.maDesc.Family = css::awt::FontFamily::MODERN;       break;
        case OOX_FONTFAMILY_SCRIPT:         maApiData.maDesc.Family = css::awt::FontFamily::SCRIPT;       break;
        case OOX_FONTFAMILY_DECORATIVE:     maApiData.maDesc.Family = css::awt::FontFamily::DECORATIVE;   break;
    }

    // character set (API font descriptor uses rtl_TextEncoding in member CharSet!)
    if( (0 <= maModel.mnCharSet) && (maModel.mnCharSet <= SAL_MAX_UINT8) )
        maApiData.maDesc.CharSet = static_cast< sal_Int16 >(
            rtl_getTextEncodingFromWindowsCharset( static_cast< sal_uInt8 >( maModel.mnCharSet ) ) );

    // color, height, weight, slant, strikeout, outline, shadow
    maApiData.mnColor          = maModel.maColor.getColor( getBaseFilter().getGraphicHelper() );
    maApiData.maDesc.Height    = static_cast< sal_Int16 >( maModel.mfHeight * 20.0 );
    maApiData.maDesc.Weight    = maModel.mbBold ? css::awt::FontWeight::BOLD : css::awt::FontWeight::NORMAL;
    maApiData.maDesc.Slant     = maModel.mbItalic ? css::awt::FontSlant_ITALIC : css::awt::FontSlant_NONE;
    maApiData.maDesc.Strikeout = maModel.mbStrikeout ? css::awt::FontStrikeout::SINGLE : css::awt::FontStrikeout::NONE;
    maApiData.mbOutline        = maModel.mbOutline;
    maApiData.mbShadow         = maModel.mbShadow;

    // underline
    switch( maModel.mnUnderline )
    {
        case XML_double:            maApiData.maDesc.Underline = css::awt::FontUnderline::DOUBLE; break;
        case XML_doubleAccounting:  maApiData.maDesc.Underline = css::awt::FontUnderline::DOUBLE; break;
        case XML_none:              maApiData.maDesc.Underline = css::awt::FontUnderline::NONE;   break;
        case XML_single:            maApiData.maDesc.Underline = css::awt::FontUnderline::SINGLE; break;
        case XML_singleAccounting:  maApiData.maDesc.Underline = css::awt::FontUnderline::SINGLE; break;
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
        PropertySet aDocProps( getDocument() );
        Reference< XDevice > xDevice( aDocProps.getAnyProperty( PROP_ReferenceDevice ), UNO_QUERY );
        if( xDevice.is() )
        {
            Reference< XFont2 > xFont( xDevice->getFont( maApiData.maDesc ), UNO_QUERY );
            if( xFont.is() )
            {
                // #91658# CJK fonts
                bool bHasAsian =
                    xFont->hasGlyphs( OUString( u'\x3041' ) ) ||    // 3040-309F: Hiragana
                    xFont->hasGlyphs( OUString( u'\x30A1' ) ) ||    // 30A0-30FF: Katakana
                    xFont->hasGlyphs( OUString( u'\x3111' ) ) ||    // 3100-312F: Bopomofo
                    xFont->hasGlyphs( OUString( u'\x3131' ) ) ||    // 3130-318F: Hangul Compatibility Jamo
                    xFont->hasGlyphs( OUString( u'\x3301' ) ) ||    // 3300-33FF: CJK Compatibility
                    xFont->hasGlyphs( OUString( u'\x3401' ) ) ||    // 3400-4DBF: CJK Unified Ideographs Extension A
                    xFont->hasGlyphs( OUString( u'\x4E01' ) ) ||    // 4E00-9FFF: CJK Unified Ideographs
                    xFont->hasGlyphs( OUString( u'\x7E01' ) ) ||    // 4E00-9FFF: CJK Unified Ideographs
                    xFont->hasGlyphs( OUString( u'\xA001' ) ) ||    // A001-A48F: Yi Syllables
                    xFont->hasGlyphs( OUString( u'\xAC01' ) ) ||    // AC00-D7AF: Hangul Syllables
                    xFont->hasGlyphs( OUString( u'\xCC01' ) ) ||    // AC00-D7AF: Hangul Syllables
                    xFont->hasGlyphs( OUString( u'\xF901' ) ) ||    // F900-FAFF: CJK Compatibility Ideographs
                    xFont->hasGlyphs( OUString( u'\xFF71' ) );      // FF00-FFEF: Halfwidth/Fullwidth Forms
                // #113783# CTL fonts
                bool bHasCmplx =
                    xFont->hasGlyphs( OUString( u'\x05D1' ) ) ||    // 0590-05FF: Hebrew
                    xFont->hasGlyphs( OUString( u'\x0631' ) ) ||    // 0600-06FF: Arabic
                    xFont->hasGlyphs( OUString( u'\x0721' ) ) ||    // 0700-074F: Syriac
                    xFont->hasGlyphs( OUString( u'\x0911' ) ) ||    // 0900-0DFF: Indic scripts
                    xFont->hasGlyphs( OUString( u'\x0E01' ) ) ||    // 0E00-0E7F: Thai
                    xFont->hasGlyphs( OUString( u'\xFB21' ) ) ||    // FB1D-FB4F: Hebrew Presentation Forms
                    xFont->hasGlyphs( OUString( u'\xFB51' ) ) ||    // FB50-FDFF: Arabic Presentation Forms-A
                    xFont->hasGlyphs( OUString( u'\xFE71' ) );      // FE70-FEFF: Arabic Presentation Forms-B
                // Western fonts
                bool bHasLatin =
                    (!bHasAsian && !bHasCmplx) ||
                    xFont->hasGlyphs( OUString( 'A' ) );

                lclSetFontName( maApiData.maLatinFont, maApiData.maDesc, bHasLatin );
                lclSetFontName( maApiData.maAsianFont, maApiData.maDesc, bHasAsian );
                lclSetFontName( maApiData.maCmplxFont, maApiData.maDesc, bHasCmplx );
            }
        }
    }
}

bool Font::needsRichTextFormat() const
{
    return maApiData.mnEscapement != API_ESCAPE_NONE;
}

static ::FontFamily lcl_getFontFamily( sal_Int32 nFamily )
{
    ::FontFamily eScFamily = FAMILY_DONTKNOW;
    switch( nFamily )
    {
        case css::awt::FontFamily::DONTKNOW:
            eScFamily = FAMILY_DONTKNOW;
            break;
        case css::awt::FontFamily::ROMAN:
            eScFamily = FAMILY_ROMAN;
            break;
        case css::awt::FontFamily::SWISS:
            eScFamily = FAMILY_SWISS;
            break;
        case css::awt::FontFamily::MODERN:
            eScFamily = FAMILY_MODERN;
            break;
        case css::awt::FontFamily::SCRIPT:
            eScFamily = FAMILY_SCRIPT;
            break;
        case css::awt::FontFamily::DECORATIVE:
            eScFamily = FAMILY_DECORATIVE;
            break;
    }
    return eScFamily;
}

void Font::fillToItemSet( SfxItemSet& rItemSet, bool bEditEngineText, bool bSkipPoolDefs ) const
{
    if ( maUsedFlags.mbNameUsed )
    {
        if( !maApiData.maLatinFont.maName.isEmpty() )
        {
            rtl_TextEncoding eFontEnc = maApiData.maLatinFont.mnTextEnc;
            // taken from binary importer
            rtl_TextEncoding eTempTextEnc = (bEditEngineText && (eFontEnc == getTextEncoding())) ?
                ScfTools::GetSystemTextEncoding() : eFontEnc;

            SvxFontItem aFontItem( lcl_getFontFamily( maApiData.maLatinFont.mnFamily ), maApiData.maLatinFont.maName, OUString(),
                PITCH_DONTKNOW, eTempTextEnc, ATTR_FONT );
            ScfTools::PutItem( rItemSet, aFontItem, bEditEngineText ? static_cast<sal_uInt16>(EE_CHAR_FONTINFO) : ATTR_FONT, bSkipPoolDefs );
        }
        if( !maApiData.maAsianFont.maName.isEmpty() )
        {
            rtl_TextEncoding eFontEnc = maApiData.maAsianFont.mnTextEnc;
            // taken from binary importer
            rtl_TextEncoding eTempTextEnc = (bEditEngineText && (eFontEnc == getTextEncoding())) ?
                ScfTools::GetSystemTextEncoding() : eFontEnc;
            SvxFontItem aFontItem( lcl_getFontFamily( maApiData.maAsianFont.mnFamily ), maApiData.maAsianFont.maName, OUString(),
                PITCH_DONTKNOW, eTempTextEnc, ATTR_FONT );
            ScfTools::PutItem( rItemSet, aFontItem, bEditEngineText ? static_cast<sal_uInt16>(EE_CHAR_FONTINFO_CJK) : ATTR_CJK_FONT, bSkipPoolDefs );
        }
        if( !maApiData.maCmplxFont.maName.isEmpty() )
        {
            rtl_TextEncoding eFontEnc = maApiData.maCmplxFont.mnTextEnc;
            // taken from binary importer
            rtl_TextEncoding eTempTextEnc = (bEditEngineText && (eFontEnc == getTextEncoding())) ?
                ScfTools::GetSystemTextEncoding() : eFontEnc;
            SvxFontItem aFontItem( lcl_getFontFamily( maApiData.maCmplxFont.mnFamily ), maApiData.maCmplxFont.maName, OUString(),
                PITCH_DONTKNOW, eTempTextEnc, ATTR_FONT );
            ScfTools::PutItem( rItemSet, aFontItem, bEditEngineText ? static_cast<sal_uInt16>(EE_CHAR_FONTINFO_CTL) : ATTR_CTL_FONT, bSkipPoolDefs );
        }
    }
    // font height
    if( maUsedFlags.mbHeightUsed )
    {
        sal_Int32 nHeight = maApiData.maDesc.Height;
        // do we use XclFontItemType::HeaderFooter ( or is it just relevant for the binary filter )
        if( bEditEngineText/* && (eType != XclFontItemType::HeaderFooter) */)     // do not convert header/footer height
            nHeight = (nHeight * 127 + 36) / EXC_POINTS_PER_INCH;   // 1 in == 72 pt
        SvxFontHeightItem aHeightItem( nHeight, 100, ATTR_FONT_HEIGHT );
        ScfTools::PutItem( rItemSet, aHeightItem, bEditEngineText ? static_cast<sal_uInt16>(EE_CHAR_FONTHEIGHT) :  ATTR_FONT_HEIGHT, bSkipPoolDefs );
        ScfTools::PutItem( rItemSet, aHeightItem, bEditEngineText ? static_cast<sal_uInt16>(EE_CHAR_FONTHEIGHT_CJK) : ATTR_CJK_FONT_HEIGHT, bSkipPoolDefs );
        ScfTools::PutItem( rItemSet, aHeightItem, bEditEngineText ? static_cast<sal_uInt16>(EE_CHAR_FONTHEIGHT_CTL) : ATTR_CTL_FONT_HEIGHT, bSkipPoolDefs );
    }
    // font weight
    if( maUsedFlags.mbWeightUsed )
    {
        ::FontWeight fWeight = vcl::unohelper::ConvertFontWeight( maApiData.maDesc.Weight );
        SvxWeightItem aWeightItem( fWeight, ATTR_FONT_WEIGHT );
        ScfTools::PutItem( rItemSet, aWeightItem, bEditEngineText ? static_cast<sal_uInt16>(EE_CHAR_WEIGHT) : ATTR_FONT_WEIGHT, bSkipPoolDefs );
        ScfTools::PutItem( rItemSet, aWeightItem, bEditEngineText ? static_cast<sal_uInt16>(EE_CHAR_WEIGHT_CTL) : ATTR_CTL_FONT_WEIGHT, bSkipPoolDefs );
        ScfTools::PutItem( rItemSet, aWeightItem, bEditEngineText ? static_cast<sal_uInt16>(EE_CHAR_WEIGHT_CJK) : ATTR_CJK_FONT_WEIGHT, bSkipPoolDefs );
    }
    // font posture
    if( maUsedFlags.mbPostureUsed )
    {
        SvxPostureItem aPostItem( ( maApiData.maDesc.Slant == css::awt::FontSlant_ITALIC ) ? ITALIC_NORMAL :  ITALIC_NONE,  ATTR_FONT_POSTURE);
        ScfTools::PutItem( rItemSet, aPostItem, bEditEngineText ? static_cast<sal_uInt16>(EE_CHAR_ITALIC) : ATTR_FONT_POSTURE, bSkipPoolDefs );
        ScfTools::PutItem( rItemSet, aPostItem, bEditEngineText ? static_cast<sal_uInt16>(EE_CHAR_ITALIC_CJK) : ATTR_CJK_FONT_POSTURE, bSkipPoolDefs );
        ScfTools::PutItem( rItemSet, aPostItem, bEditEngineText ? static_cast<sal_uInt16>(EE_CHAR_ITALIC_CTL) : ATTR_CTL_FONT_POSTURE, bSkipPoolDefs );
    }
    // character color
    if( maUsedFlags.mbColorUsed )
    {
        ScfTools::PutItem( rItemSet,SvxColorItem( maApiData.mnColor, bEditEngineText ? static_cast<sal_uInt16>(EE_CHAR_COLOR) : ATTR_FONT_COLOR), bSkipPoolDefs );
    }
    // underline style
    if( maUsedFlags.mbUnderlineUsed )
    {
        FontLineStyle eScUnderl;
        if ( maApiData.maDesc.Underline == css::awt::FontUnderline::DOUBLE )
            eScUnderl = LINESTYLE_DOUBLE;
        else if ( maApiData.maDesc.Underline == css::awt::FontUnderline::SINGLE )
            eScUnderl = LINESTYLE_SINGLE;
        else
            eScUnderl = LINESTYLE_NONE;
        SvxUnderlineItem aUnderlItem( eScUnderl, ATTR_FONT_UNDERLINE );
        ScfTools::PutItem( rItemSet, aUnderlItem, bEditEngineText ? static_cast<sal_uInt16>(EE_CHAR_UNDERLINE) : ATTR_FONT_UNDERLINE, bSkipPoolDefs );
    }
    // strike out style
    if( maUsedFlags.mbStrikeoutUsed )
    {
        ScfTools::PutItem( rItemSet, SvxCrossedOutItem( maModel.mbStrikeout ? STRIKEOUT_SINGLE : STRIKEOUT_NONE, bEditEngineText ? static_cast<sal_uInt16>(EE_CHAR_STRIKEOUT) : ATTR_FONT_CROSSEDOUT ), bEditEngineText ? static_cast<sal_uInt16>(EE_CHAR_STRIKEOUT) : ATTR_FONT_CROSSEDOUT, bSkipPoolDefs );
    }

    // outline style
    if( maUsedFlags.mbOutlineUsed )
    {
        ScfTools::PutItem( rItemSet, SvxContourItem( maApiData.mbOutline, ATTR_FONT_CONTOUR ), bEditEngineText ? static_cast<sal_uInt16>(EE_CHAR_OUTLINE) : ATTR_FONT_CONTOUR, bSkipPoolDefs );
    }

    // shadow style
    if( maUsedFlags.mbShadowUsed )
    {
        ScfTools::PutItem( rItemSet, SvxShadowedItem( maApiData.mbShadow, ATTR_FONT_SHADOWED ), bEditEngineText ? static_cast<sal_uInt16>(EE_CHAR_SHADOW) : ATTR_FONT_SHADOWED, bSkipPoolDefs );
    }
    if( maUsedFlags.mbEscapementUsed )
    {
        SvxEscapement eScEscapem = SvxEscapement::Off;
        if ( maApiData.mnEscapement == API_ESCAPE_SUPERSCRIPT )
            eScEscapem = SvxEscapement::Superscript;
        else if ( maApiData.mnEscapement == API_ESCAPE_SUBSCRIPT )
            eScEscapem = SvxEscapement::Subscript;
        if( bEditEngineText )
        {
           // #TODO handle EscapementHeight
            rItemSet.Put( SvxEscapementItem( eScEscapem, EE_CHAR_ESCAPEMENT ) );
        }
    }
}

void Font::writeToPropertyMap( PropertyMap& rPropMap ) const
{
    // font name properties
    if( maUsedFlags.mbNameUsed )
    {
        if( !maApiData.maLatinFont.maName.isEmpty() )
        {
            rPropMap.setProperty( PROP_CharFontName, maApiData.maLatinFont.maName);
            rPropMap.setProperty( PROP_CharFontFamily, maApiData.maLatinFont.mnFamily);
            rPropMap.setProperty( PROP_CharFontCharSet, maApiData.maLatinFont.mnTextEnc);
        }
        if( !maApiData.maAsianFont.maName.isEmpty() )
        {
            rPropMap.setProperty( PROP_CharFontNameAsian, maApiData.maAsianFont.maName);
            rPropMap.setProperty( PROP_CharFontFamilyAsian, maApiData.maAsianFont.mnFamily);
            rPropMap.setProperty( PROP_CharFontCharSetAsian, maApiData.maAsianFont.mnTextEnc);
        }
        if( !maApiData.maCmplxFont.maName.isEmpty() )
        {
            rPropMap.setProperty( PROP_CharFontNameComplex, maApiData.maCmplxFont.maName);
            rPropMap.setProperty( PROP_CharFontFamilyComplex, maApiData.maCmplxFont.mnFamily);
            rPropMap.setProperty( PROP_CharFontCharSetComplex, maApiData.maCmplxFont.mnTextEnc);
        }
    }
    // font height
    if( maUsedFlags.mbHeightUsed )
    {
        float fHeight = static_cast< float >( maApiData.maDesc.Height / 20.0 ); // twips to points
        rPropMap.setProperty( PROP_CharHeight, fHeight);
        rPropMap.setProperty( PROP_CharHeightAsian, fHeight);
        rPropMap.setProperty( PROP_CharHeightComplex, fHeight);
    }
    // font weight
    if( maUsedFlags.mbWeightUsed )
    {
        float fWeight = maApiData.maDesc.Weight;
        rPropMap.setProperty( PROP_CharWeight, fWeight);
        rPropMap.setProperty( PROP_CharWeightAsian, fWeight);
        rPropMap.setProperty( PROP_CharWeightComplex, fWeight);
    }
    // font posture
    if( maUsedFlags.mbPostureUsed )
    {
        rPropMap.setProperty( PROP_CharPosture, maApiData.maDesc.Slant);
        rPropMap.setProperty( PROP_CharPostureAsian, maApiData.maDesc.Slant);
        rPropMap.setProperty( PROP_CharPostureComplex, maApiData.maDesc.Slant);
    }
    // character color
    if( maUsedFlags.mbColorUsed )
        rPropMap.setProperty( PROP_CharColor, maApiData.mnColor);
    // underline style
    if( maUsedFlags.mbUnderlineUsed )
        rPropMap.setProperty( PROP_CharUnderline, maApiData.maDesc.Underline);
    // strike out style
    if( maUsedFlags.mbStrikeoutUsed )
        rPropMap.setProperty( PROP_CharStrikeout, maApiData.maDesc.Strikeout);
    // outline style
    if( maUsedFlags.mbOutlineUsed )
        rPropMap.setProperty( PROP_CharContoured, maApiData.mbOutline);
    // shadow style
    if( maUsedFlags.mbShadowUsed )
        rPropMap.setProperty( PROP_CharShadowed, maApiData.mbShadow);
    // escapement
    if( maUsedFlags.mbEscapementUsed )
    {
        rPropMap.setProperty( PROP_CharEscapement, maApiData.mnEscapement);
        rPropMap.setProperty( PROP_CharEscapementHeight, maApiData.mnEscapeHeight);
    }
}

void Font::writeToPropertySet( PropertySet& rPropSet ) const
{
    PropertyMap aPropMap;
    writeToPropertyMap( aPropMap );
    rPropSet.setProperties( aPropMap );
}

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

void AlignmentModel::setBiffHorAlign( sal_uInt8 nHorAlign )
{
    static const sal_Int32 spnHorAligns[] = {
        XML_general, XML_left, XML_center, XML_right,
        XML_fill, XML_justify, XML_centerContinuous, XML_distributed };
    mnHorAlign = STATIC_ARRAY_SELECT( spnHorAligns, nHorAlign, XML_general );
}

void AlignmentModel::setBiffVerAlign( sal_uInt8 nVerAlign )
{
    static const sal_Int32 spnVerAligns[] = {
        XML_top, XML_center, XML_bottom, XML_justify, XML_distributed };
    mnVerAlign = STATIC_ARRAY_SELECT( spnVerAligns, nVerAlign, XML_bottom );
}

ApiAlignmentData::ApiAlignmentData() :
    meHorJustify( css::table::CellHoriJustify_STANDARD ),
    mnHorJustifyMethod( css::table::CellJustifyMethod::AUTO ),
    mnVerJustify( css::table::CellVertJustify2::STANDARD ),
    mnVerJustifyMethod( css::table::CellJustifyMethod::AUTO ),
    meOrientation( css::table::CellOrientation_STANDARD ),
    mnRotation( 0 ),
    mnWritingMode( css::text::WritingMode2::PAGE ),
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

Alignment::Alignment( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
}

void Alignment::importAlignment( const AttributeList& rAttribs )
{
    maModel.mnVerAlign     = rAttribs.getToken( XML_vertical, XML_bottom );
    maModel.mnTextDir      = rAttribs.getInteger( XML_readingOrder, OOX_XF_TEXTDIR_CONTEXT );
    maModel.mnRotation     = rAttribs.getInteger( XML_textRotation, OOX_XF_ROTATION_NONE );
    sal_Int32 nDefaultHorAlign = XML_general;
    if (maModel.mnRotation != OOX_XF_ROTATION_NONE)
    {
        if (maModel.mnRotation < 90 || maModel.mnRotation == 180)
        {
            nDefaultHorAlign = XML_left;
        }
        else
        {
            nDefaultHorAlign = XML_right;
        }
    }
    maModel.mnHorAlign     = rAttribs.getToken( XML_horizontal, nDefaultHorAlign );
    maModel.mnIndent       = rAttribs.getInteger( XML_indent, OOX_XF_INDENT_NONE );
    maModel.mbWrapText     = rAttribs.getBool( XML_wrapText, false );
    maModel.mbShrink       = rAttribs.getBool( XML_shrinkToFit, false );
    maModel.mbJustLastLine = rAttribs.getBool( XML_justifyLastLine, false );
}

void Alignment::setBiff12Data( sal_uInt32 nFlags )
{
    maModel.setBiffHorAlign( extractValue< sal_uInt8 >( nFlags, 16, 3 ) );
    maModel.setBiffVerAlign( extractValue< sal_uInt8 >( nFlags, 19, 3 ) );
    maModel.mnTextDir      = extractValue< sal_Int32 >( nFlags, 26, 2 );
    maModel.mnRotation     = extractValue< sal_Int32 >( nFlags, 0, 8 );
    maModel.mnIndent       = extractValue< sal_uInt8 >( nFlags, 8, 8 );
    maModel.mbWrapText     = getFlag( nFlags, BIFF12_XF_WRAPTEXT );
    maModel.mbShrink       = getFlag( nFlags, BIFF12_XF_SHRINK );
    maModel.mbJustLastLine = getFlag( nFlags, BIFF12_XF_JUSTLASTLINE );
}

void Alignment::finalizeImport()
{
    // horizontal alignment
    switch( maModel.mnHorAlign )
    {
        case XML_center:            maApiData.meHorJustify = css::table::CellHoriJustify_CENTER;    break;
        case XML_centerContinuous:  maApiData.meHorJustify = css::table::CellHoriJustify_CENTER;    break;
        case XML_distributed:       maApiData.meHorJustify = css::table::CellHoriJustify_BLOCK;     break;
        case XML_fill:              maApiData.meHorJustify = css::table::CellHoriJustify_REPEAT;    break;
        case XML_general:           maApiData.meHorJustify = css::table::CellHoriJustify_STANDARD;  break;
        case XML_justify:           maApiData.meHorJustify = css::table::CellHoriJustify_BLOCK;     break;
        case XML_left:              maApiData.meHorJustify = css::table::CellHoriJustify_LEFT;      break;
        case XML_right:             maApiData.meHorJustify = css::table::CellHoriJustify_RIGHT;     break;
    }

    if (maModel.mnHorAlign == XML_distributed)
        maApiData.mnHorJustifyMethod = css::table::CellJustifyMethod::DISTRIBUTE;

    // vertical alignment
    switch( maModel.mnVerAlign )
    {
        case XML_bottom:        maApiData.mnVerJustify = css::table::CellVertJustify2::BOTTOM;    break;
        case XML_center:        maApiData.mnVerJustify = css::table::CellVertJustify2::CENTER;    break;
        case XML_distributed:   maApiData.mnVerJustify = css::table::CellVertJustify2::BLOCK;     break;
        case XML_justify:       maApiData.mnVerJustify = css::table::CellVertJustify2::BLOCK;     break;
        case XML_top:           maApiData.mnVerJustify = css::table::CellVertJustify2::TOP;       break;
    }

    if (maModel.mnVerAlign == XML_distributed)
        maApiData.mnVerJustifyMethod = css::table::CellJustifyMethod::DISTRIBUTE;

    /*  indentation: expressed as number of blocks of 3 space characters in
        OOXML. */
    sal_Int32 nIndent = getUnitConverter().scaleToMm100( 3.0 * maModel.mnIndent, Unit::Space );
    if( (0 <= nIndent) && (nIndent <= SAL_MAX_INT16) )
        maApiData.mnIndent = static_cast< sal_Int16 >( nIndent );

    // complex text direction
    switch( maModel.mnTextDir )
    {
        case OOX_XF_TEXTDIR_CONTEXT:    maApiData.mnWritingMode = css::text::WritingMode2::PAGE;   break;
        case OOX_XF_TEXTDIR_LTR:        maApiData.mnWritingMode = css::text::WritingMode2::LR_TB;  break;
        case OOX_XF_TEXTDIR_RTL:        maApiData.mnWritingMode = css::text::WritingMode2::RL_TB;  break;
    }

    // rotation: 0-90 means 0 to 90 degrees ccw, 91-180 means 1 to 90 degrees cw, 255 means stacked
    sal_Int32 nOoxRot = maModel.mnRotation;
    maApiData.mnRotation = ((0 <= nOoxRot) && (nOoxRot <= 90)) ?
        (100 * nOoxRot) :
        (((91 <= nOoxRot) && (nOoxRot <= 180)) ? (100 * (450 - nOoxRot)) : 0);

    // "Orientation" property used for character stacking
    maApiData.meOrientation = (nOoxRot == OOX_XF_ROTATION_STACKED) ?
        css::table::CellOrientation_STACKED : css::table::CellOrientation_STANDARD;

    // alignment flags (#i84960 automatic line break, if vertically justified/distributed)
    maApiData.mbWrapText = maModel.mbWrapText || (maModel.mnVerAlign == XML_distributed) || (maModel.mnVerAlign == XML_justify);
    maApiData.mbShrink = maModel.mbShrink;

}

::SvxCellVerJustify Alignment::GetScVerAlign() const
{
    ::SvxCellVerJustify nVert = ::SvxCellVerJustify::Standard;
    switch ( maApiData.mnVerJustify )
    {
        case css::table::CellVertJustify2::BOTTOM:
            nVert = ::SvxCellVerJustify::Bottom;
            break;
        case css::table::CellVertJustify2::CENTER:
            nVert = ::SvxCellVerJustify::Center;
            break;
        case css::table::CellVertJustify2::TOP:
            nVert = ::SvxCellVerJustify::Top;
            break;
        case css::table::CellVertJustify2::BLOCK:
            nVert = ::SvxCellVerJustify::Block;
            break;
        case css::table::CellVertJustify2::STANDARD:
        default:
            nVert = ::SvxCellVerJustify::Standard;
            break;
    }
    return nVert;
}

::SvxCellHorJustify Alignment::GetScHorAlign() const
{
    ::SvxCellHorJustify nHori = ::SvxCellHorJustify::Standard;
    switch( maApiData.meHorJustify )
    {
        case css::table::CellHoriJustify_LEFT:
            nHori = ::SvxCellHorJustify::Left;
            break;
        case css::table::CellHoriJustify_CENTER:
            nHori = ::SvxCellHorJustify::Center;
            break;
        case css::table::CellHoriJustify_RIGHT:
            nHori = ::SvxCellHorJustify::Right;
            break;
        case css::table::CellHoriJustify_BLOCK:
            nHori = ::SvxCellHorJustify::Block;
            break;
        case css::table::CellHoriJustify_REPEAT:
            nHori = ::SvxCellHorJustify::Repeat;
            break;
        case css::table::CellHoriJustify_STANDARD:
        default:
            nHori = ::SvxCellHorJustify::Standard;
            break;
    }
    return nHori;
}

SvxFrameDirection Alignment::GetScFrameDir() const
{
    SvxFrameDirection eFrameDir = SvxFrameDirection::Environment;
    switch( maApiData.mnWritingMode )
    {
        case css::text::WritingMode2::PAGE:
            eFrameDir = SvxFrameDirection::Environment;
            break;
        case css::text::WritingMode2::LR_TB:
            eFrameDir = SvxFrameDirection::Horizontal_LR_TB;
            break;
        case css::text::WritingMode2::RL_TB:
            eFrameDir = SvxFrameDirection::Horizontal_RL_TB;
            break;
        default:
            OSL_FAIL( "GetScFrameDir - unknown CTL text direction" );
    }
    return eFrameDir;
}

void Alignment::fillToItemSet( SfxItemSet& rItemSet, bool bSkipPoolDefs ) const
{
    // horizontal alignment
    ScfTools::PutItem( rItemSet, SvxHorJustifyItem( GetScHorAlign(), ATTR_HOR_JUSTIFY ), bSkipPoolDefs );
    ScfTools::PutItem( rItemSet, SvxJustifyMethodItem( ( maApiData.mnHorJustifyMethod == css::table::CellJustifyMethod::DISTRIBUTE ) ? ::SvxCellJustifyMethod::Distribute : ::SvxCellJustifyMethod::Auto, ATTR_HOR_JUSTIFY_METHOD ), bSkipPoolDefs );
    ScfTools::PutItem( rItemSet, SvxVerJustifyItem( GetScVerAlign(), ATTR_VER_JUSTIFY ), bSkipPoolDefs );
    // vertical alignment
    ScfTools::PutItem( rItemSet, SvxJustifyMethodItem( ( maApiData.mnVerJustifyMethod == css::table::CellJustifyMethod::DISTRIBUTE ) ? ::SvxCellJustifyMethod::Distribute : ::SvxCellJustifyMethod::Auto, ATTR_VER_JUSTIFY_METHOD ), bSkipPoolDefs );

    // CTL text direction
    ScfTools::PutItem( rItemSet, SvxFrameDirectionItem( GetScFrameDir(), ATTR_WRITINGDIR ), bSkipPoolDefs );
    // set an angle in the range from -90 to 90 degrees
    ScfTools::PutItem( rItemSet, SfxInt32Item( ATTR_ROTATE_VALUE, maApiData.mnRotation ), bSkipPoolDefs );
    // Orientation
    ScfTools::PutItem( rItemSet, SfxBoolItem( ATTR_STACKED, maApiData.meOrientation == css::table::CellOrientation_STACKED ), bSkipPoolDefs );
    // indent
    ScfTools::PutItem( rItemSet, SfxUInt16Item( ATTR_INDENT, maApiData.mnIndent ), bSkipPoolDefs );
    // line wrap
    ScfTools::PutItem( rItemSet, SfxBoolItem( ATTR_LINEBREAK, maApiData.mbWrapText ), bSkipPoolDefs );
    ScfTools::PutItem( rItemSet, SfxBoolItem( ATTR_SHRINKTOFIT, maApiData.mbShrink ), bSkipPoolDefs );
}

ProtectionModel::ProtectionModel() :
    mbLocked( true ),   // default in Excel and Calc
    mbHidden( false )
{
}

ApiProtectionData::ApiProtectionData() :
    maCellProt( true, false, false, false )
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

Protection::Protection( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
}

void Protection::importProtection( const AttributeList& rAttribs )
{
    maModel.mbLocked = rAttribs.getBool( XML_locked, true );
    maModel.mbHidden = rAttribs.getBool( XML_hidden, false );
}

void Protection::setBiff12Data( sal_uInt32 nFlags )
{
    maModel.mbLocked = getFlag( nFlags, BIFF12_XF_LOCKED );
    maModel.mbHidden = getFlag( nFlags, BIFF12_XF_HIDDEN );
}

void Protection::finalizeImport()
{
    maApiData.maCellProt.IsLocked = maModel.mbLocked;
    maApiData.maCellProt.IsFormulaHidden = maModel.mbHidden;
}

void Protection::fillToItemSet( SfxItemSet& rItemSet, bool bSkipPoolDefs ) const
{
    ScfTools::PutItem( rItemSet, ScProtectionAttr( maApiData.maCellProt.IsLocked, maApiData.maCellProt.IsFormulaHidden ), bSkipPoolDefs );
}

namespace {

bool lcl_isBorder(const css::table::BorderLine& rBorder)
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

ApiBorderData::ApiBorderData() :
    mbBorderUsed( false ),
    mbDiagUsed( false )
{
}

bool ApiBorderData::hasAnyOuterBorder() const
{
    return
        ( lcl_isBorder( maTop )    &&  maTop.OuterLineWidth > 0   ) ||
        ( lcl_isBorder( maBottom ) && maBottom.OuterLineWidth > 0 ) ||
        ( lcl_isBorder( maLeft )   && maLeft.OuterLineWidth > 0   ) ||
        ( lcl_isBorder( maRight )  && maRight.OuterLineWidth > 0  );
}

namespace {

void lclSetBorderLineWidth( BorderLine& rBorderLine,
        sal_Int16 nOuter, sal_Int16 nDist = API_LINE_NONE, sal_Int16 nInner = API_LINE_NONE )
{
    rBorderLine.OuterLineWidth = nOuter;
    rBorderLine.LineDistance = nDist;
    rBorderLine.InnerLineWidth = nInner;
}

} // namespace

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

void Border::importBorder( SequenceInputStream& rStrm )
{
    sal_uInt8 nFlags = rStrm.readuInt8();
    maModel.mbDiagTLtoBR = getFlag( nFlags, BIFF12_BORDER_DIAG_TLBR );
    maModel.mbDiagBLtoTR = getFlag( nFlags, BIFF12_BORDER_DIAG_BLTR );
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

void Border::importDxfBorder( sal_Int32 nElement, SequenceInputStream& rStrm )
{
    SAL_WARN_IF( !mbDxf, "sc", "Border::importDxfBorder - missing conditional formatting flag" );
    if( BorderLineModel* pBorderLine = getBorderLine( nElement ) )
    {
        sal_uInt16 nStyle;
        rStrm >> pBorderLine->maColor;
        nStyle = rStrm.readuInt16();
        pBorderLine->setBiffStyle( nStyle );
        pBorderLine->mbUsed = true;
    }
}

void Border::finalizeImport( bool bRTL )
{
    if ( bRTL )
    {
        BorderLineModel aTmp = maModel.maLeft;
        maModel.maLeft = maModel.maRight;
        maModel.maRight = aTmp;
    }
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

void Border::fillToItemSet( SfxItemSet& rItemSet, bool bSkipPoolDefs ) const
{
    if( maApiData.mbBorderUsed )
    {
         SvxBoxItem aBoxItem( ATTR_BORDER );
         ::editeng::SvxBorderLine aLine;

         if (SvxBoxItem::LineToSvxLine(maApiData.maLeft, aLine, false))
         {
             aBoxItem.SetLine( &aLine, SvxBoxItemLine::LEFT );
         }
         if (SvxBoxItem::LineToSvxLine(maApiData.maRight, aLine, false))
         {
             aBoxItem.SetLine( &aLine, SvxBoxItemLine::RIGHT );
         }
         if (SvxBoxItem::LineToSvxLine(maApiData.maTop, aLine, false))
         {
             aBoxItem.SetLine( &aLine, SvxBoxItemLine::TOP );
         }
         if (SvxBoxItem::LineToSvxLine(maApiData.maBottom, aLine, false))
         {
             aBoxItem.SetLine( &aLine, SvxBoxItemLine::BOTTOM );
         }
         ScfTools::PutItem( rItemSet, aBoxItem, bSkipPoolDefs );
    }
    if ( maApiData.mbDiagUsed )
    {
        SvxLineItem aTLBRItem( ATTR_BORDER_TLBR );
        SvxLineItem aBLTRItem( ATTR_BORDER_BLTR );
        ::editeng::SvxBorderLine aLine;
        if (SvxBoxItem::LineToSvxLine(maApiData.maTLtoBR, aLine, false))
        {
            aTLBRItem.SetLine( &aLine );
        }
        if (SvxBoxItem::LineToSvxLine(maApiData.maBLtoTR, aLine, false))
        {
            aBLTRItem.SetLine( &aLine );
        }
        ScfTools::PutItem( rItemSet, aTLBRItem, bSkipPoolDefs );
        ScfTools::PutItem( rItemSet, aBLTRItem, bSkipPoolDefs );
    }
}

BorderLineModel* Border::getBorderLine( sal_Int32 nElement )
{
    switch( nElement )
    {
        case XLS_TOKEN( left ):     return &maModel.maLeft;
        case XLS_TOKEN( start ):     return &maModel.maLeft;
        case XLS_TOKEN( right ):    return &maModel.maRight;
        case XLS_TOKEN( end ):    return &maModel.maRight;
        case XLS_TOKEN( top ):      return &maModel.maTop;
        case XLS_TOKEN( bottom ):   return &maModel.maBottom;
        case XLS_TOKEN( diagonal ): return &maModel.maDiagonal;
    }
    return nullptr;
}

bool Border::convertBorderLine( BorderLine2& rBorderLine, const BorderLineModel& rModel )
{
    // Document: sc/qa/unit/data/README.cellborders

    rBorderLine.Color = sal_Int32(rModel.maColor.getColor( getBaseFilter().getGraphicHelper(), API_RGB_BLACK ));
    switch( rModel.mnStyle )
    {
        case XML_dashDot:
            lclSetBorderLineWidth( rBorderLine, API_LINE_THIN );
            rBorderLine.LineStyle = BorderLineStyle::DASH_DOT;
        break;
        case XML_dashDotDot:
            lclSetBorderLineWidth( rBorderLine, API_LINE_THIN );
            rBorderLine.LineStyle = BorderLineStyle::DASH_DOT_DOT;
        break;
        case XML_dashed:
            lclSetBorderLineWidth( rBorderLine, API_LINE_THIN );
            rBorderLine.LineStyle = BorderLineStyle::FINE_DASHED;
        break;
        case XML_dotted:
            lclSetBorderLineWidth( rBorderLine, API_LINE_THIN );
            rBorderLine.LineStyle = BorderLineStyle::DOTTED;
        break;
        case XML_double:
            lclSetBorderLineWidth( rBorderLine, 10, 15, 10 );
            rBorderLine.LineStyle = BorderLineStyle::DOUBLE_THIN;
        break;
        case XML_hair:              lclSetBorderLineWidth( rBorderLine, API_LINE_HAIR );    break;
        case XML_medium:            lclSetBorderLineWidth( rBorderLine, API_LINE_MEDIUM );  break;
        case XML_mediumDashDot:
            lclSetBorderLineWidth( rBorderLine, API_LINE_MEDIUM );
            rBorderLine.LineStyle = BorderLineStyle::DASH_DOT;
            break;
        case XML_mediumDashDotDot:
            lclSetBorderLineWidth( rBorderLine, API_LINE_MEDIUM );
            rBorderLine.LineStyle = BorderLineStyle::DASH_DOT_DOT;
            break;
        case XML_mediumDashed:
            lclSetBorderLineWidth( rBorderLine, API_LINE_MEDIUM );
            rBorderLine.LineStyle = BorderLineStyle::DASHED;
        break;
        case XML_none:              lclSetBorderLineWidth( rBorderLine, API_LINE_NONE );    break;
        case XML_slantDashDot:
            lclSetBorderLineWidth( rBorderLine, API_LINE_MEDIUM );
            rBorderLine.LineStyle = BorderLineStyle::FINE_DASHED;
            break;
        case XML_thick:             lclSetBorderLineWidth( rBorderLine, API_LINE_THICK );   break;
        case XML_thin:              lclSetBorderLineWidth( rBorderLine, API_LINE_THIN );    break;
        default:                    lclSetBorderLineWidth( rBorderLine, API_LINE_NONE );    break;
    }
    return rModel.mbUsed;
}

PatternFillModel::PatternFillModel( bool bDxf ) :
    mnPattern( XML_none ),
    mbPattColorUsed( !bDxf ),
    mbFillColorUsed( !bDxf ),
    mbPatternUsed( !bDxf )
{
    maPatternColor.setIndexed( OOX_COLOR_WINDOWTEXT );
    maFillColor.setIndexed( OOX_COLOR_WINDOWBACK );
}

void PatternFillModel::setBiffPattern( sal_Int32 nPattern )
{
    static const sal_Int32 spnPatternIds[] = {
        XML_none, XML_solid, XML_mediumGray, XML_darkGray,
        XML_lightGray, XML_darkHorizontal, XML_darkVertical, XML_darkDown,
        XML_darkUp, XML_darkGrid, XML_darkTrellis, XML_lightHorizontal,
        XML_lightVertical, XML_lightDown, XML_lightUp, XML_lightGrid,
        XML_lightTrellis, XML_gray125, XML_gray0625 };
    mnPattern = STATIC_ARRAY_SELECT( spnPatternIds, nPattern, XML_none );
}

GradientFillModel::GradientFillModel() :
    mnType( XML_linear ),
    mfAngle( 0.0 ),
    mfLeft( 0.0 ),
    mfRight( 0.0 ),
    mfTop( 0.0 ),
    mfBottom( 0.0 )
{
}

void GradientFillModel::readGradient( SequenceInputStream& rStrm )
{
    sal_Int32 nType;
    nType = rStrm.readInt32();
    mfAngle = rStrm.readDouble();
    mfLeft = rStrm.readDouble();
    mfRight = rStrm.readDouble();
    mfTop = rStrm.readDouble();
    mfBottom = rStrm.readDouble();
    static const sal_Int32 spnTypes[] = { XML_linear, XML_path };
    mnType = STATIC_ARRAY_SELECT( spnTypes, nType, XML_TOKEN_INVALID );
}

void GradientFillModel::readGradientStop( SequenceInputStream& rStrm, bool bDxf )
{
    Color aColor;
    double fPosition;
    if( bDxf )
    {
        rStrm.skip( 2 );
        fPosition = rStrm.readDouble();
        rStrm >> aColor;
    }
    else
    {
        rStrm >> aColor;
        fPosition = rStrm.readDouble();
    }
    if( !rStrm.isEof() && (fPosition >= 0.0) )
        maColors[ fPosition ] = aColor;
}

ApiSolidFillData::ApiSolidFillData() :
    mnColor( API_RGB_TRANSPARENT ),
    mbTransparent( true ),
    mbUsed( false )
{
}

namespace {

sal_Int32 lclGetMixedColorComp( sal_Int32 nPatt, sal_Int32 nFill, sal_Int32 nAlpha )
{
    return ((nPatt - nFill) * nAlpha) / 0x80 + nFill;
}

::Color lclGetMixedColor( ::Color nPattColor, ::Color nFillColor, sal_Int32 nAlpha )
{
    return ::Color(
        lclGetMixedColorComp( nPattColor.GetRed(), nFillColor.GetRed(), nAlpha ),
        lclGetMixedColorComp( nPattColor.GetGreen(), nFillColor.GetGreen(), nAlpha ),
        lclGetMixedColorComp( nPattColor.GetBlue(), nFillColor.GetBlue(), nAlpha ) );
}

} // namespace

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

void Fill::importFill( SequenceInputStream& rStrm )
{
    SAL_WARN_IF( mbDxf, "sc", "Fill::importFill - unexpected conditional formatting flag" );
    sal_Int32 nPattern = rStrm.readInt32();
    if( nPattern == BIFF12_FILL_GRADIENT )
    {
        mxGradientModel.reset( new GradientFillModel );
        sal_Int32 nStopCount;
        rStrm.skip( 16 );
        mxGradientModel->readGradient( rStrm );
        nStopCount = rStrm.readInt32();
        for( sal_Int32 nStop = 0; (nStop < nStopCount) && !rStrm.isEof(); ++nStop )
            mxGradientModel->readGradientStop( rStrm, false );
    }
    else
    {
        mxPatternModel.reset( new PatternFillModel( mbDxf ) );
        mxPatternModel->setBiffPattern( nPattern );
        rStrm >> mxPatternModel->maPatternColor >> mxPatternModel->maFillColor;
    }
}

void Fill::importDxfPattern( SequenceInputStream& rStrm )
{
    SAL_WARN_IF( !mbDxf, "sc", "Fill::importDxfPattern - missing conditional formatting flag" );
    if( !mxPatternModel )
        mxPatternModel.reset( new PatternFillModel( mbDxf ) );
    mxPatternModel->setBiffPattern( rStrm.readuInt8() );
    mxPatternModel->mbPatternUsed = true;
}

void Fill::importDxfFgColor( SequenceInputStream& rStrm )
{
    SAL_WARN_IF( !mbDxf, "sc", "Fill::importDxfFgColor - missing conditional formatting flag" );
    if( !mxPatternModel )
        mxPatternModel.reset( new PatternFillModel( mbDxf ) );
    mxPatternModel->maPatternColor.importColor( rStrm );
    mxPatternModel->mbPattColorUsed = true;
}

void Fill::importDxfBgColor( SequenceInputStream& rStrm )
{
    SAL_WARN_IF( !mbDxf, "sc", "Fill::importDxfBgColor - missing conditional formatting flag" );
    if( !mxPatternModel )
        mxPatternModel.reset( new PatternFillModel( mbDxf ) );
    mxPatternModel->maFillColor.importColor( rStrm );
    mxPatternModel->mbFillColorUsed = true;
}

void Fill::importDxfGradient( SequenceInputStream& rStrm )
{
    SAL_WARN_IF( !mbDxf, "sc", "Fill::importDxfGradient - missing conditional formatting flag" );
    if( !mxGradientModel )
        mxGradientModel.reset( new GradientFillModel );
    mxGradientModel->readGradient( rStrm );
}

void Fill::importDxfStop( SequenceInputStream& rStrm )
{
    SAL_WARN_IF( !mbDxf, "sc", "Fill::importDxfStop - missing conditional formatting flag" );
    if( !mxGradientModel )
        mxGradientModel.reset( new GradientFillModel );
    mxGradientModel->readGradientStop( rStrm, true );
}

void Fill::finalizeImport()
{
    const GraphicHelper& rGraphicHelper = getBaseFilter().getGraphicHelper();

    if( mxPatternModel.get() )
    {
        // finalize the OOXML data struct
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

            ::Color nWinTextColor = rGraphicHelper.getSystemColor( XML_windowText );
            ::Color nWinColor = rGraphicHelper.getSystemColor( XML_window );

            if( !rModel.mbPattColorUsed )
                rModel.maPatternColor.setAuto();
            ::Color nPattColor = rModel.maPatternColor.getColor( rGraphicHelper, nWinTextColor );

            if( !rModel.mbFillColorUsed )
                rModel.maFillColor.setAuto();
            ::Color nFillColor = rModel.maFillColor.getColor( rGraphicHelper, nWinColor );

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
            ::Color nEndColor = aIt->second.getColor( rGraphicHelper, API_RGB_WHITE );
            maApiData.mnColor = lclGetMixedColor( maApiData.mnColor, nEndColor, 0x40 );
            maApiData.mbTransparent = false;
        }
    }
}

void Fill::fillToItemSet( SfxItemSet& rItemSet, bool bSkipPoolDefs ) const
{
    if( maApiData.mbUsed )
    {
        SvxBrushItem aBrushItem( ATTR_BACKGROUND );
        if ( maApiData.mbTransparent )
        {
            aBrushItem.SetColor( COL_TRANSPARENT );
        }
        else
        {
            aBrushItem.SetColor( maApiData.mnColor  );
        }
        ScfTools::PutItem( rItemSet, aBrushItem, bSkipPoolDefs );
    }
}

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

Xf::AttrList::AttrList(const ScPatternAttr* pDefPattern):
    mbLatinNumFmtOnly(true),
    mpDefPattern(pDefPattern)
{}

Xf::Xf( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    mnScNumFmt(0),
    maAlignment( rHelper ),
    maProtection( rHelper ),
    meRotationRef( css::table::CellVertJustify2::STANDARD ),
    mpStyleSheet( nullptr )
{
}

void Xf::importXf( const AttributeList& rAttribs, bool bCellXf )
{
    maModel.mbCellXf = bCellXf;
    // tdf#70565 Set proper default value to "0" of xfId attribute
    // When xfId is not exist during .xlsx import
    // it must have values set to "0".
    // This doesn't impact spreadsheets created with MS Excel,
    // as xfId attribute is always created during export to .xlsx
    // Not setting "0" value is causing wrong .xlsx import by LibreOffice,
    // for spreadsheets created by external applications (ex. SAP BI).
    if ( maModel.mbCellXf )
    {
        maModel.mnStyleXfId = rAttribs.getInteger( XML_xfId, 0 );
    }
    else
    {
        maModel.mnStyleXfId = rAttribs.getInteger( XML_xfId, -1 );
    }
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

void Xf::importXf( SequenceInputStream& rStrm, bool bCellXf )
{
    maModel.mbCellXf = bCellXf;
    maModel.mnStyleXfId = rStrm.readuInt16();
    maModel.mnNumFmtId = rStrm.readuInt16();
    maModel.mnFontId = rStrm.readuInt16();
    maModel.mnFillId = rStrm.readuInt16();
    maModel.mnBorderId = rStrm.readuInt16();
    sal_uInt32 nFlags = rStrm.readuInt32();
    maAlignment.setBiff12Data( nFlags );
    maProtection.setBiff12Data( nFlags );
    // used flags, see comments in Xf::setBiffUsedFlags()
    sal_uInt16 nUsedFlags = rStrm.readuInt16();
    maModel.mbFontUsed   = maModel.mbCellXf == getFlag( nUsedFlags, BIFF12_XF_FONT_USED );
    maModel.mbNumFmtUsed = maModel.mbCellXf == getFlag( nUsedFlags, BIFF12_XF_NUMFMT_USED );
    maModel.mbAlignUsed  = maModel.mbCellXf == getFlag( nUsedFlags, BIFF12_XF_ALIGN_USED );
    maModel.mbProtUsed   = maModel.mbCellXf == getFlag( nUsedFlags, BIFF12_XF_PROT_USED );
    maModel.mbBorderUsed = maModel.mbCellXf == getFlag( nUsedFlags, BIFF12_XF_BORDER_USED );
    maModel.mbAreaUsed   = maModel.mbCellXf == getFlag( nUsedFlags, BIFF12_XF_AREA_USED );
}

void Xf::finalizeImport()
{
    // alignment and protection
    maAlignment.finalizeImport();
    maProtection.finalizeImport();
}

FontRef Xf::getFont() const
{
    return getStyles().getFont( maModel.mnFontId );
}

void Xf::applyPatternToAttrList( AttrList& rAttrs, SCROW nRow1, SCROW nRow2, sal_Int32 nNumFmtId )
{
    createPattern();
    ScPatternAttr& rPat = *mpPattern;
    ScDocument& rDoc = getScDocument();
    if ( isCellXf() )
    {
        StylesBuffer& rStyles = getStyles();
        rStyles.createCellStyle( maModel.mnStyleXfId );

        mpStyleSheet = rStyles.getCellStyleSheet( maModel.mnStyleXfId );
        if ( mpStyleSheet )
        {
            //rDoc.ApplySelectionStyle( static_cast<ScStyleSheet&>(*mpStyleSheet), rMarkData );
            rPat.SetStyleSheet(mpStyleSheet, false);
        }
        else
        {
            ScStyleSheetPool* pStylePool = rDoc.GetStyleSheetPool();
            if (pStylePool)
            {
                ScStyleSheet* pStyleSheet = static_cast<ScStyleSheet*>(
                    pStylePool->Find(
                        ScResId(STR_STYLENAME_STANDARD), SfxStyleFamily::Para));

                if (pStyleSheet)
                    rPat.SetStyleSheet( pStyleSheet, false );
            }
        }
    }
    if ( nNumFmtId >= 0 )
    {
        ScPatternAttr aNumPat(rDoc.GetPool());
        mnScNumFmt = getStyles().writeNumFmtToItemSet( aNumPat.GetItemSet(), nNumFmtId, false );
        rPat.GetItemSet().Put(aNumPat.GetItemSet());
    }

    if (!sc::NumFmtUtil::isLatinScript(mnScNumFmt, rDoc))
        rAttrs.mbLatinNumFmtOnly = false;

    if (rPat.GetStyleName())
    {
        // Check for a gap between the last entry and this one.
        bool bHasGap = false;
        if (rAttrs.maAttrs.empty() && nRow1 > 0)
            // First attribute range doesn't start at row 0.
            bHasGap = true;

        if (!rAttrs.maAttrs.empty() && rAttrs.maAttrs.back().nEndRow + 1 < nRow1)
            bHasGap = true;

        if (bHasGap)
        {
            // Fill this gap with the default pattern.
            ScAttrEntry aEntry;
            aEntry.nEndRow = nRow1 - 1;
            aEntry.pPattern = static_cast<const ScPatternAttr*>(&rDoc.GetPool()->Put(*rAttrs.mpDefPattern));
            rAttrs.maAttrs.push_back(aEntry);

            // Check if the default pattern is 'General'.
            if (!sc::NumFmtUtil::isLatinScript(*aEntry.pPattern, rDoc))
                rAttrs.mbLatinNumFmtOnly = false;
        }

        ScAttrEntry aEntry;
        aEntry.nEndRow = nRow2;
        aEntry.pPattern = static_cast<const ScPatternAttr*>(&rDoc.GetPool()->Put(rPat));
        rAttrs.maAttrs.push_back(aEntry);

        if (!sc::NumFmtUtil::isLatinScript(*aEntry.pPattern, rDoc))
            rAttrs.mbLatinNumFmtOnly = false;
    }
}

void Xf::writeToDoc( ScDocumentImport& rDoc, const ScRange& rRange )
{
    const StylesBuffer& rStyles = getStyles();

    if (isCellXf())
    {
        // Cell style name.
        OUString aStyleName = rStyles.createCellStyle(maModel.mnStyleXfId);

        ScStyleSheet* pStyleSheet =
            static_cast<ScStyleSheet*>(
                rDoc.getDoc().GetStyleSheetPool()->Find(aStyleName, SfxStyleFamily::Para));

        if (pStyleSheet)
        {
            rDoc.getDoc().ApplyStyleAreaTab(
                rRange.aStart.Col(), rRange.aStart.Row(), rRange.aEnd.Col(), rRange.aEnd.Row(), rRange.aStart.Tab(),
                *pStyleSheet);
        }
    }

    const ScPatternAttr& rAttr = createPattern();
    rDoc.getDoc().ApplyPatternAreaTab(
        rRange.aStart.Col(), rRange.aStart.Row(), rRange.aEnd.Col(), rRange.aEnd.Row(), rRange.aStart.Tab(), rAttr);
}

const ::ScPatternAttr&
Xf::createPattern( bool bSkipPoolDefs )
{
    if( mpPattern.get() )
        return *mpPattern;
    mpPattern.reset( new ::ScPatternAttr( getScDocument().GetPool() ) );
    SfxItemSet& rItemSet = mpPattern->GetItemSet();
    /*  Enables the used flags, if the formatting attributes differ from the
        style XF. In cell XFs Excel uses the cell attributes, if they differ
        from the parent style XF (even if the used flag is switched off).
        #109899# ...or if the respective flag is not set in parent style XF.
     */
    StylesBuffer& rStyles = getStyles();

    const Xf* pStyleXf = isCellXf() ? rStyles.getStyleXf( maModel.mnStyleXfId ).get() : nullptr;
    if( pStyleXf && !mpStyleSheet )
    {
        rStyles.createCellStyle( maModel.mnStyleXfId );
        mpStyleSheet = rStyles.getCellStyleSheet(  maModel.mnStyleXfId );
        OSL_ENSURE( mpStyleSheet, "Xf::createPattern - no parentStyle created" );

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
            maModel.mbBorderUsed = !rStyleData.mbBorderUsed || !StylesBuffer::equalBorders( maModel.mnBorderId, rStyleData.mnBorderId );
        if( !maModel.mbAreaUsed )
            maModel.mbAreaUsed = !rStyleData.mbAreaUsed || !StylesBuffer::equalFills( maModel.mnFillId, rStyleData.mnFillId );
    }
    // cell protection
    if( maModel.mbProtUsed )
    {
        maProtection.fillToItemSet( rItemSet, bSkipPoolDefs );
    }

    // font
    if( maModel.mbFontUsed )
    {
        rStyles.writeFontToItemSet( rItemSet, maModel.mnFontId, bSkipPoolDefs );
    }

    // value format
    if( maModel.mbNumFmtUsed )
    {
        mnScNumFmt = rStyles.writeNumFmtToItemSet( rItemSet, maModel.mnNumFmtId, bSkipPoolDefs );
    }
    // alignment
    if( maModel.mbAlignUsed )
    {
        maAlignment.fillToItemSet( rItemSet, bSkipPoolDefs );
    }

    // border
    if( maModel.mbBorderUsed )
    {
        rStyles.writeBorderToItemSet( rItemSet, maModel.mnBorderId, bSkipPoolDefs );
    }

    // area
    if( maModel.mbAreaUsed )
    {
        rStyles.writeFillToItemSet( rItemSet, maModel.mnFillId, bSkipPoolDefs );
    }

    /*  #i38709# Decide which rotation reference mode to use. If any outer
        border line of the cell is set (either explicitly or via cell style),
        and the cell contents are rotated, set rotation reference to bottom of
        cell. This causes the borders to be painted rotated with the text. */
    if( const Alignment* pAlignment = maModel.mbAlignUsed ? &maAlignment : (pStyleXf ? &pStyleXf->maAlignment : nullptr) )
    {
        SvxRotateMode eRotateMode = SVX_ROTATE_MODE_STANDARD;
        sal_Int32 nBorderId = maModel.mbBorderUsed ? maModel.mnBorderId : (pStyleXf ? pStyleXf->maModel.mnBorderId : -1);
        if( const Border* pBorder = rStyles.getBorder( nBorderId ).get() )
        {
            if( (pAlignment->getApiData().mnRotation != 0) && pBorder->getApiData().hasAnyOuterBorder() )
            {
                meRotationRef = css::table::CellVertJustify2::BOTTOM;
                eRotateMode = SVX_ROTATE_MODE_BOTTOM;
            }
        }
        ScfTools::PutItem( rItemSet, SvxRotateModeItem( eRotateMode, ATTR_ROTATE_MODE ), bSkipPoolDefs );
    }

    return *mpPattern;
}

Dxf::Dxf( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper )
{
}

FontRef const & Dxf::createFont( bool bAlwaysNew )
{
    if( bAlwaysNew || !mxFont )
        mxFont.reset( new Font( *this, true ) );
    return mxFont;
}

BorderRef const & Dxf::createBorder( bool bAlwaysNew )
{
    if( bAlwaysNew || !mxBorder )
        mxBorder.reset( new Border( *this, true ) );
    return mxBorder;
}

FillRef const & Dxf::createFill( bool bAlwaysNew )
{
    if( bAlwaysNew || !mxFill )
        mxFill.reset( new Fill( *this, true ) );
    return mxFill;
}

void Dxf::importNumFmt( const AttributeList& rAttribs )
{
    // don't propagate number formats defined in Dxf entries
    // they can have the same id ( but different format codes ) as those
    // defined globally earlier. We discard the id defined in XML_numFmtId
    // and generate one ourselves ( this assumes that the normal numberformat
    // import has already taken place )
    sal_Int32 nNumFmtId  = getStyles().nextFreeNumFmtId();
    OUString aFmtCode = rAttribs.getXString( XML_formatCode, OUString() );
    mxNumFmt = getStyles().createNumFmt( nNumFmtId, aFmtCode );
}

void Dxf::importDxf( SequenceInputStream& rStrm )
{
    sal_Int32 nNumFmtId = -1;
    OUString aFmtCode;
    sal_uInt16 nRecCount;
    rStrm.skip( 4 );    // flags
    nRecCount = rStrm.readuInt16();
    for( sal_uInt16 nRec = 0; !rStrm.isEof() && (nRec < nRecCount); ++nRec )
    {
        sal_uInt16 nSubRecId, nSubRecSize;
        sal_Int64 nRecEnd = rStrm.tell();
        nSubRecId = rStrm.readuInt16();
        nSubRecSize = rStrm.readuInt16();
        nRecEnd += nSubRecSize;
        switch( nSubRecId )
        {
            case BIFF12_DXF_FILL_PATTERN:       createFill( false )->importDxfPattern( rStrm );                         break;
            case BIFF12_DXF_FILL_FGCOLOR:       createFill( false )->importDxfFgColor( rStrm );                         break;
            case BIFF12_DXF_FILL_BGCOLOR:       createFill( false )->importDxfBgColor( rStrm );                         break;
            case BIFF12_DXF_FILL_GRADIENT:      createFill( false )->importDxfGradient( rStrm );                        break;
            case BIFF12_DXF_FILL_STOP:          createFill( false )->importDxfStop( rStrm );                            break;
            case BIFF12_DXF_FONT_COLOR:         createFont( false )->importDxfColor( rStrm );                           break;
            case BIFF12_DXF_BORDER_TOP:         createBorder( false )->importDxfBorder( XLS_TOKEN( top ), rStrm );      break;
            case BIFF12_DXF_BORDER_BOTTOM:      createBorder( false )->importDxfBorder( XLS_TOKEN( bottom ), rStrm );   break;
            case BIFF12_DXF_BORDER_LEFT:        createBorder( false )->importDxfBorder( XLS_TOKEN( left ), rStrm );     break;
            case BIFF12_DXF_BORDER_RIGHT:       createBorder( false )->importDxfBorder( XLS_TOKEN( right ), rStrm );    break;
            case BIFF12_DXF_FONT_NAME:          createFont( false )->importDxfName( rStrm );                            break;
            case BIFF12_DXF_FONT_WEIGHT:        createFont( false )->importDxfWeight( rStrm );                          break;
            case BIFF12_DXF_FONT_UNDERLINE:     createFont( false )->importDxfUnderline( rStrm );                       break;
            case BIFF12_DXF_FONT_ESCAPEMENT:    createFont( false )->importDxfEscapement( rStrm );                      break;
            case BIFF12_DXF_FONT_ITALIC:        createFont( false )->importDxfFlag( XML_i, rStrm );                     break;
            case BIFF12_DXF_FONT_STRIKE:        createFont( false )->importDxfFlag( XML_strike, rStrm );                break;
            case BIFF12_DXF_FONT_OUTLINE:       createFont( false )->importDxfFlag( XML_outline, rStrm );               break;
            case BIFF12_DXF_FONT_SHADOW:        createFont( false )->importDxfFlag( XML_shadow, rStrm );                break;
            case BIFF12_DXF_FONT_HEIGHT:        createFont( false )->importDxfHeight( rStrm );                          break;
            case BIFF12_DXF_FONT_SCHEME:        createFont( false )->importDxfScheme( rStrm );                          break;
            case BIFF12_DXF_NUMFMT_CODE:        aFmtCode = BiffHelper::readString( rStrm, false );                      break;
            case BIFF12_DXF_NUMFMT_ID:          nNumFmtId = rStrm.readuInt16();                                         break;
        }
        rStrm.seek( nRecEnd );
    }
    OSL_ENSURE( !rStrm.isEof() && (rStrm.getRemaining() == 0), "Dxf::importDxf - unexpected remaining data" );
    mxNumFmt = getStyles().createNumFmt( nNumFmtId, aFmtCode );
}

void Dxf::finalizeImport()
{
    if( mxFont.get() )
        mxFont->finalizeImport();
    bool bRTL = false;
    // number format already finalized by the number formats buffer
    if( mxAlignment.get() )
    {
        mxAlignment->finalizeImport();
        // how do we detect RTL when text dir is OOX_XF_CONTEXT? ( seems you
        // would need access to the cell content, which we don't here )
        if ( mxAlignment->getModel().mnTextDir == OOX_XF_TEXTDIR_RTL )
            bRTL = true;
    }
    if( mxProtection.get() )
        mxProtection->finalizeImport();
    if( mxBorder.get() )
    {
        mxBorder->finalizeImport( bRTL );
    }
    if( mxFill.get() )
        mxFill->finalizeImport();
}

void Dxf::fillToItemSet( SfxItemSet& rSet ) const
{
    if (mxFont)
        mxFont->fillToItemSet(rSet, false);
    if (mxNumFmt)
        mxNumFmt->fillToItemSet(rSet);
    if (mxAlignment)
        mxAlignment->fillToItemSet(rSet);
    if (mxProtection)
        mxProtection->fillToItemSet(rSet);
    if (mxBorder)
        mxBorder->fillToItemSet(rSet);
    if (mxFill)
        mxFill->fillToItemSet(rSet);
}

namespace {

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
    nullptr,
    nullptr,
    nullptr,
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
const sal_Int32 snStyleNamesCount = static_cast< sal_Int32 >( SAL_N_ELEMENTS( sppcStyleNames ) );

OUString lclGetBuiltinStyleName( sal_Int32 nBuiltinId, const OUString& rName, sal_Int32 nLevel = 0 )
{
    OSL_ENSURE( (0 <= nBuiltinId) && (nBuiltinId < snStyleNamesCount), "lclGetBuiltinStyleName - unknown built-in style" );
    OUStringBuffer aStyleName("Excel Built-in ");
    if( (0 <= nBuiltinId) && (nBuiltinId < snStyleNamesCount) && (sppcStyleNames[ nBuiltinId ] != nullptr) )
        aStyleName.appendAscii( sppcStyleNames[ nBuiltinId ] );
    else if( !rName.isEmpty() )
        aStyleName.append( rName );
    else
        aStyleName.append( nBuiltinId );
    if( (nBuiltinId == OOX_STYLE_ROWLEVEL) || (nBuiltinId == OOX_STYLE_COLLEVEL) )
        aStyleName.append( nLevel );
    return aStyleName.makeStringAndClear();
}

OUString lclCreateStyleName( const CellStyleModel& rModel )
{
    return rModel.mbBuiltin ? lclGetBuiltinStyleName( rModel.mnBuiltinId, rModel.maName, rModel.mnLevel ) : rModel.maName;
}

} // namespace

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

CellStyle::CellStyle( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    mbCreated( false ),
    mpStyleSheet( nullptr )
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

void CellStyle::importCellStyle( SequenceInputStream& rStrm )
{
    sal_uInt16 nFlags;
    maModel.mnXfId = rStrm.readInt32();
    nFlags = rStrm.readuInt16();
    maModel.mnBuiltinId = rStrm.readInt8();
    maModel.mnLevel = rStrm.readInt8();
    rStrm >> maModel.maName;
    maModel.mbBuiltin = getFlag( nFlags, BIFF12_CELLSTYLE_BUILTIN );
    maModel.mbCustom = getFlag( nFlags, BIFF12_CELLSTYLE_CUSTOM );
    maModel.mbHidden = getFlag( nFlags, BIFF12_CELLSTYLE_HIDDEN );
}

void CellStyle::createCellStyle()
{

    // #i1624# #i1768# ignore unnamed user styles
    bool bDefStyle = maModel.isDefaultStyle();
    if( !mbCreated )
    {
        if ( bDefStyle && maFinalName.isEmpty() )
            maFinalName = ScResId( STR_STYLENAME_STANDARD );
        mbCreated = maFinalName.isEmpty();
    }

    if( !mbCreated && !mpStyleSheet )
    {
        bool bCreatePattern = false;
        Xf* pXF = getStyles().getStyleXf( maModel.mnXfId ).get();
        ::ScDocument& rDoc = getScDocument();

        if( bDefStyle )
        {
            // use existing "Default" style sheet
            mpStyleSheet = static_cast< ScStyleSheet* >( rDoc.GetStyleSheetPool()->Find(
                ScResId( STR_STYLENAME_STANDARD ), SfxStyleFamily::Para ) );
            OSL_ENSURE( mpStyleSheet, "CellStyle::createStyle - Default style not found" );
            bCreatePattern = true;
        }
        else
        {
            mpStyleSheet = static_cast< ScStyleSheet* >( rDoc.GetStyleSheetPool()->Find( maFinalName, SfxStyleFamily::Para ) );
            if( !mpStyleSheet )
            {
                mpStyleSheet = &static_cast< ScStyleSheet& >( rDoc.GetStyleSheetPool()->Make( maFinalName, SfxStyleFamily::Para, SfxStyleSearchBits::UserDefined ) );
                bCreatePattern = true;
            }
        }

        // bDefStyle==true omits default pool items in CreatePattern()
        if( bCreatePattern && mpStyleSheet && pXF )
            mpStyleSheet->GetItemSet().Put( pXF->createPattern( bDefStyle ).GetItemSet() );
    }
}

void CellStyle::finalizeImport( const OUString& rFinalName )
{
    maFinalName = rFinalName;
    if( !maModel.isBuiltin() || maModel.mbCustom )
        createCellStyle();
}

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

CellStyleRef CellStyleBuffer::importCellStyle( SequenceInputStream& rStrm )
{
    CellStyleRef xCellStyle( new CellStyle( *this ) );
    xCellStyle->importCellStyle( rStrm );
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
        overwrite these built-in styles. */
    try
    {
        // unfortunately, com.sun.star.style.StyleFamily does not implement XEnumerationAccess...
        Reference< XIndexAccess > xStyleFamilyIA( getCellStyleFamily(), UNO_QUERY_THROW );
        for( sal_Int32 nIndex = 0, nCount = xStyleFamilyIA->getCount(); nIndex < nCount; ++nIndex )
        {
            Reference< XStyle > xStyle( xStyleFamilyIA->getByIndex( nIndex ), UNO_QUERY_THROW );
            if( !xStyle->isUserDefined() )
            {
                // create an empty entry by using ::std::map<>::operator[]
                aCellStyles[ xStyle->getName() ];
            }
        }
    }
    catch( Exception& )
    {
    }

    /*  Calculate names of built-in styles. Store styles with reserved names
        in the aConflictNameStyles list. */
    for( const auto& rxStyle : maBuiltinStyles )
    {
        const CellStyleModel& rModel = rxStyle->getModel();
        if (rModel.isDefaultStyle())
            continue;

        OUString aStyleName = lclCreateStyleName( rModel );
        /*  If a builtin style entry already exists,
            we just stick with the last definition and ignore
            the preceding ones. */
        aCellStyles[ aStyleName ] = rxStyle;
    }

    /*  Calculate names of user defined styles. Store styles with reserved
        names in the aConflictNameStyles list. */
    for( const auto& rxStyle : maUserStyles )
    {
        const CellStyleModel& rModel = rxStyle->getModel();
        OUString aStyleName = lclCreateStyleName( rModel );
        // #i1624# #i1768# ignore unnamed user styles
        if( aStyleName.getLength() > 0 )
        {
            if( aCellStyles.find( aStyleName ) != aCellStyles.end() )
                aConflictNameStyles.push_back( rxStyle );
            else
                aCellStyles[ aStyleName ] = rxStyle;
        }
    }

    // find unused names for all styles with conflicting names
    // having the index counter outside the loop prevents performance problems with opening some pathological documents (tdf#62095)
    sal_Int32 nIndex = 0;
    for( const auto& rxStyle : aConflictNameStyles )
    {
        const CellStyleModel& rModel = rxStyle->getModel();
        OUString aStyleName = lclCreateStyleName( rModel );
        OUString aUnusedName;
        do
        {
            aUnusedName = aStyleName + OUStringLiteral1(' ') + OUString::number( ++nIndex );
        }
        while( aCellStyles.find( aUnusedName ) != aCellStyles.end() );
        aCellStyles[ aUnusedName ] = rxStyle;
    }

    // set final names and create user-defined and modified built-in cell styles
    aCellStyles.forEachMemWithKey( &CellStyle::finalizeImport );
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

::ScStyleSheet*   CellStyleBuffer::getCellStyleSheet( sal_Int32 nXfId ) const
{
    return getCellStyleSheet( maStylesByXf.get( nXfId ) );
}

// private --------------------------------------------------------------------

void CellStyleBuffer::insertCellStyle( CellStyleRef const & xCellStyle )
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

::ScStyleSheet* CellStyleBuffer::getCellStyleSheet( const CellStyleRef& rxCellStyle )
{
    ::ScStyleSheet* pStyleSheet = nullptr;
    if ( rxCellStyle.get() )
        pStyleSheet = rxCellStyle->getStyleSheet();
    return pStyleSheet;
}

OUString CellStyleBuffer::createCellStyle( const CellStyleRef& rxCellStyle )
{
    if( rxCellStyle.get() )
    {
        rxCellStyle->createCellStyle();
        const OUString& rStyleName = rxCellStyle->getFinalStyleName();
        if( !rStyleName.isEmpty() )
            return rStyleName;
    }
    // on error: fallback to default style
    return lclGetBuiltinStyleName( OOX_STYLE_NORMAL, OUString() );
}

AutoFormatModel::AutoFormatModel() :
    mnAutoFormatId( 0 ),
    mbApplyNumFmt( false ),
    mbApplyFont( false ),
    mbApplyAlignment( false ),
    mbApplyBorder( false ),
    mbApplyFill( false ),
    mbApplyProtection( false )
{
}

StylesBuffer::StylesBuffer( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    maPalette( rHelper ),
    maNumFmts( rHelper ),
    maCellStyles( rHelper )
{
}

FontRef StylesBuffer::createFont()
{
    FontRef xFont( new Font( *this, false ) );
    maFonts.push_back( xFont );
    return xFont;
}

NumberFormatRef StylesBuffer::createNumFmt( sal_Int32 nNumFmtId, const OUString& rFmtCode )
{
    return maNumFmts.createNumFmt( nNumFmtId, rFmtCode );
}

sal_Int32 StylesBuffer::nextFreeNumFmtId()
{
    return maNumFmts.nextFreeId();
}

BorderRef StylesBuffer::createBorder()
{
    BorderRef xBorder( new Border( *this, false ) );
    maBorders.push_back( xBorder );
    return xBorder;
}

FillRef StylesBuffer::createFill()
{
    FillRef xFill( new Fill( *this, false ) );
    maFills.push_back( xFill );
    return xFill;
}

XfRef StylesBuffer::createCellXf()
{
    XfRef xXf( new Xf( *this ) );
    maCellXfs.push_back( xXf );
    return xXf;
}

XfRef StylesBuffer::createStyleXf()
{
    XfRef xXf( new Xf( *this ) );
    maStyleXfs.push_back( xXf );
    return xXf;
}

DxfRef StylesBuffer::createDxf()
{
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

void StylesBuffer::importPaletteColor( SequenceInputStream& rStrm )
{
    maPalette.importPaletteColor( rStrm );
}

void StylesBuffer::importNumFmt( SequenceInputStream& rStrm )
{
    maNumFmts.importNumFmt( rStrm );
}

void StylesBuffer::importCellStyle( SequenceInputStream& rStrm )
{
    maCellStyles.importCellStyle( rStrm );
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
    // is there a document wide RTL setting that we
    // would/could need to pass to finalizeImport here ?
    maBorders.forEachMem( &Border::finalizeImport, false );
    maFills.forEachMem( &Fill::finalizeImport );
    // style XFs and cell XFs
    maStyleXfs.forEachMem( &Xf::finalizeImport );
    maCellXfs.forEachMem( &Xf::finalizeImport );
    // built-in and user defined cell styles
    maCellStyles.finalizeImport();
    // differential formatting (for conditional formatting)
    maDxfs.forEachMem( &Dxf::finalizeImport );
}

::Color StylesBuffer::getPaletteColor( sal_Int32 nPaletteIdx ) const
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

bool StylesBuffer::equalBorders( sal_Int32 nBorderId1, sal_Int32 nBorderId2 )
{
    // in OOXML, borders are assumed to be unique
    return nBorderId1 == nBorderId2;
}

bool StylesBuffer::equalFills( sal_Int32 nFillId1, sal_Int32 nFillId2 )
{
    // in OOXML, fills are assumed to be unique
    return nFillId1 == nFillId2;
}

OUString StylesBuffer::getDefaultStyleName() const
{
    return maCellStyles.getDefaultStyleName();
}

OUString StylesBuffer::createCellStyle( sal_Int32 nXfId ) const
{
    return maCellStyles.createCellStyle( nXfId );
}

::ScStyleSheet* StylesBuffer::getCellStyleSheet( sal_Int32 nXfId ) const
{
    return maCellStyles.getCellStyleSheet( nXfId );
}

OUString StylesBuffer::createDxfStyle( sal_Int32 nDxfId ) const
{
    OUString& rStyleName = maDxfStyles[ nDxfId ];
    if (!rStyleName.isEmpty())
        return rStyleName;

    if (Dxf* pDxf = maDxfs.get(nDxfId).get())
    {
        rStyleName = "ConditionalStyle_" + OUString::number(nDxfId + 1);

        // Create a cell style. This may overwrite an existing style if
        // one with the same name exists.
        ScStyleSheet& rStyleSheet = ScfTools::MakeCellStyleSheet(
                *getScDocument().GetStyleSheetPool(), rStyleName, true);

        rStyleSheet.ResetParent();
        SfxItemSet& rStyleItemSet =
            rStyleSheet.GetItemSet();

        pDxf->fillToItemSet(rStyleItemSet);
    }

    // on error: fallback to default style
    if (rStyleName.isEmpty())
        rStyleName = maCellStyles.getDefaultStyleName();

    return rStyleName;
}

void StylesBuffer::writeFontToItemSet( SfxItemSet& rItemSet, sal_Int32 nFontId, bool bSkipPoolDefs ) const
{
    if( Font* pFont = maFonts.get( nFontId ).get() )
        pFont->fillToItemSet( rItemSet, false, bSkipPoolDefs );
}

sal_uInt32 StylesBuffer::writeNumFmtToItemSet( SfxItemSet& rItemSet, sal_uInt32 nNumFmtId, bool bSkipPoolDefs ) const
{
    return maNumFmts.fillToItemSet( rItemSet, nNumFmtId, bSkipPoolDefs );
}

void StylesBuffer::writeBorderToItemSet( SfxItemSet& rItemSet, sal_Int32 nBorderId, bool bSkipPoolDefs ) const
{
    if( Border* pBorder = maBorders.get( nBorderId ).get() )
        pBorder->fillToItemSet( rItemSet, bSkipPoolDefs );
}

void StylesBuffer::writeFillToItemSet( SfxItemSet& rItemSet, sal_Int32 nFillId, bool bSkipPoolDefs ) const
{
    if( Fill* pFill = maFills.get( nFillId ).get() )
        pFill->fillToItemSet( rItemSet, bSkipPoolDefs );
}

bool operator==( const XfModel& rXfModel1,  const XfModel& rXfModel2 )
{
    return ( rXfModel1.mbCellXf == rXfModel2.mbCellXf &&
        rXfModel1.mnStyleXfId == rXfModel2.mnStyleXfId &&
        rXfModel1.mbFontUsed == rXfModel2.mbFontUsed &&
        rXfModel1.mnFontId == rXfModel2.mnFontId &&
        rXfModel1.mbNumFmtUsed == rXfModel2.mbNumFmtUsed &&
        rXfModel1.mnNumFmtId == rXfModel2.mnNumFmtId &&
        rXfModel1.mbAlignUsed == rXfModel2.mbAlignUsed &&
        rXfModel1.mbBorderUsed == rXfModel2.mbBorderUsed &&
        rXfModel1.mnBorderId == rXfModel2.mnBorderId &&
        rXfModel1.mbAreaUsed == rXfModel2.mbAreaUsed &&
        rXfModel1.mnFillId == rXfModel2.mnFillId &&
        rXfModel1.mbProtUsed == rXfModel2.mbProtUsed );
}

bool operator==( const Xf& rXf1, const Xf& rXf2 )
{
    if ( rXf1.maModel == rXf2.maModel )
    {
        if ( rXf1.maModel.mbAlignUsed )
        {
            if ( !( rXf1.maAlignment.getApiData() == rXf2.maAlignment.getApiData() ) )
                return false;
        }
        if ( rXf1.maModel.mbProtUsed )
        {
            if ( !( rXf1.maProtection.getApiData() == rXf2.maProtection.getApiData() ) )
                return false;
        }
        return true;
    }
    return false;
}

void StylesBuffer::writeCellXfToDoc(
    ScDocumentImport& rDoc, const ScRange& rRange, sal_Int32 nXfId ) const
{
    Xf* pXf = maCellXfs.get(nXfId).get();
    if (!pXf)
        return;

    pXf->writeToDoc(rDoc, rRange);
}

} // namespace xls
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
