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
#include <docstat.hxx>
#include <swmodule.hxx>
#include <view.hxx>
#include <swwait.hxx>
#include <wrtsh.hxx>
#include <svl/cjkoptions.hxx>
#include <unotools/localedatawrapper.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/lok.hxx>
#include <sfx2/lokhelper.hxx>

#define IS_MOBILE (comphelper::LibreOfficeKit::isActive() && comphelper::LibreOfficeKit::isMobile(SfxLokHelper::getView()))

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

    if (m_xStandardizedPagesLabelFT->get_visible())
    {
        sal_Int64 nCharsPerStandardizedPage = officecfg::Office::Writer::WordCount::StandardizedPageSize::get();
        setDoubleValue(*m_xCurrentStandardizedPagesFT,
            static_cast<double>(rCurrent.nChar) / nCharsPerStandardizedPage);
        setDoubleValue(*m_xDocStandardizedPagesFT,
            static_cast<double>(rDoc.nChar) / nCharsPerStandardizedPage);
    }

    bool bShowCJK = (SvtCJKOptions().IsAnyEnabled() || rDoc.nAsianWord);
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
    m_xCjkcharsLabelFT->set_visible(bShowCJK);
    if (m_xCjkcharsLabel2FT.get())
        m_xCjkcharsLabel2FT->set_visible(bShowCJK);
}

void SwWordCountFloatDlg::showStandardizedPages(bool bShowStandardizedPages)
{
    m_xCurrentStandardizedPagesFT->set_visible(bShowStandardizedPages);
    m_xDocStandardizedPagesFT->set_visible(bShowStandardizedPages);
    m_xStandardizedPagesLabelFT->set_visible(bShowStandardizedPages);
    if (m_xStandardizedPagesLabel2FT.get())
        m_xStandardizedPagesLabel2FT->set_visible(bShowStandardizedPages);
}

SwWordCountFloatDlg::SwWordCountFloatDlg(SfxBindings* _pBindings,
                                         SfxChildWindow* pChild,
                                         weld::Window *pParent,
                                         SfxChildWinInfo const * pInfo)
    : SfxModelessDialogController(_pBindings, pChild, pParent, "modules/swriter/ui/wordcount.ui", "WordCountDialog")
    , m_xCloseButton(m_xBuilder->weld_button("close"))
    , m_xHelpButton(m_xBuilder->weld_button("help"))
    , m_xDocumentLabelFT(m_xBuilder->weld_label(IS_MOBILE ? "documentlabel-mobile" : "documentlabel"))
    , m_xWordsLabelFT(m_xBuilder->weld_label(IS_MOBILE ? "wordslabel-mobile" : "wordslabel"))
    , m_xIncludingSpacesLabelFT(m_xBuilder->weld_label(IS_MOBILE ? "inclspaceslabel-mobile" : "inclspaceslabel"))
    , m_xExcludingSpacesLabelFT(m_xBuilder->weld_label(IS_MOBILE ? "exclspaceslabel-mobile" : "exclspaceslabel"))
    , m_xCurrentWordFT(m_xBuilder->weld_label("selectwords"))
    , m_xCurrentCharacterFT(m_xBuilder->weld_label("selectchars"))
    , m_xCurrentCharacterExcludingSpacesFT(m_xBuilder->weld_label("selectcharsnospaces"))
    , m_xCurrentCjkcharsFT(m_xBuilder->weld_label("selectcjkchars"))
    , m_xCurrentStandardizedPagesFT(m_xBuilder->weld_label("selectstandardizedpages"))
    , m_xDocWordFT(m_xBuilder->weld_label(IS_MOBILE ? "docwords-mobile" : "docwords"))
    , m_xDocCharacterFT(m_xBuilder->weld_label(IS_MOBILE ? "docchars-mobile" : "docchars"))
    , m_xDocCharacterExcludingSpacesFT(m_xBuilder->weld_label(IS_MOBILE ? "doccharsnospaces-mobile" : "doccharsnospaces"))
    , m_xDocCjkcharsFT(m_xBuilder->weld_label(IS_MOBILE ? "doccjkchars-mobile" : "doccjkchars"))
    , m_xDocStandardizedPagesFT(m_xBuilder->weld_label(IS_MOBILE ? "docstandardizedpages-mobile" : "docstandardizedpages"))
    , m_xCjkcharsLabelFT(m_xBuilder->weld_label("cjkcharsft"))
    , m_xCjkcharsLabel2FT(IS_MOBILE ? m_xBuilder->weld_label("cjkcharsft2") : nullptr)
    , m_xStandardizedPagesLabelFT(m_xBuilder->weld_label("standardizedpages"))
    , m_xStandardizedPagesLabel2FT(IS_MOBILE ? m_xBuilder->weld_label("standardizedpages2") : nullptr)
{
    if (IS_MOBILE)
    {
        m_xCloseButton->set_visible(false);
        m_xHelpButton->set_visible(false);
    }

    m_xDocumentLabelFT->show();
    m_xDocWordFT->show();
    m_xDocCharacterFT->show();
    m_xDocCharacterExcludingSpacesFT->show();

    m_xWordsLabelFT->show();
    m_xIncludingSpacesLabelFT->show();
    m_xExcludingSpacesLabelFT->show();

    showCJK(SvtCJKOptions().IsAnyEnabled());
    showStandardizedPages(officecfg::Office::Writer::WordCount::ShowStandardizedPageCount::get());

    Initialize(pInfo);
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
