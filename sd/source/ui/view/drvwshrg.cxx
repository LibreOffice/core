/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drvwshrg.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 19:40:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "DrawViewShell.hxx"

#ifndef _SFX_TEMPLDLG_HXX //autogen
#include <sfx2/templdlg.hxx>
#endif

#include <svx/fontwork.hxx>
#include <svx/bmpmask.hxx>
#include <svx/galbrws.hxx>
#include <svx/imapdlg.hxx>
#include <svx/colrctrl.hxx>
#include <sfx2/objface.hxx>

#ifndef _SVX_F3DCHILD_HXX //autogen
#include <svx/f3dchild.hxx>
#endif
#ifndef _SVX_TBXCUSTOMSHAPES_HXX
#include <svx/tbxcustomshapes.hxx>
#endif

#include <svx/svxids.hrc>
#include <svx/hyprlink.hxx>

#ifndef _SVX_TAB_HYPERLINK_HXX
#include <svx/hyperdlg.hxx>
#endif
#ifndef _AVMEDIA_MEDIAPLAYER_HXX
#include <avmedia/mediaplayer.hxx>
#endif


#include "app.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"
#include "glob.hrc"
#include "SpellDialogChildWindow.hxx"
#include "sdresid.hxx"
#ifndef SD_DRAW_DOC_SHELL_HXX
#include "DrawDocShell.hxx"
#endif
#ifndef SD_GRAPHIC_DOC_SHELL_HXX
#include "GraphicDocShell.hxx"
#endif
#ifndef SD_GRAPHIC_VIEW_SHELL_HXX
#include "GraphicViewShell.hxx"
#endif
#ifndef SD_ANIMATION_CHILD_WINDOW_HXX
#include "AnimationChildWindow.hxx"
#endif
#ifndef SD_NAVIGATOR_CHILD_WINDOW_HXX
#include "NavigatorChildWindow.hxx"
#endif
#ifndef SD_LAYER_DIALOG_CHILD_WINDOW_HXX
#include "LayerDialogChildWindow.hxx"
#endif

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

SFX_DECL_TYPE(13);


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
}


TYPEINIT1( DrawViewShell, ViewShell );


// SdGraphicViewShell


SFX_IMPL_INTERFACE(GraphicViewShell, SfxShell, SdResId(STR_DRAWVIEWSHELL)) //SOH...
{
    SFX_POPUPMENU_REGISTRATION( SdResId(RID_DRAW_TEXTOBJ_INSIDE_POPUP) );
    SFX_CHILDWINDOW_CONTEXT_REGISTRATION( SID_NAVIGATOR );
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
}

TYPEINIT1( GraphicViewShell, DrawViewShell );


} // end of namespace sd
