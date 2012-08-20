/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
    : SfxModalDialog( &pVw->GetViewFrame()->GetWindow(), rtl::OString("LineNumberingDialog"),
        rtl::OUString("modules/swriter/ui/linenumbering.ui") )
    , pSh(pVw->GetWrtShellPtr())
{
    m_pBodyContent = static_cast<VclContainer*>(m_pUIBuilder->get_by_name("content"));
    m_pDivIntervalFT = m_pUIBuilder->get_by_name("every");
    m_pDivIntervalNF = static_cast<NumericField*>(m_pUIBuilder->get_by_name("linesspin"));
    m_pDivRowsFT = m_pUIBuilder->get_by_name("lines");
    m_pNumIntervalNF = static_cast<NumericField*>(m_pUIBuilder->get_by_name("intervalspin"));
    m_pCharStyleLB = static_cast<ListBox*>(m_pUIBuilder->get_by_name("styledropdown"));
    m_pFormatLB = static_cast<SwNumberingTypeListBox*>(m_pUIBuilder->get_by_name("formatdropdown"));
    m_pPosLB = static_cast<ListBox*>(m_pUIBuilder->get_by_name("positiondropdown"));
    m_pOffsetMF = static_cast<MetricField*>(m_pUIBuilder->get_by_name("spacingspin"));
    m_pDivisorED = static_cast<Edit*>(m_pUIBuilder->get_by_name("textentry"));
    m_pCountEmptyLinesCB = static_cast<CheckBox*>(m_pUIBuilder->get_by_name("blanklines"));
    m_pCountFrameLinesCB = static_cast<CheckBox*>(m_pUIBuilder->get_by_name("linesintextframes"));
    m_pRestartEachPageCB = static_cast<CheckBox*>(m_pUIBuilder->get_by_name("restarteverynewpage"));
    m_pNumberingOnCB = static_cast<CheckBox*>(m_pUIBuilder->get_by_name("shownumbering"));

    String sIntervalName = m_pDivIntervalFT->GetAccessibleName();
    sIntervalName += rtl::OUString("(");
    sIntervalName += m_pDivRowsFT->GetAccessibleName();
    sIntervalName += rtl::OUString(")");
    m_pDivIntervalNF->SetAccessibleName(sIntervalName);

    Window *pNumIntervalFT = m_pUIBuilder->get_by_name("interval");
    Window *pNumRowsFT = m_pUIBuilder->get_by_name("intervallines");
    sIntervalName = pNumIntervalFT->GetAccessibleName();
    sIntervalName += rtl::OUString("(");
    sIntervalName += pNumRowsFT->GetAccessibleName();
    sIntervalName += rtl::OUString(")");
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

    PushButton *pOkPB = static_cast<PushButton*>(m_pUIBuilder->get_by_name("ok"));
    pOkPB->SetClickHdl(LINK(this, SwLineNumberingDlg, OKHdl));
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
    bool bHasValue = m_pDivisorED->GetText().Len() != 0;

    m_pDivIntervalFT->Enable(bHasValue);
    m_pDivIntervalNF->Enable(bHasValue);
    m_pDivRowsFT->Enable(bHasValue);

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
