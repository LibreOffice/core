/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <sal/config.h>

#include <sfx2/bindings.hxx>
#include <sfx2/quickfind.hxx>
#include <sfx2/strings.hrc>
#include <sfx2/sfxresid.hxx>
#include <helpids.h>

SfxQuickFindWrapper::SfxQuickFindWrapper(vcl::Window* pParentWnd, sal_uInt16 nId)
    : SfxChildWindow(pParentWnd, nId)
{
}

void SfxQuickFindWrapper::Initialize() { SetHideNotDelete(true); }

SfxQuickFind::SfxQuickFind(SfxBindings* pBind, SfxChildWindow* pChildWin, vcl::Window* pParent,
                           SfxChildWinInfo* pInfo)
    : SfxDockingWindow(pBind, pChildWin, pParent, u"QuickFind"_ustr, u"sfx/ui/quickfind.ui"_ustr)
{
    SetText(SfxResId(STR_SID_QUICKFIND));
    SetHelpId(HID_QUICKFIND_WINDOW);
    SetOutputSizePixel(Size(270, 240));
    Initialize(pInfo);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
