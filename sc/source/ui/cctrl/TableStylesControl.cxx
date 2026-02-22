/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <TableStylesControl.hxx>
#include <TableStylesBox.hxx>
#include <svl/intitem.hxx>
#include <vcl/toolbox.hxx>
#include <dbdata.hxx>

using namespace sc;

SFX_IMPL_TOOLBOX_CONTROL(TableStylesControl, ScDatabaseSettingItem);

TableStylesControl::TableStylesControl(sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx)
    : SfxToolBoxControl(nSlotId, nId, rTbx)
{
}

TableStylesControl::~TableStylesControl() {}

void TableStylesControl::StateChangedAtToolBoxControl(sal_uInt16, SfxItemState eState,
                                                      const SfxPoolItem* pState)
{
    ToolBoxItemId nId = GetId();
    ToolBox& rToolBox = GetToolBox();
    TableStylesBox* pTableStylesBox = static_cast<TableStylesBox*>(rToolBox.GetItemWindow(nId));

    if (SfxItemState::DISABLED == eState)
        pTableStylesBox->Disable();
    else
        pTableStylesBox->Enable();

    rToolBox.EnableItem(nId, SfxItemState::DISABLED != eState);

    switch (eState)
    {
        case SfxItemState::DEFAULT:
        {
            const ScDatabaseSettingItem* pDBItem
                = static_cast<const ScDatabaseSettingItem*>(pState);
            if (pDBItem)
                pTableStylesBox->Update(pDBItem);
            break;
        }

        default:
            break;
    }
}

VclPtr<InterimItemWindow> TableStylesControl::CreateItemWindow(vcl::Window* pParent)
{
    VclPtr<TableStylesBox> pControl = VclPtr<TableStylesBox>::Create(pParent);
    pControl->Show();

    return pControl;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
