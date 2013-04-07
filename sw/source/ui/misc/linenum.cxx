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

#include <IDocumentStylePoolAccess.hxx>

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

    String sIntervalName = m_pDivIntervalFT->GetAccessibleName();
    sIntervalName += OUString("(");
    sIntervalName += m_pDivRowsFT->GetAccessibleName();
    sIntervalName += OUString(")");
    m_pDivIntervalNF->SetAccessibleName(sIntervalName);

    Window *pNumIntervalFT = get<Window>("interval");
    Window *pNumRowsFT = get<Window>("intervallines");
    sIntervalName = pNumIntervalFT->GetAccessibleName();
    sIntervalName += OUString("(");
    sIntervalName += pNumRowsFT->GetAccessibleName();
    sIntervalName += OUString(")");
    m_pNumIntervalNF->SetAccessibleName(sIntervalName);

    // char styles
    ::FillCharStyleListBox(*m_pCharStyleLB, pSh->GetView().GetDocShell());

    const SwLineNumberInfo &rInf = pSh->GetLineNumberInfo();
    IDocumentStylePoolAccess* pIDSPA = pSh->getIDocumentStylePoolAccess();

    String sStyleName(rInf.GetCharFmt( *pIDSPA )->GetName());
    const sal_uInt16 nPos = m_pCharStyleLB->GetEntryPos(sStyleName);

    if (nPos != LISTBOX_ENTRY_NOTFOUND)
        m_pCharStyleLB->SelectEntryPos(nPos);
    else
    {
        if (sStyleName.Len())
        {
            m_pCharStyleLB->InsertEntry(sStyleName);
            m_pCharStyleLB->SelectEntry(sStyleName);
        }
    }

    // format
    sal_uInt16 nSelFmt = rInf.GetNumType().GetNumberingType();

    m_pFormatLB->SelectNumberingType(nSelFmt);

    // position
    m_pPosLB->SelectEntryPos((sal_uInt16)rInf.GetPos());

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

    m_pNumberingOnCB->SetClickHdl(LINK(this, SwLineNumberingDlg, LineOnOffHdl));
    m_pDivisorED->SetModifyHdl(LINK(this, SwLineNumberingDlg, ModifyHdl));
    ModifyHdl();
    LineOnOffHdl();

    get<PushButton>("ok")->SetClickHdl(LINK(this, SwLineNumberingDlg, OKHdl));
}

SwLineNumberingDlg::~SwLineNumberingDlg()
{
}

IMPL_LINK_NOARG(SwLineNumberingDlg, OKHdl)
{
    SwLineNumberInfo aInf(pSh->GetLineNumberInfo());

    // char styles
    String sCharFmtName(m_pCharStyleLB->GetSelectEntry());
    SwCharFmt *pCharFmt = pSh->FindCharFmtByName(sCharFmtName);

    if (!pCharFmt)
    {
        SfxStyleSheetBasePool* pPool = pSh->GetView().GetDocShell()->GetStyleSheetPool();
        SfxStyleSheetBase* pBase;
        pBase = pPool->Find(sCharFmtName, SFX_STYLE_FAMILY_CHAR);
        if(!pBase)
            pBase = &pPool->Make(sCharFmtName, SFX_STYLE_FAMILY_CHAR);
        pCharFmt = ((SwDocStyleSheet*)pBase)->GetCharFmt();
    }

    if (pCharFmt)
        aInf.SetCharFmt(pCharFmt);

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

    EndDialog( RET_OK );

    return 0;
}

/*--------------------------------------------------------------------
    Description: modify
 --------------------------------------------------------------------*/
IMPL_LINK_NOARG(SwLineNumberingDlg, ModifyHdl)
{
    bool bEnable = m_pNumberingOnCB->IsChecked() && !m_pDivisorED->GetText().isEmpty();

    m_pDivIntervalFT->Enable(bEnable);
    m_pDivIntervalNF->Enable(bEnable);
    m_pDivRowsFT->Enable(bEnable);

    return 0;
}

/*--------------------------------------------------------------------
    Description: On/Off
 --------------------------------------------------------------------*/
IMPL_LINK_NOARG(SwLineNumberingDlg, LineOnOffHdl)
{
    bool bEnable = m_pNumberingOnCB->IsChecked();
    m_pBodyContent->Enable(bEnable);
    ModifyHdl();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
