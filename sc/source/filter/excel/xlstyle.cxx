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

#include <xlstyle.hxx>
#include <com/sun/star/awt/FontFamily.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/font.hxx>
#include <sal/macros.h>
#include <sal/log.hxx>
#include <rtl/tencinfo.h>
#include <svl/numformat.hxx>
#include <svtools/colorcfg.hxx>
#include <vcl/unohelp.hxx>
#include <editeng/svxfont.hxx>
#include <o3tl/unit_conversion.hxx>
#include <global.hxx>
#include <xlroot.hxx>
#include <xltools.hxx>
// Color data =================================================================

/** Standard EGA colors, bright. */
#define EXC_PALETTE_EGA_COLORS_LIGHT \
            Color(0x000000), Color(0xFFFFFF), Color(0xFF0000), Color(0x00FF00), Color(0x0000FF), Color(0xFFFF00), Color(0xFF00FF), Color(0x00FFFF)
/** Standard EGA colors, dark. */
#define EXC_PALETTE_EGA_COLORS_DARK \
            Color(0x800000), Color(0x008000), Color(0x000080), Color(0x808000), Color(0x800080), Color(0x008080), Color(0xC0C0C0), Color(0x808080)

/** Default color table for BIFF2. */
const Color spnDefColorTable2[] =
{
/*  0 */    EXC_PALETTE_EGA_COLORS_LIGHT
};

/** Default color table for BIFF3/BIFF4. */
const Color spnDefColorTable3[] =
{
/*  0 */    EXC_PALETTE_EGA_COLORS_LIGHT,
/*  8 */    EXC_PALETTE_EGA_COLORS_LIGHT,
/* 16 */    EXC_PALETTE_EGA_COLORS_DARK
};

/** Default color table for BIFF5/BIFF7. */
const Color spnDefColorTable5[] =
{
/*  0 */    EXC_PALETTE_EGA_COLORS_LIGHT,
/*  8 */    EXC_PALETTE_EGA_COLORS_LIGHT,
/* 16 */    EXC_PALETTE_EGA_COLORS_DARK,
/* 24 */    Color(0x8080FF), Color(0x802060), Color(0xFFFFC0), Color(0xA0E0E0), Color(0x600080), Color(0xFF8080), Color(0x0080C0), Color(0xC0C0FF),
/* 32 */    Color(0x000080), Color(0xFF00FF), Color(0xFFFF00), Color(0x00FFFF), Color(0x800080), Color(0x800000), Color(0x008080), Color(0x0000FF),
/* 40 */    Color(0x00CFFF), Color(0x69FFFF), Color(0xE0FFE0), Color(0xFFFF80), Color(0xA6CAF0), Color(0xDD9CB3), Color(0xB38FEE), Color(0xE3E3E3),
/* 48 */    Color(0x2A6FF9), Color(0x3FB8CD), Color(0x488436), Color(0x958C41), Color(0x8E5E42), Color(0xA0627A), Color(0x624FAC), Color(0x969696),
/* 56 */    Color(0x1D2FBE), Color(0x286676), Color(0x004500), Color(0x453E01), Color(0x6A2813), Color(0x85396A), Color(0x4A3285), Color(0x424242)
};

/** Default color table for BIFF8. */
const Color spnDefColorTable8[] =
{
/*  0 */    EXC_PALETTE_EGA_COLORS_LIGHT,
/*  8 */    EXC_PALETTE_EGA_COLORS_LIGHT,
/* 16 */    EXC_PALETTE_EGA_COLORS_DARK,
/* 24 */    Color(0x9999FF), Color(0x993366), Color(0xFFFFCC), Color(0xCCFFFF), Color(0x660066), Color(0xFF8080), Color(0x0066CC), Color(0xCCCCFF),
/* 32 */    Color(0x000080), Color(0xFF00FF), Color(0xFFFF00), Color(0x00FFFF), Color(0x800080), Color(0x800000), Color(0x008080), Color(0x0000FF),
/* 40 */    Color(0x00CCFF), Color(0xCCFFFF), Color(0xCCFFCC), Color(0xFFFF99), Color(0x99CCFF), Color(0xFF99CC), Color(0xCC99FF), Color(0xFFCC99),
/* 48 */    Color(0x3366FF), Color(0x33CCCC), Color(0x99CC00), Color(0xFFCC00), Color(0xFF9900), Color(0xFF6600), Color(0x666699), Color(0x969696),
/* 56 */    Color(0x003366), Color(0x339966), Color(0x003300), Color(0x333300), Color(0x993300), Color(0x993366), Color(0x333399), Color(0x333333)
};

#undef EXC_PALETTE_EGA_COLORS_LIGHT
#undef EXC_PALETTE_EGA_COLORS_DARK

XclDefaultPalette::XclDefaultPalette( const XclRoot& rRoot ) :
    mpnColorTable( nullptr ),
    mnTableSize( 0 )
{
    const StyleSettings& rSett = Application::GetSettings().GetStyleSettings();
    mnFaceColor = rSett.GetFaceColor();
    // Don't use the system HelpBack and HelpText colours as it causes problems
    // with modern gnome. This is because mnNoteText and mnNoteBack are used
    // when colour indices ( instead of real colours ) are specified.
    // Note: That this it is not an unusual scenario that we get the Note
    // background specified as a real colour and the text specified as a
    // colour index. That means the text colour would be picked from
    // the system where the note background would be picked from a real colour.
    // Previously the note text colour was picked from the system tooltip
    // text colour, on modern gnome(e.g. 3) that tends to be 'white' with the
    // default theme.
    // Using the Libreoffice defaults ( instead of system specific colours
    // ) lessens the chance of the one colour being an unsuitable combination
    // because by default the note text is black and the note background is
    // a light yellow colour ( very similar to Excel's normal defaults )
    mnNoteText =  svtools::ColorConfig::GetDefaultColor( svtools::FONTCOLOR );
    mnNoteBack =  svtools::ColorConfig::GetDefaultColor( svtools::CALCNOTESBACKGROUND );

    // default colors
    switch( rRoot.GetBiff() )
    {
        case EXC_BIFF2:
            mpnColorTable = spnDefColorTable2;
            mnTableSize = std::size( spnDefColorTable2 );
        break;
        case EXC_BIFF3:
        case EXC_BIFF4:
            mpnColorTable = spnDefColorTable3;
            mnTableSize = std::size( spnDefColorTable3 );
        break;
        case EXC_BIFF5:
            mpnColorTable = spnDefColorTable5;
            mnTableSize = std::size( spnDefColorTable5 );
        break;
        case EXC_BIFF8:
            mpnColorTable = spnDefColorTable8;
            mnTableSize = std::size( spnDefColorTable8 );
        break;
        default:
            DBG_ERROR_BIFF();
    }
}

Color XclDefaultPalette::GetDefColor( sal_uInt16 nXclIndex ) const
{
    Color nColor;
    if( nXclIndex < mnTableSize )
        nColor = mpnColorTable[ nXclIndex ];
    else switch( nXclIndex )
    {
        case EXC_COLOR_WINDOWTEXT3:
        case EXC_COLOR_WINDOWTEXT:
        case EXC_COLOR_CHWINDOWTEXT:    nColor = COL_BLACK;     break;
        case EXC_COLOR_WINDOWBACK3:
        case EXC_COLOR_WINDOWBACK:
        case EXC_COLOR_CHWINDOWBACK:    nColor = COL_WHITE;     break;
        case EXC_COLOR_BUTTONBACK:      nColor = mnFaceColor;   break;
        case EXC_COLOR_CHBORDERAUTO:    nColor = COL_BLACK;     break;  // TODO: really always black?
        case EXC_COLOR_NOTEBACK:        nColor = mnNoteBack;    break;
        case EXC_COLOR_NOTETEXT:        nColor = mnNoteText;    break;
        case EXC_COLOR_FONTAUTO:        nColor = COL_AUTO;      break;
        default:
            SAL_WARN("sc",  "XclDefaultPalette::GetDefColor - unknown default color index: " << nXclIndex );
            nColor = COL_AUTO;
    }
    return nColor;
}

// Font Data ==================================================================

namespace Awt              = css::awt;
namespace AwtFontFamily    = Awt::FontFamily;
namespace AwtFontLineStyle  = Awt::FontUnderline;
namespace AwtFontStrikeout = Awt::FontStrikeout;

XclFontData::XclFontData()
{
    Clear();
}

XclFontData::XclFontData(const vcl::Font& rFont, model::ComplexColor const& rComplexColor)
{
    Clear();
    FillFromVclFont(rFont, rComplexColor);
}

XclFontData::XclFontData(const SvxFont& rFont, model::ComplexColor const& rComplexColor)
{
    FillFromSvxFont(rFont, rComplexColor);
}

void XclFontData::Clear()
{
    maName.clear();
    maStyle.clear();
    maComplexColor.setColor(COL_AUTO);
    mnHeight = 0;
    mnWeight = EXC_FONTWGHT_DONTKNOW;
    mnEscapem = EXC_FONTESC_NONE;
    mnFamily = EXC_FONTFAM_SYSTEM;
    mnCharSet = EXC_FONTCSET_ANSI_LATIN;
    mnUnderline = EXC_FONTUNDERL_NONE;
    mbItalic = mbStrikeout = mbOutline = mbShadow = false;
}

void XclFontData::FillFromVclFont(const vcl::Font& rFont, model::ComplexColor const& rComplexColor)
{
    maName = XclTools::GetXclFontName( rFont.GetFamilyName() );   // substitute with MS fonts
    maStyle.clear();
    SetScUnderline( rFont.GetUnderline() );
    mnEscapem = EXC_FONTESC_NONE;
    SetScHeight( rFont.GetFontSize().Height() );
    SetScWeight( rFont.GetWeight() );
    SetScFamily( rFont.GetFamilyType() );
    SetFontEncoding( rFont.GetCharSet() );
    SetScPosture( rFont.GetItalic() );
    SetScStrikeout( rFont.GetStrikeout() );
    mbOutline = rFont.IsOutline();
    mbShadow = rFont.IsShadow();

    maComplexColor = rComplexColor;
}

void XclFontData::FillFromSvxFont(const SvxFont& rFont, model::ComplexColor const& rComplexColor)
{
    FillFromVclFont(rFont, rComplexColor);
    SetScEscapement(rFont.GetEscapement());
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

FontLineStyle XclFontData::GetScUnderline() const
{
    FontLineStyle eScUnderl = LINESTYLE_NONE;
    switch( mnUnderline )
    {
        case EXC_FONTUNDERL_SINGLE:
        case EXC_FONTUNDERL_SINGLE_ACC: eScUnderl = LINESTYLE_SINGLE;  break;
        case EXC_FONTUNDERL_DOUBLE:
        case EXC_FONTUNDERL_DOUBLE_ACC: eScUnderl = LINESTYLE_DOUBLE;  break;
    }
    return eScUnderl;
}

SvxEscapement XclFontData::GetScEscapement() const
{
    SvxEscapement eScEscapem = SvxEscapement::Off;
    switch( mnEscapem )
    {
        case EXC_FONTESC_SUPER: eScEscapem = SvxEscapement::Superscript;    break;
        case EXC_FONTESC_SUB:   eScEscapem = SvxEscapement::Subscript;      break;
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

void XclFontData::SetScUnderline( FontLineStyle eScUnderl )
{
    switch( eScUnderl )
    {
        case LINESTYLE_NONE:
        case LINESTYLE_DONTKNOW:    mnUnderline = EXC_FONTUNDERL_NONE;      break;
        case LINESTYLE_DOUBLE:
        case LINESTYLE_DOUBLEWAVE:  mnUnderline = EXC_FONTUNDERL_DOUBLE;    break;
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
    return o3tl::convert<double>(mnHeight, o3tl::Length::twip, o3tl::Length::pt);
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
    return vcl::unohelper::ConvertFontWeight( GetScWeight() );
}

sal_Int16 XclFontData::GetApiUnderline() const
{
    sal_Int16 nApiUnderl = AwtFontLineStyle::NONE;
    switch( mnUnderline )
    {
        case EXC_FONTUNDERL_SINGLE:
        case EXC_FONTUNDERL_SINGLE_ACC: nApiUnderl = AwtFontLineStyle::SINGLE;  break;
        case EXC_FONTUNDERL_DOUBLE:
        case EXC_FONTUNDERL_DOUBLE_ACC: nApiUnderl = AwtFontLineStyle::DOUBLE;  break;
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
    mnHeight = std::min(o3tl::convert(fPoint, o3tl::Length::pt, o3tl::Length::twip) + 0.5, 32767.0);
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
    SetScWeight( vcl::unohelper::ConvertFontWeight( fApiWeight ) );
}

void XclFontData::SetApiUnderline( sal_Int16 nApiUnderl )
{
    switch( nApiUnderl )
    {
        case AwtFontLineStyle::NONE:
        case AwtFontLineStyle::DONTKNOW:    mnUnderline = EXC_FONTUNDERL_NONE;      break;
        case AwtFontLineStyle::DOUBLE:
        case AwtFontLineStyle::DOUBLEWAVE:  mnUnderline = EXC_FONTUNDERL_DOUBLE;    break;
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

bool operator==( const XclFontData& rLeft, const XclFontData& rRight )
{
    return
        (rLeft.mnHeight    == rRight.mnHeight)    &&
        (rLeft.mnWeight    == rRight.mnWeight)    &&
        (rLeft.mnUnderline == rRight.mnUnderline) &&
        (rLeft.maComplexColor == rRight.maComplexColor) &&
        (rLeft.mnEscapem   == rRight.mnEscapem)   &&
        (rLeft.mnFamily    == rRight.mnFamily)    &&
        (rLeft.mnCharSet   == rRight.mnCharSet)   &&
        (rLeft.mbItalic    == rRight.mbItalic)    &&
        (rLeft.mbStrikeout == rRight.mbStrikeout) &&
        (rLeft.mbOutline   == rRight.mbOutline)   &&
        (rLeft.mbShadow    == rRight.mbShadow)    &&
        (rLeft.maName      == rRight.maName);
}

namespace {

/** Property names for common font settings. */
const char *const sppcPropNamesChCommon[] =
{
    "CharUnderline", "CharStrikeout", "CharColor", "CharContoured", "CharShadowed", nullptr
};
/** Property names for Western font settings. */
const char *const sppcPropNamesChWstrn[] =
{
    "CharFontName", "CharHeight", "CharPosture", "CharWeight", nullptr
};
/** Property names for Asian font settings. */
const char *const sppcPropNamesChAsian[] =
{
    "CharFontNameAsian", "CharHeightAsian", "CharPostureAsian", "CharWeightAsian", nullptr
};
/** Property names for Complex font settings. */
const char *const sppcPropNamesChCmplx[] =
{
    "CharFontNameComplex", "CharHeightComplex", "CharPostureComplex", "CharWeightComplex", nullptr
};
/** Property names for escapement. */
const char *const sppcPropNamesChEscapement[] =
{
    "CharEscapement", "CharEscapementHeight", nullptr
};
const sal_Int8 EXC_API_ESC_HEIGHT           = 58;   /// Default escapement font height.

/** Property names for Western font settings without font name. */
const char *const *const sppcPropNamesChWstrnNoName = sppcPropNamesChWstrn + 1;
/** Property names for Asian font settings without font name. */
const char *const *const sppcPropNamesChAsianNoName = sppcPropNamesChAsian + 1;
/** Property names for Complex font settings without font name. */
const char *const *const sppcPropNamesChCmplxNoName = sppcPropNamesChCmplx + 1;

/** Property names for font settings in form controls. */
const char *const sppcPropNamesControl[] =
{
    "FontName", "FontFamily", "FontCharset", "FontHeight", "FontSlant",
    "FontWeight", "FontLineStyle", "FontStrikeout", "TextColor", nullptr
};

/** Inserts all passed API font settings into the font data object. */
void lclSetApiFontSettings( XclFontData& rFontData,
        const OUString& rApiFontName, float fApiHeight, float fApiWeight,
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
            OUString aApiFontName;
            float fApiHeight, fApiWeight;
            sal_Int16 nApiUnderl = 0, nApiStrikeout = 0;
            Awt::FontSlant eApiPosture;

            // read script type dependent properties
            ScfPropSetHelper& rPropSetHlp = GetChartHelper( nScript );
            rPropSetHlp.ReadFromPropertySet( rPropSet );
            rPropSetHlp >> aApiFontName >> fApiHeight >> eApiPosture >> fApiWeight;
            // read common properties
            maHlpChCommon.ReadFromPropertySet( rPropSet );
            maHlpChCommon >> nApiUnderl;
            maHlpChCommon >> nApiStrikeout;
            Color aColor;
            maHlpChCommon >> aColor;
            rFontData.maComplexColor.setColor(aColor);
            maHlpChCommon >> rFontData.mbOutline;
            maHlpChCommon >> rFontData.mbShadow;

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
            OUString aApiFontName;
            float fApiHeight(0.0), fApiWeight(0.0);
            sal_Int16 nApiFamily(0), nApiCharSet(0), nApiPosture(0), nApiUnderl(0), nApiStrikeout(0);

            // read font properties
            maHlpControl.ReadFromPropertySet( rPropSet );
            maHlpControl >> aApiFontName;
            maHlpControl >> nApiFamily;
            maHlpControl >> nApiCharSet;
            maHlpControl >> fApiHeight;
            maHlpControl >> nApiPosture;
            maHlpControl >> fApiWeight;
            maHlpControl >> nApiUnderl;
            maHlpControl >> nApiStrikeout;
            Color aColor;
            maHlpControl >> aColor;
            rFontData.maComplexColor.setColor(aColor);

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
            Color aColor = pFontColor ? *pFontColor : rFontData.maComplexColor.getFinalColor();
            maHlpChCommon   << rFontData.GetApiUnderline()
                            << rFontData.GetApiStrikeout()
                            << aColor
                            << rFontData.mbOutline
                            << rFontData.mbShadow;
            maHlpChCommon.WriteToPropertySet( rPropSet );

            // write script type dependent properties
            lclWriteChartFont( rPropSet, maHlpChWstrn, maHlpChWstrnNoName, rFontData, bHasWstrn );
            lclWriteChartFont( rPropSet, maHlpChAsian, maHlpChAsianNoName, rFontData, bHasAsian );
            lclWriteChartFont( rPropSet, maHlpChCmplx, maHlpChCmplxNoName, rFontData, bHasCmplx );

            // font escapement
            if( rFontData.GetScEscapement() != SvxEscapement::Off )
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
                            << rFontData.maComplexColor.getFinalColor();
            maHlpControl.WriteToPropertySet( rPropSet );
        }
        break;
    }
}

ScfPropSetHelper& XclFontPropSetHelper::GetChartHelper( sal_Int16 nScript )
{
    namespace ApiScriptType = css::i18n::ScriptType;
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

/** Special number format index describing a reused format. */
const NfIndexTableOffset PRV_NF_INDEX_REUSE = NF_INDEX_TABLE_ENTRIES;

/** German primary language not defined, LANGUAGE_GERMAN belongs to Germany. */
constexpr LanguageType PRV_LANGUAGE_GERMAN_PRIM = primary(LANGUAGE_GERMAN);
/** French primary language not defined, LANGUAGE_FRENCH belongs to France. */
constexpr LanguageType PRV_LANGUAGE_FRENCH_PRIM = primary(LANGUAGE_FRENCH);
/** Parent language identifier for Asian languages. */
constexpr LanguageType PRV_LANGUAGE_ASIAN_PRIM = primary(LANGUAGE_CHINESE);

/** Stores the number format used in Calc for an Excel built-in number format. */
struct XclBuiltInFormat
{
    sal_uInt16         mnXclNumFmt = EXC_FORMAT_NOTFOUND; /// Excel built-in index.
    OUString           maFormat;       /// Format string, may be 0 (meOffset used then).
    NfIndexTableOffset meOffset = NF_NUMBER_STANDARD; /// SvNumberFormatter format index, if maFormat=="".
    sal_uInt16         mnXclReuseFmt = 0; /// Use this Excel format, if meOffset==PRV_NF_INDEX_REUSE.

    /** Defines a literal Excel built-in number format. */
    XclBuiltInFormat(sal_uInt16 nXclNumFmt, const OUString& rFormat)
        : mnXclNumFmt(nXclNumFmt)
        , maFormat(rFormat)
    {
    }

    /** Defines an Excel built-in number format that maps to an own built-in format. */
    XclBuiltInFormat(sal_uInt16 nXclNumFmt, NfIndexTableOffset eOffset)
        : mnXclNumFmt(nXclNumFmt)
        , meOffset(eOffset)
    {
    }

    /** Defines an Excel built-in number format that is the same as the specified. */
    XclBuiltInFormat(sal_uInt16 nXclNumFmt, sal_uInt16 nXclReuse)
        : mnXclNumFmt(nXclNumFmt)
        , meOffset(PRV_NF_INDEX_REUSE)
        , mnXclReuseFmt(nXclReuse)
    {
    }
};

/** Default number format table. Last parent of all other tables, used for unknown languages. */
const XclBuiltInFormat spBuiltInFormats_DONTKNOW[] =
{
    {  0, NF_NUMBER_STANDARD   },     // General
    {  1, NF_NUMBER_INT        },     // 0
    {  2, NF_NUMBER_DEC2       },     // 0.00
    {  3, NF_NUMBER_1000INT    },     // #,##0
    {  4, NF_NUMBER_1000DEC2   },     // #,##0.00
    // 5...8 contained in file
    {  9, NF_PERCENT_INT       },     // 0%
    { 10, NF_PERCENT_DEC2      },     // 0.00%
    { 11, NF_SCIENTIFIC_000E00 },     // 0.00E+00
    { 12, NF_FRACTION_1D       },     // # ?/?
    { 13, NF_FRACTION_2D       },     // # ??/??

    // 14...22 date and time formats
    { 14, NF_DATE_SYS_DDMMYYYY },
    { 15, NF_DATE_SYS_DMMMYY   },
    { 16, NF_DATE_SYS_DDMMM    },
    { 17, NF_DATE_SYS_MMYY     },
    { 18, NF_TIME_HHMMAMPM     },
    { 19, NF_TIME_HHMMSSAMPM   },
    { 20, NF_TIME_HHMM         },
    { 21, NF_TIME_HHMMSS       },
    { 22, NF_DATETIME_SYSTEM_SHORT_HHMM },

    // 23...36 international formats
    { 23, 0  },
    { 24, 0  },
    { 25, 0  },
    { 26, 0  },
    { 27, 14 },
    { 28, 14 },
    { 29, 14 },
    { 30, 14 },
    { 31, 14 },
    { 32, 21 },
    { 33, 21 },
    { 34, 21 },
    { 35, 21 },
    { 36, 14 },

    // 37...44 accounting formats
    // 41...44 contained in file
    { 37, u"#,##0;-#,##0"_ustr            },
    { 38, u"#,##0;[RED]-#,##0"_ustr       },
    { 39, u"#,##0.00;-#,##0.00"_ustr      },
    { 40, u"#,##0.00;[RED]-#,##0.00"_ustr },

    // 45...49 more special formats
    { 45, u"mm:ss"_ustr     },
    { 46, u"[h]:mm:ss"_ustr },
    { 47, u"mm:ss.0"_ustr   },
    { 48, u"##0.0E+0"_ustr  },
    { 49, NF_TEXT           },

    // 50...81 international formats
    { 50, 14 },
    { 51, 14 },
    { 52, 14 },
    { 53, 14 },
    { 54, 14 },
    { 55, 14 },
    { 56, 14 },
    { 57, 14 },
    { 58, 14 },
    { 59, 1  },
    { 60, 2  },
    { 61, 3  },
    { 62, 4  },
    { 67, 9  },
    { 68, 10 },
    { 69, 12 },
    { 70, 13 },
    { 71, 14 },
    { 72, 14 },
    { 73, 15 },
    { 74, 16 },
    { 75, 17 },
    { 76, 20 },
    { 77, 21 },
    { 78, 22 },
    { 79, 45 },
    { 80, 46 },
    { 81, 47 },

    // 82...163 not used, must not occur in a file (Excel may crash)
};

// ENGLISH --------------------------------------------------------------------

/** Base table for English locales. */
const XclBuiltInFormat spBuiltInFormats_ENGLISH[] =
{
    { 15, u"DD-MMM-YY"_ustr        },
    { 16, u"DD-MMM"_ustr           },
    { 17, u"MMM-YY"_ustr           },
    { 18, u"h:mm AM/PM"_ustr       },
    { 19, u"h:mm:ss AM/PM"_ustr    },
    { 22, u"DD/MM/YYYY hh:mm"_ustr },
};

const XclBuiltInFormat spBuiltInFormats_ENGLISH_UK[] =
{
    { 63, u"£#,##0;-£#,##0"_ustr            },
    { 64, u"£#,##0;[RED]-£#,##0"_ustr       },
    { 65, u"£#,##0.00;-£#,##0.00"_ustr      },
    { 66, u"£#,##0.00;[RED]-£#,##0.00"_ustr },
};

const XclBuiltInFormat spBuiltInFormats_ENGLISH_EIRE[] =
{
    { 63, u"€#,##0;-€#,##0"_ustr            },
    { 64, u"€#,##0;[RED]-€#,##0"_ustr       },
    { 65, u"€#,##0.00;-€#,##0.00"_ustr      },
    { 66, u"€#,##0.00;[RED]-€#,##0.00"_ustr },
};

const XclBuiltInFormat spBuiltInFormats_ENGLISH_US[] =
{
    { 14, u"M/D/YYYY"_ustr                     },
    { 15, u"D-MMM-YY"_ustr                     },
    { 16, u"D-MMM"_ustr                        },
    { 20, u"h:mm"_ustr                         },
    { 21, u"h:mm:ss"_ustr                      },
    { 22, u"M/D/YYYY h:mm"_ustr                },
    { 37, u"#,##0_);(#,##0)"_ustr              },
    { 38, u"#,##0_);[RED](#,##0)"_ustr         },
    { 39, u"#,##0.00_);(#,##0.00)"_ustr        },
    { 40, u"#,##0.00_);[RED](#,##0.00)"_ustr   },
    { 63, u"$#,##0_);($#,##0)"_ustr            },
    { 64, u"$#,##0_);[RED]($#,##0)"_ustr       },
    { 65, u"$#,##0.00_);($#,##0.00)"_ustr      },
    { 66, u"$#,##0.00_);[RED]($#,##0.00)"_ustr },
};

const XclBuiltInFormat spBuiltInFormats_ENGLISH_CAN[] =
{
    { 20, u"h:mm"_ustr                      },
    { 21, u"h:mm:ss"_ustr                   },
    { 22, u"DD/MM/YYYY h:mm"_ustr           },
    { 63, u"$#,##0;-$#,##0"_ustr            },
    { 64, u"$#,##0;[RED]-$#,##0"_ustr       },
    { 65, u"$#,##0.00;-$#,##0.00"_ustr      },
    { 66, u"$#,##0.00;[RED]-$#,##0.00"_ustr },
};

const XclBuiltInFormat spBuiltInFormats_ENGLISH_AUS[] =
{
    { 14, u"D/MM/YYYY"_ustr                 },
    { 15, u"D-MMM-YY"_ustr                  },
    { 16, u"D-MMM"_ustr                     },
    { 20, u"h:mm"_ustr                      },
    { 21, u"h:mm:ss"_ustr                   },
    { 22, u"D/MM/YYYY h:mm"_ustr            },
    { 63, u"$#,##0;-$#,##0"_ustr            },
    { 64, u"$#,##0;[RED]-$#,##0"_ustr       },
    { 65, u"$#,##0.00;-$#,##0.00"_ustr      },
    { 66, u"$#,##0.00;[RED]-$#,##0.00"_ustr },
};

const XclBuiltInFormat spBuiltInFormats_ENGLISH_SAFRICA[] =
{
    { 14, u"YYYY/MM/DD"_ustr                      },
    { 18, NF_TIME_HHMMAMPM                        },
    { 19, NF_TIME_HHMMSSAMPM                      },
    { 22, u"YYYY/MM/DD hh:mm"_ustr                },
    { 63, u"\\R #,##0;\\R -#,##0"_ustr            },
    { 64, u"\\R #,##0;[RED]\\R -#,##0"_ustr       },
    { 65, u"\\R #,##0.00;\\R -#,##0.00"_ustr      },
    { 66, u"\\R #,##0.00;[RED]\\R -#,##0.00"_ustr },
};

// FRENCH ---------------------------------------------------------------------

/** Base table for French locales. */
const XclBuiltInFormat spBuiltInFormats_FRENCH[] =
{
    { 15, u"DD-MMM-YY"_ustr     },
    { 16, u"DD-MMM"_ustr        },
    { 17, u"MMM-YY"_ustr        },
    { 18, u"h:mm AM/PM"_ustr    },
    { 19, u"h:mm:ss AM/PM"_ustr },
};

const XclBuiltInFormat spBuiltInFormats_FRENCH_FRANCE[] =
{
    { 22, u"DD/MM/YYYY hh:mm"_ustr                  },
    { 37, u"#,##0\\ _€;-#,##0\\ _€"_ustr            },
    { 38, u"#,##0\\ _€;[RED]-#,##0\\ _€"_ustr       },
    { 39, u"#,##0.00\\ _€;-#,##0.00\\ _€"_ustr      },
    { 40, u"#,##0.00\\ _€;[RED]-#,##0.00\\ _€"_ustr },
    { 63, u"#,##0\\ €;-#,##0\\ €"_ustr              },
    { 64, u"#,##0\\ €;[RED]-#,##0\\ €"_ustr         },
    { 65, u"#,##0.00\\ €;-#,##0.00\\ €"_ustr        },
    { 66, u"#,##0.00\\ €;[RED]-#,##0.00\\ €"_ustr   },
};

const XclBuiltInFormat spBuiltInFormats_FRENCH_CANADIAN[] =
{
    { 22, u"YYYY-MM-DD hh:mm"_ustr                    },
    { 37, u"#,##0\\ _$_-;#,##0\\ _$-"_ustr            },
    { 38, u"#,##0\\ _$_-;[RED]#,##0\\ _$-"_ustr       },
    { 39, u"#,##0.00\\ _$_-;#,##0.00\\ _$-"_ustr      },
    { 40, u"#,##0.00\\ _$_-;[RED]#,##0.00\\ _$-"_ustr },
    { 63, u"#,##0\\ $_-;#,##0\\ $-"_ustr              },
    { 64, u"#,##0\\ $_-;[RED]#,##0\\ $-"_ustr         },
    { 65, u"#,##0.00\\ $_-;#,##0.00\\ $-"_ustr        },
    { 66, u"#,##0.00\\ $_-;[RED]#,##0.00\\ $-"_ustr   },
};

const XclBuiltInFormat spBuiltInFormats_FRENCH_SWISS[] =
{
    { 15, u"DD.MMM.YY"_ustr                                 },
    { 16, u"DD.MMM"_ustr                                    },
    { 17, u"MMM.YY"_ustr                                    },
    { 22, u"DD.MM.YYYY hh:mm"_ustr                          },
    { 63, u"\"SFr. \"#,##0;\"SFr. \"-#,##0"_ustr            },
    { 64, u"\"SFr. \"#,##0;[RED]\"SFr. \"-#,##0"_ustr       },
    { 65, u"\"SFr. \"#,##0.00;\"SFr. \"-#,##0.00"_ustr      },
    { 66, u"\"SFr. \"#,##0.00;[RED]\"SFr. \"-#,##0.00"_ustr },
};

const XclBuiltInFormat spBuiltInFormats_FRENCH_BELGIAN[] =
{
    { 14, u"D/MM/YYYY"_ustr      },
    { 15, u"D-MMM-YY"_ustr       },
    { 16, u"D-MMM"_ustr          },
    { 20, u"h:mm"_ustr           },
    { 21, u"h:mm:ss"_ustr        },
    { 22, u"D/MM/YYYY h:mm"_ustr },
};

// GERMAN ---------------------------------------------------------------------

/** Base table for German locales. */
const XclBuiltInFormat spBuiltInFormats_GERMAN[] =
{
    { 15, u"DD. MMM YY"_ustr       },
    { 16, u"DD. MMM"_ustr          },
    { 17, u"MMM YY"_ustr           },
    { 18, u"h:mm AM/PM"_ustr       },
    { 19, u"h:mm:ss AM/PM"_ustr    },
    { 22, u"DD.MM.YYYY hh:mm"_ustr },
};

const XclBuiltInFormat spBuiltInFormats_GERMAN_GERMANY[] =
{
    { 37, u"#,##0 _€;-#,##0 _€"_ustr            },
    { 38, u"#,##0 _€;[RED]-#,##0 _€"_ustr       },
    { 39, u"#,##0.00 _€;-#,##0.00 _€"_ustr      },
    { 40, u"#,##0.00 _€;[RED]-#,##0.00 _€"_ustr },
    { 63, u"#,##0 €;-#,##0 €"_ustr              },
    { 64, u"#,##0 €;[RED]-#,##0 €"_ustr         },
    { 65, u"#,##0.00 €;-#,##0.00 €"_ustr        },
    { 66, u"#,##0.00 €;[RED]-#,##0.00 €"_ustr   },
};

const XclBuiltInFormat spBuiltInFormats_GERMAN_AUSTRIAN[] =
{
    { 15, u"DD.MMM.YY"_ustr                   },
    { 16, u"DD.MMM"_ustr                      },
    { 17, u"MMM.YY"_ustr                      },
    { 63, u"€ #,##0;-€ #,##0"_ustr            },
    { 64, u"€ #,##0;[RED]-€ #,##0"_ustr       },
    { 65, u"€ #,##0.00;-€ #,##0.00"_ustr      },
    { 66, u"€ #,##0.00;[RED]-€ #,##0.00"_ustr },
};

const XclBuiltInFormat spBuiltInFormats_GERMAN_SWISS[] =
{
    { 63, u"\"SFr. \"#,##0;\"SFr. \"-#,##0"_ustr            },
    { 64, u"\"SFr. \"#,##0;[RED]\"SFr. \"-#,##0"_ustr       },
    { 65, u"\"SFr. \"#,##0.00;\"SFr. \"-#,##0.00"_ustr      },
    { 66, u"\"SFr. \"#,##0.00;[RED]\"SFr. \"-#,##0.00"_ustr },
};

const XclBuiltInFormat spBuiltInFormats_GERMAN_LUXEMBOURG[] =
{
    { 15, u"DD.MMM.YY"_ustr                     },
    { 16, u"DD.MMM"_ustr                        },
    { 17, u"MMM.YY"_ustr                        },
    { 37, u"#,##0 _€;-#,##0 _€"_ustr            },
    { 38, u"#,##0 _€;[RED]-#,##0 _€"_ustr       },
    { 39, u"#,##0.00 _€;-#,##0.00 _€"_ustr      },
    { 40, u"#,##0.00 _€;[RED]-#,##0.00 _€"_ustr },
    { 63, u"#,##0 €;-#,##0 €"_ustr              },
    { 64, u"#,##0 €;[RED]-#,##0 €"_ustr         },
    { 65, u"#,##0.00 €;-#,##0.00 €"_ustr        },
    { 66, u"#,##0.00 €;[RED]-#,##0.00 €"_ustr   },
};

const XclBuiltInFormat spBuiltInFormats_GERMAN_LIECHTENSTEIN[] =
{
    { 63, u"\"CHF \"#,##0;\"CHF \"-#,##0"_ustr            },
    { 64, u"\"CHF \"#,##0;[RED]\"CHF \"-#,##0"_ustr       },
    { 65, u"\"CHF \"#,##0.00;\"CHF \"-#,##0.00"_ustr      },
    { 66, u"\"CHF \"#,##0.00;[RED]\"CHF \"-#,##0.00"_ustr },
};

// ITALIAN --------------------------------------------------------------------

const XclBuiltInFormat spBuiltInFormats_ITALIAN_ITALY[] =
{
    { 15, u"DD-MMM-YY"_ustr                   },
    { 16, u"DD-MMM"_ustr                      },
    { 17, u"MMM-YY"_ustr                      },
    { 18, u"h:mm AM/PM"_ustr                  },
    { 19, u"h:mm:ss AM/PM"_ustr               },
    { 20, u"h:mm"_ustr                        },
    { 21, u"h:mm:ss"_ustr                     },
    { 22, u"DD/MM/YYYY h:mm"_ustr             },
    { 63, u"€ #,##0;-€ #,##0"_ustr            },
    { 64, u"€ #,##0;[RED]-€ #,##0"_ustr       },
    { 65, u"€ #,##0.00;-€ #,##0.00"_ustr      },
    { 66, u"€ #,##0.00;[RED]-€ #,##0.00"_ustr },
};

const XclBuiltInFormat spBuiltInFormats_ITALIAN_SWISS[] =
{
    { 15, u"DD.MMM.YY"_ustr                                 },
    { 16, u"DD.MMM"_ustr                                    },
    { 17, u"MMM.YY"_ustr                                    },
    { 18, u"h:mm AM/PM"_ustr                                },
    { 19, u"h:mm:ss AM/PM"_ustr                             },
    { 22, u"DD.MM.YYYY hh:mm"_ustr                          },
    { 63, u"\"SFr. \"#,##0;\"SFr. \"-#,##0"_ustr            },
    { 64, u"\"SFr. \"#,##0;[RED]\"SFr. \"-#,##0"_ustr       },
    { 65, u"\"SFr. \"#,##0.00;\"SFr. \"-#,##0.00"_ustr      },
    { 66, u"\"SFr. \"#,##0.00;[RED]\"SFr. \"-#,##0.00"_ustr },
};

// SWEDISH --------------------------------------------------------------------

const XclBuiltInFormat spBuiltInFormats_SWEDISH_SWEDEN[] =
{
    { 15, u"DD-MMM-YY"_ustr                             },
    { 16, u"DD-MMM"_ustr                                },
    { 17, u"MMM-YY"_ustr                                },
    { 18, u"h:mm AM/PM"_ustr                            },
    { 19, u"h:mm:ss AM/PM"_ustr                         },
    { 22, u"YYYY-MM-DD hh:mm"_ustr                      },
    { 37, u"#,##0 _k_r;-#,##0 _k_r"_ustr                },
    { 38, u"#,##0 _k_r;[RED]-#,##0 _k_r"_ustr           },
    { 39, u"#,##0.00 _k_r;-#,##0.00 _k_r"_ustr          },
    { 40, u"#,##0.00 _k_r;[RED]-#,##0.00 _k_r"_ustr     },
    { 63, u"#,##0 \"kr\";-#,##0 \"kr\""_ustr            },
    { 64, u"#,##0 \"kr\";[RED]-#,##0 \"kr\""_ustr       },
    { 65, u"#,##0.00 \"kr\";-#,##0.00 \"kr\""_ustr      },
    { 66, u"#,##0.00 \"kr\";[RED]-#,##0.00 \"kr\""_ustr },
};

const XclBuiltInFormat spBuiltInFormats_SWEDISH_FINLAND[] =
{
    {  9, u"0 %"_ustr                           },
    { 10, u"0.00 %"_ustr                        },
    { 15, u"DD.MMM.YY"_ustr                     },
    { 16, u"DD.MMM"_ustr                        },
    { 17, u"MMM.YY"_ustr                        },
    { 18, u"h:mm AM/PM"_ustr                    },
    { 19, u"h:mm:ss AM/PM"_ustr                 },
    { 22, u"D.M.YYYY hh:mm"_ustr                },
    { 37, u"#,##0 _€;-#,##0 _€"_ustr            },
    { 38, u"#,##0 _€;[RED]-#,##0 _€"_ustr       },
    { 39, u"#,##0.00 _€;-#,##0.00 _€"_ustr      },
    { 40, u"#,##0.00 _€;[RED]-#,##0.00 _€"_ustr },
    { 63, u"#,##0 €;-#,##0 €"_ustr              },
    { 64, u"#,##0 €;[RED]-#,##0 €"_ustr         },
    { 65, u"#,##0.00 €;-#,##0.00 €"_ustr        },
    { 66, u"#,##0.00 €;[RED]-#,##0.00 €"_ustr   },
};

// ASIAN ----------------------------------------------------------------------

/** Base table for Asian locales. */
const XclBuiltInFormat spBuiltInFormats_ASIAN[] =
{
    { 18, u"h:mm AM/PM"_ustr                   },
    { 19, u"h:mm:ss AM/PM"_ustr                },
    { 20, u"h:mm"_ustr                         },
    { 21, u"h:mm:ss"_ustr                      },
    { 23, u"$#,##0_);($#,##0)"_ustr            },
    { 24, u"$#,##0_);[RED]($#,##0)"_ustr       },
    { 25, u"$#,##0.00_);($#,##0.00)"_ustr      },
    { 26, u"$#,##0.00_);[RED]($#,##0.00)"_ustr },
    { 29, 28 },
    { 36, 27 },
    { 50, 27 },
    { 51, 28 },
    { 52, 34 },
    { 53, 35 },
    { 54, 28 },
    { 55, 34 },
    { 56, 35 },
    { 57, 27 },
    { 58, 28 },
};

const XclBuiltInFormat spBuiltInFormats_JAPANESE[] =
{
    { 14, u"YYYY/M/D"_ustr                  },
    { 15, u"D-MMM-YY"_ustr                  },
    { 16, u"D-MMM"_ustr                     },
    { 17, u"MMM-YY"_ustr                    },
    { 22, u"YYYY/M/D h:mm"_ustr             },
    { 27, u"[$-0411]GE.M.D"_ustr            },
    { 28, u"[$-0411]GGGE年M月D日"_ustr       },
    { 30, u"[$-0411]M/D/YY"_ustr            },
    { 31, u"[$-0411]YYYY年M月D日"_ustr       },
    { 32, u"[$-0411]h時mm分"_ustr            },
    { 33, u"[$-0411]h時mm分ss秒"_ustr        },
    { 34, u"[$-0411]YYYY年M月"_ustr          },
    { 35, u"[$-0411]M月D日"_ustr             },
    { 63, u"¥#,##0;-¥#,##0"_ustr            },
    { 64, u"¥#,##0;[RED]-¥#,##0"_ustr       },
    { 65, u"¥#,##0.00;-¥#,##0.00"_ustr      },
    { 66, u"¥#,##0.00;[RED]-¥#,##0.00"_ustr },
};

const XclBuiltInFormat spBuiltInFormats_KOREAN[] =
{
    { 14, u"YYYY-MM-DD"_ustr                 },
    { 15, u"DD-MMM-YY"_ustr                  },
    { 16, u"DD-MMM"_ustr                     },
    { 17, u"MMM-YY"_ustr                     },
    { 22, u"YYYY-MM-DD h:mm"_ustr            },
    { 27, u"[$-0412]YYYY年 MM月 DD日"_ustr    },
    { 28, u"[$-0412]MM-DD"_ustr              },
    { 30, u"[$-0412]MM-DD-YY"_ustr           },
    { 31, u"[$-0412]YYYY년 MM월 DD일"_ustr    },
    { 32, u"[$-0412]h시 mm분"_ustr            },
    { 33, u"[$-0412]h시 mm분 ss초"_ustr       },
    { 34, u"[$-0412]YYYY\"/\"MM\"/\"DD"_ustr },
    { 35, u"[$-0412]YYYY-MM-DD"_ustr         },
    { 63, u"￦#,##0;-￦#,##0"_ustr            },
    { 64, u"￦#,##0;[RED]-￦#,##0"_ustr       },
    { 65, u"￦#,##0.00;-￦#,##0.00"_ustr      },
    { 66, u"￦#,##0.00;[RED]-￦#,##0.00"_ustr },
};

const XclBuiltInFormat spBuiltInFormats_CHINESE_SIMPLIFIED[] =
{
    { 14, u"YYYY-M-D"_ustr                   },
    { 15, u"D-MMM-YY"_ustr                   },
    { 16, u"D-MMM"_ustr                      },
    { 17, u"MMM-YY"_ustr                     },
    { 22, u"YYYY-M-D h:mm"_ustr              },
    { 27, u"[$-0804]YYYY年M月"_ustr           },
    { 28, u"[$-0804]M月D日"_ustr              },
    { 30, u"[$-0804]M-D-YY"_ustr             },
    { 31, u"[$-0804]YYYY年M月D日"_ustr        },
    { 32, u"[$-0804]h时mm分"_ustr             },
    { 33, u"[$-0804]h时mm分ss秒"_ustr         },
    { 34, u"[$-0804]AM/PMh时mm分"_ustr        },
    { 35, u"[$-0804]AM/PMh时mm分ss秒"_ustr    },
    { 52, 27                                 },
    { 53, 28                                 },
    { 63, u"￥#,##0;-￥#,##0"_ustr            },
    { 64, u"￥#,##0;[RED]-￥#,##0"_ustr       },
    { 65, u"￥#,##0.00;-￥#,##0.00"_ustr      },
    { 66, u"￥#,##0.00;[RED]-￥#,##0.00"_ustr },
};

const XclBuiltInFormat spBuiltInFormats_CHINESE_TRADITIONAL[] =
{
    { 15, u"D-MMM-YY"_ustr                         },
    { 16, u"D-MMM"_ustr                            },
    { 17, u"MMM-YY"_ustr                           },
    { 18, u"hh:mm AM/PM"_ustr                      },
    { 19, u"hh:mm:ss AM/PM"_ustr                   },
    { 20, NF_TIME_HHMM                             },
    { 21, NF_TIME_HHMMSS                           },
    { 22, u"YYYY/M/D hh:mm"_ustr                   },
    { 23, u"US$#,##0_);(US$#,##0)"_ustr            },
    { 24, u"US$#,##0_);[RED](US$#,##0)"_ustr       },
    { 25, u"US$#,##0.00_);(US$#,##0.00)"_ustr      },
    { 26, u"US$#,##0.00_);[RED](US$#,##0.00)"_ustr },
    { 27, u"[$-0404]E/M/D"_ustr                    },
    { 28, u"[$-0404]E年M月D日"_ustr                 },
    { 30, u"[$-0404]M/D/YY"_ustr                   },
    { 31, u"[$-0404]YYYY年M月D日"_ustr              },
    { 32, u"[$-0404]hh時mm分"_ustr                  },
    { 33, u"[$-0404]hh時mm分ss秒"_ustr              },
    { 34, u"[$-0404]AM/PMhh時mm分"_ustr             },
    { 35, u"[$-0404]AM/PMhh時mm分ss秒"_ustr         },
    { 63, u"$#,##0;-$#,##0"_ustr                   },
    { 64, u"$#,##0;[RED]-$#,##0"_ustr              },
    { 65, u"$#,##0.00;-$#,##0.00"_ustr             },
    { 66, u"$#,##0.00;[RED]-$#,##0.00"_ustr        },
};

// OTHER ----------------------------------------------------------------------

const XclBuiltInFormat spBuiltInFormats_HEBREW[] =
{
    { 15, u"DD-MMMM-YY"_ustr                  },
    { 16, u"DD-MMMM"_ustr                     },
    { 17, u"MMMM-YY"_ustr                     },
    { 18, u"h:mm AM/PM"_ustr                  },
    { 19, u"h:mm:ss AM/PM"_ustr               },
    { 63, u"₪ #,##0;₪ -#,##0"_ustr            },
    { 64, u"₪ #,##0;[RED]₪ -#,##0"_ustr       },
    { 65, u"₪ #,##0.00;₪ -#,##0.00"_ustr      },
    { 66, u"₪ #,##0.00;[RED]₪ -#,##0.00"_ustr },
};

const XclBuiltInFormat spBuiltInFormats_THAI[] =
{
    { 14, u"D/M/YYYY"_ustr                       },
    { 15, u"D-MMM-YY"_ustr                       },
    { 16, u"D-MMM"_ustr                          },
    { 17, u"MMM-YY"_ustr                         },
    { 18, u"h:mm AM/PM"_ustr                     },
    { 19, u"h:mm:ss AM/PM"_ustr                  },
    { 22, u"D/M/YYYY h:mm"_ustr                  },
    { 59, u"t0"_ustr                             },
    { 60, u"t0.00"_ustr                          },
    { 61, u"t#,##0"_ustr                         },
    { 62, u"t#,##0.00"_ustr                      },
    { 63, u"t฿#,##0_);t(฿#,##0)"_ustr            },
    { 64, u"t฿#,##0_);[RED]t(฿#,##0)"_ustr       },
    { 65, u"t฿#,##0.00_);t(฿#,##0.00)"_ustr      },
    { 66, u"t฿#,##0.00_);[RED]t(฿#,##0.00)"_ustr },
    { 67, u"t0%"_ustr                            },
    { 68, u"t0.00%"_ustr                         },
    { 69, u"t# ?/?"_ustr                         },
    { 70, u"t# ?\?/?\?"_ustr                     },
    { 71, u"tD/M/EE"_ustr                        },
    { 72, u"tD-MMM-E"_ustr                       },
    { 73, u"tD-MMM"_ustr                         },
    { 74, u"tMMM-E"_ustr                         },
    { 75, u"th:mm"_ustr                          },
    { 76, u"th:mm:ss"_ustr                       },
    { 77, u"tD/M/EE h:mm"_ustr                   },
    { 78, u"tmm:ss"_ustr                         },
    { 79, u"t[h]:mm:ss"_ustr                     },
    { 80, u"tmm:ss.0"_ustr                       },
    { 81, u"D/M/E"_ustr                          },
};

/** Specifies a number format table for a specific language. */
struct XclBuiltInFormatTable
{
    LanguageType        meLanguage;         /// The language of this table.
    LanguageType        meParentLang;       /// The language of the parent table.
    std::span<const XclBuiltInFormat> maFormats; /// The number format table.
};

constexpr XclBuiltInFormatTable spBuiltInFormatTables[] =
{   //  language                        parent language             format table
    {   LANGUAGE_DONTKNOW,              LANGUAGE_NONE,              spBuiltInFormats_DONTKNOW               },

    {   LANGUAGE_ENGLISH,               LANGUAGE_DONTKNOW,          spBuiltInFormats_ENGLISH                },
    {   LANGUAGE_ENGLISH_UK,            LANGUAGE_ENGLISH,           spBuiltInFormats_ENGLISH_UK             },
    {   LANGUAGE_ENGLISH_EIRE,          LANGUAGE_ENGLISH,           spBuiltInFormats_ENGLISH_EIRE           },
    {   LANGUAGE_ENGLISH_US,            LANGUAGE_ENGLISH,           spBuiltInFormats_ENGLISH_US             },
    {   LANGUAGE_ENGLISH_CAN,           LANGUAGE_ENGLISH,           spBuiltInFormats_ENGLISH_CAN            },
    {   LANGUAGE_ENGLISH_AUS,           LANGUAGE_ENGLISH,           spBuiltInFormats_ENGLISH_AUS            },
    {   LANGUAGE_ENGLISH_SAFRICA,       LANGUAGE_ENGLISH,           spBuiltInFormats_ENGLISH_SAFRICA        },
    {   LANGUAGE_ENGLISH_NZ,            LANGUAGE_ENGLISH_AUS,       {}                                      },

    {   PRV_LANGUAGE_FRENCH_PRIM,       LANGUAGE_DONTKNOW,          spBuiltInFormats_FRENCH                 },
    {   LANGUAGE_FRENCH,                PRV_LANGUAGE_FRENCH_PRIM,   spBuiltInFormats_FRENCH_FRANCE          },
    {   LANGUAGE_FRENCH_CANADIAN,       PRV_LANGUAGE_FRENCH_PRIM,   spBuiltInFormats_FRENCH_CANADIAN        },
    {   LANGUAGE_FRENCH_SWISS,          PRV_LANGUAGE_FRENCH_PRIM,   spBuiltInFormats_FRENCH_SWISS           },
    {   LANGUAGE_FRENCH_BELGIAN,        LANGUAGE_FRENCH,            spBuiltInFormats_FRENCH_BELGIAN         },
    {   LANGUAGE_FRENCH_LUXEMBOURG,     LANGUAGE_FRENCH,            {}                                      },
    {   LANGUAGE_FRENCH_MONACO,         LANGUAGE_FRENCH,            {}                                      },

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

} // namespace

XclNumFmtBuffer::XclNumFmtBuffer( const XclRoot& rRoot ) :
    meSysLang( rRoot.GetSysLanguage() ),
    mnStdScNumFmt( rRoot.GetFormatter().GetStandardIndex( ScGlobal::eLnge ) )
{
    // *** insert default formats (BIFF5+ only)***

    if( rRoot.GetBiff() >= EXC_BIFF5 )
        InsertBuiltinFormats();
}

void XclNumFmtBuffer::InitializeImport()
{
    maFmtMap.clear();
}

void XclNumFmtBuffer::InsertFormat( sal_uInt16 nXclNumFmt, const OUString& rFormat )
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
    for(const auto &rTable : spBuiltInFormatTables)
        aBuiltInMap[ rTable.meLanguage ] = &rTable;

    // build a list of table pointers for the current language, with all parent tables
    typedef ::std::vector< const XclBuiltInFormatTable* > XclBuiltInVec;
    XclBuiltInVec aBuiltInVec;
    for( XclBuiltInMap::const_iterator aMIt = aBuiltInMap.find( meSysLang ), aMEnd = aBuiltInMap.end();
            aMIt != aMEnd; aMIt = aBuiltInMap.find( aMIt->second->meParentLang ) )
        aBuiltInVec.push_back( aMIt->second );
    // language not supported
    if( aBuiltInVec.empty() )
    {
        SAL_WARN("sc",  "XclNumFmtBuffer::InsertBuiltinFormats - language not supported (#i29949#) 0x" << std::hex << meSysLang );
        XclBuiltInMap::const_iterator aMIt = aBuiltInMap.find( LANGUAGE_DONTKNOW );
        OSL_ENSURE( aMIt != aBuiltInMap.end(), "XclNumFmtBuffer::InsertBuiltinFormats - default map not found" );
        if( aMIt != aBuiltInMap.end() )
            aBuiltInVec.push_back( aMIt->second );
    }

    // insert the default formats in the format map, from root parent to system language
    std::map< sal_uInt16, sal_uInt16 > aReuseMap;
    for( XclBuiltInVec::reverse_iterator aVIt = aBuiltInVec.rbegin(), aVEnd = aBuiltInVec.rend(); aVIt != aVEnd; ++aVIt )
    {
        // put LANGUAGE_SYSTEM for all entries in default table
        LanguageType eLang = ((*aVIt)->meLanguage == LANGUAGE_DONTKNOW) ? LANGUAGE_SYSTEM : meSysLang;
        for (const XclBuiltInFormat& rBuiltIn : (*aVIt)->maFormats)
        {
            XclNumFmt& rNumFmt = maFmtMap[rBuiltIn.mnXclNumFmt];

            rNumFmt.meOffset = rBuiltIn.meOffset;
            rNumFmt.meLanguage = eLang;

            rNumFmt.maFormat = rBuiltIn.maFormat;

            if (rBuiltIn.meOffset == PRV_NF_INDEX_REUSE)
                aReuseMap[rBuiltIn.mnXclNumFmt] = rBuiltIn.mnXclReuseFmt;
            else
                aReuseMap.erase(rBuiltIn.mnXclNumFmt);
        }
    }

    // copy reused number formats
    for( const auto& [rXclNumFmt, rXclReuseFmt] : aReuseMap )
        maFmtMap[ rXclNumFmt ] = maFmtMap[ rXclReuseFmt ];
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
    SvxCellHorJustify eHorJust = SvxCellHorJustify::Standard;
    switch( mnHorAlign )
    {
        case EXC_XF_HOR_GENERAL:    eHorJust = SvxCellHorJustify::Standard;    break;
        case EXC_XF_HOR_LEFT:       eHorJust = SvxCellHorJustify::Left;        break;
        case EXC_XF_HOR_CENTER_AS:
        case EXC_XF_HOR_CENTER:     eHorJust = SvxCellHorJustify::Center;      break;
        case EXC_XF_HOR_RIGHT:      eHorJust = SvxCellHorJustify::Right;       break;
        case EXC_XF_HOR_FILL:       eHorJust = SvxCellHorJustify::Repeat;      break;
        case EXC_XF_HOR_JUSTIFY:
        case EXC_XF_HOR_DISTRIB:    eHorJust = SvxCellHorJustify::Block;       break;
        default:    OSL_FAIL( "XclCellAlign::GetScHorAlign - unknown horizontal alignment" );
    }
    return eHorJust;
}

SvxCellJustifyMethod XclCellAlign::GetScHorJustifyMethod() const
{
    return (mnHorAlign == EXC_XF_HOR_DISTRIB) ? SvxCellJustifyMethod::Distribute : SvxCellJustifyMethod::Auto;
}

SvxCellVerJustify XclCellAlign::GetScVerAlign() const
{
    SvxCellVerJustify eVerJust = SvxCellVerJustify::Standard;
    switch( mnVerAlign )
    {
        case EXC_XF_VER_TOP:        eVerJust = SvxCellVerJustify::Top;         break;
        case EXC_XF_VER_CENTER:     eVerJust = SvxCellVerJustify::Center;      break;
        case EXC_XF_VER_BOTTOM:     eVerJust = SvxCellVerJustify::Standard;    break;
        case EXC_XF_VER_JUSTIFY:
        case EXC_XF_VER_DISTRIB:    eVerJust = SvxCellVerJustify::Block;       break;
        default:    OSL_FAIL( "XclCellAlign::GetScVerAlign - unknown vertical alignment" );
    }
    return eVerJust;
}

SvxCellJustifyMethod XclCellAlign::GetScVerJustifyMethod() const
{
    return (mnVerAlign == EXC_XF_VER_DISTRIB) ? SvxCellJustifyMethod::Distribute : SvxCellJustifyMethod::Auto;
}

SvxFrameDirection XclCellAlign::GetScFrameDir() const
{
    SvxFrameDirection eFrameDir = SvxFrameDirection::Environment;
    switch( mnTextDir )
    {
        case EXC_XF_TEXTDIR_CONTEXT:    eFrameDir = SvxFrameDirection::Environment;     break;
        case EXC_XF_TEXTDIR_LTR:        eFrameDir = SvxFrameDirection::Horizontal_LR_TB;   break;
        case EXC_XF_TEXTDIR_RTL:        eFrameDir = SvxFrameDirection::Horizontal_RL_TB;  break;
        default:    OSL_FAIL( "XclCellAlign::GetScFrameDir - unknown CTL text direction" );
    }
    return eFrameDir;
}

void XclCellAlign::SetScHorAlign( SvxCellHorJustify eHorJust )
{
    switch( eHorJust )
    {
        case SvxCellHorJustify::Standard:  mnHorAlign = EXC_XF_HOR_GENERAL;    break;
        case SvxCellHorJustify::Left:      mnHorAlign = EXC_XF_HOR_LEFT;       break;
        case SvxCellHorJustify::Center:    mnHorAlign = EXC_XF_HOR_CENTER;     break;
        case SvxCellHorJustify::Right:     mnHorAlign = EXC_XF_HOR_RIGHT;      break;
        case SvxCellHorJustify::Block:     mnHorAlign = EXC_XF_HOR_JUSTIFY;    break;
        case SvxCellHorJustify::Repeat:    mnHorAlign = EXC_XF_HOR_FILL;       break;
        default:                        mnHorAlign = EXC_XF_HOR_GENERAL;
            OSL_FAIL( "XclCellAlign::SetScHorAlign - unknown horizontal alignment" );
    }
}

void XclCellAlign::SetScVerAlign( SvxCellVerJustify eVerJust )
{
    switch( eVerJust )
    {
        case SvxCellVerJustify::Standard:  mnVerAlign = EXC_XF_VER_BOTTOM; break;
        case SvxCellVerJustify::Top:       mnVerAlign = EXC_XF_VER_TOP;    break;
        case SvxCellVerJustify::Center:    mnVerAlign = EXC_XF_VER_CENTER; break;
        case SvxCellVerJustify::Bottom:    mnVerAlign = EXC_XF_VER_BOTTOM; break;
        default:                        mnVerAlign = EXC_XF_VER_BOTTOM;
            OSL_FAIL( "XclCellAlign::SetScVerAlign - unknown vertical alignment" );
    }
}

void XclCellAlign::SetScFrameDir( SvxFrameDirection eFrameDir )
{
    switch( eFrameDir )
    {
        case SvxFrameDirection::Environment:      mnTextDir = EXC_XF_TEXTDIR_CONTEXT; break;
        case SvxFrameDirection::Horizontal_LR_TB: mnTextDir = EXC_XF_TEXTDIR_LTR;     break;
        case SvxFrameDirection::Horizontal_RL_TB: mnTextDir = EXC_XF_TEXTDIR_RTL;     break;
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

XclCellArea::XclCellArea() :
    mnForeColor( EXC_COLOR_WINDOWTEXT ),
    mnBackColor( EXC_COLOR_WINDOWBACK ),
    mnPattern( EXC_PATT_NONE )
{
}

XclCellArea::XclCellArea(sal_uInt8 nPattern) :
    mnForeColor( EXC_COLOR_WINDOWTEXT ),
    mnBackColor( EXC_COLOR_WINDOWBACK ),
    mnPattern( nPattern )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
