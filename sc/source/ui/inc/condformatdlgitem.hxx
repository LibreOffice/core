/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <svl/poolitem.hxx>

#include <memory>

namespace condformat::dialog
{
enum ScCondFormatDialogType
{
    NONE,
    CONDITION,
    COLORSCALE,
    DATABAR,
    ICONSET,
    DATE
};
}

class ScConditionalFormatList;

class ScCondFormatDlgItem : public SfxPoolItem
{
public:
    ScCondFormatDlgItem(std::shared_ptr<ScConditionalFormatList> pCondFormats, sal_Int32 nItem,
                        bool bManaged);

    virtual ~ScCondFormatDlgItem() override;

    ScCondFormatDlgItem(ScCondFormatDlgItem const&) = default;
    ScCondFormatDlgItem(ScCondFormatDlgItem&&) = default;
    ScCondFormatDlgItem& operator=(ScCondFormatDlgItem const&) = delete; // due to SfxPoolItem
    ScCondFormatDlgItem& operator=(ScCondFormatDlgItem&&) = delete; // due to SfxPoolItem

    virtual bool operator==(const SfxPoolItem&) const override;
    virtual ScCondFormatDlgItem* Clone(SfxItemPool* pPool = nullptr) const override;

    bool IsManaged() const;
    condformat::dialog::ScCondFormatDialogType GetDialogType() const;
    sal_Int32 GetIndex() const;

    void SetDialogType(condformat::dialog::ScCondFormatDialogType eType);

    ScConditionalFormatList* GetConditionalFormatList();

private:
    std::shared_ptr<ScConditionalFormatList> mpCondFormats;
    sal_Int32 mnItem;
    condformat::dialog::ScCondFormatDialogType meDialogType;
    bool mbManaged;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
