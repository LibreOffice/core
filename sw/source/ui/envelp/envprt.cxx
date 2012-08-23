/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <vcl/print.hxx>
#include <svtools/prnsetup.hxx>

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

    // Metrics
    FieldUnit eUnit = ::GetDfltMetric(sal_False);
    SetMetric(aRightField, eUnit);
    SetMetric(aDownField , eUnit);

    // Install handlers
    aTopButton   .SetClickHdl(LINK(this, SwEnvPrtPage, ClickHdl));
    aBottomButton.SetClickHdl(LINK(this, SwEnvPrtPage, ClickHdl));

    aPrtSetup    .SetClickHdl(LINK(this, SwEnvPrtPage, ButtonHdl));

    // Bitmaps
    aBottomButton.GetClickHdl().Call(&aBottomButton);

    // ToolBox
    Size aSz = aAlignBox.CalcWindowSizePixel();
    aAlignBox.SetSizePixel(aSz);
    aAlignBox.SetClickHdl(LINK(this, SwEnvPrtPage, AlignHdl));
}

SwEnvPrtPage::~SwEnvPrtPage()
{
}

IMPL_LINK_NOARG(SwEnvPrtPage, ClickHdl)
{
    if (aBottomButton.IsChecked())
    {
        // Envelope from botton
        aAlignBox.SetItemImage(ITM_HOR_LEFT, Bitmap(SW_RES(BMP_HOR_LEFT_LOWER)));
        aAlignBox.SetItemImage(ITM_HOR_CNTR, Bitmap(SW_RES(BMP_HOR_CNTR_LOWER)));
        aAlignBox.SetItemImage(ITM_HOR_RGHT, Bitmap(SW_RES(BMP_HOR_RGHT_LOWER)));
        aAlignBox.SetItemImage(ITM_VER_LEFT, Bitmap(SW_RES(BMP_VER_LEFT_LOWER)));
        aAlignBox.SetItemImage(ITM_VER_CNTR, Bitmap(SW_RES(BMP_VER_CNTR_LOWER)));
        aAlignBox.SetItemImage(ITM_VER_RGHT, Bitmap(SW_RES(BMP_VER_RGHT_LOWER)));
    }
    else
    {
        // Envelope from top
        aAlignBox.SetItemImage(ITM_HOR_LEFT, Bitmap(SW_RES(BMP_HOR_LEFT_UPPER)));
        aAlignBox.SetItemImage(ITM_HOR_CNTR, Bitmap(SW_RES(BMP_HOR_CNTR_UPPER)));
        aAlignBox.SetItemImage(ITM_HOR_RGHT, Bitmap(SW_RES(BMP_HOR_RGHT_UPPER)));
        aAlignBox.SetItemImage(ITM_VER_LEFT, Bitmap(SW_RES(BMP_VER_LEFT_UPPER)));
        aAlignBox.SetItemImage(ITM_VER_CNTR, Bitmap(SW_RES(BMP_VER_CNTR_UPPER)));
        aAlignBox.SetItemImage(ITM_VER_RGHT, Bitmap(SW_RES(BMP_VER_RGHT_UPPER)));
    }
    return 0;
}

IMPL_LINK( SwEnvPrtPage, ButtonHdl, Button *, pBtn )
{
    if (pBtn == &aPrtSetup)
    {
        // Call printer setup
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

IMPL_LINK_NOARG(SwEnvPrtPage, AlignHdl)
{
    if (aAlignBox.GetCurItemId())
    {
        for (sal_uInt16 i = ITM_HOR_LEFT; i <= ITM_VER_RGHT; i++)
            aAlignBox.CheckItem(i, sal_False);
        aAlignBox.CheckItem(aAlignBox.GetCurItemId(), sal_True);
    }
    else
    {
        // GetCurItemId() == 0 is possible!
        const SwEnvItem& rItem = (const SwEnvItem&) GetItemSet().Get(FN_ENVELOP);
        aAlignBox.CheckItem((sal_uInt16) rItem.eAlign + ITM_HOR_LEFT, sal_True);
    }
    return 0;
}

SfxTabPage* SwEnvPrtPage::Create(Window* pParent, const SfxItemSet& rSet)
{
    return new SwEnvPrtPage(pParent, rSet);
}

void SwEnvPrtPage::ActivatePage(const SfxItemSet&)
{
    if (pPrt)
        aPrinterInfo.SetText(pPrt->GetName());
}

int SwEnvPrtPage::DeactivatePage(SfxItemSet* _pSet)
{
    if( _pSet )
        FillItemSet(*_pSet);
    return SfxTabPage::LEAVE_PAGE;
}

void SwEnvPrtPage::FillItem(SwEnvItem& rItem)
{
    sal_uInt16 nID = 0;
    for (sal_uInt16 i = ITM_HOR_LEFT; i <= ITM_VER_RGHT && !nID; i++)
        if (aAlignBox.IsItemChecked(i))
            nID = i;

    rItem.eAlign          = (SwEnvAlign) (nID - ITM_HOR_LEFT);
    rItem.bPrintFromAbove = aTopButton.IsChecked();
    rItem.lShiftRight     = static_cast< sal_Int32 >(GetFldVal(aRightField));
    rItem.lShiftDown      = static_cast< sal_Int32 >(GetFldVal(aDownField ));
}

sal_Bool SwEnvPrtPage::FillItemSet(SfxItemSet& rSet)
{
    FillItem(GetParent()->aEnvItem);
    rSet.Put(GetParent()->aEnvItem);
    return sal_True;
}

void SwEnvPrtPage::Reset(const SfxItemSet& rSet)
{
    // Read item
    const SwEnvItem& rItem = (const SwEnvItem&) rSet.Get(FN_ENVELOP);
    aAlignBox.CheckItem((sal_uInt16) rItem.eAlign + ITM_HOR_LEFT);

    if (rItem.bPrintFromAbove)
        aTopButton   .Check();
    else
        aBottomButton.Check();

    SetFldVal(aRightField, rItem.lShiftRight);
    SetFldVal(aDownField , rItem.lShiftDown );

    ActivatePage(rSet);
    ClickHdl(&aTopButton);
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
