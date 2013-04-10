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



#include "precompiled_sd.hxx"
#include "PaneShells.hxx"

#include "PaneChildWindows.hxx"

#include "glob.hrc"
#include "sdresid.hxx"

#include <sfx2/msg.hxx>
#include <sfx2/objface.hxx>

namespace sd {

//===== LeftImpressPaneShell ==================================================

#define ShellClass LeftImpressPaneShell

SFX_SLOTMAP(LeftImpressPaneShell)
{
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

SFX_IMPL_INTERFACE(LeftImpressPaneShell, SfxShell, SdResId(STR_LEFT_IMPRESS_PANE_SHELL))
{
    SFX_CHILDWINDOW_REGISTRATION(
        ::sd::LeftPaneImpressChildWindow::GetChildWindowId());
}

TYPEINIT1(LeftImpressPaneShell, SfxShell);



LeftImpressPaneShell::LeftImpressPaneShell (void)
    : SfxShell()
{
    SetName(rtl::OUString::createFromAscii("LeftImpressPane"));
}




LeftImpressPaneShell::~LeftImpressPaneShell (void)
{
}




//===== LeftDrawPaneShell =====================================================

#undef ShellClass
#define ShellClass LeftDrawPaneShell

SFX_SLOTMAP(LeftDrawPaneShell)
{
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

SFX_IMPL_INTERFACE(LeftDrawPaneShell, SfxShell, SdResId(STR_LEFT_DRAW_PANE_SHELL))
{
    SFX_CHILDWINDOW_REGISTRATION(
        ::sd::LeftPaneDrawChildWindow::GetChildWindowId());
}

TYPEINIT1(LeftDrawPaneShell, SfxShell);



LeftDrawPaneShell::LeftDrawPaneShell (void)
    : SfxShell()
{
    SetName(rtl::OUString::createFromAscii("LeftDrawPane"));
}




LeftDrawPaneShell::~LeftDrawPaneShell (void)
{
}


} // end of namespace ::sd
