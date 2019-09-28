/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/docfile.hxx>
#include <sfx2/frame.hxx>
#include <vcl/weld.hxx>
#include <vcl/window.hxx>

#include <dialoghelp.hxx>
#include <doc.hxx>
#include <docsh.hxx>
#include <view.hxx>

weld::Window* GetFrameWeld(const SfxFrame* pFrame)
{
    return pFrame ? pFrame->GetWindow().GetFrameWeld() : nullptr;
}

weld::Window* GetFrameWeld(const SfxMedium* pMedium)
{
    return GetFrameWeld(pMedium ? pMedium->GetLoadTargetFrame() : nullptr);
}

weld::Window* GetFrameWeld(SwDocShell* pDocShell)
{
    if (!pDocShell)
        return nullptr;
    weld::Window* pRet = GetFrameWeld(pDocShell->GetMedium());
    if (!pRet)
    {
        if (SwView* pView = pDocShell->GetView())
            pRet = pView->GetFrameWeld();
    }
    return pRet;
}

weld::Window* GetFrameWeld(SwDoc* pDoc)
{
    return GetFrameWeld(pDoc ? pDoc->GetDocShell() : nullptr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
