/*************************************************************************
 *
 *  $RCSfile: xlstyle.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-30 16:21:25 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SC_XLSTYLE_HXX
#include "xlstyle.hxx"
#endif

#ifndef _COM_SUN_STAR_AWT_FONTFAMILY_HPP_
#include <com/sun/star/awt/FontFamily.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTSLANT_HPP_
#include <com/sun/star/awt/FontSlant.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTUNDERLINE_HPP_
#include <com/sun/star/awt/FontUnderline.hpp>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_FONT_HXX
#include <vcl/font.hxx>
#endif
#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif
#ifndef _SVX_SVXFONT_HXX
#include <svx/svxfont.hxx>
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif
#ifndef SC_XLROOT_HXX
#include "xlroot.hxx"
#endif

// Color data =================================================================

/** Built-in color table (color indexes 0-7). */
#define EXC_PALETTE_BUILTIN_COLORS \
            0x000000, 0xFFFFFF, 0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00, 0xFF00FF, 0x00FFFF

/** Default color table for BIFF2. */
static const ColorData pDefColorTable2[] =
{
/*  0 */    EXC_PALETTE_BUILTIN_COLORS
};

/** Default color table for BIFF3/BIFF4. */
static const ColorData pDefColorTable3[] =
{
/*  0 */    EXC_PALETTE_BUILTIN_COLORS,
/*  8 */    0x000000, 0xFFFFFF, 0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00, 0xFF00FF, 0x00FFFF,
/* 16 */    0x800000, 0x008000, 0x000080, 0x808000, 0x800080, 0x008080, 0xC0C0C0, 0x808080
};

/** Default color table for BIFF5/BIFF7. */
static const ColorData pDefColorTable5[] =
{
/*  0 */    EXC_PALETTE_BUILTIN_COLORS,
/*  8 */    0x000000, 0xFFFFFF, 0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00, 0xFF00FF, 0x00FFFF,
/* 16 */    0x800000, 0x008000, 0x000080, 0x808000, 0x800080, 0x008080, 0xC0C0C0, 0x808080,
/* 24 */    0x8080FF, 0x802060, 0xFFFFC0, 0xA0E0E0, 0x600080, 0xFF8080, 0x0080C0, 0xC0C0FF,
/* 32 */    0x000080, 0xFF00FF, 0xFFFF00, 0x00FFFF, 0x800080, 0x800000, 0x008080, 0x0000FF,
/* 40 */    0x00CFFF, 0x69FFFF, 0xE0FFE0, 0xFFFF80, 0xA6CAF0, 0xDD9CB3, 0xB38FEE, 0xE3E3E3,
/* 48 */    0x2A6FF9, 0x3FB8CD, 0x488436, 0x958C41, 0x8E5E42, 0xA0627A, 0x624FAC, 0x969696,
/* 56 */    0x1D2FBE, 0x286676, 0x004500, 0x453E01, 0x6A2813, 0x85396A, 0x4A3285, 0x424242
};

/** Default color table for BIFF8. */
static const ColorData pDefColorTable8[] =
{
/*  0 */    EXC_PALETTE_BUILTIN_COLORS,
/*  8 */    0x000000, 0xFFFFFF, 0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00, 0xFF00FF, 0x00FFFF,
/* 16 */    0x800000, 0x008000, 0x000080, 0x808000, 0x800080, 0x008080, 0xC0C0C0, 0x808080,
/* 24 */    0x9999FF, 0x993366, 0xFFFFCC, 0xCCFFFF, 0x660066, 0xFF8080, 0x0066CC, 0xCCCCFF,
/* 32 */    0x000080, 0xFF00FF, 0xFFFF00, 0x00FFFF, 0x800080, 0x800000, 0x008080, 0x0000FF,
/* 40 */    0x00CCFF, 0xCCFFFF, 0xCCFFCC, 0xFFFF99, 0x99CCFF, 0xFF99CC, 0xCC99FF, 0xFFCC99,
/* 48 */    0x3366FF, 0x33CCCC, 0x99CC00, 0xFFCC00, 0xFF9900, 0xFF6600, 0x666699, 0x969696,
/* 56 */    0x003366, 0x339966, 0x003300, 0x333300, 0x993300, 0x993366, 0x333399, 0x333333
};

#undef EXC_PALETTE_BUILTIN_COLORS

// ----------------------------------------------------------------------------

XclDefaultPalette::XclDefaultPalette( const XclRoot& rRoot ) :
    mpColorTable( 0 ),
    mnTableSize( 0 )
{
    const StyleSettings& rSett = Application::GetSettings().GetStyleSettings();
    mnWindowText = rSett.GetWindowTextColor().GetColor();
    mnWindowBack = rSett.GetWindowColor().GetColor();
    mnFaceColor = rSett.GetFaceColor().GetColor();
    mnNoteText = rSett.GetHelpTextColor().GetColor();
    mnNoteBack = rSett.GetHelpColor().GetColor();

    // default colors
    switch( rRoot.GetBiff() )
    {
        case xlBiff2:
            mpColorTable = pDefColorTable2;
            mnTableSize = STATIC_TABLE_SIZE( pDefColorTable2 );
        break;
        case xlBiff3:
        case xlBiff4:
            mpColorTable = pDefColorTable3;
            mnTableSize = STATIC_TABLE_SIZE( pDefColorTable3 );
        break;
        case xlBiff5:
        case xlBiff7:
            mpColorTable = pDefColorTable5;
            mnTableSize = STATIC_TABLE_SIZE( pDefColorTable5 );
        break;
        case xlBiff8:
            mpColorTable = pDefColorTable8;
            mnTableSize = STATIC_TABLE_SIZE( pDefColorTable8 );
        break;
        default:
            DBG_ERROR_BIFF();
    }
}

ColorData XclDefaultPalette::GetDefColorData( sal_uInt16 nXclIndex ) const
{
    ColorData nColor;
    if( nXclIndex < mnTableSize )
        nColor = mpColorTable[ nXclIndex ];
    else switch( nXclIndex )
    {
        case EXC_COLOR_WINDOWTEXT3:
        case EXC_COLOR_WINDOWTEXT:
        case EXC_COLOR_WINDOWTEXT_CH:   nColor = mnWindowText;  break;
        case EXC_COLOR_WINDOWBACK3:
        case EXC_COLOR_WINDOWBACK:
        case EXC_COLOR_WINDOWBACK_CH:   nColor = mnWindowBack;  break;
        case EXC_COLOR_BUTTONBACK:      nColor = mnFaceColor;   break;
        case EXC_COLOR_BORDERAUTO_CH:   nColor = COL_BLACK;     break;  // TODO: really always black?
        case EXC_COLOR_NOTEBACK:        nColor = mnNoteBack;    break;
        case EXC_COLOR_NOTETEXT:        nColor = mnNoteText;    break;
        case EXC_COLOR_FONTAUTO:        nColor = COL_AUTO;      break;
        default:
            DBG_ERROR1( "XclDefaultPalette::GetDefColorData - unknown default color index: %d", nXclIndex );
            nColor = COL_AUTO;
    }
    return nColor;
}

// Font Data ==================================================================

XclFontData::XclFontData()
{
    Clear();
}

XclFontData::XclFontData( const Font& rFont )
{
    Clear();
    FillFromFont( rFont );
}

XclFontData::XclFontData( const SvxFont& rFont )
{
    FillFromSvxFont( rFont );
}

void XclFontData::Clear()
{
    maName.Erase();
    maStyle.Erase();
    mnHeight = 0;
    mnColor = EXC_COLOR_FONTAUTO;
    mnWeight = EXC_FONTWGHT_DONTKNOW;
    mnEscapem = EXC_FONTESC_NONE;
    mnFamily = EXC_FONTFAM_SYSTEM;
    mnCharSet = EXC_FONTCSET_ANSI_LATIN;
    mnUnderline = EXC_FONTUNDERL_NONE;
    mbItalic = mbStrikeout = mbOutline = mbShadow = false;
}

void XclFontData::FillFromFont( const Font& rFont )
{
    maName = XclTools::GetXclFontName( rFont.GetName() );   // #106246# substitute with MS fonts
    maStyle.Erase();
    SetScUnderline( rFont.GetUnderline() );
    mnEscapem = EXC_FONTESC_NONE;
    SetScHeight( rFont.GetSize().Height() );
    mnColor = EXC_COLOR_FONTAUTO;
    SetScWeight( rFont.GetWeight() );
    SetScFamily( rFont.GetFamily() );
    SetScCharSet( rFont.GetCharSet() );
    SetScPosture( rFont.GetItalic() );
    SetScStrikeout( rFont.GetStrikeout() );
    mbOutline = rFont.IsOutline();
    mbShadow = rFont.IsShadow();
}

void XclFontData::FillFromSvxFont( const SvxFont& rFont )
{
    FillFromFont( rFont );
    SetScEscapement( rFont.GetEscapement() );
}

// *** conversion of VCL/SVX constants *** ------------------------------------

FontFamily XclFontData::GetScFamily( CharSet eDefCharSet ) const
{
    FontFamily eScFamily;
    // ! format differs from Windows documentation: family is in lower nibble, pitch unknown
    switch( mnFamily & 0x0F )
    {
        case EXC_FONTFAM_ROMAN:         eScFamily = FAMILY_ROMAN;       break;
        case EXC_FONTFAM_SWISS:         eScFamily = FAMILY_SWISS;       break;
        case EXC_FONTFAM_MODERN:        eScFamily = FAMILY_MODERN;      break;
        case EXC_FONTFAM_SCRIPT:        eScFamily = FAMILY_SCRIPT;      break;
        case EXC_FONTFAM_DECORATIVE:    eScFamily = FAMILY_DECORATIVE;  break;
        default:
            eScFamily =
                ((eDefCharSet == RTL_TEXTENCODING_APPLE_ROMAN) &&
                (maName.EqualsIgnoreCaseAscii( "Geneva" ) || maName.EqualsIgnoreCaseAscii( "Chicago" ))) ?
                FAMILY_SWISS : FAMILY_DONTKNOW;
    }
    return eScFamily;
}

CharSet XclFontData::GetScCharSet() const
{
    return rtl_getTextEncodingFromWindowsCharset( mnCharSet );
}

FontItalic XclFontData::GetScPosture() const
{
    return mbItalic ? ITALIC_NORMAL : ITALIC_NONE;
}

FontWeight XclFontData::GetScWeight() const
{
    FontWeight eScWeight;

    if( !mnWeight )             eScWeight = WEIGHT_DONTKNOW;
    else if( mnWeight < 150 )   eScWeight = WEIGHT_THIN;
    else if( mnWeight < 250 )   eScWeight = WEIGHT_ULTRALIGHT;
    else if( mnWeight < 325 )   eScWeight = WEIGHT_LIGHT;
    else if( mnWeight < 375 )   eScWeight = WEIGHT_SEMILIGHT;
    else if( mnWeight < 450 )   eScWeight = WEIGHT_NORMAL;
    else if( mnWeight < 550 )   eScWeight = WEIGHT_MEDIUM;
    else if( mnWeight < 650 )   eScWeight = WEIGHT_SEMIBOLD;
    else if( mnWeight < 750 )   eScWeight = WEIGHT_BOLD;
    else if( mnWeight < 850 )   eScWeight = WEIGHT_ULTRABOLD;
    else                        eScWeight = WEIGHT_BLACK;

    return eScWeight;
}

FontUnderline XclFontData::GetScUnderline() const
{
    FontUnderline eScUnderl = UNDERLINE_NONE;
    switch( mnUnderline )
    {
        case EXC_FONTUNDERL_SINGLE:
        case EXC_FONTUNDERL_SINGLE_ACC: eScUnderl = UNDERLINE_SINGLE;  break;
        case EXC_FONTUNDERL_DOUBLE:
        case EXC_FONTUNDERL_DOUBLE_ACC: eScUnderl = UNDERLINE_DOUBLE;  break;
    }
    return eScUnderl;
}

SvxEscapement XclFontData::GetScEscapement() const
{
    SvxEscapement eScEscapem = SVX_ESCAPEMENT_OFF;
    switch( mnEscapem )
    {
        case EXC_FONTESC_SUPER: eScEscapem = SVX_ESCAPEMENT_SUPERSCRIPT;    break;
        case EXC_FONTESC_SUB:   eScEscapem = SVX_ESCAPEMENT_SUBSCRIPT;      break;
    }
    return eScEscapem;
}

FontStrikeout XclFontData::GetScStrikeout() const
{
    return mbStrikeout ? STRIKEOUT_SINGLE : STRIKEOUT_NONE;
}

void XclFontData::SetScHeight( sal_Int32 nTwips )
{
    mnHeight = static_cast< sal_uInt16 >( ::std::min( nTwips, 0x7FFFL ) );
}

void XclFontData::SetScFamily( FontFamily eScFamily )
{
    switch( eScFamily )
    {
        case FAMILY_DONTKNOW:   mnFamily = EXC_FONTFAM_DONTKNOW;    break;
        case FAMILY_DECORATIVE: mnFamily = EXC_FONTFAM_DECORATIVE;  break;
        case FAMILY_MODERN:     mnFamily = EXC_FONTFAM_MODERN;      break;
        case FAMILY_ROMAN:      mnFamily = EXC_FONTFAM_ROMAN;       break;
        case FAMILY_SCRIPT:     mnFamily = EXC_FONTFAM_SCRIPT;      break;
        case FAMILY_SWISS:      mnFamily = EXC_FONTFAM_SWISS;       break;
        case FAMILY_SYSTEM:     mnFamily = EXC_FONTFAM_SYSTEM;      break;
        default:
            DBG_ERRORFILE( "XclFontData::SetScFamily - unknown font family" );
            mnFamily = EXC_FONTFAM_DONTKNOW;
    }
}

void XclFontData::SetScCharSet( CharSet eScCharSet )
{
    mnCharSet = rtl_getBestWindowsCharsetFromTextEncoding( eScCharSet );
}


void XclFontData::SetScPosture( FontItalic eScPosture )
{
    mbItalic = (eScPosture == ITALIC_OBLIQUE) || (eScPosture == ITALIC_NORMAL);
}

void XclFontData::SetScWeight( FontWeight eScWeight )
{
    switch( eScWeight )
    {
        case WEIGHT_DONTKNOW:   mnWeight = EXC_FONTWGHT_DONTKNOW;   break;
        case WEIGHT_THIN:       mnWeight = EXC_FONTWGHT_THIN;       break;
        case WEIGHT_ULTRALIGHT: mnWeight = EXC_FONTWGHT_ULTRALIGHT; break;
        case WEIGHT_LIGHT:      mnWeight = EXC_FONTWGHT_LIGHT;      break;
        case WEIGHT_SEMILIGHT:  mnWeight = EXC_FONTWGHT_SEMILIGHT;  break;
        case WEIGHT_NORMAL:     mnWeight = EXC_FONTWGHT_NORMAL;     break;
        case WEIGHT_MEDIUM:     mnWeight = EXC_FONTWGHT_MEDIUM;     break;
        case WEIGHT_SEMIBOLD:   mnWeight = EXC_FONTWGHT_SEMIBOLD;   break;
        case WEIGHT_BOLD:       mnWeight = EXC_FONTWGHT_BOLD;       break;
        case WEIGHT_ULTRABOLD:  mnWeight = EXC_FONTWGHT_ULTRABOLD;  break;
        case WEIGHT_BLACK:      mnWeight = EXC_FONTWGHT_BLACK;      break;
        default:                mnWeight = EXC_FONTWGHT_NORMAL;
    }
}

void XclFontData::SetScUnderline( FontUnderline eScUnderl )
{
    switch( eScUnderl )
    {
        case UNDERLINE_NONE:
        case UNDERLINE_DONTKNOW:    mnUnderline = EXC_FONTUNDERL_NONE;      break;
        case UNDERLINE_DOUBLE:
        case UNDERLINE_DOUBLEWAVE:  mnUnderline = EXC_FONTUNDERL_DOUBLE;    break;
        default:                    mnUnderline = EXC_FONTUNDERL_SINGLE;
    }
}

void XclFontData::SetScEscapement( short nScEscapem )
{
    if( nScEscapem > 0 )
        mnEscapem = EXC_FONTESC_SUPER;
    else if( nScEscapem < 0 )
        mnEscapem = EXC_FONTESC_SUB;
    else
        mnEscapem = EXC_FONTESC_NONE;
}

void XclFontData::SetScStrikeout( FontStrikeout eScStrikeout )
{
    mbStrikeout =
        (eScStrikeout == STRIKEOUT_SINGLE) || (eScStrikeout == STRIKEOUT_DOUBLE) ||
        (eScStrikeout == STRIKEOUT_BOLD)   || (eScStrikeout == STRIKEOUT_SLASH)  ||
        (eScStrikeout == STRIKEOUT_X);
}

// *** conversion of API constants *** ----------------------------------------

namespace ApiAwt = ::com::sun::star::awt;

float XclFontData::GetApiHeight() const
{
    return static_cast< float >( mnHeight / TWIPS_PER_POINT );
}

sal_Int16 XclFontData::GetApiFamily() const
{
    sal_Int16 nApiFamily = ApiAwt::FontFamily::DONTKNOW;
    switch( mnFamily )
    {
        case FAMILY_DECORATIVE: nApiFamily = ApiAwt::FontFamily::DECORATIVE;    break;
        case FAMILY_MODERN:     nApiFamily = ApiAwt::FontFamily::MODERN;        break;
        case FAMILY_ROMAN:      nApiFamily = ApiAwt::FontFamily::ROMAN;         break;
        case FAMILY_SCRIPT:     nApiFamily = ApiAwt::FontFamily::SCRIPT;        break;
        case FAMILY_SWISS:      nApiFamily = ApiAwt::FontFamily::SWISS;         break;
        case FAMILY_SYSTEM:     nApiFamily = ApiAwt::FontFamily::SYSTEM;        break;
    }
    return nApiFamily;
}

sal_Int16 XclFontData::GetApiCharSet() const
{
    return static_cast< sal_Int16 >( GetScCharSet() );
}

ApiAwt::FontSlant XclFontData::GetApiPosture() const
{
    return mbItalic ? ApiAwt::FontSlant_ITALIC : ApiAwt::FontSlant_NONE;
}

float XclFontData::GetApiWeight() const
{
    return VCLUnoHelper::ConvertFontWeight( GetScWeight() );
}

sal_Int16 XclFontData::GetApiUnderline() const
{
    sal_Int16 nApiUnderl = ApiAwt::FontUnderline::NONE;
    switch( mnUnderline )
    {
        case EXC_FONTUNDERL_SINGLE:
        case EXC_FONTUNDERL_SINGLE_ACC: nApiUnderl = ApiAwt::FontUnderline::SINGLE; break;
        case EXC_FONTUNDERL_DOUBLE:
        case EXC_FONTUNDERL_DOUBLE_ACC: nApiUnderl = ApiAwt::FontUnderline::DOUBLE; break;
    }
    return nApiUnderl;
}

sal_Int16 XclFontData::GetApiStrikeout() const
{
    return mbStrikeout ? ApiAwt::FontStrikeout::SINGLE : ApiAwt::FontStrikeout::NONE;
}

void XclFontData::SetApiHeight( float fPoint )
{
    mnHeight = static_cast< sal_uInt16 >( ::std::min( fPoint * TWIPS_PER_POINT + 0.5, 32767.0 ) );
}

void XclFontData::SetApiFamily( sal_Int16 nApiFamily )
{
    switch( nApiFamily )
    {
        case ApiAwt::FontFamily::DECORATIVE:    mnFamily = FAMILY_DECORATIVE;   break;
        case ApiAwt::FontFamily::MODERN:        mnFamily = FAMILY_MODERN;       break;
        case ApiAwt::FontFamily::ROMAN:         mnFamily = FAMILY_ROMAN;        break;
        case ApiAwt::FontFamily::SCRIPT:        mnFamily = FAMILY_SCRIPT;       break;
        case ApiAwt::FontFamily::SWISS:         mnFamily = FAMILY_SWISS;        break;
        case ApiAwt::FontFamily::SYSTEM:        mnFamily = FAMILY_SYSTEM;       break;
        default:                                mnFamily = FAMILY_DONTKNOW;
    }
}

void XclFontData::SetApiCharSet( sal_Int16 nApiCharSet )
{
    SetScCharSet( static_cast< CharSet >( nApiCharSet ) );
}

void XclFontData::SetApiPosture( ApiAwt::FontSlant eApiPosture )
{
    mbItalic =
        (eApiPosture == ApiAwt::FontSlant_OBLIQUE) ||
        (eApiPosture == ApiAwt::FontSlant_ITALIC) ||
        (eApiPosture == ApiAwt::FontSlant_REVERSE_OBLIQUE) ||
        (eApiPosture == ApiAwt::FontSlant_REVERSE_ITALIC);
}

void XclFontData::SetApiWeight( float fApiWeight )
{
    SetScWeight( VCLUnoHelper::ConvertFontWeight( fApiWeight ) );
}

void XclFontData::SetApiUnderline( sal_Int16 nApiUnderl )
{
    switch( nApiUnderl )
    {
        case ApiAwt::FontUnderline::NONE:
        case ApiAwt::FontUnderline::DONTKNOW:   mnUnderline = EXC_FONTUNDERL_NONE;      break;
        case ApiAwt::FontUnderline::DOUBLE:
        case ApiAwt::FontUnderline::DOUBLEWAVE: mnUnderline = EXC_FONTUNDERL_DOUBLE;    break;
        default:                                mnUnderline = EXC_FONTUNDERL_SINGLE;
    }
}

void XclFontData::SetApiStrikeout( sal_Int16 nApiStrikeout )
{
    mbStrikeout =
        (nApiStrikeout != ApiAwt::FontStrikeout::NONE) &&
        (nApiStrikeout != ApiAwt::FontStrikeout::DONTKNOW);
}

// ----------------------------------------------------------------------------

bool operator==( const XclFontData& rLeft, const XclFontData& rRight )
{
    return
        (rLeft.mnHeight    == rRight.mnHeight)    &&
        (rLeft.mnWeight    == rRight.mnWeight)    &&
        (rLeft.mnColor     == rRight.mnColor)     &&
        (rLeft.mnUnderline == rRight.mnUnderline) &&
        (rLeft.mnEscapem   == rRight.mnEscapem)   &&
        (rLeft.mnFamily    == rRight.mnFamily)    &&
        (rLeft.mnCharSet   == rRight.mnCharSet)   &&
        (rLeft.mbItalic    == rRight.mbItalic)    &&
        (rLeft.mbStrikeout == rRight.mbStrikeout) &&
        (rLeft.mbOutline   == rRight.mbOutline)   &&
        (rLeft.mbShadow    == rRight.mbShadow)    &&
        (rLeft.maName      == rRight.maName);
}

// Number formats =============================================================

namespace {

// ----------------------------------------------------------------------------

const sal_uInt16 EXC_BUILTIN_NOFORMAT = 0xFFFF;

/** Special number format index describing a reused format. */
const NfIndexTableOffset PRV_NF_INDEX_REUSE = NF_INDEX_TABLE_ENTRIES;

/** German primary language not defined, LANGUAGE_GERMAN belongs to Germany. */
const LanguageType PRV_LANGUAGE_GERMAN_PRIM = LANGUAGE_GERMAN & 0x03FF;
/** French primary language not defined, LANGUAGE_FRENCH belongs to France. */
const LanguageType PRV_LANGUAGE_FRENCH_PRIM = LANGUAGE_FRENCH & 0x03FF;

// ----------------------------------------------------------------------------

/** Stores the number format used in Calc for an Excel built-in number format. */
struct XclBuiltInFormat
{
    sal_uInt16          mnXclNumFmt;    /// Excel built-in index.
    const sal_Char*     mpFormat;       /// Format string, may be 0 (meOffset used then).
    NfIndexTableOffset  meOffset;       /// SvNumberFormatter format index, if mpFormat==0.
    sal_uInt16          mnXclReuseFmt;  /// Use this Excel format, if meOffset==PRV_NF_INDEX_REUSE.
};

// ----------------------------------------------------------------------------

/** Defines a literal Excel built-in number format. */
#define EXC_NUMFMT_STRING( nXclNumFmt, pcUtf8 ) \
    { nXclNumFmt, pcUtf8, NF_NUMBER_STANDARD }

/** Defines an Excel built-in number format that maps to an own built-in format. */
#define EXC_NUMFMT_OFFSET( nXclNumFmt, eOffset ) \
    { nXclNumFmt, 0, eOffset }

/** Defines an Excel built-in number format that is the same as the specified. */
#define EXC_NUMFMT_REUSE( nXclNumFmt, nXclReuse ) \
    { nXclNumFmt, 0, PRV_NF_INDEX_REUSE, nXclReuse }

/** Terminates an Excel built-in number format table. */
#define EXC_NUMFMT_ENDTABLE() \
    { EXC_BUILTIN_NOFORMAT }

// ----------------------------------------------------------------------------

// Special UTF-8 characters
#define UTF8_EURO       "\342\202\254"

// Japanese/Chinese date/time characters
#define UTF8_YEAR       "\345\271\264"
#define UTF8_MON        "\346\234\210"
#define UTF8_DAY        "\346\227\245"
#define UTF8_HOUR       "\346\231\202"
#define UTF8_MIN        "\345\210\206"
#define UTF8_SEC        "\347\247\222"

// Chinese Simplified date/time characters
#define UTF8_CS_HOUR    "\346\227\266"

// Korean date/time characters
#define UTF8_KO_YEAR    "\353\205\204"
#define UTF8_KO_MON     "\354\233\224"
#define UTF8_KO_DAY     "\354\235\274"
#define UTF8_KO_HOUR    "\354\213\234"
#define UTF8_KO_MIN     "\353\266\204"
#define UTF8_KO_SEC     "\354\264\210"

// ----------------------------------------------------------------------------

/** Default number format table. Last parent of all other tables, used for unknown languages. */
static const XclBuiltInFormat spBuiltInFormats_DONTKNOW[] =
{
    EXC_NUMFMT_OFFSET(   0, NF_NUMBER_STANDARD ),       // General
    EXC_NUMFMT_OFFSET(   1, NF_NUMBER_INT ),            // 0
    EXC_NUMFMT_OFFSET(   2, NF_NUMBER_DEC2 ),           // 0.00
    EXC_NUMFMT_OFFSET(   3, NF_NUMBER_1000INT ),        // #,##0
    EXC_NUMFMT_OFFSET(   4, NF_NUMBER_1000DEC2 ),       // #,##0.00
    // 5...8 contained in file
    EXC_NUMFMT_OFFSET(   9, NF_PERCENT_INT ),           // 0%
    EXC_NUMFMT_OFFSET(  10, NF_PERCENT_DEC2 ),          // 0.00%
    EXC_NUMFMT_OFFSET(  11, NF_SCIENTIFIC_000E00 ),     // 0.00E+00
    EXC_NUMFMT_OFFSET(  12, NF_FRACTION_1 ),            // # ?/?
    EXC_NUMFMT_OFFSET(  13, NF_FRACTION_2 ),            // # ??/??

    // 14...22 date and time formats
    EXC_NUMFMT_OFFSET(  14, NF_DATE_SYS_DDMMYYYY ),
    EXC_NUMFMT_OFFSET(  15, NF_DATE_SYS_DMMMYY ),
    EXC_NUMFMT_OFFSET(  16, NF_DATE_SYS_DDMMM ),
    EXC_NUMFMT_OFFSET(  17, NF_DATE_SYS_MMYY ),
    EXC_NUMFMT_OFFSET(  18, NF_TIME_HHMMAMPM ),
    EXC_NUMFMT_OFFSET(  19, NF_TIME_HHMMSSAMPM ),
    EXC_NUMFMT_OFFSET(  20, NF_TIME_HHMM ),
    EXC_NUMFMT_OFFSET(  21, NF_TIME_HHMMSS ),
    EXC_NUMFMT_OFFSET(  22, NF_DATETIME_SYSTEM_SHORT_HHMM ),

    // 23...36 international formats
    EXC_NUMFMT_REUSE(   23, 0 ),
    EXC_NUMFMT_REUSE(   24, 0 ),
    EXC_NUMFMT_REUSE(   25, 0 ),
    EXC_NUMFMT_REUSE(   26, 0 ),
    EXC_NUMFMT_REUSE(   27, 14 ),
    EXC_NUMFMT_REUSE(   28, 14 ),
    EXC_NUMFMT_REUSE(   29, 14 ),
    EXC_NUMFMT_REUSE(   30, 14 ),
    EXC_NUMFMT_REUSE(   31, 14 ),
    EXC_NUMFMT_REUSE(   32, 21 ),
    EXC_NUMFMT_REUSE(   33, 21 ),
    EXC_NUMFMT_REUSE(   34, 21 ),
    EXC_NUMFMT_REUSE(   35, 21 ),
    EXC_NUMFMT_REUSE(   36, 14 ),

    // 37...44 accounting formats
    // 41...44 contained in file
    EXC_NUMFMT_STRING(  37, "#,##0;-#,##0" ),
    EXC_NUMFMT_STRING(  38, "#,##0;[RED]-#,##0" ),
    EXC_NUMFMT_STRING(  39, "#,##0.00;-#,##0.00" ),
    EXC_NUMFMT_STRING(  40, "#,##0.00;[RED]-#,##0.00" ),

    // 45...49 more special formats
    EXC_NUMFMT_STRING(  45, "mm:ss" ),
    EXC_NUMFMT_STRING(  46, "[h]:mm:ss" ),
    EXC_NUMFMT_STRING(  47, "mm:ss.0" ),
    EXC_NUMFMT_STRING(  48, "##0.0E+0" ),
    EXC_NUMFMT_OFFSET(  49, NF_TEXT ),

    // 50...58 international formats
    EXC_NUMFMT_REUSE(   50, 14 ),
    EXC_NUMFMT_REUSE(   51, 14 ),
    EXC_NUMFMT_REUSE(   52, 14 ),
    EXC_NUMFMT_REUSE(   53, 14 ),
    EXC_NUMFMT_REUSE(   54, 14 ),
    EXC_NUMFMT_REUSE(   55, 14 ),
    EXC_NUMFMT_REUSE(   56, 14 ),
    EXC_NUMFMT_REUSE(   57, 14 ),
    EXC_NUMFMT_REUSE(   58, 14 ),

    // 59...?? repeating formats
    EXC_NUMFMT_REUSE(   59, 1 ),
    EXC_NUMFMT_REUSE(   60, 2 ),
    EXC_NUMFMT_REUSE(   61, 3 ),
    EXC_NUMFMT_REUSE(   62, 4 ),

    EXC_NUMFMT_ENDTABLE()
};

// ENGLISH --------------------------------------------------------------------

/** Base table for English locales. */
static const XclBuiltInFormat spBuiltInFormats_ENGLISH[] =
{
    EXC_NUMFMT_STRING(  15, "DD-MMM-YY" ),
    EXC_NUMFMT_STRING(  16, "DD-MMM" ),
    EXC_NUMFMT_STRING(  17, "MMM-YY" ),
    EXC_NUMFMT_STRING(  18, "h:mm AM/PM" ),
    EXC_NUMFMT_STRING(  19, "h:mm:ss AM/PM" ),
    EXC_NUMFMT_STRING(  22, "DD/MM/YYYY hh:mm" ),
    EXC_NUMFMT_ENDTABLE()
};

static const XclBuiltInFormat spBuiltInFormats_ENGLISH_US[] =
{
    EXC_NUMFMT_STRING(  14, "M/D/YYYY" ),
    EXC_NUMFMT_STRING(  15, "D-MMM-YY" ),
    EXC_NUMFMT_STRING(  16, "D-MMM" ),
    EXC_NUMFMT_STRING(  20, "h:mm" ),
    EXC_NUMFMT_STRING(  21, "h:mm:ss" ),
    EXC_NUMFMT_STRING(  22, "M/D/YYYY h:mm" ),
    EXC_NUMFMT_STRING(  37, "#,##0_);(#,##0)" ),
    EXC_NUMFMT_STRING(  38, "#,##0_);[RED](#,##0)" ),
    EXC_NUMFMT_STRING(  39, "#,##0.00_);(#,##0.00)" ),
    EXC_NUMFMT_STRING(  40, "#,##0.00_);[RED](#,##0.00)" ),
    EXC_NUMFMT_ENDTABLE()
};

static const XclBuiltInFormat spBuiltInFormats_ENGLISH_CAN[] =
{
    EXC_NUMFMT_STRING(  20, "h:mm" ),
    EXC_NUMFMT_STRING(  21, "h:mm:ss" ),
    EXC_NUMFMT_STRING(  22, "DD/MM/YYYY h:mm" ),
    EXC_NUMFMT_ENDTABLE()
};

static const XclBuiltInFormat spBuiltInFormats_ENGLISH_AUS[] =
{
    EXC_NUMFMT_STRING(  14, "D/MM/YYYY" ),
    EXC_NUMFMT_STRING(  15, "D-MMM-YY" ),
    EXC_NUMFMT_STRING(  16, "D-MMM" ),
    EXC_NUMFMT_STRING(  20, "h:mm" ),
    EXC_NUMFMT_STRING(  21, "h:mm:ss" ),
    EXC_NUMFMT_STRING(  22, "D/MM/YYYY h:mm" ),
    EXC_NUMFMT_ENDTABLE()
};

static const XclBuiltInFormat spBuiltInFormats_ENGLISH_SAFRICA[] =
{
    EXC_NUMFMT_STRING(  14, "YYYY/MM/DD" ),
    EXC_NUMFMT_OFFSET(  18, NF_TIME_HHMMAMPM ),
    EXC_NUMFMT_OFFSET(  19, NF_TIME_HHMMSSAMPM ),
    EXC_NUMFMT_STRING(  22, "YYYY/MM/DD hh:mm" ),
    EXC_NUMFMT_ENDTABLE()
};

// FRENCH ---------------------------------------------------------------------

/** Base table for French locales. */
static const XclBuiltInFormat spBuiltInFormats_FRENCH[] =
{
    EXC_NUMFMT_STRING(  15, "DD-MMM-YY" ),
    EXC_NUMFMT_STRING(  16, "DD-MMM" ),
    EXC_NUMFMT_STRING(  17, "MMM-YY" ),
    EXC_NUMFMT_STRING(  18, "h:mm AM/PM" ),
    EXC_NUMFMT_STRING(  19, "h:mm:ss AM/PM" ),
    EXC_NUMFMT_ENDTABLE()
};

static const XclBuiltInFormat spBuiltInFormats_FRENCH_FRANCE[] =
{
    EXC_NUMFMT_STRING(  22, "DD/MM/YYYY hh:mm" ),
    EXC_NUMFMT_STRING(  37, "#,##0\\ _" UTF8_EURO ";-#,##0\\ _" UTF8_EURO ),
    EXC_NUMFMT_STRING(  38, "#,##0\\ _" UTF8_EURO ";[RED]-#,##0\\ _" UTF8_EURO ),
    EXC_NUMFMT_STRING(  39, "#,##0.00\\ _" UTF8_EURO ";-#,##0.00\\ _" UTF8_EURO ),
    EXC_NUMFMT_STRING(  40, "#,##0.00\\ _" UTF8_EURO ";[RED]-#,##0.00\\ _" UTF8_EURO ),
    EXC_NUMFMT_ENDTABLE()
};

static const XclBuiltInFormat spBuiltInFormats_FRENCH_CANADIAN[] =
{
    EXC_NUMFMT_STRING(  22, "YYYY-MM-DD hh:mm" ),
    EXC_NUMFMT_STRING(  37, "#,##0\\ _$_-;#,##0\\ _$-" ),
    EXC_NUMFMT_STRING(  38, "#,##0\\ _$_-;[RED]#,##0\\ _$-" ),
    EXC_NUMFMT_STRING(  39, "#,##0.00\\ _$_-;#,##0.00\\ _$-" ),
    EXC_NUMFMT_STRING(  40, "#,##0.00\\ _$_-;[RED]#,##0.00\\ _$-" ),
    EXC_NUMFMT_ENDTABLE()
};

static const XclBuiltInFormat spBuiltInFormats_FRENCH_SWISS[] =
{
    EXC_NUMFMT_STRING(  15, "DD.MMM.YY" ),
    EXC_NUMFMT_STRING(  16, "DD.MMM" ),
    EXC_NUMFMT_STRING(  17, "MMM.YY" ),
    EXC_NUMFMT_STRING(  22, "DD.MM.YYYY hh:mm" ),
    EXC_NUMFMT_ENDTABLE()
};

static const XclBuiltInFormat spBuiltInFormats_FRENCH_BELGIAN[] =
{
    EXC_NUMFMT_STRING(  14, "D/MM/YYYY" ),
    EXC_NUMFMT_STRING(  15, "D-MMM-YY" ),
    EXC_NUMFMT_STRING(  16, "D-MMM" ),
    EXC_NUMFMT_STRING(  20, "h:mm" ),
    EXC_NUMFMT_STRING(  21, "h:mm:ss" ),
    EXC_NUMFMT_STRING(  22, "D/MM/YYYY h:mm" ),
    EXC_NUMFMT_ENDTABLE()
};

// GERMAN ---------------------------------------------------------------------

/** Base table for German locales. */
static const XclBuiltInFormat spBuiltInFormats_GERMAN[] =
{
    EXC_NUMFMT_STRING(  15, "DD. MMM YY" ),
    EXC_NUMFMT_STRING(  16, "DD. MMM" ),
    EXC_NUMFMT_STRING(  17, "MMM YY" ),
    EXC_NUMFMT_STRING(  18, "h:mm AM/PM" ),
    EXC_NUMFMT_STRING(  19, "h:mm:ss AM/PM" ),
    EXC_NUMFMT_STRING(  22, "DD.MM.YYYY hh:mm" ),
    EXC_NUMFMT_ENDTABLE()
};

static const XclBuiltInFormat spBuiltInFormats_GERMAN_GERMANY[] =
{
    EXC_NUMFMT_STRING(  37, "#,##0 _" UTF8_EURO ";-#,##0 _" UTF8_EURO ),
    EXC_NUMFMT_STRING(  38, "#,##0 _" UTF8_EURO ";[RED]-#,##0 _" UTF8_EURO ),
    EXC_NUMFMT_STRING(  39, "#,##0.00 _" UTF8_EURO ";-#,##0.00 _" UTF8_EURO ),
    EXC_NUMFMT_STRING(  40, "#,##0.00 _" UTF8_EURO ";[RED]-#,##0.00 _" UTF8_EURO ),
    EXC_NUMFMT_ENDTABLE()
};

static const XclBuiltInFormat spBuiltInFormats_GERMAN_AUSTRIAN[] =
{
    EXC_NUMFMT_STRING(  15, "DD.MMM.YY" ),
    EXC_NUMFMT_STRING(  16, "DD.MMM" ),
    EXC_NUMFMT_STRING(  17, "MMM.YY" ),
    EXC_NUMFMT_ENDTABLE()
};

static const XclBuiltInFormat spBuiltInFormats_GERMAN_LUXEMBOURG[] =
{
    EXC_NUMFMT_STRING(  15, "DD.MMM.YY" ),
    EXC_NUMFMT_STRING(  16, "DD.MMM" ),
    EXC_NUMFMT_STRING(  17, "MMM.YY" ),
    EXC_NUMFMT_STRING(  37, "#,##0 _" UTF8_EURO ";-#,##0 _" UTF8_EURO ),
    EXC_NUMFMT_STRING(  38, "#,##0 _" UTF8_EURO ";[RED]-#,##0 _" UTF8_EURO ),
    EXC_NUMFMT_STRING(  39, "#,##0.00 _" UTF8_EURO ";-#,##0.00 _" UTF8_EURO ),
    EXC_NUMFMT_STRING(  40, "#,##0.00 _" UTF8_EURO ";[RED]-#,##0.00 _" UTF8_EURO ),
    EXC_NUMFMT_ENDTABLE()
};

// ITALIAN --------------------------------------------------------------------

static const XclBuiltInFormat spBuiltInFormats_ITALIAN_ITALY[] =
{
    EXC_NUMFMT_STRING(  15, "DD-MMM-YY" ),
    EXC_NUMFMT_STRING(  16, "DD-MMM" ),
    EXC_NUMFMT_STRING(  17, "MMM-YY" ),
    EXC_NUMFMT_STRING(  18, "h:mm AM/PM" ),
    EXC_NUMFMT_STRING(  19, "h:mm:ss AM/PM" ),
    EXC_NUMFMT_STRING(  20, "h:mm" ),
    EXC_NUMFMT_STRING(  21, "h:mm:ss" ),
    EXC_NUMFMT_STRING(  22, "DD/MM/YYYY h:mm" ),
    EXC_NUMFMT_ENDTABLE()
};

static const XclBuiltInFormat spBuiltInFormats_ITALIAN_SWISS[] =
{
    EXC_NUMFMT_STRING(  15, "DD.MMM.YY" ),
    EXC_NUMFMT_STRING(  16, "DD.MMM" ),
    EXC_NUMFMT_STRING(  17, "MMM.YY" ),
    EXC_NUMFMT_STRING(  18, "h:mm AM/PM" ),
    EXC_NUMFMT_STRING(  19, "h:mm:ss AM/PM" ),
    EXC_NUMFMT_STRING(  22, "DD.MM.YYYY hh:mm" ),
    EXC_NUMFMT_ENDTABLE()
};

// ASIAN ----------------------------------------------------------------------

/** Base table for Asian locales. */
static const XclBuiltInFormat spBuiltInFormats_CHINESE[] =
{
    EXC_NUMFMT_STRING(  18, "h:mm AM/PM" ),
    EXC_NUMFMT_STRING(  19, "h:mm:ss AM/PM" ),
    EXC_NUMFMT_STRING(  20, "h:mm" ),
    EXC_NUMFMT_STRING(  21, "h:mm:ss" ),
    EXC_NUMFMT_STRING(  23, "$#,##0_);($#,##0)" ),
    EXC_NUMFMT_STRING(  24, "$#,##0_);[RED]($#,##0)" ),
    EXC_NUMFMT_STRING(  25, "$#,##0.00_);($#,##0.00)" ),
    EXC_NUMFMT_STRING(  26, "$#,##0.00_);[RED]($#,##0.00)" ),
    EXC_NUMFMT_REUSE(   29, 28 ),
    EXC_NUMFMT_REUSE(   36, 27 ),
    EXC_NUMFMT_REUSE(   50, 27 ),
    EXC_NUMFMT_REUSE(   51, 28 ),
    EXC_NUMFMT_REUSE(   52, 34 ),
    EXC_NUMFMT_REUSE(   53, 35 ),
    EXC_NUMFMT_REUSE(   54, 28 ),
    EXC_NUMFMT_REUSE(   55, 34 ),
    EXC_NUMFMT_REUSE(   56, 35 ),
    EXC_NUMFMT_REUSE(   57, 27 ),
    EXC_NUMFMT_REUSE(   58, 28 ),
    EXC_NUMFMT_ENDTABLE()
};

static const XclBuiltInFormat spBuiltInFormats_JAPANESE[] =
{
    EXC_NUMFMT_STRING(  14, "YYYY/M/D" ),
    EXC_NUMFMT_STRING(  15, "D-MMM-YY" ),
    EXC_NUMFMT_STRING(  16, "D-MMM" ),
    EXC_NUMFMT_STRING(  17, "MMM-YY" ),
    EXC_NUMFMT_STRING(  22, "YYYY/M/D h:mm" ),
    EXC_NUMFMT_STRING(  27, "[$-0411]GE.M.D" ),
    EXC_NUMFMT_STRING(  28, "[$-0411]GGGE" UTF8_YEAR "M" UTF8_MON "D" UTF8_DAY ),
    EXC_NUMFMT_STRING(  30, "[$-0411]M/D/YY" ),
    EXC_NUMFMT_STRING(  31, "[$-0411]YYYY" UTF8_YEAR "M" UTF8_MON "D" UTF8_DAY ),
    EXC_NUMFMT_STRING(  32, "[$-0411]h" UTF8_HOUR "mm" UTF8_MIN ),
    EXC_NUMFMT_STRING(  33, "[$-0411]h" UTF8_HOUR "mm" UTF8_MIN "ss" UTF8_SEC ),
    EXC_NUMFMT_STRING(  34, "[$-0411]YYYY" UTF8_YEAR "M" UTF8_MON ),
    EXC_NUMFMT_STRING(  35, "[$-0411]M" UTF8_MON "D" UTF8_DAY ),
    EXC_NUMFMT_ENDTABLE()
};

static const XclBuiltInFormat spBuiltInFormats_KOREAN[] =
{
    EXC_NUMFMT_STRING(  14, "YYYY-MM-DD" ),
    EXC_NUMFMT_STRING(  15, "DD-MMM-YY" ),
    EXC_NUMFMT_STRING(  16, "DD-MMM" ),
    EXC_NUMFMT_STRING(  17, "MMM-YY" ),
    EXC_NUMFMT_STRING(  22, "YYYY-MM-DD h:mm" ),
    EXC_NUMFMT_STRING(  27, "[$-0412]YYYY" UTF8_YEAR " MM" UTF8_MON " DD" UTF8_DAY ),
    EXC_NUMFMT_STRING(  28, "[$-0412]MM-DD" ),
    EXC_NUMFMT_STRING(  30, "[$-0412]MM-DD-YY" ),
    EXC_NUMFMT_STRING(  31, "[$-0412]YYYY" UTF8_KO_YEAR " MM" UTF8_KO_MON " DD" UTF8_KO_DAY ),
    EXC_NUMFMT_STRING(  32, "[$-0412]h" UTF8_KO_HOUR " mm" UTF8_KO_MIN ),
    EXC_NUMFMT_STRING(  33, "[$-0412]h" UTF8_KO_HOUR " mm" UTF8_KO_MIN " ss" UTF8_KO_SEC ),
    EXC_NUMFMT_STRING(  34, "[$-0412]YYYY\"/\"MM\"/\"DD" ),
    EXC_NUMFMT_STRING(  35, "[$-0412]YYYY-MM-DD" ),
    EXC_NUMFMT_ENDTABLE()
};

static const XclBuiltInFormat spBuiltInFormats_CHINESE_SIMPLIFIED[] =
{
    EXC_NUMFMT_STRING(  14, "YYYY-M-D" ),
    EXC_NUMFMT_STRING(  15, "D-MMM-YY" ),
    EXC_NUMFMT_STRING(  16, "D-MMM" ),
    EXC_NUMFMT_STRING(  17, "MMM-YY" ),
    EXC_NUMFMT_STRING(  22, "YYYY-M-D h:mm" ),
    EXC_NUMFMT_STRING(  27, "[$-0804]YYYY" UTF8_YEAR "M" UTF8_MON ),
    EXC_NUMFMT_STRING(  28, "[$-0804]M" UTF8_MON "D" UTF8_DAY ),
    EXC_NUMFMT_STRING(  30, "[$-0804]M-D-YY" ),
    EXC_NUMFMT_STRING(  31, "[$-0804]YYYY" UTF8_YEAR "M" UTF8_MON "D" UTF8_DAY ),
    EXC_NUMFMT_STRING(  32, "[$-0804]h" UTF8_CS_HOUR "mm" UTF8_MIN ),
    EXC_NUMFMT_STRING(  33, "[$-0804]h" UTF8_CS_HOUR "mm" UTF8_MIN "ss" UTF8_SEC ),
    EXC_NUMFMT_STRING(  34, "[$-0804]AM/PMh" UTF8_CS_HOUR "mm" UTF8_MIN ),
    EXC_NUMFMT_STRING(  35, "[$-0804]AM/PMh" UTF8_CS_HOUR "mm" UTF8_MIN "ss" UTF8_SEC ),
    EXC_NUMFMT_REUSE(   52, 27 ),
    EXC_NUMFMT_REUSE(   53, 28 ),
    EXC_NUMFMT_ENDTABLE()
};

static const XclBuiltInFormat spBuiltInFormats_CHINESE_TRADITIONAL[] =
{
    EXC_NUMFMT_STRING(  15, "D-MMM-YY" ),
    EXC_NUMFMT_STRING(  16, "D-MMM" ),
    EXC_NUMFMT_STRING(  17, "MMM-YY" ),
    EXC_NUMFMT_STRING(  18, "hh:mm AM/PM" ),
    EXC_NUMFMT_STRING(  19, "hh:mm:ss AM/PM" ),
    EXC_NUMFMT_OFFSET(  20, NF_TIME_HHMM ),
    EXC_NUMFMT_OFFSET(  21, NF_TIME_HHMMSS ),
    EXC_NUMFMT_STRING(  22, "YYYY/M/D hh:mm" ),
    EXC_NUMFMT_STRING(  23, "US$#,##0_);(US$#,##0)" ),
    EXC_NUMFMT_STRING(  24, "US$#,##0_);[RED](US$#,##0)" ),
    EXC_NUMFMT_STRING(  25, "US$#,##0.00_);(US$#,##0.00)" ),
    EXC_NUMFMT_STRING(  26, "US$#,##0.00_);[RED](US$#,##0.00)" ),
    EXC_NUMFMT_STRING(  27, "[$-0404]E/M/D" ),
    EXC_NUMFMT_STRING(  28, "[$-0404]E" UTF8_YEAR "M" UTF8_MON "D" UTF8_DAY ),
    EXC_NUMFMT_STRING(  30, "[$-0404]M/D/YY" ),
    EXC_NUMFMT_STRING(  31, "[$-0404]YYYY" UTF8_YEAR "M" UTF8_MON "D" UTF8_DAY ),
    EXC_NUMFMT_STRING(  32, "[$-0404]hh" UTF8_HOUR "mm" UTF8_MIN ),
    EXC_NUMFMT_STRING(  33, "[$-0404]hh" UTF8_HOUR "mm" UTF8_MIN "ss" UTF8_SEC ),
    EXC_NUMFMT_STRING(  34, "[$-0404]AM/PMhh" UTF8_HOUR "mm" UTF8_MIN ),
    EXC_NUMFMT_STRING(  35, "[$-0404]AM/PMhh" UTF8_HOUR "mm" UTF8_MIN "ss" UTF8_SEC ),
    EXC_NUMFMT_ENDTABLE()
};

// OTHER ----------------------------------------------------------------------

static const XclBuiltInFormat spBuiltInFormats_HEBREW[] =
{
    EXC_NUMFMT_STRING(  15, "DD-MMMM-YY" ),
    EXC_NUMFMT_STRING(  16, "DD-MMMM" ),
    EXC_NUMFMT_STRING(  17, "MMMM-YY" ),
    EXC_NUMFMT_STRING(  18, "h:mm AM/PM" ),
    EXC_NUMFMT_STRING(  19, "h:mm:ss AM/PM" ),
    EXC_NUMFMT_ENDTABLE()
};

// ----------------------------------------------------------------------------

#undef EXC_NUMFMT_ENDTABLE
#undef EXC_NUMFMT_REUSE
#undef EXC_NUMFMT_OFFSET
#undef EXC_NUMFMT_STRING

// ----------------------------------------------------------------------------

/** Specifies a number format table for a specific langauge. */
struct XclBuiltInFormatTable
{
    LanguageType        meLanguage;         /// The language of this table.
    LanguageType        meParentLang;       /// The language of the parent table.
    const XclBuiltInFormat* mpFormats;      /// The number format table.
};

static const XclBuiltInFormatTable spBuiltInFormatTables[] =
{   //  language                        parent language             format table
    {   LANGUAGE_DONTKNOW,              LANGUAGE_NONE,              spBuiltInFormats_DONTKNOW               },

    {   LANGUAGE_ENGLISH,               LANGUAGE_DONTKNOW,          spBuiltInFormats_ENGLISH                },
    {   LANGUAGE_ENGLISH_UK,            LANGUAGE_ENGLISH,           0                                       },
    {   LANGUAGE_ENGLISH_EIRE,          LANGUAGE_ENGLISH,           0                                       },
    {   LANGUAGE_ENGLISH_US,            LANGUAGE_ENGLISH,           spBuiltInFormats_ENGLISH_US             },
    {   LANGUAGE_ENGLISH_CAN,           LANGUAGE_ENGLISH,           spBuiltInFormats_ENGLISH_CAN            },
    {   LANGUAGE_ENGLISH_AUS,           LANGUAGE_ENGLISH,           spBuiltInFormats_ENGLISH_AUS            },
    {   LANGUAGE_ENGLISH_SAFRICA,       LANGUAGE_ENGLISH,           spBuiltInFormats_ENGLISH_SAFRICA        },
    {   LANGUAGE_ENGLISH_NZ,            LANGUAGE_ENGLISH_AUS,       0                                       },

    {   PRV_LANGUAGE_FRENCH_PRIM,       LANGUAGE_DONTKNOW,          spBuiltInFormats_FRENCH                 },
    {   LANGUAGE_FRENCH,                PRV_LANGUAGE_FRENCH_PRIM,   spBuiltInFormats_FRENCH_FRANCE          },
    {   LANGUAGE_FRENCH_CANADIAN,       PRV_LANGUAGE_FRENCH_PRIM,   spBuiltInFormats_FRENCH_CANADIAN        },
    {   LANGUAGE_FRENCH_SWISS,          PRV_LANGUAGE_FRENCH_PRIM,   spBuiltInFormats_FRENCH_SWISS           },
    {   LANGUAGE_FRENCH_BELGIAN,        LANGUAGE_FRENCH,            spBuiltInFormats_FRENCH_BELGIAN         },
    {   LANGUAGE_FRENCH_LUXEMBOURG,     LANGUAGE_FRENCH,            0                                       },
    {   LANGUAGE_FRENCH_MONACO,         LANGUAGE_FRENCH,            0                                       },

    {   PRV_LANGUAGE_GERMAN_PRIM,       LANGUAGE_DONTKNOW,          spBuiltInFormats_GERMAN                 },
    {   LANGUAGE_GERMAN,                PRV_LANGUAGE_GERMAN_PRIM,   spBuiltInFormats_GERMAN_GERMANY         },
    {   LANGUAGE_GERMAN_AUSTRIAN,       PRV_LANGUAGE_GERMAN_PRIM,   spBuiltInFormats_GERMAN_AUSTRIAN        },
    {   LANGUAGE_GERMAN_SWISS,          PRV_LANGUAGE_GERMAN_PRIM,   0                                       },
    {   LANGUAGE_GERMAN_LUXEMBOURG,     PRV_LANGUAGE_GERMAN_PRIM,   spBuiltInFormats_GERMAN_LUXEMBOURG      },
    {   LANGUAGE_GERMAN_LIECHTENSTEIN,  PRV_LANGUAGE_GERMAN_PRIM,   0                                       },

    {   LANGUAGE_ITALIAN,               LANGUAGE_DONTKNOW,          spBuiltInFormats_ITALIAN_ITALY          },
    {   LANGUAGE_ITALIAN_SWISS,         LANGUAGE_DONTKNOW,          spBuiltInFormats_ITALIAN_SWISS          },

    {   LANGUAGE_CHINESE,               LANGUAGE_DONTKNOW,          spBuiltInFormats_CHINESE                },
    {   LANGUAGE_JAPANESE,              LANGUAGE_CHINESE,           spBuiltInFormats_JAPANESE               },
    {   LANGUAGE_KOREAN,                LANGUAGE_CHINESE,           spBuiltInFormats_KOREAN                 },
    {   LANGUAGE_CHINESE_SIMPLIFIED,    LANGUAGE_CHINESE,           spBuiltInFormats_CHINESE_SIMPLIFIED     },
    {   LANGUAGE_CHINESE_TRADITIONAL,   LANGUAGE_CHINESE,           spBuiltInFormats_CHINESE_TRADITIONAL    },

    {   LANGUAGE_HEBREW,                LANGUAGE_DONTKNOW,          spBuiltInFormats_HEBREW                 }
};

// ----------------------------------------------------------------------------

} // namespace

// ============================================================================

XclNumFmtBuffer::XclNumFmtBuffer( const XclRoot& rRoot ) :
    meSysLang( rRoot.GetSysLanguage() ),
    mnStdScNumFmt( rRoot.GetFormatter().GetStandardFormat( ScGlobal::eLnge ) )
{
    // *** insert default formats (BIFF5+ only)***

    if( rRoot.GetBiff() >= xlBiff5 )
        InsertBuiltinFormats();
}

const XclNumFmt* XclNumFmtBuffer::GetFormat( sal_uInt16 nXclNumFmt ) const
{
    XclNumFmtMap::const_iterator aIt = maFmtMap.find( nXclNumFmt );
    return (aIt != maFmtMap.end()) ? &aIt->second : 0;
}

void XclNumFmtBuffer::InsertFormat( sal_uInt16 nXclNumFmt, const String& rFormat )
{
    XclNumFmt& rNumFmt = maFmtMap[ nXclNumFmt ];
    rNumFmt.maFormat = rFormat;
    rNumFmt.meLanguage = LANGUAGE_SYSTEM;
}

void XclNumFmtBuffer::InsertBuiltinFormats()
{
    // build a map containing tables for all languages
    typedef ::std::map< LanguageType, const XclBuiltInFormatTable* > XclBuiltInMap;
    XclBuiltInMap aBuiltInMap;
    for( const XclBuiltInFormatTable* pTable = spBuiltInFormatTables;
            pTable != STATIC_TABLE_END( spBuiltInFormatTables ); ++pTable )
        aBuiltInMap[ pTable->meLanguage ] = pTable;

    // build a list of table pointers for the current language, with all parent tables
    typedef ::std::vector< const XclBuiltInFormatTable* > XclBuiltInVec;
    XclBuiltInVec aBuiltInVec;
    for( XclBuiltInMap::const_iterator aMIt = aBuiltInMap.find( meSysLang ), aMEnd = aBuiltInMap.end();
            aMIt != aMEnd; aMIt = aBuiltInMap.find( aMIt->second->meParentLang ) )
        aBuiltInVec.push_back( aMIt->second );
    // language not supported
    if( aBuiltInVec.empty() )
    {
        DBG_ERROR1( "XclNumFmtBuffer::InsertBuiltinFormats - language 0x%04hX not supported (#i29949#)", meSysLang );
        XclBuiltInMap::const_iterator aMIt = aBuiltInMap.find( LANGUAGE_DONTKNOW );
        DBG_ASSERT( aMIt != aBuiltInMap.end(), "XclNumFmtBuffer::InsertBuiltinFormats - default map not found" );
        if( aMIt != aBuiltInMap.end() )
            aBuiltInVec.push_back( aMIt->second );
    }

    // insert the default formats in the format map, from root parent to system language
    typedef ::std::map< sal_uInt16, sal_uInt16 > XclReuseMap;
    XclReuseMap aReuseMap;
    for( XclBuiltInVec::reverse_iterator aVIt = aBuiltInVec.rbegin(), aVEnd = aBuiltInVec.rend(); aVIt != aVEnd; ++aVIt )
    {
        // put LANGUAGE_SYSTEM for all entries in default table
        LanguageType eLang = ((*aVIt)->meLanguage == LANGUAGE_DONTKNOW) ? LANGUAGE_SYSTEM : meSysLang;
        for( const XclBuiltInFormat* pBuiltIn = (*aVIt)->mpFormats; pBuiltIn && (pBuiltIn->mnXclNumFmt != EXC_BUILTIN_NOFORMAT); ++pBuiltIn )
        {
            XclNumFmt& rNumFmt = maFmtMap[ pBuiltIn->mnXclNumFmt ];

            rNumFmt.meOffset = pBuiltIn->meOffset;
            rNumFmt.meLanguage = eLang;

            if( pBuiltIn->mpFormat )
                rNumFmt.maFormat = String( pBuiltIn->mpFormat, RTL_TEXTENCODING_UTF8 );
            else
                rNumFmt.maFormat = EMPTY_STRING;

            if( pBuiltIn->meOffset == PRV_NF_INDEX_REUSE )
                aReuseMap[ pBuiltIn->mnXclNumFmt ] = pBuiltIn->mnXclReuseFmt;
            else
                aReuseMap.erase( pBuiltIn->mnXclNumFmt );
        }
    }

    // copy reused number formats
    for( XclReuseMap::const_iterator aRIt = aReuseMap.begin(), aREnd = aReuseMap.end(); aRIt != aREnd; ++aRIt )
        maFmtMap[ aRIt->first ] = maFmtMap[ aRIt->second ];
}

// Cell formatting data (XF) ==================================================

XclCellProt::XclCellProt() :
    mbLocked( true ),       // default in Excel and Calc
    mbHidden( false )
{
}

bool operator==( const XclCellProt& rLeft, const XclCellProt& rRight )
{
    return (rLeft.mbLocked == rRight.mbLocked) && (rLeft.mbHidden == rRight.mbHidden);
}

// ----------------------------------------------------------------------------

XclCellAlign::XclCellAlign() :
    meHorAlign( xlHAlign_Default ),
    meVerAlign( xlVAlign_Default ),
    meTextDir( xlTextDir_Default ),
    meOrient( xlTextOrient_Default ),
    mnRotation( 0 ),
    mnIndent( 0 ),
    mbWrapped( false )
{
}

bool operator==( const XclCellAlign& rLeft, const XclCellAlign& rRight )
{
    return
        (rLeft.meHorAlign == rRight.meHorAlign) && (rLeft.meVerAlign == rRight.meVerAlign) &&
        (rLeft.meTextDir  == rRight.meTextDir)  &&
        (rLeft.meOrient   == rRight.meOrient)   && (rLeft.mnRotation == rRight.mnRotation) &&
        (rLeft.mnIndent   == rRight.mnIndent)   && (rLeft.mbWrapped  == rRight.mbWrapped);
}

// ----------------------------------------------------------------------------

XclCellBorder::XclCellBorder() :
    mnLeftColor( 0 ),
    mnRightColor( 0 ),
    mnTopColor( 0 ),
    mnBottomColor( 0 ),
    mnLeftLine( EXC_LINE_NONE ),
    mnRightLine( EXC_LINE_NONE ),
    mnTopLine( EXC_LINE_NONE ),
    mnBottomLine( EXC_LINE_NONE )
{
}

bool operator==( const XclCellBorder& rLeft, const XclCellBorder& rRight )
{
    return
        (rLeft.mnLeftColor == rRight.mnLeftColor) && (rLeft.mnRightColor  == rRight.mnRightColor)  &&
        (rLeft.mnTopColor  == rRight.mnTopColor)  && (rLeft.mnBottomColor == rRight.mnBottomColor) &&
        (rLeft.mnLeftLine  == rRight.mnLeftLine)  && (rLeft.mnRightLine   == rRight.mnRightLine)   &&
        (rLeft.mnTopLine   == rRight.mnTopLine)   && (rLeft.mnBottomLine  == rRight.mnBottomLine);
}

// ----------------------------------------------------------------------------

XclCellArea::XclCellArea() :
    mnForeColor( EXC_COLOR_WINDOWTEXT ),
    mnBackColor( EXC_COLOR_WINDOWBACK ),
    mnPattern( EXC_PATT_NONE )
{
}

bool XclCellArea::IsTransparent() const
{
    return (mnPattern == EXC_PATT_NONE) && (mnBackColor == EXC_COLOR_WINDOWBACK);
}

bool operator==( const XclCellArea& rLeft, const XclCellArea& rRight )
{
    return
        (rLeft.mnForeColor == rRight.mnForeColor) && (rLeft.mnBackColor == rRight.mnBackColor) &&
        (rLeft.mnPattern == rRight.mnPattern);
}

// ----------------------------------------------------------------------------

XclXFBase::XclXFBase( bool bCellXF ) :
    mnParent( bCellXF ? EXC_XF_DEFAULTSTYLE : EXC_XF_STYLEPARENT ),
    mbCellXF( bCellXF )
{
    SetAllUsedFlags( false );
}

void XclXFBase::SetAllUsedFlags( bool bUsed )
{
    mbProtUsed = mbFontUsed = mbFmtUsed = mbAlignUsed = mbBorderUsed = mbAreaUsed = bUsed;
}

bool XclXFBase::Equals( const XclXFBase& rCmp ) const
{
    return
        (mbCellXF     == rCmp.mbCellXF)     && (mnParent    == rCmp.mnParent)    &&
        (mbProtUsed   == rCmp.mbProtUsed)   && (mbFontUsed  == rCmp.mbFontUsed)  &&
        (mbFmtUsed    == rCmp.mbFmtUsed)    && (mbAlignUsed == rCmp.mbAlignUsed) &&
        (mbBorderUsed == rCmp.mbBorderUsed) && (mbAreaUsed  == rCmp.mbAreaUsed);
}

// ============================================================================

