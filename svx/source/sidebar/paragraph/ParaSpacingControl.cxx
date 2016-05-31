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

#include "ParaSpacingWindow.hxx"

#include <editeng/ulspitem.hxx>
#include <editeng/lrspitem.hxx>
#include <svx/ParaSpacingControl.hxx>
#include <vcl/toolbox.hxx>
#include <svl/itempool.hxx>

using namespace svx;

SFX_IMPL_TOOLBOX_CONTROL(ParaULSpacingControl, SvxULSpaceItem);
SFX_IMPL_TOOLBOX_CONTROL(ParaLRSpacingControl, SvxLRSpaceItem);

ParaULSpacingControl::ParaULSpacingControl(sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx)
    : SfxToolBoxControl(nSlotId, nId, rTbx)
{
}

ParaULSpacingControl::~ParaULSpacingControl()
{
}

void ParaULSpacingControl::StateChanged(sal_uInt16 /*nSID*/, SfxItemState eState,
                              const SfxPoolItem* pState)
{
    sal_uInt16 nId = GetId();
    ToolBox& rTbx = GetToolBox();
    ParaULSpacingWindow* pWindow = static_cast<ParaULSpacingWindow*>(rTbx.GetItemWindow(nId));

    DBG_ASSERT( pWindow, "Control not found!" );

    if(SfxItemState::DISABLED == eState)
        pWindow->Disable();
    else
        pWindow->Enable();

    rTbx.EnableItem(nId, SfxItemState::DISABLED != eState);

    if(pState)
        pWindow->SetValue(static_cast<const SvxULSpaceItem*>(pState));
}

VclPtr<vcl::Window> ParaULSpacingControl::CreateItemWindow(vcl::Window* pParent)
{
    VclPtr<ParaULSpacingWindow> pWindow = VclPtr<ParaULSpacingWindow>::Create(pParent, m_xFrame);
    pWindow->Show();

    return pWindow;
}

ParaLRSpacingControl::ParaLRSpacingControl(sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx)
    : SfxToolBoxControl(nSlotId, nId, rTbx)
{
}

ParaLRSpacingControl::~ParaLRSpacingControl()
{
}

void ParaLRSpacingControl::StateChanged(sal_uInt16 /*nSID*/, SfxItemState eState,
                              const SfxPoolItem* pState)
{
    sal_uInt16 nId = GetId();
    ToolBox& rTbx = GetToolBox();
    ParaLRSpacingWindow* pWindow = static_cast<ParaLRSpacingWindow*>(rTbx.GetItemWindow(nId));

    DBG_ASSERT( pWindow, "Control not found!" );

    pWindow->Initialize();
    pWindow->SetValue(eState, pState);
}

VclPtr<vcl::Window> ParaLRSpacingControl::CreateItemWindow(vcl::Window* pParent)
{
    VclPtr<ParaLRSpacingWindow> pWindow = VclPtr<ParaLRSpacingWindow>::Create(pParent, m_xFrame);
    pWindow->Show();

    return pWindow;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
