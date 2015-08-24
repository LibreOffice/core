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

SwWordCountWrapper::SwWordCountWrapper(   vcl::Window *pParentWindow,
                            sal_uInt16 nId,
                            SfxBindings* pBindings,
                            SfxChildWinInfo* pInfo ) :
        SfxChildWindow(pParentWindow, nId)
{
    SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
    assert(pFact && "SwAbstractDialogFactory fail!");
    xAbstDlg.reset(pFact->CreateSwWordCountDialog(pBindings, this, pParentWindow, pInfo));
    assert(xAbstDlg && "Dialog construction failed!");
    SetWindow(xAbstDlg->GetWindow());
}

SfxChildWinInfo SwWordCountWrapper::GetInfo() const
{
    SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();
    return aInfo;
}

void SwWordCountWrapper::UpdateCounts()
{
    xAbstDlg->UpdateCounts();
}

void SwWordCountWrapper::SetCounts(const SwDocStat &rCurrCnt, const SwDocStat &rDocStat)
{
    xAbstDlg->SetCounts(rCurrCnt, rDocStat);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
