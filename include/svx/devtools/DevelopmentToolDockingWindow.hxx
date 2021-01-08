/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <svx/svxdllapi.h>
#include <sfx2/dockwin.hxx>
#include <vcl/customweld.hxx>
#include <vcl/weld.hxx>

class SAL_WARN_UNUSED SVX_DLLPUBLIC DevelopmentToolChildWindow final : public SfxChildWindow
{
    SFX_DECL_CHILDWINDOW_WITHID(DevelopmentToolChildWindow);

    DevelopmentToolChildWindow(vcl::Window* pParentWindow, sal_uInt16 nId, SfxBindings* pBindings,
                               SfxChildWinInfo* pInfo);
    virtual ~DevelopmentToolChildWindow() override;
};

class SAL_WARN_UNUSED SVX_DLLPUBLIC DevelopmentToolDockingWindow final : public SfxDockingWindow
{
public:
    DevelopmentToolDockingWindow(SfxBindings* pBindings, SfxChildWindow* pChildWindow,
                                 vcl::Window* pParent);
    virtual ~DevelopmentToolDockingWindow() override;

    virtual void dispose() override;

    virtual void EndDocking(const tools::Rectangle& rReactangle, bool bFloatMode) override;
    virtual void ToggleFloatingMode() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
