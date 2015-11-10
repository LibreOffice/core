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

#include <officecfg/Office/Writer.hxx>
#include <swtypes.hxx>
#include <wordcountdialog.hxx>
#include <docstat.hxx>
#include <dialog.hrc>
#include <cmdid.h>
#include <swmodule.hxx>
#include <wview.hxx>
#include <swwait.hxx>
#include <wrtsh.hxx>
#include <comphelper/string.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/cjkoptions.hxx>
#include <unotools/localedatawrapper.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/settings.hxx>

IMPL_STATIC_LINK_NOARG_TYPED(SwWordCountFloatDlg, CloseHdl, Button*, void)
{
    SfxViewFrame* pVFrame = ::GetActiveView()->GetViewFrame();
    if (pVFrame != nullptr)
    {
        pVFrame->ToggleChildWindow(FN_WORDCOUNT_DIALOG);
    }
}

SwWordCountFloatDlg::~SwWordCountFloatDlg()
{
    disposeOnce();
}

void SwWordCountFloatDlg::dispose()
{
    SwViewShell::SetCareWin( nullptr );
    m_pCurrentWordFT.clear();
    m_pCurrentCharacterFT.clear();
    m_pCurrentCharacterExcludingSpacesFT.clear();
    m_pCurrentCjkcharsFT.clear();
    m_pCurrentStandardizedPagesFT.clear();
    m_pDocWordFT.clear();
    m_pDocCharacterFT.clear();
    m_pDocCharacterExcludingSpacesFT.clear();
    m_pDocCjkcharsFT.clear();
    m_pDocStandardizedPagesFT.clear();
    m_pCjkcharsLabelFT.clear();
    m_pStandardizedPagesLabelFT.clear();
    m_pClosePB.clear();
    SfxModelessDialog::dispose();
}

namespace
{
    void setValue(FixedText *pWidget, sal_uLong nValue, const LocaleDataWrapper& rLocaleData)
    {
        pWidget->SetText(rLocaleData.getNum(nValue, 0));
    }

    void setDoubleValue(FixedText *pWidget, double fValue)
    {
        OUString sValue(OUString::number(::rtl::math::round(fValue, 1)));
        pWidget->SetText(sValue);
    }
}

void SwWordCountFloatDlg::SetValues(const SwDocStat& rCurrent, const SwDocStat& rDoc)
{
    const LocaleDataWrapper& rLocaleData = GetSettings().GetUILocaleDataWrapper();
    setValue(m_pCurrentWordFT, rCurrent.nWord, rLocaleData);
    setValue(m_pCurrentCharacterFT, rCurrent.nChar, rLocaleData);
    setValue(m_pCurrentCharacterExcludingSpacesFT, rCurrent.nCharExcludingSpaces, rLocaleData);
    setValue(m_pCurrentCjkcharsFT, rCurrent.nAsianWord, rLocaleData);
    setValue(m_pDocWordFT, rDoc.nWord, rLocaleData);
    setValue(m_pDocCharacterFT, rDoc.nChar, rLocaleData);
    setValue(m_pDocCharacterExcludingSpacesFT, rDoc.nCharExcludingSpaces, rLocaleData);
    setValue(m_pDocCjkcharsFT, rDoc.nAsianWord, rLocaleData);

    if (m_pStandardizedPagesLabelFT->IsVisible())
    {
        sal_Int64 nCharsPerStandardizedPage = officecfg::Office::Writer::WordCount::StandardizedPageSize::get();
        setDoubleValue(m_pCurrentStandardizedPagesFT,
            (double)rCurrent.nChar / nCharsPerStandardizedPage);
        setDoubleValue(m_pDocStandardizedPagesFT,
            (double)rDoc.nChar / nCharsPerStandardizedPage);
    }

    bool bShowCJK = (SvtCJKOptions().IsAnyEnabled() || rDoc.nAsianWord);
    bool bToggleCJK = m_pCurrentCjkcharsFT->IsVisible() != bShowCJK;
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

void SwWordCountFloatDlg::showStandardizedPages(bool bShowStandardizedPages)
{
    m_pCurrentStandardizedPagesFT->Show(bShowStandardizedPages);
    m_pDocStandardizedPagesFT->Show(bShowStandardizedPages);
    m_pStandardizedPagesLabelFT->Show(bShowStandardizedPages);
}

SwWordCountFloatDlg::SwWordCountFloatDlg(SfxBindings* _pBindings,
                                         SfxChildWindow* pChild,
                                         vcl::Window *pParent,
                                         SfxChildWinInfo* pInfo)
    : SfxModelessDialog(_pBindings, pChild, pParent, "WordCountDialog", "modules/swriter/ui/wordcount.ui")
{
    get(m_pCurrentWordFT, "selectwords");
    get(m_pCurrentCharacterFT, "selectchars");
    get(m_pCurrentCharacterExcludingSpacesFT, "selectcharsnospaces");
    get(m_pCurrentCjkcharsFT, "selectcjkchars");
    get(m_pCurrentStandardizedPagesFT, "selectstandardizedpages");

    get(m_pDocWordFT, "docwords");
    get(m_pDocCharacterFT, "docchars");
    get(m_pDocCharacterExcludingSpacesFT, "doccharsnospaces");
    get(m_pDocCjkcharsFT, "doccjkchars");
    get(m_pDocStandardizedPagesFT, "docstandardizedpages");

    get(m_pCjkcharsLabelFT, "cjkcharsft");
    get(m_pStandardizedPagesLabelFT, "standardizedpages");

    get(m_pClosePB, "close");

    showCJK(SvtCJKOptions().IsAnyEnabled());
    showStandardizedPages(officecfg::Office::Writer::WordCount::ShowStandardizedPageCount::get());

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
        SwWait aWait( *::GetActiveView()->GetDocShell(), true );
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
