/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "LayerDialogChildWindow.hxx"
#include <sfx2/dockwin.hxx>
#include "app.hrc"
#include <sfx2/app.hxx>

// Instantiate the implementation of the docking window before files
// are included that define ::sd::Window.  The ... macros are not really
// namespace proof.
namespace sd {
SFX_IMPL_DOCKINGWINDOW(LayerDialogChildWindow, SID_LAYER_DIALOG_WIN)
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
