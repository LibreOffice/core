/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/notebookbar/SfxNotebookBar.hxx>
#include <unotools/viewoptions.hxx>
#include <vcl/notebookbar.hxx>
#include <vcl/syswin.hxx>

using namespace sfx2;

void SfxNotebookBar::ExecMethod(SfxBindings& rBindings)
{
    SvtViewOptions aViewOpt(E_WINDOW, "notebookbar");
    aViewOpt.SetVisible(!aViewOpt.IsVisible());

    // trigger the StateMethod
    rBindings.Invalidate(SID_NOTEBOOKBAR);
    rBindings.Update();
}

void SfxNotebookBar::StateMethod(SfxBindings& rBindings, const OUString& rUIFile)
{
    SfxFrame& rFrame = rBindings.GetDispatcher_Impl()->GetFrame()->GetFrame();

    SvtViewOptions aViewOpt(E_WINDOW, "notebookbar");

    if (aViewOpt.IsVisible())
    {
        // setup if necessary
        rFrame.GetSystemWindow()->SetNotebookBar(rUIFile, rFrame.GetFrameInterface());

        rFrame.GetSystemWindow()->GetNotebookBar()->Show();
    }
    else if (auto pNotebookBar = rFrame.GetSystemWindow()->GetNotebookBar())
        pNotebookBar->Hide();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
