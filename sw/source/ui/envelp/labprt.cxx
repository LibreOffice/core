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

#include <svtools/prnsetup.hxx>
#include <unotools/cmdoptions.hxx>
#include <vcl/print.hxx>
#include <label.hxx>
#include "labprt.hxx"
#include <labimg.hxx>

SwLabPrtPage::SwLabPrtPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, u"modules/swriter/ui/labeloptionspage.ui"_ustr, u"LabelOptionsPage"_ustr, &rSet)
    , m_pPrinter(nullptr)
    , m_xPageButton(m_xBuilder->weld_radio_button(u"entirepage"_ustr))
    , m_xSingleButton(m_xBuilder->weld_radio_button(u"singlelabel"_ustr))
    , m_xSingleGrid(m_xBuilder->weld_widget(u"singlegrid"_ustr))
    , m_xPrinterFrame(m_xBuilder->weld_widget(u"printerframe"_ustr))
    , m_xColField(m_xBuilder->weld_spin_button(u"cols"_ustr))
    , m_xRowField(m_xBuilder->weld_spin_button(u"rows"_ustr))
    , m_xSynchronCB(m_xBuilder->weld_check_button(u"synchronize"_ustr))
    , m_xPrinterInfo(m_xBuilder->weld_label(u"printername"_ustr))
    , m_xPrtSetup(m_xBuilder->weld_button(u"setup"_ustr))
{
    SetExchangeSupport();

    // Install handlers
    Link<weld::Toggleable&,void> aLk = LINK(this, SwLabPrtPage, CountHdl);
    m_xPageButton->connect_toggled(aLk);
    m_xSingleButton->connect_toggled(aLk);
    m_xPrtSetup->connect_clicked(LINK(this, SwLabPrtPage, PrtSetupHdl));

    SvtCommandOptions aCmdOpts;
    if (aCmdOpts.LookupDisabled(u"Print"_ustr))
    {
        m_xPrinterFrame->hide();
    }
}

SwLabPrtPage::~SwLabPrtPage()
{
    m_pPrinter.disposeAndClear();
}

IMPL_LINK( SwLabPrtPage, PrtSetupHdl, weld::Button&, rButton, void )
{
    // Call printer setup
    if (!m_pPrinter)
        m_pPrinter = VclPtr<Printer>::Create();

    PrinterSetupDialog aDlg(GetFrameWeld());
    aDlg.SetPrinter(m_pPrinter);
    aDlg.run();
    rButton.grab_focus();
    m_xPrinterInfo->set_label(m_pPrinter->GetName());
}

IMPL_LINK(SwLabPrtPage, CountHdl, weld::Toggleable&, rButton, void)
{
    if (!rButton.get_active())
        return;

    const bool bEnable = m_xSingleButton->get_active();
    m_xSingleGrid->set_sensitive(bEnable);
    m_xSynchronCB->set_sensitive(!bEnable);

    if (bEnable)
        m_xColField->grab_focus();
}

std::unique_ptr<SfxTabPage> SwLabPrtPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rSet)
{
    return std::make_unique<SwLabPrtPage>(pPage, pController, *rSet );
}

void SwLabPrtPage::ActivatePage( const SfxItemSet& rSet )
{
    Reset(&rSet);
}

DeactivateRC SwLabPrtPage::DeactivatePage(SfxItemSet* _pSet)
{
    if ( _pSet )
        FillItemSet(_pSet);

    return DeactivateRC::LeavePage;
}

void SwLabPrtPage::FillItem(SwLabItem& rItem)
{
    rItem.m_bPage = m_xPageButton->get_active();
    rItem.m_nCol = m_xColField->get_value();
    rItem.m_nRow = m_xRowField->get_value();
    rItem.m_bSynchron = m_xSynchronCB->get_active() && m_xSynchronCB->get_sensitive();
}

bool SwLabPrtPage::FillItemSet(SfxItemSet* rSet)
{
    SwLabItem aItem;
    GetParentSwLabDlg()->GetLabItem(aItem);
    FillItem(aItem);
    rSet->Put(aItem);

    return true;
}

void SwLabPrtPage::Reset(const SfxItemSet* )
{
    SwLabItem aItem;
    GetParentSwLabDlg()->GetLabItem(aItem);

    m_xColField->set_value(aItem.m_nCol);
    m_xRowField->set_value(aItem.m_nRow);

    if (aItem.m_bPage)
    {
        m_xPageButton->set_active(true);
        CountHdl(*m_xPageButton);
    }
    else
    {
        CountHdl(*m_xSingleButton);
        m_xSingleButton->set_active(true);
    }

    if (m_pPrinter)
    {
        // show printer
        m_xPrinterInfo->set_label(m_pPrinter->GetName());
    }
    else
        m_xPrinterInfo->set_label(Printer::GetDefaultPrinterName());

    m_xColField->set_max(aItem.m_nCols);
    m_xRowField->set_max(aItem.m_nRows);

    m_xSynchronCB->set_active(aItem.m_bSynchron);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
