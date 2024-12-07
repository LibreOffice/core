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
#include <condformatdlgdata.hxx>

ScCondFormatDlgData::ScCondFormatDlgData(std::shared_ptr<ScConditionalFormatList> pCondFormats,
        sal_uInt32 nItem, bool bManaged):
    mpCondFormats(std::move(pCondFormats)),
    mnItem(nItem),
    meDialogType(condformat::dialog::CONDITION),
    mbManaged(bManaged)
{
}

bool ScCondFormatDlgData::IsManaged() const
{
    return mbManaged;
}

condformat::dialog::ScCondFormatDialogType ScCondFormatDlgData::GetDialogType() const
{
    return meDialogType;
}

sal_uInt32 ScCondFormatDlgData::GetIndex() const
{
    return mnItem;
}

ScConditionalFormatList* ScCondFormatDlgData::GetConditionalFormatList()
{
    return mpCondFormats.get();
}

void ScCondFormatDlgData::SetDialogType(condformat::dialog::ScCondFormatDialogType eType)
{
    meDialogType = eType;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
