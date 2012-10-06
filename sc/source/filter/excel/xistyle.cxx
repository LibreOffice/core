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

#include "xistyle.hxx"
#include <sfx2/printer.hxx>
#include <sfx2/objsh.hxx>
#include <svtools/ctrltool.hxx>
#include <editeng/editobj.hxx>
#include "scitems.hxx"
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/crsditem.hxx>
#include <editeng/cntritem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/escpitem.hxx>
#include <svx/algitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/bolnitem.hxx>
#include <svx/rotmodit.hxx>
#include <editeng/colritem.hxx>
#include <editeng/brshitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/flstitem.hxx>
#include <editeng/justifyitem.hxx>
#include <sal/macros.h>
#include "document.hxx"
#include "docpool.hxx"
#include "attrib.hxx"
#include "stlpool.hxx"
#include "stlsheet.hxx"
#include "cell.hxx"
#include "globstr.hrc"
#include "attarray.hxx"
#include "xltracer.hxx"
#include "xistream.hxx"
#include "xicontent.hxx"

#include "root.hxx"
#include "colrowst.hxx"
#include "svl/poolcach.hxx"

#include <list>

using ::std::list;

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
using namespace ::com::sun::star;

typedef ::cppu::WeakImplHelper1< container::XIndexAccess > XIndexAccess_BASE;
typedef ::std::vector< ColorData > ColorDataVec;

class PaletteIndex : public XIndexAccess_BASE
{
public:
    PaletteIndex( const ColorDataVec& rColorDataTable ) : maColorData( rColorDataTable ) {}

    // Methods XIndexAccess
    virtual ::sal_Int32 SAL_CALL getCount() throw (uno::RuntimeException)
    {
         return  maColorData.size();
    }

    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
    {
        //--Index;  // apparently the palette is already 1 based
        return uno::makeAny( sal_Int32( maColorData[ Index ] ) );
    }

    // Methods XElementAcess
    virtual uno::Type SAL_CALL getElementType() throw (uno::RuntimeException)
    {
        return ::getCppuType( (sal_Int32*)0 );
    }
    virtual ::sal_Bool SAL_CALL hasElements() throw (uno::RuntimeException)
    {
        return (maColorData.size() > 0);
    }

private:
    ColorDataVec        maColorData;
};

void
XclImpPalette::ExportPalette()
{
    if( SfxObjectShell* pDocShell = mrRoot.GetDocShell() )
    {
        // copy values in color palette
        sal_Int16 nColors =  maColorTable.size();
        ColorDataVec aColors;
        aColors.resize( nColors );
        for( sal_uInt16 nIndex = 0; nIndex < nColors; ++nIndex )
            aColors[ nIndex ] = GetColorData( nIndex );

        uno::Reference< beans::XPropertySet > xProps( pDocShell->GetModel(), uno::UNO_QUERY );
        if ( xProps.is() )
        {
            uno::Reference< container::XIndexAccess > xIndex( new PaletteIndex( aColors ) );
            xProps->setPropertyValue( "ColorPalette", uno::makeAny( xIndex ) );
        }
    }

}
// PALETTE record - color information =========================================

XclImpPalette::XclImpPalette( const XclImpRoot& rRoot ) :
    XclDefaultPalette( rRoot ), mrRoot( rRoot )
{
}

void XclImpPalette::Initialize()
{
    maColorTable.clear();
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
    OSL_ENSURE( rStrm.GetRecLeft() == static_cast< sal_Size >( 4 * nCount ),
        "XclImpPalette::ReadPalette - size mismatch" );

    maColorTable.resize( nCount );
    Color aColor;
    for( sal_uInt16 nIndex = 0; nIndex < nCount; ++nIndex )
    {
        rStrm >> aColor;
        maColorTable[ nIndex ] = aColor.GetColor();
    }
    ExportPalette();
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
    XclImpRoot( rRoot )
{
    SetFontData( rFontData, false );
}

void XclImpFont::SetAllUsedFlags( bool bUsed )
{
    mbFontNameUsed = mbHeightUsed = mbColorUsed = mbWeightUsed = mbEscapemUsed =
        mbUnderlUsed = mbItalicUsed = mbStrikeUsed = mbOutlineUsed = mbShadowUsed = bUsed;
}

void XclImpFont::SetFontData( const XclFontData& rFontData, bool bHasCharSet )
{
    maData = rFontData;
    mbHasCharSet = bHasCharSet;
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
    OSL_ENSURE_BIFF( GetBiff() == EXC_BIFF8 );
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
        // set only for valid script types
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
            nHeight = (nHeight * 127 + 36) / EXC_POINTS_PER_INCH;   // 1 in == 72 pt

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

    // find the script types for which the font contains characters
    if( OutputDevice* pPrinter = GetPrinter() )
    {
        Font aFont( maData.maName, Size( 0, 10 ) );
        FontCharMap aCharMap;

        pPrinter->SetFont( aFont );
        if( pPrinter->GetFontCharMap( aCharMap ) )
        {
            // CJK fonts
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
            // CTL fonts
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
    XclImpRoot( rRoot ),
    maFont4( rRoot ),
    maCtrlFont( rRoot )
{
    Initialize();

    // default font for form controls without own font information
    XclFontData aCtrlFontData;
    switch( GetBiff() )
    {
        case EXC_BIFF2:
        case EXC_BIFF3:
        case EXC_BIFF4:
        case EXC_BIFF5:
            aCtrlFontData.maName.AssignAscii( "Helv" );
            aCtrlFontData.mnHeight = 160;
            aCtrlFontData.mnWeight = EXC_FONTWGHT_BOLD;
        break;
        case EXC_BIFF8:
            aCtrlFontData.maName.AssignAscii( "Tahoma" );
            aCtrlFontData.mnHeight = 160;
            aCtrlFontData.mnWeight = EXC_FONTWGHT_NORMAL;
        break;
        default:
            DBG_ERROR_BIFF();
    }
    maCtrlFont.SetFontData( aCtrlFontData, false );
}

void XclImpFontBuffer::Initialize()
{
    maFontList.clear();

    // application font for column width calculation, later filled with first font from font list
    XclFontData aAppFontData;
    aAppFontData.maName.AssignAscii( "Arial" );
    aAppFontData.mnHeight = 200;
    aAppFontData.mnWeight = EXC_FONTWGHT_NORMAL;
    UpdateAppFont( aAppFontData, false );
}

const XclImpFont* XclImpFontBuffer::GetFont( sal_uInt16 nFontIndex ) const
{
    /*  Font with index 4 is not stored in an Excel file, but used e.g. by
        BIFF5 form pushbutton objects. It is the bold default font.
        This also means that entries above 4 are out by one in the list. */

    if (nFontIndex == 4)
        return &maFont4;

    if (nFontIndex < 4)
    {
        // Font ID is zero-based when it's less than 4.
        return nFontIndex >= maFontList.size() ? NULL : &maFontList[nFontIndex];
    }

    // Font ID is greater than 4.  It is now 1-based.
    return nFontIndex > maFontList.size() ? NULL : &maFontList[nFontIndex-1];
}

void XclImpFontBuffer::ReadFont( XclImpStream& rStrm )
{
    XclImpFont* pFont = new XclImpFont( GetRoot() );
    pFont->ReadFont( rStrm );
    maFontList.push_back( pFont );

    if( maFontList.size() == 1 )
    {
        UpdateAppFont( pFont->GetFontData(), pFont->HasCharSet() );
        // #i71033# set text encoding from application font, if CODEPAGE is missing
        SetAppFontEncoding( pFont->GetFontEncoding() );
    }
}

void XclImpFontBuffer::ReadEfont( XclImpStream& rStrm )
{
    if( !maFontList.empty() )
        maFontList.back().ReadEfont( rStrm );
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

void XclImpFontBuffer::WriteDefaultCtrlFontProperties( ScfPropertySet& rPropSet ) const
{
    maCtrlFont.WriteFontProperties( rPropSet, EXC_FONTPROPSET_CONTROL );
}

void XclImpFontBuffer::UpdateAppFont( const XclFontData& rFontData, bool bHasCharSet )
{
    maAppFont = rFontData;
    // #i3006# Calculate the width of '0' from first font and current printer.
    SetCharWidth( maAppFont );

    // font 4 is bold font 0
    XclFontData aFont4Data( maAppFont );
    aFont4Data.mnWeight = EXC_FONTWGHT_BOLD;
    maFont4.SetFontData( aFont4Data, bHasCharSet );
}

// FORMAT record - number formats =============================================

XclImpNumFmtBuffer::XclImpNumFmtBuffer( const XclImpRoot& rRoot ) :
    XclNumFmtBuffer( rRoot ),
    XclImpRoot( rRoot ),
    mnNextXclIdx( 0 )
{
}

void XclImpNumFmtBuffer::Initialize()
{
    maIndexMap.clear();
    mnNextXclIdx = 0;
    InitializeImport();     // base class
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
    OSL_ENSURE( maIndexMap.empty(), "XclImpNumFmtBuffer::CreateScFormats - already created" );

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

sal_uLong XclImpNumFmtBuffer::GetScFormat( sal_uInt16 nXclNumFmt ) const
{
    XclImpIndexMap::const_iterator aIt = maIndexMap.find( nXclNumFmt );
    return (aIt != maIndexMap.end()) ? aIt->second : NUMBERFORMAT_ENTRY_NOT_FOUND;
}

void XclImpNumFmtBuffer::FillToItemSet( SfxItemSet& rItemSet, sal_uInt16 nXclNumFmt, bool bSkipPoolDefs ) const
{
    sal_uLong nScNumFmt = GetScFormat( nXclNumFmt );
    if( nScNumFmt == NUMBERFORMAT_ENTRY_NOT_FOUND )
        nScNumFmt = GetStdScNumFmt();
    FillScFmtToItemSet( rItemSet, nScNumFmt, bSkipPoolDefs );
}

void XclImpNumFmtBuffer::FillScFmtToItemSet( SfxItemSet& rItemSet, sal_uLong nScNumFmt, bool bSkipPoolDefs ) const
{
    OSL_ENSURE( nScNumFmt != NUMBERFORMAT_ENTRY_NOT_FOUND, "XclImpNumFmtBuffer::FillScFmtToItemSet - invalid number format" );
    ScfTools::PutItem( rItemSet, SfxUInt32Item( ATTR_VALUE_FORMAT, nScNumFmt ), bSkipPoolDefs );
    if( rItemSet.GetItemState( ATTR_VALUE_FORMAT, false ) == SFX_ITEM_SET )
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
    mnHorAlign = ::extract_value< sal_uInt8 >( nFlags, 0, 3 );
}

void XclImpCellAlign::FillFromXF3( sal_uInt16 nAlign )
{
    mnHorAlign = ::extract_value< sal_uInt8 >( nAlign, 0, 3 );
    mbLineBreak = ::get_flag( nAlign, EXC_XF_LINEBREAK );   // new in BIFF3
}

void XclImpCellAlign::FillFromXF4( sal_uInt16 nAlign )
{
    FillFromXF3( nAlign );
    mnVerAlign = ::extract_value< sal_uInt8 >( nAlign, 4, 2 );  // new in BIFF4
    mnOrient = ::extract_value< sal_uInt8 >( nAlign, 6, 2 );    // new in BIFF4
}

void XclImpCellAlign::FillFromXF5( sal_uInt16 nAlign )
{
    mnHorAlign = ::extract_value< sal_uInt8 >( nAlign, 0, 3 );
    mnVerAlign = ::extract_value< sal_uInt8 >( nAlign, 4, 3 );
    mbLineBreak = ::get_flag( nAlign, EXC_XF_LINEBREAK );
    mnOrient = ::extract_value< sal_uInt8 >( nAlign, 8, 2 );
}

void XclImpCellAlign::FillFromXF8( sal_uInt16 nAlign, sal_uInt16 nMiscAttrib )
{
    mnHorAlign = ::extract_value< sal_uInt8 >( nAlign, 0, 3 );
    mnVerAlign = ::extract_value< sal_uInt8 >( nAlign, 4, 3 );
    mbLineBreak = ::get_flag( nAlign, EXC_XF_LINEBREAK );
    mnRotation = ::extract_value< sal_uInt8 >( nAlign, 8, 8 );      // new in BIFF8
    mnIndent = ::extract_value< sal_uInt8 >( nMiscAttrib, 0, 4 );   // new in BIFF8
    mbShrink = ::get_flag( nMiscAttrib, EXC_XF8_SHRINK );           // new in BIFF8
    mnTextDir = ::extract_value< sal_uInt8 >( nMiscAttrib, 6, 2 );  // new in BIFF8
}

void XclImpCellAlign::FillToItemSet( SfxItemSet& rItemSet, const XclImpFont* pFont, bool bSkipPoolDefs ) const
{
    // horizontal alignment
    ScfTools::PutItem( rItemSet, SvxHorJustifyItem( GetScHorAlign(), ATTR_HOR_JUSTIFY ), bSkipPoolDefs );
    ScfTools::PutItem( rItemSet, SvxJustifyMethodItem( GetScHorJustifyMethod(), ATTR_HOR_JUSTIFY_METHOD ), bSkipPoolDefs );

    // text wrap (#i74508# always if vertical alignment is justified or distributed)
    bool bLineBreak = mbLineBreak || (mnVerAlign == EXC_XF_VER_JUSTIFY) || (mnVerAlign == EXC_XF_VER_DISTRIB);
    ScfTools::PutItem( rItemSet, SfxBoolItem( ATTR_LINEBREAK, bLineBreak ), bSkipPoolDefs );

    // vertical alignment
    ScfTools::PutItem( rItemSet, SvxVerJustifyItem( GetScVerAlign(), ATTR_VER_JUSTIFY ), bSkipPoolDefs );
    ScfTools::PutItem( rItemSet, SvxJustifyMethodItem( GetScVerJustifyMethod(), ATTR_VER_JUSTIFY_METHOD ), bSkipPoolDefs );

    // indent
    sal_uInt16 nScIndent = mnIndent * 200; // 1 Excel unit == 10 pt == 200 twips
    ScfTools::PutItem( rItemSet, SfxUInt16Item( ATTR_INDENT, nScIndent ), bSkipPoolDefs );

    // shrink to fit
    ScfTools::PutItem( rItemSet, SfxBoolItem( ATTR_SHRINKTOFIT, mbShrink ), bSkipPoolDefs );

    // text orientation/rotation (BIFF2-BIFF7 sets mnOrient)
    sal_uInt8 nXclRot = (mnOrient == EXC_ORIENT_NONE) ? mnRotation : XclTools::GetXclRotFromOrient( mnOrient );
    bool bStacked = (nXclRot == EXC_ROT_STACKED);
    ScfTools::PutItem( rItemSet, SfxBoolItem( ATTR_STACKED, bStacked ), bSkipPoolDefs );
    // set an angle in the range from -90 to 90 degrees
    sal_Int32 nAngle = XclTools::GetScRotation( nXclRot, 0 );
    ScfTools::PutItem( rItemSet, SfxInt32Item( ATTR_ROTATE_VALUE, nAngle ), bSkipPoolDefs );
    // set "Use asian vertical layout", if cell is stacked and font contains CKJ characters
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
    mnTopLine     = ::extract_value< sal_uInt8  >( nBorder,  0, 3 );
    mnLeftLine    = ::extract_value< sal_uInt8  >( nBorder,  8, 3 );
    mnBottomLine  = ::extract_value< sal_uInt8  >( nBorder, 16, 3 );
    mnRightLine   = ::extract_value< sal_uInt8  >( nBorder, 24, 3 );
    mnTopColor    = ::extract_value< sal_uInt16 >( nBorder,  3, 5 );
    mnLeftColor   = ::extract_value< sal_uInt16 >( nBorder, 11, 5 );
    mnBottomColor = ::extract_value< sal_uInt16 >( nBorder, 19, 5 );
    mnRightColor  = ::extract_value< sal_uInt16 >( nBorder, 27, 5 );
    SetUsedFlags( true, false );
}

void XclImpCellBorder::FillFromXF5( sal_uInt32 nBorder, sal_uInt32 nArea )
{
    mnTopLine     = ::extract_value< sal_uInt8  >( nBorder,  0, 3 );
    mnLeftLine    = ::extract_value< sal_uInt8  >( nBorder,  3, 3 );
    mnBottomLine  = ::extract_value< sal_uInt8  >( nArea,   22, 3 );
    mnRightLine   = ::extract_value< sal_uInt8  >( nBorder,  6, 3 );
    mnTopColor    = ::extract_value< sal_uInt16 >( nBorder,  9, 7 );
    mnLeftColor   = ::extract_value< sal_uInt16 >( nBorder, 16, 7 );
    mnBottomColor = ::extract_value< sal_uInt16 >( nArea,   25, 7 );
    mnRightColor  = ::extract_value< sal_uInt16 >( nBorder, 23, 7 );
    SetUsedFlags( true, false );
}

void XclImpCellBorder::FillFromXF8( sal_uInt32 nBorder1, sal_uInt32 nBorder2 )
{
    mnLeftLine    = ::extract_value< sal_uInt8  >( nBorder1,  0, 4 );
    mnRightLine   = ::extract_value< sal_uInt8  >( nBorder1,  4, 4 );
    mnTopLine     = ::extract_value< sal_uInt8  >( nBorder1,  8, 4 );
    mnBottomLine  = ::extract_value< sal_uInt8  >( nBorder1, 12, 4 );
    mnLeftColor   = ::extract_value< sal_uInt16 >( nBorder1, 16, 7 );
    mnRightColor  = ::extract_value< sal_uInt16 >( nBorder1, 23, 7 );
    mnTopColor    = ::extract_value< sal_uInt16 >( nBorder2,  0, 7 );
    mnBottomColor = ::extract_value< sal_uInt16 >( nBorder2,  7, 7 );
    mbDiagTLtoBR  = ::get_flag( nBorder1, EXC_XF_DIAGONAL_TL_TO_BR );
    mbDiagBLtoTR  = ::get_flag( nBorder1, EXC_XF_DIAGONAL_BL_TO_TR );
    if( mbDiagTLtoBR || mbDiagBLtoTR )
    {
        mnDiagLine  = ::extract_value< sal_uInt8 >( nBorder2, 21, 4 );
        mnDiagColor = ::extract_value< sal_uInt16 >( nBorder2, 14, 7 );
    }
    SetUsedFlags( true, true );
}

void XclImpCellBorder::FillFromCF8( sal_uInt16 nLineStyle, sal_uInt32 nLineColor, sal_uInt32 nFlags )
{
    mnLeftLine    = ::extract_value< sal_uInt8  >( nLineStyle,  0, 4 );
    mnRightLine   = ::extract_value< sal_uInt8  >( nLineStyle,  4, 4 );
    mnTopLine     = ::extract_value< sal_uInt8  >( nLineStyle,  8, 4 );
    mnBottomLine  = ::extract_value< sal_uInt8  >( nLineStyle, 12, 4 );
    mnLeftColor   = ::extract_value< sal_uInt16 >( nLineColor,  0, 7 );
    mnRightColor  = ::extract_value< sal_uInt16 >( nLineColor,  7, 7 );
    mnTopColor    = ::extract_value< sal_uInt16 >( nLineColor, 16, 7 );
    mnBottomColor = ::extract_value< sal_uInt16 >( nLineColor, 23, 7 );
    mbLeftUsed    = !::get_flag( nFlags, EXC_CF_BORDER_LEFT );
    mbRightUsed   = !::get_flag( nFlags, EXC_CF_BORDER_RIGHT );
    mbTopUsed     = !::get_flag( nFlags, EXC_CF_BORDER_TOP );
    mbBottomUsed  = !::get_flag( nFlags, EXC_CF_BORDER_BOTTOM );
    mbDiagUsed    = false;
}

bool XclImpCellBorder::HasAnyOuterBorder() const
{
    return
        (mbLeftUsed   && (mnLeftLine != EXC_LINE_NONE)) ||
        (mbRightUsed  && (mnRightLine != EXC_LINE_NONE)) ||
        (mbTopUsed    && (mnTopLine != EXC_LINE_NONE)) ||
        (mbBottomUsed && (mnBottomLine != EXC_LINE_NONE));
}

namespace {

/** Converts the passed line style to a ::editeng::SvxBorderLine, or returns false, if style is "no line". */
bool lclConvertBorderLine( ::editeng::SvxBorderLine& rLine, const XclImpPalette& rPalette, sal_uInt8 nXclLine, sal_uInt16 nXclColor )
{
    static const sal_uInt16 ppnLineParam[][ 4 ] =
    {
        //  outer width,        type
        {   0,                  table::BorderLineStyle::SOLID },  // 0 = none
        {   EXC_BORDER_THIN,    table::BorderLineStyle::SOLID },  // 1 = thin
        {   EXC_BORDER_MEDIUM,  table::BorderLineStyle::SOLID },  // 2 = medium
        {   EXC_BORDER_THIN,    table::BorderLineStyle::DASHED }, // 3 = dashed
        {   EXC_BORDER_THIN,    table::BorderLineStyle::DOTTED }, // 4 = dotted
        {   EXC_BORDER_THICK,   table::BorderLineStyle::SOLID },  // 5 = thick
        {   EXC_BORDER_THIN,    table::BorderLineStyle::DOUBLE }, // 6 = double
        {   EXC_BORDER_HAIR,    table::BorderLineStyle::SOLID },  // 7 = hair
        {   EXC_BORDER_MEDIUM,  table::BorderLineStyle::DASHED }, // 8 = med dash
        {   EXC_BORDER_THIN,    table::BorderLineStyle::SOLID },  // 9 = thin dashdot
        {   EXC_BORDER_MEDIUM,  table::BorderLineStyle::SOLID },  // A = med dashdot
        {   EXC_BORDER_THIN,    table::BorderLineStyle::SOLID },  // B = thin dashdotdot
        {   EXC_BORDER_MEDIUM,  table::BorderLineStyle::SOLID },  // C = med dashdotdot
        {   EXC_BORDER_MEDIUM,  table::BorderLineStyle::SOLID }   // D = med slant dashdot
    };

    if( nXclLine == EXC_LINE_NONE )
        return false;
    if( nXclLine >= SAL_N_ELEMENTS( ppnLineParam ) )
        nXclLine = EXC_LINE_THIN;

    rLine.SetColor( rPalette.GetColor( nXclColor ) );
    rLine.SetWidth( ppnLineParam[ nXclLine ][ 0 ] );
    rLine.SetBorderLineStyle( static_cast< ::editeng::SvxBorderStyle>(
                ppnLineParam[ nXclLine ][ 1 ]) );
    return true;
}

} // namespace

void XclImpCellBorder::FillToItemSet( SfxItemSet& rItemSet, const XclImpPalette& rPalette, bool bSkipPoolDefs ) const
{
    if( mbLeftUsed || mbRightUsed || mbTopUsed || mbBottomUsed )
    {
        SvxBoxItem aBoxItem( ATTR_BORDER );
        ::editeng::SvxBorderLine aLine;
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
        ::editeng::SvxBorderLine aLine;
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
    mnPattern   = ::extract_value< sal_uInt8  >( nArea,  0, 6 );
    mnForeColor = ::extract_value< sal_uInt16 >( nArea,  6, 5 );
    mnBackColor = ::extract_value< sal_uInt16 >( nArea, 11, 5 );
    SetUsedFlags( true );
}

void XclImpCellArea::FillFromXF5( sal_uInt32 nArea )
{
    mnPattern   = ::extract_value< sal_uInt8  >( nArea, 16, 6 );
    mnForeColor = ::extract_value< sal_uInt16 >( nArea,  0, 7 );
    mnBackColor = ::extract_value< sal_uInt16 >( nArea,  7, 7 );
    SetUsedFlags( true );
}

void XclImpCellArea::FillFromXF8( sal_uInt32 nBorder2, sal_uInt16 nArea )
{
    mnPattern   = ::extract_value< sal_uInt8  >( nBorder2, 26, 6 );
    mnForeColor = ::extract_value< sal_uInt16 >( nArea,     0, 7 );
    mnBackColor = ::extract_value< sal_uInt16 >( nArea,     7, 7 );
    SetUsedFlags( true );
}

void XclImpCellArea::FillFromCF8( sal_uInt16 nPattern, sal_uInt16 nColor, sal_uInt32 nFlags )
{
    mnForeColor = ::extract_value< sal_uInt16 >( nColor,    0, 7 );
    mnBackColor = ::extract_value< sal_uInt16 >( nColor,    7, 7 );
    mnPattern   = ::extract_value< sal_uInt8  >( nPattern, 10, 6 );
    mbForeUsed  = !::get_flag( nFlags, EXC_CF_AREA_FGCOLOR );
    mbBackUsed  = !::get_flag( nFlags, EXC_CF_AREA_BGCOLOR );
    mbPattUsed  = !::get_flag( nFlags, EXC_CF_AREA_PATTERN );

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

        // do not use IsTransparent() - old Calc filter writes tranparency with different color indexes
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
    mnXclFont( 0 )
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

    // XF type/parent, attribute used flags
    mbCellXF = !::get_flag( nTypeProt, EXC_XF_STYLE );          // new in BIFF3
    mnParent = ::extract_value< sal_uInt16 >( nAlign, 4, 12 );  // new in BIFF3
    SetUsedFlags( ::extract_value< sal_uInt8 >( nTypeProt, 10, 6 ) );

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

    // XF type/parent, attribute used flags
    mbCellXF = !::get_flag( nTypeProt, EXC_XF_STYLE );
    mnParent = ::extract_value< sal_uInt16 >( nTypeProt, 4, 12 );
    SetUsedFlags( ::extract_value< sal_uInt8 >( nAlign, 10, 6 ) );

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

    // XF type/parent, attribute used flags
    mbCellXF = !::get_flag( nTypeProt, EXC_XF_STYLE );
    mnParent = ::extract_value< sal_uInt16 >( nTypeProt, 4, 12 );
    SetUsedFlags( ::extract_value< sal_uInt8 >( nAlign, 10, 6 ) );

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

    // XF type/parent, attribute used flags
    mbCellXF = !::get_flag( nTypeProt, EXC_XF_STYLE );
    mnParent = ::extract_value< sal_uInt16 >( nTypeProt, 4, 12 );
    SetUsedFlags( ::extract_value< sal_uInt8 >( nMiscAttrib, 10, 6 ) );

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

const ScPatternAttr& XclImpXF::CreatePattern( bool bSkipPoolDefs )
{
    if( mpPattern.get() )
        return *mpPattern;

    // create new pattern attribute set
    mpPattern.reset( new ScPatternAttr( GetDoc().GetPool() ) );
    SfxItemSet& rItemSet = mpPattern->GetItemSet();
    XclImpXF* pParentXF = IsCellXF() ? GetXFBuffer().GetXF( mnParent ) : 0;

    // parent cell style
    if( IsCellXF() && !mpStyleSheet )
    {
        mpStyleSheet = GetXFBuffer().CreateStyleSheet( mnParent );

        /*  Enables mb***Used flags, if the formatting attributes differ from
            the passed XF record. In cell XFs Excel uses the cell attributes,
            if they differ from the parent style XF.
            ...or if the respective flag is not set in parent style XF. */
        if( pParentXF )
        {
            if( !mbProtUsed )
                mbProtUsed = !pParentXF->mbProtUsed || !(maProtection == pParentXF->maProtection);
            if( !mbFontUsed )
                mbFontUsed = !pParentXF->mbFontUsed || (mnXclFont != pParentXF->mnXclFont);
            if( !mbFmtUsed )
                mbFmtUsed = !pParentXF->mbFmtUsed || (mnXclNumFmt != pParentXF->mnXclNumFmt);
            if( !mbAlignUsed )
                mbAlignUsed = !pParentXF->mbAlignUsed || !(maAlignment == pParentXF->maAlignment);
            if( !mbBorderUsed )
                mbBorderUsed = !pParentXF->mbBorderUsed || !(maBorder == pParentXF->maBorder);
            if( !mbAreaUsed )
                mbAreaUsed = !pParentXF->mbAreaUsed || !(maArea == pParentXF->maArea);
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

    /*  #i38709# Decide which rotation reference mode to use. If any outer
        border line of the cell is set (either explicitly or via cell style),
        and the cell contents are rotated, set rotation reference to bottom of
        cell. This causes the borders to be painted rotated with the text. */
    if( mbAlignUsed || mbBorderUsed )
    {
        SvxRotateMode eRotateMode = SVX_ROTATE_MODE_STANDARD;
        const XclImpCellAlign* pAlign = mbAlignUsed ? &maAlignment : (pParentXF ? &pParentXF->maAlignment : 0);
        const XclImpCellBorder* pBorder = mbBorderUsed ? &maBorder : (pParentXF ? &pParentXF->maBorder : 0);
        if( pAlign && pBorder && (0 < pAlign->mnRotation) && (pAlign->mnRotation <= 180) && pBorder->HasAnyOuterBorder() )
            eRotateMode = SVX_ROTATE_MODE_BOTTOM;
        ScfTools::PutItem( rItemSet, SvxRotateModeItem( eRotateMode, ATTR_ROTATE_MODE ), bSkipPoolDefs );
    }

    // Excel's cell margins are different from Calc's default margins.
    SvxMarginItem aItem(40, 40, 40, 40, ATTR_MARGIN);
    ScfTools::PutItem(rItemSet, aItem, bSkipPoolDefs);

    return *mpPattern;
}

void XclImpXF::ApplyPatternToAttrList(
    list<ScAttrEntry>& rAttrs, SCROW nRow1, SCROW nRow2, sal_uInt32 nForceScNumFmt)
{
    // force creation of cell style and hard formatting, do it here to have mpStyleSheet
    CreatePattern();
    ScPatternAttr& rPat = *mpPattern;

    // insert into document
    ScDocument& rDoc = GetDoc();

    if (IsCellXF())
    {
        if (mpStyleSheet)
        {
            // Apply style sheet.  Don't clear the direct formats.
            rPat.SetStyleSheet(mpStyleSheet, false);
        }
        else
        {
            // When the cell format is not associated with any style, use the
            // 'Default' style.  Some buggy XLS docs generated by apps other
            // than Excel (such as 1C) may not have any built-in styles at
            // all.
            ScStyleSheetPool* pStylePool = rDoc.GetStyleSheetPool();
            if (pStylePool)
            {
                ScStyleSheet* pStyleSheet = static_cast<ScStyleSheet*>(
                    pStylePool->Find(
                        ScGlobal::GetRscString(STR_STYLENAME_STANDARD), SFX_STYLE_FAMILY_PARA));

                if (pStyleSheet)
                    rPat.SetStyleSheet(pStyleSheet, false);
            }

        }
    }

    if (nForceScNumFmt != NUMBERFORMAT_ENTRY_NOT_FOUND)
    {
        ScPatternAttr aNumPat(rDoc.GetPool());
        GetNumFmtBuffer().FillScFmtToItemSet(aNumPat.GetItemSet(), nForceScNumFmt);
        rPat.GetItemSet().Put(aNumPat.GetItemSet());
    }

    // Make sure we skip unnamed styles.
    if (rPat.GetStyleName())
    {
        // Check for a gap between the last entry and this one.
        bool bHasGap = false;
        if (rAttrs.empty() && nRow1 > 0)
            // First attribute range doesn't start at row 0.
            bHasGap = true;

        if (!rAttrs.empty() && rAttrs.back().nRow + 1 < nRow1)
            bHasGap = true;

        if (bHasGap)
        {
            // Fill this gap with the default pattern.
            ScAttrEntry aEntry;
            aEntry.nRow = nRow1 - 1;
            aEntry.pPattern = rDoc.GetDefPattern();
            rAttrs.push_back(aEntry);
        }

        ScAttrEntry aEntry;
        aEntry.nRow = nRow2;
        aEntry.pPattern = static_cast<const ScPatternAttr*>(&rDoc.GetPool()->Put(rPat));
        rAttrs.push_back(aEntry);
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

// ----------------------------------------------------------------------------

XclImpStyle::XclImpStyle( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot ),
    mnXfId( EXC_XF_NOTFOUND ),
    mnBuiltinId( EXC_STYLE_USERDEF ),
    mnLevel( EXC_STYLE_NOLEVEL ),
    mbBuiltin( false ),
    mbCustom( false ),
    mbHidden( false ),
    mpStyleSheet( 0 )
{
}

void XclImpStyle::ReadStyle( XclImpStream& rStrm )
{
    OSL_ENSURE_BIFF( GetBiff() >= EXC_BIFF3 );

    sal_uInt16 nXFIndex;
    rStrm >> nXFIndex;
    mnXfId = nXFIndex & EXC_STYLE_XFMASK;
    mbBuiltin = ::get_flag( nXFIndex, EXC_STYLE_BUILTIN );

    if( mbBuiltin )
    {
        rStrm >> mnBuiltinId >> mnLevel;
    }
    else
    {
        maName = (GetBiff() <= EXC_BIFF5) ? rStrm.ReadByteString( false ) : rStrm.ReadUniString();
        // #i103281# check if this is a new built-in style introduced in XL2007
        if( (GetBiff() == EXC_BIFF8) && (rStrm.GetNextRecId() == EXC_ID_STYLEEXT) && rStrm.StartNextRecord() )
        {
            sal_uInt8 nExtFlags;
            rStrm.Ignore( 12 );
            rStrm >> nExtFlags;
            mbBuiltin = ::get_flag( nExtFlags, EXC_STYLEEXT_BUILTIN );
            mbCustom = ::get_flag( nExtFlags, EXC_STYLEEXT_CUSTOM );
            mbHidden = ::get_flag( nExtFlags, EXC_STYLEEXT_HIDDEN );
            if( mbBuiltin )
            {
                rStrm.Ignore( 1 );  // category
                rStrm >> mnBuiltinId >> mnLevel;
            }
        }
    }
}

ScStyleSheet* XclImpStyle::CreateStyleSheet()
{
    // #i1624# #i1768# ignore unnamed user styles
    if( !mpStyleSheet && (maFinalName.Len() > 0) )
    {
        bool bCreatePattern = false;
        XclImpXF* pXF = GetXFBuffer().GetXF( mnXfId );

        bool bDefStyle = mbBuiltin && (mnBuiltinId == EXC_STYLE_NORMAL);
        if( bDefStyle )
        {
            // set all flags to true to get all items in XclImpXF::CreatePattern()
            if( pXF ) pXF->SetAllUsedFlags( true );
            // use existing "Default" style sheet
            mpStyleSheet = static_cast< ScStyleSheet* >( GetStyleSheetPool().Find(
                ScGlobal::GetRscString( STR_STYLENAME_STANDARD ), SFX_STYLE_FAMILY_PARA ) );
            OSL_ENSURE( mpStyleSheet, "XclImpStyle::CreateStyleSheet - Default style not found" );
            bCreatePattern = true;
        }
        else
        {
            /*  #i103281# do not create another style sheet of the same name,
                if it exists already. This is needed to prevent that styles
                pasted from clipboard get duplicated over and over. */
            mpStyleSheet = static_cast< ScStyleSheet* >( GetStyleSheetPool().Find( maFinalName, SFX_STYLE_FAMILY_PARA ) );
            if( !mpStyleSheet )
            {
                mpStyleSheet = &static_cast< ScStyleSheet& >( GetStyleSheetPool().Make( maFinalName, SFX_STYLE_FAMILY_PARA, SFXSTYLEBIT_USERDEF ) );
                bCreatePattern = true;
            }
        }

        // bDefStyle==true omits default pool items in CreatePattern()
        if( bCreatePattern && mpStyleSheet && pXF )
            mpStyleSheet->GetItemSet().Put( pXF->CreatePattern( bDefStyle ).GetItemSet() );
    }
    return mpStyleSheet;
}

void XclImpStyle::CreateUserStyle( const String& rFinalName )
{
    maFinalName = rFinalName;
    if( !IsBuiltin() || mbCustom )
        CreateStyleSheet();
}

// ----------------------------------------------------------------------------

XclImpXFBuffer::XclImpXFBuffer( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot )
{
}

void XclImpXFBuffer::Initialize()
{
    maXFList.clear();
    maBuiltinStyles.clear();
    maUserStyles.clear();
    maStylesByXf.clear();
}

void XclImpXFBuffer::ReadXF( XclImpStream& rStrm )
{
    XclImpXF* pXF = new XclImpXF( GetRoot() );
    pXF->ReadXF( rStrm );
    maXFList.push_back( pXF );
}

void XclImpXFBuffer::ReadStyle( XclImpStream& rStrm )
{
    XclImpStyle* pStyle = new XclImpStyle( GetRoot() );
    pStyle->ReadStyle( rStrm );
    (pStyle->IsBuiltin() ? maBuiltinStyles : maUserStyles).push_back( pStyle );
    OSL_ENSURE( maStylesByXf.count( pStyle->GetXfId() ) == 0, "XclImpXFBuffer::ReadStyle - multiple styles with equal XF identifier" );
    maStylesByXf[ pStyle->GetXfId() ] = pStyle;
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

namespace {

/** Functor for case-insensitive string comparison, usable in maps etc. */
struct IgnoreCaseCompare
{
    inline bool operator()( const String& rName1, const String& rName2 ) const
        { return rName1.CompareIgnoreCaseToAscii( rName2 ) == COMPARE_LESS; }
};

} // namespace

void XclImpXFBuffer::CreateUserStyles()
{
    // calculate final names of all styles
    typedef ::std::map< String, XclImpStyle*, IgnoreCaseCompare > CellStyleNameMap;
    typedef ::std::vector< XclImpStyle* > XclImpStyleVector;

    CellStyleNameMap aCellStyles;
    XclImpStyleVector aConflictNameStyles;

    /*  First, reserve style names that are built-in in Calc. This causes that
        imported cell styles get different unused names and thus do not try to
        overwrite these built-in styles. For BIFF4 workbooks (which contain a
        separate list of cell styles per sheet), reserve all existing styles if
        current sheet is not the first sheet (this styles buffer will be
        initialized again for every new sheet). This will create unique names
        for styles in different sheets with the same name. Assuming that the
        BIFF4W import filter is never used to import from clipboard... */
    bool bReserveAll = (GetBiff() == EXC_BIFF4) && (GetCurrScTab() > 0);
    SfxStyleSheetIterator aStyleIter( GetDoc().GetStyleSheetPool(), SFX_STYLE_FAMILY_PARA );
    String aStandardName = ScGlobal::GetRscString( STR_STYLENAME_STANDARD );
    for( SfxStyleSheetBase* pStyleSheet = aStyleIter.First(); pStyleSheet; pStyleSheet = aStyleIter.Next() )
        if( (pStyleSheet->GetName() != aStandardName) && (bReserveAll || !pStyleSheet->IsUserDefined()) )
            if( aCellStyles.count( pStyleSheet->GetName() ) == 0 )
                aCellStyles[ pStyleSheet->GetName() ] = 0;

    /*  Calculate names of built-in styles. Store styles with reserved names
        in the aConflictNameStyles list. */
    for( XclImpStyleList::iterator itStyle = maBuiltinStyles.begin(); itStyle != maBuiltinStyles.end(); ++itStyle )
    {
        String aStyleName = XclTools::GetBuiltInStyleName( itStyle->GetBuiltinId(), itStyle->GetName(), itStyle->GetLevel() );
        OSL_ENSURE( bReserveAll || (aCellStyles.count( aStyleName ) == 0),
            "XclImpXFBuffer::CreateUserStyles - multiple styles with equal built-in identifier" );
        if( aCellStyles.count( aStyleName ) > 0 )
            aConflictNameStyles.push_back( &(*itStyle) );
        else
            aCellStyles[ aStyleName ] = &(*itStyle);
    }

    /*  Calculate names of user defined styles. Store styles with reserved
        names in the aConflictNameStyles list. */
    for( XclImpStyleList::iterator itStyle = maUserStyles.begin(); itStyle != maUserStyles.end(); ++itStyle )
    {
        // #i1624# #i1768# ignore unnamed user styles
        if( itStyle->GetName().Len() > 0 )
        {
            if( aCellStyles.count( itStyle->GetName() ) > 0 )
                aConflictNameStyles.push_back( &(*itStyle) );
            else
                aCellStyles[ itStyle->GetName() ] = &(*itStyle);
        }
    }

    // find unused names for all styles with conflicting names
    for( XclImpStyleVector::iterator aIt = aConflictNameStyles.begin(), aEnd = aConflictNameStyles.end(); aIt != aEnd; ++aIt )
    {
        XclImpStyle* pStyle = *aIt;
        String aUnusedName;
        sal_Int32 nIndex = 0;
        do
        {
            aUnusedName.Assign( pStyle->GetName() ).Append( ' ' ).Append( String::CreateFromInt32( ++nIndex ) );
        }
        while( aCellStyles.count( aUnusedName ) > 0 );
        aCellStyles[ aUnusedName ] = pStyle;
    }

    // set final names and create user-defined and modified built-in cell styles
    for( CellStyleNameMap::iterator aIt = aCellStyles.begin(), aEnd = aCellStyles.end(); aIt != aEnd; ++aIt )
        if( aIt->second )
            aIt->second->CreateUserStyle( aIt->first );
}

ScStyleSheet* XclImpXFBuffer::CreateStyleSheet( sal_uInt16 nXFIndex )
{
    XclImpStyleMap::iterator aIt = maStylesByXf.find( nXFIndex );
    return (aIt == maStylesByXf.end()) ? 0 : aIt->second->CreateStyleSheet();
}

// Buffer for XF indexes in cells =============================================

IMPL_FIXEDMEMPOOL_NEWDEL( XclImpXFRange )

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
    OSL_ENSURE( mnScRow2 < rNextRange.mnScRow1, "XclImpXFRange::Expand - rows out of order" );
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
    OSL_ENSURE( maIndexList.empty(), "XclImpXFRangeColumn::SetDefaultXF - Setting Default Column XF is not empty" );

    // insert a complete row range with one insert.
    maIndexList.push_back( new XclImpXFRange( 0, MAXROW, rXFIndex ) );
}

// ----------------------------------------------------------------------------

void XclImpXFRangeColumn::SetXF( SCROW nScRow, const XclImpXFIndex& rXFIndex )
{
    XclImpXFRange* pPrevRange;
    XclImpXFRange* pNextRange;
    sal_uLong nNextIndex;

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
            sal_uLong nIndex = nNextIndex - 1;
            XclImpXFRange* pThisRange = pPrevRange;
            pPrevRange = (nIndex > 0 && nIndex <= maIndexList.size()) ? &(maIndexList[ nIndex - 1 ]) : 0;

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
                    Insert( new XclImpXFRange( nScRow, rXFIndex ), nIndex );
            }
            else if( nLastScRow == nScRow )         // replace last XF
            {
                --(pThisRange->mnScRow2);
                if( !pNextRange || !pNextRange->Expand( nScRow, rXFIndex ) )
                    Insert( new XclImpXFRange( nScRow, rXFIndex ), nNextIndex );
            }
            else                                    // insert in the middle of the range
            {
                pThisRange->mnScRow1 = nScRow + 1;
                // List::Insert() moves entries towards end of list, so insert twice at nIndex
                Insert( new XclImpXFRange( nScRow, rXFIndex ), nIndex );
                Insert( new XclImpXFRange( nFirstScRow, nScRow - 1, pThisRange->maXFIndex ), nIndex );
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
    Insert( new XclImpXFRange( nScRow, rXFIndex ), nNextIndex );
}

void XclImpXFRangeColumn::Insert(XclImpXFRange* pXFRange, sal_uLong nIndex)
{
    maIndexList.insert( maIndexList.begin() + nIndex, pXFRange );
}

void XclImpXFRangeColumn::Find(
        XclImpXFRange*& rpPrevRange, XclImpXFRange*& rpNextRange,
        sal_uLong& rnNextIndex, SCROW nScRow )
{

    // test whether list is empty
    if( maIndexList.empty() )
    {
        rpPrevRange = rpNextRange = 0;
        rnNextIndex = 0;
        return;
    }

    rpPrevRange = &maIndexList.front();
    rpNextRange = &maIndexList.back();

    // test whether row is at end of list (contained in or behind last range)
    // rpPrevRange will contain a possible existing row
    if( rpNextRange->mnScRow1 <= nScRow )
    {
        rpPrevRange = rpNextRange;
        rpNextRange = 0;
        rnNextIndex = maIndexList.size();
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
    sal_uLong nPrevIndex = 0;
    sal_uLong nMidIndex;
    rnNextIndex = maIndexList.size() - 1;
    XclImpXFRange* pMidRange;
    while( ((rnNextIndex - nPrevIndex) > 1) && (rpPrevRange->mnScRow2 < nScRow) )
    {
        nMidIndex = (nPrevIndex + rnNextIndex) / 2;
        pMidRange = &maIndexList[nMidIndex];
        OSL_ENSURE( pMidRange, "XclImpXFRangeColumn::Find - missing XF index range" );
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
        rpNextRange = &maIndexList[rnNextIndex];
    }
}

void XclImpXFRangeColumn::TryConcatPrev( sal_uLong nIndex )
{
    if( !nIndex || nIndex >= maIndexList.size() )
        return;

    XclImpXFRange& prevRange = maIndexList[ nIndex - 1 ];
    XclImpXFRange& nextRange = maIndexList[ nIndex ];

    if( prevRange.Expand( nextRange ) )
        maIndexList.erase( maIndexList.begin() + nIndex );
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
    // remember all Boolean cells, they will get 'Standard' number format
    maColumns[ nIndex ]->SetXF( nScRow, XclImpXFIndex( nXFIndex, eMode == xlXFModeBoolCell ) );

    // set "center across selection" and "fill" attribute for all following empty cells
    // ignore it on row default XFs
    if( eMode != xlXFModeRow )
    {
        const XclImpXF* pXF = GetXFBuffer().GetXF( nXFIndex );
        if( pXF && ((pXF->GetHorAlign() == EXC_XF_HOR_CENTER_AS) || (pXF->GetHorAlign() == EXC_XF_HOR_FILL)) )
        {
            // expand last merged range if this attribute is set repeatedly
            ScRange* pRange = maMergeList.empty() ? NULL : maMergeList.back();
            if (pRange && (pRange->aEnd.Row() == nScRow) && (pRange->aEnd.Col() + 1 == nScCol) && (eMode == xlXFModeBlank))
                pRange->aEnd.IncCol();
            else if( eMode != xlXFModeBlank )   // do not merge empty cells
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
    OSL_ENSURE( !maColumns[ nIndex ], "XclImpXFRangeBuffer::SetColumnDefXF - default column of XFs already has values" );
    maColumns[ nIndex ].reset( new XclImpXFRangeColumn );
    maColumns[ nIndex ]->SetDefaultXF( XclImpXFIndex( nXFIndex ) );
}

void XclImpXFRangeBuffer::SetBorderLine( const ScRange& rRange, SCTAB nScTab, sal_uInt16 nLine )
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
        if( aVIt->get() )
        {
            XclImpXFRangeColumn& rColumn = **aVIt;
            SCCOL nScCol = static_cast< SCCOL >( aVIt - aVBeg );
            list<ScAttrEntry> aAttrs;

            for (XclImpXFRangeColumn::IndexList::iterator itr = rColumn.begin(), itrEnd = rColumn.end();
                 itr != itrEnd; ++itr)
            {
                XclImpXFRange& rStyle = *itr;
                const XclImpXFIndex& rXFIndex = rStyle.maXFIndex;
                XclImpXF* pXF = rXFBuffer.GetXF( rXFIndex.GetXFIndex() );
                if (!pXF)
                    continue;

                sal_uInt32 nForceScNumFmt = rXFIndex.IsBoolCell() ?
                    GetNumFmtBuffer().GetStdScNumFmt() : NUMBERFORMAT_ENTRY_NOT_FOUND;

                pXF->ApplyPatternToAttrList(aAttrs, rStyle.mnScRow1, rStyle.mnScRow2, nForceScNumFmt);
            }

            if (aAttrs.empty() || aAttrs.back().nRow != MAXROW)
            {
                ScAttrEntry aEntry;
                aEntry.nRow = MAXROW;
                aEntry.pPattern = rDoc.GetDefPattern();
                aAttrs.push_back(aEntry);
            }

            size_t nAttrSize = aAttrs.size();
            ScAttrEntry* pData = new ScAttrEntry[nAttrSize];
            list<ScAttrEntry>::const_iterator itr = aAttrs.begin(), itrEnd = aAttrs.end();
            for (size_t i = 0; itr != itrEnd; ++itr, ++i)
                pData[i] = *itr;

            rDoc.SetAttrEntries(nScCol, nScTab, pData, static_cast<SCSIZE>(nAttrSize));
        }
    }

    // insert hyperlink cells
    for( XclImpHyperlinkList::const_iterator aLIt = maHyperlinks.begin(), aLEnd = maHyperlinks.end(); aLIt != aLEnd; ++aLIt )
        XclImpHyperlink::InsertUrl( GetRoot(), aLIt->first, aLIt->second );

    // apply cell merging
    for ( size_t i = 0, nRange = maMergeList.size(); i < nRange; ++i )
    {
        const ScRange* pRange = maMergeList[ i ];
        const ScAddress& rStart = pRange->aStart;
        const ScAddress& rEnd = pRange->aEnd;
        bool bMultiCol = rStart.Col() != rEnd.Col();
        bool bMultiRow = rStart.Row() != rEnd.Row();
        // set correct right border
        if( bMultiCol )
            SetBorderLine( *pRange, nScTab, BOX_LINE_RIGHT );
        // set correct lower border
        if( bMultiRow )
            SetBorderLine( *pRange, nScTab, BOX_LINE_BOTTOM );
        // do merge
        if( bMultiCol || bMultiRow )
            rDoc.DoMerge( nScTab, rStart.Col(), rStart.Row(), rEnd.Col(), rEnd.Row() );
        // #i93609# merged range in a single row: test if manual row height is needed
        if( !bMultiRow )
        {
            bool bTextWrap = static_cast< const SfxBoolItem* >( rDoc.GetAttr( rStart.Col(), rStart.Row(), rStart.Tab(), ATTR_LINEBREAK ) )->GetValue();
            if( !bTextWrap && (rDoc.GetCellType( rStart ) == CELLTYPE_EDIT) )
                if( const EditTextObject* pEditObj = static_cast< const ScEditCell* >( rDoc.GetCell( rStart ) )->GetData() )
                    bTextWrap = pEditObj->GetParagraphCount() > 1;
            if( bTextWrap )
                GetOldRoot().pColRowBuff->SetManualRowHeight( rStart.Row() );
        }
    }
}

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
