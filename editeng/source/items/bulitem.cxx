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

#include <vcl/outdev.hxx>

#include <editeng/bulletitem.hxx>

SvxBulletItem::SvxBulletItem( sal_uInt16 _nWhich )
    : SfxPoolItem(_nWhich)
    , aFont(OutputDevice::GetDefaultFont( DefaultFontType::FIXED, LANGUAGE_SYSTEM, GetDefaultFontFlags::NONE ))
    , nStart(1)
    , nStyle(SvxBulletStyle::N123)
    , nWidth(1200)  // 1.2cm
    , nScale(75)
    , cSymbol(' ')
{
    aFont.SetAlignment(ALIGN_BOTTOM);
    aFont.SetTransparent( true );
}


SvxBulletItem::SvxBulletItem( const SvxBulletItem& rItem )
    : SfxPoolItem(rItem)
    , aFont(rItem.aFont)
    , pGraphicObject(rItem.pGraphicObject ? new GraphicObject( *rItem.pGraphicObject ) : nullptr)
    , aPrevText(rItem.aPrevText)
    , aFollowText(rItem.aFollowText)
    , nStart(rItem.nStart)
    , nStyle(rItem.nStyle)
    , nWidth(rItem.nWidth)
    , nScale(rItem.nScale)
    , cSymbol(rItem.cSymbol)
{
}


SvxBulletItem::~SvxBulletItem()
{
}

SvxBulletItem* SvxBulletItem::Clone( SfxItemPool * /*pPool*/ ) const
{
    return new SvxBulletItem( *this );
}

void SvxBulletItem::CopyValidProperties( const SvxBulletItem& rCopyFrom )
{
    vcl::Font _aFont = GetFont();
    vcl::Font aNewFont = rCopyFrom.GetFont();
    _aFont.SetFamilyName( aNewFont.GetFamilyName() );
    _aFont.SetFamily( aNewFont.GetFamilyType() );
    _aFont.SetStyleName( aNewFont.GetStyleName() );
    _aFont.SetColor( aNewFont.GetColor() );
    SetSymbol( rCopyFrom.cSymbol );
    SetGraphicObject( rCopyFrom.GetGraphicObject() );
    SetScale( rCopyFrom.nScale );
    SetStart( rCopyFrom.nStart );
    SetStyle( rCopyFrom.nStyle );
    aPrevText = rCopyFrom.aPrevText;
    aFollowText = rCopyFrom.aFollowText;
    SetFont( _aFont );
}


bool SvxBulletItem::operator==( const SfxPoolItem& rItem ) const
{
    assert(SfxPoolItem::operator==(rItem));
    const SvxBulletItem& rBullet = static_cast<const SvxBulletItem&>(rItem);
    // Compare with ValidMask, otherwise no put possible in an AttrSet if the
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


OUString SvxBulletItem::GetFullText() const
{
    return aPrevText + OUStringChar(cSymbol) + aFollowText;
}


bool SvxBulletItem::GetPresentation
(
    SfxItemPresentation /*ePres*/,
    MapUnit             /*eCoreUnit*/,
    MapUnit             /*ePresUnit*/,
    OUString&           rText, const IntlWrapper&
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
    if( ( GraphicType::NONE == rGraphicObject.GetType() ) || ( GraphicType::Default == rGraphicObject.GetType() ) )
    {
         pGraphicObject.reset();
    }
    else
    {
        pGraphicObject.reset( new GraphicObject( rGraphicObject ) );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
