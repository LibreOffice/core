/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
