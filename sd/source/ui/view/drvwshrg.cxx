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

#include "DrawViewShell.hxx"
#include <sfx2/request.hxx>
#include <sfx2/templdlg.hxx>
#include <sfx2/infobar.hxx>

#include <svx/fontwork.hxx>
#include <svx/bmpmask.hxx>
#include <svx/imapdlg.hxx>
#include <svx/SvxColorChildWindow.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/sidebar/SidebarChildWindow.hxx>
#include <svx/f3dchild.hxx>

#include <svx/svxids.hrc>
#include <svx/hyperdlg.hxx>
#include <avmedia/mediaplayer.hxx>

#include "app.hrc"
#include "strings.hrc"

#include "strings.hrc"
#include "SpellDialogChildWindow.hxx"
#include "sdresid.hxx"
#include "DrawDocShell.hxx"
#include "GraphicDocShell.hxx"
#include "GraphicViewShell.hxx"
#include "AnimationChildWindow.hxx"
#include "NavigatorChildWindow.hxx"

using namespace sd;
#define DrawViewShell
#include "sdslots.hxx"
#define GraphicViewShell
#include "sdgslots.hxx"

namespace sd {

/**
 * Declare SFX-Slotmap and Standardinterface
 */

SFX_IMPL_INTERFACE(DrawViewShell, SfxShell)

void DrawViewShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterPopupMenu("drawtext");

    GetStaticInterface()->RegisterChildWindow(SID_NAVIGATOR, true);

    GetStaticInterface()->RegisterChildWindow(SfxInfoBarContainerChild::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(SvxFontWorkChildWindow::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(SvxColorChildWindow::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(AnimationChildWindow::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(Svx3DChildWindow::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(SvxBmpMaskChildWindow::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(SvxIMapDlgChildWindow::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(SvxHlinkDlgWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(::sd::SpellDialogChildWindow::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(SID_SEARCH_DLG);
    GetStaticInterface()->RegisterChildWindow(::avmedia::MediaPlayer::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(::sfx2::sidebar::SidebarChildWindow::GetChildWindowId());
}


// SdGraphicViewShell
SFX_IMPL_INTERFACE(GraphicViewShell, SfxShell)

void GraphicViewShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterPopupMenu("drawtext");

    GetStaticInterface()->RegisterChildWindow(SID_NAVIGATOR, true);

    GetStaticInterface()->RegisterChildWindow(SfxInfoBarContainerChild::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(SvxFontWorkChildWindow::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(SvxColorChildWindow::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(Svx3DChildWindow::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(SvxBmpMaskChildWindow::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(SvxIMapDlgChildWindow::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(SvxHlinkDlgWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(::sd::SpellDialogChildWindow::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(SID_SEARCH_DLG);
    GetStaticInterface()->RegisterChildWindow(::avmedia::MediaPlayer::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(::sfx2::sidebar::SidebarChildWindow::GetChildWindowId());
}


} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
