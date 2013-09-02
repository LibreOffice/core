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

#include <tools/stream.hxx>
#include <vcl/outdev.hxx>

#include <editeng/bulletitem.hxx>
#include <editeng/editrids.hrc>

#include <tools/tenccvt.hxx>
#include <vcl/dibtools.hxx>

#define BULITEM_VERSION     ((sal_uInt16)2)

// -----------------------------------------------------------------------

TYPEINIT1(SvxBulletItem,SfxPoolItem);

// -----------------------------------------------------------------------

void SvxBulletItem::StoreFont( SvStream& rStream, const Font& rFont )
{
    sal_uInt16 nTemp;

    rStream << rFont.GetColor();
    nTemp = (sal_uInt16)rFont.GetFamily(); rStream << nTemp;

    nTemp = (sal_uInt16)GetSOStoreTextEncoding((rtl_TextEncoding)rFont.GetCharSet());
    rStream << nTemp;

    nTemp = (sal_uInt16)rFont.GetPitch(); rStream << nTemp;
    nTemp = (sal_uInt16)rFont.GetAlign(); rStream << nTemp;
    nTemp = (sal_uInt16)rFont.GetWeight(); rStream << nTemp;
    nTemp = (sal_uInt16)rFont.GetUnderline(); rStream << nTemp;
    nTemp = (sal_uInt16)rFont.GetStrikeout(); rStream << nTemp;
    nTemp = (sal_uInt16)rFont.GetItalic(); rStream << nTemp;

    // UNICODE: rStream << rFont.GetName();
    rStream.WriteUniOrByteString(rFont.GetName(), rStream.GetStreamCharSet());

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

    rStream >> nTemp;
    nTemp = (sal_uInt16)GetSOLoadTextEncoding((rtl_TextEncoding)nTemp);
    aFont.SetCharSet((rtl_TextEncoding)nTemp);

    rStream >> nTemp; aFont.SetPitch((FontPitch)nTemp);
    rStream >> nTemp; aFont.SetAlign((FontAlign)nTemp);
    rStream >> nTemp; aFont.SetWeight((FontWeight)nTemp);
    rStream >> nTemp; aFont.SetUnderline((FontUnderline)nTemp);
    rStream >> nTemp; aFont.SetStrikeout((FontStrikeout)nTemp);
    rStream >> nTemp; aFont.SetItalic((FontItalic)nTemp);

    // UNICODE: rStream >> aName; aFont.SetName( aName );
    String aName = rStream.ReadUniOrByteString(rStream.GetStreamCharSet());
    aFont.SetName( aName );

    if( nVer == 1 )
    {
        //#fdo39428 SvStream no longer supports operator>>(long&)
        sal_Int32 nHeight(0), nWidth(0);
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

SvxBulletItem::SvxBulletItem( SvStream& rStrm, sal_uInt16 _nWhich ) :
    SfxPoolItem( _nWhich ),
    pGraphicObject( NULL )
{
    rStrm >> nStyle;

    if( nStyle != BS_BMP )
        aFont = CreateFont( rStrm, BULITEM_VERSION );
    else
    {
        // Safe Load with Test on empty Bitmap
        Bitmap          aBmp;
        const sal_uInt32    nOldPos = rStrm.Tell();
        // Ignore Errorcode when reading Bitmap,
        // see comment in SvxBulletItem::Store()
        sal_Bool bOldError = rStrm.GetError() ? sal_True : sal_False;
        ReadDIB(aBmp, rStrm, true);

        if ( !bOldError && rStrm.GetError() )
        {
            rStrm.ResetError();
        }

        if( aBmp.IsEmpty() )
        {
            rStrm.Seek( nOldPos );
            nStyle = BS_NONE;
        }
        else
            pGraphicObject = new GraphicObject( aBmp );
    }

    //#fdo39428 SvStream no longer supports operator>>(long&)
    sal_Int32 nTmp(0);
    rStrm >> nTmp; nWidth = nTmp;
    rStrm >> nStart;
    rStrm >> nJustify;

    char cTmpSymbol;
    rStrm >> cTmpSymbol;
    //convert single byte to unicode
    cSymbol = OUString(&cTmpSymbol, 1, aFont.GetCharSet()).toChar();

    rStrm >> nScale;

    // UNICODE: rStrm >> aPrevText;
    aPrevText = rStrm.ReadUniOrByteString(rStrm.GetStreamCharSet());

    // UNICODE: rStrm >> aFollowText;
    aFollowText = rStrm.ReadUniOrByteString(rStrm.GetStreamCharSet());

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
    // Compare with ValidMask, otherwise no put possible in a AttrSet if the
    // item differs only in terms of the ValidMask from an existing one.
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
    // Correction for empty bitmap
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

        // Small preliminary estimate of the size ...
        sal_uInt16 nFac = ( rStrm.GetCompressMode() != COMPRESSMODE_NONE ) ? 3 : 1;
        const Bitmap aBmp( pGraphicObject->GetGraphic().GetBitmap() );
        sal_uLong nBytes = aBmp.GetSizeBytes();
        if ( nBytes < sal_uLong(0xFF00*nFac) )
        {
            WriteDIB(aBmp, rStrm, false, true);
        }

        sal_uLong nEnd = rStrm.Tell();
        // Item can not write with an overhead more than 64K or SfxMultiRecord
        // will crash. Then prefer to forego on the bitmap, it is only
        // important for the outliner and only for <= 5.0.
        // When reading, the stream-operator makes note of the bitmap and the
        // fact that there is none. This is now the case how it works with
        // large bitmap created from another file format, which do not occupy a
        // 64K chunk, but if a bitmap > 64K is used, the SvxNumBulletItem will
        // have problem loading it, but does not crash.

        if ( (nEnd-_nStart) > 0xFF00 )
            rStrm.Seek( _nStart );
    }
    rStrm << static_cast<sal_Int32>(nWidth);
    rStrm << nStart;
    rStrm << nJustify;
    rStrm << OUStringToOString(OUString(cSymbol), aFont.GetCharSet()).toChar();
    rStrm << nScale;

    // UNICODE: rStrm << aPrevText;
    rStrm.WriteUniOrByteString(aPrevText, rStrm.GetStreamCharSet());

    // UNICODE: rStrm << aFollowText;
    rStrm.WriteUniOrByteString(aFollowText, rStrm.GetStreamCharSet());

    return rStrm;
}

//------------------------------------------------------------------------

OUString SvxBulletItem::GetFullText() const
{
    OUStringBuffer aStr(aPrevText);
    aStr.append(cSymbol);
    aStr.append(aFollowText);
    return aStr.makeStringAndClear();
}

//------------------------------------------------------------------------

SfxItemPresentation SvxBulletItem::GetPresentation
(
    SfxItemPresentation ePres,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    SfxItemPresentation eRet = SFX_ITEM_PRESENTATION_NONE;
    switch ( ePres )
    {
        case SFX_ITEM_PRESENTATION_NONE:
            rText = OUString();
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
