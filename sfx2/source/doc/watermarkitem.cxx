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
#include <comphelper/propertysequence.hxx>

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
    rVal <<= comphelper::InitPropertySequence( {
        { "Text", css::uno::makeAny( m_aText ) },
        { "Font", css::uno::makeAny( m_aFont ) },
        { "Angle", css::uno::makeAny( m_nAngle ) },
        { "Transparency", css::uno::makeAny( m_nTransparency ) },
        { "Color", css::uno::makeAny( m_nColor ) },
    } );

    return true;
}

bool SfxWatermarkItem::PutValue( const css::uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    css::uno::Sequence<css::beans::PropertyValue> aSequence;

    if ( rVal >>= aSequence )
    {
        for(const auto& aEntry : aSequence)
        {
            if(aEntry.Name == "Text")
                aEntry.Value >>= m_aText;
            if(aEntry.Name == "Font")
                aEntry.Value >>= m_aFont;
            if(aEntry.Name == "Angle")
                aEntry.Value >>= m_nAngle;
            if(aEntry.Name == "Transparency")
                aEntry.Value >>= m_nTransparency;
            if(aEntry.Name == "Color")
                aEntry.Value >>= m_nColor;
        }
        return true;
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
