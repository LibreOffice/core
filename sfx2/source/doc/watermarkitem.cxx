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
, m_aFont( "Liberation Sans" )
, m_nAngle( 45 )
, m_nTransparency( 50 )
, m_nColor( 0xc0c0c0 )
{
}

SfxPoolItem* SfxWatermarkItem::CreateDefault()
{
    return new SfxWatermarkItem();
}

SfxWatermarkItem::SfxWatermarkItem( const SfxWatermarkItem& rCopy )
: SfxPoolItem( rCopy )
, m_aText( rCopy.m_aText )
, m_aFont( rCopy.m_aFont )
, m_nAngle( rCopy.m_nAngle )
, m_nTransparency( rCopy.m_nTransparency )
, m_nColor( rCopy.m_nColor )
{
}

bool SfxWatermarkItem::operator==( const SfxPoolItem& rCmp ) const
{
    return ( SfxPoolItem::operator==( rCmp ) &&
             m_aText == static_cast<const SfxWatermarkItem&>(rCmp).m_aText &&
             m_aFont == static_cast<const SfxWatermarkItem&>(rCmp).m_aFont &&
             m_nAngle == static_cast<const SfxWatermarkItem&>(rCmp).m_nAngle &&
             m_nTransparency == static_cast<const SfxWatermarkItem&>(rCmp).m_nTransparency &&
             m_nColor == static_cast<const SfxWatermarkItem&>(rCmp).m_nColor );
}

SfxPoolItem* SfxWatermarkItem::Clone( SfxItemPool *) const
{
    return new SfxWatermarkItem(*this);
}

bool SfxWatermarkItem::QueryValue( css::uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    rVal <<= m_aText;
    rVal <<= m_aFont;
    rVal <<= m_nAngle;
    rVal <<= m_nTransparency;
    rVal <<= m_nColor;

    return true;
}

bool SfxWatermarkItem::PutValue( const css::uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    OUString aText;

    if ( rVal >>= aText )
    {
        m_aText = aText;
        rVal >>= m_aFont;
        rVal >>= m_nAngle;
        rVal >>= m_nTransparency;
        rVal >>= m_nColor;
        return true;
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
