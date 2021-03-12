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

#include <sfx2/dllapi.h>
#include <sfx2/childwin.hxx>

/**
 * Necessary child window for the development tools docking window
 */
class SAL_WARN_UNUSED SFX2_DLLPUBLIC DevelopmentToolChildWindow final : public SfxChildWindow
{
    SFX_DECL_CHILDWINDOW_WITHID(DevelopmentToolChildWindow);

    DevelopmentToolChildWindow(vcl::Window* pParentWindow, sal_uInt16 nId, SfxBindings* pBindings,
                               SfxChildWinInfo* pInfo);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
