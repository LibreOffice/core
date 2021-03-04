/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <scalectrl.hxx>

#include <vcl/commandevent.hxx>
#include <vcl/status.hxx>
#include <vcl/weldutils.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/stritem.hxx>
#include <sfx2/sfxsids.hrc>

#include <ViewShellBase.hxx>
#include <drawdoc.hxx>
#include <app.hrc>
#include <sdresid.hxx>
#include <strings.hrc>

SFX_IMPL_STATUSBAR_CONTROL(SdScaleControl, SfxStringItem);

// class SdScaleControl ------------------------------------------
SdScaleControl::SdScaleControl(sal_uInt16 _nSlotId, sal_uInt16 _nId, StatusBar& rStb)
    : SfxStatusBarControl(_nSlotId, _nId, rStb)
{
    GetStatusBar().SetQuickHelpText(GetId(), SdResId(STR_SCALE_TOOLTIP));
}

SdScaleControl::~SdScaleControl() {}

void SdScaleControl::StateChanged(sal_uInt16 /*nSID*/, SfxItemState eState,
                                  const SfxPoolItem* pState)
{
    if (eState != SfxItemState::DEFAULT || pState->IsVoidItem())
        return;
    auto pStringItem = dynamic_cast<const SfxStringItem*>(pState);
    GetStatusBar().SetItemText(GetId(), pStringItem->GetValue());
}

void SdScaleControl::Command(const CommandEvent& rCEvt)
{
    if (rCEvt.GetCommand() != CommandEventId::ContextMenu
        || GetStatusBar().GetItemText(GetId()).isEmpty())
        return;

    SfxViewFrame* pViewFrame = SfxViewFrame::Current();

    sd::ViewShellBase* pViewShellBase = sd::ViewShellBase::GetViewShellBase(pViewFrame);
    if (!pViewShellBase)
        return;

    SdDrawDocument* pDoc = pViewShellBase->GetDocument();
    if (!pDoc)
        return;

    std::unique_ptr<weld::Builder> xBuilder(
        Application::CreateBuilder(nullptr, "modules/simpress/ui/masterpagemenu.ui"));
    std::unique_ptr<weld::Menu> xPopup(xBuilder->weld_menu("menu"));

    sal_uInt16 aTable[12] = { 1, 2, 4, 5, 8, 10, 16, 20, 30, 40, 50, 100 };

    for (sal_uInt16 i = 11; i > 0; i--)
        xPopup->append(OUString::number(12 - i), OUString::number(aTable[i]) + ":1");
    for (sal_uInt16 i = 0; i < 12; i++)
        xPopup->append(OUString::number(12 + i), "1:" + OUString::number(aTable[i]));

    ::tools::Rectangle aRect(rCEvt.GetMousePosPixel(), Size(1, 1));
    weld::Window* pParent = weld::GetPopupParent(GetStatusBar(), aRect);
    OString sResult = xPopup->popup_at_rect(pParent, aRect);
    if (!sResult.isEmpty())
    {
        sal_Int32 i = sResult.toUInt32();
        sal_Int32 nX;
        sal_Int32 nY;
        if (i > 11)
            nX = 1;
        else
            nX = aTable[12 - i % 12];
        if (i > 11)
            nY = aTable[i % 12];
        else
            nY = 1;
        pDoc->SetUIScale(Fraction(nX, nY));

        SfxBindings& pBindings = pViewFrame->GetBindings();
        pBindings.Invalidate(SID_SCALE); //update statusbar
        pBindings.Invalidate(SID_ATTR_METRIC); //update sidebar
        pViewShellBase->UpdateBorder(true); // update ruler
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
