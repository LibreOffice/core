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

namespace basctl
{
class LineStatusControl final : public SfxStatusBarControl
{
public:
    SFX_DECL_STATUSBAR_CONTROL();

    LineStatusControl(sal_uInt16 nSlotId, sal_uInt16 nId, StatusBar& rStb);
    virtual ~LineStatusControl() override;

    virtual void StateChangedAtStatusBarControl(sal_uInt16 nSID, SfxItemState eState,
                                                const SfxPoolItem* pState) override;
};

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
