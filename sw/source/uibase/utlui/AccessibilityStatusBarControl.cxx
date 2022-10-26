/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swtypes.hxx>
#include <strings.hrc>
#include <AccessibilityStatusBarControl.hxx>
#include <svl/intitem.hxx>
#include <vcl/status.hxx>
#include <vcl/event.hxx>

SFX_IMPL_STATUSBAR_CONTROL(sw::AccessibilityStatusBarControl, SfxInt32Item);

namespace sw
{
AccessibilityStatusBarControl::AccessibilityStatusBarControl(sal_uInt16 _nSlotId, sal_uInt16 _nId,
                                                             StatusBar& rStb)
    : SfxStatusBarControl(_nSlotId, _nId, rStb)
    , mnIssues(0)
{
}

AccessibilityStatusBarControl::~AccessibilityStatusBarControl() = default;

void AccessibilityStatusBarControl::StateChangedAtStatusBarControl(sal_uInt16 /*nSID*/,
                                                                   SfxItemState eState,
                                                                   const SfxPoolItem* pState)
{
    if (eState != SfxItemState::DEFAULT)
    {
        mnIssues = -1;
    }
    else if (auto pItem = dynamic_cast<const SfxInt32Item*>(pState))
    {
        mnIssues = pItem->GetValue();
    }
    else
    {
        mnIssues = -1;
    }

    GetStatusBar().SetItemData(GetId(), nullptr); // necessary ?
    GetStatusBar().SetItemText(GetId(), ""); // necessary ?

    if (eState == SfxItemState::DEFAULT) // Can access pState
    {
        GetStatusBar().SetQuickHelpText(GetId(), SwResId(STR_ACCESSIBILITY_CHECK_HINT));
    }
    else
    {
        GetStatusBar().SetQuickHelpText(GetId(), u"");
    }
}

void AccessibilityStatusBarControl::Paint(const UserDrawEvent& rUserEvent)
{
    vcl::RenderContext* pRenderContext = rUserEvent.GetRenderContext();

    tools::Rectangle aRect = rUserEvent.GetRect();
    Color aOldLineColor = pRenderContext->GetLineColor();
    Color aOldFillColor = pRenderContext->GetFillColor();

    if (mnIssues > 0)
        pRenderContext->SetFillColor(COL_RED);
    else
        pRenderContext->SetFillColor(COL_GREEN);

    pRenderContext->DrawRect(aRect);

    pRenderContext->SetLineColor(aOldLineColor);
    pRenderContext->SetFillColor(aOldFillColor);
}

} // end sw

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
