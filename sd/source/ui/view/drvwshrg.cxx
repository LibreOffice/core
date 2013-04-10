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

#include "DrawViewShell.hxx"
#include <sfx2/templdlg.hxx>

#include <svx/fontwork.hxx>
#include <svx/bmpmask.hxx>
#include <svx/galbrws.hxx>
#include <svx/imapdlg.hxx>
#include <svx/colrctrl.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/sidebar/SidebarChildWindow.hxx>
#include <svx/f3dchild.hxx>
#include <svx/tbxcustomshapes.hxx>

#include <svx/svxids.hrc>
#include <svx/hyprlink.hxx>
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

//AF:unused #define TABCONTROL_INITIAL_SIZE     500

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
    SFX_CHILDWINDOW_REGISTRATION( SvxFontWorkChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SvxColorChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( AnimationChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( Svx3DChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SvxBmpMaskChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( GalleryChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SvxIMapDlgChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SvxHyperlinkDlgWrapper::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SvxHlinkDlgWrapper::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( ::sd::SpellDialogChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SID_SEARCH_DLG );
    SFX_CHILDWINDOW_REGISTRATION( ::avmedia::MediaPlayer::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION(::sfx2::sidebar::SidebarChildWindow::GetChildWindowId());
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
    SFX_CHILDWINDOW_REGISTRATION( SvxHyperlinkDlgWrapper::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SvxHlinkDlgWrapper::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( ::sd::SpellDialogChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SID_SEARCH_DLG );
    SFX_CHILDWINDOW_REGISTRATION( ::avmedia::MediaPlayer::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION(::sfx2::sidebar::SidebarChildWindow::GetChildWindowId());
}

TYPEINIT1( GraphicViewShell, DrawViewShell );


} // end of namespace sd
