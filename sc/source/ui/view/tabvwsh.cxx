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

#include "scitems.hxx"
#include <svx/galbrws.hxx>
#include <svx/imapdlg.hxx>
#include <svl/srchitem.hxx>
#include <sfx2/templdlg.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/app.hxx>

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

TYPEINIT2(ScTabViewShell,SfxViewShell,SfxListener);

SFX_IMPL_INTERFACE(ScTabViewShell,SfxViewShell,ScResId(SCSTR_TABVIEWSHELL))
{
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_TOOLS  | SFX_VISIBILITY_STANDARD |
                                SFX_VISIBILITY_FULLSCREEN | SFX_VISIBILITY_SERVER,
                                ScResId(RID_OBJECTBAR_TOOLS) );

    SFX_CHILDWINDOW_REGISTRATION(FID_INPUTLINE_STATUS);
    SFX_CHILDWINDOW_REGISTRATION(SfxTemplateDialogWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_CONTEXT_REGISTRATION(SID_NAVIGATOR);
    SFX_CHILDWINDOW_REGISTRATION(SID_TASKPANE);
    SFX_CHILDWINDOW_REGISTRATION(ScNameDlgWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(ScNameDefDlgWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(ScCondFormatConditionDlgWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(ScCondFormatColorScaleDlgWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(ScCondFormatDataBarDlgWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(ScSolverDlgWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(ScOptSolverDlgWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(ScPivotLayoutWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(ScTabOpDlgWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(ScFilterDlgWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(ScSpecialFilterDlgWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(ScDbNameDlgWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(ScConsolidateDlgWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(ScPrintAreasDlgWrapper::GetChildWindowId());
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
    SFX_CHILDWINDOW_REGISTRATION(ScValidityRefChildWin::GetChildWindowId());
}

SFX_IMPL_NAMED_VIEWFACTORY( ScTabViewShell, "Default" )
{
    SFX_VIEW_REGISTRATION(ScDocShell);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
