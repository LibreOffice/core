/*************************************************************************
 *
 *  $RCSfile: envprt.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: os $ $Date: 2002-12-05 12:40:34 $
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
    aNoNameFL    (this, SW_RES(FL_NONAME )),
    aPrinterFL   (this, SW_RES(FL_PRINTER )),
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
//    aAlignBox.SetPosPixel(Point(aNoNameFL.GetPosPixel().X() + (aNoNameFL.GetSizePixel().Width() - aSz.Width()) / 2, aAlignBox.GetPosPixel().Y()));
    aAlignBox.SetClickHdl(LINK(this, SwEnvPrtPage, AlignHdl));

}

// --------------------------------------------------------------------------



SwEnvPrtPage::~SwEnvPrtPage()
{
}

// --------------------------------------------------------------------------



IMPL_LINK( SwEnvPrtPage, ClickHdl, Button *, EMPTYARG )
{
    sal_Bool bHC = GetDisplayBackground().GetColor().IsDark();
    if (aBottomButton.IsChecked())
    {
        // Briefumschlaege von unten
        aAlignBox.SetItemImage(ITM_HOR_LEFT, Bitmap(SW_RES(bHC ? BMP_HOR_LEFT_LOWER_H : BMP_HOR_LEFT_LOWER)));
        aAlignBox.SetItemImage(ITM_HOR_CNTR, Bitmap(SW_RES(bHC ? BMP_HOR_CNTR_LOWER_H : BMP_HOR_CNTR_LOWER)));
        aAlignBox.SetItemImage(ITM_HOR_RGHT, Bitmap(SW_RES(bHC ? BMP_HOR_RGHT_LOWER_H : BMP_HOR_RGHT_LOWER)));
        aAlignBox.SetItemImage(ITM_VER_LEFT, Bitmap(SW_RES(bHC ? BMP_VER_LEFT_LOWER_H : BMP_VER_LEFT_LOWER)));
        aAlignBox.SetItemImage(ITM_VER_CNTR, Bitmap(SW_RES(bHC ? BMP_VER_CNTR_LOWER_H : BMP_VER_CNTR_LOWER)));
        aAlignBox.SetItemImage(ITM_VER_RGHT, Bitmap(SW_RES(bHC ? BMP_VER_RGHT_LOWER_H : BMP_VER_RGHT_LOWER)));
    }
    else
    {
        // Briefumschlaege von oben
        aAlignBox.SetItemImage(ITM_HOR_LEFT, Bitmap(SW_RES(bHC ? BMP_HOR_LEFT_UPPER_H : BMP_HOR_LEFT_UPPER)));
        aAlignBox.SetItemImage(ITM_HOR_CNTR, Bitmap(SW_RES(bHC ? BMP_HOR_CNTR_UPPER_H : BMP_HOR_CNTR_UPPER)));
        aAlignBox.SetItemImage(ITM_HOR_RGHT, Bitmap(SW_RES(bHC ? BMP_HOR_RGHT_UPPER_H : BMP_HOR_RGHT_UPPER)));
        aAlignBox.SetItemImage(ITM_VER_LEFT, Bitmap(SW_RES(bHC ? BMP_VER_LEFT_UPPER_H : BMP_VER_LEFT_UPPER)));
        aAlignBox.SetItemImage(ITM_VER_CNTR, Bitmap(SW_RES(bHC ? BMP_VER_CNTR_UPPER_H : BMP_VER_CNTR_UPPER)));
        aAlignBox.SetItemImage(ITM_VER_RGHT, Bitmap(SW_RES(bHC ? BMP_VER_RGHT_UPPER_H : BMP_VER_RGHT_UPPER)));
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
    if( pSet )
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




