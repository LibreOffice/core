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

#include <sfx2/objface.hxx>
#include <vcl/EnumContext.hxx>
#include <view.hxx>
#include <frmsh.hxx>
#include <olesh.hxx>

#include <sfx2/sidebar/SidebarController.hxx>

#define ShellClass_SwOleShell
#include <sfx2/msg.hxx>
#include <swslots.hxx>

using namespace css::uno;
using namespace sfx2::sidebar;

namespace {

bool inChartContext(SwView& rViewShell)
{
    SidebarController* pSidebar = SidebarController::GetSidebarControllerForView(&rViewShell);
    if (pSidebar)
        return pSidebar->hasChartContextCurrently();

    return false;
}

} // anonymous namespace

SFX_IMPL_INTERFACE(SwOleShell, SwFrameShell)

void SwOleShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterPopupMenu("oleobject");

    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_OBJECT, SfxVisibilityFlags::Invisible, ToolbarId::Ole_Toolbox);
}

void SwOleShell::Activate(bool bMDI)
{
    if(!inChartContext(GetView()))
        SwFrameShell::Activate(bMDI);
    else
    {
        // Avoid context changes for chart during activation / deactivation.
        const bool bIsContextBroadcasterEnabled (SfxShell::SetContextBroadcasterEnabled(false));

        SwFrameShell::Activate(bMDI);

        SfxShell::SetContextBroadcasterEnabled(bIsContextBroadcasterEnabled);
    }
}

void SwOleShell::Deactivate(bool bMDI)
{
    if(!inChartContext(GetView()))
        SwFrameShell::Deactivate(bMDI);
    else
    {
        // Avoid context changes for chart during activation / deactivation.
        const bool bIsContextBroadcasterEnabled (SfxShell::SetContextBroadcasterEnabled(false));

        SwFrameShell::Deactivate(bMDI);

        SfxShell::SetContextBroadcasterEnabled(bIsContextBroadcasterEnabled);
    }
}

SwOleShell::SwOleShell(SwView &_rView) :
    SwFrameShell(_rView)

{
    SetName("Object");
    SfxShell::SetContextName(vcl::EnumContext::GetContextName(vcl::EnumContext::Context::OLE));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
