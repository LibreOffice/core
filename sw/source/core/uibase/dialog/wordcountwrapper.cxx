/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swtypes.hxx>
#include <wordcountdialog.hxx>
#include <docstat.hxx>
#include <dialog.hrc>
#include <cmdid.h>

SFX_IMPL_CHILDWINDOW_WITHID(SwWordCountWrapper, FN_WORDCOUNT_DIALOG)

SwWordCountWrapper::SwWordCountWrapper(   Window *pParentWindow,
                            sal_uInt16 nId,
                            SfxBindings* pBindings,
                            SfxChildWinInfo* pInfo ) :
        SfxChildWindow(pParentWindow, nId)
{
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");
    pAbstDlg = pFact->CreateSwWordCountDialog(pBindings, this, pParentWindow, pInfo);
    OSL_ENSURE(pAbstDlg, "Dialog construction failed!");
    pWindow = pAbstDlg->GetWindow();

    eChildAlignment = SFX_ALIGN_NOALIGNMENT;
}

SfxChildWinInfo SwWordCountWrapper::GetInfo() const
{
    SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();
    return aInfo;
}

void SwWordCountWrapper::UpdateCounts()
{
    pAbstDlg->UpdateCounts();
}

void SwWordCountWrapper::SetCounts(const SwDocStat &rCurrCnt, const SwDocStat &rDocStat)
{
    pAbstDlg->SetCounts(rCurrCnt, rDocStat);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
