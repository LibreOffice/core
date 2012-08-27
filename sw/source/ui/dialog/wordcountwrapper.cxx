/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Matt Pratt <mattpratt.au@gmail.com>
 * Portions created by the Initial Developer are Copyright (C) 2011 the
 * Initial Developer. All Rights Reserved.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <swtypes.hxx>
#include <wordcountdialog.hxx>
#include <docstat.hxx>
#include <dialog.hrc>
#include <wordcountdialog.hrc>
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
    pAbstDlg = pFact->CreateSwWordCountDialog( DLG_WORDCOUNT, pBindings, this, pParentWindow, pInfo );
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
