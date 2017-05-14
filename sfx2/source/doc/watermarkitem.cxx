/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/watermarkitem.hxx>
#include <sfx2/sfxsids.hrc>

SfxWatermarkItem::SfxWatermarkItem()
: SfxPoolItem( SID_WATERMARK )
, m_aText( "" )
{
}

SfxPoolItem* SfxWatermarkItem::CreateDefault()
{
    return new SfxWatermarkItem();
}

SfxWatermarkItem::SfxWatermarkItem( sal_uInt16 nWhichId, const OUString& rText )
: SfxPoolItem( nWhichId )
, m_aText( rText )
{
}

SfxWatermarkItem::SfxWatermarkItem( const SfxWatermarkItem& rCopy )
: SfxPoolItem( rCopy )
, m_aText( rCopy.m_aText )
{
}

bool SfxWatermarkItem::operator==( const SfxPoolItem& rCmp ) const
{
    return ( SfxPoolItem::operator==( rCmp ) &&
             m_aText == static_cast<const SfxWatermarkItem&>(rCmp).m_aText );
}

SfxPoolItem* SfxWatermarkItem::Clone( SfxItemPool *) const
{
    return new SfxWatermarkItem(*this);
}

bool SfxWatermarkItem::QueryValue( css::uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    rVal <<= m_aText;

    return true;
}

bool SfxWatermarkItem::PutValue( const css::uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    OUString aText;

    if ( rVal >>= aText )
    {
        m_aText = aText;
        return true;
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
