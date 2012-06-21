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


#include "LayerDialogChildWindow.hxx"
#include <sfx2/dockwin.hxx>
#include "app.hrc"
#include <sfx2/app.hxx>

// Instantiate the implementation of the docking window before files
// are included that define ::sd::Window.  The ... macros are not really
// namespace proof.
namespace sd {
SFX_IMPL_DOCKINGWINDOW_WITHID(LayerDialogChildWindow, SID_LAYER_DIALOG_WIN)
}

#include "LayerDialog.hrc"
#include "LayerDialogContent.hxx"


#include "ViewShellBase.hxx"
#include "DrawViewShell.hxx"
#include <sfx2/dispatch.hxx>

namespace sd {


LayerDialogChildWindow::LayerDialogChildWindow (
    ::Window* _pParent,
    sal_uInt16 nId,
    SfxBindings* pBindings,
    SfxChildWinInfo* pInfo)
    : SfxChildWindow (_pParent, nId)
{
    ViewShellBase& rBase (*ViewShellBase::GetViewShellBase(
        pBindings->GetDispatcher()->GetFrame()));
    LayerDialogContent* pContent = new LayerDialogContent (
        pBindings,
        this,
        _pParent,
        SdResId( FLT_WIN_LAYER_DIALOG),
        rBase);
    pWindow = pContent;

    eChildAlignment = SFX_ALIGN_NOALIGNMENT;

    pContent->Initialize(pInfo);
}



LayerDialogChildWindow::~LayerDialogChildWindow (void)
{
}



} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
