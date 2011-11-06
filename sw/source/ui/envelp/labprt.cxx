/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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




