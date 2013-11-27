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
#include "precompiled_sc.hxx"



//------------------------------------------------------------------
// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <svx/galbrws.hxx>
#include <svx/imapdlg.hxx>
#include <svl/srchitem.hxx>
#include <sfx2/templdlg.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/app.hxx>
#include <sfx2/sidebar/SidebarChildWindow.hxx>
#include <avmedia/mediaplayer.hxx>

//IAccessibility2 Implementation 2009-----
#include "cell.hxx"
#include "docoptio.hxx"
//-----IAccessibility2 Implementation 2009

#include "tabvwsh.hxx"
#include "docsh.hxx"
#include "reffact.hxx"
#include "scresid.hxx"
#include "dwfunctr.hxx"
#include "sc.hrc"       // -> SID_TOOL_xxx
#include "spelldialog.hxx"


#define ScTabViewShell
#include "scslots.hxx"

#define SearchSettings

SFX_IMPL_INTERFACE(ScTabViewShell,SfxViewShell,ScResId(SCSTR_TABVIEWSHELL))
{
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_TOOLS  | SFX_VISIBILITY_STANDARD |
                                SFX_VISIBILITY_FULLSCREEN | SFX_VISIBILITY_SERVER,
                                ScResId(RID_OBJECTBAR_TOOLS) );

    SFX_CHILDWINDOW_REGISTRATION(FID_INPUTLINE_STATUS);
    SFX_CHILDWINDOW_REGISTRATION(SfxTemplateDialogWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_CONTEXT_REGISTRATION(SID_NAVIGATOR);
    SFX_CHILDWINDOW_REGISTRATION(::sfx2::sidebar::SidebarChildWindow::GetChildWindowId());
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
    SFX_CHILDWINDOW_REGISTRATION(ScValidityRefChildWin::GetChildWindowId());
}

SFX_IMPL_NAMED_VIEWFACTORY( ScTabViewShell, "Default" )
{
    SFX_VIEW_REGISTRATION(ScDocShell);
}

//------------------------------------------------------------------

IMPL_LINK( ScTabViewShell, HtmlOptionsHdl, void*, EMPTYARG )
{
    // Invalidierung, falls blinkender Text erlaubt/verboten wurde
    GetViewFrame()->GetBindings().Invalidate(SID_DRAW_TEXT_MARQUEE);
    return 0;
}

//IAccessibility2 Implementation 2009-----
rtl::OUString ScTabViewShell::GetFormula(ScAddress& rAddress)
{
    String sFormula;
    ScDocument* pDoc = GetViewData()->GetDocument();
    ScBaseCell* pCell = pDoc->GetCell(rAddress);
    if (pCell && pCell->GetCellType()==CELLTYPE_FORMULA)
    {
        static_cast<ScFormulaCell*>(pCell)->GetFormula(sFormula);
    }
    return sFormula;
}
//-----IAccessibility2 Implementation 2009



