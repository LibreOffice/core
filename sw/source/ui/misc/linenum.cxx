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

#include <sfx2/viewfrm.hxx>
#include <svl/style.hxx>
#include <vcl/msgbox.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <docsh.hxx>
#include <charfmt.hxx>

#include <docstyle.hxx>

#include "fldbas.hxx"
#include "lineinfo.hxx"
#include "linenum.hxx"
#include "uitool.hxx"
#include <fmtline.hxx>
#include "strings.hrc"

#include <IDocumentStylePoolAccess.hxx>

static rtl::Reference<SwDocStyleSheet> lcl_getDocStyleSheet(const OUString& rName, SwWrtShell *pSh)
{
    SfxStyleSheetBasePool* pBase =  pSh->GetView().GetDocShell()->GetStyleSheetPool();
    SfxStyleSheetBase* pStyle = pBase->Find(rName, SfxStyleFamily::Para);
    SAL_WARN_IF( !pStyle, "sw.ui", "Style not found" );
    if(!pStyle)
        return nullptr;
    return new SwDocStyleSheet(*static_cast<SwDocStyleSheet*>(pStyle));
}

static void lcl_setLineNumbering(const OUString& rName, SwWrtShell* pSh, bool bLineNumber)
{
    rtl::Reference<SwDocStyleSheet> xStyleSheet = lcl_getDocStyleSheet(rName, pSh);
    if(!xStyleSheet.is())
        return;
    SfxItemSet& rSet = xStyleSheet->GetItemSet();
    SwFormatLineNumber aFormat;
    aFormat.SetCountLines(bLineNumber);
    rSet.Put(aFormat);
    xStyleSheet->MergeIndentAttrsOfListStyle( rSet );
    xStyleSheet->SetItemSet(rSet);
}

SwLineNumberingDlg::SwLineNumberingDlg(SwView *pVw)
    : SfxModalDialog( &pVw->GetViewFrame()->GetWindow(), "LineNumberingDialog",
        "modules/swriter/ui/linenumbering.ui" )
    , pSh(pVw->GetWrtShellPtr())
{
    get(m_pBodyContent, "content");
    get(m_pDivIntervalFT, "every");
    get(m_pDivIntervalNF, "linesspin");
    get(m_pDivRowsFT, "lines");
    get(m_pNumIntervalNF, "intervalspin");
    get(m_pCharStyleLB, "styledropdown");
    get(m_pFormatLB, "formatdropdown");
    get(m_pPosLB, "positiondropdown");
    get(m_pOffsetMF, "spacingspin");
    get(m_pDivisorED, "textentry");
    get(m_pCountEmptyLinesCB, "blanklines");
    get(m_pCountFrameLinesCB, "linesintextframes");
    get(m_pRestartEachPageCB, "restarteverynewpage");
    get(m_pNumberingOnCB, "shownumbering");
    get(m_pNumberingOnFooterHeader, "showfooterheadernumbering");

    OUString sIntervalName = m_pDivIntervalFT->GetAccessibleName()
                             + "("
                             + m_pDivRowsFT->GetAccessibleName()
                             + ")";
    m_pDivIntervalNF->SetAccessibleName(sIntervalName);

    vcl::Window *pNumIntervalFT = get<vcl::Window>("interval");
    vcl::Window *pNumRowsFT = get<vcl::Window>("intervallines");
    sIntervalName = pNumIntervalFT->GetAccessibleName()
                    + "("
                    + pNumRowsFT->GetAccessibleName()
                    + ")";
    m_pNumIntervalNF->SetAccessibleName(sIntervalName);

    // char styles
    ::FillCharStyleListBox(*m_pCharStyleLB, pSh->GetView().GetDocShell());

    const SwLineNumberInfo &rInf = pSh->GetLineNumberInfo();
    IDocumentStylePoolAccess& rIDSPA = pSh->getIDocumentStylePoolAccess();

    OUString sStyleName(rInf.GetCharFormat( rIDSPA )->GetName());
    const sal_Int32 nPos = m_pCharStyleLB->GetEntryPos(sStyleName);

    if (nPos != LISTBOX_ENTRY_NOTFOUND)
        m_pCharStyleLB->SelectEntryPos(nPos);
    else
    {
        if (!sStyleName.isEmpty())
        {
            m_pCharStyleLB->InsertEntry(sStyleName);
            m_pCharStyleLB->SelectEntry(sStyleName);
        }
    }

    // format
    SvxNumType nSelFormat = rInf.GetNumType().GetNumberingType();

    m_pFormatLB->SelectNumberingType(nSelFormat);

    // position
    m_pPosLB->SelectEntryPos((sal_Int32)rInf.GetPos());

    // offset
    sal_uInt16 nOffset = rInf.GetPosFromLeft();
    if (nOffset == USHRT_MAX)
        nOffset = 0;

    m_pOffsetMF->SetValue(m_pOffsetMF->Normalize(nOffset), FUNIT_TWIP);

    // numbering offset
    m_pNumIntervalNF->SetValue(rInf.GetCountBy());

    // divider
    m_pDivisorED->SetText(rInf.GetDivider());

    // divider offset
    m_pDivIntervalNF->SetValue(rInf.GetDividerCountBy());

    // count
    m_pCountEmptyLinesCB->Check(rInf.IsCountBlankLines());
    m_pCountFrameLinesCB->Check(rInf.IsCountInFlys());
    m_pRestartEachPageCB->Check(rInf.IsRestartEachPage());

    m_pNumberingOnCB->Check(rInf.IsPaintLineNumbers());

    // Header/Footer Line Numbering
    rtl::Reference< SwDocStyleSheet > xStyleSheet = lcl_getDocStyleSheet(SwResId(STR_POOLCOLL_FOOTER), pSh);
    if(xStyleSheet.is())
    {
        SfxItemSet& rSet = xStyleSheet->GetItemSet();
        const SwFormatLineNumber &aFormat = static_cast<const SwFormatLineNumber&>(rSet.Get(RES_LINENUMBER));
        if(aFormat.IsCount())
            m_pNumberingOnFooterHeader->SetState(TRISTATE_TRUE);
        else
            m_pNumberingOnFooterHeader->SetState(TRISTATE_FALSE);
    }

    // Line Numbering
    m_pNumberingOnCB->SetClickHdl(LINK(this, SwLineNumberingDlg, LineOnOffHdl));
    m_pDivisorED->SetModifyHdl(LINK(this, SwLineNumberingDlg, ModifyHdl));
    ModifyHdl(*m_pDivisorED);
    LineOnOffHdl();

    get<PushButton>("ok")->SetClickHdl(LINK(this, SwLineNumberingDlg, OKHdl));
}

SwLineNumberingDlg::~SwLineNumberingDlg()
{
    disposeOnce();
}

void SwLineNumberingDlg::dispose()
{
    m_pBodyContent.clear();
    m_pDivIntervalFT.clear();
    m_pDivIntervalNF.clear();
    m_pDivRowsFT.clear();
    m_pNumIntervalNF.clear();
    m_pCharStyleLB.clear();
    m_pFormatLB.clear();
    m_pPosLB.clear();
    m_pOffsetMF.clear();
    m_pDivisorED.clear();
    m_pCountEmptyLinesCB.clear();
    m_pCountFrameLinesCB.clear();
    m_pRestartEachPageCB.clear();
    m_pNumberingOnCB.clear();
    m_pNumberingOnFooterHeader.clear();
    SfxModalDialog::dispose();
}


IMPL_LINK_NOARG(SwLineNumberingDlg, OKHdl, Button*, void)
{
    SwLineNumberInfo aInf(pSh->GetLineNumberInfo());

    // char styles
    OUString sCharFormatName(m_pCharStyleLB->GetSelectEntry());
    SwCharFormat *pCharFormat = pSh->FindCharFormatByName(sCharFormatName);

    if (!pCharFormat)
    {
        SfxStyleSheetBasePool* pPool = pSh->GetView().GetDocShell()->GetStyleSheetPool();
        SfxStyleSheetBase* pBase;
        pBase = pPool->Find(sCharFormatName, SfxStyleFamily::Char);
        if(!pBase)
            pBase = &pPool->Make(sCharFormatName, SfxStyleFamily::Char);
        pCharFormat = static_cast<SwDocStyleSheet*>(pBase)->GetCharFormat();
    }

    if (pCharFormat)
        aInf.SetCharFormat(pCharFormat);

    // format
    SvxNumberType aType;
    aType.SetNumberingType(m_pFormatLB->GetSelectedNumberingType());
    aInf.SetNumType(aType);

    // position
    aInf.SetPos((LineNumberPosition)m_pPosLB->GetSelectEntryPos());

    // offset
    aInf.SetPosFromLeft((sal_uInt16)m_pOffsetMF->Denormalize(m_pOffsetMF->GetValue(FUNIT_TWIP)));

    // numbering offset
    aInf.SetCountBy((sal_uInt16)m_pNumIntervalNF->GetValue());

    // divider
    aInf.SetDivider(m_pDivisorED->GetText());

    // divider offset
    aInf.SetDividerCountBy((sal_uInt16)m_pDivIntervalNF->GetValue());

    // count
    aInf.SetCountBlankLines(m_pCountEmptyLinesCB->IsChecked());
    aInf.SetCountInFlys(m_pCountFrameLinesCB->IsChecked());
    aInf.SetRestartEachPage(m_pRestartEachPageCB->IsChecked());

    aInf.SetPaintLineNumbers(m_pNumberingOnCB->IsChecked());

    pSh->SetLineNumberInfo(aInf);

    // Set LineNumber explicitly for Header and Footer
    lcl_setLineNumbering(SwResId(STR_POOLCOLL_FOOTER), pSh, m_pNumberingOnFooterHeader->IsChecked());
    lcl_setLineNumbering(SwResId(STR_POOLCOLL_HEADER), pSh, m_pNumberingOnFooterHeader->IsChecked());
    if( m_pNumberingOnFooterHeader->IsChecked())
       m_pNumberingOnFooterHeader->SetState(TRISTATE_TRUE);
    else
       m_pNumberingOnFooterHeader->SetState(TRISTATE_FALSE);

    EndDialog( RET_OK );
}

// modify
IMPL_LINK_NOARG(SwLineNumberingDlg, ModifyHdl, Edit&, void)
{
    bool bEnable = m_pNumberingOnCB->IsChecked() && !m_pDivisorED->GetText().isEmpty();

    m_pDivIntervalFT->Enable(bEnable);
    m_pDivIntervalNF->Enable(bEnable);
    m_pDivRowsFT->Enable(bEnable);
}

// On/Off
IMPL_LINK_NOARG(SwLineNumberingDlg, LineOnOffHdl, Button*, void)
{
    bool bEnable = m_pNumberingOnCB->IsChecked();
    m_pBodyContent->Enable(bEnable);
    ModifyHdl(*m_pDivisorED);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
