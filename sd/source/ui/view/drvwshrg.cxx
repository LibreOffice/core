/*************************************************************************
 *
 *  $RCSfile: drvwshrg.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:58:47 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "DrawViewShell.hxx"

#ifndef _SFX_TEMPLDLG_HXX //autogen
#include <sfx2/templdlg.hxx>
#endif

#include <svx/fontwork.hxx>
#include <svx/bmpmask.hxx>
#include <svx/galbrws.hxx>
#include <svx/imapdlg.hxx>
#include <svx/colrctrl.hxx>

#ifndef _SVX_F3DCHILD_HXX //autogen
#include <svx/f3dchild.hxx>
#endif

#include <svx/svxids.hrc>
#include <svx/hyprlink.hxx>

#ifndef _SVX_TAB_HYPERLINK_HXX
#include <svx/hyperdlg.hxx>
#endif

#pragma hdrstop

#include "app.hrc"
#include "strings.hrc"
#include "res_bmp.hrc"
#include "glob.hrc"
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
#ifndef SD_PREVIEW_CHILD_WINDOW_HXX
#include "PreviewChildWindow.hxx"
#endif
#ifndef SD_EFFECT_CHILD_WINDOW_HXX
#include "EffectChildWindow.hxx"
#endif
#ifndef SD_LAYER_DIALOG_CHILD_WINDOW_HXX
#include "LayerDialogChildWindow.hxx"
#endif
#ifndef SD_SLIDE_CHANGE_CHILD_WINDOW_HXX
#include "SlideChangeChildWindow.hxx"
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
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_TOOLS | SFX_VISIBILITY_STANDARD |
                                SFX_VISIBILITY_FULLSCREEN | SFX_VISIBILITY_SERVER,
                                SdResId(RID_DRAW_TOOLBOX));
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_OPTIONS | SFX_VISIBILITY_STANDARD |
                                SFX_VISIBILITY_SERVER,
                                SdResId(RID_DRAW_OPTIONS_TOOLBOX));
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_COMMONTASK | SFX_VISIBILITY_STANDARD |
                                SFX_VISIBILITY_SERVER,
                                SdResId(RID_DRAW_COMMONTASK_TOOLBOX));
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_APPLICATION | SFX_VISIBILITY_DESKTOP | SFX_VISIBILITY_STANDARD | SFX_VISIBILITY_CLIENT | SFX_VISIBILITY_VIEWER | SFX_VISIBILITY_READONLYDOC,
                                SdResId(RID_DRAW_VIEWER_TOOLBOX) );
    SFX_CHILDWINDOW_CONTEXT_REGISTRATION( SID_NAVIGATOR );
    SFX_CHILDWINDOW_REGISTRATION( SfxTemplateDialogWrapper::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SvxFontWorkChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SvxColorChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( AnimationChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( PreviewChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( EffectChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( Svx3DChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SlideChangeChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SvxBmpMaskChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( GalleryChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SvxIMapDlgChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SvxHyperlinkDlgWrapper::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SvxHlinkDlgWrapper::GetChildWindowId() );
    //AF    SFX_CHILDWINDOW_REGISTRATION(
    //        ::sd::LayerDialogChildWindow::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION( SID_SEARCH_DLG );
}


TYPEINIT1( DrawViewShell, ViewShell );


// SdGraphicViewShell


SFX_IMPL_INTERFACE(GraphicViewShell, SfxShell, SdResId(STR_DRAWVIEWSHELL)) //SOH...
{
    SFX_POPUPMENU_REGISTRATION( SdResId(RID_DRAW_TEXTOBJ_INSIDE_POPUP) );
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_TOOLS | SFX_VISIBILITY_STANDARD |
                                SFX_VISIBILITY_FULLSCREEN | SFX_VISIBILITY_SERVER,
                                SdResId(RID_GRAPHIC_TOOLBOX));
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_OPTIONS | SFX_VISIBILITY_STANDARD |
                                SFX_VISIBILITY_SERVER,
                                SdResId(RID_GRAPHIC_OPTIONS_TOOLBOX));
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_APPLICATION | SFX_VISIBILITY_DESKTOP | SFX_VISIBILITY_STANDARD | SFX_VISIBILITY_CLIENT | SFX_VISIBILITY_VIEWER | SFX_VISIBILITY_READONLYDOC,
                                SdResId(RID_GRAPHIC_VIEWER_TOOLBOX) );
    /*SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_COMMONTASK | SFX_VISIBILITY_STANDARD |
                                SFX_VISIBILITY_SERVER,
                                SdResId(RID_DRAW_COMMONTASK_TOOLBOX));*/
    SFX_CHILDWINDOW_CONTEXT_REGISTRATION( SID_NAVIGATOR );
    SFX_CHILDWINDOW_REGISTRATION( SfxTemplateDialogWrapper::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SvxFontWorkChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SvxColorChildWindow::GetChildWindowId() );
    //SFX_CHILDWINDOW_REGISTRATION( SdAnimationChildWindow::GetChildWindowId() );
    /* ? */SFX_CHILDWINDOW_REGISTRATION( PreviewChildWindow::GetChildWindowId() );
    //SFX_CHILDWINDOW_REGISTRATION( SdEffectChildWindow::GetChildWindowId() );
    //SFX_CHILDWINDOW_REGISTRATION( SdSlideChangeChildWindow::GetChildWindowId() );
    //SFX_CHILDWINDOW_REGISTRATION( Sd3DChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( Svx3DChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SvxBmpMaskChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( GalleryChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SvxIMapDlgChildWindow::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SvxHyperlinkDlgWrapper::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SvxHlinkDlgWrapper::GetChildWindowId() );
    SFX_CHILDWINDOW_REGISTRATION( SID_SEARCH_DLG );
}

TYPEINIT1( GraphicViewShell, DrawViewShell );


} // end of namespace sd
