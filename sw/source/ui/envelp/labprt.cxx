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
#include <wrtsh.hxx>
#include <label.hxx>
#include <labprt.hxx>
#include <labimg.hxx>
#include "swuilabimp.hxx"

#include <cmdid.h>

SwLabPrtPage::SwLabPrtPage(Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "LabelOptionsPage",
        "modules/swriter/ui/labeloptionspage.ui", rSet)
    , pPrinter(0)
{
    get(m_pPageButton, "entirepage");
    get(m_pSingleButton, "singlelabel");
    get(m_pSingleGrid, "singlegrid");
    get(m_pColField, "cols");
    get(m_pRowField, "rows");
    get(m_pSynchronCB, "synchronize");
    get(m_pPrinterFrame, "printerframe");
    get(m_pPrinterInfo, "printername");
    get(m_pPrtSetup, "setup");
    SetExchangeSupport();

    // Install handlers
    Link aLk = LINK(this, SwLabPrtPage, CountHdl);
    m_pPageButton->SetClickHdl( aLk );
    m_pSingleButton->SetClickHdl( aLk );

    m_pPrtSetup->SetClickHdl( aLk );

    SvtCommandOptions aCmdOpts;
    if ( aCmdOpts.Lookup(
             SvtCommandOptions::CMDOPTION_DISABLED,
             OUString( "Print"  ) ) )
    {
        m_pPrinterFrame->Hide();
    }
}

SwLabPrtPage::~SwLabPrtPage()
{
    delete pPrinter;
}

IMPL_LINK( SwLabPrtPage, CountHdl, Button *, pButton )
{
    if (pButton == m_pPrtSetup)
    {
        // Call printer setup
        if (!pPrinter)
            pPrinter = new Printer;

        PrinterSetupDialog* pDlg = new PrinterSetupDialog(this );
        pDlg->SetPrinter(pPrinter);
        pDlg->Execute();
        delete pDlg;
        GrabFocus();
        m_pPrinterInfo->SetText(pPrinter->GetName());
        return 0;
    }
    const bool bEnable = pButton == m_pSingleButton;
    m_pSingleGrid->Enable(bEnable);
    m_pSynchronCB->Enable(!bEnable);

    OSL_ENSURE(!bEnable || pButton == m_pPageButton, "NewButton?" );
    if ( bEnable )
    {
        m_pColField->GrabFocus();
    }
    return 0;
}

SfxTabPage* SwLabPrtPage::Create(Window* pParent, const SfxItemSet& rSet)
{
    return new SwLabPrtPage( pParent, rSet );
}

void SwLabPrtPage::ActivatePage( const SfxItemSet& rSet )
{
    Reset(rSet);
}

int SwLabPrtPage::DeactivatePage(SfxItemSet* _pSet)
{
    if ( _pSet )
        FillItemSet(*_pSet);

    return sal_True;
}

void SwLabPrtPage::FillItem(SwLabItem& rItem)
{
    rItem.bPage = m_pPageButton->IsChecked();
    rItem.nCol  = (sal_uInt16) m_pColField->GetValue();
    rItem.nRow  = (sal_uInt16) m_pRowField->GetValue();
    rItem.bSynchron = m_pSynchronCB->IsChecked() && m_pSynchronCB->IsEnabled();
}

sal_Bool SwLabPrtPage::FillItemSet(SfxItemSet& rSet)
{
    SwLabItem aItem;
    GetParentSwLabDlg()->GetLabItem(aItem);
    FillItem(aItem);
    rSet.Put(aItem);

    return sal_True;
}

void SwLabPrtPage::Reset(const SfxItemSet& )
{
    SwLabItem aItem;
    GetParentSwLabDlg()->GetLabItem(aItem);

    m_pColField->SetValue   (aItem.nCol);
    m_pRowField->SetValue   (aItem.nRow);

    if (aItem.bPage)
    {
        m_pPageButton->Check();
        m_pPageButton->GetClickHdl().Call(m_pPageButton);
    }
    else
    {
        m_pSingleButton->GetClickHdl().Call(m_pSingleButton);
        m_pSingleButton->Check();
    }

    if (pPrinter)
    {
        // show printer
        m_pPrinterInfo->SetText(pPrinter->GetName());
    }
    else
        m_pPrinterInfo->SetText(Printer::GetDefaultPrinterName());

    m_pColField->SetMax(aItem.nCols);
    m_pRowField->SetMax(aItem.nRows);

    m_pColField->SetLast(m_pColField->GetMax());
    m_pRowField->SetLast(m_pRowField->GetMax());

    m_pSynchronCB->Check(aItem.bSynchron);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
