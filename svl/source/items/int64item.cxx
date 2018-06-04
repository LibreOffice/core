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
#include <o3tl/make_unique.hxx>

SfxInt64Item::SfxInt64Item( sal_uInt16 nWhich, sal_Int64 nVal ) :
    SfxPoolItem(nWhich), mnValue(nVal)
{
}

SfxInt64Item::SfxInt64Item( sal_uInt16 nWhich, SvStream& rStream ) :
    SfxPoolItem(nWhich), mnValue(0)
{
    rStream.ReadInt64(mnValue);
}

SfxInt64Item::~SfxInt64Item() {}

bool SfxInt64Item::operator== ( const SfxPoolItem& rItem ) const
{
    return mnValue == static_cast<const SfxInt64Item&>(rItem).mnValue;
}

bool SfxInt64Item::GetPresentation(
    SfxItemPresentation, MapUnit, MapUnit, OUString& rText,
    const IntlWrapper& /*rIntlWrapper*/ ) const
{
    rText = OUString::number(mnValue);
    return true;
}

bool SfxInt64Item::QueryValue(
    css::uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    rVal <<= mnValue;
    return true;
}

bool SfxInt64Item::PutValue(
    const css::uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    sal_Int64 nVal;

    if (rVal >>= nVal)
    {
        mnValue = nVal;
        return true;
    }

    return false;
}

std::unique_ptr<SfxPoolItem> SfxInt64Item::CreateInternal( SvStream& rStream, sal_uInt16 /*nItemVersion*/ ) const
{
    return o3tl::make_unique<SfxInt64Item>(Which(), rStream);
}

SvStream& SfxInt64Item::Store( SvStream& rStream, sal_uInt16 /*nItemVersion*/ ) const
{
    return rStream.WriteInt64(mnValue);
}

std::unique_ptr<SfxPoolItem> SfxInt64Item::CloneInternal( SfxItemPool* /*pOther*/ ) const
{
    return std::unique_ptr<SfxPoolItem>(new SfxInt64Item(*this));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
