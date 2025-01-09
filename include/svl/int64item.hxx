/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVL_INT64ITEM_HXX
#define INCLUDED_SVL_INT64ITEM_HXX

#include <svl/poolitem.hxx>
#include <svl/svldllapi.h>

class SVL_DLLPUBLIC SfxInt64Item final : public SfxPoolItem
{
    sal_Int64 mnValue;

public:
    DECLARE_ITEM_TYPE_FUNCTION(SfxInt64Item)
    SfxInt64Item( sal_uInt16 nWhich, sal_Int64 nVal );

    virtual ~SfxInt64Item() override;

    SfxInt64Item(SfxInt64Item const &) = default;
    SfxInt64Item(SfxInt64Item &&) = default;
    SfxInt64Item & operator =(SfxInt64Item const &) = delete; // due to SfxPoolItem;
    SfxInt64Item & operator =(SfxInt64Item &&) = delete; // due to SfxPoolItem

    virtual bool operator== ( const SfxPoolItem& rItem ) const override;

    virtual bool GetPresentation(
        SfxItemPresentation, MapUnit, MapUnit,
        OUString& rText, const IntlWrapper& rIntlWrapper ) const override;

    virtual bool QueryValue(
        css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;

    virtual bool PutValue(
        const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    virtual SfxInt64Item* Clone( SfxItemPool* pOther = nullptr ) const override;

    sal_Int64 GetValue() const { return mnValue;}

};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
