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
#include <labprt.hrc>

SwLabPrtPage::SwLabPrtPage(Window* pParent, const SfxItemSet& rSet) :

    SfxTabPage(pParent, SW_RES(TP_LAB_PRT), rSet),

    pPrinter( 0 ),
    aFLDontKnow    (this, SW_RES(FL_DONTKNOW)),
    aPageButton    (this, SW_RES(BTN_PAGE   )),
    aSingleButton  (this, SW_RES(BTN_SINGLE )),
    aColText       (this, SW_RES(TXT_COL    )),
    aColField      (this, SW_RES(FLD_COL    )),
    aRowText       (this, SW_RES(TXT_ROW    )),
    aRowField      (this, SW_RES(FLD_ROW    )),
    aSynchronCB    (this, SW_RES(CB_SYNCHRON)),
    aFLPrinter     (this, SW_RES(FL_PRINTER )),
    aPrinterInfo   (this, SW_RES(INF_PRINTER)),
    aPrtSetup      (this, SW_RES(BTN_PRTSETUP))

{
    FreeResource();
    SetExchangeSupport();

    // Install handlers
    Link aLk = LINK(this, SwLabPrtPage, CountHdl);
    aPageButton  .SetClickHdl( aLk );
    aSingleButton.SetClickHdl( aLk );

    aPrtSetup.SetClickHdl( aLk );

    SvtCommandOptions aCmdOpts;
    if ( aCmdOpts.Lookup(
             SvtCommandOptions::CMDOPTION_DISABLED,
             OUString( "Print"  ) ) )
    {
        aPrinterInfo.Hide();
        aPrtSetup.Hide();
        aFLPrinter.Hide();
    }
}

SwLabPrtPage::~SwLabPrtPage()
{
    delete pPrinter;
}

IMPL_LINK( SwLabPrtPage, CountHdl, Button *, pButton )
{
    if (pButton == &aPrtSetup)
    {
        // Call printer setup
        if (!pPrinter)
            pPrinter = new Printer;

        PrinterSetupDialog* pDlg = new PrinterSetupDialog(this );
        pDlg->SetPrinter(pPrinter);
        pDlg->Execute();
        delete pDlg;
        GrabFocus();
        aPrinterInfo.SetText(pPrinter->GetName());
        return 0;
    }
    const sal_Bool bEnable = pButton == &aSingleButton;
    aColText .Enable(bEnable);
    aColField.Enable(bEnable);
    aRowText .Enable(bEnable);
    aRowField.Enable(bEnable);
    aSynchronCB.Enable(!bEnable);

    OSL_ENSURE(!bEnable || pButton == &aPageButton, "NewButton?" );
    if ( bEnable )
    {
        aColField.GrabFocus();
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
    rItem.bPage = aPageButton.IsChecked();
    rItem.nCol  = (sal_uInt16) aColField.GetValue();
    rItem.nRow  = (sal_uInt16) aRowField.GetValue();
    rItem.bSynchron = aSynchronCB.IsChecked() && aSynchronCB.IsEnabled();
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

    aColField.SetValue   (aItem.nCol);
    aRowField.SetValue   (aItem.nRow);

    if (aItem.bPage)
    {
        aPageButton.Check();
        aPageButton.GetClickHdl().Call(&aPageButton);
    }
    else
    {
        aSingleButton.GetClickHdl().Call(&aSingleButton);
        aSingleButton.Check();
    }

    if (pPrinter)
    {
        // show printer
        aPrinterInfo.SetText(pPrinter->GetName());
    }
    else
        aPrinterInfo.SetText(Printer::GetDefaultPrinterName());

    aColField.SetMax(aItem.nCols);
    aRowField.SetMax(aItem.nRows);

    aColField.SetLast(aColField.GetMax());
    aRowField.SetLast(aRowField.GetMax());

    aSynchronCB.Check(aItem.bSynchron);
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
