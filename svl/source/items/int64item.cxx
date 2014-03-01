/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svl/int64item.hxx>
#include <tools/stream.hxx>

SfxInt64Item::SfxInt64Item( sal_uInt16 nWhich, sal_Int64 nVal ) :
    SfxPoolItem(nWhich), mnValue(nVal)
{
}

SfxInt64Item::SfxInt64Item( sal_uInt16 nWhich, SvStream& rStream ) :
    SfxPoolItem(nWhich)
{
    rStream.ReadInt64(mnValue);
}

SfxInt64Item::SfxInt64Item( const SfxInt64Item& rItem ) :
    SfxPoolItem(rItem), mnValue(rItem.mnValue)
{
}

SfxInt64Item::~SfxInt64Item() {}

bool SfxInt64Item::operator== ( const SfxPoolItem& rItem ) const
{
    return mnValue == static_cast<const SfxInt64Item&>(rItem).mnValue;
}

int SfxInt64Item::Compare( const SfxPoolItem& r ) const
{
    sal_Int64 nOther = static_cast<const SfxInt64Item&>(r).mnValue;

    if (mnValue < nOther)
        return -1;

    if (mnValue > nOther)
        return 1;

    return 0;
}

int SfxInt64Item::Compare( const SfxPoolItem& r, const IntlWrapper& /*rIntlWrapper*/ ) const
{
    return Compare(r);
}

SfxItemPresentation SfxInt64Item::GetPresentation(
    SfxItemPresentation, SfxMapUnit, SfxMapUnit, OUString& rText,
    const IntlWrapper* /*pIntlWrapper*/ ) const
{
    rText = OUString::number(mnValue);
    return SFX_ITEM_PRESENTATION_NAMELESS;
}

bool SfxInt64Item::QueryValue(
    com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    rVal <<= mnValue;
    return true;
}

bool SfxInt64Item::PutValue(
    const com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    sal_Int64 nVal;

    if (rVal >>= nVal)
    {
        mnValue = nVal;
        return true;
    }

    return false;
}

SfxPoolItem* SfxInt64Item::Create( SvStream& rStream, sal_uInt16 /*nItemVersion*/ ) const
{
    return new SfxInt64Item(Which(), rStream);
}

SvStream& SfxInt64Item::Store( SvStream& rStream, sal_uInt16 /*nItemVersion*/ ) const
{
    return rStream.WriteInt64(mnValue);
}

SfxPoolItem* SfxInt64Item::Clone( SfxItemPool* /*pOther*/ ) const
{
    return new SfxInt64Item(*this);
}

sal_Int64 SfxInt64Item::GetValue() const
{
    return mnValue;
}

void SfxInt64Item::SetValue( sal_Int64 nVal )
{
    mnValue = nVal;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
