/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <SheetViewControl.hxx>
#include <SheetViewBox.hxx>
#include <svl/intitem.hxx>
#include <vcl/toolbox.hxx>

using namespace sc;

SFX_IMPL_TOOLBOX_CONTROL(SheetViewControl, SfxInt32Item);

SheetViewControl::SheetViewControl(sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx)
    : SfxToolBoxControl(nSlotId, nId, rTbx)
{
}

SheetViewControl::~SheetViewControl() {}

void SheetViewControl::StateChangedAtToolBoxControl(sal_uInt16, SfxItemState eState,
                                                    const SfxPoolItem* pState)
{
    ToolBoxItemId nId = GetId();
    ToolBox& rToolBox = GetToolBox();
    SheetViewBox* pSheetViewBox = static_cast<SheetViewBox*>(rToolBox.GetItemWindow(nId));

    if (SfxItemState::DISABLED == eState)
        pSheetViewBox->Disable();
    else
        pSheetViewBox->Enable();

    rToolBox.EnableItem(nId, SfxItemState::DISABLED != eState);

    switch (eState)
    {
        case SfxItemState::DEFAULT:
        {
            const auto* pItem = static_cast<const SfxInt32Item*>(pState);
            sc::SheetViewID nValue = pItem->GetValue();
            pSheetViewBox->Update(nValue);
            break;
        }

        default:
            break;
    }
}

VclPtr<InterimItemWindow> SheetViewControl::CreateItemWindow(vcl::Window* pParent)
{
    VclPtr<SheetViewBox> pControl = VclPtr<SheetViewBox>::Create(pParent);
    pControl->Show();

    return pControl;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
