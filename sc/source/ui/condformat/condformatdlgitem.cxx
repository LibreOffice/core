/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#include <utility>

#include <scitems.hxx>
#include <condformatdlgitem.hxx>

ScCondFormatDlgItem::ScCondFormatDlgItem(std::shared_ptr<ScConditionalFormatList> pCondFormats,
        sal_Int32 nItem, bool bManaged):
    SfxPoolItem(SCITEM_CONDFORMATDLGDATA, SfxItemType::ScCondFormatDlgItemType),
    mpCondFormats(std::move(pCondFormats)),
    mnItem(nItem),
    meDialogType(condformat::dialog::CONDITION),
    mbManaged(bManaged)
{
}

ScCondFormatDlgItem::~ScCondFormatDlgItem()
{
}

bool ScCondFormatDlgItem::operator==(const SfxPoolItem& rItem) const
{
    assert(SfxPoolItem::operator==(rItem)); (void)rItem;
    return false;
}

ScCondFormatDlgItem* ScCondFormatDlgItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new ScCondFormatDlgItem(*this);
}

bool ScCondFormatDlgItem::IsManaged() const
{
    return mbManaged;
}

condformat::dialog::ScCondFormatDialogType ScCondFormatDlgItem::GetDialogType() const
{
    return meDialogType;
}

sal_Int32 ScCondFormatDlgItem::GetIndex() const
{
    return mnItem;
}

ScConditionalFormatList* ScCondFormatDlgItem::GetConditionalFormatList()
{
    return mpCondFormats.get();
}

void ScCondFormatDlgItem::SetDialogType(condformat::dialog::ScCondFormatDialogType eType)
{
    meDialogType = eType;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
