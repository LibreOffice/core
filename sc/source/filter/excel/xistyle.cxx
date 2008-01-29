/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xistyle.cxx,v $
 *
 *  $Revision: 1.36 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:28:01 $
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
 *
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

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
#ifndef _SVX_BOLNITEM_HXX
#include <svx/bolnitem.hxx>
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

#ifndef SC_XLTRACER_HXX
#include "xltracer.hxx"
#endif
#ifndef SC_XISTREAM_HXX
#include "xistream.hxx"
#endif
#ifndef SC_XICONTENT_HXX
#include "xicontent.hxx"
#endif

#include "root.hxx"

// PALETTE record - color information =========================================

XclImpPalette::XclImpPalette( const XclImpRoot& rRoot ) :
    XclDefaultPalette( rRoot )
{
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
    DBG_ASSERT( rStrm.GetRecLeft() == static_cast< sal_Size >( 4 * nCount ),
        "XclImpPalette::ReadPalette - size mismatch" );

    maColorTable.resize( nCount );
    Color aColor;
    for( sal_uInt16 nIndex = 0; nIndex < nCount; ++nIndex )
    {
        rStrm >> aColor;
        maColorTable[ nIndex ] = aColor.GetColor();
    }
}

// FONT record - font information =============================================

XclImpFont::XclImpFont( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot ),
    mbHasCharSet( false ),
    mbHasWstrn( true ),
    mbHasAsian( false ),
    mbHasCmplx( false )
{
    SetAllUsedFlags( false );
}

XclImpFont::XclImpFont( const XclImpRoot& rRoot, const XclFontData& rFontData ) :
    XclImpRoot( rRoot ),
    maData( rFontData ),
    mbHasCharSet( false )
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

rtl_TextEncoding XclImpFont::GetFontEncoding() const
{
    // #i63105# use text encoding from FONT record
    // #i67768# BIFF2-BIFF4 FONT records do not contain character set
    rtl_TextEncoding eFontEnc = mbHasCharSet ? maData.GetFontEncoding() : GetTextEncoding();
    return (eFontEnc == RTL_TEXTENCODING_DONTKNOW) ? GetTextEncoding() : eFontEnc;
}

void XclImpFont::ReadFont( XclImpStream& rStrm )
{
    switch( GetBiff() )
    {
        case EXC_BIFF2:
            ReadFontData2( rStrm );
            ReadFontName2( rStrm );
        break;
        case EXC_BIFF3:
        case EXC_BIFF4:
            ReadFontData2( rStrm );
            ReadFontColor( rStrm );
            ReadFontName2( rStrm );
        break;
        case EXC_BIFF5:
            ReadFontData5( rStrm );
            ReadFontName2( rStrm );
        break;
        case EXC_BIFF8:
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

void XclImpFont::ReadEfont( XclImpStream& rStrm )
{
    ReadFontColor( rStrm );
}

void XclImpFont::ReadCFFontBlock( XclImpStream& rStrm )
{
    DBG_ASSERT_BIFF( GetBiff() == EXC_BIFF8 );
    if( GetBiff() != EXC_BIFF8 )
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
    if( (mbUnderlUsed = !::get_flag( nFontFlags3, EXC_CF_FONT_UNDERL ) && (nUnderl <= 0x7F)) == true )
        maData.mnUnderline = nUnderl;
    if( (mbColorUsed = (nColor <= 0x7FFF)) == true )
        maData.maColor = GetPalette().GetColor( static_cast< sal_uInt16 >( nColor ) );
    if( (mbStrikeUsed = !::get_flag( nFontFlags1, EXC_CF_FONT_STRIKEOUT )) == true )
        maData.mbStrikeout = ::get_flag( nStyle, EXC_CF_FONT_STRIKEOUT );
}

void XclImpFont::FillToItemSet( SfxItemSet& rItemSet, XclFontItemType eType, bool bSkipPoolDefs ) const
{
    // true = edit engine Which-IDs (EE_CHAR_*); false = Calc Which-IDs (ATTR_*)
    bool bEE = eType != EXC_FONTITEM_CELL;

// item = the item to put into the item set
// sc_which = the Calc Which-ID of the item
// ee_which = the edit engine Which-ID of the item
#define PUTITEM( item, sc_which, ee_which ) \
    ScfTools::PutItem( rItemSet, item, (bEE ? (ee_which) : (sc_which)), bSkipPoolDefs )

// Font item
    // #i36997# do not set default Tahoma font from notes
    bool bDefNoteFont = (eType == EXC_FONTITEM_NOTE) && (maData.maName.EqualsIgnoreCaseAscii( "Tahoma" ));
    if( mbFontNameUsed && !bDefNoteFont )
    {
        rtl_TextEncoding eFontEnc = maData.GetFontEncoding();
        rtl_TextEncoding eTempTextEnc = (bEE && (eFontEnc == GetTextEncoding())) ?
            ScfTools::GetSystemTextEncoding() : eFontEnc;

        SvxFontItem aFontItem( maData.GetScFamily( GetTextEncoding() ), maData.maName, EMPTY_STRING,
                PITCH_DONTKNOW, eTempTextEnc, ATTR_FONT );
        // #91658# set only for valid script types
        if( mbHasWstrn )
            PUTITEM( aFontItem, ATTR_FONT,      EE_CHAR_FONTINFO );
        if( mbHasAsian )
            PUTITEM( aFontItem, ATTR_CJK_FONT,  EE_CHAR_FONTINFO_CJK );
        if( mbHasCmplx )
            PUTITEM( aFontItem, ATTR_CTL_FONT,  EE_CHAR_FONTINFO_CTL );
    }

// Font height (for all script types)
    if( mbHeightUsed )
    {
        sal_Int32 nHeight = maData.mnHeight;
        if( bEE && (eType != EXC_FONTITEM_HF) )     // do not convert header/footer height
            nHeight = (nHeight * 127 + 36) / EXC_POINTS_PER_INCH;   // #98527# 1 in == 72 pt

        SvxFontHeightItem aHeightItem( nHeight, 100, ATTR_FONT_HEIGHT );
        PUTITEM( aHeightItem,   ATTR_FONT_HEIGHT,       EE_CHAR_FONTHEIGHT );
        PUTITEM( aHeightItem,   ATTR_CJK_FONT_HEIGHT,   EE_CHAR_FONTHEIGHT_CJK );
        PUTITEM( aHeightItem,   ATTR_CTL_FONT_HEIGHT,   EE_CHAR_FONTHEIGHT_CTL );
    }

// Font color - pass AUTO_COL to item
    if( mbColorUsed )
        PUTITEM( SvxColorItem( maData.maColor, ATTR_FONT_COLOR  ), ATTR_FONT_COLOR, EE_CHAR_COLOR );

// Font weight (for all script types)
    if( mbWeightUsed )
    {
        SvxWeightItem aWeightItem( maData.GetScWeight(), ATTR_FONT_WEIGHT );
        PUTITEM( aWeightItem,   ATTR_FONT_WEIGHT,       EE_CHAR_WEIGHT );
        PUTITEM( aWeightItem,   ATTR_CJK_FONT_WEIGHT,   EE_CHAR_WEIGHT_CJK );
        PUTITEM( aWeightItem,   ATTR_CTL_FONT_WEIGHT,   EE_CHAR_WEIGHT_CTL );
    }

// Font underline
    if( mbUnderlUsed )
    {
        SvxUnderlineItem aUnderlItem( maData.GetScUnderline(), ATTR_FONT_UNDERLINE );
        PUTITEM( aUnderlItem,   ATTR_FONT_UNDERLINE,    EE_CHAR_UNDERLINE );
    }

// Font posture (for all script types)
    if( mbItalicUsed )
    {
        SvxPostureItem aPostItem( maData.GetScPosture(), ATTR_FONT_POSTURE );
        PUTITEM( aPostItem, ATTR_FONT_POSTURE,      EE_CHAR_ITALIC );
        PUTITEM( aPostItem, ATTR_CJK_FONT_POSTURE,  EE_CHAR_ITALIC_CJK );
        PUTITEM( aPostItem, ATTR_CTL_FONT_POSTURE,  EE_CHAR_ITALIC_CTL );
    }

// Boolean attributes crossed out, contoured, shadowed
    if( mbStrikeUsed )
        PUTITEM( SvxCrossedOutItem( maData.GetScStrikeout(), ATTR_FONT_CROSSEDOUT ), ATTR_FONT_CROSSEDOUT, EE_CHAR_STRIKEOUT );
    if( mbOutlineUsed )
        PUTITEM( SvxContourItem( maData.mbOutline, ATTR_FONT_CONTOUR ), ATTR_FONT_CONTOUR, EE_CHAR_OUTLINE );
    if( mbShadowUsed )
        PUTITEM( SvxShadowedItem( maData.mbShadow, ATTR_FONT_SHADOWED ), ATTR_FONT_SHADOWED, EE_CHAR_SHADOW );

// Super-/subscript: only on edit engine objects
    if( mbEscapemUsed && bEE )
        rItemSet.Put( SvxEscapementItem( maData.GetScEscapement(), EE_CHAR_ESCAPEMENT ) );

#undef PUTITEM
}

void XclImpFont::WriteFontProperties( ScfPropertySet& rPropSet,
        XclFontPropSetType eType, const Color* pFontColor ) const
{
    GetFontPropSetHelper().WriteFontProperties(
        rPropSet, eType, maData, mbHasWstrn, mbHasAsian, mbHasCmplx, pFontColor );
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
    mbHasCharSet = false;
}

void XclImpFont::ReadFontData5( XclImpStream& rStrm )
{
    sal_uInt16 nFlags;

    rStrm >> maData.mnHeight >> nFlags;
    ReadFontColor( rStrm );
    rStrm >> maData.mnWeight >> maData.mnEscapem >> maData.mnUnderline >> maData.mnFamily >> maData.mnCharSet;
    rStrm.Ignore( 1 );

    maData.mbItalic     = ::get_flag( nFlags, EXC_FONTATTR_ITALIC );
    maData.mbStrikeout  = ::get_flag( nFlags, EXC_FONTATTR_STRIKEOUT );
    maData.mbOutline    = ::get_flag( nFlags, EXC_FONTATTR_OUTLINE );
    maData.mbShadow     = ::get_flag( nFlags, EXC_FONTATTR_SHADOW );
    mbHasCharSet = true;
}

void XclImpFont::ReadFontColor( XclImpStream& rStrm )
{
    maData.maColor = GetPalette().GetColor( rStrm.ReaduInt16() );
}

void XclImpFont::ReadFontName2( XclImpStream& rStrm )
{
    maData.maName = rStrm.ReadByteString( false );
}

void XclImpFont::ReadFontName8( XclImpStream& rStrm )
{
    maData.maName = rStrm.ReadUniString( rStrm.ReaduInt8() );
}

void XclImpFont::GuessScriptType()
{
    mbHasWstrn = true;
    mbHasAsian = mbHasCmplx = false;

    // #91658# #113783# find the script types for which the font contains characters
    if( OutputDevice* pPrinter = GetPrinter() )
    {
        Font aFont( maData.maName, Size( 0, 10 ) );
        FontCharMap aCharMap;

        pPrinter->SetFont( aFont );
        if( pPrinter->GetFontCharMap( aCharMap ) )
        {
            // #91658# CJK fonts
            mbHasAsian =
                aCharMap.HasChar( 0x3041 ) ||   // 3040-309F: Hiragana
                aCharMap.HasChar( 0x30A1 ) ||   // 30A0-30FF: Katakana
                aCharMap.HasChar( 0x3111 ) ||   // 3100-312F: Bopomofo
                aCharMap.HasChar( 0x3131 ) ||   // 3130-318F: Hangul Compatibility Jamo
                aCharMap.HasChar( 0x3301 ) ||   // 3300-33FF: CJK Compatibility
                aCharMap.HasChar( 0x3401 ) ||   // 3400-4DBF: CJK Unified Ideographs Extension A
                aCharMap.HasChar( 0x4E01 ) ||   // 4E00-9FAF: CJK Unified Ideographs
                aCharMap.HasChar( 0x7E01 ) ||   // 4E00-9FAF: CJK unified ideographs
                aCharMap.HasChar( 0xA001 ) ||   // A001-A48F: Yi Syllables
                aCharMap.HasChar( 0xAC01 ) ||   // AC00-D7AF: Hangul Syllables
                aCharMap.HasChar( 0xCC01 ) ||   // AC00-D7AF: Hangul Syllables
                aCharMap.HasChar( 0xF901 ) ||   // F900-FAFF: CJK Compatibility Ideographs
                aCharMap.HasChar( 0xFF71 );     // FF00-FFEF: Halfwidth/Fullwidth Forms
            // #113783# CTL fonts
            mbHasCmplx =
                aCharMap.HasChar( 0x05D1 ) ||   // 0590-05FF: Hebrew
                aCharMap.HasChar( 0x0631 ) ||   // 0600-06FF: Arabic
                aCharMap.HasChar( 0x0721 ) ||   // 0700-074F: Syriac
                aCharMap.HasChar( 0x0911 ) ||   // 0900-0DFF: Indic scripts
                aCharMap.HasChar( 0x0E01 ) ||   // 0E00-0E7F: Thai
                aCharMap.HasChar( 0xFB21 ) ||   // FB1D-FB4F: Hebrew Presentation Forms
                aCharMap.HasChar( 0xFB51 ) ||   // FB50-FDFF: Arabic Presentation Forms-A
                aCharMap.HasChar( 0xFE71 );     // FE70-FEFF: Arabic Presentation Forms-B
            // Western fonts
            mbHasWstrn = (!mbHasAsian && !mbHasCmplx) || aCharMap.HasChar( 'A' );
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
        // #i71033# set text encoding from application font, if CODEPAGE is missing
        SetAppFontEncoding( pFont->GetFontEncoding() );
    }
}

void XclImpFontBuffer::ReadEfont( XclImpStream& rStrm )
{
    if( XclImpFont* pFont = maFontList.Last() )
        pFont->ReadEfont( rStrm );
}

void XclImpFontBuffer::FillToItemSet(
        SfxItemSet& rItemSet, XclFontItemType eType,
        sal_uInt16 nFontIdx, bool bSkipPoolDefs ) const
{
    if( const XclImpFont* pFont = GetFont( nFontIdx ) )
        pFont->FillToItemSet( rItemSet, eType, bSkipPoolDefs );
}

void XclImpFontBuffer::WriteFontProperties( ScfPropertySet& rPropSet,
        XclFontPropSetType eType, sal_uInt16 nFontIdx, const Color* pFontColor ) const
{
    if( const XclImpFont* pFont = GetFont( nFontIdx ) )
        pFont->WriteFontProperties( rPropSet, eType, pFontColor );
}

// FORMAT record - number formats =============================================

XclImpNumFmtBuffer::XclImpNumFmtBuffer( const XclImpRoot& rRoot ) :
    XclNumFmtBuffer( rRoot ),
    XclImpRoot( rRoot ),
    mnNextXclIdx( 0 )
{
}

void XclImpNumFmtBuffer::ReadFormat( XclImpStream& rStrm )
{
    String aFormat;
    switch( GetBiff() )
    {
        case EXC_BIFF2:
        case EXC_BIFF3:
            aFormat = rStrm.ReadByteString( false );
        break;

        case EXC_BIFF4:
            rStrm.Ignore( 2 );  // in BIFF4 the index field exists, but is undefined
            aFormat = rStrm.ReadByteString( false );
        break;

        case EXC_BIFF5:
            rStrm >> mnNextXclIdx;
            aFormat = rStrm.ReadByteString( false );
        break;

        case EXC_BIFF8:
            rStrm >> mnNextXclIdx;
            aFormat = rStrm.ReadUniString();
        break;

        default:
            DBG_ERROR_BIFF();
            return;
    }

    if( mnNextXclIdx < 0xFFFF )
    {
        InsertFormat( mnNextXclIdx, aFormat );
        ++mnNextXclIdx;
    }
}

void XclImpNumFmtBuffer::CreateScFormats()
{
    DBG_ASSERT( maIndexMap.empty(), "XclImpNumFmtBuffer::CreateScFormats - already created" );

    SvNumberFormatter& rFormatter = GetFormatter();
    for( XclNumFmtMap::const_iterator aIt = GetFormatMap().begin(), aEnd = GetFormatMap().end(); aIt != aEnd; ++aIt )
    {
        const XclNumFmt& rNumFmt = aIt->second;

        // insert/convert the Excel number format
        xub_StrLen nCheckPos;
        short nType = NUMBERFORMAT_DEFINED;
        sal_uInt32 nKey;
        if( rNumFmt.maFormat.Len() )
        {
            String aFormat( rNumFmt.maFormat );
            rFormatter.PutandConvertEntry( aFormat, nCheckPos,
                nType, nKey, LANGUAGE_ENGLISH_US, rNumFmt.meLanguage );
        }
        else
            nKey = rFormatter.GetFormatIndex( rNumFmt.meOffset, rNumFmt.meLanguage );

        // insert the resulting format key into the Excel->Calc index map
        maIndexMap[ aIt->first ] = nKey;
    }
}

ULONG XclImpNumFmtBuffer::GetScFormat( sal_uInt16 nXclNumFmt ) const
{
    XclImpIndexMap::const_iterator aIt = maIndexMap.find( nXclNumFmt );
    return (aIt != maIndexMap.end()) ? aIt->second : NUMBERFORMAT_ENTRY_NOT_FOUND;
}

void XclImpNumFmtBuffer::FillToItemSet( SfxItemSet& rItemSet, sal_uInt16 nXclNumFmt, bool bSkipPoolDefs ) const
{
    ULONG nScNumFmt = GetScFormat( nXclNumFmt );
    if( nScNumFmt == NUMBERFORMAT_ENTRY_NOT_FOUND )
        nScNumFmt = GetStdScNumFmt();
    FillScFmtToItemSet( rItemSet, nScNumFmt, bSkipPoolDefs );
}

void XclImpNumFmtBuffer::FillScFmtToItemSet( SfxItemSet& rItemSet, ULONG nScNumFmt, bool bSkipPoolDefs ) const
{
    DBG_ASSERT( nScNumFmt != NUMBERFORMAT_ENTRY_NOT_FOUND, "XclImpNumFmtBuffer::FillScFmtToItemSet - invalid number format" );
    ScfTools::PutItem( rItemSet, SfxUInt32Item( ATTR_VALUE_FORMAT, nScNumFmt ), bSkipPoolDefs );
    if( rItemSet.GetItemState( ATTR_VALUE_FORMAT, FALSE ) == SFX_ITEM_SET )
        ScGlobal::AddLanguage( rItemSet, GetFormatter() );
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
    ScfTools::PutItem( rItemSet, ScProtectionAttr( mbLocked, mbHidden ), bSkipPoolDefs );
}


// ----------------------------------------------------------------------------

void XclImpCellAlign::FillFromXF2( sal_uInt8 nFlags )
{
    ::extract_value( mnHorAlign, nFlags, 0, 3 );
}

void XclImpCellAlign::FillFromXF3( sal_uInt16 nAlign )
{
    ::extract_value( mnHorAlign, nAlign, 0, 3 );
    mbLineBreak = ::get_flag( nAlign, EXC_XF_LINEBREAK );   // new in BIFF3
}

void XclImpCellAlign::FillFromXF4( sal_uInt16 nAlign )
{
    FillFromXF3( nAlign );
    ::extract_value( mnVerAlign, nAlign, 4, 2 );            // new in BIFF4
    ::extract_value( mnOrient, nAlign, 6, 2 );              // new in BIFF4
}

void XclImpCellAlign::FillFromXF5( sal_uInt16 nAlign )
{
    ::extract_value( mnHorAlign, nAlign, 0, 3 );
    ::extract_value( mnVerAlign, nAlign, 4, 3 );
    mbLineBreak = ::get_flag( nAlign, EXC_XF_LINEBREAK );
    ::extract_value( mnOrient, nAlign, 8, 2 );
}

void XclImpCellAlign::FillFromXF8( sal_uInt16 nAlign, sal_uInt16 nMiscAttrib )
{
    ::extract_value( mnHorAlign, nAlign, 0, 3 );
    ::extract_value( mnVerAlign, nAlign, 4, 3 );
    mbLineBreak = ::get_flag( nAlign, EXC_XF_LINEBREAK );
    ::extract_value( mnRotation, nAlign, 8, 8 );            // new in BIFF8
    ::extract_value( mnIndent, nMiscAttrib, 0, 4 );         // new in BIFF8
    mbShrink = ::get_flag( nMiscAttrib, EXC_XF8_SHRINK );   // new in BIFF8
    ::extract_value( mnTextDir, nMiscAttrib, 6, 2 );        // new in BIFF8X
}

void XclImpCellAlign::FillToItemSet( SfxItemSet& rItemSet, const XclImpFont* pFont, bool bSkipPoolDefs ) const
{
    // horizontal alignment
    ScfTools::PutItem( rItemSet, SvxHorJustifyItem( GetScHorAlign(), ATTR_HOR_JUSTIFY ), bSkipPoolDefs );

    // text wrap (#i74508# always if vertical alignment is justified or distributed)
    bool bLineBreak = mbLineBreak || (mnVerAlign == EXC_XF_VER_JUSTIFY) || (mnVerAlign == EXC_XF_VER_DISTRIB);
    ScfTools::PutItem( rItemSet, SfxBoolItem( ATTR_LINEBREAK, bLineBreak ), bSkipPoolDefs );

    // vertical alignment
    ScfTools::PutItem( rItemSet, SvxVerJustifyItem( GetScVerAlign(), ATTR_VER_JUSTIFY ), bSkipPoolDefs );

    // indent
    sal_uInt16 nScIndent = mnIndent * 200; // 1 Excel unit == 10 pt == 200 twips
    ScfTools::PutItem( rItemSet, SfxUInt16Item( ATTR_INDENT, nScIndent ), bSkipPoolDefs );

    // shrink to fit
    ScfTools::PutItem( rItemSet, SfxBoolItem( ATTR_SHRINKTOFIT, mbShrink ), bSkipPoolDefs );

    // text orientation/rotation (BIFF2-BIFF7 sets mnOrient)
    sal_uInt8 nXclRot = (mnOrient == EXC_ORIENT_NONE) ? mnRotation : XclTools::GetXclRotFromOrient( mnOrient );
    bool bStacked = (nXclRot == EXC_ROT_STACKED);
    ScfTools::PutItem( rItemSet, SfxBoolItem( ATTR_STACKED, bStacked ), bSkipPoolDefs );
    ScfTools::PutItem( rItemSet, SvxRotateModeItem( SVX_ROTATE_MODE_STANDARD, ATTR_ROTATE_MODE ), bSkipPoolDefs );
    // set an angle in the range from -90 to 90 degrees
    sal_Int32 nAngle = XclTools::GetScRotation( nXclRot, 0 );
    ScfTools::PutItem( rItemSet, SfxInt32Item( ATTR_ROTATE_VALUE, nAngle ), bSkipPoolDefs );
    // #105933# set "Use asian vertical layout", if cell is stacked and font contains CKJ characters
    bool bAsianVert = bStacked && pFont && pFont->HasAsianChars();
    ScfTools::PutItem( rItemSet, SfxBoolItem( ATTR_VERTICAL_ASIAN, bAsianVert ), bSkipPoolDefs );

    // CTL text direction
    ScfTools::PutItem( rItemSet, SvxFrameDirectionItem( GetScFrameDir(), ATTR_WRITINGDIR ), bSkipPoolDefs );
}

// ----------------------------------------------------------------------------

XclImpCellBorder::XclImpCellBorder()
{
    SetUsedFlags( false, false );
}

void XclImpCellBorder::SetUsedFlags( bool bOuterUsed, bool bDiagUsed )
{
    mbLeftUsed = mbRightUsed = mbTopUsed = mbBottomUsed = bOuterUsed;
    mbDiagUsed = bDiagUsed;
}

void XclImpCellBorder::FillFromXF2( sal_uInt8 nFlags )
{
    mnLeftLine   = ::get_flagvalue( nFlags, EXC_XF2_LEFTLINE,   EXC_LINE_THIN, EXC_LINE_NONE );
    mnRightLine  = ::get_flagvalue( nFlags, EXC_XF2_RIGHTLINE,  EXC_LINE_THIN, EXC_LINE_NONE );
    mnTopLine    = ::get_flagvalue( nFlags, EXC_XF2_TOPLINE,    EXC_LINE_THIN, EXC_LINE_NONE );
    mnBottomLine = ::get_flagvalue( nFlags, EXC_XF2_BOTTOMLINE, EXC_LINE_THIN, EXC_LINE_NONE );
    mnLeftColor = mnRightColor = mnTopColor = mnBottomColor = EXC_COLOR_BIFF2_BLACK;
    SetUsedFlags( true, false );
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
    SetUsedFlags( true, false );
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
    SetUsedFlags( true, false );
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
    mbDiagTLtoBR = ::get_flag( nBorder1, EXC_XF_DIAGONAL_TL_TO_BR );
    mbDiagBLtoTR = ::get_flag( nBorder1, EXC_XF_DIAGONAL_BL_TO_TR );
    if( mbDiagTLtoBR || mbDiagBLtoTR )
    {
        ::extract_value( mnDiagLine,  nBorder2, 21, 4 );
        ::extract_value( mnDiagColor, nBorder2, 14, 7 );
    }
    SetUsedFlags( true, true );
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
    mbDiagUsed   = false;
}

namespace {

/** Converts the passed line style to a SvxBorderLine, or returns false, if style is "no line". */
bool lclConvertBorderLine( SvxBorderLine& rLine, const XclImpPalette& rPalette, sal_uInt8 nXclLine, sal_uInt16 nXclColor )
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

} // namespace

void XclImpCellBorder::FillToItemSet( SfxItemSet& rItemSet, const XclImpPalette& rPalette, bool bSkipPoolDefs ) const
{
    if( mbLeftUsed || mbRightUsed || mbTopUsed || mbBottomUsed )
    {
        SvxBoxItem aBoxItem( ATTR_BORDER );
        SvxBorderLine aLine;
        if( mbLeftUsed && lclConvertBorderLine( aLine, rPalette, mnLeftLine, mnLeftColor ) )
            aBoxItem.SetLine( &aLine, BOX_LINE_LEFT );
        if( mbRightUsed && lclConvertBorderLine( aLine, rPalette, mnRightLine, mnRightColor ) )
            aBoxItem.SetLine( &aLine, BOX_LINE_RIGHT );
        if( mbTopUsed && lclConvertBorderLine( aLine, rPalette, mnTopLine, mnTopColor ) )
            aBoxItem.SetLine( &aLine, BOX_LINE_TOP );
        if( mbBottomUsed && lclConvertBorderLine( aLine, rPalette, mnBottomLine, mnBottomColor ) )
            aBoxItem.SetLine( &aLine, BOX_LINE_BOTTOM );
        ScfTools::PutItem( rItemSet, aBoxItem, bSkipPoolDefs );
    }
    if( mbDiagUsed )
    {
        SvxLineItem aTLBRItem( ATTR_BORDER_TLBR );
        SvxLineItem aBLTRItem( ATTR_BORDER_BLTR );
        SvxBorderLine aLine;
        if( lclConvertBorderLine( aLine, rPalette, mnDiagLine, mnDiagColor ) )
        {
            if( mbDiagTLtoBR )
                aTLBRItem.SetLine( &aLine );
            if( mbDiagBLtoTR )
                aBLTRItem.SetLine( &aLine );
        }
        ScfTools::PutItem( rItemSet, aTLBRItem, bSkipPoolDefs );
        ScfTools::PutItem( rItemSet, aBLTRItem, bSkipPoolDefs );
    }
}

// ----------------------------------------------------------------------------

XclImpCellArea::XclImpCellArea()
{
    SetUsedFlags( false );
}

void XclImpCellArea::SetUsedFlags( bool bUsed )
{
    mbForeUsed = mbBackUsed = mbPattUsed = bUsed;
}

void XclImpCellArea::FillFromXF2( sal_uInt8 nFlags )
{
    mnPattern = ::get_flagvalue( nFlags, EXC_XF2_BACKGROUND, EXC_PATT_12_5_PERC, EXC_PATT_NONE );
    mnForeColor = EXC_COLOR_BIFF2_BLACK;
    mnBackColor = EXC_COLOR_BIFF2_WHITE;
    SetUsedFlags( true );
}

void XclImpCellArea::FillFromXF3( sal_uInt16 nArea )
{
    ::extract_value( mnPattern,   nArea,  0, 6 );
    ::extract_value( mnForeColor, nArea,  6, 5 );
    ::extract_value( mnBackColor, nArea, 11, 5 );
    SetUsedFlags( true );
}

void XclImpCellArea::FillFromXF5( sal_uInt32 nArea )
{
    ::extract_value( mnPattern,   nArea, 16, 6 );
    ::extract_value( mnForeColor, nArea,  0, 7 );
    ::extract_value( mnBackColor, nArea,  7, 7 );
    SetUsedFlags( true );
}

void XclImpCellArea::FillFromXF8( sal_uInt32 nBorder2, sal_uInt16 nArea )
{
    ::extract_value( mnPattern,   nBorder2, 26, 6 );
    ::extract_value( mnForeColor, nArea,     0, 7 );
    ::extract_value( mnBackColor, nArea,     7, 7 );
    SetUsedFlags( true );
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
    if( mbPattUsed )    // colors may be both unused in cond. formats
    {
        SvxBrushItem aBrushItem( ATTR_BACKGROUND );

        // #108935# do not use IsTransparent() - old Calc filter writes tranparency with different color indexes
        if( mnPattern == EXC_PATT_NONE )
        {
            aBrushItem.SetColor( Color( COL_TRANSPARENT ) );
        }
        else
        {
            Color aFore( rPalette.GetColor( mbForeUsed ? mnForeColor : EXC_COLOR_WINDOWTEXT ) );
            Color aBack( rPalette.GetColor( mbBackUsed ? mnBackColor : EXC_COLOR_WINDOWBACK ) );
            aBrushItem.SetColor( XclTools::GetPatternColor( aFore, aBack, mnPattern ) );
        }

        ScfTools::PutItem( rItemSet, aBrushItem, bSkipPoolDefs );
    }
}


// ----------------------------------------------------------------------------

XclImpXF::XclImpXF( const XclImpRoot& rRoot ) :
    XclXFBase( true ),      // default is cell XF
    XclImpRoot( rRoot ),
    mpStyleSheet( 0 ),
    mnXclNumFmt( 0 ),
    mnXclFont( 0 ),
    mbWasBuiltIn( false ),
    mbForceCreate( false )
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
    mnXclFont = nReadFont;
    mnXclNumFmt = nReadNumFmt & EXC_XF2_VALFMT_MASK;
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
    mnXclFont = nReadFont;
    mnXclNumFmt = nReadNumFmt;
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
    mnXclFont = nReadFont;
    mnXclNumFmt = nReadNumFmt;
    maAlignment.FillFromXF4( nAlign );
    maBorder.FillFromXF3( nBorder );
    maArea.FillFromXF3( nArea );
}

void XclImpXF::ReadXF5( XclImpStream& rStrm )
{
    sal_uInt32 nArea, nBorder;
    sal_uInt16 nTypeProt, nAlign;
    rStrm >> mnXclFont >> mnXclNumFmt >> nTypeProt >> nAlign >> nArea >> nBorder;

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
    rStrm >> mnXclFont >> mnXclNumFmt >> nTypeProt >> nAlign >> nMiscAttrib >> nBorder1 >> nBorder2 >> nArea;

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
        case EXC_BIFF2: ReadXF2( rStrm );   break;
        case EXC_BIFF3: ReadXF3( rStrm );   break;
        case EXC_BIFF4: ReadXF4( rStrm );   break;
        case EXC_BIFF5: ReadXF5( rStrm );   break;
        case EXC_BIFF8: ReadXF8( rStrm );   break;
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
        mbForceCreate = true;
    }
}

void XclImpXF::SetBuiltInStyleName( sal_uInt8 nStyleId, sal_uInt8 nLevel )
{
    DBG_ASSERT( IsStyleXF(), "XclImpXF::SetStyleName - not a style XF" );
    if( IsStyleXF() && !maStyleName.Len() )
    {
        mbWasBuiltIn = true;
        maStyleName = XclTools::GetBuiltInStyleName( nStyleId, nLevel );
        mbForceCreate = nStyleId == EXC_STYLE_NORMAL;   // force creation of "Default" style
    }
}

void XclImpXF::CreateUserStyle()
{
    if( IsStyleXF() && mbForceCreate )
        CreateStyleSheet();
}

void XclImpXF::ApplyPattern(
        SCCOL nScCol1, SCROW nScRow1, SCCOL nScCol2, SCROW nScRow2,
        SCTAB nScTab, ULONG nForceScNumFmt )
{
    // force creation of cell style and hard formatting, do it here to have mpStyleSheet
    const ScPatternAttr& rPattern = CreatePattern();

    // insert into document
    ScDocument& rDoc = GetDoc();
    if( IsCellXF() && mpStyleSheet )
        rDoc.ApplyStyleAreaTab( nScCol1, nScRow1, nScCol2, nScRow2, nScTab, *mpStyleSheet );
    if( HasUsedFlags() )
        rDoc.ApplyPatternAreaTab( nScCol1, nScRow1, nScCol2, nScRow2, nScTab, rPattern );

    // #108770# apply special number format
    if( nForceScNumFmt != NUMBERFORMAT_ENTRY_NOT_FOUND )
    {
        ScPatternAttr aPattern( GetDoc().GetPool() );
        GetNumFmtBuffer().FillScFmtToItemSet( aPattern.GetItemSet(), nForceScNumFmt );
        rDoc.ApplyPatternAreaTab( nScCol1, nScRow1, nScCol2, nScRow2, nScTab, aPattern );
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
        mbFontUsed = !rParentXF.mbFontUsed || (mnXclFont != rParentXF.mnXclFont);
    if( !mbFmtUsed )
        mbFmtUsed = !rParentXF.mbFmtUsed || (mnXclNumFmt != rParentXF.mnXclNumFmt);
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
        GetFontBuffer().FillToItemSet( rItemSet, EXC_FONTITEM_CELL, mnXclFont, bSkipPoolDefs );

    // value format
    if( mbFmtUsed )
    {
        GetNumFmtBuffer().FillToItemSet( rItemSet, mnXclNumFmt, bSkipPoolDefs );
        // Trace occurrences of Windows date formats
        GetTracer().TraceDates( mnXclNumFmt );
    }

    // alignment
    if( mbAlignUsed )
        maAlignment.FillToItemSet( rItemSet, GetFontBuffer().GetFont( mnXclFont ), bSkipPoolDefs );

    // border
    if( mbBorderUsed )
    {
        maBorder.FillToItemSet( rItemSet, GetPalette(), bSkipPoolDefs );
        GetTracer().TraceBorderLineStyle(maBorder.mnLeftLine > EXC_LINE_HAIR ||
            maBorder.mnRightLine > EXC_LINE_HAIR || maBorder.mnTopLine > EXC_LINE_HAIR ||
            maBorder.mnBottomLine > EXC_LINE_HAIR );
    }

    // area
    if( mbAreaUsed )
    {
        maArea.FillToItemSet( rItemSet, GetPalette(), bSkipPoolDefs );
        GetTracer().TraceFillPattern(maArea.mnPattern != EXC_PATT_NONE &&
            maArea.mnPattern != EXC_PATT_SOLID);
    }

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

    if( (GetBiff() >= EXC_BIFF3) && (maXFList.Count() == 1) )
        // set the name of the "Default" cell style (always the first XF in an Excel file)
        pXF->SetBuiltInStyleName( EXC_STYLE_NORMAL, 0 );
}

void XclImpXFBuffer::ReadStyle( XclImpStream& rStrm )
{
    DBG_ASSERT_BIFF( GetBiff() >= EXC_BIFF3 );

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
            if( GetBiff() <= EXC_BIFF5 )
                aStyleName = rStrm.ReadByteString( false );    // 8 bit length
            else
                aStyleName = rStrm.ReadUniString();
            if( aStyleName.Len() )  // #i1624# #i1768# ignore unnamed styles
                pXF->SetStyleName( aStyleName );
        }
    }
}

sal_uInt16 XclImpXFBuffer::GetFontIndex( sal_uInt16 nXFIndex ) const
{
    const XclImpXF* pXF = GetXF( nXFIndex );
    return pXF ? pXF->GetFontIndex() : EXC_FONT_NOTFOUND;
}

const XclImpFont* XclImpXFBuffer::GetFont( sal_uInt16 nXFIndex ) const
{
    return GetFontBuffer().GetFont( GetFontIndex( nXFIndex ) );
}

void XclImpXFBuffer::CreateUserStyles()
{
    for( XclImpXF* pXF = maXFList.First(); pXF; pXF = maXFList.Next() )
        pXF->CreateUserStyle();
}

void XclImpXFBuffer::ApplyPattern(
        SCCOL nScCol1, SCROW nScRow1, SCCOL nScCol2, SCROW nScRow2,
        SCTAB nScTab, const XclImpXFIndex& rXFIndex )
{
    if( XclImpXF* pXF = GetXF( rXFIndex.GetXFIndex() ) )
    {
        // #108770# set 'Standard' number format for all Boolean cells
        ULONG nForceScNumFmt = rXFIndex.IsBoolCell() ? GetNumFmtBuffer().GetStdScNumFmt() : NUMBERFORMAT_ENTRY_NOT_FOUND;
        pXF->ApplyPattern( nScCol1, nScRow1, nScCol2, nScRow2, nScTab, nForceScNumFmt );
    }
}

// Buffer for XF indexes in cells =============================================

IMPL_FIXEDMEMPOOL_NEWDEL( XclImpXFRange, 100, 500 )

bool XclImpXFRange::Expand( SCROW nScRow, const XclImpXFIndex& rXFIndex )
{
    if( maXFIndex != rXFIndex )
        return false;

    if( mnScRow2 + 1 == nScRow )
    {
        ++mnScRow2;
        return true;
    }
    if( mnScRow1 > 0 && (mnScRow1 - 1 == nScRow) )
    {
        --mnScRow1;
        return true;
    }

    return false;
}

bool XclImpXFRange::Expand( const XclImpXFRange& rNextRange )
{
    DBG_ASSERT( mnScRow2 < rNextRange.mnScRow1, "XclImpXFRange::Expand - rows out of order" );
    if( (maXFIndex == rNextRange.maXFIndex) && (mnScRow2 + 1 == rNextRange.mnScRow1) )
    {
        mnScRow2 = rNextRange.mnScRow2;
        return true;
    }
    return false;
}

// ----------------------------------------------------------------------------

void XclImpXFRangeColumn::SetDefaultXF( const XclImpXFIndex& rXFIndex )
{
    // List should be empty when inserting the default column format.
    // Later explicit SetXF() calls will break up this range.
    DBG_ASSERT( maIndexList.Empty(), "XclImpXFRangeColumn::SetDefaultXF - Setting Default Column XF is not empty" );

    // insert a complete row range with one insert.
    maIndexList.Append( new XclImpXFRange( 0, MAXROW, rXFIndex ) );
}

// ----------------------------------------------------------------------------

void XclImpXFRangeColumn::SetXF( SCROW nScRow, const XclImpXFIndex& rXFIndex )
{
    XclImpXFRange* pPrevRange;
    XclImpXFRange* pNextRange;
    ULONG nNextIndex;

    Find( pPrevRange, pNextRange, nNextIndex, nScRow );

    // previous range:
    // try to overwrite XF (if row is contained in) or try to expand range
    if( pPrevRange )
    {
        if( pPrevRange->Contains( nScRow ) )        // overwrite old XF
        {
            if( rXFIndex == pPrevRange->maXFIndex )
                return;

            SCROW nFirstScRow = pPrevRange->mnScRow1;
            SCROW nLastScRow = pPrevRange->mnScRow2;
            ULONG nIndex = nNextIndex - 1;
            XclImpXFRange* pThisRange = pPrevRange;
            pPrevRange = nIndex ? maIndexList.GetObject( nIndex - 1 ) : 0;

            if( nFirstScRow == nLastScRow )         // replace solely XF
            {
                pThisRange->maXFIndex = rXFIndex;
                TryConcatPrev( nNextIndex );        // try to concat. next with this
                TryConcatPrev( nIndex );            // try to concat. this with previous
            }
            else if( nFirstScRow == nScRow )        // replace first XF
            {
                ++(pThisRange->mnScRow1);
                // try to concatenate with previous of this
                if( !pPrevRange || !pPrevRange->Expand( nScRow, rXFIndex ) )
                    maIndexList.Insert( new XclImpXFRange( nScRow, rXFIndex ), nIndex );
            }
            else if( nLastScRow == nScRow )         // replace last XF
            {
                --(pThisRange->mnScRow2);
                if( !pNextRange || !pNextRange->Expand( nScRow, rXFIndex ) )
                    maIndexList.Insert( new XclImpXFRange( nScRow, rXFIndex ), nNextIndex );
            }
            else                                    // insert in the middle of the range
            {
                pThisRange->mnScRow1 = nScRow + 1;
                // List::Insert() moves entries towards end of list, so insert twice at nIndex
                maIndexList.Insert( new XclImpXFRange( nScRow, rXFIndex ), nIndex );
                maIndexList.Insert( new XclImpXFRange( nFirstScRow, nScRow - 1, pThisRange->maXFIndex ), nIndex );
            }
            return;
        }
        else if( pPrevRange->Expand( nScRow, rXFIndex ) )    // try to expand
        {
            TryConcatPrev( nNextIndex );    // try to concatenate next with expanded
            return;
        }
    }

    // try to expand next range
    if( pNextRange && pNextRange->Expand( nScRow, rXFIndex ) )
        return;

    // create new range
    maIndexList.Insert( new XclImpXFRange( nScRow, rXFIndex ), nNextIndex );
}

void XclImpXFRangeColumn::Find(
        XclImpXFRange*& rpPrevRange, XclImpXFRange*& rpNextRange,
        ULONG& rnNextIndex, SCROW nScRow ) const
{

    // test whether list is empty
    if( maIndexList.Empty() )
    {
        rpPrevRange = rpNextRange = 0;
        rnNextIndex = 0;
        return;
    }

    rpPrevRange = maIndexList.GetObject( 0 );
    rpNextRange = maIndexList.GetObject( maIndexList.Count() - 1 );

    // test whether row is at end of list (contained in or behind last range)
    // rpPrevRange will contain a possible existing row
    if( rpNextRange->mnScRow1 <= nScRow )
    {
        rpPrevRange = rpNextRange;
        rpNextRange = 0;
        rnNextIndex = maIndexList.Count();
        return;
    }

    // test whether row is at beginning of list (really before first range)
    if( nScRow < rpPrevRange->mnScRow1 )
    {
        rpNextRange = rpPrevRange;
        rpPrevRange = 0;
        rnNextIndex = 0;
        return;
    }

    // loop: find range entries before and after new row
    // break the loop if there is no more range between first and last -or-
    // if rpPrevRange contains nScRow (rpNextRange will never contain nScRow)
    ULONG nPrevIndex = 0;
    ULONG nMidIndex;
    rnNextIndex = maIndexList.Count() - 1;
    XclImpXFRange* pMidRange;
    while( ((rnNextIndex - nPrevIndex) > 1) && (rpPrevRange->mnScRow2 < nScRow) )
    {
        nMidIndex = (nPrevIndex + rnNextIndex) / 2;
        pMidRange = maIndexList.GetObject( nMidIndex );
        DBG_ASSERT( pMidRange, "XclImpXFRangeColumn::Find - missing XF index range" );
        if( nScRow < pMidRange->mnScRow1 )      // row is really before pMidRange
        {
            rpNextRange = pMidRange;
            rnNextIndex = nMidIndex;
        }
        else                                    // row is in or after pMidRange
        {
            rpPrevRange = pMidRange;
            nPrevIndex = nMidIndex;
        }
    }

    // find next rpNextRange if rpPrevRange contains nScRow
    if( nScRow <= rpPrevRange->mnScRow2 )
    {
        rnNextIndex = nPrevIndex + 1;
        rpNextRange = maIndexList.GetObject( rnNextIndex );
    }
}

void XclImpXFRangeColumn::TryConcatPrev( ULONG nIndex )
{
    if( !nIndex )
        return;

    XclImpXFRange* pPrevRange = maIndexList.GetObject( nIndex - 1 );
    XclImpXFRange* pNextRange = maIndexList.GetObject( nIndex );
    if( !pPrevRange || !pNextRange )
        return;

    if( pPrevRange->Expand( *pNextRange ) )
        maIndexList.Delete( nIndex );
}

// ----------------------------------------------------------------------------

XclImpXFRangeBuffer::XclImpXFRangeBuffer( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot )
{
}

XclImpXFRangeBuffer::~XclImpXFRangeBuffer()
{
}

void XclImpXFRangeBuffer::Initialize()
{
    maColumns.clear();
    maHyperlinks.clear();
    maMergeList.RemoveAll();
}

void XclImpXFRangeBuffer::SetXF( const ScAddress& rScPos, sal_uInt16 nXFIndex, XclImpXFInsertMode eMode )
{
    SCCOL nScCol = rScPos.Col();
    SCROW nScRow = rScPos.Row();

    // set cell XF's
    size_t nIndex = static_cast< size_t >( nScCol );
    if( maColumns.size() <= nIndex )
        maColumns.resize( nIndex + 1 );
    if( !maColumns[ nIndex ] )
        maColumns[ nIndex ].reset( new XclImpXFRangeColumn );
    // #108770# remember all Boolean cells, they will get 'Standard' number format
    maColumns[ nIndex ]->SetXF( nScRow, XclImpXFIndex( nXFIndex, eMode == xlXFModeBoolCell ) );

    // set "center across selection" and "fill" attribute for all following empty cells
    // #97130# ignore it on row default XFs
    if( eMode != xlXFModeRow )
    {
        const XclImpXF* pXF = GetXFBuffer().GetXF( nXFIndex );
        if( pXF && ((pXF->GetHorAlign() == EXC_XF_HOR_CENTER_AS) || (pXF->GetHorAlign() == EXC_XF_HOR_FILL)) )
        {
            // expand last merged range if this attribute is set repeatedly
            ScRange* pRange = maMergeList.Last();
            if( pRange && (pRange->aEnd.Row() == nScRow) && (pRange->aEnd.Col() + 1 == nScCol)
                    && (eMode == xlXFModeBlank) )
                pRange->aEnd.IncCol();
            else if( eMode != xlXFModeBlank )   // #108781# do not merge empty cells
                SetMerge( nScCol, nScRow );
        }
    }
}

void XclImpXFRangeBuffer::SetXF( const ScAddress& rScPos, sal_uInt16 nXFIndex )
{
    SetXF( rScPos, nXFIndex, xlXFModeCell );
}

void XclImpXFRangeBuffer::SetBlankXF( const ScAddress& rScPos, sal_uInt16 nXFIndex )
{
    SetXF( rScPos, nXFIndex, xlXFModeBlank );
}

void XclImpXFRangeBuffer::SetBoolXF( const ScAddress& rScPos, sal_uInt16 nXFIndex )
{
    SetXF( rScPos, nXFIndex, xlXFModeBoolCell );
}

void XclImpXFRangeBuffer::SetRowDefXF( SCROW nScRow, sal_uInt16 nXFIndex )
{
    for( SCCOL nScCol = 0; nScCol <= MAXCOL; ++nScCol )
        SetXF( ScAddress( nScCol, nScRow, 0 ), nXFIndex, xlXFModeRow );
}

void XclImpXFRangeBuffer::SetColumnDefXF( SCCOL nScCol, sal_uInt16 nXFIndex )
{
    // our array should not have values when creating the default column format.
    size_t nIndex = static_cast< size_t >( nScCol );
    if( maColumns.size() <= nIndex )
        maColumns.resize( nIndex + 1 );
    DBG_ASSERT( !maColumns[ nIndex ], "XclImpXFRangeBuffer::SetColumnDefXF - default column of XFs already has values" );
    maColumns[ nIndex ].reset( new XclImpXFRangeColumn );
    maColumns[ nIndex ]->SetDefaultXF( XclImpXFIndex( nXFIndex ) );
}

void XclImpXFRangeBuffer::SetBorderLine( const ScRange& rRange, SCTAB nScTab, USHORT nLine )
{
    SCCOL nFromScCol = (nLine == BOX_LINE_RIGHT) ? rRange.aEnd.Col() : rRange.aStart.Col();
    SCROW nFromScRow = (nLine == BOX_LINE_BOTTOM) ? rRange.aEnd.Row() : rRange.aStart.Row();
    ScDocument& rDoc = GetDoc();

    const SvxBoxItem* pFromItem = static_cast< const SvxBoxItem* >(
        rDoc.GetAttr( nFromScCol, nFromScRow, nScTab, ATTR_BORDER ) );
    const SvxBoxItem* pToItem = static_cast< const SvxBoxItem* >(
        rDoc.GetAttr( rRange.aStart.Col(), rRange.aStart.Row(), nScTab, ATTR_BORDER ) );

    SvxBoxItem aNewItem( *pToItem );
    aNewItem.SetLine( pFromItem->GetLine( nLine ), nLine );
    rDoc.ApplyAttr( rRange.aStart.Col(), rRange.aStart.Row(), nScTab, aNewItem );
}

void XclImpXFRangeBuffer::SetHyperlink( const XclRange& rXclRange, const String& rUrl )
{
    maHyperlinks.push_back( XclImpHyperlinkRange( rXclRange, rUrl ) );
}

void XclImpXFRangeBuffer::SetMerge( SCCOL nScCol, SCROW nScRow )
{
    maMergeList.Append( ScRange( nScCol, nScRow, 0 ) );
}

void XclImpXFRangeBuffer::SetMerge( SCCOL nScCol1, SCROW nScRow1, SCCOL nScCol2, SCROW nScRow2 )
{
    if( (nScCol1 < nScCol2) || (nScRow1 < nScRow2) )
        maMergeList.Append( ScRange( nScCol1, nScRow1, 0, nScCol2, nScRow2, 0 ) );
}

void XclImpXFRangeBuffer::Finalize()
{
    ScDocument& rDoc = GetDoc();
    SCTAB nScTab = GetCurrScTab();

    // apply patterns
    XclImpXFBuffer& rXFBuffer = GetXFBuffer();
    for( XclImpXFRangeColumnVec::const_iterator aVBeg = maColumns.begin(), aVEnd = maColumns.end(), aVIt = aVBeg; aVIt != aVEnd; ++aVIt )
    {
        // apply all cell styles of an existing column
        if( aVIt->is() )
        {
            XclImpXFRangeColumn& rColumn = **aVIt;
            SCCOL nScCol = static_cast< SCCOL >( aVIt - aVBeg );
            for( XclImpXFRange* pStyle = rColumn.First(); pStyle; pStyle = rColumn.Next() )
                rXFBuffer.ApplyPattern( nScCol, pStyle->mnScRow1, nScCol, pStyle->mnScRow2, nScTab, pStyle->maXFIndex );
        }
    }

    // insert hyperlink cells
    for( XclImpHyperlinkList::const_iterator aLIt = maHyperlinks.begin(), aLEnd = maHyperlinks.end(); aLIt != aLEnd; ++aLIt )
        XclImpHyperlink::InsertUrl( GetRoot(), aLIt->first, aLIt->second );

    // apply cell merging
    for( const ScRange* pRange = maMergeList.First(); pRange; pRange = maMergeList.Next() )
    {
        bool bMultiCol = (pRange->aStart.Col() != pRange->aEnd.Col());
        bool bMultiRow = (pRange->aStart.Row() != pRange->aEnd.Row());
        // set correct right border
        if( bMultiCol )
            SetBorderLine( *pRange, nScTab, BOX_LINE_RIGHT );
        // set correct lower border
        if( bMultiRow )
            SetBorderLine( *pRange, nScTab, BOX_LINE_BOTTOM );
        // do merge
        if( bMultiCol || bMultiRow )
            rDoc.DoMerge( nScTab, pRange->aStart.Col(), pRange->aStart.Row(),
                pRange->aEnd.Col(), pRange->aEnd.Row() );
    }
}

// ============================================================================

