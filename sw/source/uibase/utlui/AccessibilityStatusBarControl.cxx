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
#include <officecfg/Office/Common.hxx>
#include <bitmaps.hlst>

SFX_IMPL_STATUSBAR_CONTROL(sw::AccessibilityStatusBarControl, SfxInt32Item);

namespace sw
{
AccessibilityStatusBarControl::AccessibilityStatusBarControl(sal_uInt16 _nSlotId, sal_uInt16 _nId,
                                                             StatusBar& rStb)
    : SfxStatusBarControl(_nSlotId, _nId, rStb)
    , mnIssues(0)
    , maImageIssuesFound(Image(StockImage::Yes, RID_BMP_A11Y_CHECK_ISSUES_FOUND))
    , maImageIssuesNotFound(Image(StockImage::Yes, RID_BMP_A11Y_CHECK_ISSUES_NOT_FOUND))
{
}

AccessibilityStatusBarControl::~AccessibilityStatusBarControl() = default;

void AccessibilityStatusBarControl::StateChangedAtStatusBarControl(sal_uInt16 /*nSID*/,
                                                                   SfxItemState eState,
                                                                   const SfxPoolItem* pState)
{
    mnIssues = -1;

    bool bOnlineCheckStatus
        = officecfg::Office::Common::Accessibility::OnlineAccessibilityCheck::get();

    if (eState == SfxItemState::DEFAULT && bOnlineCheckStatus)
    {
        if (auto pItem = dynamic_cast<const SfxInt32Item*>(pState))
            mnIssues = pItem->GetValue();
        OUString aString = SwResId(STR_ACCESSIBILITY_CHECK_HINT)
                               .replaceFirst("%issues%", OUString::number(mnIssues));
        GetStatusBar().SetQuickHelpText(GetId(), aString);
    }
    else
    {
        GetStatusBar().SetQuickHelpText(GetId(), u"");
    }

    GetStatusBar().Invalidate();
}

void AccessibilityStatusBarControl::Paint(const UserDrawEvent& rUserEvent)
{
    if (mnIssues < 0)
        return;

    vcl::RenderContext* pRenderContext = rUserEvent.GetRenderContext();

    tools::Rectangle aRect = rUserEvent.GetRect();
    const tools::Rectangle aControlRect = getControlRect();

    Image aImage = mnIssues > 0 ? maImageIssuesFound : maImageIssuesNotFound;

    Size aSize(aImage.GetSizePixel());

    auto aPosition = Point(aRect.Left() + (aControlRect.GetWidth() - aSize.Width()) / 2,
                           aRect.Top() + (aControlRect.GetHeight() - aSize.Height()) / 2);

    pRenderContext->DrawImage(aPosition, aImage);
}

} // end sw

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
