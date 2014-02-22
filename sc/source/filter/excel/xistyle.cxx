/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

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
#include <editeng/crossedoutitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/escapementitem.hxx>
#include <svx/algitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/lineitem.hxx>
#include <svx/rotmodit.hxx>
#include <editeng/colritem.hxx>
#include <editeng/brushitem.hxx>
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
#include "formulacell.hxx"
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

    
    virtual ::sal_Int32 SAL_CALL getCount() throw (uno::RuntimeException)
    {
         return  maColorData.size();
    }

    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
    {
        
        return uno::makeAny( sal_Int32( maColorData[ Index ] ) );
    }

    
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
    if( !maData.maStyle.isEmpty() )
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
        maData.maStyle = "";
    }
    GuessScriptType();
    SetAllUsedFlags( true );
}

rtl_TextEncoding XclImpFont::GetFontEncoding() const
{
    
    
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
    
    bool bEE = eType != EXC_FONTITEM_CELL;




#define PUTITEM( item, sc_which, ee_which ) \
    ScfTools::PutItem( rItemSet, item, (bEE ? (ee_which) : (sc_which)), bSkipPoolDefs )


    
    bool bDefNoteFont = (eType == EXC_FONTITEM_NOTE) && (maData.maName.equalsIgnoreAsciiCase( "Tahoma" ));
    if( mbFontNameUsed && !bDefNoteFont )
    {
        rtl_TextEncoding eFontEnc = maData.GetFontEncoding();
        rtl_TextEncoding eTempTextEnc = (bEE && (eFontEnc == GetTextEncoding())) ?
            ScfTools::GetSystemTextEncoding() : eFontEnc;

        
        FontPitch ePitch = PITCH_DONTKNOW;
        FontFamily eFtFamily = maData.GetScFamily( GetTextEncoding() );
        switch( eFtFamily ) 
        {
            case FAMILY_ROMAN:              ePitch = PITCH_VARIABLE;        break;
            case FAMILY_SWISS:              ePitch = PITCH_VARIABLE;        break;
            case FAMILY_MODERN:             ePitch = PITCH_FIXED;           break;
            default:                        break;
         }
        SvxFontItem aFontItem( eFtFamily , maData.maName, EMPTY_OUSTRING, ePitch, eTempTextEnc, ATTR_FONT );

        
        if( mbHasWstrn )
            PUTITEM( aFontItem, ATTR_FONT,      EE_CHAR_FONTINFO );
        if( mbHasAsian )
            PUTITEM( aFontItem, ATTR_CJK_FONT,  EE_CHAR_FONTINFO_CJK );
        if( mbHasCmplx )
            PUTITEM( aFontItem, ATTR_CTL_FONT,  EE_CHAR_FONTINFO_CTL );
    }


    if( mbHeightUsed )
    {
        sal_Int32 nHeight = maData.mnHeight;
        if( bEE && (eType != EXC_FONTITEM_HF) )     
            nHeight = (nHeight * 127 + 36) / EXC_POINTS_PER_INCH;   

        SvxFontHeightItem aHeightItem( nHeight, 100, ATTR_FONT_HEIGHT );
        PUTITEM( aHeightItem,   ATTR_FONT_HEIGHT,       EE_CHAR_FONTHEIGHT );
        PUTITEM( aHeightItem,   ATTR_CJK_FONT_HEIGHT,   EE_CHAR_FONTHEIGHT_CJK );
        PUTITEM( aHeightItem,   ATTR_CTL_FONT_HEIGHT,   EE_CHAR_FONTHEIGHT_CTL );
    }


    if( mbColorUsed )
        PUTITEM( SvxColorItem( maData.maColor, ATTR_FONT_COLOR  ), ATTR_FONT_COLOR, EE_CHAR_COLOR );


    if( mbWeightUsed )
    {
        SvxWeightItem aWeightItem( maData.GetScWeight(), ATTR_FONT_WEIGHT );
        PUTITEM( aWeightItem,   ATTR_FONT_WEIGHT,       EE_CHAR_WEIGHT );
        PUTITEM( aWeightItem,   ATTR_CJK_FONT_WEIGHT,   EE_CHAR_WEIGHT_CJK );
        PUTITEM( aWeightItem,   ATTR_CTL_FONT_WEIGHT,   EE_CHAR_WEIGHT_CTL );
    }


    if( mbUnderlUsed )
    {
        SvxUnderlineItem aUnderlItem( maData.GetScUnderline(), ATTR_FONT_UNDERLINE );
        PUTITEM( aUnderlItem,   ATTR_FONT_UNDERLINE,    EE_CHAR_UNDERLINE );
    }


    if( mbItalicUsed )
    {
        SvxPostureItem aPostItem( maData.GetScPosture(), ATTR_FONT_POSTURE );
        PUTITEM( aPostItem, ATTR_FONT_POSTURE,      EE_CHAR_ITALIC );
        PUTITEM( aPostItem, ATTR_CJK_FONT_POSTURE,  EE_CHAR_ITALIC_CJK );
        PUTITEM( aPostItem, ATTR_CTL_FONT_POSTURE,  EE_CHAR_ITALIC_CTL );
    }


    if( mbStrikeUsed )
        PUTITEM( SvxCrossedOutItem( maData.GetScStrikeout(), ATTR_FONT_CROSSEDOUT ), ATTR_FONT_CROSSEDOUT, EE_CHAR_STRIKEOUT );
    if( mbOutlineUsed )
        PUTITEM( SvxContourItem( maData.mbOutline, ATTR_FONT_CONTOUR ), ATTR_FONT_CONTOUR, EE_CHAR_OUTLINE );
    if( mbShadowUsed )
        PUTITEM( SvxShadowedItem( maData.mbShadow, ATTR_FONT_SHADOWED ), ATTR_FONT_SHADOWED, EE_CHAR_SHADOW );


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

    
    if( OutputDevice* pPrinter = GetPrinter() )
    {
        Font aFont( maData.maName, Size( 0, 10 ) );
        FontCharMap aCharMap;

        pPrinter->SetFont( aFont );
        if( pPrinter->GetFontCharMap( aCharMap ) )
        {
            
            mbHasAsian =
                aCharMap.HasChar( 0x3041 ) ||   
                aCharMap.HasChar( 0x30A1 ) ||   
                aCharMap.HasChar( 0x3111 ) ||   
                aCharMap.HasChar( 0x3131 ) ||   
                aCharMap.HasChar( 0x3301 ) ||   
                aCharMap.HasChar( 0x3401 ) ||   
                aCharMap.HasChar( 0x4E01 ) ||   
                aCharMap.HasChar( 0x7E01 ) ||   
                aCharMap.HasChar( 0xA001 ) ||   
                aCharMap.HasChar( 0xAC01 ) ||   
                aCharMap.HasChar( 0xCC01 ) ||   
                aCharMap.HasChar( 0xF901 ) ||   
                aCharMap.HasChar( 0xFF71 );     
            
            mbHasCmplx =
                aCharMap.HasChar( 0x05D1 ) ||   
                aCharMap.HasChar( 0x0631 ) ||   
                aCharMap.HasChar( 0x0721 ) ||   
                aCharMap.HasChar( 0x0911 ) ||   
                aCharMap.HasChar( 0x0E01 ) ||   
                aCharMap.HasChar( 0xFB21 ) ||   
                aCharMap.HasChar( 0xFB51 ) ||   
                aCharMap.HasChar( 0xFE71 );     
            
            mbHasWstrn = (!mbHasAsian && !mbHasCmplx) || aCharMap.HasChar( 'A' );
        }
    }
}



XclImpFontBuffer::XclImpFontBuffer( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot ),
    maFont4( rRoot ),
    maCtrlFont( rRoot )
{
    Initialize();

    
    XclFontData aCtrlFontData;
    switch( GetBiff() )
    {
        case EXC_BIFF2:
        case EXC_BIFF3:
        case EXC_BIFF4:
        case EXC_BIFF5:
            aCtrlFontData.maName = "Helv";
            aCtrlFontData.mnHeight = 160;
            aCtrlFontData.mnWeight = EXC_FONTWGHT_BOLD;
        break;
        case EXC_BIFF8:
            aCtrlFontData.maName = "Tahoma";
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

    
    XclFontData aAppFontData;
    aAppFontData.maName = "Arial";
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
        
        return nFontIndex >= maFontList.size() ? NULL : &maFontList[nFontIndex];
    }

    
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
    
    SetCharWidth( maAppFont );

    
    XclFontData aFont4Data( maAppFont );
    aFont4Data.mnWeight = EXC_FONTWGHT_BOLD;
    maFont4.SetFontData( aFont4Data, bHasCharSet );
}



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
    InitializeImport();     
}

void XclImpNumFmtBuffer::ReadFormat( XclImpStream& rStrm )
{
    OUString aFormat;
    switch( GetBiff() )
    {
        case EXC_BIFF2:
        case EXC_BIFF3:
            aFormat = rStrm.ReadByteString( false );
        break;

        case EXC_BIFF4:
            rStrm.Ignore( 2 );  
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

        
        sal_Int32 nCheckPos;
        short nType = NUMBERFORMAT_DEFINED;
        sal_uInt32 nKey;
        if( !rNumFmt.maFormat.isEmpty() )
        {
            OUString aFormat( rNumFmt.maFormat );
            rFormatter.PutandConvertEntry( aFormat, nCheckPos,
                                           nType, nKey, LANGUAGE_ENGLISH_US, rNumFmt.meLanguage );
        }
        else
            nKey = rFormatter.GetFormatIndex( rNumFmt.meOffset, rNumFmt.meLanguage );

        
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




void XclImpCellAlign::FillFromXF2( sal_uInt8 nFlags )
{
    mnHorAlign = ::extract_value< sal_uInt8 >( nFlags, 0, 3 );
}

void XclImpCellAlign::FillFromXF3( sal_uInt16 nAlign )
{
    mnHorAlign = ::extract_value< sal_uInt8 >( nAlign, 0, 3 );
    mbLineBreak = ::get_flag( nAlign, EXC_XF_LINEBREAK );   
}

void XclImpCellAlign::FillFromXF4( sal_uInt16 nAlign )
{
    FillFromXF3( nAlign );
    mnVerAlign = ::extract_value< sal_uInt8 >( nAlign, 4, 2 );  
    mnOrient = ::extract_value< sal_uInt8 >( nAlign, 6, 2 );    
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
    mnRotation = ::extract_value< sal_uInt8 >( nAlign, 8, 8 );      
    mnIndent = ::extract_value< sal_uInt8 >( nMiscAttrib, 0, 4 );   
    mbShrink = ::get_flag( nMiscAttrib, EXC_XF8_SHRINK );           
    mnTextDir = ::extract_value< sal_uInt8 >( nMiscAttrib, 6, 2 );  
}

void XclImpCellAlign::FillToItemSet( SfxItemSet& rItemSet, const XclImpFont* pFont, bool bSkipPoolDefs ) const
{
    
    ScfTools::PutItem( rItemSet, SvxHorJustifyItem( GetScHorAlign(), ATTR_HOR_JUSTIFY ), bSkipPoolDefs );
    ScfTools::PutItem( rItemSet, SvxJustifyMethodItem( GetScHorJustifyMethod(), ATTR_HOR_JUSTIFY_METHOD ), bSkipPoolDefs );

    
    bool bLineBreak = mbLineBreak || (mnVerAlign == EXC_XF_VER_JUSTIFY) || (mnVerAlign == EXC_XF_VER_DISTRIB);
    ScfTools::PutItem( rItemSet, SfxBoolItem( ATTR_LINEBREAK, bLineBreak ), bSkipPoolDefs );

    
    ScfTools::PutItem( rItemSet, SvxVerJustifyItem( GetScVerAlign(), ATTR_VER_JUSTIFY ), bSkipPoolDefs );
    ScfTools::PutItem( rItemSet, SvxJustifyMethodItem( GetScVerJustifyMethod(), ATTR_VER_JUSTIFY_METHOD ), bSkipPoolDefs );

    
    sal_uInt16 nScIndent = mnIndent * 200; 
    ScfTools::PutItem( rItemSet, SfxUInt16Item( ATTR_INDENT, nScIndent ), bSkipPoolDefs );

    
    ScfTools::PutItem( rItemSet, SfxBoolItem( ATTR_SHRINKTOFIT, mbShrink ), bSkipPoolDefs );

    
    sal_uInt8 nXclRot = (mnOrient == EXC_ORIENT_NONE) ? mnRotation : XclTools::GetXclRotFromOrient( mnOrient );
    bool bStacked = (nXclRot == EXC_ROT_STACKED);
    ScfTools::PutItem( rItemSet, SfxBoolItem( ATTR_STACKED, bStacked ), bSkipPoolDefs );
    
    sal_Int32 nAngle = XclTools::GetScRotation( nXclRot, 0 );
    ScfTools::PutItem( rItemSet, SfxInt32Item( ATTR_ROTATE_VALUE, nAngle ), bSkipPoolDefs );
    
    bool bAsianVert = bStacked && pFont && pFont->HasAsianChars();
    ScfTools::PutItem( rItemSet, SfxBoolItem( ATTR_VERTICAL_ASIAN, bAsianVert ), bSkipPoolDefs );

    
    ScfTools::PutItem( rItemSet, SvxFrameDirectionItem( GetScFrameDir(), ATTR_WRITINGDIR ), bSkipPoolDefs );
}



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
        
        {   0,                  table::BorderLineStyle::SOLID },  
        {   EXC_BORDER_THIN,    table::BorderLineStyle::SOLID },  
        {   EXC_BORDER_MEDIUM,  table::BorderLineStyle::SOLID },  
        {   EXC_BORDER_THIN,    table::BorderLineStyle::FINE_DASHED }, 
        {   EXC_BORDER_THIN,    table::BorderLineStyle::DOTTED }, 
        {   EXC_BORDER_THICK,   table::BorderLineStyle::SOLID },  
        {   EXC_BORDER_THIN,    table::BorderLineStyle::DOUBLE }, 
        {   EXC_BORDER_HAIR,    table::BorderLineStyle::SOLID },  
        {   EXC_BORDER_MEDIUM,  table::BorderLineStyle::DASHED }, 
        {   EXC_BORDER_THIN,    table::BorderLineStyle::SOLID },  
        {   EXC_BORDER_MEDIUM,  table::BorderLineStyle::SOLID },  
        {   EXC_BORDER_THIN,    table::BorderLineStyle::SOLID },  
        {   EXC_BORDER_MEDIUM,  table::BorderLineStyle::SOLID },  
        {   EXC_BORDER_MEDIUM,  table::BorderLineStyle::SOLID }   
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

} 

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
    if( mbPattUsed )    
    {
        SvxBrushItem aBrushItem( ATTR_BACKGROUND );

        
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




XclImpXF::XclImpXF( const XclImpRoot& rRoot ) :
    XclXFBase( true ),      
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

    
    SetAllUsedFlags( true );

    
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

    
    mbCellXF = !::get_flag( nTypeProt, EXC_XF_STYLE );          
    mnParent = ::extract_value< sal_uInt16 >( nAlign, 4, 12 );  
    SetUsedFlags( ::extract_value< sal_uInt8 >( nTypeProt, 10, 6 ) );

    
    maProtection.FillFromXF3( nTypeProt );
    mnXclFont = nReadFont;
    mnXclNumFmt = nReadNumFmt;
    maAlignment.FillFromXF3( nAlign );
    maBorder.FillFromXF3( nBorder );
    maArea.FillFromXF3( nArea );                        
}

void XclImpXF::ReadXF4( XclImpStream& rStrm )
{
    sal_uInt32 nBorder;
    sal_uInt16 nTypeProt, nAlign, nArea;
    sal_uInt8 nReadFont, nReadNumFmt;
    rStrm >> nReadFont >> nReadNumFmt >> nTypeProt >> nAlign >> nArea >> nBorder;

    
    mbCellXF = !::get_flag( nTypeProt, EXC_XF_STYLE );
    mnParent = ::extract_value< sal_uInt16 >( nTypeProt, 4, 12 );
    SetUsedFlags( ::extract_value< sal_uInt8 >( nAlign, 10, 6 ) );

    
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

    
    mbCellXF = !::get_flag( nTypeProt, EXC_XF_STYLE );
    mnParent = ::extract_value< sal_uInt16 >( nTypeProt, 4, 12 );
    SetUsedFlags( ::extract_value< sal_uInt8 >( nAlign, 10, 6 ) );

    
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

    
    mbCellXF = !::get_flag( nTypeProt, EXC_XF_STYLE );
    mnParent = ::extract_value< sal_uInt16 >( nTypeProt, 4, 12 );
    SetUsedFlags( ::extract_value< sal_uInt8 >( nMiscAttrib, 10, 6 ) );

    
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

    
    mpPattern.reset( new ScPatternAttr( GetDoc().GetPool() ) );
    SfxItemSet& rItemSet = mpPattern->GetItemSet();
    XclImpXF* pParentXF = IsCellXF() ? GetXFBuffer().GetXF( mnParent ) : 0;

    
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

    
    if( mbProtUsed )
        maProtection.FillToItemSet( rItemSet, bSkipPoolDefs );

    
    if( mbFontUsed )
        GetFontBuffer().FillToItemSet( rItemSet, EXC_FONTITEM_CELL, mnXclFont, bSkipPoolDefs );

    
    if( mbFmtUsed )
    {
        GetNumFmtBuffer().FillToItemSet( rItemSet, mnXclNumFmt, bSkipPoolDefs );
        
        GetTracer().TraceDates( mnXclNumFmt );
    }

    
    if( mbAlignUsed )
        maAlignment.FillToItemSet( rItemSet, GetFontBuffer().GetFont( mnXclFont ), bSkipPoolDefs );

    
    if( mbBorderUsed )
    {
        maBorder.FillToItemSet( rItemSet, GetPalette(), bSkipPoolDefs );
        GetTracer().TraceBorderLineStyle(maBorder.mnLeftLine > EXC_LINE_HAIR ||
            maBorder.mnRightLine > EXC_LINE_HAIR || maBorder.mnTopLine > EXC_LINE_HAIR ||
            maBorder.mnBottomLine > EXC_LINE_HAIR );
    }

    
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

    
    SvxMarginItem aItem(40, 40, 40, 40, ATTR_MARGIN);
    ScfTools::PutItem(rItemSet, aItem, bSkipPoolDefs);

    return *mpPattern;
}

void XclImpXF::ApplyPatternToAttrList(
    list<ScAttrEntry>& rAttrs, SCROW nRow1, SCROW nRow2, sal_uInt32 nForceScNumFmt)
{
    
    CreatePattern();
    ScPatternAttr& rPat = *mpPattern;

    
    ScDocument& rDoc = GetDoc();

    if (IsCellXF())
    {
        if (mpStyleSheet)
        {
            
            rPat.SetStyleSheet(mpStyleSheet, false);
        }
        else
        {
            
            
            
            
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

    
    if (rPat.GetStyleName())
    {
        
        bool bHasGap = false;
        if (rAttrs.empty() && nRow1 > 0)
            
            bHasGap = true;

        if (!rAttrs.empty() && rAttrs.back().nRow + 1 < nRow1)
            bHasGap = true;

        if (bHasGap)
        {
            
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

void XclImpXF::ApplyPattern(
        SCCOL nScCol1, SCROW nScRow1, SCCOL nScCol2, SCROW nScRow2,
        SCTAB nScTab, sal_uLong nForceScNumFmt )
{
    
    const ScPatternAttr& rPattern = CreatePattern();

    
    ScDocument& rDoc = GetDoc();
    if( IsCellXF() && mpStyleSheet )
        rDoc.ApplyStyleAreaTab( nScCol1, nScRow1, nScCol2, nScRow2, nScTab, *mpStyleSheet );
    if( HasUsedFlags() )
        rDoc.ApplyPatternAreaTab( nScCol1, nScRow1, nScCol2, nScRow2, nScTab, rPattern );

    
    if( nForceScNumFmt != NUMBERFORMAT_ENTRY_NOT_FOUND )
    {
        ScPatternAttr aPattern( GetDoc().GetPool() );
        GetNumFmtBuffer().FillScFmtToItemSet( aPattern.GetItemSet(), nForceScNumFmt );
        rDoc.ApplyPatternAreaTab( nScCol1, nScRow1, nScCol2, nScRow2, nScTab, aPattern );
    }
}

/*static*/ void XclImpXF::ApplyPatternForBiff2CellFormat( const XclImpRoot& rRoot,
        const ScAddress& rScPos, sal_uInt8 nFlags1, sal_uInt8 nFlags2, sal_uInt8 nFlags3 )
{
    /*  Create an XF object and let it do the work. We will have access to its
        private members here. */
    XclImpXF aXF( rRoot );

    
    aXF.SetAllUsedFlags( true );

    
    aXF.maProtection.FillFromXF2( nFlags1 );
    aXF.maAlignment.FillFromXF2( nFlags3 );
    aXF.maBorder.FillFromXF2( nFlags3 );
    aXF.maArea.FillFromXF2( nFlags3 );
    aXF.mnXclNumFmt = ::extract_value< sal_uInt16 >( nFlags2, 0, 6 );
    aXF.mnXclFont = ::extract_value< sal_uInt16 >( nFlags2, 6, 2 );

    
    aXF.ApplyPattern( rScPos.Col(), rScPos.Row(), rScPos.Col(), rScPos.Row(), rScPos.Tab() );
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
                rStrm.Ignore( 1 );  
                rStrm >> mnBuiltinId >> mnLevel;
            }
        }
    }
}

ScStyleSheet* XclImpStyle::CreateStyleSheet()
{
    
    if( !mpStyleSheet && (!maFinalName.isEmpty()) )
    {
        bool bCreatePattern = false;
        XclImpXF* pXF = GetXFBuffer().GetXF( mnXfId );

        bool bDefStyle = mbBuiltin && (mnBuiltinId == EXC_STYLE_NORMAL);
        if( bDefStyle )
        {
            
            if( pXF ) pXF->SetAllUsedFlags( true );
            
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

        
        if( bCreatePattern && mpStyleSheet && pXF )
            mpStyleSheet->GetItemSet().Put( pXF->CreatePattern( bDefStyle ).GetItemSet() );
    }
    return mpStyleSheet;
}

void XclImpStyle::CreateUserStyle( const OUString& rFinalName )
{
    maFinalName = rFinalName;
    if( !IsBuiltin() || mbCustom )
        CreateStyleSheet();
}



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
    inline bool operator()( const OUString& rName1, const OUString& rName2 ) const
        { return rName1.compareToIgnoreAsciiCase( rName2 ) < 0; }
};

} 

void XclImpXFBuffer::CreateUserStyles()
{
    
    typedef ::std::map< OUString, XclImpStyle*, IgnoreCaseCompare > CellStyleNameMap;
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
    OUString aStandardName = ScGlobal::GetRscString( STR_STYLENAME_STANDARD );
    for( SfxStyleSheetBase* pStyleSheet = aStyleIter.First(); pStyleSheet; pStyleSheet = aStyleIter.Next() )
        if( (pStyleSheet->GetName() != aStandardName) && (bReserveAll || !pStyleSheet->IsUserDefined()) )
            if( aCellStyles.count( pStyleSheet->GetName() ) == 0 )
                aCellStyles[ pStyleSheet->GetName() ] = 0;

    /*  Calculate names of built-in styles. Store styles with reserved names
        in the aConflictNameStyles list. */
    for( XclImpStyleList::iterator itStyle = maBuiltinStyles.begin(); itStyle != maBuiltinStyles.end(); ++itStyle )
    {
        OUString aStyleName = XclTools::GetBuiltInStyleName( itStyle->GetBuiltinId(), itStyle->GetName(), itStyle->GetLevel() );
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
        
        if( !itStyle->GetName().isEmpty() )
        {
            if( aCellStyles.count( itStyle->GetName() ) > 0 )
                aConflictNameStyles.push_back( &(*itStyle) );
            else
                aCellStyles[ itStyle->GetName() ] = &(*itStyle);
        }
    }

    
    for( XclImpStyleVector::iterator aIt = aConflictNameStyles.begin(), aEnd = aConflictNameStyles.end(); aIt != aEnd; ++aIt )
    {
        XclImpStyle* pStyle = *aIt;
        OUString aUnusedName;
        sal_Int32 nIndex = 0;
        do
        {
            aUnusedName = pStyle->GetName() + " " + OUString::number( ++nIndex );
        }
        while( aCellStyles.count( aUnusedName ) > 0 );
        aCellStyles[ aUnusedName ] = pStyle;
    }

    
    for( CellStyleNameMap::iterator aIt = aCellStyles.begin(), aEnd = aCellStyles.end(); aIt != aEnd; ++aIt )
        if( aIt->second )
            aIt->second->CreateUserStyle( aIt->first );
}

ScStyleSheet* XclImpXFBuffer::CreateStyleSheet( sal_uInt16 nXFIndex )
{
    XclImpStyleMap::iterator aIt = maStylesByXf.find( nXFIndex );
    return (aIt == maStylesByXf.end()) ? 0 : aIt->second->CreateStyleSheet();
}



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



void XclImpXFRangeColumn::SetDefaultXF( const XclImpXFIndex& rXFIndex )
{
    
    
    OSL_ENSURE( maIndexList.empty(), "XclImpXFRangeColumn::SetDefaultXF - Setting Default Column XF is not empty" );

    
    maIndexList.push_back( new XclImpXFRange( 0, MAXROW, rXFIndex ) );
}



void XclImpXFRangeColumn::SetXF( SCROW nScRow, const XclImpXFIndex& rXFIndex )
{
    XclImpXFRange* pPrevRange;
    XclImpXFRange* pNextRange;
    sal_uLong nNextIndex;

    Find( pPrevRange, pNextRange, nNextIndex, nScRow );

    
    
    if( pPrevRange )
    {
        if( pPrevRange->Contains( nScRow ) )        
        {
            if( rXFIndex == pPrevRange->maXFIndex )
                return;

            SCROW nFirstScRow = pPrevRange->mnScRow1;
            SCROW nLastScRow = pPrevRange->mnScRow2;
            sal_uLong nIndex = nNextIndex - 1;
            XclImpXFRange* pThisRange = pPrevRange;
            pPrevRange = (nIndex > 0 && nIndex <= maIndexList.size()) ? &(maIndexList[ nIndex - 1 ]) : 0;

            if( nFirstScRow == nLastScRow )         
            {
                pThisRange->maXFIndex = rXFIndex;
                TryConcatPrev( nNextIndex );        
                TryConcatPrev( nIndex );            
            }
            else if( nFirstScRow == nScRow )        
            {
                ++(pThisRange->mnScRow1);
                
                if( !pPrevRange || !pPrevRange->Expand( nScRow, rXFIndex ) )
                    Insert( new XclImpXFRange( nScRow, rXFIndex ), nIndex );
            }
            else if( nLastScRow == nScRow )         
            {
                --(pThisRange->mnScRow2);
                if( !pNextRange || !pNextRange->Expand( nScRow, rXFIndex ) )
                    Insert( new XclImpXFRange( nScRow, rXFIndex ), nNextIndex );
            }
            else                                    
            {
                pThisRange->mnScRow1 = nScRow + 1;
                
                Insert( new XclImpXFRange( nScRow, rXFIndex ), nIndex );
                Insert( new XclImpXFRange( nFirstScRow, nScRow - 1, pThisRange->maXFIndex ), nIndex );
            }
            return;
        }
        else if( pPrevRange->Expand( nScRow, rXFIndex ) )    
        {
            TryConcatPrev( nNextIndex );    
            return;
        }
    }

    
    if( pNextRange && pNextRange->Expand( nScRow, rXFIndex ) )
        return;

    
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

    
    if( maIndexList.empty() )
    {
        rpPrevRange = rpNextRange = 0;
        rnNextIndex = 0;
        return;
    }

    rpPrevRange = &maIndexList.front();
    rpNextRange = &maIndexList.back();

    
    
    if( rpNextRange->mnScRow1 <= nScRow )
    {
        rpPrevRange = rpNextRange;
        rpNextRange = 0;
        rnNextIndex = maIndexList.size();
        return;
    }

    
    if( nScRow < rpPrevRange->mnScRow1 )
    {
        rpNextRange = rpPrevRange;
        rpPrevRange = 0;
        rnNextIndex = 0;
        return;
    }

    
    
    
    sal_uLong nPrevIndex = 0;
    sal_uLong nMidIndex;
    rnNextIndex = maIndexList.size() - 1;
    XclImpXFRange* pMidRange;
    while( ((rnNextIndex - nPrevIndex) > 1) && (rpPrevRange->mnScRow2 < nScRow) )
    {
        nMidIndex = (nPrevIndex + rnNextIndex) / 2;
        pMidRange = &maIndexList[nMidIndex];
        OSL_ENSURE( pMidRange, "XclImpXFRangeColumn::Find - missing XF index range" );
        if( nScRow < pMidRange->mnScRow1 )      
        {
            rpNextRange = pMidRange;
            rnNextIndex = nMidIndex;
        }
        else                                    
        {
            rpPrevRange = pMidRange;
            nPrevIndex = nMidIndex;
        }
    }

    
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

    
    size_t nIndex = static_cast< size_t >( nScCol );
    if( maColumns.size() <= nIndex )
        maColumns.resize( nIndex + 1 );
    if( !maColumns[ nIndex ] )
        maColumns[ nIndex ].reset( new XclImpXFRangeColumn );
    
    maColumns[ nIndex ]->SetXF( nScRow, XclImpXFIndex( nXFIndex, eMode == xlXFModeBoolCell ) );

    
    
    if( eMode != xlXFModeRow )
    {
        const XclImpXF* pXF = GetXFBuffer().GetXF( nXFIndex );
        if( pXF && ((pXF->GetHorAlign() == EXC_XF_HOR_CENTER_AS) || (pXF->GetHorAlign() == EXC_XF_HOR_FILL)) )
        {
            
            ScRange* pRange = maMergeList.empty() ? NULL : maMergeList.back();
            if (pRange && (pRange->aEnd.Row() == nScRow) && (pRange->aEnd.Col() + 1 == nScCol) && (eMode == xlXFModeBlank))
                pRange->aEnd.IncCol();
            else if( eMode != xlXFModeBlank )   
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

void XclImpXFRangeBuffer::SetHyperlink( const XclRange& rXclRange, const OUString& rUrl )
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

    
    XclImpXFBuffer& rXFBuffer = GetXFBuffer();
    for( XclImpXFRangeColumnVec::const_iterator aVBeg = maColumns.begin(), aVEnd = maColumns.end(), aVIt = aVBeg; aVIt != aVEnd; ++aVIt )
    {
        
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
            assert(nAttrSize > 0);
            ScAttrEntry* pData = new ScAttrEntry[nAttrSize];
            list<ScAttrEntry>::const_iterator itr = aAttrs.begin(), itrEnd = aAttrs.end();
            for (size_t i = 0; itr != itrEnd; ++itr, ++i)
                pData[i] = *itr;

            rDoc.SetAttrEntries(nScCol, nScTab, pData, static_cast<SCSIZE>(nAttrSize));
        }
    }

    
    for( XclImpHyperlinkList::const_iterator aLIt = maHyperlinks.begin(), aLEnd = maHyperlinks.end(); aLIt != aLEnd; ++aLIt )
        XclImpHyperlink::InsertUrl( GetRoot(), aLIt->first, aLIt->second );

    
    for ( size_t i = 0, nRange = maMergeList.size(); i < nRange; ++i )
    {
        const ScRange* pRange = maMergeList[ i ];
        const ScAddress& rStart = pRange->aStart;
        const ScAddress& rEnd = pRange->aEnd;
        bool bMultiCol = rStart.Col() != rEnd.Col();
        bool bMultiRow = rStart.Row() != rEnd.Row();
        
        if( bMultiCol )
            SetBorderLine( *pRange, nScTab, BOX_LINE_RIGHT );
        
        if( bMultiRow )
            SetBorderLine( *pRange, nScTab, BOX_LINE_BOTTOM );
        
        if( bMultiCol || bMultiRow )
            rDoc.DoMerge( nScTab, rStart.Col(), rStart.Row(), rEnd.Col(), rEnd.Row() );
        
        if( !bMultiRow )
        {
            bool bTextWrap = static_cast< const SfxBoolItem* >( rDoc.GetAttr( rStart.Col(), rStart.Row(), rStart.Tab(), ATTR_LINEBREAK ) )->GetValue();
            if( !bTextWrap && (rDoc.GetCellType( rStart ) == CELLTYPE_EDIT) )
                if (const EditTextObject* pEditObj = rDoc.GetEditText(rStart))
                    bTextWrap = pEditObj->GetParagraphCount() > 1;
            if( bTextWrap )
                GetOldRoot().pColRowBuff->SetManualRowHeight( rStart.Row() );
        }
    }
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
