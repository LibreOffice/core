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
#include <sfx2/templdlg.hxx>
#include <sfx2/infobar.hxx>

#include <svx/fontwork.hxx>
#include <svx/bmpmask.hxx>
#include <svx/galbrws.hxx>
#include <svx/imapdlg.hxx>
#include <svx/colrctrl.hxx>
#include <sfx2/objface.hxx>
#include <svx/f3dchild.hxx>
#include <svx/tbxcustomshapes.hxx>

#include <svx/svxids.hrc>
#include <svx/hyperdlg.hxx>
#include <avmedia/mediaplayer.hxx>


#include "app.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"
#include "glob.hrc"
#include "SpellDialogChildWindow.hxx"
#include "sdresid.hxx"
#include "DrawDocShell.hxx"
#include "GraphicDocShell.hxx"
#include "GraphicViewShell.hxx"
#include "AnimationChildWindow.hxx"
#include "NavigatorChildWindow.hxx"
#include "LayerDialogChildWindow.hxx"

using namespace sd;
#define DrawViewShell
#include "sdslots.hxx"
#define GraphicViewShell
#include "sdgslots.hxx"

namespace sd {

/**
 * Declare SFX-Slotmap and Standardinterface
 */


SFX_IMPL_INTERFACE(DrawViewShell, SfxShell, SdResId(STR_DRAWVIEWSHELL))
{
    SFX_POPUPMENU_REGISTRATION( SdResId(RID_DRAW_TEXTOBJ_INSIDE_POPUP) );
    SFX_CHILDWINDOW_CONTEXT_REGISTRATION( SID_NAVIGATOR );
    SFX_CHILDWINDOW_REGISTRATION( SfxTemplateDialogWrapper::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SfxInfoBarContainerChild::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SvxFontWorkChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SvxColorChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( AnimationChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( Svx3DChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SvxBmpMaskChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( GalleryChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SvxIMapDlgChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SvxHlinkDlgWrapper::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( ::sd::SpellDialogChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SID_SEARCH_DLG );
    SFX_CHILDWINDOW_REGISTRATION( ::avmedia::MediaPlayer::GetChildWindowId() );
}


TYPEINIT1( DrawViewShell, ViewShell );


// SdGraphicViewShell


SFX_IMPL_INTERFACE(GraphicViewShell, SfxShell, SdResId(STR_DRAWVIEWSHELL)) //SOH...
{
    SFX_POPUPMENU_REGISTRATION( SdResId(RID_DRAW_TEXTOBJ_INSIDE_POPUP) );
    SFX_CHILDWINDOW_CONTEXT_REGISTRATION( SID_NAVIGATOR );
    SFX_CHILDWINDOW_REGISTRATION( SID_TASKPANE );
    SFX_CHILDWINDOW_REGISTRATION( SfxTemplateDialogWrapper::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SvxFontWorkChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SvxColorChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( Svx3DChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SvxBmpMaskChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( GalleryChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SvxIMapDlgChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SvxHlinkDlgWrapper::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( ::sd::SpellDialogChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SID_SEARCH_DLG );
    SFX_CHILDWINDOW_REGISTRATION( ::avmedia::MediaPlayer::GetChildWindowId() );
}

TYPEINIT1( GraphicViewShell, DrawViewShell );


} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
