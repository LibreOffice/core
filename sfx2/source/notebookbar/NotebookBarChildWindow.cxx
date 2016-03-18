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

#include <sfx2/bindings.hxx>
#include <sfx2/notebookbar/NotebookBarChildWindow.hxx>
#include <sfx2/notebookbar/NotebookBarWindow.hxx>
#include <sfx2/sfxsids.hrc>
#include "helpid.hrc"
#include <sfx2/dockwin.hxx>
#include <sfx2/dispatch.hxx>

namespace sfx2 { namespace notebookbar {

SFX_IMPL_DOCKINGWINDOW_WITHID(NotebookBarChildWindow, SID_NOTEBOOKBAR);

NotebookBarChildWindow::NotebookBarChildWindow (vcl::Window* pParentWindow, sal_uInt16 nId,
                                        SfxBindings* pBindings, SfxChildWinInfo*)
    : SfxChildWindow(pParentWindow, nId)
{
    VclPtr<NotebookBarWindow> pNotebookBarWindow = VclPtr<NotebookBarWindow>::Create(
                                                pParentWindow,
                                                *this,
                                                pBindings,
                                                "NotebookBar",
                                                "vcl/ui/notebookbar.ui");

    SetWindow(pNotebookBarWindow);
    SetAlignment(SfxChildAlignment::TOP);
    GetWindow()->SetHelpId(HID_NOTEBOOKBAR_WINDOW);
    GetWindow()->Show();
}

} } // end of namespace sfx2::notebookbar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
