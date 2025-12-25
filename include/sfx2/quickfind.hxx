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

#include <sfx2/childwin.hxx>
#include <sfx2/dockwin.hxx>

class SFX2_DLLPUBLIC SfxQuickFindWrapper : public SfxChildWindow
{
protected:
    void Initialize();

public:
    SfxQuickFindWrapper(vcl::Window* pParent, sal_uInt16 nId);
};

class SFX2_DLLPUBLIC SfxQuickFind : public SfxDockingWindow
{
public:
    SfxQuickFind(SfxBindings* pBindings, SfxChildWindow* pChildWin, vcl::Window* pParent,
                 SfxChildWinInfo* pInfo);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
