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


#include "xlstyle.hxx"
#include <com/sun/star/awt/FontFamily.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <vcl/svapp.hxx>
#include <vcl/font.hxx>
#include <sal/macros.h>
#include <rtl/tencinfo.h>
#include <svtools/colorcfg.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <editeng/svxfont.hxx>
#include "global.hxx"
#include "xlroot.hxx"
// Color data =================================================================

/** Standard EGA colors, bright. */
#define EXC_PALETTE_EGA_COLORS_LIGHT \
            0x000000, 0xFFFFFF, 0xFF0000, 0x00FF00, 0x0000FF, 0xFFFF00, 0xFF00FF, 0x00FFFF
/** Standard EGA colors, dark. */
#define EXC_PALETTE_EGA_COLORS_DARK \
            0x800000, 0x008000, 0x000080, 0x808000, 0x800080, 0x008080, 0xC0C0C0, 0x808080

/** Default color table for BIFF2. */
static const ColorData spnDefColorTable2[] =
{
/*  0 */    EXC_PALETTE_EGA_COLORS_LIGHT
};

/** Default color table for BIFF3/BIFF4. */
static const ColorData spnDefColorTable3[] =
{
/*  0 */    EXC_PALETTE_EGA_COLORS_LIGHT,
/*  8 */    EXC_PALETTE_EGA_COLORS_LIGHT,
/* 16 */    EXC_PALETTE_EGA_COLORS_DARK
};

/** Default color table for BIFF5/BIFF7. */
static const ColorData spnDefColorTable5[] =
{
/*  0 */    EXC_PALETTE_EGA_COLORS_LIGHT,
/*  8 */    EXC_PALETTE_EGA_COLORS_LIGHT,
/* 16 */    EXC_PALETTE_EGA_COLORS_DARK,
/* 24 */    0x8080FF, 0x802060, 0xFFFFC0, 0xA0E0E0, 0x600080, 0xFF8080, 0x0080C0, 0xC0C0FF,
/* 32 */    0x000080, 0xFF00FF, 0xFFFF00, 0x00FFFF, 0x800080, 0x800000, 0x008080, 0x0000FF,
/* 40 */    0x00CFFF, 0x69FFFF, 0xE0FFE0, 0xFFFF80, 0xA6CAF0, 0xDD9CB3, 0xB38FEE, 0xE3E3E3,
/* 48 */    0x2A6FF9, 0x3FB8CD, 0x488436, 0x958C41, 0x8E5E42, 0xA0627A, 0x624FAC, 0x969696,
/* 56 */    0x1D2FBE, 0x286676, 0x004500, 0x453E01, 0x6A2813, 0x85396A, 0x4A3285, 0x424242
};

/** Default color table for BIFF8. */
static const ColorData spnDefColorTable8[] =
{
/*  0 */    EXC_PALETTE_EGA_COLORS_LIGHT,
/*  8 */    EXC_PALETTE_EGA_COLORS_LIGHT,
/* 16 */    EXC_PALETTE_EGA_COLORS_DARK,
/* 24 */    0x9999FF, 0x993366, 0xFFFFCC, 0xCCFFFF, 0x660066, 0xFF8080, 0x0066CC, 0xCCCCFF,
/* 32 */    0x000080, 0xFF00FF, 0xFFFF00, 0x00FFFF, 0x800080, 0x800000, 0x008080, 0x0000FF,
/* 40 */    0x00CCFF, 0xCCFFFF, 0xCCFFCC, 0xFFFF99, 0x99CCFF, 0xFF99CC, 0xCC99FF, 0xFFCC99,
/* 48 */    0x3366FF, 0x33CCCC, 0x99CC00, 0xFFCC00, 0xFF9900, 0xFF6600, 0x666699, 0x969696,
/* 56 */    0x003366, 0x339966, 0x003300, 0x333300, 0x993300, 0x993366, 0x333399, 0x333333
};

#undef EXC_PALETTE_EGA_COLORS_LIGHT
#undef EXC_PALETTE_EGA_COLORS_DARK

// ----------------------------------------------------------------------------

XclDefaultPalette::XclDefaultPalette( const XclRoot& rRoot ) :
    mpnColorTable( 0 ),
    mnTableSize( 0 )
{
    const StyleSettings& rSett = Application::GetSettings().GetStyleSettings();
    mnWindowText = rSett.GetWindowTextColor().GetColor();
    mnWindowBack = rSett.GetWindowColor().GetColor();
    mnFaceColor = rSett.GetFaceColor().GetColor();
    // Don't use the system HelpBack and HelpText colours as it causes problems
    // with modern gnome. This is because mnNoteText and mnNoteBack are used
    // when colour indices ( instead of real colours ) are specified.
    // Note: That this it is not an unusual scenario that we get the Note
    // background specified as a real colour and the text specified as a
    // colour index. That means the text colour would be picked from
    // the system where the note background would be picked from a real colour.
    // Previously the the note text colour was picked from the system tooltip
    // text colour, on modern gnome(e.g. 3) that tends to be 'white' with the
    // default theme.
    // Using the the Libreoffice defaults ( instead of system specific colours
    // ) lessens the chance of the one colour being an unsuitable combination
    // because by default the note text is black and the note background is
    // a light yellow colour ( very similar to Excel's normal defaults )
    mnNoteText =  svtools::ColorConfig::GetDefaultColor( svtools::FONTCOLOR ).GetColor();
    mnNoteBack =  svtools::ColorConfig::GetDefaultColor( svtools::CALCNOTESBACKGROUND ).GetColor();

    // default colors
    switch( rRoot.GetBiff() )
    {
        case EXC_BIFF2:
            mpnColorTable = spnDefColorTable2;
            mnTableSize = SAL_N_ELEMENTS( spnDefColorTable2 );
        break;
        case EXC_BIFF3:
        case EXC_BIFF4:
            mpnColorTable = spnDefColorTable3;
            mnTableSize = SAL_N_ELEMENTS( spnDefColorTable3 );
        break;
        case EXC_BIFF5:
            mpnColorTable = spnDefColorTable5;
            mnTableSize = SAL_N_ELEMENTS( spnDefColorTable5 );
        break;
        case EXC_BIFF8:
            mpnColorTable = spnDefColorTable8;
            mnTableSize = SAL_N_ELEMENTS( spnDefColorTable8 );
        break;
        default:
            DBG_ERROR_BIFF();
    }
}

ColorData XclDefaultPalette::GetDefColorData( sal_uInt16 nXclIndex ) const
{
    ColorData nColor;
    if( nXclIndex < mnTableSize )
        nColor = mpnColorTable[ nXclIndex ];
    else switch( nXclIndex )
    {
        case EXC_COLOR_WINDOWTEXT3:
        case EXC_COLOR_WINDOWTEXT:
        case EXC_COLOR_CHWINDOWTEXT:    nColor = mnWindowText;  break;
        case EXC_COLOR_WINDOWBACK3:
        case EXC_COLOR_WINDOWBACK:
        case EXC_COLOR_CHWINDOWBACK:    nColor = mnWindowBack;  break;
        case EXC_COLOR_BUTTONBACK:      nColor = mnFaceColor;   break;
        case EXC_COLOR_CHBORDERAUTO:    nColor = COL_BLACK;     break;  // TODO: really always black?
        case EXC_COLOR_NOTEBACK:        nColor = mnNoteBack;    break;
        case EXC_COLOR_NOTETEXT:        nColor = mnNoteText;    break;
        case EXC_COLOR_FONTAUTO:        nColor = COL_AUTO;      break;
        default:
            OSL_TRACE( "XclDefaultPalette::GetDefColorData - unknown default color index: %d", nXclIndex );
            nColor = COL_AUTO;
    }
    return nColor;
}

// Font Data ==================================================================

namespace Awt              = ::com::sun::star::awt;
namespace AwtFontFamily    = Awt::FontFamily;
namespace AwtFontUnderline = Awt::FontUnderline;
namespace AwtFontStrikeout = Awt::FontStrikeout;

// ----------------------------------------------------------------------------

XclFontData::XclFontData()
{
    Clear();
}

XclFontData::XclFontData( const Font& rFont )
{
    Clear();
    FillFromVclFont( rFont );
}

XclFontData::XclFontData( const SvxFont& rFont )
{
    FillFromSvxFont( rFont );
}

void XclFontData::Clear()
{
    maName = "";
    maStyle.Erase();
    maColor.SetColor( COL_AUTO );
    mnHeight = 0;
    mnWeight = EXC_FONTWGHT_DONTKNOW;
    mnEscapem = EXC_FONTESC_NONE;
    mnFamily = EXC_FONTFAM_SYSTEM;
    mnCharSet = EXC_FONTCSET_ANSI_LATIN;
    mnUnderline = EXC_FONTUNDERL_NONE;
    mbItalic = mbStrikeout = mbOutline = mbShadow = false;
}

void XclFontData::FillFromVclFont( const Font& rFont )
{
    maName = XclTools::GetXclFontName( rFont.GetName() );   // substitute with MS fonts
    maStyle.Erase();
    maColor = rFont.GetColor();
    SetScUnderline( rFont.GetUnderline() );
    mnEscapem = EXC_FONTESC_NONE;
    SetScHeight( rFont.GetSize().Height() );
    SetScWeight( rFont.GetWeight() );
    SetScFamily( rFont.GetFamily() );
    SetFontEncoding( rFont.GetCharSet() );
    SetScPosture( rFont.GetItalic() );
    SetScStrikeout( rFont.GetStrikeout() );
    mbOutline = rFont.IsOutline();
    mbShadow = rFont.IsShadow();
}

void XclFontData::FillFromSvxFont( const SvxFont& rFont )
{
    FillFromVclFont( rFont );
    SetScEscapement( rFont.GetEscapement() );
}

// *** conversion of VCL/SVX constants *** ------------------------------------

FontFamily XclFontData::GetScFamily( rtl_TextEncoding eDefTextEnc ) const
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
                ((eDefTextEnc == RTL_TEXTENCODING_APPLE_ROMAN) &&
                (maName.equalsIgnoreAsciiCase( "Geneva" ) || maName.equalsIgnoreAsciiCase( "Chicago" ))) ?
                FAMILY_SWISS : FAMILY_DONTKNOW;
    }
    return eScFamily;
}

rtl_TextEncoding XclFontData::GetFontEncoding() const
{
    // convert Windows character set to text encoding identifier
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
    mnHeight = static_cast< sal_uInt16 >( ::std::min( nTwips, static_cast<sal_Int32>(0x7FFFL) ) );
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
            OSL_FAIL( "XclFontData::SetScFamily - unknown font family" );
            mnFamily = EXC_FONTFAM_DONTKNOW;
    }
}

void XclFontData::SetFontEncoding( rtl_TextEncoding eFontEnc )
{
    // convert text encoding identifier to Windows character set
    mnCharSet = rtl_getBestWindowsCharsetFromTextEncoding( eFontEnc );
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

float XclFontData::GetApiHeight() const
{
    return static_cast< float >( mnHeight / TWIPS_PER_POINT );
}

sal_Int16 XclFontData::GetApiFamily() const
{
    sal_Int16 nApiFamily = AwtFontFamily::DONTKNOW;
    switch( mnFamily )
    {
        case FAMILY_DECORATIVE: nApiFamily = AwtFontFamily::DECORATIVE; break;
        case FAMILY_MODERN:     nApiFamily = AwtFontFamily::MODERN;     break;
        case FAMILY_ROMAN:      nApiFamily = AwtFontFamily::ROMAN;      break;
        case FAMILY_SCRIPT:     nApiFamily = AwtFontFamily::SCRIPT;     break;
        case FAMILY_SWISS:      nApiFamily = AwtFontFamily::SWISS;      break;
        case FAMILY_SYSTEM:     nApiFamily = AwtFontFamily::SYSTEM;     break;
    }
    return nApiFamily;
}

sal_Int16 XclFontData::GetApiFontEncoding() const
{
    // API constants are equal to rtl_TextEncoding constants
    return static_cast< sal_Int16 >( GetFontEncoding() );
}

Awt::FontSlant XclFontData::GetApiPosture() const
{
    return mbItalic ? Awt::FontSlant_ITALIC : Awt::FontSlant_NONE;
}

float XclFontData::GetApiWeight() const
{
    return VCLUnoHelper::ConvertFontWeight( GetScWeight() );
}

sal_Int16 XclFontData::GetApiUnderline() const
{
    sal_Int16 nApiUnderl = AwtFontUnderline::NONE;
    switch( mnUnderline )
    {
        case EXC_FONTUNDERL_SINGLE:
        case EXC_FONTUNDERL_SINGLE_ACC: nApiUnderl = AwtFontUnderline::SINGLE;  break;
        case EXC_FONTUNDERL_DOUBLE:
        case EXC_FONTUNDERL_DOUBLE_ACC: nApiUnderl = AwtFontUnderline::DOUBLE;  break;
    }
    return nApiUnderl;
}

sal_Int16 XclFontData::GetApiEscapement() const
{
    sal_Int16 nApiEscapem = 0;
    switch( mnEscapem )
    {
        case EXC_FONTESC_SUPER: nApiEscapem = 33;   break;
        case EXC_FONTESC_SUB:   nApiEscapem = -33;  break;
    }
    return nApiEscapem;
}

sal_Int16 XclFontData::GetApiStrikeout() const
{
    return mbStrikeout ? AwtFontStrikeout::SINGLE : AwtFontStrikeout::NONE;
}

void XclFontData::SetApiHeight( float fPoint )
{
    mnHeight = static_cast< sal_uInt16 >( ::std::min( fPoint * TWIPS_PER_POINT + 0.5, 32767.0 ) );
}

void XclFontData::SetApiFamily( sal_Int16 nApiFamily )
{
    switch( nApiFamily )
    {
        case AwtFontFamily::DECORATIVE: mnFamily = FAMILY_DECORATIVE;   break;
        case AwtFontFamily::MODERN:     mnFamily = FAMILY_MODERN;       break;
        case AwtFontFamily::ROMAN:      mnFamily = FAMILY_ROMAN;        break;
        case AwtFontFamily::SCRIPT:     mnFamily = FAMILY_SCRIPT;       break;
        case AwtFontFamily::SWISS:      mnFamily = FAMILY_SWISS;        break;
        case AwtFontFamily::SYSTEM:     mnFamily = FAMILY_SYSTEM;       break;
        default:                        mnFamily = FAMILY_DONTKNOW;
    }
}

void XclFontData::SetApiPosture( Awt::FontSlant eApiPosture )
{
    mbItalic =
        (eApiPosture == Awt::FontSlant_OBLIQUE) ||
        (eApiPosture == Awt::FontSlant_ITALIC) ||
        (eApiPosture == Awt::FontSlant_REVERSE_OBLIQUE) ||
        (eApiPosture == Awt::FontSlant_REVERSE_ITALIC);
}

void XclFontData::SetApiWeight( float fApiWeight )
{
    SetScWeight( VCLUnoHelper::ConvertFontWeight( fApiWeight ) );
}

void XclFontData::SetApiUnderline( sal_Int16 nApiUnderl )
{
    switch( nApiUnderl )
    {
        case AwtFontUnderline::NONE:
        case AwtFontUnderline::DONTKNOW:    mnUnderline = EXC_FONTUNDERL_NONE;      break;
        case AwtFontUnderline::DOUBLE:
        case AwtFontUnderline::DOUBLEWAVE:  mnUnderline = EXC_FONTUNDERL_DOUBLE;    break;
        default:                            mnUnderline = EXC_FONTUNDERL_SINGLE;
    }
}

void XclFontData::SetApiEscapement( sal_Int16 nApiEscapem )
{
    if( nApiEscapem > 0 )
        mnEscapem = EXC_FONTESC_SUPER;
    else if( nApiEscapem < 0 )
        mnEscapem = EXC_FONTESC_SUB;
    else
        mnEscapem = EXC_FONTESC_NONE;
}

void XclFontData::SetApiStrikeout( sal_Int16 nApiStrikeout )
{
    mbStrikeout =
        (nApiStrikeout != AwtFontStrikeout::NONE) &&
        (nApiStrikeout != AwtFontStrikeout::DONTKNOW);
}

// ----------------------------------------------------------------------------

bool operator==( const XclFontData& rLeft, const XclFontData& rRight )
{
    return
        (rLeft.mnHeight    == rRight.mnHeight)    &&
        (rLeft.mnWeight    == rRight.mnWeight)    &&
        (rLeft.mnUnderline == rRight.mnUnderline) &&
        (rLeft.maColor     == rRight.maColor)     &&
        (rLeft.mnEscapem   == rRight.mnEscapem)   &&
        (rLeft.mnFamily    == rRight.mnFamily)    &&
        (rLeft.mnCharSet   == rRight.mnCharSet)   &&
        (rLeft.mbItalic    == rRight.mbItalic)    &&
        (rLeft.mbStrikeout == rRight.mbStrikeout) &&
        (rLeft.mbOutline   == rRight.mbOutline)   &&
        (rLeft.mbShadow    == rRight.mbShadow)    &&
        (rLeft.maName      == rRight.maName);
}

// ----------------------------------------------------------------------------

namespace {

/** Property names for common font settings. */
const sal_Char *const sppcPropNamesChCommon[] =
{
    "CharUnderline", "CharStrikeout", "CharColor", "CharContoured", "CharShadowed", 0
};
/** Property names for Western font settings. */
const sal_Char *const sppcPropNamesChWstrn[] =
{
    "CharFontName", "CharHeight", "CharPosture", "CharWeight", 0
};
/** Property names for Asian font settings. */
const sal_Char *const sppcPropNamesChAsian[] =
{
    "CharFontNameAsian", "CharHeightAsian", "CharPostureAsian", "CharWeightAsian", 0
};
/** Property names for Complex font settings. */
const sal_Char *const sppcPropNamesChCmplx[] =
{
    "CharFontNameComplex", "CharHeightComplex", "CharPostureComplex", "CharWeightComplex", 0
};
/** Property names for escapement. */
const sal_Char *const sppcPropNamesChEscapement[] =
{
    "CharEscapement", "CharEscapementHeight", 0
};
const sal_Int8 EXC_API_ESC_HEIGHT           = 58;   /// Default escapement font height.

/** Property names for Western font settings without font name. */
const sal_Char *const *const sppcPropNamesChWstrnNoName = sppcPropNamesChWstrn + 1;
/** Property names for Asian font settings without font name. */
const sal_Char *const *const sppcPropNamesChAsianNoName = sppcPropNamesChAsian + 1;
/** Property names for Complex font settings without font name. */
const sal_Char *const *const sppcPropNamesChCmplxNoName = sppcPropNamesChCmplx + 1;

/** Property names for font settings in form controls. */
const sal_Char *const sppcPropNamesControl[] =
{
    "FontName", "FontFamily", "FontCharset", "FontHeight", "FontSlant",
    "FontWeight", "FontUnderline", "FontStrikeout", "TextColor", 0
};

/** Inserts all passed API font settings into the font data object. */
void lclSetApiFontSettings( XclFontData& rFontData,
        const String& rApiFontName, float fApiHeight, float fApiWeight,
        Awt::FontSlant eApiPosture, sal_Int16 nApiUnderl, sal_Int16 nApiStrikeout )
{
    rFontData.maName = XclTools::GetXclFontName( rApiFontName );
    rFontData.SetApiHeight( fApiHeight );
    rFontData.SetApiWeight( fApiWeight );
    rFontData.SetApiPosture( eApiPosture );
    rFontData.SetApiUnderline( nApiUnderl );
    rFontData.SetApiStrikeout( nApiStrikeout );
}

/** Writes script dependent properties to a font property set helper. */
void lclWriteChartFont( ScfPropertySet& rPropSet,
        ScfPropSetHelper& rHlpName, ScfPropSetHelper& rHlpNoName,
        const XclFontData& rFontData, bool bHasFontName )
{
    // select the font helper
    ScfPropSetHelper& rPropSetHlp = bHasFontName ? rHlpName : rHlpNoName;
    // initialize the font helper (must be called before writing any properties)
    rPropSetHlp.InitializeWrite();
    // write font name
    if( bHasFontName )
        rPropSetHlp << rFontData.maName;
    // write remaining properties
    rPropSetHlp << rFontData.GetApiHeight() << rFontData.GetApiPosture() << rFontData.GetApiWeight();
    // write properties to property set
    rPropSetHlp.WriteToPropertySet( rPropSet );
}

} // namespace

// ----------------------------------------------------------------------------

XclFontPropSetHelper::XclFontPropSetHelper() :
    maHlpChCommon( sppcPropNamesChCommon ),
    maHlpChWstrn( sppcPropNamesChWstrn ),
    maHlpChAsian( sppcPropNamesChAsian ),
    maHlpChCmplx( sppcPropNamesChCmplx ),
    maHlpChWstrnNoName( sppcPropNamesChWstrnNoName ),
    maHlpChAsianNoName( sppcPropNamesChAsianNoName ),
    maHlpChCmplxNoName( sppcPropNamesChCmplxNoName ),
    maHlpChEscapement( sppcPropNamesChEscapement ),
    maHlpControl( sppcPropNamesControl )
{
}

void XclFontPropSetHelper::ReadFontProperties( XclFontData& rFontData,
        const ScfPropertySet& rPropSet, XclFontPropSetType eType, sal_Int16 nScript )
{
    switch( eType )
    {
        case EXC_FONTPROPSET_CHART:
        {
            String aApiFontName;
            float fApiHeight, fApiWeight;
            sal_Int16 nApiUnderl = 0, nApiStrikeout = 0;
            Awt::FontSlant eApiPosture;

            // read script type dependent properties
            ScfPropSetHelper& rPropSetHlp = GetChartHelper( nScript );
            rPropSetHlp.ReadFromPropertySet( rPropSet );
            rPropSetHlp >> aApiFontName >> fApiHeight >> eApiPosture >> fApiWeight;
            // read common properties
            maHlpChCommon.ReadFromPropertySet( rPropSet );
            maHlpChCommon   >> nApiUnderl
                            >> nApiStrikeout
                            >> rFontData.maColor
                            >> rFontData.mbOutline
                            >> rFontData.mbShadow;

            // convert API property values to Excel settings
            lclSetApiFontSettings( rFontData, aApiFontName,
                fApiHeight, fApiWeight, eApiPosture, nApiUnderl, nApiStrikeout );

            // font escapement
            sal_Int16 nApiEscapement = 0;
            sal_Int8 nApiEscHeight = 0;
            maHlpChEscapement.ReadFromPropertySet( rPropSet );
            maHlpChEscapement.ReadFromPropertySet( rPropSet );
            maHlpChEscapement.ReadFromPropertySet( rPropSet );
            maHlpChEscapement >> nApiEscapement >> nApiEscHeight;
            rFontData.SetApiEscapement( nApiEscapement );
        }
        break;

        case EXC_FONTPROPSET_CONTROL:
        {
            String aApiFontName;
            float fApiHeight, fApiWeight;
            sal_Int16 nApiFamily, nApiCharSet, nApiPosture, nApiUnderl, nApiStrikeout;

            // read font properties
            maHlpControl.ReadFromPropertySet( rPropSet );
            maHlpControl    >> aApiFontName
                            >> nApiFamily
                            >> nApiCharSet
                            >> fApiHeight
                            >> nApiPosture
                            >> fApiWeight
                            >> nApiUnderl
                            >> nApiStrikeout
                            >> rFontData.maColor;

            // convert API property values to Excel settings
            Awt::FontSlant eApiPosture = static_cast< Awt::FontSlant >( nApiPosture );
            lclSetApiFontSettings( rFontData, aApiFontName,
                fApiHeight, fApiWeight, eApiPosture, nApiUnderl, nApiStrikeout );
            rFontData.SetApiFamily( nApiFamily );
            rFontData.SetFontEncoding( nApiCharSet );
        }
        break;
    }
}

void XclFontPropSetHelper::WriteFontProperties(
        ScfPropertySet& rPropSet, XclFontPropSetType eType,
        const XclFontData& rFontData, bool bHasWstrn, bool bHasAsian, bool bHasCmplx,
        const Color* pFontColor )
{
    switch( eType )
    {
        case EXC_FONTPROPSET_CHART:
        {
            // write common properties
            maHlpChCommon.InitializeWrite();
            const Color& rColor = pFontColor ? *pFontColor : rFontData.maColor;
            maHlpChCommon   << rFontData.GetApiUnderline()
                            << rFontData.GetApiStrikeout()
                            << rColor
                            << rFontData.mbOutline
                            << rFontData.mbShadow;
            maHlpChCommon.WriteToPropertySet( rPropSet );

            // write script type dependent properties
            lclWriteChartFont( rPropSet, maHlpChWstrn, maHlpChWstrnNoName, rFontData, bHasWstrn );
            lclWriteChartFont( rPropSet, maHlpChAsian, maHlpChAsianNoName, rFontData, bHasAsian );
            lclWriteChartFont( rPropSet, maHlpChCmplx, maHlpChCmplxNoName, rFontData, bHasCmplx );

            // font escapement
            if( rFontData.GetScEscapement() != SVX_ESCAPEMENT_OFF )
            {
                maHlpChEscapement.InitializeWrite();
                maHlpChEscapement << rFontData.GetApiEscapement() << EXC_API_ESC_HEIGHT;
                maHlpChEscapement.WriteToPropertySet( rPropSet );
            }
        }
        break;

        case EXC_FONTPROPSET_CONTROL:
        {
            maHlpControl.InitializeWrite();
            maHlpControl    << rFontData.maName
                            << rFontData.GetApiFamily()
                            << rFontData.GetApiFontEncoding()
                            << static_cast< sal_Int16 >( rFontData.GetApiHeight() + 0.5 )
                            << rFontData.GetApiPosture()
                            << rFontData.GetApiWeight()
                            << rFontData.GetApiUnderline()
                            << rFontData.GetApiStrikeout()
                            << rFontData.maColor;
            maHlpControl.WriteToPropertySet( rPropSet );
        }
        break;
    }
}

ScfPropSetHelper& XclFontPropSetHelper::GetChartHelper( sal_Int16 nScript )
{
    namespace ApiScriptType = ::com::sun::star::i18n::ScriptType;
    switch( nScript )
    {
        case ApiScriptType::LATIN:      return maHlpChWstrn;
        case ApiScriptType::ASIAN:      return maHlpChAsian;
        case ApiScriptType::COMPLEX:    return maHlpChCmplx;
        default:    OSL_FAIL( "XclFontPropSetHelper::GetChartHelper - unknown script type" );
    }
    return maHlpChWstrn;
}

// Number formats =============================================================

namespace {

// ----------------------------------------------------------------------------

/** Special number format index describing a reused format. */
const NfIndexTableOffset PRV_NF_INDEX_REUSE = NF_INDEX_TABLE_ENTRIES;

/** German primary language not defined, LANGUAGE_GERMAN belongs to Germany. */
const LanguageType PRV_LANGUAGE_GERMAN_PRIM = LANGUAGE_GERMAN & LANGUAGE_MASK_PRIMARY;
/** French primary language not defined, LANGUAGE_FRENCH belongs to France. */
const LanguageType PRV_LANGUAGE_FRENCH_PRIM = LANGUAGE_FRENCH & LANGUAGE_MASK_PRIMARY;
/** Parent language identifier for Asian languages. */
const LanguageType PRV_LANGUAGE_ASIAN_PRIM = LANGUAGE_CHINESE & LANGUAGE_MASK_PRIMARY;

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
    { nXclNumFmt, pcUtf8, NF_NUMBER_STANDARD, 0 }

/** Defines an Excel built-in number format that maps to an own built-in format. */
#define EXC_NUMFMT_OFFSET( nXclNumFmt, eOffset ) \
    { nXclNumFmt, 0, eOffset, 0 }

/** Defines an Excel built-in number format that is the same as the specified. */
#define EXC_NUMFMT_REUSE( nXclNumFmt, nXclReuse ) \
    { nXclNumFmt, 0, PRV_NF_INDEX_REUSE, nXclReuse }

/** Terminates an Excel built-in number format table. */
#define EXC_NUMFMT_ENDTABLE() \
    { EXC_FORMAT_NOTFOUND, 0, NF_NUMBER_STANDARD, 0 }

// ----------------------------------------------------------------------------

// Currency unit characters
#define UTF8_BAHT       "\340\270\277"
#define UTF8_EURO       "\342\202\254"
#define UTF8_POUND_UK   "\302\243"
#define UTF8_SHEQEL     "\342\202\252"
#define UTF8_WON        "\357\277\246"
#define UTF8_YEN_CS     "\357\277\245"
#define UTF8_YEN_JP     "\302\245"

// Japanese/Chinese date/time characters
#define UTF8_CJ_YEAR    "\345\271\264"
#define UTF8_CJ_MON     "\346\234\210"
#define UTF8_CJ_DAY     "\346\227\245"
#define UTF8_CJ_HOUR    "\346\231\202"
#define UTF8_CJ_MIN     "\345\210\206"
#define UTF8_CJ_SEC     "\347\247\222"

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

    // 50...81 international formats
    EXC_NUMFMT_REUSE(   50, 14 ),
    EXC_NUMFMT_REUSE(   51, 14 ),
    EXC_NUMFMT_REUSE(   52, 14 ),
    EXC_NUMFMT_REUSE(   53, 14 ),
    EXC_NUMFMT_REUSE(   54, 14 ),
    EXC_NUMFMT_REUSE(   55, 14 ),
    EXC_NUMFMT_REUSE(   56, 14 ),
    EXC_NUMFMT_REUSE(   57, 14 ),
    EXC_NUMFMT_REUSE(   58, 14 ),
    EXC_NUMFMT_REUSE(   59, 1 ),
    EXC_NUMFMT_REUSE(   60, 2 ),
    EXC_NUMFMT_REUSE(   61, 3 ),
    EXC_NUMFMT_REUSE(   62, 4 ),
    EXC_NUMFMT_REUSE(   67, 9 ),
    EXC_NUMFMT_REUSE(   68, 10 ),
    EXC_NUMFMT_REUSE(   69, 12 ),
    EXC_NUMFMT_REUSE(   70, 13 ),
    EXC_NUMFMT_REUSE(   71, 14 ),
    EXC_NUMFMT_REUSE(   72, 14 ),
    EXC_NUMFMT_REUSE(   73, 15 ),
    EXC_NUMFMT_REUSE(   74, 16 ),
    EXC_NUMFMT_REUSE(   75, 17 ),
    EXC_NUMFMT_REUSE(   76, 20 ),
    EXC_NUMFMT_REUSE(   77, 21 ),
    EXC_NUMFMT_REUSE(   78, 22 ),
    EXC_NUMFMT_REUSE(   79, 45 ),
    EXC_NUMFMT_REUSE(   80, 46 ),
    EXC_NUMFMT_REUSE(   81, 47 ),

    // 82...163 not used, must not occur in a file (Excel may crash)

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

static const XclBuiltInFormat spBuiltInFormats_ENGLISH_UK[] =
{
    EXC_NUMFMT_STRING(  63, UTF8_POUND_UK "#,##0;-" UTF8_POUND_UK "#,##0" ),
    EXC_NUMFMT_STRING(  64, UTF8_POUND_UK "#,##0;[RED]-" UTF8_POUND_UK "#,##0" ),
    EXC_NUMFMT_STRING(  65, UTF8_POUND_UK "#,##0.00;-" UTF8_POUND_UK "#,##0.00" ),
    EXC_NUMFMT_STRING(  66, UTF8_POUND_UK "#,##0.00;[RED]-" UTF8_POUND_UK "#,##0.00" ),
    EXC_NUMFMT_ENDTABLE()
};

static const XclBuiltInFormat spBuiltInFormats_ENGLISH_EIRE[] =
{
    EXC_NUMFMT_STRING(  63, UTF8_EURO "#,##0;-" UTF8_EURO "#,##0" ),
    EXC_NUMFMT_STRING(  64, UTF8_EURO "#,##0;[RED]-" UTF8_EURO "#,##0" ),
    EXC_NUMFMT_STRING(  65, UTF8_EURO "#,##0.00;-" UTF8_EURO "#,##0.00" ),
    EXC_NUMFMT_STRING(  66, UTF8_EURO "#,##0.00;[RED]-" UTF8_EURO "#,##0.00" ),
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
    EXC_NUMFMT_STRING(  63, "$#,##0_);($#,##0)" ),
    EXC_NUMFMT_STRING(  64, "$#,##0_);[RED]($#,##0)" ),
    EXC_NUMFMT_STRING(  65, "$#,##0.00_);($#,##0.00)" ),
    EXC_NUMFMT_STRING(  66, "$#,##0.00_);[RED]($#,##0.00)" ),
    EXC_NUMFMT_ENDTABLE()
};

static const XclBuiltInFormat spBuiltInFormats_ENGLISH_CAN[] =
{
    EXC_NUMFMT_STRING(  20, "h:mm" ),
    EXC_NUMFMT_STRING(  21, "h:mm:ss" ),
    EXC_NUMFMT_STRING(  22, "DD/MM/YYYY h:mm" ),
    EXC_NUMFMT_STRING(  63, "$#,##0;-$#,##0" ),
    EXC_NUMFMT_STRING(  64, "$#,##0;[RED]-$#,##0" ),
    EXC_NUMFMT_STRING(  65, "$#,##0.00;-$#,##0.00" ),
    EXC_NUMFMT_STRING(  66, "$#,##0.00;[RED]-$#,##0.00" ),
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
    EXC_NUMFMT_STRING(  63, "$#,##0;-$#,##0" ),
    EXC_NUMFMT_STRING(  64, "$#,##0;[RED]-$#,##0" ),
    EXC_NUMFMT_STRING(  65, "$#,##0.00;-$#,##0.00" ),
    EXC_NUMFMT_STRING(  66, "$#,##0.00;[RED]-$#,##0.00" ),
    EXC_NUMFMT_ENDTABLE()
};

static const XclBuiltInFormat spBuiltInFormats_ENGLISH_SAFRICA[] =
{
    EXC_NUMFMT_STRING(  14, "YYYY/MM/DD" ),
    EXC_NUMFMT_OFFSET(  18, NF_TIME_HHMMAMPM ),
    EXC_NUMFMT_OFFSET(  19, NF_TIME_HHMMSSAMPM ),
    EXC_NUMFMT_STRING(  22, "YYYY/MM/DD hh:mm" ),
    EXC_NUMFMT_STRING(  63, "\\R #,##0;\\R -#,##0" ),
    EXC_NUMFMT_STRING(  64, "\\R #,##0;[RED]\\R -#,##0" ),
    EXC_NUMFMT_STRING(  65, "\\R #,##0.00;\\R -#,##0.00" ),
    EXC_NUMFMT_STRING(  66, "\\R #,##0.00;[RED]\\R -#,##0.00" ),
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
    EXC_NUMFMT_STRING(  63, "#,##0\\ " UTF8_EURO ";-#,##0\\ " UTF8_EURO ),
    EXC_NUMFMT_STRING(  64, "#,##0\\ " UTF8_EURO ";[RED]-#,##0\\ " UTF8_EURO ),
    EXC_NUMFMT_STRING(  65, "#,##0.00\\ " UTF8_EURO ";-#,##0.00\\ " UTF8_EURO ),
    EXC_NUMFMT_STRING(  66, "#,##0.00\\ " UTF8_EURO ";[RED]-#,##0.00\\ " UTF8_EURO ),
    EXC_NUMFMT_ENDTABLE()
};

static const XclBuiltInFormat spBuiltInFormats_FRENCH_CANADIAN[] =
{
    EXC_NUMFMT_STRING(  22, "YYYY-MM-DD hh:mm" ),
    EXC_NUMFMT_STRING(  37, "#,##0\\ _$_-;#,##0\\ _$-" ),
    EXC_NUMFMT_STRING(  38, "#,##0\\ _$_-;[RED]#,##0\\ _$-" ),
    EXC_NUMFMT_STRING(  39, "#,##0.00\\ _$_-;#,##0.00\\ _$-" ),
    EXC_NUMFMT_STRING(  40, "#,##0.00\\ _$_-;[RED]#,##0.00\\ _$-" ),
    EXC_NUMFMT_STRING(  63, "#,##0\\ $_-;#,##0\\ $-" ),
    EXC_NUMFMT_STRING(  64, "#,##0\\ $_-;[RED]#,##0\\ $-" ),
    EXC_NUMFMT_STRING(  65, "#,##0.00\\ $_-;#,##0.00\\ $-" ),
    EXC_NUMFMT_STRING(  66, "#,##0.00\\ $_-;[RED]#,##0.00\\ $-" ),
    EXC_NUMFMT_ENDTABLE()
};

static const XclBuiltInFormat spBuiltInFormats_FRENCH_SWISS[] =
{
    EXC_NUMFMT_STRING(  15, "DD.MMM.YY" ),
    EXC_NUMFMT_STRING(  16, "DD.MMM" ),
    EXC_NUMFMT_STRING(  17, "MMM.YY" ),
    EXC_NUMFMT_STRING(  22, "DD.MM.YYYY hh:mm" ),
    EXC_NUMFMT_STRING(  63, "\"SFr. \"#,##0;\"SFr. \"-#,##0" ),
    EXC_NUMFMT_STRING(  64, "\"SFr. \"#,##0;[RED]\"SFr. \"-#,##0" ),
    EXC_NUMFMT_STRING(  65, "\"SFr. \"#,##0.00;\"SFr. \"-#,##0.00" ),
    EXC_NUMFMT_STRING(  66, "\"SFr. \"#,##0.00;[RED]\"SFr. \"-#,##0.00" ),
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
    EXC_NUMFMT_STRING(  63, "#,##0 " UTF8_EURO ";-#,##0 " UTF8_EURO ),
    EXC_NUMFMT_STRING(  64, "#,##0 " UTF8_EURO ";[RED]-#,##0 " UTF8_EURO ),
    EXC_NUMFMT_STRING(  65, "#,##0.00 " UTF8_EURO ";-#,##0.00 " UTF8_EURO ),
    EXC_NUMFMT_STRING(  66, "#,##0.00 " UTF8_EURO ";[RED]-#,##0.00 " UTF8_EURO ),
    EXC_NUMFMT_ENDTABLE()
};

static const XclBuiltInFormat spBuiltInFormats_GERMAN_AUSTRIAN[] =
{
    EXC_NUMFMT_STRING(  15, "DD.MMM.YY" ),
    EXC_NUMFMT_STRING(  16, "DD.MMM" ),
    EXC_NUMFMT_STRING(  17, "MMM.YY" ),
    EXC_NUMFMT_STRING(  63, UTF8_EURO " #,##0;-" UTF8_EURO " #,##0" ),
    EXC_NUMFMT_STRING(  64, UTF8_EURO " #,##0;[RED]-" UTF8_EURO " #,##0" ),
    EXC_NUMFMT_STRING(  65, UTF8_EURO " #,##0.00;-" UTF8_EURO " #,##0.00" ),
    EXC_NUMFMT_STRING(  66, UTF8_EURO " #,##0.00;[RED]-" UTF8_EURO " #,##0.00" ),
    EXC_NUMFMT_ENDTABLE()
};

static const XclBuiltInFormat spBuiltInFormats_GERMAN_SWISS[] =
{
    EXC_NUMFMT_STRING(  63, "\"SFr. \"#,##0;\"SFr. \"-#,##0" ),
    EXC_NUMFMT_STRING(  64, "\"SFr. \"#,##0;[RED]\"SFr. \"-#,##0" ),
    EXC_NUMFMT_STRING(  65, "\"SFr. \"#,##0.00;\"SFr. \"-#,##0.00" ),
    EXC_NUMFMT_STRING(  66, "\"SFr. \"#,##0.00;[RED]\"SFr. \"-#,##0.00" ),
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
    EXC_NUMFMT_STRING(  63, "#,##0 " UTF8_EURO ";-#,##0 " UTF8_EURO ),
    EXC_NUMFMT_STRING(  64, "#,##0 " UTF8_EURO ";[RED]-#,##0 " UTF8_EURO ),
    EXC_NUMFMT_STRING(  65, "#,##0.00 " UTF8_EURO ";-#,##0.00 " UTF8_EURO ),
    EXC_NUMFMT_STRING(  66, "#,##0.00 " UTF8_EURO ";[RED]-#,##0.00 " UTF8_EURO ),
    EXC_NUMFMT_ENDTABLE()
};

static const XclBuiltInFormat spBuiltInFormats_GERMAN_LIECHTENSTEIN[] =
{
    EXC_NUMFMT_STRING(  63, "\"CHF \"#,##0;\"CHF \"-#,##0" ),
    EXC_NUMFMT_STRING(  64, "\"CHF \"#,##0;[RED]\"CHF \"-#,##0" ),
    EXC_NUMFMT_STRING(  65, "\"CHF \"#,##0.00;\"CHF \"-#,##0.00" ),
    EXC_NUMFMT_STRING(  66, "\"CHF \"#,##0.00;[RED]\"CHF \"-#,##0.00" ),
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
    EXC_NUMFMT_STRING(  63, UTF8_EURO " #,##0;-" UTF8_EURO " #,##0" ),
    EXC_NUMFMT_STRING(  64, UTF8_EURO " #,##0;[RED]-" UTF8_EURO " #,##0" ),
    EXC_NUMFMT_STRING(  65, UTF8_EURO " #,##0.00;-" UTF8_EURO " #,##0.00" ),
    EXC_NUMFMT_STRING(  66, UTF8_EURO " #,##0.00;[RED]-" UTF8_EURO " #,##0.00" ),
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
    EXC_NUMFMT_STRING(  63, "\"SFr. \"#,##0;\"SFr. \"-#,##0" ),
    EXC_NUMFMT_STRING(  64, "\"SFr. \"#,##0;[RED]\"SFr. \"-#,##0" ),
    EXC_NUMFMT_STRING(  65, "\"SFr. \"#,##0.00;\"SFr. \"-#,##0.00" ),
    EXC_NUMFMT_STRING(  66, "\"SFr. \"#,##0.00;[RED]\"SFr. \"-#,##0.00" ),
    EXC_NUMFMT_ENDTABLE()
};

// SWEDISH --------------------------------------------------------------------

static const XclBuiltInFormat spBuiltInFormats_SWEDISH_SWEDEN[] =
{
    EXC_NUMFMT_STRING(  15, "DD-MMM-YY" ),
    EXC_NUMFMT_STRING(  16, "DD-MMM" ),
    EXC_NUMFMT_STRING(  17, "MMM-YY" ),
    EXC_NUMFMT_STRING(  18, "h:mm AM/PM" ),
    EXC_NUMFMT_STRING(  19, "h:mm:ss AM/PM" ),
    EXC_NUMFMT_STRING(  22, "YYYY-MM-DD hh:mm" ),
    EXC_NUMFMT_STRING(  37, "#,##0 _k_r;-#,##0 _k_r" ),
    EXC_NUMFMT_STRING(  38, "#,##0 _k_r;[RED]-#,##0 _k_r" ),
    EXC_NUMFMT_STRING(  39, "#,##0.00 _k_r;-#,##0.00 _k_r" ),
    EXC_NUMFMT_STRING(  40, "#,##0.00 _k_r;[RED]-#,##0.00 _k_r" ),
    EXC_NUMFMT_STRING(  63, "#,##0 \"kr\";-#,##0 \"kr\"" ),
    EXC_NUMFMT_STRING(  64, "#,##0 \"kr\";[RED]-#,##0 \"kr\"" ),
    EXC_NUMFMT_STRING(  65, "#,##0.00 \"kr\";-#,##0.00 \"kr\"" ),
    EXC_NUMFMT_STRING(  66, "#,##0.00 \"kr\";[RED]-#,##0.00 \"kr\"" ),
    EXC_NUMFMT_ENDTABLE()
};

static const XclBuiltInFormat spBuiltInFormats_SWEDISH_FINLAND[] =
{
    EXC_NUMFMT_STRING(   9, "0 %" ),
    EXC_NUMFMT_STRING(  10, "0.00 %" ),
    EXC_NUMFMT_STRING(  15, "DD.MMM.YY" ),
    EXC_NUMFMT_STRING(  16, "DD.MMM" ),
    EXC_NUMFMT_STRING(  17, "MMM.YY" ),
    EXC_NUMFMT_STRING(  18, "h:mm AM/PM" ),
    EXC_NUMFMT_STRING(  19, "h:mm:ss AM/PM" ),
    EXC_NUMFMT_STRING(  22, "D.M.YYYY hh:mm" ),
    EXC_NUMFMT_STRING(  37, "#,##0 _" UTF8_EURO ";-#,##0 _" UTF8_EURO ),
    EXC_NUMFMT_STRING(  38, "#,##0 _" UTF8_EURO ";[RED]-#,##0 _" UTF8_EURO ),
    EXC_NUMFMT_STRING(  39, "#,##0.00 _" UTF8_EURO ";-#,##0.00 _" UTF8_EURO ),
    EXC_NUMFMT_STRING(  40, "#,##0.00 _" UTF8_EURO ";[RED]-#,##0.00 _" UTF8_EURO ),
    EXC_NUMFMT_STRING(  63, "#,##0 " UTF8_EURO ";-#,##0 " UTF8_EURO ),
    EXC_NUMFMT_STRING(  64, "#,##0 " UTF8_EURO ";[RED]-#,##0 " UTF8_EURO ),
    EXC_NUMFMT_STRING(  65, "#,##0.00 " UTF8_EURO ";-#,##0.00 " UTF8_EURO ),
    EXC_NUMFMT_STRING(  66, "#,##0.00 " UTF8_EURO ";[RED]-#,##0.00 " UTF8_EURO ),
    EXC_NUMFMT_ENDTABLE()
};

// ASIAN ----------------------------------------------------------------------

/** Base table for Asian locales. */
static const XclBuiltInFormat spBuiltInFormats_ASIAN[] =
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
    EXC_NUMFMT_STRING(  28, "[$-0411]GGGE" UTF8_CJ_YEAR "M" UTF8_CJ_MON "D" UTF8_CJ_DAY ),
    EXC_NUMFMT_STRING(  30, "[$-0411]M/D/YY" ),
    EXC_NUMFMT_STRING(  31, "[$-0411]YYYY" UTF8_CJ_YEAR "M" UTF8_CJ_MON "D" UTF8_CJ_DAY ),
    EXC_NUMFMT_STRING(  32, "[$-0411]h" UTF8_CJ_HOUR "mm" UTF8_CJ_MIN ),
    EXC_NUMFMT_STRING(  33, "[$-0411]h" UTF8_CJ_HOUR "mm" UTF8_CJ_MIN "ss" UTF8_CJ_SEC ),
    EXC_NUMFMT_STRING(  34, "[$-0411]YYYY" UTF8_CJ_YEAR "M" UTF8_CJ_MON ),
    EXC_NUMFMT_STRING(  35, "[$-0411]M" UTF8_CJ_MON "D" UTF8_CJ_DAY ),
    EXC_NUMFMT_STRING(  63, UTF8_YEN_JP "#,##0;-" UTF8_YEN_JP "#,##0" ),
    EXC_NUMFMT_STRING(  64, UTF8_YEN_JP "#,##0;[RED]-" UTF8_YEN_JP "#,##0" ),
    EXC_NUMFMT_STRING(  65, UTF8_YEN_JP "#,##0.00;-" UTF8_YEN_JP "#,##0.00" ),
    EXC_NUMFMT_STRING(  66, UTF8_YEN_JP "#,##0.00;[RED]-" UTF8_YEN_JP "#,##0.00" ),
    EXC_NUMFMT_ENDTABLE()
};

static const XclBuiltInFormat spBuiltInFormats_KOREAN[] =
{
    EXC_NUMFMT_STRING(  14, "YYYY-MM-DD" ),
    EXC_NUMFMT_STRING(  15, "DD-MMM-YY" ),
    EXC_NUMFMT_STRING(  16, "DD-MMM" ),
    EXC_NUMFMT_STRING(  17, "MMM-YY" ),
    EXC_NUMFMT_STRING(  22, "YYYY-MM-DD h:mm" ),
    EXC_NUMFMT_STRING(  27, "[$-0412]YYYY" UTF8_CJ_YEAR " MM" UTF8_CJ_MON " DD" UTF8_CJ_DAY ),
    EXC_NUMFMT_STRING(  28, "[$-0412]MM-DD" ),
    EXC_NUMFMT_STRING(  30, "[$-0412]MM-DD-YY" ),
    EXC_NUMFMT_STRING(  31, "[$-0412]YYYY" UTF8_KO_YEAR " MM" UTF8_KO_MON " DD" UTF8_KO_DAY ),
    EXC_NUMFMT_STRING(  32, "[$-0412]h" UTF8_KO_HOUR " mm" UTF8_KO_MIN ),
    EXC_NUMFMT_STRING(  33, "[$-0412]h" UTF8_KO_HOUR " mm" UTF8_KO_MIN " ss" UTF8_KO_SEC ),
    EXC_NUMFMT_STRING(  34, "[$-0412]YYYY\"/\"MM\"/\"DD" ),
    EXC_NUMFMT_STRING(  35, "[$-0412]YYYY-MM-DD" ),
    EXC_NUMFMT_STRING(  63, UTF8_WON "#,##0;-" UTF8_WON "#,##0" ),
    EXC_NUMFMT_STRING(  64, UTF8_WON "#,##0;[RED]-" UTF8_WON "#,##0" ),
    EXC_NUMFMT_STRING(  65, UTF8_WON "#,##0.00;-" UTF8_WON "#,##0.00" ),
    EXC_NUMFMT_STRING(  66, UTF8_WON "#,##0.00;[RED]-" UTF8_WON "#,##0.00" ),
    EXC_NUMFMT_ENDTABLE()
};

static const XclBuiltInFormat spBuiltInFormats_CHINESE_SIMPLIFIED[] =
{
    EXC_NUMFMT_STRING(  14, "YYYY-M-D" ),
    EXC_NUMFMT_STRING(  15, "D-MMM-YY" ),
    EXC_NUMFMT_STRING(  16, "D-MMM" ),
    EXC_NUMFMT_STRING(  17, "MMM-YY" ),
    EXC_NUMFMT_STRING(  22, "YYYY-M-D h:mm" ),
    EXC_NUMFMT_STRING(  27, "[$-0804]YYYY" UTF8_CJ_YEAR "M" UTF8_CJ_MON ),
    EXC_NUMFMT_STRING(  28, "[$-0804]M" UTF8_CJ_MON "D" UTF8_CJ_DAY ),
    EXC_NUMFMT_STRING(  30, "[$-0804]M-D-YY" ),
    EXC_NUMFMT_STRING(  31, "[$-0804]YYYY" UTF8_CJ_YEAR "M" UTF8_CJ_MON "D" UTF8_CJ_DAY ),
    EXC_NUMFMT_STRING(  32, "[$-0804]h" UTF8_CS_HOUR "mm" UTF8_CJ_MIN ),
    EXC_NUMFMT_STRING(  33, "[$-0804]h" UTF8_CS_HOUR "mm" UTF8_CJ_MIN "ss" UTF8_CJ_SEC ),
    EXC_NUMFMT_STRING(  34, "[$-0804]AM/PMh" UTF8_CS_HOUR "mm" UTF8_CJ_MIN ),
    EXC_NUMFMT_STRING(  35, "[$-0804]AM/PMh" UTF8_CS_HOUR "mm" UTF8_CJ_MIN "ss" UTF8_CJ_SEC ),
    EXC_NUMFMT_REUSE(   52, 27 ),
    EXC_NUMFMT_REUSE(   53, 28 ),
    EXC_NUMFMT_STRING(  63, UTF8_YEN_CS "#,##0;-" UTF8_YEN_CS "#,##0" ),
    EXC_NUMFMT_STRING(  64, UTF8_YEN_CS "#,##0;[RED]-" UTF8_YEN_CS "#,##0" ),
    EXC_NUMFMT_STRING(  65, UTF8_YEN_CS "#,##0.00;-" UTF8_YEN_CS "#,##0.00" ),
    EXC_NUMFMT_STRING(  66, UTF8_YEN_CS "#,##0.00;[RED]-" UTF8_YEN_CS "#,##0.00" ),
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
    EXC_NUMFMT_STRING(  28, "[$-0404]E" UTF8_CJ_YEAR "M" UTF8_CJ_MON "D" UTF8_CJ_DAY ),
    EXC_NUMFMT_STRING(  30, "[$-0404]M/D/YY" ),
    EXC_NUMFMT_STRING(  31, "[$-0404]YYYY" UTF8_CJ_YEAR "M" UTF8_CJ_MON "D" UTF8_CJ_DAY ),
    EXC_NUMFMT_STRING(  32, "[$-0404]hh" UTF8_CJ_HOUR "mm" UTF8_CJ_MIN ),
    EXC_NUMFMT_STRING(  33, "[$-0404]hh" UTF8_CJ_HOUR "mm" UTF8_CJ_MIN "ss" UTF8_CJ_SEC ),
    EXC_NUMFMT_STRING(  34, "[$-0404]AM/PMhh" UTF8_CJ_HOUR "mm" UTF8_CJ_MIN ),
    EXC_NUMFMT_STRING(  35, "[$-0404]AM/PMhh" UTF8_CJ_HOUR "mm" UTF8_CJ_MIN "ss" UTF8_CJ_SEC ),
    EXC_NUMFMT_STRING(  63, "$#,##0;-$#,##0" ),
    EXC_NUMFMT_STRING(  64, "$#,##0;[RED]-$#,##0" ),
    EXC_NUMFMT_STRING(  65, "$#,##0.00;-$#,##0.00" ),
    EXC_NUMFMT_STRING(  66, "$#,##0.00;[RED]-$#,##0.00" ),
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
    EXC_NUMFMT_STRING(  63, UTF8_SHEQEL " #,##0;" UTF8_SHEQEL " -#,##0" ),
    EXC_NUMFMT_STRING(  64, UTF8_SHEQEL " #,##0;[RED]" UTF8_SHEQEL " -#,##0" ),
    EXC_NUMFMT_STRING(  65, UTF8_SHEQEL " #,##0.00;" UTF8_SHEQEL " -#,##0.00" ),
    EXC_NUMFMT_STRING(  66, UTF8_SHEQEL " #,##0.00;[RED]" UTF8_SHEQEL " -#,##0.00" ),
    EXC_NUMFMT_ENDTABLE()
};

static const XclBuiltInFormat spBuiltInFormats_THAI[] =
{
    EXC_NUMFMT_STRING(  14, "D/M/YYYY" ),
    EXC_NUMFMT_STRING(  15, "D-MMM-YY" ),
    EXC_NUMFMT_STRING(  16, "D-MMM" ),
    EXC_NUMFMT_STRING(  17, "MMM-YY" ),
    EXC_NUMFMT_STRING(  18, "h:mm AM/PM" ),
    EXC_NUMFMT_STRING(  19, "h:mm:ss AM/PM" ),
    EXC_NUMFMT_STRING(  22, "D/M/YYYY h:mm" ),
    EXC_NUMFMT_STRING(  59, "t0" ),
    EXC_NUMFMT_STRING(  60, "t0.00" ),
    EXC_NUMFMT_STRING(  61, "t#,##0" ),
    EXC_NUMFMT_STRING(  62, "t#,##0.00" ),
    EXC_NUMFMT_STRING(  63, "t" UTF8_BAHT "#,##0_);t(" UTF8_BAHT "#,##0)" ),
    EXC_NUMFMT_STRING(  64, "t" UTF8_BAHT "#,##0_);[RED]t(" UTF8_BAHT "#,##0)" ),
    EXC_NUMFMT_STRING(  65, "t" UTF8_BAHT "#,##0.00_);t(" UTF8_BAHT "#,##0.00)" ),
    EXC_NUMFMT_STRING(  66, "t" UTF8_BAHT "#,##0.00_);[RED]t(" UTF8_BAHT "#,##0.00)" ),
    EXC_NUMFMT_STRING(  67, "t0%" ),
    EXC_NUMFMT_STRING(  68, "t0.00%" ),
    EXC_NUMFMT_STRING(  69, "t# ?/?" ),
    EXC_NUMFMT_STRING(  70, "t# ?\?/?\?" ),
    EXC_NUMFMT_STRING(  71, "tD/M/EE" ),
    EXC_NUMFMT_STRING(  72, "tD-MMM-E" ),
    EXC_NUMFMT_STRING(  73, "tD-MMM" ),
    EXC_NUMFMT_STRING(  74, "tMMM-E" ),
    EXC_NUMFMT_STRING(  75, "th:mm" ),
    EXC_NUMFMT_STRING(  76, "th:mm:ss" ),
    EXC_NUMFMT_STRING(  77, "tD/M/EE h:mm" ),
    EXC_NUMFMT_STRING(  78, "tmm:ss" ),
    EXC_NUMFMT_STRING(  79, "t[h]:mm:ss" ),
    EXC_NUMFMT_STRING(  80, "tmm:ss.0" ),
    EXC_NUMFMT_STRING(  81, "D/M/E" ),
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
    {   LANGUAGE_ENGLISH_UK,            LANGUAGE_ENGLISH,           spBuiltInFormats_ENGLISH_UK             },
    {   LANGUAGE_ENGLISH_EIRE,          LANGUAGE_ENGLISH,           spBuiltInFormats_ENGLISH_EIRE           },
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
    {   LANGUAGE_GERMAN_SWISS,          PRV_LANGUAGE_GERMAN_PRIM,   spBuiltInFormats_GERMAN_SWISS           },
    {   LANGUAGE_GERMAN_LUXEMBOURG,     PRV_LANGUAGE_GERMAN_PRIM,   spBuiltInFormats_GERMAN_LUXEMBOURG      },
    {   LANGUAGE_GERMAN_LIECHTENSTEIN,  PRV_LANGUAGE_GERMAN_PRIM,   spBuiltInFormats_GERMAN_LIECHTENSTEIN   },

    {   LANGUAGE_ITALIAN,               LANGUAGE_DONTKNOW,          spBuiltInFormats_ITALIAN_ITALY          },
    {   LANGUAGE_ITALIAN_SWISS,         LANGUAGE_DONTKNOW,          spBuiltInFormats_ITALIAN_SWISS          },

    {   LANGUAGE_SWEDISH,               LANGUAGE_DONTKNOW,          spBuiltInFormats_SWEDISH_SWEDEN         },
    {   LANGUAGE_SWEDISH_FINLAND,       LANGUAGE_DONTKNOW,          spBuiltInFormats_SWEDISH_FINLAND        },

    {   PRV_LANGUAGE_ASIAN_PRIM,        LANGUAGE_DONTKNOW,          spBuiltInFormats_ASIAN                  },
    {   LANGUAGE_JAPANESE,              PRV_LANGUAGE_ASIAN_PRIM,    spBuiltInFormats_JAPANESE               },
    {   LANGUAGE_KOREAN,                PRV_LANGUAGE_ASIAN_PRIM,    spBuiltInFormats_KOREAN                 },
    {   LANGUAGE_CHINESE_SIMPLIFIED,    PRV_LANGUAGE_ASIAN_PRIM,    spBuiltInFormats_CHINESE_SIMPLIFIED     },
    {   LANGUAGE_CHINESE_TRADITIONAL,   PRV_LANGUAGE_ASIAN_PRIM,    spBuiltInFormats_CHINESE_TRADITIONAL    },

    {   LANGUAGE_HEBREW,                LANGUAGE_DONTKNOW,          spBuiltInFormats_HEBREW                 },
    {   LANGUAGE_THAI,                  LANGUAGE_DONTKNOW,          spBuiltInFormats_THAI                   }
};

// ----------------------------------------------------------------------------

} // namespace

// ============================================================================

XclNumFmtBuffer::XclNumFmtBuffer( const XclRoot& rRoot ) :
    meSysLang( rRoot.GetSysLanguage() ),
    mnStdScNumFmt( rRoot.GetFormatter().GetStandardFormat( ScGlobal::eLnge ) )
{
    // *** insert default formats (BIFF5+ only)***

    if( rRoot.GetBiff() >= EXC_BIFF5 )
        InsertBuiltinFormats();
}

void XclNumFmtBuffer::InitializeImport()
{
    maFmtMap.clear();
}

void XclNumFmtBuffer::InsertFormat( sal_uInt16 nXclNumFmt, const String& rFormat )
{
    XclNumFmt& rNumFmt = maFmtMap[ nXclNumFmt ];
    rNumFmt.maFormat = rFormat;
    // #i62053# rFormat may be an empty string, meOffset must be initialized
    rNumFmt.meOffset = NF_NUMBER_STANDARD;
    rNumFmt.meLanguage = LANGUAGE_SYSTEM;
}

void XclNumFmtBuffer::InsertBuiltinFormats()
{
    // build a map containing tables for all languages
    typedef ::std::map< LanguageType, const XclBuiltInFormatTable* > XclBuiltInMap;
    XclBuiltInMap aBuiltInMap;
    for( const XclBuiltInFormatTable* pTable = spBuiltInFormatTables;
            pTable != STATIC_ARRAY_END( spBuiltInFormatTables ); ++pTable )
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
        OSL_TRACE( "XclNumFmtBuffer::InsertBuiltinFormats - language 0x%04hX not supported (#i29949#)", meSysLang );
        XclBuiltInMap::const_iterator aMIt = aBuiltInMap.find( LANGUAGE_DONTKNOW );
        OSL_ENSURE( aMIt != aBuiltInMap.end(), "XclNumFmtBuffer::InsertBuiltinFormats - default map not found" );
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
        for( const XclBuiltInFormat* pBuiltIn = (*aVIt)->mpFormats; pBuiltIn && (pBuiltIn->mnXclNumFmt != EXC_FORMAT_NOTFOUND); ++pBuiltIn )
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
    mnHorAlign( EXC_XF_HOR_GENERAL ),
    mnVerAlign( EXC_XF_VER_BOTTOM ),
    mnOrient( EXC_ORIENT_NONE ),
    mnTextDir( EXC_XF_TEXTDIR_CONTEXT ),
    mnRotation( EXC_ROT_NONE ),
    mnIndent( 0 ),
    mbLineBreak( false ),
    mbShrink( false )
{
}

SvxCellHorJustify XclCellAlign::GetScHorAlign() const
{
    SvxCellHorJustify eHorJust = SVX_HOR_JUSTIFY_STANDARD;
    switch( mnHorAlign )
    {
        case EXC_XF_HOR_GENERAL:    eHorJust = SVX_HOR_JUSTIFY_STANDARD;    break;
        case EXC_XF_HOR_LEFT:       eHorJust = SVX_HOR_JUSTIFY_LEFT;        break;
        case EXC_XF_HOR_CENTER_AS:
        case EXC_XF_HOR_CENTER:     eHorJust = SVX_HOR_JUSTIFY_CENTER;      break;
        case EXC_XF_HOR_RIGHT:      eHorJust = SVX_HOR_JUSTIFY_RIGHT;       break;
        case EXC_XF_HOR_FILL:       eHorJust = SVX_HOR_JUSTIFY_REPEAT;      break;
        case EXC_XF_HOR_JUSTIFY:
        case EXC_XF_HOR_DISTRIB:    eHorJust = SVX_HOR_JUSTIFY_BLOCK;       break;
        default:    OSL_FAIL( "XclCellAlign::GetScHorAlign - unknown horizontal alignment" );
    }
    return eHorJust;
}

SvxCellJustifyMethod XclCellAlign::GetScHorJustifyMethod() const
{
    return (mnHorAlign == EXC_XF_HOR_DISTRIB) ? SVX_JUSTIFY_METHOD_DISTRIBUTE : SVX_JUSTIFY_METHOD_AUTO;
}

SvxCellVerJustify XclCellAlign::GetScVerAlign() const
{
    SvxCellVerJustify eVerJust = SVX_VER_JUSTIFY_STANDARD;
    switch( mnVerAlign )
    {
        case EXC_XF_VER_TOP:        eVerJust = SVX_VER_JUSTIFY_TOP;         break;
        case EXC_XF_VER_CENTER:     eVerJust = SVX_VER_JUSTIFY_CENTER;      break;
        case EXC_XF_VER_BOTTOM:     eVerJust = SVX_VER_JUSTIFY_STANDARD;    break;
        case EXC_XF_VER_JUSTIFY:
        case EXC_XF_VER_DISTRIB:    eVerJust = SVX_VER_JUSTIFY_BLOCK;       break;
        default:    OSL_FAIL( "XclCellAlign::GetScVerAlign - unknown vertical alignment" );
    }
    return eVerJust;
}

SvxCellJustifyMethod XclCellAlign::GetScVerJustifyMethod() const
{
    return (mnVerAlign == EXC_XF_VER_DISTRIB) ? SVX_JUSTIFY_METHOD_DISTRIBUTE : SVX_JUSTIFY_METHOD_AUTO;
}

SvxFrameDirection XclCellAlign::GetScFrameDir() const
{
    SvxFrameDirection eFrameDir = FRMDIR_ENVIRONMENT;
    switch( mnTextDir )
    {
        case EXC_XF_TEXTDIR_CONTEXT:    eFrameDir = FRMDIR_ENVIRONMENT;     break;
        case EXC_XF_TEXTDIR_LTR:        eFrameDir = FRMDIR_HORI_LEFT_TOP;   break;
        case EXC_XF_TEXTDIR_RTL:        eFrameDir = FRMDIR_HORI_RIGHT_TOP;  break;
        default:    OSL_FAIL( "XclCellAlign::GetScFrameDir - unknown CTL text direction" );
    }
    return eFrameDir;
}

void XclCellAlign::SetScHorAlign( SvxCellHorJustify eHorJust )
{
    switch( eHorJust )
    {
        case SVX_HOR_JUSTIFY_STANDARD:  mnHorAlign = EXC_XF_HOR_GENERAL;    break;
        case SVX_HOR_JUSTIFY_LEFT:      mnHorAlign = EXC_XF_HOR_LEFT;       break;
        case SVX_HOR_JUSTIFY_CENTER:    mnHorAlign = EXC_XF_HOR_CENTER;     break;
        case SVX_HOR_JUSTIFY_RIGHT:     mnHorAlign = EXC_XF_HOR_RIGHT;      break;
        case SVX_HOR_JUSTIFY_BLOCK:     mnHorAlign = EXC_XF_HOR_JUSTIFY;    break;
        case SVX_HOR_JUSTIFY_REPEAT:    mnHorAlign = EXC_XF_HOR_FILL;       break;
        default:                        mnHorAlign = EXC_XF_HOR_GENERAL;
            OSL_FAIL( "XclCellAlign::SetScHorAlign - unknown horizontal alignment" );
    }
}

void XclCellAlign::SetScVerAlign( SvxCellVerJustify eVerJust )
{
    switch( eVerJust )
    {
        case SVX_VER_JUSTIFY_STANDARD:  mnVerAlign = EXC_XF_VER_BOTTOM; break;
        case SVX_VER_JUSTIFY_TOP:       mnVerAlign = EXC_XF_VER_TOP;    break;
        case SVX_VER_JUSTIFY_CENTER:    mnVerAlign = EXC_XF_VER_CENTER; break;
        case SVX_VER_JUSTIFY_BOTTOM:    mnVerAlign = EXC_XF_VER_BOTTOM; break;
        default:                        mnVerAlign = EXC_XF_VER_BOTTOM;
            OSL_FAIL( "XclCellAlign::SetScVerAlign - unknown vertical alignment" );
    }
}

void XclCellAlign::SetScFrameDir( SvxFrameDirection eFrameDir )
{
    switch( eFrameDir )
    {
        case FRMDIR_ENVIRONMENT:    mnTextDir = EXC_XF_TEXTDIR_CONTEXT; break;
        case FRMDIR_HORI_LEFT_TOP:  mnTextDir = EXC_XF_TEXTDIR_LTR;     break;
        case FRMDIR_HORI_RIGHT_TOP: mnTextDir = EXC_XF_TEXTDIR_RTL;     break;
        default:                    mnTextDir = EXC_XF_TEXTDIR_CONTEXT;
            OSL_FAIL( "XclCellAlign::SetScFrameDir - unknown CTL text direction" );
    }
}

bool operator==( const XclCellAlign& rLeft, const XclCellAlign& rRight )
{
    return
        (rLeft.mnHorAlign  == rRight.mnHorAlign)  && (rLeft.mnVerAlign == rRight.mnVerAlign) &&
        (rLeft.mnTextDir   == rRight.mnTextDir)   && (rLeft.mnOrient   == rRight.mnOrient)   &&
        (rLeft.mnRotation  == rRight.mnRotation)  && (rLeft.mnIndent   == rRight.mnIndent)   &&
        (rLeft.mbLineBreak == rRight.mbLineBreak) && (rLeft.mbShrink   == rRight.mbShrink);
}

// ----------------------------------------------------------------------------

XclCellBorder::XclCellBorder() :
    mnLeftColor( 0 ),
    mnRightColor( 0 ),
    mnTopColor( 0 ),
    mnBottomColor( 0 ),
    mnDiagColor( 0 ),
    mnLeftLine( EXC_LINE_NONE ),
    mnRightLine( EXC_LINE_NONE ),
    mnTopLine( EXC_LINE_NONE ),
    mnBottomLine( EXC_LINE_NONE ),
    mnDiagLine( EXC_LINE_NONE ),
    mbDiagTLtoBR( false ),
    mbDiagBLtoTR( false )
{
}

bool operator==( const XclCellBorder& rLeft, const XclCellBorder& rRight )
{
    return
        (rLeft.mnLeftColor  == rRight.mnLeftColor)  && (rLeft.mnRightColor  == rRight.mnRightColor)  &&
        (rLeft.mnTopColor   == rRight.mnTopColor)   && (rLeft.mnBottomColor == rRight.mnBottomColor) &&
        (rLeft.mnLeftLine   == rRight.mnLeftLine)   && (rLeft.mnRightLine   == rRight.mnRightLine)   &&
        (rLeft.mnTopLine    == rRight.mnTopLine)    && (rLeft.mnBottomLine  == rRight.mnBottomLine)  &&
        (rLeft.mnDiagColor  == rRight.mnDiagColor)  && (rLeft.mnDiagLine    == rRight.mnDiagLine)    &&
        (rLeft.mbDiagTLtoBR == rRight.mbDiagTLtoBR) && (rLeft.mbDiagBLtoTR  == rRight.mbDiagBLtoTR);
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

XclXFBase::~XclXFBase()
{
}

void XclXFBase::SetAllUsedFlags( bool bUsed )
{
    mbProtUsed = mbFontUsed = mbFmtUsed = mbAlignUsed = mbBorderUsed = mbAreaUsed = bUsed;
}

bool XclXFBase::HasUsedFlags() const
{
    return mbProtUsed || mbFontUsed || mbFmtUsed || mbAlignUsed || mbBorderUsed || mbAreaUsed;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
