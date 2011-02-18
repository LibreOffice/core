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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_editeng.hxx"

// include ---------------------------------------------------------------
#include <tools/stream.hxx>
#include <vcl/outdev.hxx>

#define _SVX_BULITEM_CXX

#include <editeng/bulitem.hxx>
#include <editeng/editrids.hrc>

// #90477#
#include <tools/tenccvt.hxx>

#define BULITEM_VERSION     ((sal_uInt16)2)

// -----------------------------------------------------------------------

TYPEINIT1(SvxBulletItem,SfxPoolItem);

// -----------------------------------------------------------------------

void SvxBulletItem::StoreFont( SvStream& rStream, const Font& rFont )
{
    sal_uInt16 nTemp;

    rStream << rFont.GetColor();
    nTemp = (sal_uInt16)rFont.GetFamily(); rStream << nTemp;

    // #90477# nTemp = (sal_uInt16)GetStoreCharSet( rFont.GetCharSet(), rStream.GetVersion() );
    nTemp = (sal_uInt16)GetSOStoreTextEncoding((rtl_TextEncoding)rFont.GetCharSet(), (sal_uInt16)rStream.GetVersion());
    rStream << nTemp;

    nTemp = (sal_uInt16)rFont.GetPitch(); rStream << nTemp;
    nTemp = (sal_uInt16)rFont.GetAlign(); rStream << nTemp;
    nTemp = (sal_uInt16)rFont.GetWeight(); rStream << nTemp;
    nTemp = (sal_uInt16)rFont.GetUnderline(); rStream << nTemp;
    nTemp = (sal_uInt16)rFont.GetStrikeout(); rStream << nTemp;
    nTemp = (sal_uInt16)rFont.GetItalic(); rStream << nTemp;

    // UNICODE: rStream << rFont.GetName();
    rStream.WriteByteString(rFont.GetName());

    rStream << rFont.IsOutline();
    rStream << rFont.IsShadow();
    rStream << rFont.IsTransparent();
}

// -----------------------------------------------------------------------

Font SvxBulletItem::CreateFont( SvStream& rStream, sal_uInt16 nVer )
{
    Font aFont;
    Color aColor;
    rStream >> aColor;    aFont.SetColor( aColor );
    sal_uInt16 nTemp;
    rStream >> nTemp; aFont.SetFamily((FontFamily)nTemp);

    // #90477#
    rStream >> nTemp;
    nTemp = (sal_uInt16)GetSOLoadTextEncoding((rtl_TextEncoding)nTemp, (sal_uInt16)rStream.GetVersion());
    aFont.SetCharSet((rtl_TextEncoding)nTemp);

    rStream >> nTemp; aFont.SetPitch((FontPitch)nTemp);
    rStream >> nTemp; aFont.SetAlign((FontAlign)nTemp);
    rStream >> nTemp; aFont.SetWeight((FontWeight)nTemp);
    rStream >> nTemp; aFont.SetUnderline((FontUnderline)nTemp);
    rStream >> nTemp; aFont.SetStrikeout((FontStrikeout)nTemp);
    rStream >> nTemp; aFont.SetItalic((FontItalic)nTemp);

    // UNICODE: rStream >> aName; aFont.SetName( aName );
    String aName;
    rStream.ReadByteString(aName);
    aFont.SetName( aName );

    if( nVer == 1 )
    {
        long nHeight, nWidth;
        rStream >> nHeight; rStream >> nWidth; Size aSize( nWidth, nHeight );
        aFont.SetSize( aSize );
    }

    sal_Bool bTemp;
    rStream >> bTemp; aFont.SetOutline( bTemp );
    rStream >> bTemp; aFont.SetShadow( bTemp );
    rStream >> bTemp; aFont.SetTransparent( bTemp );
    return aFont;
}


// -----------------------------------------------------------------------

SvxBulletItem::SvxBulletItem( sal_uInt16 _nWhich ) : SfxPoolItem( _nWhich )
{
    SetDefaultFont_Impl();
    SetDefaults_Impl();
    nValidMask = 0xFFFF;
}

// -----------------------------------------------------------------------

SvxBulletItem::SvxBulletItem( sal_uInt8 nNewStyle, const Font& rFont, sal_uInt16 /*nStart*/, sal_uInt16 _nWhich ) : SfxPoolItem( _nWhich )
{
    SetDefaults_Impl();
    nStyle = nNewStyle;
    aFont  = rFont;
    nValidMask = 0xFFFF;
}

// -----------------------------------------------------------------------

SvxBulletItem::SvxBulletItem( const Font& rFont, xub_Unicode cSymb, sal_uInt16 _nWhich ) : SfxPoolItem( _nWhich )
{
    SetDefaults_Impl();
    aFont   = rFont;
    cSymbol = cSymb;
    nStyle  = BS_BULLET;
    nValidMask = 0xFFFF;
}

// -----------------------------------------------------------------------

SvxBulletItem::SvxBulletItem( const Bitmap& rBmp, sal_uInt16 _nWhich ) : SfxPoolItem( _nWhich )
{
    SetDefaults_Impl();

    if( !rBmp.IsEmpty() )
    {
        pGraphicObject = new GraphicObject( rBmp );
        nStyle = BS_BMP;
    }

    nValidMask = 0xFFFF;
}

// -----------------------------------------------------------------------

SvxBulletItem::SvxBulletItem( const GraphicObject& rGraphicObject, sal_uInt16 _nWhich ) : SfxPoolItem( _nWhich )
{
    SetDefaults_Impl();

    if( ( GRAPHIC_NONE != pGraphicObject->GetType() ) && ( GRAPHIC_DEFAULT != pGraphicObject->GetType() ) )
    {
        pGraphicObject = new GraphicObject( rGraphicObject );
        nStyle = BS_BMP;
    }

    nValidMask = 0xFFFF;
}

// -----------------------------------------------------------------------

SvxBulletItem::SvxBulletItem( SvStream& rStrm, sal_uInt16 _nWhich ) :
    SfxPoolItem( _nWhich ),
    pGraphicObject( NULL )
{
    rStrm >> nStyle;

    if( nStyle != BS_BMP )
        aFont = CreateFont( rStrm, BULITEM_VERSION );
    else
    {
        // Sicheres Laden mit Test auf leere Bitmap
        Bitmap          aBmp;
        const sal_uInt32    nOldPos = rStrm.Tell();
        // #69345# Errorcode beim Bitmap lesen ignorieren,
        // siehe Kommentar #67581# in SvxBulletItem::Store()
        sal_Bool bOldError = rStrm.GetError() ? sal_True : sal_False;
        rStrm >> aBmp;
        if ( !bOldError && rStrm.GetError() )
        {
            rStrm.ResetError();
            // #71493# Keine Warnung: Das BulletItem interessiert seit 5.0 im Dateiformat nicht mehr.
            // rStrm.SetError(ERRCODE_CLASS_READ | ERRCODE_SVX_BULLETITEM_NOBULLET | ERRCODE_WARNING_MASK);
        }

        if( aBmp.IsEmpty() )
        {
            rStrm.Seek( nOldPos );
            nStyle = BS_NONE;
        }
        else
            pGraphicObject = new GraphicObject( aBmp );
    }

    rStrm >> nWidth;
    rStrm >> nStart;
    rStrm >> nJustify;

    char cTmpSymbol;
    rStrm >> cTmpSymbol;
    cSymbol = ByteString::ConvertToUnicode( cTmpSymbol, aFont.GetCharSet() );

    rStrm >> nScale;

    // UNICODE: rStrm >> aPrevText;
    rStrm.ReadByteString(aPrevText);

    // UNICODE: rStrm >> aFollowText;
    rStrm.ReadByteString(aFollowText);

    nValidMask = 0xFFFF;
}

// -----------------------------------------------------------------------

SvxBulletItem::SvxBulletItem( const SvxBulletItem& rItem) : SfxPoolItem( rItem )
{
    aFont           = rItem.aFont;
    pGraphicObject  = ( rItem.pGraphicObject ? new GraphicObject( *rItem.pGraphicObject ) : NULL );
    aPrevText       = rItem.aPrevText;
    aFollowText     = rItem.aFollowText;
    nStart          = rItem.nStart;
    nStyle          = rItem.nStyle;
    nWidth          = rItem.nWidth;
    nScale          = rItem.nScale;
    cSymbol         = rItem.cSymbol;
    nJustify        = rItem.nJustify;
    nValidMask      = rItem.nValidMask;
}

// -----------------------------------------------------------------------

SvxBulletItem::~SvxBulletItem()
{
    if( pGraphicObject )
        delete pGraphicObject;
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxBulletItem::Clone( SfxItemPool * /*pPool*/ ) const
{
    return new SvxBulletItem( *this );
}

// -----------------------------------------------------------------------

SfxPoolItem* SvxBulletItem::Create( SvStream& rStrm, sal_uInt16 /*nVersion*/ ) const
{
    return new SvxBulletItem( rStrm, Which() );
}

// -----------------------------------------------------------------------

void SvxBulletItem::SetDefaultFont_Impl()
{
    aFont = OutputDevice::GetDefaultFont( DEFAULTFONT_FIXED, LANGUAGE_SYSTEM, 0 );
    aFont.SetAlign( ALIGN_BOTTOM);
    aFont.SetTransparent( sal_True );
}

// -----------------------------------------------------------------------

void SvxBulletItem::SetDefaults_Impl()
{
    pGraphicObject  = NULL;
    nWidth          = 1200;  // 1.2cm
    nStart          = 1;
    nStyle          = BS_123;
    nJustify        = BJ_HLEFT | BJ_VCENTER;
    cSymbol         = sal_Unicode(' ');
    nScale          = 75;
}

// -----------------------------------------------------------------------

sal_uInt16 SvxBulletItem::GetVersion( sal_uInt16 /*nVersion*/ ) const
{
    return BULITEM_VERSION;
}

// -----------------------------------------------------------------------

void SvxBulletItem::CopyValidProperties( const SvxBulletItem& rCopyFrom )
{
    Font _aFont = GetFont();
    Font aNewFont = rCopyFrom.GetFont();
    if ( rCopyFrom.IsValid( VALID_FONTNAME ) )
    {
        _aFont.SetName( aNewFont.GetName() );
        _aFont.SetFamily( aNewFont.GetFamily() );
        _aFont.SetStyleName( aNewFont.GetStyleName() );
    }
    if ( rCopyFrom.IsValid( VALID_FONTCOLOR ) )
        _aFont.SetColor( aNewFont.GetColor() );
    if ( rCopyFrom.IsValid( VALID_SYMBOL ) )
        SetSymbol( rCopyFrom.GetSymbol() );
    if ( rCopyFrom.IsValid( VALID_BITMAP ) )
        SetGraphicObject( rCopyFrom.GetGraphicObject() );
    if ( rCopyFrom.IsValid( VALID_SCALE ) )
        SetScale( rCopyFrom.GetScale() );
    if ( rCopyFrom.IsValid( VALID_START ) )
        SetStart( rCopyFrom.GetStart() );
    if ( rCopyFrom.IsValid( VALID_STYLE ) )
        SetStyle( rCopyFrom.GetStyle() );
    if ( rCopyFrom.IsValid( VALID_PREVTEXT ) )
        SetPrevText( rCopyFrom.GetPrevText() );
    if ( rCopyFrom.IsValid( VALID_FOLLOWTEXT ) )
        SetFollowText( rCopyFrom.GetFollowText() );

    SetFont( _aFont );
}


// -----------------------------------------------------------------------

int SvxBulletItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT(rItem.ISA(SvxBulletItem),"operator==Types not matching");
    const SvxBulletItem& rBullet = (const SvxBulletItem&)rItem;
    // ValidMask mitvergleichen, da sonst kein Putten in ein AttrSet moeglich,
    // wenn sich das Item nur in der ValidMask von einem existierenden unterscheidet.
    if( nValidMask != rBullet.nValidMask    ||
        nStyle != rBullet.nStyle            ||
        nScale != rBullet.nScale            ||
        nJustify != rBullet.nJustify        ||
        nWidth != rBullet.nWidth            ||
        nStart != rBullet.nStart            ||
        cSymbol != rBullet.cSymbol          ||
        aPrevText != rBullet.aPrevText      ||
        aFollowText != rBullet.aFollowText )
            return 0;

    if( ( nStyle != BS_BMP ) && ( aFont != rBullet.aFont ) )
        return 0;

    if( nStyle == BS_BMP )
    {
        if( ( pGraphicObject && !rBullet.pGraphicObject ) || ( !pGraphicObject && rBullet.pGraphicObject ) )
            return 0;

        if( ( pGraphicObject && rBullet.pGraphicObject ) &&
            ( ( *pGraphicObject != *rBullet.pGraphicObject ) ||
              ( pGraphicObject->GetPrefSize() != rBullet.pGraphicObject->GetPrefSize() ) ) )
        {
            return 0;
        }
    }

    return 1;
}

// -----------------------------------------------------------------------

SvStream& SvxBulletItem::Store( SvStream& rStrm, sal_uInt16 /*nItemVersion*/ ) const
{
    // Korrektur bei leerer Bitmap
    if( ( nStyle == BS_BMP ) &&
        ( !pGraphicObject || ( GRAPHIC_NONE == pGraphicObject->GetType() ) || ( GRAPHIC_DEFAULT == pGraphicObject->GetType() ) ) )
    {
        if( pGraphicObject )
        {
            delete( const_cast< SvxBulletItem* >( this )->pGraphicObject );
            const_cast< SvxBulletItem* >( this )->pGraphicObject = NULL;
        }

        const_cast< SvxBulletItem* >( this )->nStyle = BS_NONE;
    }

    rStrm << nStyle;

    if( nStyle != BS_BMP )
        StoreFont( rStrm, aFont );
    else
    {
        sal_uLong _nStart = rStrm.Tell();

        // Kleine Vorab-Schaetzung der Groesse...
        sal_uInt16 nFac = ( rStrm.GetCompressMode() != COMPRESSMODE_NONE ) ? 3 : 1;
        const Bitmap aBmp( pGraphicObject->GetGraphic().GetBitmap() );
        sal_uLong nBytes = aBmp.GetSizeBytes();
        if ( nBytes < sal_uLong(0xFF00*nFac) )
            rStrm << aBmp;

        sal_uLong nEnd = rStrm.Tell();
        // #67581# Item darf mit Overhead nicht mehr als 64K schreiben,
        // sonst platzt der SfxMultiRecord
        // Dann lieber auf die Bitmap verzichten, ist nur fuer Outliner
        // und auch nur fuer <= 5.0 wichtig.
        // Beim Einlesen merkt der Stream-Operator der Bitmap, dass dort keine steht.
        // Hiermit funktioniert jetzt der Fall das die grosse Bitmap aus einem anderen
        // Fileformat entstanden ist, welches keine 64K belegt, aber wenn eine
        // Bitmap > 64K verwendet wird, hat das SvxNumBulletItem beim Laden ein Problem,
        // stuerzt aber nicht ab.

        if ( (nEnd-_nStart) > 0xFF00 )
            rStrm.Seek( _nStart );
    }
    rStrm << nWidth;
    rStrm << nStart;
    rStrm << nJustify;
    rStrm << (char)ByteString::ConvertFromUnicode( cSymbol, aFont.GetCharSet() );
    rStrm << nScale;

    // UNICODE: rStrm << aPrevText;
    rStrm.WriteByteString(aPrevText);

    // UNICODE: rStrm << aFollowText;
    rStrm.WriteByteString(aFollowText);

    return rStrm;
}

//------------------------------------------------------------------------

XubString SvxBulletItem::GetFullText() const
{
    XubString aStr( aPrevText );
    aStr += cSymbol;
    aStr += aFollowText;
    return aStr;
}

//------------------------------------------------------------------------

SfxItemPresentation SvxBulletItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    XubString&          rText, const IntlWrapper *
)   const
{
    SfxItemPresentation eRet = SFX_ITEM_PRESENTATION_NONE;
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText.Erase();
            eRet = SFX_ITEM_PRESENTATION_NONE;
            break;

        case SFX_ITEM_PRESENTATION_NAMELESS:
        case SFX_ITEM_PRESENTATION_COMPLETE:
            rText = GetFullText();
            eRet = SFX_ITEM_PRESENTATION_COMPLETE;
            break;
        default: ; //prevent warning
    }
    return eRet;
}

//------------------------------------------------------------------------

Bitmap SvxBulletItem::GetBitmap() const
{
    if( pGraphicObject )
        return pGraphicObject->GetGraphic().GetBitmap();
    else
    {
        const Bitmap aDefaultBitmap;
        return aDefaultBitmap;
    }
}

//------------------------------------------------------------------------

void SvxBulletItem::SetBitmap( const Bitmap& rBmp )
{
    if( rBmp.IsEmpty() )
    {
        if( pGraphicObject )
        {
            delete pGraphicObject;
            pGraphicObject = NULL;
        }
    }
    else
    {
        delete pGraphicObject;
        pGraphicObject = new GraphicObject( rBmp );

    }
}

//------------------------------------------------------------------------

const GraphicObject& SvxBulletItem::GetGraphicObject() const
{
    if( pGraphicObject )
        return *pGraphicObject;
    else
    {
        static const GraphicObject aDefaultObject;
        return aDefaultObject;
    }
}

//------------------------------------------------------------------------

void SvxBulletItem::SetGraphicObject( const GraphicObject& rGraphicObject )
{
    if( ( GRAPHIC_NONE == rGraphicObject.GetType() ) || ( GRAPHIC_DEFAULT == rGraphicObject.GetType() ) )
    {
        if( pGraphicObject )
        {
            delete pGraphicObject;
            pGraphicObject = NULL;
        }
    }
    else
    {
        delete pGraphicObject;
        pGraphicObject = new GraphicObject( rGraphicObject );
    }
}
