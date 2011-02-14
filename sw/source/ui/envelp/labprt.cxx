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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif


#include <svtools/prnsetup.hxx>
#include <unotools/cmdoptions.hxx>
#include <vcl/print.hxx>
#include <wrtsh.hxx>
#include <label.hxx>
#include <labprt.hxx>
#include <labimg.hxx>
#ifndef _LABIMP_HXX
#include "swuilabimp.hxx"
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _LABPRT_HRC
#include <labprt.hrc>
#endif


// --------------------------------------------------------------------------



SwLabPrtPage::SwLabPrtPage(Window* pParent, const SfxItemSet& rSet) :

    SfxTabPage(pParent, SW_RES(TP_LAB_PRT), rSet),

    pPrinter( 0 ),
    aPageButton    (this, SW_RES(BTN_PAGE   )),
    aSingleButton  (this, SW_RES(BTN_SINGLE )),
    aColText       (this, SW_RES(TXT_COL    )),
    aColField      (this, SW_RES(FLD_COL    )),
    aRowText       (this, SW_RES(TXT_ROW    )),
    aRowField      (this, SW_RES(FLD_ROW    )),
    aSynchronCB    (this, SW_RES(CB_SYNCHRON)),
    aFLDontKnow    (this, SW_RES(FL_DONTKNOW)),
    aPrinterInfo   (this, SW_RES(INF_PRINTER)),
    aPrtSetup      (this, SW_RES(BTN_PRTSETUP)),
    aFLPrinter     (this, SW_RES(FL_PRINTER ))

{
    FreeResource();
    SetExchangeSupport();

    // Handler installieren
    Link aLk = LINK(this, SwLabPrtPage, CountHdl);
    aPageButton  .SetClickHdl( aLk );
    aSingleButton.SetClickHdl( aLk );

    aPrtSetup.SetClickHdl( aLk );

    SvtCommandOptions aCmdOpts;
    if ( aCmdOpts.Lookup(
             SvtCommandOptions::CMDOPTION_DISABLED,
             rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Print"  ) ) ) )
    {
        aPrinterInfo.Hide();
        aPrtSetup.Hide();
        aFLPrinter.Hide();
    }
}

// --------------------------------------------------------------------------



SwLabPrtPage::~SwLabPrtPage()
{
    if (pPrinter)
        delete pPrinter;
}

// --------------------------------------------------------------------------



IMPL_LINK( SwLabPrtPage, CountHdl, Button *, pButton )
{
    if (pButton == &aPrtSetup)
    {
        // Druck-Setup aufrufen
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

    if ( bEnable )
        aColField.GrabFocus();
#ifdef DBG_UTIL
    else
        ASSERT( pButton == &aPageButton, "NewButton?" );
#endif
    return 0;
}

// --------------------------------------------------------------------------



SfxTabPage* SwLabPrtPage::Create(Window* pParent, const SfxItemSet& rSet)
{
    return new SwLabPrtPage( pParent, rSet );
}

// --------------------------------------------------------------------------



void SwLabPrtPage::ActivatePage( const SfxItemSet& rSet )
{
    Reset(rSet);
}

// --------------------------------------------------------------------------



int SwLabPrtPage::DeactivatePage(SfxItemSet* _pSet)
{
    if ( _pSet )
        FillItemSet(*_pSet);

    return sal_True;
}

// --------------------------------------------------------------------------



void SwLabPrtPage::FillItem(SwLabItem& rItem)
{
    rItem.bPage = aPageButton.IsChecked();
    rItem.nCol  = (sal_uInt16) aColField.GetValue();
    rItem.nRow  = (sal_uInt16) aRowField.GetValue();
    rItem.bSynchron = aSynchronCB.IsChecked() && aSynchronCB.IsEnabled();
}

// --------------------------------------------------------------------------



sal_Bool SwLabPrtPage::FillItemSet(SfxItemSet& rSet)
{
    SwLabItem aItem;
    GetParent()->GetLabItem(aItem);
    FillItem(aItem);
    rSet.Put(aItem);

    return sal_True;
}

// --------------------------------------------------------------------------



void SwLabPrtPage::Reset(const SfxItemSet& )
{
    SwLabItem aItem;
    GetParent()->GetLabItem(aItem);

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
        // Drucker anzeigen
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




