/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SVL_GRABBAGITEM_HXX
#define INCLUDED_SVL_GRABBAGITEM_HXX

#include <map>

#include <svl/svldllapi.h>
#include <svl/poolitem.hxx>
#include <com/sun/star/uno/Any.hxx>

/// Grab bag item provides a string-any map for keys with untyped values.
class SVL_DLLPUBLIC SfxGrabBagItem final : public SfxPoolItem
{
private:
    std::map<OUString, css::uno::Any> m_aMap;

public:
    DECLARE_ITEM_TYPE_FUNCTION(SfxGrabBagItem)
    SfxGrabBagItem();
    SfxGrabBagItem(sal_uInt16 nWhich);
    SfxGrabBagItem(sal_uInt16 nWhich, std::map<OUString, css::uno::Any> aMap);
    ~SfxGrabBagItem() override;

    SfxGrabBagItem(SfxGrabBagItem const&) = default;
    SfxGrabBagItem(SfxGrabBagItem&&) = default;
    SfxGrabBagItem& operator=(SfxGrabBagItem const&) = delete; // due to SfxPoolItem
    SfxGrabBagItem& operator=(SfxGrabBagItem&&) = delete; // due to SfxPoolItem

    const std::map<OUString, css::uno::Any>& GetGrabBag() const { return m_aMap; }

    bool operator==(const SfxPoolItem& rItem) const override;
    virtual bool supportsHashCode() const override { return true; }
    virtual size_t hashCode() const override;
    SfxGrabBagItem* Clone(SfxItemPool* pPool = nullptr) const override;

    bool PutValue(const css::uno::Any& rVal, sal_uInt8 nMemberId) override;
    bool QueryValue(css::uno::Any& rVal, sal_uInt8 nMemberId = 0) const override;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
