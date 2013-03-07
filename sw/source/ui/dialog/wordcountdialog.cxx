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

#include <swtypes.hxx>
#include <wordcountdialog.hxx>
#include <docstat.hxx>
#include <dialog.hxx>
#include <cmdid.h>
#include <swmodule.hxx>
#include <wview.hxx>
#include <swwait.hxx>
#include <wrtsh.hxx>
#include <comphelper/string.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/cjkoptions.hxx>
#include <vcl/msgbox.hxx>

IMPL_LINK_NOARG(SwWordCountFloatDlg, CloseHdl)
{
    SfxViewFrame* pVFrame = ::GetActiveView()->GetViewFrame();
    if (pVFrame != NULL)
    {
        pVFrame->ToggleChildWindow(FN_WORDCOUNT_DIALOG);
    }
    return 0;
}

SwWordCountFloatDlg::~SwWordCountFloatDlg()
{
    ViewShell::SetCareWin( 0 );
}

namespace
{
    void setValue(FixedText *pWidget, sal_uLong nValue)
    {
        rtl::OUString sValue(OUString::number(nValue));
        pWidget->SetText(sValue);
    }
}

void SwWordCountFloatDlg::SetValues(const SwDocStat& rCurrent, const SwDocStat& rDoc)
{
    setValue(m_pCurrentWordFT, rCurrent.nWord);
    setValue(m_pCurrentCharacterFT, rCurrent.nChar);
    setValue(m_pCurrentCharacterExcludingSpacesFT, rCurrent.nCharExcludingSpaces);
    setValue(m_pCurrentCjkcharsFT, rCurrent.nAsianWord);
    setValue(m_pDocWordFT, rDoc.nWord);
    setValue(m_pDocCharacterFT, rDoc.nChar);
    setValue(m_pDocCharacterExcludingSpacesFT, rDoc.nCharExcludingSpaces);
    setValue(m_pDocCjkcharsFT, rDoc.nAsianWord);

    bool bShowCJK = (SvtCJKOptions().IsAnyEnabled() || rDoc.nAsianWord);
    bool bToggleCJK = m_pCurrentCjkcharsFT->IsVisible() != static_cast<sal_Bool>(bShowCJK);
    if (bToggleCJK)
    {
        showCJK(bShowCJK);
        setOptimalLayoutSize(); //force resize of dialog
    }
}

void SwWordCountFloatDlg::showCJK(bool bShowCJK)
{
    m_pCurrentCjkcharsFT->Show(bShowCJK);
    m_pDocCjkcharsFT->Show(bShowCJK);
    m_pCjkcharsLabelFT->Show(bShowCJK);
}

SwWordCountFloatDlg::SwWordCountFloatDlg(SfxBindings* _pBindings,
                                         SfxChildWindow* pChild,
                                         Window *pParent,
                                         SfxChildWinInfo* pInfo)
    : SfxModelessDialog(_pBindings, pChild, pParent, "WordCountDialog", "modules/swriter/ui/wordcount.ui")
{
    get(m_pCurrentWordFT, "selectwords");
    get(m_pCurrentCharacterFT, "selectchars");
    get(m_pCurrentCharacterExcludingSpacesFT, "selectcharsnospaces");
    get(m_pCurrentCjkcharsFT, "selectcjkchars");

    get(m_pDocWordFT, "docwords");
    get(m_pDocCharacterFT, "docchars");
    get(m_pDocCharacterExcludingSpacesFT, "doccharsnospaces");
    get(m_pDocCjkcharsFT, "doccjkchars");

    get(m_pCjkcharsLabelFT, "cjkcharsft");

    get(m_pClosePB, "close");

    showCJK(SvtCJKOptions().IsAnyEnabled());

    Initialize(pInfo);

    m_pClosePB->SetClickHdl(LINK(this, SwWordCountFloatDlg, CloseHdl));
    m_pClosePB->GrabFocus();
}

void SwWordCountFloatDlg::Activate()
{
    SfxModelessDialog::Activate();
}

void SwWordCountFloatDlg::UpdateCounts()
{
    SwWrtShell &rSh = ::GetActiveView()->GetWrtShell();
    SwDocStat aCurrCnt;
    SwDocStat aDocStat;
    {
        SwWait aWait( *::GetActiveView()->GetDocShell(), sal_True );
        rSh.StartAction();
        rSh.CountWords( aCurrCnt );
        aDocStat = rSh.GetUpdatedDocStat();
        rSh.EndAction();
    }
    SetValues(aCurrCnt, aDocStat);
}

void SwWordCountFloatDlg::SetCounts(const SwDocStat &rCurrCnt, const SwDocStat &rDocStat)
{
    SetValues(rCurrCnt, rDocStat);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
