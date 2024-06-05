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
#include <svtools/unitconv.hxx>
#include <sal/log.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <docsh.hxx>
#include <charfmt.hxx>

#include <docstyle.hxx>

#include <lineinfo.hxx>
#include <linenum.hxx>
#include <swmodule.hxx>
#include <uitool.hxx>
#include <usrpref.hxx>
#include <wdocsh.hxx>
#include <fmtline.hxx>
#include <strings.hrc>

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

SwLineNumberingDlg::SwLineNumberingDlg(const SwView& rVw)
    : SfxDialogController(rVw.GetViewFrame().GetFrameWeld(),
            u"modules/swriter/ui/linenumbering.ui"_ustr, u"LineNumberingDialog"_ustr)
    , m_pSh(rVw.GetWrtShellPtr())
    , m_xBodyContent(m_xBuilder->weld_widget(u"content"_ustr))
    , m_xDivIntervalFT(m_xBuilder->weld_widget(u"every"_ustr))
    , m_xDivIntervalNF(m_xBuilder->weld_spin_button(u"linesspin"_ustr))
    , m_xDivRowsFT(m_xBuilder->weld_widget(u"lines"_ustr))
    , m_xNumIntervalNF(m_xBuilder->weld_spin_button(u"intervalspin"_ustr))
    , m_xCharStyleLB(m_xBuilder->weld_combo_box(u"styledropdown"_ustr))
    , m_xFormatLB(new SwNumberingTypeListBox(m_xBuilder->weld_combo_box(u"formatdropdown"_ustr)))
    , m_xPosLB(m_xBuilder->weld_combo_box(u"positiondropdown"_ustr))
    , m_xOffsetMF(m_xBuilder->weld_metric_spin_button(u"spacingspin"_ustr, FieldUnit::CM))
    , m_xDivisorED(m_xBuilder->weld_entry(u"textentry"_ustr))
    , m_xCountEmptyLinesCB(m_xBuilder->weld_check_button(u"blanklines"_ustr))
    , m_xCountFrameLinesCB(m_xBuilder->weld_check_button(u"linesintextframes"_ustr))
    , m_xRestartEachPageCB(m_xBuilder->weld_check_button(u"restarteverynewpage"_ustr))
    , m_xNumberingOnCB(m_xBuilder->weld_check_button(u"shownumbering"_ustr))
    , m_xNumberingOnFooterHeader(m_xBuilder->weld_check_button(u"showfooterheadernumbering"_ustr))
    , m_xOKButton(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xNumIntervalFT(m_xBuilder->weld_widget(u"interval"_ustr))
    , m_xNumRowsFT(m_xBuilder->weld_widget(u"intervallines"_ustr))
{
    m_xFormatLB->Reload(SwInsertNumTypes::Extended);

    OUString sIntervalName = m_xDivIntervalFT->get_accessible_name()
                             + "("
                             + m_xDivRowsFT->get_accessible_name()
                             + ")";
    m_xDivIntervalNF->set_accessible_name(sIntervalName);

    sIntervalName = m_xNumIntervalFT->get_accessible_name()
                    + "("
                    + m_xNumRowsFT->get_accessible_name()
                    + ")";
    m_xNumIntervalNF->set_accessible_name(sIntervalName);

    // char styles
    ::FillCharStyleListBox(*m_xCharStyleLB, m_pSh->GetView().GetDocShell());

    const SwLineNumberInfo &rInf = m_pSh->GetLineNumberInfo();
    IDocumentStylePoolAccess& rIDSPA = m_pSh->getIDocumentStylePoolAccess();

    OUString sStyleName(rInf.GetCharFormat( rIDSPA )->GetName());
    const int nPos = m_xCharStyleLB->find_text(sStyleName);

    if (nPos != -1)
        m_xCharStyleLB->set_active(nPos);
    else
    {
        if (!sStyleName.isEmpty())
        {
            m_xCharStyleLB->append_text(sStyleName);
            m_xCharStyleLB->set_active_text(sStyleName);
        }
    }

    // format
    SvxNumType nSelFormat = rInf.GetNumType().GetNumberingType();

    m_xFormatLB->SelectNumberingType(nSelFormat);

    // position
    m_xPosLB->set_active(rInf.GetPos());

    // offset
    sal_uInt16 nOffset = rInf.GetPosFromLeft();
    if (nOffset == USHRT_MAX)
        nOffset = 0;

    FieldUnit eFieldUnit = SW_MOD()->GetUsrPref(dynamic_cast< const SwWebDocShell*>(
                                rVw.GetDocShell()) != nullptr)->GetMetric();
    ::SetFieldUnit(*m_xOffsetMF, eFieldUnit);
    m_xOffsetMF->set_value(m_xOffsetMF->normalize(nOffset), FieldUnit::TWIP);

    // numbering offset
    m_xNumIntervalNF->set_value(rInf.GetCountBy());

    // divider
    m_xDivisorED->set_text(rInf.GetDivider());

    // divider offset
    m_xDivIntervalNF->set_value(rInf.GetDividerCountBy());

    // count
    m_xCountEmptyLinesCB->set_active(rInf.IsCountBlankLines());
    m_xCountFrameLinesCB->set_active(rInf.IsCountInFlys());
    m_xRestartEachPageCB->set_active(rInf.IsRestartEachPage());

    m_xNumberingOnCB->set_active(rInf.IsPaintLineNumbers());

    // Header/Footer Line Numbering
    rtl::Reference< SwDocStyleSheet > xStyleSheet = lcl_getDocStyleSheet(SwResId(STR_POOLCOLL_FOOTER), m_pSh);
    if(xStyleSheet.is())
    {
        SfxItemSet& rSet = xStyleSheet->GetItemSet();
        const SwFormatLineNumber &aFormat = rSet.Get(RES_LINENUMBER);
        if (aFormat.IsCount())
            m_xNumberingOnFooterHeader->set_state(TRISTATE_TRUE);
        else
            m_xNumberingOnFooterHeader->set_state(TRISTATE_FALSE);
    }

    // Line Numbering
    m_xNumberingOnCB->connect_toggled(LINK(this, SwLineNumberingDlg, LineOnOffHdl));
    m_xDivisorED->connect_changed(LINK(this, SwLineNumberingDlg, ModifyHdl));
    ModifyHdl(*m_xDivisorED);
    LineOnOffHdl(*m_xNumberingOnCB);

    m_xOKButton->connect_clicked(LINK(this, SwLineNumberingDlg, OKHdl));
}

SwLineNumberingDlg::~SwLineNumberingDlg()
{
}

IMPL_LINK_NOARG(SwLineNumberingDlg, OKHdl, weld::Button&, void)
{
    SwLineNumberInfo aInf(m_pSh->GetLineNumberInfo());

    // char styles
    OUString sCharFormatName(m_xCharStyleLB->get_active_text());
    SwCharFormat *pCharFormat = m_pSh->FindCharFormatByName(sCharFormatName);

    if (!pCharFormat)
    {
        SfxStyleSheetBasePool* pPool = m_pSh->GetView().GetDocShell()->GetStyleSheetPool();
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
    aType.SetNumberingType(m_xFormatLB->GetSelectedNumberingType());
    aInf.SetNumType(aType);

    // position
    aInf.SetPos(static_cast<LineNumberPosition>(m_xPosLB->get_active()));

    // offset
    aInf.SetPosFromLeft(o3tl::narrowing<sal_uInt16>(m_xOffsetMF->denormalize(m_xOffsetMF->get_value(FieldUnit::TWIP))));

    // numbering offset
    aInf.SetCountBy(o3tl::narrowing<sal_uInt16>(m_xNumIntervalNF->get_value()));

    // divider
    aInf.SetDivider(m_xDivisorED->get_text());

    // divider offset
    aInf.SetDividerCountBy(o3tl::narrowing<sal_uInt16>(m_xDivIntervalNF->get_value()));

    // count
    aInf.SetCountBlankLines(m_xCountEmptyLinesCB->get_active());
    aInf.SetCountInFlys(m_xCountFrameLinesCB->get_active());
    aInf.SetRestartEachPage(m_xRestartEachPageCB->get_active());

    aInf.SetPaintLineNumbers(m_xNumberingOnCB->get_active());

    m_pSh->SetLineNumberInfo(aInf);

    // Set LineNumber explicitly for Header and Footer
    lcl_setLineNumbering(SwResId(STR_POOLCOLL_FOOTER), m_pSh, m_xNumberingOnFooterHeader->get_active());
    lcl_setLineNumbering(SwResId(STR_POOLCOLL_HEADER), m_pSh, m_xNumberingOnFooterHeader->get_active());
    if( m_xNumberingOnFooterHeader->get_active())
       m_xNumberingOnFooterHeader->set_state(TRISTATE_TRUE);
    else
       m_xNumberingOnFooterHeader->set_state(TRISTATE_FALSE);

    m_xDialog->response(RET_OK);
}

// modify
IMPL_LINK_NOARG(SwLineNumberingDlg, ModifyHdl, weld::Entry&, void)
{
    bool bEnable = m_xNumberingOnCB->get_active() && !m_xDivisorED->get_text().isEmpty();

    m_xDivIntervalFT->set_sensitive(bEnable);
    m_xDivIntervalNF->set_sensitive(bEnable);
    m_xDivRowsFT->set_sensitive(bEnable);
}

// On/Off
IMPL_LINK_NOARG(SwLineNumberingDlg, LineOnOffHdl, weld::Toggleable&, void)
{
    bool bEnable = m_xNumberingOnCB->get_active();
    m_xBodyContent->set_sensitive(bEnable);
    ModifyHdl(*m_xDivisorED);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
