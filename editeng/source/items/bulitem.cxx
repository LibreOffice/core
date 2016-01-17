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


void SvxBulletItem::StoreFont( SvStream& rStream, const vcl::Font& rFont )
{
    sal_uInt16 nTemp;

    WriteColor( rStream, rFont.GetColor() );
    nTemp = (sal_uInt16)rFont.GetFamily(); rStream.WriteUInt16( nTemp );

    nTemp = (sal_uInt16)GetSOStoreTextEncoding((rtl_TextEncoding)rFont.GetCharSet());
    rStream.WriteUInt16( nTemp );

    nTemp = (sal_uInt16)rFont.GetPitch(); rStream.WriteUInt16( nTemp );
    nTemp = (sal_uInt16)rFont.GetAlign(); rStream.WriteUInt16( nTemp );
    nTemp = (sal_uInt16)rFont.GetWeight(); rStream.WriteUInt16( nTemp );
    nTemp = (sal_uInt16)rFont.GetUnderline(); rStream.WriteUInt16( nTemp );
    nTemp = (sal_uInt16)rFont.GetStrikeout(); rStream.WriteUInt16( nTemp );
    nTemp = (sal_uInt16)rFont.GetItalic(); rStream.WriteUInt16( nTemp );

    // UNICODE: rStream << rFont.GetFamilyName();
    rStream.WriteUniOrByteString(rFont.GetFamilyName(), rStream.GetStreamCharSet());

    rStream.WriteBool( rFont.IsOutline() );
    rStream.WriteBool( rFont.IsShadow() );
    rStream.WriteBool( rFont.IsTransparent() );
}


vcl::Font SvxBulletItem::CreateFont( SvStream& rStream, sal_uInt16 nVer )
{
    vcl::Font aFont;
    Color aColor;
    ReadColor( rStream, aColor );    aFont.SetColor( aColor );
    sal_uInt16 nTemp;
    rStream.ReadUInt16( nTemp ); aFont.SetFamily((FontFamily)nTemp);

    rStream.ReadUInt16( nTemp );
    nTemp = (sal_uInt16)GetSOLoadTextEncoding((rtl_TextEncoding)nTemp);
    aFont.SetCharSet((rtl_TextEncoding)nTemp);

    rStream.ReadUInt16( nTemp ); aFont.SetPitch((FontPitch)nTemp);
    rStream.ReadUInt16( nTemp ); aFont.SetAlign((FontAlign)nTemp);
    rStream.ReadUInt16( nTemp ); aFont.SetWeight((FontWeight)nTemp);
    rStream.ReadUInt16( nTemp ); aFont.SetUnderline((FontUnderline)nTemp);
    rStream.ReadUInt16( nTemp ); aFont.SetStrikeout((FontStrikeout)nTemp);
    rStream.ReadUInt16( nTemp ); aFont.SetItalic((FontItalic)nTemp);

    // UNICODE: rStream >> aName; aFont.SetName( aName );
    OUString aName = rStream.ReadUniOrByteString(rStream.GetStreamCharSet());
    aFont.SetName( aName );

    if( nVer == 1 )
    {
        sal_Int32 nHeight(0), nWidth(0);
        rStream.ReadInt32( nHeight ); rStream.ReadInt32( nWidth ); Size aSize( nWidth, nHeight );
        aFont.SetSize( aSize );
    }

    bool bTemp;
    rStream.ReadCharAsBool( bTemp ); aFont.SetOutline( bTemp );
    rStream.ReadCharAsBool( bTemp ); aFont.SetShadow( bTemp );
    rStream.ReadCharAsBool( bTemp ); aFont.SetTransparent( bTemp );
    return aFont;
}


SvxBulletItem::SvxBulletItem( sal_uInt16 _nWhich ) : SfxPoolItem( _nWhich )
{
    SetDefaultFont_Impl();
    SetDefaults_Impl();
}

SvxBulletItem::SvxBulletItem( SvStream& rStrm, sal_uInt16 _nWhich )
    : SfxPoolItem(_nWhich)
    , pGraphicObject(nullptr)
    , nStart(0)
    , nStyle(SvxBulletStyle::ABC_BIG)
    , nScale(0)
{
    sal_uInt16 nTmp1;
    rStrm.ReadUInt16( nTmp1 );
    nStyle = static_cast<SvxBulletStyle>(nTmp1);

    if( nStyle != SvxBulletStyle::BMP )
        aFont = CreateFont( rStrm, BULITEM_VERSION );
    else
    {
        // Safe Load with Test on empty Bitmap
        Bitmap          aBmp;
        const sal_Size    nOldPos = rStrm.Tell();
        // Ignore Errorcode when reading Bitmap,
        // see comment in SvxBulletItem::Store()
        bool bOldError = rStrm.GetError() != 0;
        ReadDIB(aBmp, rStrm, true);

        if ( !bOldError && rStrm.GetError() )
        {
            rStrm.ResetError();
        }

        if( aBmp.IsEmpty() )
        {
            rStrm.Seek( nOldPos );
            nStyle = SvxBulletStyle::NONE;
        }
        else
            pGraphicObject = new GraphicObject( aBmp );
    }

    sal_Int32 nTmp(0);
    rStrm.ReadInt32( nTmp ); nWidth = nTmp;
    rStrm.ReadUInt16( nStart );
    sal_uInt8 nTmpInt8(0);
    rStrm.ReadUChar( nTmpInt8 ); // used to be nJustify

    char cTmpSymbol(0);
    rStrm.ReadChar( cTmpSymbol );
    //convert single byte to unicode
    cSymbol = OUString(&cTmpSymbol, 1, aFont.GetCharSet()).toChar();

    rStrm.ReadUInt16( nScale );

    // UNICODE: rStrm >> aPrevText;
    aPrevText = rStrm.ReadUniOrByteString(rStrm.GetStreamCharSet());

    // UNICODE: rStrm >> aFollowText;
    aFollowText = rStrm.ReadUniOrByteString(rStrm.GetStreamCharSet());
}

SvxBulletItem::SvxBulletItem( const SvxBulletItem& rItem) : SfxPoolItem( rItem )
{
    aFont           = rItem.aFont;
    pGraphicObject  = ( rItem.pGraphicObject ? new GraphicObject( *rItem.pGraphicObject ) : nullptr );
    aPrevText       = rItem.aPrevText;
    aFollowText     = rItem.aFollowText;
    nStart          = rItem.nStart;
    nStyle          = rItem.nStyle;
    nWidth          = rItem.nWidth;
    nScale          = rItem.nScale;
    cSymbol         = rItem.cSymbol;
}


SvxBulletItem::~SvxBulletItem()
{
    delete pGraphicObject;
}


SfxPoolItem* SvxBulletItem::Clone( SfxItemPool * /*pPool*/ ) const
{
    return new SvxBulletItem( *this );
}


SfxPoolItem* SvxBulletItem::Create( SvStream& rStrm, sal_uInt16 /*nVersion*/ ) const
{
    return new SvxBulletItem( rStrm, Which() );
}


void SvxBulletItem::SetDefaultFont_Impl()
{
    aFont = OutputDevice::GetDefaultFont( DefaultFontType::FIXED, LANGUAGE_SYSTEM, GetDefaultFontFlags::NONE );
    aFont.SetAlign( ALIGN_BOTTOM);
    aFont.SetTransparent( true );
}


void SvxBulletItem::SetDefaults_Impl()
{
    pGraphicObject  = nullptr;
    nWidth          = 1200;  // 1.2cm
    nStart          = 1;
    nStyle          = SvxBulletStyle::N123;
    cSymbol         = ' ';
    nScale          = 75;
}


sal_uInt16 SvxBulletItem::GetVersion( sal_uInt16 /*nVersion*/ ) const
{
    return BULITEM_VERSION;
}


void SvxBulletItem::CopyValidProperties( const SvxBulletItem& rCopyFrom )
{
    vcl::Font _aFont = GetFont();
    vcl::Font aNewFont = rCopyFrom.GetFont();
    _aFont.SetName( aNewFont.GetFamilyName() );
    _aFont.SetFamily( aNewFont.GetFamily() );
    _aFont.SetStyleName( aNewFont.GetStyleName() );
    _aFont.SetColor( aNewFont.GetColor() );
    SetSymbol( rCopyFrom.GetSymbol() );
    SetGraphicObject( rCopyFrom.GetGraphicObject() );
    SetScale( rCopyFrom.GetScale() );
    SetStart( rCopyFrom.GetStart() );
    SetStyle( rCopyFrom.GetStyle() );
    SetPrevText( rCopyFrom.GetPrevText() );
    SetFollowText( rCopyFrom.GetFollowText() );
    SetFont( _aFont );
}


bool SvxBulletItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT(dynamic_cast< const SvxBulletItem* >(&rItem) !=  nullptr,"operator==Types not matching");
    const SvxBulletItem& rBullet = static_cast<const SvxBulletItem&>(rItem);
    // Compare with ValidMask, otherwise no put possible in a AttrSet if the
    // item differs only in terms of the ValidMask from an existing one.
    if( nStyle != rBullet.nStyle            ||
        nScale != rBullet.nScale            ||
        nWidth != rBullet.nWidth            ||
        nStart != rBullet.nStart            ||
        cSymbol != rBullet.cSymbol          ||
        aPrevText != rBullet.aPrevText      ||
        aFollowText != rBullet.aFollowText )
            return false;

    if( ( nStyle != SvxBulletStyle::BMP ) && ( aFont != rBullet.aFont ) )
        return false;

    if( nStyle == SvxBulletStyle::BMP )
    {
        if( ( pGraphicObject && !rBullet.pGraphicObject ) || ( !pGraphicObject && rBullet.pGraphicObject ) )
            return false;

        if( ( pGraphicObject && rBullet.pGraphicObject ) &&
            ( ( *pGraphicObject != *rBullet.pGraphicObject ) ||
              ( pGraphicObject->GetPrefSize() != rBullet.pGraphicObject->GetPrefSize() ) ) )
        {
            return false;
        }
    }

    return true;
}


SvStream& SvxBulletItem::Store( SvStream& rStrm, sal_uInt16 /*nItemVersion*/ ) const
{
    // Correction for empty bitmap
    if( ( nStyle == SvxBulletStyle::BMP ) &&
        ( !pGraphicObject || ( GRAPHIC_NONE == pGraphicObject->GetType() ) || ( GRAPHIC_DEFAULT == pGraphicObject->GetType() ) ) )
    {
        if( pGraphicObject )
        {
            delete( const_cast< SvxBulletItem* >( this )->pGraphicObject );
            const_cast< SvxBulletItem* >( this )->pGraphicObject = nullptr;
        }

        const_cast< SvxBulletItem* >( this )->nStyle = SvxBulletStyle::NONE;
    }

    rStrm.WriteUInt16( static_cast<sal_uInt16>(nStyle) );

    if( nStyle != SvxBulletStyle::BMP )
        StoreFont( rStrm, aFont );
    else
    {
        sal_Size _nStart = rStrm.Tell();

        // Small preliminary estimate of the size ...
        sal_uInt16 nFac = ( rStrm.GetCompressMode() != SvStreamCompressFlags::NONE ) ? 3 : 1;
        const Bitmap aBmp( pGraphicObject->GetGraphic().GetBitmap() );
        sal_uLong nBytes = aBmp.GetSizeBytes();
        if ( nBytes < sal_uLong(0xFF00*nFac) )
        {
            WriteDIB(aBmp, rStrm, false, true);
        }

        sal_Size nEnd = rStrm.Tell();
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
    rStrm.WriteInt32( nWidth );
    rStrm.WriteUInt16( nStart );
    rStrm.WriteUChar( 0 ); // used to be nJustify
    rStrm.WriteChar( OUStringToOString(OUString(cSymbol), aFont.GetCharSet()).toChar() );
    rStrm.WriteUInt16( nScale );

    // UNICODE: rStrm << aPrevText;
    rStrm.WriteUniOrByteString(aPrevText, rStrm.GetStreamCharSet());

    // UNICODE: rStrm << aFollowText;
    rStrm.WriteUniOrByteString(aFollowText, rStrm.GetStreamCharSet());

    return rStrm;
}


OUString SvxBulletItem::GetFullText() const
{
    OUStringBuffer aStr(aPrevText);
    aStr.append(cSymbol);
    aStr.append(aFollowText);
    return aStr.makeStringAndClear();
}


bool SvxBulletItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    SfxMapUnit          /*eCoreUnit*/,
    SfxMapUnit          /*ePresUnit*/,
    OUString&           rText, const IntlWrapper *
)   const
{
    rText = GetFullText();
    return true;
}


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


void SvxBulletItem::SetGraphicObject( const GraphicObject& rGraphicObject )
{
    if( ( GRAPHIC_NONE == rGraphicObject.GetType() ) || ( GRAPHIC_DEFAULT == rGraphicObject.GetType() ) )
    {
        if( pGraphicObject )
        {
            delete pGraphicObject;
            pGraphicObject = nullptr;
        }
    }
    else
    {
        delete pGraphicObject;
        pGraphicObject = new GraphicObject( rGraphicObject );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
