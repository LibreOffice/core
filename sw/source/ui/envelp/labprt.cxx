/*************************************************************************
 *
 *  $RCSfile: labprt.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:36 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include <vcl/system.hxx>
#ifndef _SV_PRNSETUP_HXX_ //autogen
#include <svtools/prnsetup.hxx>
#endif
#ifndef _SV_PRINT_HXX
#include <vcl/print.hxx>
#endif

#include "wrtsh.hxx"
#include "cmdid.h"
#include "label.hxx"
#include "labprt.hxx"
#include "labimg.hxx"
#include "labimp.hxx"

#include "labprt.hrc"


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
    aGBDontKnow    (this, SW_RES(GB_DONTKNOW)),
    aPrinterInfo   (this, SW_RES(INF_PRINTER)),
    aPrtSetup      (this, SW_RES(BTN_PRTSETUP)),
    aGBPrinter     (this, SW_RES(GB_PRINTER ))

{
    FreeResource();
    SetExchangeSupport();

    // Handler installieren
    Link aLk = LINK(this, SwLabPrtPage, CountHdl);
    aPageButton  .SetClickHdl( aLk );
    aSingleButton.SetClickHdl( aLk );

    aPrtSetup.SetClickHdl( aLk );
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

// --------------------------------------------------------------------------
/*
$Log: not supported by cvs2svn $
Revision 1.37  2000/09/18 16:05:26  willem.vandorp
OpenOffice header added.

Revision 1.36  2000/03/03 15:17:00  os
StarView remainders removed

Revision 1.35  2000/02/11 14:45:41  hr
#70473# changes for unicode ( patched by automated patchtool )

Revision 1.34  1998/09/11 10:29:48  OM
#56319# Richtiges Etikettenformat verwenden


      Rev 1.33   11 Sep 1998 12:29:48   OM
   #56319# Richtiges Etikettenformat verwenden

      Rev 1.32   07 Sep 1998 16:59:32   OM
   #55930# Einzelnes Etikett an der korrekten Position drucken

      Rev 1.31   14 Mar 1998 17:06:50   OM
   Gelinkte Etiketten

      Rev 1.30   14 Mar 1998 14:26:48   OM
   ExchangeSupport repariert/implementiert

      Rev 1.29   14 Mar 1998 14:10:46   OM
   ExchangeSupport repariert/implementiert

      Rev 1.28   16 Feb 1998 15:35:42   RG
   ->DOS-Format

      Rev 1.27   24 Nov 1997 11:52:14   MA
   includes

      Rev 1.26   03 Nov 1997 13:17:10   MA
   precomp entfernt

      Rev 1.25   01 Oct 1997 10:53:46   TRI
   PrintDialogumstellung

      Rev 1.24   11 Nov 1996 09:44:18   MA
   ResMgr

      Rev 1.23   29 Oct 1996 17:59:30   HJS
   includes

      Rev 1.22   07 Oct 1996 09:33:18   MA
   Umstellung Enable/Disable

      Rev 1.21   26 Jul 1996 20:36:38   MA
   includes

      Rev 1.20   28 Jun 1996 16:37:30   HJS
   includes

      Rev 1.19   06 Feb 1996 15:19:08   JP
   Link Umstellung 305

      Rev 1.18   25 Jan 1996 16:58:22   OM
   Dialogfelder richtig initialisieren

      Rev 1.17   08 Dec 1995 18:07:14   OM
   Drucker im Dtor loeschen

      Rev 1.16   27 Nov 1995 19:36:34   OM
   HasExchangeSupport->303a

      Rev 1.15   24 Nov 1995 16:59:40   OM
   PCH->PRECOMPILED

      Rev 1.14   23 Nov 1995 18:03:02   OM
   Etikettenanzahl richtig initialisieren

      Rev 1.13   13 Nov 1995 13:44:04   MA
   chg: Link-Cast entfernt

      Rev 1.12   08 Nov 1995 13:48:20   OM
   Change->Set

      Rev 1.11   04 Aug 1995 18:32:48   OM
   Umstellung Drucktabpage

      Rev 1.10   03 Aug 1995 18:49:38   OM
   Umstellung Print-TabPage

      Rev 1.9   13 Jul 1995 13:44:18   MA
   chg: LabelDlg jetzt ohne vorher ein Dokument zu erzeugen

      Rev 1.8   10 Jul 1995 09:50:20   MA
   LabelDlg optimiert und etwas aufgeraeumt.

      Rev 1.7   24 May 1995 18:13:48   ER
   Segmentierung

      Rev 1.6   18 Apr 1995 18:08:34   PK
   kleine bugfixes

      Rev 1.5   05 Apr 1995 09:27:00   JP
   Benutzung vom Link-Makro eingeschraenkt

      Rev 1.4   23 Mar 1995 18:33:50   PK
   geht immer weiter ...

      Rev 1.3   17 Mar 1995 17:10:32   PK
   geht immer weiter

      Rev 1.2   15 Mar 1995 13:27:08   PK
   geht immer weiter

      Rev 1.1   06 Mar 1995 02:33:38   PK
   envelp cebit-fertig

      Rev 1.0   06 Mar 1995 00:08:26   PK
   linkbarer envelp-zustand
*/



