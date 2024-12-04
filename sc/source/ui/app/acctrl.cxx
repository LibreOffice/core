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
#include <tools/debug.hxx>

#include <svl/eitem.hxx>
#include <vcl/event.hxx>
#include <vcl/image.hxx>
#include <vcl/status.hxx>

#include <acctrl.hxx>
#include <bitmaps.hlst>
#include <scresid.hxx>
#include <strings.hrc>

SFX_IMPL_STATUSBAR_CONTROL(ScAutoCalculateControl, SfxBoolItem);

ScAutoCalculateControl::ScAutoCalculateControl(sal_uInt16 _nSlotId, sal_uInt16 _nId,
                                               StatusBar& rStb)
    : SfxStatusBarControl(_nSlotId, _nId, rStb)
    , m_bIsActive(false)
{
}

ScAutoCalculateControl::~ScAutoCalculateControl() {}

void ScAutoCalculateControl::StateChangedAtStatusBarControl(sal_uInt16, SfxItemState eState,
                                                            const SfxPoolItem* pState)
{
    if (eState != SfxItemState::DEFAULT || SfxItemState::DISABLED == eState)
        return;

    auto pItem = static_cast<const SfxBoolItem*>(pState);
    if (!pItem)
    {
        SAL_WARN("sc", "Item wasn't a SfxBoolItem");
        return;
    }
    m_bIsActive = pItem->GetValue();

    GetStatusBar().SetQuickHelpText(GetId(), m_bIsActive ? "" : ScResId(STR_AUTOCALC_OFF));
    GetStatusBar().Invalidate();
}

void ScAutoCalculateControl::Paint(const UserDrawEvent& rUsrEvt)
{
    vcl::RenderContext* pDev = rUsrEvt.GetRenderContext();
    tools::Rectangle aRect(rUsrEvt.GetRect());
    pDev->Erase();

    if (!m_bIsActive)
    {
        const Image aImage(StockImage::Yes, RID_BMP_CALCULATOR_RED);

        Point aPt = aRect.TopLeft();
        aPt += Point((aRect.GetSize().getWidth() - aImage.GetSizePixel().getWidth()) / 2,
                     (aRect.GetSize().getHeight() - aImage.GetSizePixel().getHeight()) / 2);

        pDev->DrawImage(aPt, aImage);
    }
}

void ScAutoCalculateControl::Click()
{
    if (!m_bIsActive)
        SfxStatusBarControl::Click(); // exec FID_AUTO_CALC and toggle AutoCalc on
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
