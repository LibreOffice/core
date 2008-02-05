/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tabvwsh.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: ihi $ $Date: 2008-02-05 15:48:51 $
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
#include "precompiled_sc.hxx"



//------------------------------------------------------------------
// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <svx/galbrws.hxx>
#include <svx/imapdlg.hxx>
#include <svx/srchitem.hxx>
#include <sfx2/templdlg.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/app.hxx>
#include <avmedia/mediaplayer.hxx>

#include "tabvwsh.hxx"
#include "docsh.hxx"
#include "reffact.hxx"
#include "scresid.hxx"
#include "dwfunctr.hxx"
#include "sc.hrc"       // -> SID_TOOL_xxx
#include "drawattr.hxx" // -> SvxDrawToolItem
#include "spelldialog.hxx"


#define ScTabViewShell
#include "scslots.hxx"

#define SearchSettings
#include <svx/svxslots.hxx>

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
    SFX_CHILDWINDOW_REGISTRATION(ScOptSolverDlgWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(ScPivotLayoutWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(ScTabOpDlgWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(ScFilterDlgWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(ScSpecialFilterDlgWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(ScDbNameDlgWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(ScConsolidateDlgWrapper::GetChildWindowId());
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
    SFX_CHILDWINDOW_REGISTRATION(GalleryChildWindow::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(ScSpellDialogChildWindow::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION( ::avmedia::MediaPlayer::GetChildWindowId() );
}

SFX_IMPL_VIEWFACTORY( ScTabViewShell, ScResId(STR_NONAME) )
{
    SFX_VIEW_REGISTRATION(ScDocShell);
}

//------------------------------------------------------------------


