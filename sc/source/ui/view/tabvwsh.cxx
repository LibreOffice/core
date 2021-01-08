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

#include <svx/imapdlg.hxx>
#include <svx/srchdlg.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/infobar.hxx>
#include <sfx2/sidebar/SidebarChildWindow.hxx>
#include <svx/devtools/DevelopmentToolDockingWindow.hxx>
#include <sfx2/viewfac.hxx>

#include <cellvalue.hxx>

#include <tabvwsh.hxx>
#include <docsh.hxx>
#include <reffact.hxx>
#include <sc.hrc>
#include <spelldialog.hxx>
#include <formulacell.hxx>
#include <searchresults.hxx>

    // needed for -fsanitize=function visibility of typeinfo for functions of
    // type void(SfxShell*,SfxRequest&) defined in scslots.hxx
#define ShellClass_ScTabViewShell
#include <scslots.hxx>


SFX_IMPL_INTERFACE(ScTabViewShell, SfxViewShell)

void ScTabViewShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_TOOLS,
                                            SfxVisibilityFlags::Standard | SfxVisibilityFlags::FullScreen | SfxVisibilityFlags::Server,
                                            ToolbarId::Objectbar_Tools);

    GetStaticInterface()->RegisterChildWindow(FID_INPUTLINE_STATUS);
    GetStaticInterface()->RegisterChildWindow(SfxInfoBarContainerChild::GetChildWindowId());

    GetStaticInterface()->RegisterChildWindow(SID_NAVIGATOR, true);

    GetStaticInterface()->RegisterChildWindow(::sfx2::sidebar::SidebarChildWindow::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(DevelopmentToolChildWindow::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(ScNameDlgWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(ScNameDefDlgWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(ScSolverDlgWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(ScOptSolverDlgWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(ScXMLSourceDlgWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(ScPivotLayoutWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(ScTabOpDlgWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(ScFilterDlgWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(ScSpecialFilterDlgWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(ScDbNameDlgWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(ScConsolidateDlgWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(ScPrintAreasDlgWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(ScColRowNameRangesDlgWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(ScFormulaDlgWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(SvxIMapDlgChildWindow::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(ScFormulaDlgWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(ScAcceptChgDlgWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(ScHighlightChgDlgWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(ScSimpleRefDlgWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(SvxSearchDialogWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(SID_HYPERLINK_DIALOG);
    GetStaticInterface()->RegisterChildWindow(ScSpellDialogChildWindow::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(ScValidityRefChildWin::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(sc::SearchResultsDlgWrapper::GetChildWindowId());

    GetStaticInterface()->RegisterChildWindow(ScRandomNumberGeneratorDialogWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(ScSamplingDialogWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(ScDescriptiveStatisticsDialogWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(ScAnalysisOfVarianceDialogWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(ScCorrelationDialogWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(ScCovarianceDialogWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(ScExponentialSmoothingDialogWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(ScMovingAverageDialogWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(ScRegressionDialogWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(ScTTestDialogWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(ScFTestDialogWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(ScZTestDialogWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(ScChiSquareTestDialogWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(ScFourierAnalysisDialogWrapper::GetChildWindowId());
    GetStaticInterface()->RegisterChildWindow(ScCondFormatDlgWrapper::GetChildWindowId());
}

SFX_IMPL_NAMED_VIEWFACTORY( ScTabViewShell, "Default" )
{
    SFX_VIEW_REGISTRATION(ScDocShell);
}

OUString ScTabViewShell::GetFormula(const ScAddress& rAddress)
{
    OUString sFormula;
    ScDocument& rDoc = GetViewData().GetDocument();
    ScRefCellValue aCell(rDoc, rAddress);
    if (!aCell.isEmpty() && aCell.meType == CELLTYPE_FORMULA)
    {
        aCell.mpFormula->GetFormula( sFormula);
    }
    return sFormula;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
