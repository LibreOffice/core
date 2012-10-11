/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

/*************************************************************************
|*
|* SFX-Slotmap und Standardinterface deklarieren
|*
\************************************************************************/


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
