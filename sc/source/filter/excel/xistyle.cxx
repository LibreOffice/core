/*************************************************************************
 *
 *  $RCSfile: xistyle.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 18:04:37 $
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

#ifndef SC_XISTYLE_HXX
#include "xistyle.hxx"
#endif

#ifndef _SFX_PRINTER_HXX
#include <sfx2/printer.hxx>
#endif
#ifndef _SFX_OBJSH_HXX
#include <sfx2/objsh.hxx>
#endif
#ifndef _ZFORLIST_HXX
#include <svtools/zforlist.hxx>
#endif
#ifndef _CTRLTOOL_HXX
#include <svtools/ctrltool.hxx>
#endif

#ifndef SC_ITEMS_HXX
#include "scitems.hxx"
#endif
#ifndef _SVX_FONTITEM_HXX
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX
#include <svx/wghtitem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX
#include <svx/postitem.hxx>
#endif
#ifndef _SVX_CRSDITEM_HXX
#include <svx/crsditem.hxx>
#endif
#ifndef _SVX_ITEM_HXX
#include <svx/cntritem.hxx>
#endif
#ifndef _SVX_SHDDITEM_HXX
#include <svx/shdditem.hxx>
#endif
#ifndef _SVX_ESCPITEM_HXX
#include <svx/escpitem.hxx>
#endif
#ifndef _SVX_ALGITEM_HXX
#include <svx/algitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_ROTMODIT_HXX
#include <svx/rotmodit.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_FRMDIRITEM_HXX
#include <svx/frmdiritem.hxx>
#endif
#ifndef _EEITEM_HXX
#include <svx/eeitem.hxx>
#endif
#ifndef _SVX_FLSTITEM_HXX
#include <svx/flstitem.hxx>
#endif

#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_SCDOCPOL_HXX
#include "docpool.hxx"
#endif
#ifndef SC_SCATTR_HXX
#include "attrib.hxx"
#endif
#ifndef SC_STLPOOL_HXX
#include "stlpool.hxx"
#endif

#ifndef SC_XISTREAM_HXX
#include "xistream.hxx"
#endif


#include "root.hxx"

// PALETTE record - color information =========================================

XclImpPalette::XclImpPalette( const XclImpRoot& rRoot ) :
    XclDefaultPalette( rRoot.GetBiff() ),
    XclImpRoot( rRoot )
{
}

void XclImpPalette::OnChangeBiff()
{
    SetDefaultColors( GetBiff() );
}

ColorData XclImpPalette::GetColorData( sal_uInt16 nXclIndex, ColorData nDefault ) const
{
    if( nXclIndex >= GetIndexOffset() )
    {
        sal_uInt32 nIx = nXclIndex - GetIndexOffset();
        if( nIx < maColorTable.size() )
            return maColorTable[ nIx ];
    }
    return GetDefColorData( nXclIndex, nDefault );
}

void XclImpPalette::ReadPalette( XclImpStream& rStrm )
{
    sal_uInt16 nCount;
    rStrm >> nCount;
    DBG_ASSERT( rStrm.GetRecLeft() == 4UL * nCount, "XclImpPalette::ReadPalette - size mismatch" );

    maColorTable.resize( nCount );
    for( sal_uInt16 nIndex = 0; nIndex < nCount; ++nIndex )
    {
        sal_uInt8 nRed, nGreen, nBlue;
        rStrm >> nRed >> nGreen >> nBlue;
        rStrm.Ignore( 1 );
        maColorTable[ nIndex ] = RGB_COLORDATA( nRed, nGreen, nBlue );
    }
}


// FONT record - font information =============================================

inline XclImpStream& operator>>( XclImpStream& rStrm, XclUnderline& reUnderl )
{
    reUnderl = static_cast< XclUnderline >( rStrm.ReaduInt8() );
    return rStrm;
}

inline XclImpStream& operator>>( XclImpStream& rStrm, XclEscapement& reEscapem )
{
    reEscapem = static_cast< XclEscapement >( rStrm.ReaduInt16() );
    return rStrm;
};


// ----------------------------------------------------------------------------

XclImpFont::XclImpFont( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot ),
    meFontFamily( FAMILY_DONTKNOW ),
    meFontCharSet( ScfTools::GetSystemCharSet() ),
    mbIsWestern( true ),
    mbIsAsian( false ),
    mbIsComplex( false )
{
}

XclImpFont::XclImpFont( const XclImpRoot& rRoot, const XclFontData& rFontData ) :
    XclImpRoot( rRoot ),
    maData( rFontData )
{
    if( maData.maStyle.Len() )
    {
        if( SfxObjectShell* pDocShell = GetDocShell() )
        {
            if( const SvxFontListItem* pInfoItem = static_cast< const SvxFontListItem* >(
                pDocShell->GetItem( SID_ATTR_CHAR_FONTLIST ) ) )
            {
                if( const FontList* pFontList = pInfoItem->GetFontList() )
                {
                    FontInfo aFontInfo( pFontList->Get( maData.maName, maData.maStyle ) );
                    if( aFontInfo.GetWeight() > WEIGHT_NORMAL )
                        maData.mnWeight = EXC_FONTWGHT_BOLD;
                    maData.mbItalic = (aFontInfo.GetItalic() != ITALIC_NONE);
                }
            }
        }
    }
    Update();   /// sets internal font charset/family and script types
}

long XclImpFont::GetCharWidth() const
{
    if( SfxPrinter* pPrinter = GetPrinter() )
    {
        Font aFont( maData.maName, Size( 0, maData.mnHeight ) );
        aFont.SetFamily( meFontFamily );
        aFont.SetCharSet( meFontCharSet );
        pPrinter->SetFont( aFont );
        return pPrinter->GetTextWidth( String( '0' ) );
    }
    return 11 * maData.mnHeight / 20;
}

void XclImpFont::ReadFont( XclImpStream& rStrm )
{
    switch( GetBiff() )
    {
        case xlBiff2:
            ReadFontData2( rStrm );
            ReadFontName2( rStrm );
        break;
        case xlBiff3:
        case xlBiff4:
            ReadFontData2( rStrm );
            rStrm >> maData.mnColor;
            ReadFontName2( rStrm );
        break;
        case xlBiff5:
        case xlBiff7:
            ReadFontData5( rStrm );
            ReadFontName2( rStrm );
        break;
        case xlBiff8:
            ReadFontData5( rStrm );
            ReadFontName8( rStrm );
        break;
        default:
            DBG_ERROR_BIFF();
            return;
    }
    Update();
}

void XclImpFont::FillToItemSet( SfxItemSet& rItemSet, XclFontWhichIDMode eMode ) const
{
    bool bEE = (eMode == xlFontEEIDs) || (eMode == xlFontHFIDs);

// aItem = the item to put into the item set
// nScID = the Calc which ID of the item
// nEEID = the edit engine which ID of the item
#define PUTITEM( aItem, nScID, nEEID )  rItemSet.Put( aItem, (bEE ? (nEEID) : (nScID)) )

// Font item - #91658# set only for valid script types
    CharSet eTempCharSet = (bEE && (meFontCharSet == GetCharSet())) ?
        ScfTools::GetSystemCharSet() : meFontCharSet;

    SvxFontItem aFontItem( meFontFamily, maData.maName, EMPTY_STRING, PITCH_DONTKNOW, eTempCharSet );
    if( mbIsWestern )
        PUTITEM( aFontItem, ATTR_FONT,      EE_CHAR_FONTINFO );
    if( mbIsAsian )
        PUTITEM( aFontItem, ATTR_CJK_FONT,  EE_CHAR_FONTINFO_CJK );
    if( mbIsComplex )
        PUTITEM( aFontItem, ATTR_CTL_FONT,  EE_CHAR_FONTINFO_CTL );

// Font height (for all script types)
    sal_Int32 nHeight = maData.mnHeight;
    if( eMode == xlFontEEIDs )  // do not convert header/footer height
        nHeight = (nHeight * 127 + 36) / EXC_POINTS_PER_INCH;   // #98527# 1 in == 72 pt

    SvxFontHeightItem aHeightItem( nHeight );
    PUTITEM( aHeightItem,   ATTR_FONT_HEIGHT,       EE_CHAR_FONTHEIGHT );
    PUTITEM( aHeightItem,   ATTR_CJK_FONT_HEIGHT,   EE_CHAR_FONTHEIGHT_CJK );
    PUTITEM( aHeightItem,   ATTR_CTL_FONT_HEIGHT,   EE_CHAR_FONTHEIGHT_CTL );

// Font color - pass AUTO_COL to item
    PUTITEM( SvxColorItem( GetPalette().GetColor( maData.mnColor ) ), ATTR_FONT_COLOR, EE_CHAR_COLOR );

// Font weight (for all script types)
    SvxWeightItem aWeightItem( GetScFontWeight( maData.mnWeight ) );
    PUTITEM( aWeightItem,   ATTR_FONT_WEIGHT,       EE_CHAR_WEIGHT );
    PUTITEM( aWeightItem,   ATTR_CJK_FONT_WEIGHT,   EE_CHAR_WEIGHT_CJK );
    PUTITEM( aWeightItem,   ATTR_CTL_FONT_WEIGHT,   EE_CHAR_WEIGHT_CTL );

// Font underline
    FontUnderline eUnderl = GetScFontUnderline( maData.meUnderline );
    if( bEE || (eUnderl != UNDERLINE_NONE) )
        PUTITEM( SvxUnderlineItem( eUnderl ), ATTR_FONT_UNDERLINE, EE_CHAR_UNDERLINE );

// Font posture (for all script types)
    FontItalic ePosture = GetScFontPosture( maData.mbItalic );
    if( bEE || (ePosture != ITALIC_NONE) )
    {
        SvxPostureItem aPostItem( ePosture );
        PUTITEM( aPostItem, ATTR_FONT_POSTURE,      EE_CHAR_ITALIC );
        PUTITEM( aPostItem, ATTR_CJK_FONT_POSTURE,  EE_CHAR_ITALIC_CJK );
        PUTITEM( aPostItem, ATTR_CTL_FONT_POSTURE,  EE_CHAR_ITALIC_CTL );
    }

// Boolean attributes crossed out, contoured, shadowed
    if( bEE || maData.mbStrikeout )
        PUTITEM( SvxCrossedOutItem( maData.mbStrikeout ? STRIKEOUT_SINGLE : STRIKEOUT_NONE ),
            ATTR_FONT_CROSSEDOUT, EE_CHAR_STRIKEOUT );
    if( bEE || maData.mbOutline )
        PUTITEM( SvxContourItem( maData.mbOutline ), ATTR_FONT_CONTOUR, EE_CHAR_OUTLINE );
    if( bEE || maData.mbShadow )
        PUTITEM( SvxShadowedItem( maData.mbShadow ), ATTR_FONT_SHADOWED, EE_CHAR_SHADOW );

// Super-/subscript: only on edit engine objects
    if( bEE )
        rItemSet.Put( SvxEscapementItem( GetScFontEscapement( maData.meEscapem ), EE_CHAR_ESCAPEMENT ) );

#undef PUTITEM
}

void XclImpFont::ReadFontData2( XclImpStream& rStrm )
{
    sal_uInt16 nFlags;
    rStrm >> maData.mnHeight >> nFlags;

    maData.meUnderline  = ::get_flagvalue( nFlags, EXC_FONTATTR_UNDERLINE, xlUnderlSingle, xlUnderlNone );
    maData.mnWeight     = ::get_flagvalue( nFlags, EXC_FONTATTR_BOLD, EXC_FONTWGHT_BOLD, EXC_FONTWGHT_NORMAL );
    maData.mbItalic     = ::get_flag( nFlags, EXC_FONTATTR_ITALIC );
    maData.mbStrikeout  = ::get_flag( nFlags, EXC_FONTATTR_STRIKEOUT );
    maData.mbOutline    = ::get_flag( nFlags, EXC_FONTATTR_OUTLINE );
    maData.mbShadow     = ::get_flag( nFlags, EXC_FONTATTR_SHADOW );
}

void XclImpFont::ReadFontData5( XclImpStream& rStrm )
{
    sal_uInt16 nFlags;

    rStrm   >> maData.mnHeight >> nFlags >> maData.mnColor >> maData.mnWeight
            >> maData.meEscapem >> maData.meUnderline >> maData.mnFamily >> maData.mnCharSet;
    rStrm.Ignore( 1 );

    maData.mbItalic     = ::get_flag( nFlags, EXC_FONTATTR_ITALIC );
    maData.mbStrikeout  = ::get_flag( nFlags, EXC_FONTATTR_STRIKEOUT );
    maData.mbOutline    = ::get_flag( nFlags, EXC_FONTATTR_OUTLINE );
    maData.mbShadow     = ::get_flag( nFlags, EXC_FONTATTR_SHADOW );
}

void XclImpFont::ReadFontName2( XclImpStream& rStrm )
{
    maData.maName.Erase();
    rStrm.AppendByteString( maData.maName, false );
}

void XclImpFont::ReadFontName8( XclImpStream& rStrm )
{
    maData.maName.Erase();
    rStrm.AppendUniString( maData.maName, rStrm.ReaduInt8() );
}

void XclImpFont::GuessScriptType()
{
    mbIsWestern = true;
    mbIsAsian = mbIsComplex = false;

    // #91658# find the script types for which the font contains characters
    SfxPrinter* pPrinter = GetPrinter();
    if( pPrinter )
    {
        Font aFont( maData.maName, Size( 0, 10 ) );
        FontCharMap aCharMap;

        pPrinter->SetFont( aFont );
        if( pPrinter->GetFontCharMap( aCharMap ) )
        {
            mbIsAsian = (aCharMap.HasChar( 0x4E01 ) ||      // CJK unified ideographs
                         aCharMap.HasChar( 0x7E01 ) ||      // CJK unified ideographs
                         aCharMap.HasChar( 0xAC01 ) ||      // Hangul syllables
                         aCharMap.HasChar( 0xCC01 ) ||      // Hangul syllables
                         aCharMap.HasChar( 0xF901 ));       // CJK compatibility ideographs
            //2do: complex characters

            mbIsWestern = aCharMap.HasChar( 'A' ) || (!mbIsAsian && !mbIsComplex);
        }
    }
}

void XclImpFont::Update()
{
    meFontFamily = GetScFontFamily( maData.mnFamily, maData.maName, GetCharSet() );
    meFontCharSet = GetScFontCharSet( maData.mnCharSet );
    GuessScriptType();
}

FontFamily XclImpFont::GetScFontFamily( sal_uInt8 nXclFamily, const String& rName, CharSet eDefCharSet )
{
    FontFamily eScFamily;
    // ! format differs from Windows documentation: family is in lower nibble, pitch unknown
    switch( nXclFamily & 0x0F )
    {
        case EXC_FONTFAM_ROMAN:         eScFamily = FAMILY_ROMAN;       break;
        case EXC_FONTFAM_SWISS:         eScFamily = FAMILY_SWISS;       break;
        case EXC_FONTFAM_MODERN:        eScFamily = FAMILY_MODERN;      break;
        case EXC_FONTFAM_SCRIPT:        eScFamily = FAMILY_SCRIPT;      break;
        case EXC_FONTFAM_DECORATIVE:    eScFamily = FAMILY_DECORATIVE;  break;
        default:
            eScFamily =
                ((eDefCharSet == RTL_TEXTENCODING_APPLE_ROMAN) &&
                (rName.EqualsAscii( "Geneva" ) || rName.EqualsAscii( "Chicago" ))) ?
                FAMILY_SWISS : FAMILY_DONTKNOW;
    }
    return eScFamily;
}

CharSet XclImpFont::GetScFontCharSet( sal_uInt8 nXclCharSet )
{
    return rtl_getTextEncodingFromWindowsCharset(nXclCharSet);
}

FontItalic XclImpFont::GetScFontPosture( bool bXclItalic )
{
    return bXclItalic ? ITALIC_NORMAL : ITALIC_NONE;
}

FontWeight XclImpFont::GetScFontWeight( sal_uInt16 nXclWeight )
{
    FontWeight eScWeight;

    if( !nXclWeight )               eScWeight = WEIGHT_DONTKNOW;
    else if( nXclWeight < 150 )     eScWeight = WEIGHT_THIN;
    else if( nXclWeight < 250 )     eScWeight = WEIGHT_ULTRALIGHT;
    else if( nXclWeight < 325 )     eScWeight = WEIGHT_LIGHT;
    else if( nXclWeight < 375 )     eScWeight = WEIGHT_SEMILIGHT;
    else if( nXclWeight < 450 )     eScWeight = WEIGHT_NORMAL;
    else if( nXclWeight < 550 )     eScWeight = WEIGHT_MEDIUM;
    else if( nXclWeight < 650 )     eScWeight = WEIGHT_SEMIBOLD;
    else if( nXclWeight < 750 )     eScWeight = WEIGHT_BOLD;
    else if( nXclWeight < 850 )     eScWeight = WEIGHT_ULTRABOLD;
    else                            eScWeight = WEIGHT_BLACK;

    return eScWeight;
}

FontUnderline XclImpFont::GetScFontUnderline( XclUnderline eXclUnderl )
{
    FontUnderline eScUnderl = UNDERLINE_NONE;
    switch( eXclUnderl )
    {
        case xlUnderlSingle:
        case xlUnderlSingleAcc: eScUnderl = UNDERLINE_SINGLE;  break;
        case xlUnderlDouble:
        case xlUnderlDoubleAcc: eScUnderl = UNDERLINE_DOUBLE;  break;
    }
    return eScUnderl;
}

SvxEscapement XclImpFont::GetScFontEscapement( XclEscapement eXclEscapem )
{
    SvxEscapement eScEscapem = SVX_ESCAPEMENT_OFF;
    switch( eXclEscapem )
    {
        case xlEscSuper:    eScEscapem = SVX_ESCAPEMENT_SUPERSCRIPT;    break;
        case xlEscSub:      eScEscapem = SVX_ESCAPEMENT_SUBSCRIPT;      break;
    }
    return eScEscapem;
}


// ----------------------------------------------------------------------------

XclImpFontBuffer::XclImpFontBuffer( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot )
{
    // application font for column width calculation, filled with first font from font list
    maAppFont.maName.AssignAscii( "Arial" );
    maAppFont.mnHeight = 200;
    SetCharWidth( 110 );
}

void XclImpFontBuffer::ReadFont( XclImpStream& rStrm )
{
    // Font with index 4 is not stored in an Excel file -> create a dummy font.
    if( maFontList.Count() == 4 )
        maFontList.Append( new XclImpFont( *GetFont( 0 ) ) );

    XclImpFont* pFont = new XclImpFont( GetRoot() );
    pFont->ReadFont( rStrm );
    maFontList.Append( pFont );

    if( maFontList.Count() == 1 )
    {
        maAppFont = pFont->GetFontData();
        // #i3006# Calculate the width of '0' from first font and current printer.
        SetCharWidth( pFont->GetCharWidth() );
    }
}

void XclImpFontBuffer::FillToItemSet( sal_uInt16 nFontIndex, SfxItemSet& rItemSet, XclFontWhichIDMode eMode ) const
{
    const XclImpFont* pFont = GetFont( (nFontIndex < maFontList.Count()) ? nFontIndex : 0 );
    if( pFont )
        pFont->FillToItemSet( rItemSet, eMode );
}


// FORMAT record - number formats =============================================

/** Stores the SvNumberFormatter index offset of an Excel built-in number format. */
struct XclImpBuiltInFormat
{
    sal_uInt32                  mnIndex;        /// Excel built-in index.
    NfIndexTableOffset          meOffset;       /// SvNumberFormatter format index.
    const sal_Char*             mpFormat;       /// Format string, if no format index exists.
    LanguageType                eLanguage;      /// Set hard language (default == 0 == system).
};

static const XclImpBuiltInFormat pBuiltInFormats[] =
{
    {   0,      NF_NUMBER_STANDARD              },  // General
    {   1,      NF_NUMBER_INT                   },  // 0
    {   2,      NF_NUMBER_DEC2                  },  // 0.00
    {   3,      NF_NUMBER_1000INT               },  // #,##0
    {   4,      NF_NUMBER_1000DEC2              },  // #,##0.00
    // 5...8 contained in file
    {   9,      NF_PERCENT_INT                  },  // 0%
    {   10,     NF_PERCENT_DEC2                 },  // 0.00%
    {   11,     NF_SCIENTIFIC_000E00            },  // 0.00E+00
    {   12,     NF_FRACTION_1                   },  // # ?/?
    {   13,     NF_FRACTION_2                   },  // # ??/??
    {   14,     NF_DATE_SYS_DDMMYY              },  // DD.MM.YY
    {   15,     NF_DATE_SYS_DMMMYY              },  // DD. MMM YY
    {   16,     NF_DATE_SYS_DDMMM               },  // DD. MMM
    {   17,     NF_DATE_SYS_MMYY                },  // MMM YY
    {   18,     NF_TIME_HHMMAMPM                },  // h:mm AM/PM
    {   19,     NF_TIME_HHMMSSAMPM              },  // h:mm:ss AM/PM
    {   20,     NF_TIME_HHMM                    },  // hh:mm
    {   21,     NF_TIME_HHMMSS                  },  // hh:mm:ss
    {   22,     NF_DATETIME_SYSTEM_SHORT_HHMM   },  // DD.MM.YYYY hh:mm
    // 23...36 international formats
    {   30,     NF_NUMBER_STANDARD,             "[$-0411]D/M/YY",           LANGUAGE_JAPANESE },    // ???
    {   31,     NF_NUMBER_STANDARD,             "[$-0411]YYYY年M月D日",  LANGUAGE_JAPANESE },
    {   33,     NF_NUMBER_STANDARD,             "[$-0411]hh時mm分ss秒",  LANGUAGE_JAPANESE },
    // ---
    {   37,     NF_NUMBER_STANDARD,             "#,##0 _$;-#,##0 _$",               },
    {   38,     NF_NUMBER_STANDARD,             "#,##0 _$;[RED]-#,##0 _$"           },
    {   39,     NF_NUMBER_STANDARD,             "#,##0.00 _$;-#,##0.00 _$"          },
    {   40,     NF_NUMBER_STANDARD,             "#,##0.00 _$;[RED]-#,##0.00 _$"     },
    // 41...44 contained in file
    {   45,     NF_NUMBER_STANDARD,             "mm:ss"                             },
    {   46,     NF_TIME_HH_MMSS                 },  // [h]:mm:ss
    {   47,     NF_TIME_MMSS00                  },  // mm:ss,0
    {   48,     NF_NUMBER_STANDARD,             "##0.0E+0"                          },
    {   49,     NF_TEXT                         },  // @
    // 50...??? international formats
    {   56,     NF_NUMBER_STANDARD,             "[$-0411]M月D日",         LANGUAGE_JAPANESE },
    {   57,     NF_NUMBER_STANDARD,             "[$-030411]GE.M.D",         LANGUAGE_JAPANESE },
    {   58,     NF_NUMBER_STANDARD,             "[$-030411]GGGE年M月D日",LANGUAGE_JAPANESE }
};


// ----------------------------------------------------------------------------

XclImpNumFmtBuffer::XclImpNumFmtBuffer( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot ),
    mnStdFmt( GetFormatter().GetStandardFormat( GetDefLanguage() ) )
{
}

sal_uInt32 XclImpNumFmtBuffer::GetFormat( sal_uInt32 nIndex )
{
    if( maKeyVec.empty() )
        InsertBuiltinFormats();

    return (nIndex < maKeyVec.size()) ? maKeyVec[ nIndex ] : mnStdFmt;
}

void XclImpNumFmtBuffer::ReadFormat( XclImpStream& rStrm )
{
    if( maKeyVec.empty() && (GetBiff() >= xlBiff5) )
        InsertBuiltinFormats();

    String aFormat;
    sal_uInt16 nIndex;
    bool bAppend = false;       // BIFF2-BIFF3: append, BIFF4+: read position from record

    switch( GetBiff() )
    {
        case xlBiff2:
        case xlBiff3:
            rStrm.AppendByteString( aFormat, false );
            bAppend = true;
        break;

        case xlBiff4:
            bAppend = true;     // In BIFF4 the index field exists, but is undefined
            // run through
        case xlBiff5:
        case xlBiff7:
            rStrm >> nIndex;
            rStrm.AppendByteString( aFormat, false );
        break;

        case xlBiff8:
            rStrm >> nIndex;
            rStrm.AppendUniString( aFormat );
        break;

        default:
            DBG_ERROR_BIFF();
            return;
    }

    xub_StrLen nCheckPos;
    sal_Int16 nType = NUMBERFORMAT_DEFINED;
    sal_uInt32 nKey;

    GetFormatter().PutandConvertEntry( aFormat, nCheckPos, nType, nKey,
        LANGUAGE_ENGLISH_US, GetDefLanguage() );

    if( bAppend )
        maKeyVec.push_back( nKey );
    else
        InsertKey( nIndex, nKey );
}

void XclImpNumFmtBuffer::InsertBuiltinFormats()
{
    const XclImpBuiltInFormat* pEnd = STATIC_TABLE_END( pBuiltInFormats );

    String aFormat;
    xub_StrLen nCheckPos;
    sal_Int16 nType = NUMBERFORMAT_DEFINED;
    sal_uInt32 nKey;

    for( const XclImpBuiltInFormat* pCurr = pBuiltInFormats; pCurr != pEnd; ++pCurr )
    {
        if( pCurr->mpFormat )
        {
            aFormat = String( pCurr->mpFormat, RTL_TEXTENCODING_UTF8 );
            GetFormatter().PutandConvertEntry( aFormat, nCheckPos, nType, nKey,
                LANGUAGE_ENGLISH_US, pCurr->eLanguage );
        }
        else
            nKey = GetFormatter().GetFormatIndex( pCurr->meOffset, pCurr->eLanguage );
        InsertKey( pCurr->mnIndex, nKey );
    }
}

void XclImpNumFmtBuffer::InsertKey( sal_uInt32 nIndex, sal_uInt32 nFormatKey )
{
    if( nIndex >= maKeyVec.size() )
        maKeyVec.resize( nIndex + 1, mnStdFmt );
    maKeyVec[ nIndex ] = nFormatKey;
}


// XF, STYLE record - Cell formatting =========================================

void lcl_SetBorder2( XclImpXFBorder& rBorder, sal_uInt8 nFlags )
{
    rBorder.mnLeftLine   = ::get_flagvalue( nFlags, EXC_XF2_LEFTLINE,   EXC_LINE_THIN, EXC_LINE_NONE );
    rBorder.mnRightLine  = ::get_flagvalue( nFlags, EXC_XF2_RIGHTLINE,  EXC_LINE_THIN, EXC_LINE_NONE );
    rBorder.mnTopLine    = ::get_flagvalue( nFlags, EXC_XF2_TOPLINE,    EXC_LINE_THIN, EXC_LINE_NONE );
    rBorder.mnBottomLine = ::get_flagvalue( nFlags, EXC_XF2_BOTTOMLINE, EXC_LINE_THIN, EXC_LINE_NONE );
    rBorder.mnLeftColor = rBorder.mnRightColor = rBorder.mnTopColor = rBorder.mnBottomColor = EXC_COLOR_BIFF2_BLACK;
}

void lcl_SetBorder3( XclImpXFBorder& rBorder, sal_uInt32 nBorder )
{
    ::extract_value( rBorder.mnTopLine,     nBorder,  0, 3 );
    ::extract_value( rBorder.mnLeftLine,    nBorder,  8, 3 );
    ::extract_value( rBorder.mnBottomLine,  nBorder, 16, 3 );
    ::extract_value( rBorder.mnRightLine,   nBorder, 24, 3 );
    ::extract_value( rBorder.mnTopColor,    nBorder,  3, 5 );
    ::extract_value( rBorder.mnLeftColor,   nBorder, 11, 5 );
    ::extract_value( rBorder.mnBottomColor, nBorder, 19, 5 );
    ::extract_value( rBorder.mnRightColor,  nBorder, 27, 5 );
}

void lcl_SetBorder5( XclImpXFBorder& rBorder, sal_uInt32 nBorder, sal_uInt32 nArea )
{
    ::extract_value( rBorder.mnTopLine,     nBorder,  0, 3 );
    ::extract_value( rBorder.mnLeftLine,    nBorder,  3, 3 );
    ::extract_value( rBorder.mnBottomLine,  nArea,   22, 3 );
    ::extract_value( rBorder.mnRightLine,   nBorder,  6, 3 );
    ::extract_value( rBorder.mnTopColor,    nBorder,  9, 7 );
    ::extract_value( rBorder.mnLeftColor,   nBorder, 16, 7 );
    ::extract_value( rBorder.mnBottomColor, nArea,   25, 7 );
    ::extract_value( rBorder.mnRightColor,  nBorder, 23, 7 );
}

void lcl_SetBorder8( XclImpXFBorder& rBorder, sal_uInt32 nBorder1, sal_uInt32 nBorder2 )
{
    ::extract_value( rBorder.mnLeftLine,    nBorder1,  0, 4 );
    ::extract_value( rBorder.mnRightLine,   nBorder1,  4, 4 );
    ::extract_value( rBorder.mnTopLine,     nBorder1,  8, 4 );
    ::extract_value( rBorder.mnBottomLine,  nBorder1, 12, 4 );
    ::extract_value( rBorder.mnLeftColor,   nBorder1, 16, 7 );
    ::extract_value( rBorder.mnRightColor,  nBorder1, 23, 7 );
    ::extract_value( rBorder.mnTopColor,    nBorder2,  0, 7 );
    ::extract_value( rBorder.mnBottomColor, nBorder2,  7, 7 );
}


// ----------------------------------------------------------------------------

void lcl_SetArea2( XclImpXFArea& rArea, sal_uInt8 nFlags )
{
    rArea.mnPattern = ::get_flagvalue( nFlags, EXC_XF2_BACKGROUND, EXC_PATT_12_5_PERC, EXC_PATT_NONE );
    rArea.mnForeColor = EXC_COLOR_BIFF2_BLACK;
    rArea.mnBackColor = EXC_COLOR_BIFF2_WHITE;
}

void lcl_SetArea3( XclImpXFArea& rArea, sal_uInt16 nArea )
{
    ::extract_value( rArea.mnPattern,     nArea,  0, 6 );
    ::extract_value( rArea.mnForeColor,   nArea,  6, 5 );
    ::extract_value( rArea.mnBackColor,   nArea, 11, 5 );
}

void lcl_SetArea5( XclImpXFArea& rArea, sal_uInt32 nArea )
{
    ::extract_value( rArea.mnPattern,     nArea, 16, 6 );
    ::extract_value( rArea.mnForeColor,   nArea,  0, 7 );
    ::extract_value( rArea.mnBackColor,   nArea,  7, 7 );
}

void lcl_SetArea8( XclImpXFArea& rArea, sal_uInt32 nBorder2, sal_uInt16 nArea )
{
    ::extract_value( rArea.mnPattern,     nBorder2, 26, 6 );
    ::extract_value( rArea.mnForeColor,   nArea,     0, 7 );
    ::extract_value( rArea.mnBackColor,   nArea,     7, 7 );
}


// ----------------------------------------------------------------------------

XclImpXF::XclImpXF( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot ),
    meHorAlign( xlHAlignGeneral ),
    meVerAlign( xlVAlignBottom ),
    meOrient( xlTextOrientNoRot ),
    meTextDir( xlTextDirContext ),
    mnParent( 0 ),
    mnNumFmt( 0 ),
    mnFont( 0 ),
    mnIndent( 0 ),
    mnRotation( 0 ),
    mbCellXF( true ),       // default is cell XF
    mbLocked( true ),       // default in Excel and Calc
    mbHidden( false ),
    mbWrapped( false ),
    mbProtUsed( false ),
    mbFontUsed( false ),
    mbFmtUsed( false ),
    mbAlignUsed( false ),
    mbBorderUsed( false ),
    mbAreaUsed( false )
{
}

XclImpXF::~XclImpXF()
{
}

void XclImpXF::ReadXF2( XclImpStream& rStrm )
{
    sal_uInt8 nNumFmt, nFlags;

    mnFont = rStrm.ReaduInt8();
    rStrm.Ignore( 1 );
    rStrm >> nNumFmt >> nFlags;

    lcl_SetBorder2( maBorder, nFlags );
    lcl_SetArea2( maArea, nFlags );
    mnNumFmt = GetNumFmtBuffer().GetFormat( nNumFmt & EXC_XF2_VALFMT_MASK );
    ::extract_value( meHorAlign, nFlags, 0, 3 );
    mbLocked = ::get_flag( nNumFmt, EXC_XF2_LOCKED );
    mbHidden = ::get_flag( nNumFmt, EXC_XF2_HIDDEN );

    //! TODO flags
    mbProtUsed = mbFontUsed = mbFmtUsed = mbAlignUsed = mbBorderUsed = mbAreaUsed = true;
}

void XclImpXF::ReadXF3( XclImpStream& rStrm )
{
    sal_uInt32 nBorder;
    sal_uInt16 nFlags, nAlign, nArea;
    sal_uInt8 nNumFmt;

    mnFont = rStrm.ReaduInt8();
    rStrm >> nNumFmt >> nFlags >> nAlign >> nArea >> nBorder;

    mbCellXF = !::get_flag( nFlags, EXC_XF_STYLE );     // new in BIFF3
    ::extract_value( mnParent, nAlign, 4, 12 );         // new in BIFF3
    lcl_SetBorder3( maBorder, nBorder );
    lcl_SetArea3( maArea, nArea );                      // new in BIFF3
    mnNumFmt = GetNumFmtBuffer().GetFormat( nNumFmt );
    ::extract_value( meHorAlign, nAlign, 0, 3 );
    mbWrapped = ::get_flag( nAlign, EXC_XF_WRAPPED );   // new in BIFF3
    mbLocked = ::get_flag( nFlags, EXC_XF_LOCKED );
    mbHidden = ::get_flag( nFlags, EXC_XF_HIDDEN );

    //! TODO flags
    mbProtUsed = mbFontUsed = mbFmtUsed = mbAlignUsed = mbBorderUsed = mbAreaUsed = true;
}

void XclImpXF::ReadXF4( XclImpStream& rStrm )
{
    sal_uInt32 nBorder;
    sal_uInt16 nFlags, nAlign, nArea;
    sal_uInt8 nNumFmt;

    mnFont = rStrm.ReaduInt8();
    rStrm >> nNumFmt >> nFlags >> nAlign >> nArea >> nBorder;

    mbCellXF = !::get_flag( nFlags, EXC_XF_STYLE );
    ::extract_value( mnParent, nFlags, 4, 12 );
    lcl_SetBorder3( maBorder, nBorder );
    lcl_SetArea3( maArea, nArea );
    mnNumFmt = GetNumFmtBuffer().GetFormat( nNumFmt );
    ::extract_value( meHorAlign, nAlign, 0, 3 );
    ::extract_value( meVerAlign, nAlign, 4, 2 );        // new in BIFF4
    mbWrapped = ::get_flag( nAlign, EXC_XF_WRAPPED );
    ::extract_value( meOrient, nAlign, 6, 2 );          // new in BIFF4
    mbLocked = ::get_flag( nFlags, EXC_XF_LOCKED );
    mbHidden = ::get_flag( nFlags, EXC_XF_HIDDEN );

    //! TODO flags
    mbProtUsed = mbFontUsed = mbFmtUsed = mbAlignUsed = mbBorderUsed = mbAreaUsed = true;
}

void XclImpXF::ReadXF5( XclImpStream& rStrm )
{
    sal_uInt32 nArea, nBorder;
    sal_uInt16 nNumFmt, nFlags, nAlign;
    rStrm >> mnFont >> nNumFmt >> nFlags >> nAlign >> nArea >> nBorder;

    mbCellXF = !::get_flag( nFlags, EXC_XF_STYLE );
    ::extract_value( mnParent, nFlags, 4, 12 );
    lcl_SetBorder5( maBorder, nBorder, nArea );
    lcl_SetArea5( maArea, nArea );
    mnNumFmt = GetNumFmtBuffer().GetFormat( nNumFmt );
    ::extract_value( meHorAlign, nAlign, 0, 3 );
    ::extract_value( meVerAlign, nAlign, 4, 3 );
    mbWrapped = ::get_flag( nAlign, EXC_XF_WRAPPED );
    ::extract_value( meOrient, nAlign, 8, 2 );
    mbLocked = ::get_flag( nFlags, EXC_XF_LOCKED );
    mbHidden = ::get_flag( nFlags, EXC_XF_HIDDEN );

    //! TODO flags
    mbProtUsed = mbFontUsed = mbFmtUsed = mbAlignUsed = mbBorderUsed = mbAreaUsed = true;
}

void XclImpXF::ReadXF8( XclImpStream& rStrm )
{
    sal_uInt32 nBorder1, nBorder2;
    sal_uInt16 nReadFont, nReadNumFmt, nTypeProt, nAlign, nMiscAttrib, nArea;
    rStrm >> nReadFont >> nReadNumFmt >> nTypeProt >> nAlign >> nMiscAttrib >> nBorder1 >> nBorder2 >> nArea;

    mbCellXF = !::get_flag( nTypeProt, EXC_XF_STYLE );
    if( mbCellXF )
        ::extract_value( mnParent, nTypeProt, 4, 12 );

    // Remark about finding the mb***Used flags:
    // - In cell XFs a *set* bit means a used attribute.
    // - In style XFs a *cleared* bit means a used attribute.
    // The mb***Used members always store true, if the attribute is used.
    // The "mbCellXF == ::get_flag(...)" construct evaluates to true in
    // both mentioned cases: cell XF and set bit; or style XF and cleared bit.
    sal_uInt8 nUsedFlags;
    ::extract_value( nUsedFlags, nMiscAttrib, 10, 6 );

    // Remark about using the attributes from the XFs:
    // Cell XF flag used (set) -> take from cell XF
    // Cell XF flag unused (cleared):
    //     Parent style XF flag used (cleared) -> take from style XF
    //     Parent style XF flag unused (set) -> take from cell XF (!)
    //! TODO not implemented yet

    // cell border
    mbBorderUsed = (mbCellXF == ::get_flag( nUsedFlags, EXC_XF_DIFF_BORDER ));
    if( mbCellXF || mbBorderUsed )     //! always in cell XFs until the parent XF is really read
        lcl_SetBorder8( maBorder, nBorder1, nBorder2 );

    // background area
    mbAreaUsed = (mbCellXF == ::get_flag( nUsedFlags, EXC_XF_DIFF_AREA ));
    if( mbCellXF || mbAreaUsed )
        lcl_SetArea8( maArea, nBorder2, nArea );

    // font
    mbFontUsed = (mbCellXF == ::get_flag( nUsedFlags, EXC_XF_DIFF_FONT ));
    if( mbCellXF || mbFontUsed )
        mnFont = nReadFont;

    // number format
    mbFmtUsed = (mbCellXF == ::get_flag( nUsedFlags, EXC_XF_DIFF_VALFMT ));
    if( mbCellXF || mbFmtUsed )
        mnNumFmt = GetNumFmtBuffer().GetFormat( nReadNumFmt );

    // alignment
    mbAlignUsed = (mbCellXF == ::get_flag( nUsedFlags, EXC_XF_DIFF_ALIGN ));
    if( mbCellXF || mbAlignUsed )
    {
        ::extract_value( meHorAlign, nAlign, 0, 3 );
        ::extract_value( meVerAlign, nAlign, 4, 3 );
        mbWrapped = ::get_flag( nAlign, EXC_XF_WRAPPED );
        ::extract_value( mnRotation, nAlign, 8, 8 );        // new in BIFF8
        meOrient = (mnRotation == EXC_XF8_STACKED) ? xlTextOrientTopBottom : xlTextOrientRot;
        ::extract_value( mnIndent, nMiscAttrib, 0, 4 );     // new in BIFF8
        mnIndent *= 200;
        ::extract_value( meTextDir, nMiscAttrib, 6, 2 );    // new in BIFF8X
    }

    // cell protection
    mbProtUsed = (mbCellXF == ::get_flag( nUsedFlags, EXC_XF_DIFF_PROT ));
    if( mbCellXF || mbProtUsed )
    {
        mbLocked = ::get_flag( nTypeProt, EXC_XF_LOCKED );
        mbHidden = ::get_flag( nTypeProt, EXC_XF_HIDDEN );
    }
}

void XclImpXF::ReadXF( XclImpStream& rStrm )
{
    switch( GetBiff() )
    {
        case xlBiff2:   ReadXF2( rStrm );   break;
        case xlBiff3:   ReadXF3( rStrm );   break;
        case xlBiff4:   ReadXF4( rStrm );   break;
        case xlBiff5:
        case xlBiff7:   ReadXF5( rStrm );   break;
        case xlBiff8:   ReadXF8( rStrm );   break;
        default:        DBG_ERROR_BIFF();
    }
}

const ScPatternAttr& XclImpXF::GetPattern() const
{
    if( mpPattern.get() )
        return *mpPattern;

    // create new set item
    mpPattern.reset( new ScPatternAttr( GetDoc().GetPool() ) );
    SfxItemSet& rItemSet = mpPattern->GetItemSet();

    // locked/hidden
    if( mbCellXF || mbProtUsed )
        rItemSet.Put( ScProtectionAttr( mbLocked, mbHidden ) );

    // font
    if( mbCellXF || mbFontUsed )
        GetFontBuffer().FillToItemSet( mnFont, rItemSet, xlFontScIDs );

    // value format
    if( mbCellXF || mbFmtUsed )
    {
        rItemSet.Put( SfxUInt32Item( ATTR_VALUE_FORMAT, static_cast< sal_uInt32 >( mnNumFmt ) ) );
        ScGlobal::AddLanguage( rItemSet, GetFormatter() );
    }

    // border
    if( mbCellXF || mbBorderUsed )
        SetBorder( rItemSet, GetPalette(), maBorder );

    // area
    if( mbCellXF || mbAreaUsed )
        SetArea( rItemSet, GetPalette(), maArea );

    // alignment
    if( mbCellXF || mbAlignUsed )
    {
        // horizontal alignment
        SvxCellHorJustify eHorJust = SVX_HOR_JUSTIFY_STANDARD;
        switch( meHorAlign )
        {
            case xlHAlignGeneral:       eHorJust = SVX_HOR_JUSTIFY_STANDARD;    break;
            case xlHAlignLeft:          eHorJust = SVX_HOR_JUSTIFY_LEFT;        break;
            case xlHAlignCenterAcrSel:
            case xlHAlignCenter:        eHorJust = SVX_HOR_JUSTIFY_CENTER;      break;
            case xlHAlignRight:         eHorJust = SVX_HOR_JUSTIFY_RIGHT;       break;
            case xlHAlignFill:          eHorJust = SVX_HOR_JUSTIFY_REPEAT;      break;
            case xlHAlignJustify:
            case xlHAlignDistrib:       eHorJust = SVX_HOR_JUSTIFY_BLOCK;       break;
            default:    DBG_ERRORFILE( "XclImpXF::GetPattern - unknown horizontal adjustment" );
        }
        rItemSet.Put( SvxHorJustifyItem( eHorJust ) );

        // text wrap
        SfxBoolItem aWrap( ATTR_LINEBREAK );
        aWrap.SetValue( mbWrapped );
        rItemSet.Put( aWrap );

        // vertical alignment
        SvxCellVerJustify eVertJust = SVX_VER_JUSTIFY_STANDARD;
        switch( meVerAlign )
        {
            case xlVAlignTop:       eVertJust = SVX_VER_JUSTIFY_TOP;    break;
            case xlVAlignCenter:    eVertJust = SVX_VER_JUSTIFY_CENTER; break;
            case xlVAlignBottom:    eVertJust = SVX_VER_JUSTIFY_BOTTOM; break;
            case xlVAlignJustify:
            case xlVAlignDistrib:   eVertJust = SVX_VER_JUSTIFY_TOP;    break;
            default:    DBG_ERRORFILE( "XclImpXF::GetPattern - unknown vertical adjustment" );
        }
        rItemSet.Put( SvxVerJustifyItem( eVertJust ) );

        // indent
        rItemSet.Put( SfxUInt16Item( ATTR_INDENT, mnIndent ) );

        // text orientation
        if( (meOrient == xlTextOrientRot) && (mnRotation != EXC_XF8_STACKED) )
        {
            // set an angle in the range from -90 to 90 degrees
            DBG_ASSERT( mnRotation <= 180, "XclImpXF::GetPattern - illegal rotation angle" );
            sal_Int32 nAngle = XclTools::GetScRotation( mnRotation );
            if( nAngle == 9000 )
                rItemSet.Put( SvxOrientationItem( SVX_ORIENTATION_BOTTOMTOP ) );
            else if( nAngle == 27000 )
                rItemSet.Put( SvxOrientationItem( SVX_ORIENTATION_TOPBOTTOM ) );
            rItemSet.Put( SfxInt32Item( ATTR_ROTATE_VALUE, nAngle ) );
            rItemSet.Put( SvxRotateModeItem( SVX_ROTATE_MODE_STANDARD, ATTR_ROTATE_MODE ) );
        }
        else
        {
            // set "stacked" or standard values for rotation
            SvxCellOrientation eSvxOrient = SVX_ORIENTATION_STANDARD;
            switch( meOrient )
            {
                case xlTextOrientNoRot:     eSvxOrient = SVX_ORIENTATION_STANDARD;   break;
                case xlTextOrientRot:
                    DBG_ASSERT( mnRotation == EXC_XF8_STACKED,
                        "XclImpXF::GetPattern - stacked expected (0xFF)" );
                // run through
                case xlTextOrientTopBottom: eSvxOrient = SVX_ORIENTATION_STACKED;    break;
                case xlTextOrient90ccw:     eSvxOrient = SVX_ORIENTATION_BOTTOMTOP;  break;
                case xlTextOrient90cw:      eSvxOrient = SVX_ORIENTATION_TOPBOTTOM;  break;
                default:    DBG_ERRORFILE( "XclImpXF::GetPattern - unknown text orientation" );
            }
            rItemSet.Put( SvxOrientationItem( eSvxOrient ) );
        }

        // CTL text direction
        SvxFrameDirection eFrameDir = FRMDIR_ENVIRONMENT;
        switch( meTextDir )
        {
            case xlTextDirContext:  eFrameDir = FRMDIR_ENVIRONMENT;     break;
            case xlTextDirLTR:      eFrameDir = FRMDIR_HORI_LEFT_TOP;   break;
            case xlTextDirRTL:      eFrameDir = FRMDIR_HORI_RIGHT_TOP;  break;
            default:    DBG_ERRORFILE( "XclImpXF::GetPattern - unknown CTL text direction" );
        }
        rItemSet.Put( SvxFrameDirectionItem( eFrameDir, ATTR_WRITINGDIR ) );
    }

    return *mpPattern;
}

SvxBorderLine* XclImpXF::CreateBorderLine( const XclImpPalette& rPalette, sal_uInt8 nXclLine, sal_uInt16 nXclColor )
{
    if( nXclLine == EXC_LINE_NONE )
        return NULL;

    static const sal_uInt16 ppnLineParam[][ 3 ] =
    {
        //  outer width,        inner width,        distance
        {   0,                  0,                  0 },                // 0 = none
        {   DEF_LINE_WIDTH_1,   0,                  0 },                // 1 = thin
        {   DEF_LINE_WIDTH_2,   0,                  0 },                // 2 = medium
        {   DEF_LINE_WIDTH_1,   0,                  0 },                // 3 = dashed
        {   DEF_LINE_WIDTH_0,   0,                  0 },                // 4 = dotted
        {   DEF_LINE_WIDTH_3,   0,                  0 },                // 5 = thick
        {   DEF_LINE_WIDTH_1,   DEF_LINE_WIDTH_1,   DEF_LINE_WIDTH_1 }, // 6 = double
        {   DEF_LINE_WIDTH_0,   0,                  0 },                // 7 = hair
        {   DEF_LINE_WIDTH_2,   0,                  0 },                // 8 = med dash
        {   DEF_LINE_WIDTH_1,   0,                  0 },                // 9 = thin dashdot
        {   DEF_LINE_WIDTH_2,   0,                  0 },                // A = med dashdot
        {   DEF_LINE_WIDTH_1,   0,                  0 },                // B = thin dashdotdot
        {   DEF_LINE_WIDTH_2,   0,                  0 },                // C = med dashdotdot
        {   DEF_LINE_WIDTH_2,   0,                  0 }                 // D = med slant dashdot
    };

    if( nXclLine >= STATIC_TABLE_SIZE( ppnLineParam ) )
        nXclLine = EXC_LINE_THIN;

    SvxBorderLine* pItem = new SvxBorderLine;
    pItem->SetColor( rPalette.GetColor( nXclColor, COL_BLACK ) );
    pItem->SetOutWidth( ppnLineParam[ nXclLine ][ 0 ] );
    pItem->SetInWidth( ppnLineParam[ nXclLine ][ 1 ] );
    pItem->SetDistance( ppnLineParam[ nXclLine ][ 2 ] );
    return pItem;
}

void XclImpXF::SetBorder( SfxItemSet& rItemSet, const XclImpPalette& rPalette, const XclImpXFBorder& rBorder )
{
    SvxBoxItem aBox( ATTR_BORDER );

    ::std::auto_ptr< SvxBorderLine > pLine;
    pLine.reset( CreateBorderLine( rPalette, rBorder.mnLeftLine, rBorder.mnLeftColor ) );
    aBox.SetLine( pLine.get(), BOX_LINE_LEFT );
    pLine.reset( CreateBorderLine( rPalette, rBorder.mnRightLine, rBorder.mnRightColor ) );
    aBox.SetLine( pLine.get(), BOX_LINE_RIGHT );
    pLine.reset( CreateBorderLine( rPalette, rBorder.mnTopLine, rBorder.mnTopColor ) );
    aBox.SetLine( pLine.get(), BOX_LINE_TOP );
    pLine.reset( CreateBorderLine( rPalette, rBorder.mnBottomLine, rBorder.mnBottomColor ) );
    aBox.SetLine( pLine.get(), BOX_LINE_BOTTOM );

    rItemSet.Put( aBox );
}

void XclImpXF::SetArea( SfxItemSet& rItemSet, const XclImpPalette& rPalette, const XclImpXFArea& rArea )
{
    static const sal_uInt16 pnRatioTable[] =
    {                                           // 0x8000 = 100%
        0x8000, 0x0000, 0x4000, 0x2000,         // 00 - 03
        0x6000, 0x4000, 0x4000, 0x4000,         // 04 - 07
        0x4000, 0x4000, 0x2000, 0x6000,         // 08 - 11
        0x6000, 0x6000, 0x6000, 0x4800,         // 12 - 15
        0x5000, 0x7000, 0x7800                  // 16 - 18
    };

    // no background -> set nothing!
    if( rArea.mnPattern != EXC_PATT_NONE )
    {
        Color aFore( rPalette.GetColor( rArea.mnForeColor, COL_WHITE ) );
        if( rArea.mnForeColor == 64 )
            aFore.SetColor( COL_BLACK );

        Color aBack( rPalette.GetColor( rArea.mnBackColor, COL_WHITE ) );
        if( rArea.mnBackColor == 64 )
            aBack.SetColor( COL_BLACK );

        if( rArea.mnPattern < STATIC_TABLE_SIZE( pnRatioTable ) )
            aFore = ScfTools::GetMixedColor( aFore, aBack, pnRatioTable[ rArea.mnPattern ] );

        rItemSet.Put( SvxBrushItem( aFore ) );
    }
}


// ----------------------------------------------------------------------------

XclImpXFBuffer::XclImpXFBuffer( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot ),
    maDefPattern( rRoot.GetDoc().GetPool() )
{
}

void XclImpXFBuffer::ReadXF( XclImpStream& rStrm )
{
    XclImpXF* pXF = new XclImpXF( GetRoot() );
    pXF->ReadXF( rStrm );
    maXFList.Append( pXF );
}

void XclImpXFBuffer::ReadStyle( XclImpStream& rStrm )
{
    if( GetBiff() != xlBiff8 )
    {
        DBG_ASSERT_BIFF( GetBiff() < xlBiff8 );
        return;
    }

    sal_uInt16 nXFIndex;
    rStrm >> nXFIndex;

    if( !::get_flag( nXFIndex, EXC_STYLE_BUILTIN ) )   // do not read built-in styles
    {
        nXFIndex &= EXC_STYLE_MASK;         // bits 0...11 are used for XF index
        String aStyleName( rStrm.ReadUniString() );
        if( aStyleName.Len() )
        {
            XclImpXF* pXF = maXFList.GetObject( nXFIndex );
            if( pXF && pXF->IsStyleXF() )
            {
                SfxItemSet& rItemSet = GetDoc().GetStyleSheetPool()->Make(
                    aStyleName, SFX_STYLE_FAMILY_PARA, SFXSTYLEBIT_USERDEF ).GetItemSet();
                const ScPatternAttr& rAttr = pXF->GetPattern();
                rItemSet.Put( rAttr.GetItemSet() );
            }
        }
    }
}

const ScPatternAttr& XclImpXFBuffer::GetPattern( sal_uInt32 nXFIndex ) const
{
    XclImpXF* pXF = maXFList.GetObject( nXFIndex );
    DBG_ASSERT( pXF || (GetBiff() == xlBiff2), "XclImpXFBuffer::GetPattern - XF not found" );
    return pXF ? pXF->GetPattern() : maDefPattern;
}

sal_uInt16 XclImpXFBuffer::GetFontIndex( sal_uInt32 nXFIndex ) const
{
    XclImpXF* pXF = maXFList.GetObject( nXFIndex );
    return pXF ? pXF->GetFont() : 0;
}

bool XclImpXFBuffer::HasEscapement( sal_uInt32 nXFIndex ) const
{
    const XclImpXF* pXF = maXFList.GetObject( nXFIndex );
    const XclImpFont* pFont = pXF ? GetFontBuffer().GetFont( pXF->GetFont() ) : NULL;
    return pFont ? pFont->HasEscapement() : false;
}


// Buffer for XF indexes in cells =============================================

IMPL_FIXEDMEMPOOL_NEWDEL( XclImpXFIndex, 100, 500 )

bool XclImpXFIndex::Expand( sal_uInt16 nRow, sal_uInt16 nXFIndex )
{
    if( mnXF != nXFIndex )
        return false;

    if( mnLastRow + 1 == nRow )
    {
        ++mnLastRow;
        return true;
    }
    if( mnFirstRow && (mnFirstRow - 1 == nRow) )
    {
        --mnFirstRow;
        return true;
    }

    return false;
}

bool XclImpXFIndex::Expand( const XclImpXFIndex& rNextStyle )
{
    DBG_ASSERT( mnLastRow < rNextStyle.mnFirstRow, "XclImpXFIndex::Expand - rows out of order" );
    if( (mnXF == rNextStyle.mnXF) && (mnLastRow + 1 == rNextStyle.mnFirstRow) )
    {
        mnLastRow = rNextStyle.mnLastRow;
        return true;
    }
    return false;
}


// ----------------------------------------------------------------------------

void XclImpXFIndexColumn::SetXF( sal_uInt16 nRow, sal_uInt16 nXFIndex )
{
    XclImpXFIndex* pPrevStyle;
    XclImpXFIndex* pNextStyle;
    sal_uInt32 nNextIndex;

    Find( pPrevStyle, pNextStyle, nNextIndex, nRow );

    // previous range:
    // try to overwrite XF (if row is contained in) or try to expand range
    if( pPrevStyle )
    {
        if( pPrevStyle->Contains( nRow ) )              // overwrite old XF
        {
            if( nXFIndex == pPrevStyle->mnXF )
                return;

            sal_uInt16 nFirst = pPrevStyle->mnFirstRow;
            sal_uInt16 nLast = pPrevStyle->mnLastRow;
            sal_uInt32 nIndex = nNextIndex - 1;
            XclImpXFIndex* pThisStyle = pPrevStyle;
            pPrevStyle = nIndex ? maIndexList.GetObject( nIndex - 1 ) : NULL;

            if( nFirst == nLast )                   // replace solely XF
            {
                pThisStyle->mnXF = nXFIndex;
                TryConcatPrev( nNextIndex );        // try to concat. next with this
                TryConcatPrev( nIndex );            // try to concat. this with previous
            }
            else if( nFirst == nRow )               // replace first XF
            {
                ++(pThisStyle->mnFirstRow);
                // try to concatenate with previous of this
                if( !pPrevStyle || !pPrevStyle->Expand( nRow, nXFIndex ) )
                    maIndexList.Insert( new XclImpXFIndex( nRow, nXFIndex ), nIndex );
            }
            else if( nLast == nRow )                // replace last XF
            {
                --(pThisStyle->mnLastRow);
                if( !pNextStyle || !pNextStyle->Expand( nRow, nXFIndex ) )
                    maIndexList.Insert( new XclImpXFIndex( nRow, nXFIndex ), nNextIndex );
            }
            else                                    // insert in the middle of the range
            {
                pThisStyle->mnFirstRow = nRow + 1;
                // List::Insert() moves entries towards end of list, so insert twice at nIndex
                maIndexList.Insert( new XclImpXFIndex( nRow, nXFIndex ), nIndex );
                maIndexList.Insert( new XclImpXFIndex( nFirst, nRow - 1, pThisStyle->mnXF ), nIndex );
            }
            return;
        }
        else if( pPrevStyle->Expand( nRow, nXFIndex ) ) // try to expand
        {
            TryConcatPrev( nNextIndex );    // try to concatenate next with expanded
            return;
        }
    }

    // try to expand next range
    if( pNextStyle && pNextStyle->Expand( nRow, nXFIndex ) )
        return;

    // create new range
    maIndexList.Insert( new XclImpXFIndex( nRow, nXFIndex ), nNextIndex );
}

void XclImpXFIndexColumn::Find(
        XclImpXFIndex*& rpPrevStyle,
        XclImpXFIndex*& rpNextStyle,
        sal_uInt32& rnNextIndex,
        sal_uInt16 nRow ) const
{

    // test whether list is empty
    if( maIndexList.Empty() )
    {
        rpPrevStyle = rpNextStyle = NULL;
        rnNextIndex = 0;
        return;
    }

    rpPrevStyle = maIndexList.GetObject( 0 );
    rpNextStyle = maIndexList.GetObject( maIndexList.Count() - 1 );

    // test whether row is at end of list (contained in or behind last range)
    // rpPrevStyle will contain a possible existing row
    if( rpNextStyle->mnFirstRow <= nRow )
    {
        rpPrevStyle = rpNextStyle;
        rpNextStyle = NULL;
        rnNextIndex = maIndexList.Count();
        return;
    }

    // test whether row is at beginning of list (really before first range)
    if( nRow < rpPrevStyle->mnFirstRow )
    {
        rpNextStyle = rpPrevStyle;
        rpPrevStyle = NULL;
        rnNextIndex = 0;
        return;
    }

    // loop: find style entries before and after new row
    // break the loop if there is no more range between first and last -or-
    // if rpPrevStyle contains nRow (rpNextStyle will never contain nRow)
    sal_uInt32 nPrevIndex = 0;
    sal_uInt32 nMidIndex;
    rnNextIndex = maIndexList.Count() - 1;
    XclImpXFIndex* pMidStyle;
    while( ((rnNextIndex - nPrevIndex) > 1) && (rpPrevStyle->mnLastRow < nRow) )
    {
        nMidIndex = (nPrevIndex + rnNextIndex) / 2;
        pMidStyle = maIndexList.GetObject( nMidIndex );
        DBG_ASSERT( pMidStyle, "XclImpXFIndexColumn::Find - missing style" );
        if( nRow < pMidStyle->mnFirstRow )   // row is really before pMidStyle
        {
            rpNextStyle = pMidStyle;
            rnNextIndex = nMidIndex;
        }
        else                                    // row is in or after pMidStyle
        {
            rpPrevStyle = pMidStyle;
            nPrevIndex = nMidIndex;
        }
    }

    // find next rpNextStyle if rpPrevStyle contains nRow
    if( nRow <= rpPrevStyle->mnLastRow )
    {
        rnNextIndex = nPrevIndex + 1;
        rpNextStyle = maIndexList.GetObject( rnNextIndex );
    }
}

void XclImpXFIndexColumn::TryConcatPrev( sal_uInt32 nIndex )
{
    if( !nIndex )
        return;

    XclImpXFIndex* pPrevStyle = maIndexList.GetObject( nIndex - 1 );
    XclImpXFIndex* pNextStyle = maIndexList.GetObject( nIndex );
    if( !pPrevStyle || !pNextStyle )
        return;

    if( pPrevStyle->Expand( *pNextStyle ) )
        maIndexList.Delete( nIndex );
}


// ----------------------------------------------------------------------------

const sal_uInt32 COLUMNBUFFERCOUNT = MAXCOL + 1;    /// Number of allowed columns.


XclImpXFIndexBuffer::XclImpXFIndexBuffer( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot ),
    mppColumns( new XclImpXFIndexColumnPtr[ COLUMNBUFFERCOUNT ] ),
    mnUsedCount( 0 )
{
}

XclImpXFIndexBuffer::~XclImpXFIndexBuffer()
{
    delete[] mppColumns;
}

void XclImpXFIndexBuffer::Clear()
{
    for( sal_uInt32 nCol = 0; nCol < mnUsedCount; ++nCol )
        mppColumns[ nCol ].reset();
    mnUsedCount = 0;

    maMergeList.RemoveAll();
}

void XclImpXFIndexBuffer::SetXF( sal_uInt16 nCol, sal_uInt16 nRow, sal_uInt16 nXFIndex, XclImpXFInsertMode eMode )
{
    DBG_ASSERT( (nCol <= MAXCOL) && (nRow <= MAXROW), "XclImpXFIndexBuffer::SetXF - out of range" );

    // set cell XF's
    if( !mppColumns[ nCol ].get() )
        mppColumns[ nCol ].reset( new XclImpXFIndexColumn );
    if( nCol >= mnUsedCount )
        mnUsedCount = nCol + 1;
    mppColumns[ nCol ]->SetXF( nRow, nXFIndex );

    // set "center across selection" attribute
    // #97130# ignore it on row default XFs
    if( eMode != xlXFModeRow )
    {
        const XclImpXF* pXF = GetXFBuffer().GetXF( nXFIndex );
        if( pXF && (pXF->GetHorAlign() == xlHAlignCenterAcrSel) )
        {
            // expand last merged range if this attribute is set repeatedly
            ScRange* pRange = maMergeList.Last();
            if( pRange && (pRange->aEnd.Row() == nRow) && (pRange->aEnd.Col() + 1 == nCol) && (eMode == xlXFModeBlank) )
                pRange->aEnd.IncCol();
            else
                SetMerge( nCol, nRow );
        }
    }
}

void XclImpXFIndexBuffer::SetXF( sal_uInt16 nCol, sal_uInt16 nRow, sal_uInt16 nXFIndex )
{
    SetXF( nCol, nRow, nXFIndex, xlXFModeCell );
}

void XclImpXFIndexBuffer::SetBlankXF( sal_uInt16 nCol, sal_uInt16 nRow, sal_uInt16 nXFIndex )
{
    SetXF( nCol, nRow, nXFIndex, xlXFModeBlank );
}

void XclImpXFIndexBuffer::SetRowDefXF( sal_uInt16 nRow, sal_uInt16 nXFIndex )
{
    for( sal_uInt16 nCol = 0; nCol <= MAXCOL; ++nCol )
        SetXF( nCol, nRow, nXFIndex, xlXFModeRow );
}

void XclImpXFIndexBuffer::SetBorderLine( const ScRange& rRange, sal_uInt16 nTab, sal_uInt16 nLine )
{
    sal_uInt16 nFromCol = (nLine == BOX_LINE_RIGHT) ? rRange.aEnd.Col() : rRange.aStart.Col();
    sal_uInt16 nFromRow = (nLine == BOX_LINE_BOTTOM) ? rRange.aEnd.Row() : rRange.aStart.Row();
    ScDocument& rDoc = GetDoc();

    const SvxBoxItem* pFromItem = static_cast< const SvxBoxItem* >(
        rDoc.GetAttr( nFromCol, nFromRow, nTab, ATTR_BORDER ) );
    const SvxBoxItem* pToItem = static_cast< const SvxBoxItem* >(
        rDoc.GetAttr( rRange.aStart.Col(), rRange.aStart.Row(), nTab, ATTR_BORDER ) );

    SvxBoxItem aNewItem( *pToItem );
    aNewItem.SetLine( pFromItem->GetLine( nLine ), nLine );
    rDoc.ApplyAttr( rRange.aStart.Col(), rRange.aStart.Row(), nTab, aNewItem );
}

void XclImpXFIndexBuffer::SetMerge( sal_uInt16 nCol, sal_uInt16 nRow )
{
    maMergeList.Append( ScRange( nCol, nRow, 0 ) );
}

void XclImpXFIndexBuffer::SetMerge( sal_uInt16 nFirstCol, sal_uInt16 nFirstRow, sal_uInt16 nLastCol, sal_uInt16 nLastRow )
{
    maMergeList.Append( ScRange( nFirstCol, nFirstRow, 0, nLastCol, nLastRow, 0 ) );
}

void XclImpXFIndexBuffer::Apply()
{
    ScDocument& rDoc = GetDoc();
    sal_uInt16 nTab = GetScTab();

    // apply patterns
    XclImpXFBuffer& rXFBuffer = GetXFBuffer();
    for( sal_uInt16 nCol = 0; nCol < mnUsedCount; ++nCol )
    {
        // apply all cell styles of an existing column
        if( mppColumns[ nCol ].get() )
        {
            XclImpXFIndexColumn& rColumn = *mppColumns[ nCol ];
            for( XclImpXFIndex* pStyle = rColumn.First(); pStyle; pStyle = rColumn.Next() )
                rDoc.ApplyPatternAreaTab( nCol, pStyle->mnFirstRow,
                    nCol, pStyle->mnLastRow, nTab, rXFBuffer.GetPattern( pStyle->mnXF ) );
        }
    }

    // apply cell merging
    for( const ScRange* pRange = maMergeList.First(); pRange; pRange = maMergeList.Next() )
    {
        bool bMultiCol = (pRange->aStart.Col() != pRange->aEnd.Col());
        bool bMultiRow = (pRange->aStart.Row() != pRange->aEnd.Row());
        // set correct right border
        if( bMultiCol )
            SetBorderLine( *pRange, nTab, BOX_LINE_RIGHT );
        // set correct lower border
        if( bMultiRow )
            SetBorderLine( *pRange, nTab, BOX_LINE_BOTTOM );
        // do merge
        if( bMultiCol || bMultiRow )
            rDoc.DoMerge( nTab, pRange->aStart.Col(), pRange->aStart.Row(),
                pRange->aEnd.Col(), pRange->aEnd.Row() );
    }

    Clear();
}


// ============================================================================

