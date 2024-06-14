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
#include <wordcountdialog.hxx>
#include <docsh.hxx>
#include <docstat.hxx>
#include <swmodule.hxx>
#include <view.hxx>
#include <swwait.hxx>
#include <wrtsh.hxx>
#include <rtl/math.hxx>
#include <svl/cjkoptions.hxx>
#include <unotools/localedatawrapper.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/lok.hxx>
#include <PostItMgr.hxx>

static bool isLOKMobilePhone()
{
    if (!comphelper::LibreOfficeKit::isActive())
        return false;
    SfxViewShell* pCurrent = SfxViewShell::Current();
    return pCurrent && pCurrent->isLOKMobilePhone();
}

SwWordCountFloatDlg::~SwWordCountFloatDlg()
{
    SwViewShell::SetCareDialog(nullptr);
}

namespace
{
    void setValue(weld::Label& rWidget, sal_uLong nValue, const LocaleDataWrapper& rLocaleData)
    {
        rWidget.set_label(rLocaleData.getNum(nValue, 0));
    }

    void setDoubleValue(weld::Label& rWidget, double fValue)
    {
        OUString sValue(OUString::number(::rtl::math::round(fValue, 1)));
        rWidget.set_label(sValue);
    }
}

void SwWordCountFloatDlg::SetValues(const SwDocStat& rCurrent, const SwDocStat& rDoc)
{
    const LocaleDataWrapper& rLocaleData = Application::GetSettings().GetUILocaleDataWrapper();
    setValue(*m_xCurrentWordFT, rCurrent.nWord, rLocaleData);
    setValue(*m_xCurrentCharacterFT, rCurrent.nChar, rLocaleData);
    setValue(*m_xCurrentCharacterExcludingSpacesFT, rCurrent.nCharExcludingSpaces, rLocaleData);
    setValue(*m_xCurrentCjkcharsFT, rCurrent.nAsianWord, rLocaleData);
    setValue(*m_xDocWordFT, rDoc.nWord, rLocaleData);
    setValue(*m_xDocCharacterFT, rDoc.nChar, rLocaleData);
    setValue(*m_xDocCharacterExcludingSpacesFT, rDoc.nCharExcludingSpaces, rLocaleData);
    setValue(*m_xDocCjkcharsFT, rDoc.nAsianWord, rLocaleData);
    setValue(*m_xDocComments, rCurrent.nComments, rLocaleData);

    const sal_Int64 nCharsPerStandardizedPage = m_xStandardizedPagesLabelFT->get_visible() ?
        officecfg::Office::Writer::WordCount::StandardizedPageSize::get() : 0;
    if (nCharsPerStandardizedPage)
    {
        setDoubleValue(*m_xCurrentStandardizedPagesFT,
            static_cast<double>(rCurrent.nChar) / nCharsPerStandardizedPage);
        setDoubleValue(*m_xDocStandardizedPagesFT,
            static_cast<double>(rDoc.nChar) / nCharsPerStandardizedPage);
    }

    bool bShowCJK = (SvtCJKOptions::IsAnyEnabled() || rDoc.nAsianWord);
    bool bToggleCJK = m_xCurrentCjkcharsFT->get_visible() != bShowCJK;
    if (bToggleCJK)
    {
        showCJK(bShowCJK);
        m_xDialog->resize_to_request(); //force resize of dialog
    }
}

void SwWordCountFloatDlg::showCJK(bool bShowCJK)
{
    m_xCurrentCjkcharsFT->set_visible(bShowCJK);
    m_xDocCjkcharsFT->set_visible(bShowCJK);
    if (isLOKMobilePhone() && m_xCjkcharsLabelFT2)
        m_xCjkcharsLabelFT2->set_visible(bShowCJK);
    m_xCjkcharsLabelFT->set_visible(bShowCJK);
}

void SwWordCountFloatDlg::showStandardizedPages(bool bShowStandardizedPages)
{
    m_xCurrentStandardizedPagesFT->set_visible(bShowStandardizedPages);
    m_xDocStandardizedPagesFT->set_visible(bShowStandardizedPages);
    if (isLOKMobilePhone() && m_xStandardizedPagesLabelFT2)
        m_xStandardizedPagesLabelFT2->set_visible(bShowStandardizedPages);
    m_xStandardizedPagesLabelFT->set_visible(bShowStandardizedPages);
}

SwWordCountFloatDlg::SwWordCountFloatDlg(SfxBindings* _pBindings,
                                         SfxChildWindow* pChild,
                                         weld::Window *pParent,
                                         SfxChildWinInfo const * pInfo)
    : SfxModelessDialogController(_pBindings, pChild, pParent, isLOKMobilePhone() ? u"modules/swriter/ui/wordcount-mobile.ui"_ustr : u"modules/swriter/ui/wordcount.ui"_ustr, u"WordCountDialog"_ustr)
    , m_xCurrentWordFT(m_xBuilder->weld_label(u"selectwords"_ustr))
    , m_xCurrentCharacterFT(m_xBuilder->weld_label(u"selectchars"_ustr))
    , m_xCurrentCharacterExcludingSpacesFT(m_xBuilder->weld_label(u"selectcharsnospaces"_ustr))
    , m_xCurrentCjkcharsFT(m_xBuilder->weld_label(u"selectcjkchars"_ustr))
    , m_xCurrentStandardizedPagesFT(m_xBuilder->weld_label(u"selectstandardizedpages"_ustr))
    , m_xDocWordFT(m_xBuilder->weld_label(u"docwords"_ustr))
    , m_xDocCharacterFT(m_xBuilder->weld_label(u"docchars"_ustr))
    , m_xDocCharacterExcludingSpacesFT(m_xBuilder->weld_label(u"doccharsnospaces"_ustr))
    , m_xDocCjkcharsFT(m_xBuilder->weld_label(u"doccjkchars"_ustr))
    , m_xDocStandardizedPagesFT(m_xBuilder->weld_label(u"docstandardizedpages"_ustr))
    , m_xCjkcharsLabelFT(m_xBuilder->weld_label(u"cjkcharsft"_ustr))
    , m_xCjkcharsLabelFT2(m_xBuilder->weld_label(u"cjkcharsft2"_ustr))
    , m_xStandardizedPagesLabelFT(m_xBuilder->weld_label(u"standardizedpages"_ustr))
    , m_xStandardizedPagesLabelFT2(m_xBuilder->weld_label(u"standardizedpages2"_ustr))
    , m_xDocComments(m_xBuilder->weld_label(u"docComments"_ustr))
{
    showCJK(SvtCJKOptions::IsAnyEnabled());
    showStandardizedPages(officecfg::Office::Writer::WordCount::ShowStandardizedPageCount::get());

    Initialize(pInfo);
}

void SwWordCountFloatDlg::UpdateCounts()
{
    if (SwView* pView = GetActiveView())
    {
        SwWrtShell &rSh = pView->GetWrtShell();
        SwDocStat aCurrCnt;
        SwDocStat aDocStat;
        {
            auto& rDocShell(*pView->GetDocShell());
            SwWait aWait(rDocShell, true);
            auto aLock = rDocShell.LockAllViews();
            rSh.StartAction();
            rSh.CountWords( aCurrCnt );
            aDocStat = rSh.GetUpdatedDocStat();
            rSh.EndAction();
        }
        SwPostItMgr* pPostItMgr = rSh.GetPostItMgr();
        aCurrCnt.nComments = pPostItMgr->end() - pPostItMgr->begin();
        SetValues(aCurrCnt, aDocStat);
    }
}

void SwWordCountFloatDlg::SetCounts(const SwDocStat &rCurrCnt, const SwDocStat &rDocStat)
{
    SetValues(rCurrCnt, rDocStat);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
