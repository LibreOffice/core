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

#include <vcl/print.hxx>
#include <svtools/prnsetup.hxx>

#include "swtypes.hxx"
#include "cmdid.h"
#include "envprt.hxx"
#include "envlop.hxx"
#include "uitool.hxx"

#include "envprt.hrc"

SwEnvPrtPage::SwEnvPrtPage(Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "EnvPrinterPage",
        "modules/swriter/ui/envprinterpage.ui", rSet)
{
    get(m_pAlignBox, "alignbox");
    get(m_pTopButton, "top");
    get(m_pBottomButton, "bottom");
    get(m_pRightField, "right");
    get(m_pDownField, "down");
    get(m_pPrinterInfo, "printername");
    get(m_pPrtSetup, "setup");
    SetExchangeSupport();

    // Metrics
    FieldUnit eUnit = ::GetDfltMetric(sal_False);
    SetMetric(*m_pRightField, eUnit);
    SetMetric(*m_pDownField, eUnit);

    // Install handlers
    m_pTopButton->SetClickHdl(LINK(this, SwEnvPrtPage, ClickHdl));
    m_pBottomButton->SetClickHdl(LINK(this, SwEnvPrtPage, ClickHdl));

    m_pPrtSetup->SetClickHdl(LINK(this, SwEnvPrtPage, ButtonHdl));

    // Bitmaps
    m_pBottomButton->GetClickHdl().Call(m_pBottomButton);

    // ToolBox
    m_pAlignBox->SetClickHdl(LINK(this, SwEnvPrtPage, AlignHdl));
    m_aIds[ENV_HOR_LEFT] = m_pAlignBox->GetItemId("horileft");
    m_aIds[ENV_HOR_CNTR] = m_pAlignBox->GetItemId("horicenter");
    m_aIds[ENV_HOR_RGHT] = m_pAlignBox->GetItemId("horiright");
    m_aIds[ENV_VER_LEFT] = m_pAlignBox->GetItemId("vertleft");
    m_aIds[ENV_VER_CNTR] = m_pAlignBox->GetItemId("vertcenter");
    m_aIds[ENV_VER_RGHT] = m_pAlignBox->GetItemId("vertright");
}

IMPL_LINK_NOARG(SwEnvPrtPage, ClickHdl)
{
    if (m_pBottomButton->IsChecked())
    {
        // Envelope from botton
        m_pAlignBox->SetItemImage(m_aIds[ENV_HOR_LEFT], Bitmap(SW_RES(BMP_HOR_LEFT_LOWER)));
        m_pAlignBox->SetItemImage(m_aIds[ENV_HOR_CNTR], Bitmap(SW_RES(BMP_HOR_CNTR_LOWER)));
        m_pAlignBox->SetItemImage(m_aIds[ENV_HOR_RGHT], Bitmap(SW_RES(BMP_HOR_RGHT_LOWER)));
        m_pAlignBox->SetItemImage(m_aIds[ENV_VER_LEFT], Bitmap(SW_RES(BMP_VER_LEFT_LOWER)));
        m_pAlignBox->SetItemImage(m_aIds[ENV_VER_CNTR], Bitmap(SW_RES(BMP_VER_CNTR_LOWER)));
        m_pAlignBox->SetItemImage(m_aIds[ENV_VER_RGHT], Bitmap(SW_RES(BMP_VER_RGHT_LOWER)));
    }
    else
    {
        // Envelope from top
        m_pAlignBox->SetItemImage(m_aIds[ENV_HOR_LEFT], Bitmap(SW_RES(BMP_HOR_LEFT_UPPER)));
        m_pAlignBox->SetItemImage(m_aIds[ENV_HOR_CNTR], Bitmap(SW_RES(BMP_HOR_CNTR_UPPER)));
        m_pAlignBox->SetItemImage(m_aIds[ENV_HOR_RGHT], Bitmap(SW_RES(BMP_HOR_RGHT_UPPER)));
        m_pAlignBox->SetItemImage(m_aIds[ENV_VER_LEFT], Bitmap(SW_RES(BMP_VER_LEFT_UPPER)));
        m_pAlignBox->SetItemImage(m_aIds[ENV_VER_CNTR], Bitmap(SW_RES(BMP_VER_CNTR_UPPER)));
        m_pAlignBox->SetItemImage(m_aIds[ENV_VER_RGHT], Bitmap(SW_RES(BMP_VER_RGHT_UPPER)));
    }
    return 0;
}

IMPL_LINK( SwEnvPrtPage, ButtonHdl, Button *, pBtn )
{
    if (pBtn == m_pPrtSetup)
    {
        // Call printer setup
        if (pPrt)
        {
            PrinterSetupDialog* pDlg = new PrinterSetupDialog(this );
            pDlg->SetPrinter(pPrt);
            pDlg->Execute();
            delete pDlg;
            GrabFocus();
            m_pPrinterInfo->SetText(pPrt->GetName());
        }
    }
    return 0;
}

IMPL_LINK_NOARG(SwEnvPrtPage, AlignHdl)
{
    if (m_pAlignBox->GetCurItemId())
    {
        for (sal_uInt16 i = ENV_HOR_LEFT; i <= ENV_VER_RGHT; ++i)
            m_pAlignBox->CheckItem(m_aIds[i], false);
        m_pAlignBox->CheckItem(m_pAlignBox->GetCurItemId(), true);
    }
    else
    {
        // GetCurItemId() == 0 is possible!
        const SwEnvItem& rItem = (const SwEnvItem&) GetItemSet().Get(FN_ENVELOP);
        m_pAlignBox->CheckItem(m_aIds[rItem.eAlign], true);
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
        m_pPrinterInfo->SetText(pPrt->GetName());
}

int SwEnvPrtPage::DeactivatePage(SfxItemSet* _pSet)
{
    if( _pSet )
        FillItemSet(*_pSet);
    return SfxTabPage::LEAVE_PAGE;
}

void SwEnvPrtPage::FillItem(SwEnvItem& rItem)
{
    sal_uInt16 nOrient = 0;
    for (sal_uInt16 i = ENV_HOR_LEFT; i <= ENV_VER_RGHT; ++i)
    {
        if (m_pAlignBox->IsItemChecked(m_aIds[i]))
        {
            nOrient = i;
            break;
        }
    }

    rItem.eAlign          = (SwEnvAlign) (nOrient);
    rItem.bPrintFromAbove = m_pTopButton->IsChecked();
    rItem.lShiftRight     = static_cast< sal_Int32 >(GetFldVal(*m_pRightField));
    rItem.lShiftDown      = static_cast< sal_Int32 >(GetFldVal(*m_pDownField ));
}

sal_Bool SwEnvPrtPage::FillItemSet(SfxItemSet& rSet)
{
    FillItem(GetParentSwEnvDlg()->aEnvItem);
    rSet.Put(GetParentSwEnvDlg()->aEnvItem);
    return sal_True;
}

void SwEnvPrtPage::Reset(const SfxItemSet& rSet)
{
    // Read item
    const SwEnvItem& rItem = (const SwEnvItem&) rSet.Get(FN_ENVELOP);
    m_pAlignBox->CheckItem(m_aIds[rItem.eAlign]);

    if (rItem.bPrintFromAbove)
        m_pTopButton->Check();
    else
        m_pBottomButton->Check();

    SetFldVal(*m_pRightField, rItem.lShiftRight);
    SetFldVal(*m_pDownField , rItem.lShiftDown );

    ActivatePage(rSet);
    ClickHdl(m_pTopButton);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
