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

/// Grab bag item provides a string-any map for interim interop purposes.
class SVL_DLLPUBLIC SfxGrabBagItem : public SfxPoolItem
{
private:
    std::map<OUString, css::uno::Any> m_aMap;

public:

    SfxGrabBagItem();
    SfxGrabBagItem(sal_uInt16 nWhich, const std::map<OUString, css::uno::Any>* pMap = nullptr);
    SfxGrabBagItem(const SfxGrabBagItem& rItem);
    virtual ~SfxGrabBagItem();

    const std::map<OUString, css::uno::Any>& GetGrabBag() const
    {
        return m_aMap;
    }

    std::map<OUString, css::uno::Any>& GetGrabBag()
    {
        return m_aMap;
    }

    virtual bool operator==(const SfxPoolItem&) const override;
    virtual SfxPoolItem* Clone(SfxItemPool* pPool = nullptr) const override;

    virtual bool PutValue(const css::uno::Any& rVal, sal_uInt8 nMemberId) override;
    virtual bool QueryValue(css::uno::Any& rVal, sal_uInt8 nMemberId = 0) const override;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
