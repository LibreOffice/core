/*************************************************************************
 *
 *  $RCSfile: xlstyle.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-05 13:36:50 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif
#pragma hdrstop

// ============================================================================

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

XclDefaultPalette::XclDefaultPalette( XclBiff eBiff ) :
    mpColorTable( NULL ),
    mnTableSize( 0 )
{
    const StyleSettings& rSett = Application::GetSettings().GetStyleSettings();
    mnWindowText = rSett.GetWindowTextColor().GetColor();
    mnWindowBack = rSett.GetWindowColor().GetColor();

    if( eBiff != xlBiffUnknown )
        SetDefaultColors( eBiff );
}

void XclDefaultPalette::SetDefaultColors( XclBiff eBiff )
{
    switch( eBiff )
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
        case EXC_COLOR_WINDOWTEXT:  nColor = mnWindowText;  break;
        case EXC_COLOR_WINDOWBACK3:
        case EXC_COLOR_WINDOWBACK:  nColor = mnWindowBack;  break;
        case EXC_COLOR_FONTAUTO:    nColor = COL_AUTO;      break;
        default:                    nColor = COL_AUTO;
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
