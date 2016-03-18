/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/bindings.hxx>
#include <sfx2/notebookbar/NotebookBarChildWindow.hxx>
#include <sfx2/sfxsids.hrc>
#include "helpid.hrc"
#include <sfx2/dockwin.hxx>
#include <sfx2/dispatch.hxx>

using namespace sfx2;

SFX_IMPL_DOCKINGWINDOW_WITHID(NotebookBarChildWindow, SID_NOTEBOOKBAR);

NotebookBarChildWindow::NotebookBarChildWindow (vcl::Window* pParentWindow, sal_uInt16 nId,
                                                SfxBindings* pBindings, SfxChildWinInfo*)
    : SfxChildWindow(pParentWindow, nId)
{
    SfxFrame& rFrame = pBindings->GetDispatcher_Impl()->GetFrame()->GetFrame();
    VclPtr<vcl::Window> pNotebookBarWindow = rFrame.GetSystemWindow()->CreateNotebookBar("vcl/ui/notebookbar.ui", rFrame.GetFrameInterface());

    pNotebookBarWindow->SetHelpId(HID_NOTEBOOKBAR_WINDOW);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
