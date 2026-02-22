/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sfx2/tbxctrl.hxx>

namespace sc
{
/** Toolbox control for showing the Table Styles in the toolbar / notebookbar */
class TableStylesControl final : public SfxToolBoxControl
{
public:
    SFX_DECL_TOOLBOX_CONTROL();

    TableStylesControl(sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rToolBox);
    virtual ~TableStylesControl() override;

    virtual void StateChangedAtToolBoxControl(sal_uInt16 nSID, SfxItemState eState,
                                              const SfxPoolItem* pState) override;
    virtual VclPtr<InterimItemWindow> CreateItemWindow(vcl::Window* pParent) override;
};

} // end of namespace sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
