/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <svl/stritem.hxx>
#include <svx/svxdllapi.h>

enum class StatusCategory : sal_Int16
{
    NONE,
    TableCell,
    Section,
    TableOfContents,
    Numbering,
    ListStyle,
    Formula,
    RowColumn
};

class SVX_DLLPUBLIC SvxStatusItem final : public SfxStringItem
{
private:
    StatusCategory m_eCategory;

public:
    DECLARE_ITEM_TYPE_FUNCTION(SvxStatusItem)
    SvxStatusItem(TypedWhichId<SvxStatusItem> nWhich, const OUString& rString,
                  StatusCategory eStatusCategory);

    StatusCategory GetCategory() const { return m_eCategory; }

    virtual bool QueryValue(css::uno::Any& rVal, sal_uInt8 nMemberId = 0) const override;
    virtual bool PutValue(const css::uno::Any& rVal, sal_uInt8 nMemberId) override;

    virtual bool operator==(const SfxPoolItem& rItem) const override;
    virtual SvxStatusItem* Clone(SfxItemPool* pPool = nullptr) const override;

    static SfxPoolItem* CreateDefault();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
