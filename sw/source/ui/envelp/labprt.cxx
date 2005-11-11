/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: labprt.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2005-11-11 11:47:05 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif


#pragma hdrstop

#ifndef _SV_PRNSETUP_HXX_ //autogen
#include <svtools/prnsetup.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_CMDOPTIONS_HXX
#include <svtools/cmdoptions.hxx>
#endif
#ifndef _SV_PRINT_HXX
#include <vcl/print.hxx>
#endif

#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _LABEL_HXX
#include <label.hxx>
#endif
#ifndef _LABPRT_HXX
#include <labprt.hxx>
#endif
#ifndef _LABIMG_HXX
#include <labimg.hxx>
#endif
#ifndef _LABIMP_HXX
#include "swuilabimp.hxx" //CHINA001
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
    const BOOL bEnable = pButton == &aSingleButton;
    aColText .Enable(bEnable);
    aColField.Enable(bEnable);
    aRowText .Enable(bEnable);
    aRowField.Enable(bEnable);
    aSynchronCB.Enable(!bEnable);

    if ( bEnable )
        aColField.GrabFocus();
#ifndef PRODUCT
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



int SwLabPrtPage::DeactivatePage(SfxItemSet* pSet)
{
    if ( pSet )
        FillItemSet(*pSet);

    return TRUE;
}

// --------------------------------------------------------------------------



void SwLabPrtPage::FillItem(SwLabItem& rItem)
{
    rItem.bPage = aPageButton.IsChecked();
    rItem.nCol  = (USHORT) aColField.GetValue();
    rItem.nRow  = (USHORT) aRowField.GetValue();
    rItem.bSynchron = aSynchronCB.IsChecked() && aSynchronCB.IsEnabled();
}

// --------------------------------------------------------------------------



BOOL SwLabPrtPage::FillItemSet(SfxItemSet& rSet)
{
    SwLabItem aItem;
    GetParent()->GetLabItem(aItem);
    FillItem(aItem);
    rSet.Put(aItem);

    return TRUE;
}

// --------------------------------------------------------------------------



void SwLabPrtPage::Reset(const SfxItemSet& rSet)
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




