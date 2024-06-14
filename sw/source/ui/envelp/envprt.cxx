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
#include <svtools/unitconv.hxx>

#include <cmdid.h>
#include "envprt.hxx"
#include <envlop.hxx>
#include <uitool.hxx>

SwEnvPrtPage::SwEnvPrtPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, u"modules/swriter/ui/envprinterpage.ui"_ustr, u"EnvPrinterPage"_ustr, &rSet)
    , m_xUpper(m_xBuilder->weld_widget(u"upper"_ustr))
    , m_xLower(m_xBuilder->weld_widget(u"lower"_ustr))
    , m_xTopButton(m_xBuilder->weld_radio_button(u"top"_ustr))
    , m_xBottomButton(m_xBuilder->weld_radio_button(u"bottom"_ustr))
    , m_xRightField(m_xBuilder->weld_metric_spin_button(u"right"_ustr, FieldUnit::CM))
    , m_xDownField(m_xBuilder->weld_metric_spin_button(u"down"_ustr, FieldUnit::CM))
    , m_xPrinterInfo(m_xBuilder->weld_label(u"printername"_ustr))
    , m_xPrtSetup(m_xBuilder->weld_button(u"setup"_ustr))
    , m_aIdsL { m_xBuilder->weld_radio_button(u"horileftl"_ustr),
                m_xBuilder->weld_radio_button(u"horicenterl"_ustr),
                m_xBuilder->weld_radio_button(u"horirightl"_ustr),
                m_xBuilder->weld_radio_button(u"vertleftl"_ustr),
                m_xBuilder->weld_radio_button(u"vertcenterl"_ustr),
                m_xBuilder->weld_radio_button(u"vertrightl"_ustr) }
    , m_aIdsU { m_xBuilder->weld_radio_button(u"horileftu"_ustr),
                m_xBuilder->weld_radio_button(u"horicenteru"_ustr),
                m_xBuilder->weld_radio_button(u"horirightu"_ustr),
                m_xBuilder->weld_radio_button(u"vertleftu"_ustr),
                m_xBuilder->weld_radio_button(u"vertcenteru"_ustr),
                m_xBuilder->weld_radio_button(u"vertrightu"_ustr) }
{
    SetExchangeSupport();

    // Metrics
    FieldUnit eUnit = ::GetDfltMetric(false);
    ::SetFieldUnit(*m_xRightField, eUnit);
    ::SetFieldUnit(*m_xDownField, eUnit);

    // Install handlers
    m_xTopButton->connect_toggled(LINK(this, SwEnvPrtPage, ClickHdl));
    m_xBottomButton->connect_toggled(LINK(this, SwEnvPrtPage, ClickHdl));

    m_xPrtSetup->connect_clicked(LINK(this, SwEnvPrtPage, ButtonHdl));

    for (auto& a : m_aIdsL)
        a->connect_toggled(LINK(this, SwEnvPrtPage, LowerHdl));
    for (auto& a : m_aIdsU)
        a->connect_toggled(LINK(this, SwEnvPrtPage, UpperHdl));

    // Bitmaps
    ClickHdl(*m_xBottomButton);
}

SwEnvPrtPage::~SwEnvPrtPage()
{
    m_xPrt.clear();
}

IMPL_LINK_NOARG(SwEnvPrtPage, ClickHdl, weld::Toggleable&, void)
{
    // Envelope from bottom, otherwise Envelope from top
    const bool bLowerActive = m_xBottomButton->get_active();
    m_xUpper->set_visible(!bLowerActive);
    m_xLower->set_visible(bLowerActive);
}

IMPL_LINK(SwEnvPrtPage, LowerHdl, weld::Toggleable&, rButton, void)
{
    for (int i = ENV_HOR_LEFT; i <= ENV_VER_RGHT; ++i)
    {
        if (&rButton == m_aIdsL[i].get())
        {
            m_aIdsU[i]->set_active(m_aIdsL[i]->get_active());
            break;
        }
    }
}

IMPL_LINK(SwEnvPrtPage, UpperHdl, weld::Toggleable&, rButton, void)
{
    for (int i = ENV_HOR_LEFT; i <= ENV_VER_RGHT; ++i)
    {
        if (&rButton == m_aIdsU[i].get())
        {
            m_aIdsL[i]->set_active(m_aIdsU[i]->get_active());
            break;
        }
    }
}

IMPL_LINK(SwEnvPrtPage, ButtonHdl, weld::Button&, rBtn, void)
{
    if (&rBtn == m_xPrtSetup.get())
    {
        // Call printer setup
        if (m_xPrt)
        {
            PrinterSetupDialog aDlg(GetFrameWeld());
            aDlg.SetPrinter(m_xPrt);
            aDlg.run();
            rBtn.grab_focus();
            m_xPrinterInfo->set_label(m_xPrt->GetName());
        }
    }
}

std::unique_ptr<SfxTabPage> SwEnvPrtPage::Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rSet)
{
    return std::make_unique<SwEnvPrtPage>(pPage, pController, *rSet);
}

void SwEnvPrtPage::ActivatePage(const SfxItemSet&)
{
    if (m_xPrt)
        m_xPrinterInfo->set_label(m_xPrt->GetName());
}

DeactivateRC SwEnvPrtPage::DeactivatePage(SfxItemSet* _pSet)
{
    if( _pSet )
        FillItemSet(_pSet);
    return DeactivateRC::LeavePage;
}

void SwEnvPrtPage::FillItem(SwEnvItem& rItem)
{
    int nOrient = 0;
    for (int i = ENV_HOR_LEFT; i <= ENV_VER_RGHT; ++i)
    {
        assert(m_aIdsL[i]->get_active() == m_aIdsU[i]->get_active());
        if (m_aIdsL[i]->get_active())
        {
            nOrient = i;
            break;
        }
    }

    rItem.m_eAlign          = static_cast<SwEnvAlign>(nOrient);
    rItem.m_bPrintFromAbove = m_xTopButton->get_active();
    rItem.m_nShiftRight     = getfieldval(*m_xRightField);
    rItem.m_nShiftDown      = getfieldval(*m_xDownField);
}

bool SwEnvPrtPage::FillItemSet(SfxItemSet* rSet)
{
    FillItem(GetParentSwEnvDlg()->m_aEnvItem);
    rSet->Put(GetParentSwEnvDlg()->m_aEnvItem);
    return true;
}

void SwEnvPrtPage::Reset(const SfxItemSet* rSet)
{
    // Read item
    const SwEnvItem& rItem = static_cast<const SwEnvItem&>( rSet->Get(FN_ENVELOP) );
    m_aIdsL[rItem.m_eAlign]->set_active(true);
    m_aIdsU[rItem.m_eAlign]->set_active(true);

    if (rItem.m_bPrintFromAbove)
        m_xTopButton->set_active(true);
    else
        m_xBottomButton->set_active(true);

    setfieldval(*m_xRightField, rItem.m_nShiftRight);
    setfieldval(*m_xDownField , rItem.m_nShiftDown );

    ActivatePage(*rSet);
    ClickHdl(*m_xTopButton);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
