/*************************************************************************
 *
 *  $RCSfile: envprt.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:35 $
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

#ifndef _SV_PRINT_HXX //autogen
#include <vcl/print.hxx>
#endif
#ifndef _SV_PRNSETUP_HXX_ //autogen
#include <svtools/prnsetup.hxx>
#endif

#include "swtypes.hxx"
#include "cmdid.h"
#include "envprt.hxx"
#include "envlop.hxx"
#include "uitool.hxx"

#include "envprt.hrc"





SwEnvPrtPage::SwEnvPrtPage(Window* pParent, const SfxItemSet& rSet) :

    SfxTabPage(pParent, SW_RES(TP_ENV_PRT), rSet),

    aAlignBox    (this, SW_RES(BOX_ALIGN  )),
    aTopButton   (this, SW_RES(BTN_TOP    )),
    aBottomButton(this, SW_RES(BTN_BOTTOM )),
    aRightText   (this, SW_RES(TXT_RIGHT  )),
    aRightField  (this, SW_RES(FLD_RIGHT  )),
    aDownText    (this, SW_RES(TXT_DOWN   )),
    aDownField   (this, SW_RES(FLD_DOWN   )),
    aPrinterInfo (this, SW_RES(TXT_PRINTER)),
    aNoNameGroup (this, SW_RES(GRP_NONAME )),
    aPrtSetup    (this, SW_RES(BTN_PRTSETUP))

{
    FreeResource();
    SetExchangeSupport();

    // Metriken
    FieldUnit eUnit = ::GetDfltMetric(FALSE);
    SetMetric(aRightField, eUnit);
    SetMetric(aDownField , eUnit);

    // Handler installieren
    aTopButton   .SetClickHdl(LINK(this, SwEnvPrtPage, ClickHdl));
    aBottomButton.SetClickHdl(LINK(this, SwEnvPrtPage, ClickHdl));

    aPrtSetup    .SetClickHdl(LINK(this, SwEnvPrtPage, ButtonHdl));

    // Bitmaps
    aBottomButton.GetClickHdl().Call(&aBottomButton);

    // ToolBox
    Size aSz = aAlignBox.CalcWindowSizePixel();
    aAlignBox.SetSizePixel(aSz);
    aAlignBox.SetPosPixel(Point(aNoNameGroup.GetPosPixel().X() + (aNoNameGroup.GetSizePixel().Width() - aSz.Width()) / 2, aAlignBox.GetPosPixel().Y()));
    aAlignBox.SetClickHdl(LINK(this, SwEnvPrtPage, AlignHdl));

}

// --------------------------------------------------------------------------



SwEnvPrtPage::~SwEnvPrtPage()
{
}

// --------------------------------------------------------------------------



IMPL_LINK( SwEnvPrtPage, ClickHdl, Button *, EMPTYARG )
{
    if (aBottomButton.IsChecked())
    {
        // Briefumschlaege von unten
        aAlignBox.SetItemImage(ITM_HOR_LEFT, Bitmap(SW_RES(BMP_HOR_LEFT_LOWER)));
        aAlignBox.SetItemImage(ITM_HOR_CNTR, Bitmap(SW_RES(BMP_HOR_CNTR_LOWER)));
        aAlignBox.SetItemImage(ITM_HOR_RGHT, Bitmap(SW_RES(BMP_HOR_RGHT_LOWER)));
        aAlignBox.SetItemImage(ITM_VER_LEFT, Bitmap(SW_RES(BMP_VER_LEFT_LOWER)));
        aAlignBox.SetItemImage(ITM_VER_CNTR, Bitmap(SW_RES(BMP_VER_CNTR_LOWER)));
        aAlignBox.SetItemImage(ITM_VER_RGHT, Bitmap(SW_RES(BMP_VER_RGHT_LOWER)));
    }
    else
    {
        // Briefumschlaege von oben
        aAlignBox.SetItemImage(ITM_HOR_LEFT, Bitmap(SW_RES(BMP_HOR_LEFT_UPPER)));
        aAlignBox.SetItemImage(ITM_HOR_CNTR, Bitmap(SW_RES(BMP_HOR_CNTR_UPPER)));
        aAlignBox.SetItemImage(ITM_HOR_RGHT, Bitmap(SW_RES(BMP_HOR_RGHT_UPPER)));
        aAlignBox.SetItemImage(ITM_VER_LEFT, Bitmap(SW_RES(BMP_VER_LEFT_UPPER)));
        aAlignBox.SetItemImage(ITM_VER_CNTR, Bitmap(SW_RES(BMP_VER_CNTR_UPPER)));
        aAlignBox.SetItemImage(ITM_VER_RGHT, Bitmap(SW_RES(BMP_VER_RGHT_UPPER)));
    }
    return 0;
}

// --------------------------------------------------------------------------



IMPL_LINK( SwEnvPrtPage, ButtonHdl, Button *, pBtn )
{
    if (pBtn == &aPrtSetup)
    {
        // Druck-Setup aufrufen
        if (pPrt)
        {
            PrinterSetupDialog* pDlg = new PrinterSetupDialog(this );
            pDlg->SetPrinter(pPrt);
            pDlg->Execute();
            delete pDlg;
            GrabFocus();
            aPrinterInfo.SetText(pPrt->GetName());
        }
    }
    return 0;
}

// --------------------------------------------------------------------------



IMPL_LINK( SwEnvPrtPage, AlignHdl, ToolBox *, EMPTYARG )
{
    if (aAlignBox.GetCurItemId())
    {
        for (USHORT i = ITM_HOR_LEFT; i <= ITM_VER_RGHT; i++)
            aAlignBox.CheckItem(i, FALSE);
        aAlignBox.CheckItem(aAlignBox.GetCurItemId(), TRUE);
    }
    else
    {
        // GetCurItemId() == 0 ist moeglich!
        const SwEnvItem& rItem = (const SwEnvItem&) GetItemSet().Get(FN_ENVELOP);
        aAlignBox.CheckItem((USHORT) rItem.eAlign + ITM_HOR_LEFT, TRUE);
    }
    return 0;
}

// --------------------------------------------------------------------------



SfxTabPage* SwEnvPrtPage::Create(Window* pParent, const SfxItemSet& rSet)
{
    return new SwEnvPrtPage(pParent, rSet);
}

// --------------------------------------------------------------------------



void SwEnvPrtPage::ActivatePage(const SfxItemSet& rSet)
{
    if (pPrt)
        aPrinterInfo.SetText(pPrt->GetName());
}

// --------------------------------------------------------------------------



int SwEnvPrtPage::DeactivatePage(SfxItemSet* pSet)
{
    FillItemSet(*pSet);
    return SfxTabPage::LEAVE_PAGE;
}

// --------------------------------------------------------------------------



void SwEnvPrtPage::FillItem(SwEnvItem& rItem)
{
    USHORT nID = 0;
    for (USHORT i = ITM_HOR_LEFT; i <= ITM_VER_RGHT && !nID; i++)
        if (aAlignBox.IsItemChecked(i))
            nID = i;

    rItem.eAlign          = (SwEnvAlign) (nID - ITM_HOR_LEFT);
    rItem.bPrintFromAbove = aTopButton.IsChecked();
    rItem.lShiftRight     = GetFldVal(aRightField);
    rItem.lShiftDown      = GetFldVal(aDownField );
}

// --------------------------------------------------------------------------



BOOL SwEnvPrtPage::FillItemSet(SfxItemSet& rSet)
{
    FillItem(GetParent()->aEnvItem);
    rSet.Put(GetParent()->aEnvItem);
    return TRUE;
}

// ----------------------------------------------------------------------------



void SwEnvPrtPage::Reset(const SfxItemSet& rSet)
{
//    SfxItemSet aSet(rSet);
//    aSet.Put(GetParent()->aEnvItem);

    // Item auslesen
    const SwEnvItem& rItem = (const SwEnvItem&) rSet.Get(FN_ENVELOP);
    aAlignBox.CheckItem((USHORT) rItem.eAlign + ITM_HOR_LEFT);

    if (rItem.bPrintFromAbove)
        aTopButton   .Check();
    else
        aBottomButton.Check();

    SetFldVal(aRightField, rItem.lShiftRight);
    SetFldVal(aDownField , rItem.lShiftDown );

    ActivatePage(rSet);
    ClickHdl(&aTopButton);
}



// ----------------------------------------------------------------------------
/*
$Log: not supported by cvs2svn $
Revision 1.43  2000/09/18 16:05:25  willem.vandorp
OpenOffice header added.

Revision 1.42  2000/02/11 14:45:23  hr
#70473# changes for unicode ( patched by automated patchtool )

Revision 1.41  1999/10/05 10:18:49  os
#67889# some printer problems solved

Revision 1.40  1998/09/08 14:52:44  OS
#56134# Metric fuer Text und HTML getrennt


      Rev 1.39   08 Sep 1998 16:52:44   OS
   #56134# Metric fuer Text und HTML getrennt

      Rev 1.38   16 Feb 1998 15:33:18   RG
   ->DOS-Format

      Rev 1.37   28 Nov 1997 18:18:56   MA
   includes

      Rev 1.36   24 Nov 1997 11:52:10   MA
   includes

      Rev 1.35   03 Nov 1997 13:17:14   MA
   precomp entfernt

      Rev 1.34   01 Oct 1997 10:49:10   TRI
   PrintDialogumstellung

      Rev 1.33   20 Feb 1997 17:51:40   MA
   fix: Deactivate ueber Fill

      Rev 1.32   11 Nov 1996 09:44:16   MA
   ResMgr

      Rev 1.31   29 Oct 1996 17:59:28   HJS
   includes

      Rev 1.30   26 Jul 1996 20:36:36   MA
   includes

      Rev 1.29   06 Feb 1996 15:19:00   JP
   Link Umstellung 305

      Rev 1.28   28 Nov 1995 21:14:58   JP
   UiSystem-Klasse aufgehoben, in initui/swtype aufgeteilt

      Rev 1.27   27 Nov 1995 19:35:48   OM
   HasExchangeSupport->303a

      Rev 1.26   24 Nov 1995 16:59:42   OM
   PCH->PRECOMPILED

      Rev 1.25   13 Nov 1995 13:44:50   MA
   chg: Link-Cast entfernt

      Rev 1.24   10 Nov 1995 18:23:40   OM
   Brief-Image richtig initialisiert

      Rev 1.23   08 Nov 1995 13:48:24   OM
   Change->Set

      Rev 1.22   05 Aug 1995 19:15:40   ER
   segprag

      Rev 1.21   04 Aug 1995 18:32:44   OM
   Umstellung Drucktabpage

      Rev 1.20   03 Aug 1995 18:49:36   OM
   Umstellung Print-TabPage

      Rev 1.19   24 May 1995 18:14:56   ER
   Segmentierung

      Rev 1.18   24 Mar 1995 20:30:38   PK
   geht immer weiter

      Rev 1.17   23 Mar 1995 18:33:22   PK
   geht immer weiter ...

      Rev 1.16   17 Mar 1995 17:10:06   PK
   geht immer weiter

      Rev 1.15   15 Mar 1995 18:37:42   PK
   geht immer weiter

      Rev 1.14   06 Mar 1995 00:08:20   PK
   linkbarer envelp-zustand

      Rev 1.13   04 Mar 1995 22:55:30   PK
   geht immer weiter

      Rev 1.12   21 Feb 1995 15:39:10   PK
   erstmal eingecheckt

      Rev 1.11   25 Oct 1994 17:33:28   ER
   add: PCH

      Rev 1.10   09 Aug 1994 16:12:52   MS
   Entwarnung

      Rev 1.9   04 Aug 1994 09:38:32   SWG
   swg32: SED Size to SSize, LSize to Size etc.

      Rev 1.8   19 Jul 1994 15:14:28   PK
   globale metriken

      Rev 1.7   31 Mar 1994 16:26:36   PK
   (vor)letzter feinschliff beim druck

      Rev 1.6   23 Mar 1994 12:50:28   PK
   bugfix in alignhdl()

      Rev 1.5   22 Mar 1994 17:51:02   PK
   umschlaege vorerst fertig

      Rev 1.4   22 Mar 1994 09:41:34   SWG
   fehlerhafte FUNCDEFS gerichtet

      Rev 1.3   21 Mar 1994 19:02:56   PK
   weiter

      Rev 1.2   09 Mar 1994 19:33:48   PK
   ->update

      Rev 1.1   08 Mar 1994 19:33:04   PK


      Rev 1.0   08 Mar 1994 12:22:24   PK

*/

