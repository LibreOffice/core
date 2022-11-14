/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sfx2/stbitem.hxx>
#include <vcl/image.hxx>

namespace sw
{
class AccessibilityStatusBarControl final : public SfxStatusBarControl
{
    sal_Int32 mnIssues;
    Image maImageIssuesFound;
    Image maImageIssuesNotFound;

public:
    SFX_DECL_STATUSBAR_CONTROL();

    AccessibilityStatusBarControl(sal_uInt16 nSlotId, sal_uInt16 nId, StatusBar& rStb);
    virtual ~AccessibilityStatusBarControl() override;

    void StateChangedAtStatusBarControl(sal_uInt16 nSID, SfxItemState eState,
                                        const SfxPoolItem* pState) override;
    void Paint(const UserDrawEvent& rEvent) override;
};

} // end sw

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
