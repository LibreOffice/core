/*************************************************************************
 *
 *  $RCSfile: xistyle.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: obo $ $Date: 2003-10-21 08:48:17 $
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
#ifndef SC_STLSHEET_HXX
#include "stlsheet.hxx"
#endif
#ifndef __GLOBSTR_HRC_
#include "globstr.hrc"
#endif

#ifndef SC_XLCONTENT_HXX
#include "xlcontent.hxx"
#endif
#ifndef SC_XISTREAM_HXX
#include "xistream.hxx"
#endif


#include "root.hxx"

// helpers ====================================================================

/** Puts the item into the item set.
    @descr  The item will be put into the item set, if bSkipPoolDef is false,
    or if the item differs from the default pool item.
    @param rItemSet  The destination item set.
    @param rItem  The item to put into the item set.
    @param nWhichId  The Which-ID to set with the item.
    @param bSkipPoolDef  true = Do not put item if it is equal to pool default; false = Always put the item. */
void lcl_xistyle_PutItem(
        SfxItemSet& rItemSet, const SfxPoolItem& rItem, sal_uInt16 nWhichId, bool bSkipPoolDef )
{
    if( !bSkipPoolDef || (rItem != rItemSet.GetPool()->GetDefaultItem( nWhichId )) )
        rItemSet.Put( rItem, nWhichId );
}

/** Puts the item into the item set.
    @descr  The item will be put into the item set, if bSkipPoolDef is false,
    or if the item differs from the default pool item.
    @param rItemSet  The destination item set.
    @param rItem  The item to put into the item set.
    @param bSkipPoolDef  true = Do not put item if it is equal to pool default; false = Always put the item. */
inline void lcl_xistyle_PutItem( SfxItemSet& rItemSet, const SfxPoolItem& rItem, bool bSkipPoolDef )
{
    lcl_xistyle_PutItem( rItemSet, rItem, rItem.Which(), bSkipPoolDef );
}


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

ColorData XclImpPalette::GetColorData( sal_uInt16 nXclIndex ) const
{
    if( nXclIndex >= EXC_COLOR_USEROFFSET )
    {
        sal_uInt32 nIx = nXclIndex - EXC_COLOR_USEROFFSET;
        if( nIx < maColorTable.size() )
            return maColorTable[ nIx ];
    }
    return GetDefColorData( nXclIndex );
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

XclImpFont::XclImpFont( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot ),
    mbAscii( true ),
    mbCjk( false ),
    mbCtl( false )
{
    SetAllUsedFlags( false );
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
                    maData.SetScWeight( aFontInfo.GetWeight() );
                    maData.SetScPosture( aFontInfo.GetItalic() );
                }
            }
        }
        maData.maStyle.Erase();
    }
    GuessScriptType();
    SetAllUsedFlags( true );
}

void XclImpFont::SetAllUsedFlags( bool bUsed )
{
    mbFontNameUsed = mbHeightUsed = mbColorUsed = mbWeightUsed = mbEscapemUsed =
        mbUnderlUsed = mbItalicUsed = mbStrikeUsed = mbOutlineUsed = mbShadowUsed = bUsed;
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
    GuessScriptType();
    SetAllUsedFlags( true );
}

void XclImpFont::ReadCFFontBlock( XclImpStream& rStrm )
{
    DBG_ASSERT_BIFF( GetBiff() == xlBiff8 );
    if( GetBiff() != xlBiff8 )
        return;

    sal_uInt32 nHeight, nStyle, nColor, nFontFlags1, nFontFlags2, nFontFlags3;
    sal_uInt16 nWeight, nEscapem;
    sal_uInt8 nUnderl;

    rStrm.Ignore( 64 );
    rStrm >> nHeight >> nStyle >> nWeight >> nEscapem >> nUnderl;
    rStrm.Ignore( 3 );
    rStrm >> nColor;
    rStrm.Ignore( 4 );
    rStrm >> nFontFlags1 >> nFontFlags2 >> nFontFlags3;
    rStrm.Ignore( 18 );

    if( (mbHeightUsed = (nHeight <= 0x7FFF)) == true )
        maData.mnHeight = static_cast< sal_uInt16 >( nHeight );
    if( (mbWeightUsed = !::get_flag( nFontFlags1, EXC_CF_FONT_STYLE ) && (nWeight < 0x7FFF)) == true )
        maData.mnWeight = static_cast< sal_uInt16 >( nWeight );
    if( (mbItalicUsed = !::get_flag( nFontFlags1, EXC_CF_FONT_STYLE )) == true )
        maData.mbItalic = ::get_flag( nStyle, EXC_CF_FONT_STYLE );
    if( (mbUnderlUsed = !::get_flag( nFontFlags3, EXC_CF_FONT_UNDERL ) && (nUnderl <= 0x7FFF)) == true )
        maData.mnUnderline = nUnderl;
    if( (mbColorUsed = (nColor <= 0x7FFF)) == true )
        maData.mnColor = static_cast< sal_uInt16 >( nColor );
    if( (mbStrikeUsed = !::get_flag( nFontFlags1, EXC_CF_FONT_STRIKEOUT )) == true )
        maData.mbStrikeout = ::get_flag( nStyle, EXC_CF_FONT_STRIKEOUT );
}

void XclImpFont::FillToItemSet( SfxItemSet& rItemSet, XclFontWhichIDMode eMode, bool bSkipPoolDefs ) const
{
    bool bEE = (eMode == xlFontEEIDs) || (eMode == xlFontHFIDs);

// item = the item to put into the item set
// sc_which = the Calc Which-ID of the item
// ee_which = the edit engine Which-ID of the item
#define PUTITEM( item, sc_which, ee_which ) \
    lcl_xistyle_PutItem( rItemSet, item, (bEE ? (ee_which) : (sc_which)), bSkipPoolDefs )

// Font item - #91658# set only for valid script types
    if( mbFontNameUsed )
    {
        CharSet eFontCharSet = maData.GetScCharSet();
        CharSet eTempCharSet = (bEE && (eFontCharSet == GetCharSet())) ?
            ScfTools::GetSystemCharSet() : eFontCharSet;

        SvxFontItem aFontItem( maData.GetScFamily( GetCharSet() ), maData.maName, EMPTY_STRING, PITCH_DONTKNOW, eTempCharSet );
        if( mbAscii )
            PUTITEM( aFontItem, ATTR_FONT,      EE_CHAR_FONTINFO );
        if( mbCjk )
            PUTITEM( aFontItem, ATTR_CJK_FONT,  EE_CHAR_FONTINFO_CJK );
        if( mbCtl )
            PUTITEM( aFontItem, ATTR_CTL_FONT,  EE_CHAR_FONTINFO_CTL );
    }

// Font height (for all script types)
    if( mbHeightUsed )
    {
        sal_Int32 nHeight = maData.mnHeight;
        if( eMode == xlFontEEIDs )  // do not convert header/footer height
            nHeight = (nHeight * 127 + 36) / EXC_POINTS_PER_INCH;   // #98527# 1 in == 72 pt

        SvxFontHeightItem aHeightItem( nHeight );
        PUTITEM( aHeightItem,   ATTR_FONT_HEIGHT,       EE_CHAR_FONTHEIGHT );
        PUTITEM( aHeightItem,   ATTR_CJK_FONT_HEIGHT,   EE_CHAR_FONTHEIGHT_CJK );
        PUTITEM( aHeightItem,   ATTR_CTL_FONT_HEIGHT,   EE_CHAR_FONTHEIGHT_CTL );
    }

// Font color - pass AUTO_COL to item
    if( mbColorUsed )
        PUTITEM( SvxColorItem( GetPalette().GetColor( maData.mnColor ) ), ATTR_FONT_COLOR, EE_CHAR_COLOR );

// Font weight (for all script types)
    if( mbWeightUsed )
    {
        SvxWeightItem aWeightItem( maData.GetScWeight() );
        PUTITEM( aWeightItem,   ATTR_FONT_WEIGHT,       EE_CHAR_WEIGHT );
        PUTITEM( aWeightItem,   ATTR_CJK_FONT_WEIGHT,   EE_CHAR_WEIGHT_CJK );
        PUTITEM( aWeightItem,   ATTR_CTL_FONT_WEIGHT,   EE_CHAR_WEIGHT_CTL );
    }

// Font underline
    if( mbUnderlUsed )
    {
        SvxUnderlineItem aUnderlItem( maData.GetScUnderline() );
        PUTITEM( aUnderlItem,   ATTR_FONT_UNDERLINE,    EE_CHAR_UNDERLINE );
    }

// Font posture (for all script types)
    if( mbItalicUsed )
    {
        SvxPostureItem aPostItem( maData.GetScPosture() );
        PUTITEM( aPostItem, ATTR_FONT_POSTURE,      EE_CHAR_ITALIC );
        PUTITEM( aPostItem, ATTR_CJK_FONT_POSTURE,  EE_CHAR_ITALIC_CJK );
        PUTITEM( aPostItem, ATTR_CTL_FONT_POSTURE,  EE_CHAR_ITALIC_CTL );
    }

// Boolean attributes crossed out, contoured, shadowed
    if( mbStrikeUsed )
        PUTITEM( SvxCrossedOutItem( maData.GetScStrikeout() ), ATTR_FONT_CROSSEDOUT, EE_CHAR_STRIKEOUT );
    if( mbOutlineUsed )
        PUTITEM( SvxContourItem( maData.mbOutline ), ATTR_FONT_CONTOUR, EE_CHAR_OUTLINE );
    if( mbShadowUsed )
        PUTITEM( SvxShadowedItem( maData.mbShadow ), ATTR_FONT_SHADOWED, EE_CHAR_SHADOW );

// Super-/subscript: only on edit engine objects
    if( mbEscapemUsed && bEE )
        rItemSet.Put( SvxEscapementItem( maData.GetScEscapement(), EE_CHAR_ESCAPEMENT ) );

#undef PUTITEM
}

void XclImpFont::ReadFontData2( XclImpStream& rStrm )
{
    sal_uInt16 nFlags;
    rStrm >> maData.mnHeight >> nFlags;

    maData.mnWeight     = ::get_flagvalue( nFlags, EXC_FONTATTR_BOLD, EXC_FONTWGHT_BOLD, EXC_FONTWGHT_NORMAL );
    maData.mnUnderline  = ::get_flagvalue( nFlags, EXC_FONTATTR_UNDERLINE, EXC_FONTUNDERL_SINGLE, EXC_FONTUNDERL_NONE );
    maData.mbItalic     = ::get_flag( nFlags, EXC_FONTATTR_ITALIC );
    maData.mbStrikeout  = ::get_flag( nFlags, EXC_FONTATTR_STRIKEOUT );
    maData.mbOutline    = ::get_flag( nFlags, EXC_FONTATTR_OUTLINE );
    maData.mbShadow     = ::get_flag( nFlags, EXC_FONTATTR_SHADOW );
}

void XclImpFont::ReadFontData5( XclImpStream& rStrm )
{
    sal_uInt16 nFlags;

    rStrm   >> maData.mnHeight >> nFlags >> maData.mnColor >> maData.mnWeight
            >> maData.mnEscapem >> maData.mnUnderline >> maData.mnFamily >> maData.mnCharSet;
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
    mbAscii = true;
    mbCjk = mbCtl = false;

    // #91658# find the script types for which the font contains characters
    if( SfxPrinter* pPrinter = GetPrinter() )
    {
        Font aFont( maData.maName, Size( 0, 10 ) );
        FontCharMap aCharMap;

        pPrinter->SetFont( aFont );
        if( pPrinter->GetFontCharMap( aCharMap ) )
        {
            mbCjk = (aCharMap.HasChar( 0x4E01 ) ||      // CJK unified ideographs
                     aCharMap.HasChar( 0x7E01 ) ||      // CJK unified ideographs
                     aCharMap.HasChar( 0xAC01 ) ||      // Hangul syllables
                     aCharMap.HasChar( 0xCC01 ) ||      // Hangul syllables
                     aCharMap.HasChar( 0xF901 ));       // CJK compatibility ideographs
            mbCtl = false;  //! TODO
            mbAscii = aCharMap.HasChar( 'A' ) || (!mbCjk && !mbCtl);
        }
    }
}


// ----------------------------------------------------------------------------

XclImpFontBuffer::XclImpFontBuffer( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot )
{
    /*  Application font for column width calculation,
        later filled with first font from font list. */
    maAppFont.maName.AssignAscii( "Arial" );
    maAppFont.mnHeight = 200;
    SetCharWidth( maAppFont );
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
        SetCharWidth( maAppFont );
    }
}

void XclImpFontBuffer::FillToItemSet(
        SfxItemSet& rItemSet, XclFontWhichIDMode eMode,
        sal_uInt16 nFontIndex, bool bSkipPoolDefs ) const
{
    if( const XclImpFont* pFont = GetFont( nFontIndex ) )
        pFont->FillToItemSet( rItemSet, eMode, bSkipPoolDefs );
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

    // 14...22 date and time formats
    {   14,     NF_DATE_SYS_DDMMYYYY            },
    {   15,     NF_NUMBER_STANDARD,             "D-MMM-YY",                 LANGUAGE_ENGLISH_US },
    {   15,     NF_NUMBER_STANDARD,             "DD. MMM YY",               LANGUAGE_GERMAN },
    {   15,     NF_DATE_SYS_DMMMYY              },
    {   16,     NF_NUMBER_STANDARD,             "DD-MMM",                   LANGUAGE_ENGLISH_US },
    {   16,     NF_NUMBER_STANDARD,             "DD. MMM",                  LANGUAGE_GERMAN },
    {   16,     NF_DATE_SYS_DDMMM               },
    {   17,     NF_NUMBER_STANDARD,             "MMM-YY",                   LANGUAGE_ENGLISH_US },
    {   17,     NF_NUMBER_STANDARD,             "MMM YY",                   LANGUAGE_GERMAN },
    {   17,     NF_DATE_SYS_MMYY                },
    {   18,     NF_NUMBER_STANDARD,             "h:mm AM/PM",               LANGUAGE_ENGLISH_US },
    {   18,     NF_NUMBER_STANDARD,             "h:mm AM/PM",               LANGUAGE_GERMAN },
    {   18,     NF_TIME_HHMMAMPM                },
    {   19,     NF_NUMBER_STANDARD,             "h:mm:ss AM/PM",            LANGUAGE_ENGLISH_US },
    {   19,     NF_NUMBER_STANDARD,             "h:mm:ss AM/PM",            LANGUAGE_GERMAN },
    {   19,     NF_TIME_HHMMSSAMPM              },
    {   20,     NF_NUMBER_STANDARD,             "h:mm",                     LANGUAGE_ENGLISH_US },
    {   20,     NF_TIME_HHMM                    },
    {   21,     NF_NUMBER_STANDARD,             "h:mm:ss",                  LANGUAGE_ENGLISH_US },
    {   21,     NF_TIME_HHMMSS                  },
    {   22,     NF_NUMBER_STANDARD,             "M/D/YYYY h:mm",            LANGUAGE_ENGLISH_US },
    {   22,     NF_NUMBER_STANDARD,             "DD.MM.YYYY hh:mm",         LANGUAGE_GERMAN },
    {   22,     NF_NUMBER_STANDARD,             "[$-0404]YYYY/M/D hh:mm",   LANGUAGE_CHINESE_TRADITIONAL },
    {   22,     NF_DATETIME_SYSTEM_SHORT_HHMM   },

    // 23...36 international formats
    {   27,     NF_DATE_SYS_DDMMYYYY            },  // default
    {   28,     NF_DATE_SYS_DDMMYYYY            },  // default
    {   29,     NF_DATE_SYS_DDMMYYYY            },  // default

    {   30,     NF_NUMBER_STANDARD,             "[$-0411]M/D/YY",           LANGUAGE_JAPANESE },
    {   30,     NF_NUMBER_STANDARD,             "[$-0412]M-D-YY",           LANGUAGE_KOREAN },
    {   30,     NF_NUMBER_STANDARD,             "[$-0404]M/D/YY",           LANGUAGE_CHINESE_TRADITIONAL },
    {   30,     NF_NUMBER_STANDARD,             "[$-0804]M/D/YY",           LANGUAGE_CHINESE_SIMPLIFIED },
    {   30,     NF_DATE_SYS_DDMMYYYY            },  // default

    {   31,     NF_NUMBER_STANDARD,             "[$-0411]YYYY年M月D日",   LANGUAGE_JAPANESE },
    {   31,     NF_NUMBER_STANDARD,             "[$-0412]YYYY년 M월 D일", LANGUAGE_KOREAN },
    {   31,     NF_NUMBER_STANDARD,             "[$-0404]YYYY年M月D日",   LANGUAGE_CHINESE_TRADITIONAL },
    {   31,     NF_NUMBER_STANDARD,             "[$-0804]YYYY年M月D日",   LANGUAGE_CHINESE_SIMPLIFIED },
    {   31,     NF_DATE_SYS_DDMMYYYY            },  // default

    {   32,     NF_NUMBER_STANDARD,             "[$-0411]HH時MM分",       LANGUAGE_JAPANESE },
    {   32,     NF_NUMBER_STANDARD,             "[$-0412]H시 MM분",       LANGUAGE_KOREAN },
    {   32,     NF_NUMBER_STANDARD,             "[$-0404]HH時MM分",       LANGUAGE_CHINESE_TRADITIONAL },
    {   32,     NF_NUMBER_STANDARD,             "[$-0804]HH时MM分" ,      LANGUAGE_CHINESE_SIMPLIFIED },
    {   32,     NF_TIME_HHMMSS                  },  // default

    {   33,     NF_NUMBER_STANDARD,             "[$-0411]HH時MM分SS秒",  LANGUAGE_JAPANESE  },
    {   33,     NF_NUMBER_STANDARD,             "[$-0412]H시 MM분 SS초", LANGUAGE_KOREAN  },
    {   33,     NF_NUMBER_STANDARD,             "[$-0404]HH時MM分SS秒",  LANGUAGE_CHINESE_TRADITIONAL  },
    {   33,     NF_NUMBER_STANDARD,             "[$-0804]HH时MM分SS秒",  LANGUAGE_CHINESE_SIMPLIFIED  },
    {   33,     NF_TIME_HHMMSS                  },  // default

    {   34,     NF_TIME_HHMMSS                  },  // default
    {   35,     NF_TIME_HHMMSS                  },  // default
    {   36,     NF_DATE_SYS_DDMMYYYY            },  // default

    // ---
    {   37,     NF_NUMBER_STANDARD,             "#,##0 _$;-#,##0 _$",               },
    {   38,     NF_NUMBER_STANDARD,             "#,##0 _$;[RED]-#,##0 _$"           },
    {   39,     NF_NUMBER_STANDARD,             "#,##0.00 _$;-#,##0.00 _$"          },
    {   40,     NF_NUMBER_STANDARD,             "#,##0.00 _$;[RED]-#,##0.00 _$"     },

    // 41...44 contained in file
    {   45,     NF_NUMBER_STANDARD,             "mm:ss"                     },
    {   46,     NF_NUMBER_STANDARD,             "[h]:mm:ss",                LANGUAGE_ENGLISH_US },
    {   46,     NF_NUMBER_STANDARD,             "[h]:mm:ss",                LANGUAGE_GERMAN },
    {   46,     NF_TIME_HH_MMSS                 },  // default
    {   47,     NF_NUMBER_STANDARD,             "mm:ss.0",                  LANGUAGE_ENGLISH_US },
    {   47,     NF_NUMBER_STANDARD,             "mm:ss.0",                  LANGUAGE_GERMAN },
    {   47,     NF_TIME_MMSS00                  },  // default
    {   48,     NF_NUMBER_STANDARD,             "##0.0E+0"                  },
    {   49,     NF_TEXT                         },  // @

    // 50...??? international formats
    {   50,     NF_DATE_SYS_DDMMYYYY            },  // default
    {   51,     NF_DATE_SYS_DDMMYYYY            },  // default
    {   52,     NF_DATE_SYS_DDMMYYYY            },  // default
    {   53,     NF_DATE_SYS_DDMMYYYY            },  // default
    {   54,     NF_DATE_SYS_DDMMYYYY            },  // default

    {   55,     NF_NUMBER_STANDARD,             "[$-0411]YYYY年M月",      LANGUAGE_JAPANESE },
    {   55,     NF_NUMBER_STANDARD,             "[$-0412]YYYY-M-D",         LANGUAGE_KOREAN },
    {   55,     NF_NUMBER_STANDARD,             "[$-0404]AM/PM HH時MM分", LANGUAGE_CHINESE_TRADITIONAL },
    {   55,     NF_NUMBER_STANDARD,             "[$-0804]AM/PM HH时MM分", LANGUAGE_CHINESE_SIMPLIFIED },
    {   55,     NF_DATE_SYS_DDMMYYYY            },  // default

    {   56,     NF_NUMBER_STANDARD,             "[$-0411]M月D日",                 LANGUAGE_JAPANESE },
    // unable to locate Korean
    {   56,     NF_NUMBER_STANDARD,             "[$-0404]AM/PM HH時MM分SS秒",    LANGUAGE_CHINESE_TRADITIONAL  },
    {   56,     NF_NUMBER_STANDARD,             "[$-0804]AM/PM HH时MM分SS秒",    LANGUAGE_CHINESE_SIMPLIFIED },
    {   56,     NF_DATE_SYS_DDMMYYYY            },  // default

    {   57,     NF_NUMBER_STANDARD,             "[$-030411]GE.M.D",             LANGUAGE_JAPANESE },
    {   57,     NF_NUMBER_STANDARD,             "[$-0412]YYYY年 M月 D日",    LANGUAGE_KOREAN },
    {   57,     NF_NUMBER_STANDARD,             "[$-0404]YYYY/M/D",             LANGUAGE_CHINESE_TRADITIONAL },
    {   57,     NF_NUMBER_STANDARD,             "[$-0804]YYYY年M日",          LANGUAGE_CHINESE_SIMPLIFIED },
    {   57,     NF_DATE_SYS_DDMMYYYY            },  // default

    {   58,     NF_NUMBER_STANDARD,             "[$-030411]GGGE年M月D日",    LANGUAGE_JAPANESE },
    {   58,     NF_NUMBER_STANDARD,             "[$-0412]M-D",                  LANGUAGE_KOREAN },
    {   58,     NF_NUMBER_STANDARD,             "[$-0404]YYYY年M月D日",      LANGUAGE_CHINESE_TRADITIONAL },
    {   58,     NF_NUMBER_STANDARD,             "[$-0804]M月D日",             LANGUAGE_CHINESE_SIMPLIFIED } ,
    {   58,     NF_DATE_SYS_DDMMYYYY            }  // default
};


// ----------------------------------------------------------------------------

XclImpNumFmtBuffer::XclImpNumFmtBuffer( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot ),
    mnStdFmt( GetFormatter().GetStandardFormat( ScGlobal::eLnge ) )
{
}

sal_uInt32 XclImpNumFmtBuffer::GetFormat( sal_uInt16 nNumFmt ) const
{
    if( maKeyVec.empty() )
        const_cast< XclImpNumFmtBuffer* >( this )->InsertBuiltinFormats();

    return (nNumFmt < maKeyVec.size()) ? maKeyVec[ nNumFmt ] : mnStdFmt;
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
        LANGUAGE_ENGLISH_US, ScGlobal::eLnge );

    if( bAppend )
        maKeyVec.push_back( nKey );
    else
        InsertKey( nIndex, nKey );
}

void XclImpNumFmtBuffer::FillToItemSet( SfxItemSet& rItemSet, sal_uInt16 nNumFmt, bool bSkipPoolDefs ) const
{
    FillScFmtToItemSet( rItemSet, GetFormat( nNumFmt ), bSkipPoolDefs );
}

void XclImpNumFmtBuffer::FillScFmtToItemSet( SfxItemSet& rItemSet, sal_uInt32 nScNumFmt, bool bSkipPoolDefs ) const
{
    lcl_xistyle_PutItem( rItemSet, SfxUInt32Item( ATTR_VALUE_FORMAT, nScNumFmt ), bSkipPoolDefs );
    if( rItemSet.GetItemState( ATTR_VALUE_FORMAT, FALSE ) == SFX_ITEM_SET )
        ScGlobal::AddLanguage( rItemSet, GetFormatter() );
}

void XclImpNumFmtBuffer::InsertBuiltinFormats()
{
    const XclImpBuiltInFormat* pEnd = STATIC_TABLE_END( pBuiltInFormats );

    String aFormat;
    xub_StrLen nCheckPos;
    sal_Int16 nType = NUMBERFORMAT_DEFINED;
    sal_uInt32 nKey;
    sal_uInt32 nSaveIndex = NUMBERFORMAT_ENTRY_NOT_FOUND;

    for( const XclImpBuiltInFormat* pCurr = pBuiltInFormats; pCurr != pEnd; ++pCurr )
    {
        if((pCurr->eLanguage == LANGUAGE_SYSTEM || pCurr->eLanguage == GetSysLanguage()) &&
            nSaveIndex != pCurr->mnIndex)
        {
            if( pCurr->mpFormat )
            {
                aFormat = String( pCurr->mpFormat, RTL_TEXTENCODING_UTF8 );
                GetFormatter().PutandConvertEntry( aFormat, nCheckPos,
                    nType, nKey, LANGUAGE_ENGLISH_US, pCurr->eLanguage );
            }
            else
                nKey = GetFormatter().GetFormatIndex( pCurr->meOffset, pCurr->eLanguage );
            InsertKey( pCurr->mnIndex, nKey );
            nSaveIndex = pCurr->mnIndex;
        }
    }
}

void XclImpNumFmtBuffer::InsertKey( sal_uInt32 nIndex, sal_uInt32 nFormatKey )
{
    if( nIndex >= maKeyVec.size() )
        maKeyVec.resize( nIndex + 1, mnStdFmt );
    maKeyVec[ nIndex ] = nFormatKey;
}


// XF, STYLE record - Cell formatting =========================================

void XclImpCellProt::FillFromXF2( sal_uInt8 nNumFmt )
{
    mbLocked = ::get_flag( nNumFmt, EXC_XF2_LOCKED );
    mbHidden = ::get_flag( nNumFmt, EXC_XF2_HIDDEN );
}

void XclImpCellProt::FillFromXF3( sal_uInt16 nProt )
{
    mbLocked = ::get_flag( nProt, EXC_XF_LOCKED );
    mbHidden = ::get_flag( nProt, EXC_XF_HIDDEN );
}

void XclImpCellProt::FillToItemSet( SfxItemSet& rItemSet, bool bSkipPoolDefs ) const
{
    lcl_xistyle_PutItem( rItemSet, ScProtectionAttr( mbLocked, mbHidden ), bSkipPoolDefs );
}


// ----------------------------------------------------------------------------

void XclImpCellAlign::FillFromXF2( sal_uInt8 nFlags )
{
    ::extract_value( meHorAlign, nFlags, 0, 3 );
}

void XclImpCellAlign::FillFromXF3( sal_uInt16 nAlign )
{
    ::extract_value( meHorAlign, nAlign, 0, 3 );
    mbWrapped = ::get_flag( nAlign, EXC_XF_WRAPPED );   // new in BIFF3
}

void XclImpCellAlign::FillFromXF4( sal_uInt16 nAlign )
{
    FillFromXF3( nAlign );
    ::extract_value( meVerAlign, nAlign, 4, 2 );        // new in BIFF4
    ::extract_value( meOrient, nAlign, 6, 2 );          // new in BIFF4
}

void XclImpCellAlign::FillFromXF5( sal_uInt16 nAlign )
{
    ::extract_value( meHorAlign, nAlign, 0, 3 );
    ::extract_value( meVerAlign, nAlign, 4, 3 );
    mbWrapped = ::get_flag( nAlign, EXC_XF_WRAPPED );
    ::extract_value( meOrient, nAlign, 8, 2 );
}

void XclImpCellAlign::FillFromXF8( sal_uInt16 nAlign, sal_uInt16 nMiscAttrib )
{
    ::extract_value( meHorAlign, nAlign, 0, 3 );
    ::extract_value( meVerAlign, nAlign, 4, 3 );
    mbWrapped = ::get_flag( nAlign, EXC_XF_WRAPPED );
    ::extract_value( mnRotation, nAlign, 8, 8 );        // new in BIFF8
    ::extract_value( mnIndent, nMiscAttrib, 0, 4 );     // new in BIFF8
    ::extract_value( meTextDir, nMiscAttrib, 6, 2 );    // new in BIFF8X
}

void XclImpCellAlign::FillToItemSet( SfxItemSet& rItemSet, const XclImpFont* pFont, bool bSkipPoolDefs ) const
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
        default:    DBG_ERRORFILE( "XclImpCellAlign::FillToItemSet - unknown horizontal alignment" );
    }
    lcl_xistyle_PutItem( rItemSet, SvxHorJustifyItem( eHorJust ), bSkipPoolDefs );

    // text wrap
    SfxBoolItem aWrapItem( ATTR_LINEBREAK );
    aWrapItem.SetValue( mbWrapped );
    lcl_xistyle_PutItem( rItemSet, aWrapItem, bSkipPoolDefs );

    // vertical alignment
    SvxCellVerJustify eVertJust = SVX_VER_JUSTIFY_STANDARD;
    switch( meVerAlign )
    {
        case xlVAlignTop:       eVertJust = SVX_VER_JUSTIFY_TOP;        break;
        case xlVAlignCenter:    eVertJust = SVX_VER_JUSTIFY_CENTER;     break;
        case xlVAlignBottom:    eVertJust = SVX_VER_JUSTIFY_STANDARD;   break;
        case xlVAlignJustify:
        case xlVAlignDistrib:   eVertJust = SVX_VER_JUSTIFY_TOP;        break;
        default:    DBG_ERRORFILE( "XclImpCellAlign::FillToItemSet - unknown vertical alignment" );
    }
    lcl_xistyle_PutItem( rItemSet, SvxVerJustifyItem( eVertJust ), bSkipPoolDefs );

    // indent
    sal_uInt16 nScIndent = mnIndent * 200; // 1 Excel unit == 10 pt == 200 twips
    lcl_xistyle_PutItem( rItemSet, SfxUInt16Item( ATTR_INDENT, nScIndent ), bSkipPoolDefs );

    // text orientation/rotation
    XclTextOrient eTmpOrient = meOrient;
    switch( mnRotation )    // BIFF8 does not set meOrient
    {
        case 90:                eTmpOrient = xlTextOrient90ccw;     break;
        case 180:               eTmpOrient = xlTextOrient90cw;      break;
        case EXC_XF8_STACKED:   eTmpOrient = xlTextOrientTopBottom; break;
    };
    SvxCellOrientation eSvxOrient = SVX_ORIENTATION_STANDARD;
    switch( eTmpOrient )
    {
        case xlTextOrientNoRot:     eSvxOrient = SVX_ORIENTATION_STANDARD;   break;
        case xlTextOrientTopBottom: eSvxOrient = SVX_ORIENTATION_STACKED;    break;
        case xlTextOrient90ccw:     eSvxOrient = SVX_ORIENTATION_BOTTOMTOP;  break;
        case xlTextOrient90cw:      eSvxOrient = SVX_ORIENTATION_TOPBOTTOM;  break;
        default:    DBG_ERRORFILE( "XclImpCellAlign::FillToItemSet - unknown text orientation" );
    };
    lcl_xistyle_PutItem( rItemSet, SvxOrientationItem( eSvxOrient ), bSkipPoolDefs );
    lcl_xistyle_PutItem( rItemSet, SvxRotateModeItem( SVX_ROTATE_MODE_STANDARD, ATTR_ROTATE_MODE ), bSkipPoolDefs );
    if( eTmpOrient == xlTextOrientNoRot )
    {
        // set an angle in the range from -90 to 90 degrees
        DBG_ASSERT( mnRotation <= 180, "XclImpCellAlign::FillToItemSet - illegal rotation angle" );
        sal_Int32 nAngle = XclTools::GetScRotation( mnRotation );
        lcl_xistyle_PutItem( rItemSet, SfxInt32Item( ATTR_ROTATE_VALUE, nAngle ), bSkipPoolDefs );
    }
    // #105933# set "Use asian vertical layout", if cell is stacked and font contains CKJ characters
    SfxBoolItem aAsianVertItem( ATTR_VERTICAL_ASIAN );
    aAsianVertItem.SetValue( (eSvxOrient == SVX_ORIENTATION_STACKED) && pFont && pFont->HasCjk() );
    lcl_xistyle_PutItem( rItemSet, aAsianVertItem, bSkipPoolDefs );

    // CTL text direction
    SvxFrameDirection eFrameDir = FRMDIR_ENVIRONMENT;
    switch( meTextDir )
    {
        case xlTextDirContext:  eFrameDir = FRMDIR_ENVIRONMENT;     break;
        case xlTextDirLTR:      eFrameDir = FRMDIR_HORI_LEFT_TOP;   break;
        case xlTextDirRTL:      eFrameDir = FRMDIR_HORI_RIGHT_TOP;  break;
        default:    DBG_ERRORFILE( "XclImpCellAlign::FillToItemSet - unknown CTL text direction" );
    }
    lcl_xistyle_PutItem( rItemSet, SvxFrameDirectionItem( eFrameDir ), bSkipPoolDefs );
}


// ----------------------------------------------------------------------------

XclImpCellBorder::XclImpCellBorder()
{
    SetAllUsedFlags( false );
}

void XclImpCellBorder::SetAllUsedFlags( bool bUsed )
{
    mbLeftUsed = mbRightUsed = mbTopUsed = mbBottomUsed = bUsed;
}

void XclImpCellBorder::FillFromXF2( sal_uInt8 nFlags )
{
    mnLeftLine   = ::get_flagvalue( nFlags, EXC_XF2_LEFTLINE,   EXC_LINE_THIN, EXC_LINE_NONE );
    mnRightLine  = ::get_flagvalue( nFlags, EXC_XF2_RIGHTLINE,  EXC_LINE_THIN, EXC_LINE_NONE );
    mnTopLine    = ::get_flagvalue( nFlags, EXC_XF2_TOPLINE,    EXC_LINE_THIN, EXC_LINE_NONE );
    mnBottomLine = ::get_flagvalue( nFlags, EXC_XF2_BOTTOMLINE, EXC_LINE_THIN, EXC_LINE_NONE );
    mnLeftColor = mnRightColor = mnTopColor = mnBottomColor = EXC_COLOR_BIFF2_BLACK;
    SetAllUsedFlags( true );
}

void XclImpCellBorder::FillFromXF3( sal_uInt32 nBorder )
{
    ::extract_value( mnTopLine,     nBorder,  0, 3 );
    ::extract_value( mnLeftLine,    nBorder,  8, 3 );
    ::extract_value( mnBottomLine,  nBorder, 16, 3 );
    ::extract_value( mnRightLine,   nBorder, 24, 3 );
    ::extract_value( mnTopColor,    nBorder,  3, 5 );
    ::extract_value( mnLeftColor,   nBorder, 11, 5 );
    ::extract_value( mnBottomColor, nBorder, 19, 5 );
    ::extract_value( mnRightColor,  nBorder, 27, 5 );
    SetAllUsedFlags( true );
}

void XclImpCellBorder::FillFromXF5( sal_uInt32 nBorder, sal_uInt32 nArea )
{
    ::extract_value( mnTopLine,     nBorder,  0, 3 );
    ::extract_value( mnLeftLine,    nBorder,  3, 3 );
    ::extract_value( mnBottomLine,  nArea,   22, 3 );
    ::extract_value( mnRightLine,   nBorder,  6, 3 );
    ::extract_value( mnTopColor,    nBorder,  9, 7 );
    ::extract_value( mnLeftColor,   nBorder, 16, 7 );
    ::extract_value( mnBottomColor, nArea,   25, 7 );
    ::extract_value( mnRightColor,  nBorder, 23, 7 );
    SetAllUsedFlags( true );
}

void XclImpCellBorder::FillFromXF8( sal_uInt32 nBorder1, sal_uInt32 nBorder2 )
{
    ::extract_value( mnLeftLine,    nBorder1,  0, 4 );
    ::extract_value( mnRightLine,   nBorder1,  4, 4 );
    ::extract_value( mnTopLine,     nBorder1,  8, 4 );
    ::extract_value( mnBottomLine,  nBorder1, 12, 4 );
    ::extract_value( mnLeftColor,   nBorder1, 16, 7 );
    ::extract_value( mnRightColor,  nBorder1, 23, 7 );
    ::extract_value( mnTopColor,    nBorder2,  0, 7 );
    ::extract_value( mnBottomColor, nBorder2,  7, 7 );
    SetAllUsedFlags( true );
}

void XclImpCellBorder::FillFromCF8( sal_uInt16 nLineStyle, sal_uInt32 nLineColor, sal_uInt32 nFlags )
{
    ::extract_value( mnLeftLine,    nLineStyle,  0, 4 );
    ::extract_value( mnRightLine,   nLineStyle,  4, 4 );
    ::extract_value( mnTopLine,     nLineStyle,  8, 4 );
    ::extract_value( mnBottomLine,  nLineStyle, 12, 4 );
    ::extract_value( mnLeftColor,   nLineColor,  0, 7 );
    ::extract_value( mnRightColor,  nLineColor,  7, 7 );
    ::extract_value( mnTopColor,    nLineColor, 16, 7 );
    ::extract_value( mnBottomColor, nLineColor, 23, 7 );
    mbLeftUsed   = !::get_flag( nFlags, EXC_CF_BORDER_LEFT );
    mbRightUsed  = !::get_flag( nFlags, EXC_CF_BORDER_RIGHT );
    mbTopUsed    = !::get_flag( nFlags, EXC_CF_BORDER_TOP );
    mbBottomUsed = !::get_flag( nFlags, EXC_CF_BORDER_BOTTOM );
}

/** Converts the passed line style to a SvxBorderLine, or returns false, if style is "no line". */
bool lcl_xistyle_ConvertBorderLine( SvxBorderLine& rLine, const XclImpPalette& rPalette, sal_uInt8 nXclLine, sal_uInt16 nXclColor )
{
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

    if( nXclLine == EXC_LINE_NONE )
        return false;
    if( nXclLine >= STATIC_TABLE_SIZE( ppnLineParam ) )
        nXclLine = EXC_LINE_THIN;

    rLine.SetColor( rPalette.GetColor( nXclColor ) );
    rLine.SetOutWidth( ppnLineParam[ nXclLine ][ 0 ] );
    rLine.SetInWidth(  ppnLineParam[ nXclLine ][ 1 ] );
    rLine.SetDistance( ppnLineParam[ nXclLine ][ 2 ] );
    return true;
}

void XclImpCellBorder::FillToItemSet( SfxItemSet& rItemSet, const XclImpPalette& rPalette, bool bSkipPoolDefs ) const
{
    if( mbLeftUsed || mbRightUsed || mbTopUsed || mbBottomUsed )
    {
        SvxBoxItem aBoxItem;
        SvxBorderLine aLine;
        if( mbLeftUsed && lcl_xistyle_ConvertBorderLine( aLine, rPalette, mnLeftLine, mnLeftColor ) )
            aBoxItem.SetLine( &aLine, BOX_LINE_LEFT );
        if( mbRightUsed && lcl_xistyle_ConvertBorderLine( aLine, rPalette, mnRightLine, mnRightColor ) )
            aBoxItem.SetLine( &aLine, BOX_LINE_RIGHT );
        if( mbTopUsed && lcl_xistyle_ConvertBorderLine( aLine, rPalette, mnTopLine, mnTopColor ) )
            aBoxItem.SetLine( &aLine, BOX_LINE_TOP );
        if( mbBottomUsed && lcl_xistyle_ConvertBorderLine( aLine, rPalette, mnBottomLine, mnBottomColor ) )
            aBoxItem.SetLine( &aLine, BOX_LINE_BOTTOM );
        lcl_xistyle_PutItem( rItemSet, aBoxItem, bSkipPoolDefs );
    }
}


// ----------------------------------------------------------------------------

XclImpCellArea::XclImpCellArea()
{
    SetAllUsedFlags( false );
}

void XclImpCellArea::SetAllUsedFlags( bool bUsed )
{
    mbForeUsed = mbBackUsed = mbPattUsed = bUsed;
}

void XclImpCellArea::FillFromXF2( sal_uInt8 nFlags )
{
    mnPattern = ::get_flagvalue( nFlags, EXC_XF2_BACKGROUND, EXC_PATT_12_5_PERC, EXC_PATT_NONE );
    mnForeColor = EXC_COLOR_BIFF2_BLACK;
    mnBackColor = EXC_COLOR_BIFF2_WHITE;
    SetAllUsedFlags( true );
}

void XclImpCellArea::FillFromXF3( sal_uInt16 nArea )
{
    ::extract_value( mnPattern,   nArea,  0, 6 );
    ::extract_value( mnForeColor, nArea,  6, 5 );
    ::extract_value( mnBackColor, nArea, 11, 5 );
    SetAllUsedFlags( true );
}

void XclImpCellArea::FillFromXF5( sal_uInt32 nArea )
{
    ::extract_value( mnPattern,   nArea, 16, 6 );
    ::extract_value( mnForeColor, nArea,  0, 7 );
    ::extract_value( mnBackColor, nArea,  7, 7 );
    SetAllUsedFlags( true );
}

void XclImpCellArea::FillFromXF8( sal_uInt32 nBorder2, sal_uInt16 nArea )
{
    ::extract_value( mnPattern,   nBorder2, 26, 6 );
    ::extract_value( mnForeColor, nArea,     0, 7 );
    ::extract_value( mnBackColor, nArea,     7, 7 );
    SetAllUsedFlags( true );
}

void XclImpCellArea::FillFromCF8( sal_uInt16 nPattern, sal_uInt16 nColor, sal_uInt32 nFlags )
{
    ::extract_value( mnForeColor, nColor,    0, 7 );
    ::extract_value( mnBackColor, nColor,    7, 7 );
    ::extract_value( mnPattern,   nPattern, 10, 6 );
    mbForeUsed = !::get_flag( nFlags, EXC_CF_AREA_FGCOLOR );
    mbBackUsed = !::get_flag( nFlags, EXC_CF_AREA_BGCOLOR );
    mbPattUsed = !::get_flag( nFlags, EXC_CF_AREA_PATTERN );

    if( mbBackUsed && (!mbPattUsed || (mnPattern == EXC_PATT_SOLID)) )
    {
        mnForeColor = mnBackColor;
        mnPattern = EXC_PATT_SOLID;
        mbForeUsed = mbPattUsed = true;
    }
    else if( !mbBackUsed && mbPattUsed && (mnPattern == EXC_PATT_SOLID) )
    {
        mbPattUsed = false;
    }
}

void XclImpCellArea::FillToItemSet( SfxItemSet& rItemSet, const XclImpPalette& rPalette, bool bSkipPoolDefs ) const
{
    static const sal_uInt16 pnRatioTable[] =
    {                                           // 0x8000 = 100%
        0x8000, 0x0000, 0x4000, 0x2000,         // 00 - 03
        0x6000, 0x4000, 0x4000, 0x4000,         // 04 - 07
        0x4000, 0x4000, 0x2000, 0x6000,         // 08 - 11
        0x6000, 0x6000, 0x6000, 0x4800,         // 12 - 15
        0x5000, 0x7000, 0x7800                  // 16 - 18
    };

    if( mbPattUsed )    // colors may be both unused in cond. formats
    {
        SvxBrushItem aBrushItem;

        // #108935# do not use IsTransparent() - old Calc filter writes tranparency with different color indexes
        if( mnPattern == EXC_PATT_NONE )
        {
            aBrushItem.SetColor( Color( COL_TRANSPARENT ) );
        }
        else
        {
            Color aFore( rPalette.GetColor( mbForeUsed ? mnForeColor : EXC_COLOR_WINDOWTEXT ) );
            Color aBack( rPalette.GetColor( mbBackUsed ? mnBackColor : EXC_COLOR_WINDOWBACK ) );
            if( mnPattern < STATIC_TABLE_SIZE( pnRatioTable ) )
                aFore = ScfTools::GetMixedColor( aFore, aBack, pnRatioTable[ mnPattern ] );
            aBrushItem.SetColor( aFore );
        }

        lcl_xistyle_PutItem( rItemSet, aBrushItem, bSkipPoolDefs );
    }
}


// ----------------------------------------------------------------------------

XclImpXF::XclImpXF( const XclImpRoot& rRoot ) :
    XclXFBase( true ),      // default is cell XF
    XclImpRoot( rRoot ),
    mpStyleSheet( NULL ),
    mnNumFmt( 0 ),
    mnFont( 0 ),
    mbWasBuiltIn( false )
{
}

XclImpXF::~XclImpXF()
{
}

void XclImpXF::ReadXF2( XclImpStream& rStrm )
{
    sal_uInt8 nReadFont, nReadNumFmt, nFlags;
    rStrm >> nReadFont;
    rStrm.Ignore( 1 );
    rStrm >> nReadNumFmt >> nFlags;

    // XF type always cell, no parent, used flags always true
    SetAllUsedFlags( true );

    // attributes
    maProtection.FillFromXF2( nReadNumFmt );
    mnFont = nReadFont;
    mnNumFmt = nReadNumFmt & EXC_XF2_VALFMT_MASK;
    maAlignment.FillFromXF2( nFlags );
    maBorder.FillFromXF2( nFlags );
    maArea.FillFromXF2( nFlags );
}

void XclImpXF::ReadXF3( XclImpStream& rStrm )
{
    sal_uInt32 nBorder;
    sal_uInt16 nTypeProt, nAlign, nArea;
    sal_uInt8 nReadFont, nReadNumFmt;
    rStrm >> nReadFont >> nReadNumFmt >> nTypeProt >> nAlign >> nArea >> nBorder;

    // XF type/parent
    mbCellXF = !::get_flag( nTypeProt, EXC_XF_STYLE );  // new in BIFF3
    ::extract_value( mnParent, nAlign, 4, 12 );         // new in BIFF3
    // attribute used flags
    sal_uInt8 nUsedFlags;
    ::extract_value( nUsedFlags, nTypeProt, 10, 6 );
    SetUsedFlags( nUsedFlags );

    // attributes
    maProtection.FillFromXF3( nTypeProt );
    mnFont = nReadFont;
    mnNumFmt = nReadNumFmt;
    maAlignment.FillFromXF3( nAlign );
    maBorder.FillFromXF3( nBorder );
    maArea.FillFromXF3( nArea );                        // new in BIFF3
}

void XclImpXF::ReadXF4( XclImpStream& rStrm )
{
    sal_uInt32 nBorder;
    sal_uInt16 nTypeProt, nAlign, nArea;
    sal_uInt8 nReadFont, nReadNumFmt;
    rStrm >> nReadFont >> nReadNumFmt >> nTypeProt >> nAlign >> nArea >> nBorder;

    // XF type/parent
    mbCellXF = !::get_flag( nTypeProt, EXC_XF_STYLE );
    ::extract_value( mnParent, nTypeProt, 4, 12 );
    // attribute used flags
    sal_uInt8 nUsedFlags;
    ::extract_value( nUsedFlags, nAlign, 10, 6 );
    SetUsedFlags( nUsedFlags );

    // attributes
    maProtection.FillFromXF3( nTypeProt );
    mnFont = nReadFont;
    mnNumFmt = nReadNumFmt;
    maAlignment.FillFromXF4( nAlign );
    maBorder.FillFromXF3( nBorder );
    maArea.FillFromXF3( nArea );
}

void XclImpXF::ReadXF5( XclImpStream& rStrm )
{
    sal_uInt32 nArea, nBorder;
    sal_uInt16 nTypeProt, nAlign;
    rStrm >> mnFont >> mnNumFmt >> nTypeProt >> nAlign >> nArea >> nBorder;

    // XF type/parent
    mbCellXF = !::get_flag( nTypeProt, EXC_XF_STYLE );
    ::extract_value( mnParent, nTypeProt, 4, 12 );
    // attribute used flags
    sal_uInt8 nUsedFlags;
    ::extract_value( nUsedFlags, nAlign, 10, 6 );
    SetUsedFlags( nUsedFlags );

    // attributes
    maProtection.FillFromXF3( nTypeProt );
    maAlignment.FillFromXF5( nAlign );
    maBorder.FillFromXF5( nBorder, nArea );
    maArea.FillFromXF5( nArea );
}

void XclImpXF::ReadXF8( XclImpStream& rStrm )
{
    sal_uInt32 nBorder1, nBorder2;
    sal_uInt16 nTypeProt, nAlign, nMiscAttrib, nArea;
    rStrm >> mnFont >> mnNumFmt >> nTypeProt >> nAlign >> nMiscAttrib >> nBorder1 >> nBorder2 >> nArea;

    // XF type/parent
    mbCellXF = !::get_flag( nTypeProt, EXC_XF_STYLE );
    ::extract_value( mnParent, nTypeProt, 4, 12 );
    // attribute used flags
    sal_uInt8 nUsedFlags;
    ::extract_value( nUsedFlags, nMiscAttrib, 10, 6 );
    SetUsedFlags( nUsedFlags );

    // attributes
    maProtection.FillFromXF3( nTypeProt );
    maAlignment.FillFromXF8( nAlign, nMiscAttrib );
    maBorder.FillFromXF8( nBorder1, nBorder2 );
    maArea.FillFromXF8( nBorder2, nArea );
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

void XclImpXF::SetStyleName( const String& rStyleName )
{
    DBG_ASSERT( IsStyleXF(), "XclImpXF::SetStyleName - not a style XF" );
    DBG_ASSERT( rStyleName.Len(), "XclImpXF::SetStyleName - style name empty" );
    if( IsStyleXF() && !maStyleName.Len() )
    {
        maStyleName = rStyleName;
        CreateStyleSheet();     // force creation of the style in the Calc doc
    }
}

void XclImpXF::SetBuiltInStyleName( sal_uInt8 nStyleId, sal_uInt8 nLevel )
{
    DBG_ASSERT( IsStyleXF(), "XclImpXF::SetStyleName - not a style XF" );
    if( IsStyleXF() && !maStyleName.Len() )
    {
        mbWasBuiltIn = true;
        maStyleName = XclTools::GetBuiltInStyleName( nStyleId, nLevel );
        if( nStyleId == EXC_STYLE_NORMAL )
            CreateStyleSheet(); // force creation of "Default" style
    }
}

void XclImpXF::ApplyPattern(
        sal_uInt16 nFirstCol, sal_uInt16 nFirstRow,
        sal_uInt16 nLastCol, sal_uInt16 nLastRow,
        sal_uInt16 nTab, sal_uInt32 nForcedNumFmt )
{
    // force creation of cell style and hard formatting, do it here to have mpStyleSheet
    const ScPatternAttr& rPattern = CreatePattern();

    // insert into document
    ScDocument& rDoc = GetDoc();
    if( IsCellXF() && mpStyleSheet )
        rDoc.ApplyStyleAreaTab( nFirstCol, nFirstRow, nLastCol, nLastRow, nTab, *mpStyleSheet );
    rDoc.ApplyPatternAreaTab( nFirstCol, nFirstRow, nLastCol, nLastRow, nTab, rPattern );

    // #108770# apply special number format
    if( nForcedNumFmt != NUMBERFORMAT_ENTRY_NOT_FOUND )
    {
        ScPatternAttr aPattern( GetDoc().GetPool() );
        GetNumFmtBuffer().FillScFmtToItemSet( aPattern.GetItemSet(), nForcedNumFmt );
        rDoc.ApplyPatternAreaTab( nFirstCol, nFirstRow, nLastCol, nLastRow, nTab, aPattern );
    }
}

void XclImpXF::SetUsedFlags( sal_uInt8 nUsedFlags )
{
    /*  Notes about finding the mb***Used flags:
        - In cell XFs a *set* bit means a used attribute.
        - In style XFs a *cleared* bit means a used attribute.
        The mb***Used members always store true, if the attribute is used.
        The "mbCellXF == ::get_flag(...)" construct evaluates to true in
        both mentioned cases: cell XF and set bit; or style XF and cleared bit.
     */
    mbProtUsed   = (mbCellXF == ::get_flag( nUsedFlags, EXC_XF_DIFF_PROT ));
    mbFontUsed   = (mbCellXF == ::get_flag( nUsedFlags, EXC_XF_DIFF_FONT ));
    mbFmtUsed    = (mbCellXF == ::get_flag( nUsedFlags, EXC_XF_DIFF_VALFMT ));
    mbAlignUsed  = (mbCellXF == ::get_flag( nUsedFlags, EXC_XF_DIFF_ALIGN ));
    mbBorderUsed = (mbCellXF == ::get_flag( nUsedFlags, EXC_XF_DIFF_BORDER ));
    mbAreaUsed   = (mbCellXF == ::get_flag( nUsedFlags, EXC_XF_DIFF_AREA ));
}

void XclImpXF::UpdateUsedFlags( const XclImpXF& rParentXF )
{
    /*  Enables mb***Used flags, if the formatting attributes differ from
        the passed XF record. In cell XFs Excel uses the cell attributes,
        if they differ from the parent style XF.
        #109899# ...or if the respective flag is not set in parent style XF. */
    if( !mbProtUsed )
        mbProtUsed = !rParentXF.mbProtUsed || !(maProtection == rParentXF.maProtection);
    if( !mbFontUsed )
        mbFontUsed = !rParentXF.mbFontUsed || (mnFont != rParentXF.mnFont);
    if( !mbFmtUsed )
        mbFmtUsed = !rParentXF.mbFmtUsed || (mnNumFmt != rParentXF.mnNumFmt);
    if( !mbAlignUsed )
        mbAlignUsed = !rParentXF.mbAlignUsed || !(maAlignment == rParentXF.maAlignment);
    if( !mbBorderUsed )
        mbBorderUsed = !rParentXF.mbBorderUsed || !(maBorder == rParentXF.maBorder);
    if( !mbAreaUsed )
        mbAreaUsed = !rParentXF.mbAreaUsed || !(maArea == rParentXF.maArea);
}

const ScPatternAttr& XclImpXF::CreatePattern( bool bSkipPoolDefs )
{
    if( mpPattern.get() )
        return *mpPattern;

    // create new pattern attribute set
    mpPattern.reset( new ScPatternAttr( GetDoc().GetPool() ) );
    SfxItemSet& rItemSet = mpPattern->GetItemSet();

    // parent cell style
    if( IsCellXF() )
    {
        if( XclImpXF* pParentXF = GetXFBuffer().GetXF( mnParent ) )
        {
            mpStyleSheet = pParentXF->CreateStyleSheet();
            UpdateUsedFlags( *pParentXF );
        }
    }

    // cell protection
    if( mbProtUsed )
        maProtection.FillToItemSet( rItemSet, bSkipPoolDefs );

    // font
    if( mbFontUsed )
        GetFontBuffer().FillToItemSet( rItemSet, xlFontScIDs, mnFont, bSkipPoolDefs );

    // value format
    if( mbFmtUsed )
        GetNumFmtBuffer().FillToItemSet( rItemSet, mnNumFmt, bSkipPoolDefs );

    // alignment
    if( mbAlignUsed )
        maAlignment.FillToItemSet( rItemSet, GetFontBuffer().GetFont( mnFont ), bSkipPoolDefs );

    // border
    if( mbBorderUsed )
        maBorder.FillToItemSet( rItemSet, GetPalette(), bSkipPoolDefs );

    // area
    if( mbAreaUsed )
        maArea.FillToItemSet( rItemSet, GetPalette(), bSkipPoolDefs );

    return *mpPattern;
}

ScStyleSheet* XclImpXF::CreateStyleSheet()
{
    if( !mpStyleSheet && maStyleName.Len() )    // valid name implies style XF
    {
        // there may be a user-defined "Default" - test on built-in too!
        bool bDefStyle = mbWasBuiltIn && (maStyleName == ScGlobal::GetRscString( STR_STYLENAME_STANDARD ));
        if( bDefStyle )
        {
            // set all flags to true to get all items in CreatePattern()
            SetAllUsedFlags( true );
            // use existing "Default" style sheet
            mpStyleSheet = static_cast< ScStyleSheet* >( GetStyleSheetPool().Find(
                ScGlobal::GetRscString( STR_STYLENAME_STANDARD ), SFX_STYLE_FAMILY_PARA ) );
            DBG_ASSERT( mpStyleSheet, "XclImpXF::CreateStyleSheet - Default style not found" );
        }
        else
        {
            /*  mbWasBuiltIn==true forces renaming of equal-named user defined styles
                to be able to re-export built-in styles correctly. */
            mpStyleSheet = &ScfTools::MakeCellStyleSheet( GetStyleSheetPool(), maStyleName, mbWasBuiltIn );
        }

        // bDefStyle==true omits default pool items in CreatePattern()
        if( mpStyleSheet )
            mpStyleSheet->GetItemSet().Put( CreatePattern( bDefStyle ).GetItemSet() );
    }
    return mpStyleSheet;
}


// ----------------------------------------------------------------------------

XclImpXFBuffer::XclImpXFBuffer( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot )
{
}

void XclImpXFBuffer::ReadXF( XclImpStream& rStrm )
{
    XclImpXF* pXF = new XclImpXF( GetRoot() );
    pXF->ReadXF( rStrm );
    maXFList.Append( pXF );

    if( maXFList.Count() == 1 )
        // set the name of the "Default" cell style (always the first XF in an Excel file)
        pXF->SetBuiltInStyleName( EXC_STYLE_NORMAL, 0 );
}

void XclImpXFBuffer::ReadStyle( XclImpStream& rStrm )
{
    DBG_ASSERT_BIFF( GetBiff() > xlBiff2 );

    sal_uInt16 nXFIndex;
    rStrm >> nXFIndex;

    XclImpXF* pXF = GetXF( nXFIndex & EXC_STYLE_XFMASK );   // bits 0...11 are used for XF index
    if( pXF && pXF->IsStyleXF() )
    {
        if( ::get_flag( nXFIndex, EXC_STYLE_BUILTIN ) )     // built-in styles
        {
            sal_uInt8 nStyleId, nLevel;
            rStrm >> nStyleId >> nLevel;
            pXF->SetBuiltInStyleName( nStyleId, nLevel );
        }
        else                                                // user-defined styles
        {
            String aStyleName;
            if( GetBiff() < xlBiff8 )
                rStrm.AppendByteString( aStyleName, false );    // 8 bit length
            else
                rStrm.AppendUniString( aStyleName );
            if( aStyleName.Len() )  // #i1624# #i1768# ignore unnamed styles
                pXF->SetStyleName( aStyleName );
        }
    }
}

sal_uInt16 XclImpXFBuffer::GetFontIndex( sal_uInt32 nXFIndex ) const
{
    const XclImpXF* pXF = GetXF( nXFIndex );
    return pXF ? pXF->GetFont() : 0;
}

bool XclImpXFBuffer::HasEscapement( sal_uInt32 nXFIndex ) const
{
    const XclImpXF* pXF = GetXF( nXFIndex );
    const XclImpFont* pFont = pXF ? GetFontBuffer().GetFont( pXF->GetFont() ) : NULL;
    return pFont ? pFont->HasEscapement() : false;
}

void XclImpXFBuffer::ApplyPattern(
        sal_uInt16 nFirstCol, sal_uInt16 nFirstRow,
        sal_uInt16 nLastCol, sal_uInt16 nLastRow,
        sal_uInt16 nTab, sal_uInt16 nXFIndex, sal_uInt32 nForcedNumFmt )
{
    if( XclImpXF* pXF = GetXF( nXFIndex ) )
        pXF->ApplyPattern( nFirstCol, nFirstRow, nLastCol, nLastRow, nTab, nForcedNumFmt );
}


// Buffer for XF indexes in cells =============================================

IMPL_FIXEDMEMPOOL_NEWDEL( XclImpXFIndex, 100, 500 )

bool XclImpXFIndex::Expand( sal_uInt16 nRow, sal_uInt32 nEncXFIndex )
{
    if( mnEncXF != nEncXFIndex )
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
    if( (mnEncXF == rNextStyle.mnEncXF) && (mnLastRow + 1 == rNextStyle.mnFirstRow) )
    {
        mnLastRow = rNextStyle.mnLastRow;
        return true;
    }
    return false;
}


// ----------------------------------------------------------------------------

void XclImpXFIndexColumn::SetXF( sal_uInt16 nRow, sal_uInt32 nEncXFIndex )
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
            if( nEncXFIndex == pPrevStyle->mnEncXF )
                return;

            sal_uInt16 nFirst = pPrevStyle->mnFirstRow;
            sal_uInt16 nLast = pPrevStyle->mnLastRow;
            sal_uInt32 nIndex = nNextIndex - 1;
            XclImpXFIndex* pThisStyle = pPrevStyle;
            pPrevStyle = nIndex ? maIndexList.GetObject( nIndex - 1 ) : NULL;

            if( nFirst == nLast )                   // replace solely XF
            {
                pThisStyle->mnEncXF = nEncXFIndex;
                TryConcatPrev( nNextIndex );        // try to concat. next with this
                TryConcatPrev( nIndex );            // try to concat. this with previous
            }
            else if( nFirst == nRow )               // replace first XF
            {
                ++(pThisStyle->mnFirstRow);
                // try to concatenate with previous of this
                if( !pPrevStyle || !pPrevStyle->Expand( nRow, nEncXFIndex ) )
                    maIndexList.Insert( new XclImpXFIndex( nRow, nEncXFIndex ), nIndex );
            }
            else if( nLast == nRow )                // replace last XF
            {
                --(pThisStyle->mnLastRow);
                if( !pNextStyle || !pNextStyle->Expand( nRow, nEncXFIndex ) )
                    maIndexList.Insert( new XclImpXFIndex( nRow, nEncXFIndex ), nNextIndex );
            }
            else                                    // insert in the middle of the range
            {
                pThisStyle->mnFirstRow = nRow + 1;
                // List::Insert() moves entries towards end of list, so insert twice at nIndex
                maIndexList.Insert( new XclImpXFIndex( nRow, nEncXFIndex ), nIndex );
                maIndexList.Insert( new XclImpXFIndex( nFirst, nRow - 1, pThisStyle->mnEncXF ), nIndex );
            }
            return;
        }
        else if( pPrevStyle->Expand( nRow, nEncXFIndex ) ) // try to expand
        {
            TryConcatPrev( nNextIndex );    // try to concatenate next with expanded
            return;
        }
    }

    // try to expand next range
    if( pNextStyle && pNextStyle->Expand( nRow, nEncXFIndex ) )
        return;

    // create new range
    maIndexList.Insert( new XclImpXFIndex( nRow, nEncXFIndex ), nNextIndex );
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

/** Number of columns. */
const sal_uInt32 EXC_XFBUFF_COLUMNS     = MAXCOL + 1;

/** Flag for Boolean cells, need special number format handling. */
const sal_uInt32 EXC_XFBUFF_BOOLFLAG    = 0x00010000;


XclImpXFIndexBuffer::XclImpXFIndexBuffer( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot ),
    mppColumns( new XclImpXFIndexColumnPtr[ EXC_XFBUFF_COLUMNS ] ),
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

sal_uInt32 XclImpXFIndexBuffer::EncodeXFIndex( sal_uInt16 nXFIndex, XclImpXFInsertMode eMode ) const
{
    sal_uInt32 nEncXFIndex = nXFIndex;
    if( eMode == xlXFModeBoolCell )
        nEncXFIndex |= EXC_XFBUFF_BOOLFLAG;
    return nEncXFIndex;
}

void XclImpXFIndexBuffer::DecodeXFIndex( sal_uInt16& rnXFIndex, sal_uInt32& rnForcedNumFmt, sal_uInt32 nEncXFIndex ) const
{
    rnXFIndex = static_cast< sal_uInt16 >( nEncXFIndex );
    // #108770# set 'Standard' number format for all Boolean cells
    rnForcedNumFmt = ::get_flag( nEncXFIndex, EXC_XFBUFF_BOOLFLAG ) ? GetNumFmtBuffer().GetStandardFormat() : NUMBERFORMAT_ENTRY_NOT_FOUND;
}

void XclImpXFIndexBuffer::SetXF( sal_uInt16 nCol, sal_uInt16 nRow, sal_uInt16 nXFIndex, XclImpXFInsertMode eMode )
{
    DBG_ASSERT( (nCol <= MAXCOL) && (nRow <= MAXROW), "XclImpXFIndexBuffer::SetXF - out of range" );
    // #108770# remember all Boolean cells, they will get 'Standard' number format
    sal_uInt32 nEncXFIndex = EncodeXFIndex( nXFIndex, eMode );

    // set cell XF's
    if( !mppColumns[ nCol ].get() )
        mppColumns[ nCol ].reset( new XclImpXFIndexColumn );
    if( nCol >= mnUsedCount )
        mnUsedCount = nCol + 1;
    mppColumns[ nCol ]->SetXF( nRow, nEncXFIndex );

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
            else if( eMode != xlXFModeBlank )   // #108781# do not merge empty cells
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

void XclImpXFIndexBuffer::SetBoolXF( sal_uInt16 nCol, sal_uInt16 nRow, sal_uInt16 nXFIndex )
{
    SetXF( nCol, nRow, nXFIndex, xlXFModeBoolCell );
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
    sal_uInt16 nXFIndex;
    sal_uInt32 nForcedNumFmt;
    for( sal_uInt16 nCol = 0; nCol < mnUsedCount; ++nCol )
    {
        // apply all cell styles of an existing column
        if( mppColumns[ nCol ].get() )
        {
            XclImpXFIndexColumn& rColumn = *mppColumns[ nCol ];
            for( XclImpXFIndex* pStyle = rColumn.First(); pStyle; pStyle = rColumn.Next() )
            {
                DecodeXFIndex( nXFIndex, nForcedNumFmt, pStyle->mnEncXF );
                rXFBuffer.ApplyPattern( nCol, pStyle->mnFirstRow, nCol, pStyle->mnLastRow, nTab, nXFIndex, nForcedNumFmt );
            }
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

