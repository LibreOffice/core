/*************************************************************************
 *
 *  $RCSfile: tabvwsh.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:10 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------

#if !defined OS2 && !defined WNT
//#define _BUTTON_HXX
#endif

#define _SETBRW_HXX
#define _STACK_HXX
//#define _STATUS_HXX
#define _STDMENU_HXX
#define _TABBAR_HXX
#define _VCBRW_HXX
#define _VCTRLS_HXX
#define _VCSBX_HXX
#define _VCONT_HXX
#define _VDRWOBJ_HXX

#define _BASE_DLGS_HXX
#define _BIGINT_HXX
#define _CACHESTR_HXX
#define _CONFIG_HXX
#define _CURSOR_HXX
#define _CTRLTOOL_HXX
#define _DLGCFG_HXX
#define _DYNARR_HXX
#define _EXTATTR_HXX
#define _FILDLG_HXX
#define _FONTDLG_HXX
#define _FRM3D_HXX
#define _INTRO_HXX
#define _ISETBWR_HXX
#define _NO_SVRTF_PARSER_HXX
#define _MACRODLG_HXX
#define _MODALDLG_HXX
#define _MOREBUTTON_HXX
#define _OUTLINER_HXX
#define _PASSWD_HXX
//#define _PRNDLG_HXX
//#define _POLY_HXX
#define _PVRWIN_HXX
#define _QUEUE_HXX
#define _RULER_HXX
#define _SCRWIN_HXX
#define _STACK_HXX
#define _SETBRW_HXX
#define _STACK_HXX
//#define _STATUS_HXX
#define _STDMENU_HXX
#define _TABBAR_HXX
#define _VCBRW_HXX
#define _VCTRLS_HXX
#define _VCSBX_HXX
#define _VCONT_HXX
#define _VDRWOBJ_HXX

#define _SVDXOUT_HXX
#define _SVDATTR_HXX
#define _SVDETC_HXX
#define _SVDIO_HXX
#define _SVDRAG_HXX
#define _SVDLAYER_HXX

//#define _SFX_BINDINGS_HXX
#define _SFXFILEDLG_HXX
#define _SFX_MACRO_HXX
#define _SFXMNUITEM_HXX
#define _SFXMNUMGR_HXX
#define _SFXMSGPOOL_HXX
#define _SFXMULTISEL_HXX
#define _SFXBASIC_HXX
#define _SFXSTBMGR_HXX
//#define _SFXTBXCTRL_HXX
//#define _SFXTBXMGR_HXX
//#define _SFXIMGMGR_HXX

#define _SI_DLL_HXX
#define _SIDLL_HXX
#define _SI_NOITEMS
#define _SI_NOOTHERFORMS
#define _SI_NOSBXCONTROLS
#define _SINOSBXCONTROLS
#define _SI_NODRW
#define _SI_NOCONTROL

#define _SVX_DAILDLL_HXX
#define _SVX_HYPHEN_HXX
#define _SVX_IMPGRF_HXX
#define _SVX_OPTITEMS_HXX
#define _SVX_OPTGERL_HXX
#define _SVX_OPTSAVE_HXX
#define _SVX_OPTSPELL_HXX
#define _SVX_OPTPATH_HXX
#define _SVX_OPTLINGU_HXX
#define _SVX_RULER_HXX
#define _SVX_RULRITEM_HXX
#define _SVX_SPLWRAP_HXX
#define _SVX_SPLDLG_HXX
#define _SVX_THESDLG_HXX

// INCLUDE ---------------------------------------------------------------

#define GLOBALOVERFLOW
#include <segmentc.hxx>

#include "scitems.hxx"
#include <basctl/idetemp.hxx>
#include <svx/imapdlg.hxx>
#include <svx/srchitem.hxx>
#include <offmgr/sbasltid.hrc>
#include <sfx2/templdlg.hxx>

#include "tabvwsh.hxx"
#include "docsh.hxx"
#include "reffact.hxx"
#include "scresid.hxx"
#include "dwfunctr.hxx"
#include "sc.hrc"       // -> SID_TOOL_xxx
#include "drawattr.hxx" // -> SvxDrawToolItem


#define ScTabViewShell
#include "scslots.hxx"

#define SearchSettings
#include <svx/svxslots.hxx>

SEG_EOFGLOBALS()

#pragma SEG_SEGCLASS(SFXMACROS_SEG,STARTWORK_CODE)

TYPEINIT2(ScTabViewShell,SfxViewShell,SfxListener);

SFX_IMPL_INTERFACE(ScTabViewShell,SfxViewShell,ScResId(SCSTR_TABVIEWSHELL))
{
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_TOOLS  | SFX_VISIBILITY_STANDARD |
                                SFX_VISIBILITY_FULLSCREEN | SFX_VISIBILITY_SERVER,
                                ScResId(RID_OBJECTBAR_TOOLS) );

    SFX_CHILDWINDOW_REGISTRATION(FID_INPUTLINE_STATUS);
    SFX_CHILDWINDOW_REGISTRATION(SfxTemplateDialogWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_CONTEXT_REGISTRATION(SID_NAVIGATOR);
    SFX_CHILDWINDOW_REGISTRATION(ScNameDlgWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(ScSolverDlgWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(ScPivotLayoutWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(ScTabOpDlgWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(ScFilterDlgWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(ScSpecialFilterDlgWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(ScDbNameDlgWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(ScConsolidateDlgWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(ScChartDlgWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(ScPrintAreasDlgWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(ScCondFormatDlgWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(ScColRowNameRangesDlgWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(ScFormulaDlgWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(SvxIMapDlgChildWindow::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(ScFunctionChildWindow::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(ScFormulaDlgWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(ScAcceptChgDlgWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(ScHighlightChgDlgWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(ScSimpleRefDlgWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(SID_SEARCH_DLG);
    SFX_CHILDWINDOW_REGISTRATION(SID_HYPERLINK_DIALOG);
}

SFX_IMPL_VIEWFACTORY( ScTabViewShell, ScResId(STR_NONAME) )
{
    SFX_VIEW_REGISTRATION(ScDocShell);
}

//------------------------------------------------------------------


/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.305  2000/09/17 14:09:35  willem.vandorp
    OpenOffice header added.

    Revision 1.304  2000/09/04 13:31:53  tbe
    basicide, isetbrw, si, vcdlged moved from svx to basctl

    Revision 1.303  2000/08/31 16:38:48  willem.vandorp
    Header and footer replaced

    Revision 1.302  2000/05/09 18:30:15  nn
    use IMPL_INTERFACE macro without IDL

    Revision 1.301  2000/05/09 17:37:21  nn
    old Basic removed

    Revision 1.300  2000/04/14 08:31:38  nn
    unicode changes

    Revision 1.299  2000/02/09 09:57:30  nn
    #72165# child window registration for hyperlink dialog

    Revision 1.298  1999/06/07 15:30:30  ANK
    #66547# Searchsettings aktiviert


      Rev 1.297   07 Jun 1999 17:30:30   ANK
   #66547# Searchsettings aktiviert

      Rev 1.296   02 Jun 1999 21:42:26   ANK
   #66547# SubShells

      Rev 1.295   09 Apr 1998 21:58:40   ANK
   Redlining Erweiterungen

      Rev 1.294   13 Mar 1998 12:58:24   ANK
   Changes

      Rev 1.293   08 Mar 1998 20:38:28   ANK
   Redlining

      Rev 1.292   05 Dec 1997 19:22:38   NN
   Includes

      Rev 1.291   20 Nov 1997 20:30:56   NN
   includes angepasst

      Rev 1.290   14 Oct 1997 19:58:56   ANK
   Neue Funktionsbox

      Rev 1.289   23 May 1997 20:32:30   NN
   ScFormulaDlgWrapper registrieren

      Rev 1.288   08 Apr 1997 17:34:42   HJS
   includes, defines

      Rev 1.287   18 Nov 1996 19:11:58   ER
   new: ScColRowNameRangesDlg

      Rev 1.286   14 Nov 1996 22:00:44   NN
   CHILDWINDOW_CONTEXT_REGISTRATION fuer Navigator

      Rev 1.285   29 Oct 1996 13:36:20   NN
   ueberall ScResId statt ResId

      Rev 1.284   16 Oct 1996 16:24:52   RJ
   Einbindung des Dialogs fuer bedingte Formatierung

      Rev 1.283   09 Aug 1996 20:35:02   NN
   Svx-Includes aus scitems.hxx raus

      Rev 1.282   10 Jul 1996 14:21:18   KA
   gallery entfernt, da jetzt im Explorer

      Rev 1.281   05 Jul 1996 17:25:26   NN
   SearchSettings aus svxslots, bei GetSbxObject anlegen

      Rev 1.280   20 Jun 1996 19:04:08   NN
   Werkzeugleiste registrieren

------------------------------------------------------------------------*/

#pragma SEG_EOFMODULE


