/*************************************************************************
 *
 *  $RCSfile: sddll2.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:30 $
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

#pragma hdrstop


#define ITEMID_SIZE 0
#define ITEMID_LINE 0         // kann spaeter raus!
#define ITEMID_BRUSH 0        // kann spaeter raus!
#include <svx/editdata.hxx>
#include "eetext.hxx"
#include <svx/svxids.hrc>

#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif

#define ITEMID_FIELD    EE_FEATURE_FIELD
#include <svx/flditem.hxx>
#ifndef _BASIDE_TBXCTL_HXX //autogen
#include <basctl/btbxctl.hxx>
#endif
#ifndef _IMAPDLG_HXX_ //autogen
#include <svx/imapdlg.hxx>
#endif
#ifndef _BMPMASK_HXX_ //autogen
#include <svx/bmpmask.hxx>
#endif
#ifndef _SVX_GALBRWS_HXX_ //autogen
#include <svx/galbrws.hxx>
#endif
#ifndef _SVX_SRCHDLG_HXX //autogen
#include <svx/srchdlg.hxx>
#endif
#ifndef _SVX_FONTWORK_HXX //autogen
#include <svx/fontwork.hxx>
#endif
#ifndef _SVX_COLRCTRL_HXX //autogen
#include <svx/colrctrl.hxx>
#endif
#ifndef _SVX_DLG_HYPERLINK_HXX //autogen
#include <offmgr/hyprlink.hxx>
#endif
#ifndef _SVX_TAB_HYPERLINK_HXX
#include <svx/hyperdlg.hxx>
#endif
#ifndef _FILLCTRL_HXX //autogen
#include <svx/fillctrl.hxx>
#endif
#ifndef _SVX_LINECTRL_HXX //autogen
#include <svx/linectrl.hxx>
#endif
#ifndef _SVX_TBCONTRL_HXX //autogen
#include <svx/tbcontrl.hxx>
#endif
#ifndef _SVX_ZOOMCTRL_HXX //autogen
#include <svx/zoomctrl.hxx>
#endif
#ifndef _SVX_PSZCTRL_HXX //autogen
#include <svx/pszctrl.hxx>
#endif
#ifndef _SVX_MODCTRL_HXX //autogen
#include <svx/modctrl.hxx>
#endif
#ifndef _SVX_FNTCTL_HXX //autogen
#include <svx/fntctl.hxx>
#endif
#ifndef _SVX_FNTSZCTL_HXX //autogen
#include <svx/fntszctl.hxx>
#endif
#ifndef _SVX_F3DCHILD_HXX //autogen
#include <svx/f3dchild.hxx>
#endif
#ifndef _SVX_GRAFCTRL_HXX
#include <svx/grafctrl.hxx>
#endif

#include "sddll.hxx"
#define _SD_DIACTRL_CXX
#include "diactrl.hxx"
#include "gluectrl.hxx"
#include "tbx_ww.hxx"
#include "drtxtob.hxx"
#include "drbezob.hxx"
#include "drstdob.hxx"
#include "animobjs.hxx"
#include "navichld.hxx"
#include "prevchld.hxx"
#include "efctchld.hxx"
#include "slidchld.hxx"
//#include "3dchld.hxx"
#include "app.hrc"
#include "docdlg.hxx"
#include "drviewsh.hxx"
#include "grviewsh.hxx"



/*************************************************************************
|*
|* Register all Controllers
|*
\************************************************************************/


void SdDLL::RegisterControllers()
{
    SfxModule* pMod = SD_MOD();

    // ToolBoxControls registrieren
    SdTbxControl::RegisterControl( SID_OBJECT_ALIGN, pMod );
    SdTbxControl::RegisterControl( SID_ZOOM_TOOLBOX, pMod );
    SdTbxControl::RegisterControl( SID_OBJECT_CHOOSE_MODE, pMod );
    SdTbxControl::RegisterControl( SID_POSITION, pMod );
    SdTbxControl::RegisterControl( SID_DRAWTBX_TEXT, pMod );
    SdTbxControl::RegisterControl( SID_DRAWTBX_RECTANGLES, pMod );
    SdTbxControl::RegisterControl( SID_DRAWTBX_ELLIPSES, pMod );
    SdTbxControl::RegisterControl( SID_DRAWTBX_LINES, pMod );
    SdTbxControl::RegisterControl( SID_DRAWTBX_ARROWS, pMod );
    SdTbxControl::RegisterControl( SID_DRAWTBX_3D_OBJECTS, pMod );
    SdTbxControl::RegisterControl( SID_DRAWTBX_CONNECTORS, pMod );
    SdTbxControl::RegisterControl( SID_DRAWTBX_INSERT, pMod );
    // CLs TbxCtl !!!
    TbxControls::RegisterControl( SID_OBJECT_SELECT, pMod );

    SdTbxCtlDiaEffect::RegisterControl(0, pMod);
    SdTbxCtlDiaSpeed::RegisterControl(0, pMod);
    SdTbxCtlDiaAuto::RegisterControl(0, pMod);
    SdTbxCtlDiaTime::RegisterControl(0, pMod);
    SdTbxCtlDiaPages::RegisterControl( SID_PAGES_PER_ROW, pMod );
    SdTbxCtlGlueEscDir::RegisterControl( SID_GLUE_ESCDIR, pMod );

    SdAnimationChildWindow::RegisterChildWindow(0, pMod);
    SdNavigatorChildWindow::RegisterChildWindowContext( SdDrawViewShell::_GetInterfaceIdImpl(), pMod );
    SdNavigatorChildWindow::RegisterChildWindowContext( SdGraphicViewShell::_GetInterfaceIdImpl(), pMod );
    SdPreviewChildWindow::RegisterChildWindow(0, pMod);
    SdEffectChildWindow::RegisterChildWindow(0, pMod);
    SdSlideChangeChildWindow::RegisterChildWindow(0, pMod);
    //Sd3DChildWindow::RegisterChildWindow(0, pMod);
    Svx3DChildWindow::RegisterChildWindow(0, pMod);
    SvxFontWorkChildWindow::RegisterChildWindow(0, pMod);
    SvxColorChildWindow::RegisterChildWindow(0, pMod, SFX_CHILDWIN_TASK);
    SvxSearchDialogWrapper::RegisterChildWindow(0, pMod);
    SvxBmpMaskChildWindow::RegisterChildWindow(0, pMod);
    GalleryChildWindow::RegisterChildWindow(0, pMod);
    SvxIMapDlgChildWindow::RegisterChildWindow(0, pMod);
    SvxHyperlinkDlgWrapper::RegisterChildWindow(0, pMod);
    SvxHlinkDlgWrapper::RegisterChildWindow(0, pMod);

    SvxFillToolBoxControl::RegisterControl(0, pMod);
    SvxLineStyleToolBoxControl::RegisterControl(0, pMod);
    SvxLineWidthToolBoxControl::RegisterControl(0, pMod);
    SvxLineColorToolBoxControl::RegisterControl(0, pMod);

    SvxLineEndToolBoxControl::RegisterControl( SID_ATTR_LINEEND_STYLE, pMod );

    SvxStyleToolBoxControl::RegisterControl(0, pMod);
    SvxFontNameToolBoxControl::RegisterControl(0, pMod);
    SvxFontHeightToolBoxControl::RegisterControl(0, pMod);
    SvxFontColorToolBoxControl::RegisterControl(0, pMod);

    SdTbxControl::RegisterControl( SID_GRAFTBX_FILTERS, pMod );
    SvxGrafRedToolBoxControl::RegisterControl( SID_ATTR_GRAF_RED, pMod );
    SvxGrafGreenToolBoxControl::RegisterControl( SID_ATTR_GRAF_GREEN, pMod );
    SvxGrafBlueToolBoxControl::RegisterControl( SID_ATTR_GRAF_BLUE, pMod );
    SvxGrafLuminanceToolBoxControl::RegisterControl( SID_ATTR_GRAF_LUMINANCE, pMod );
    SvxGrafContrastToolBoxControl::RegisterControl( SID_ATTR_GRAF_CONTRAST, pMod );
    SvxGrafGammaToolBoxControl::RegisterControl( SID_ATTR_GRAF_GAMMA, pMod );
    SvxGrafTransparenceToolBoxControl::RegisterControl( SID_ATTR_GRAF_TRANSPARENCE, pMod );
    SvxGrafModeToolBoxControl::RegisterControl( SID_ATTR_GRAF_MODE, pMod );

    // StatusBarControls registrieren
    SvxZoomStatusBarControl::RegisterControl( SID_ATTR_ZOOM, pMod );
    SvxPosSizeStatusBarControl::RegisterControl( SID_ATTR_SIZE, pMod );
    SvxModifyControl::RegisterControl( SID_DOC_MODIFIED, pMod );
    //SvxInsertStatusBarControl::RegisterControl(0, pModd);

    // MenuControls fuer PopupMenu
    SvxFontMenuControl::RegisterControl( SID_ATTR_CHAR_FONT, pMod );
    SvxFontSizeMenuControl::RegisterControl( SID_ATTR_CHAR_FONTHEIGHT, pMod );

    SfxMenuControl::RegisterControl( SID_SET_SNAPITEM, pMod );
    SfxMenuControl::RegisterControl( SID_DELETE_SNAPITEM, pMod );
    SfxMenuControl::RegisterControl( SID_BEZIER_CLOSE, pMod );
}



