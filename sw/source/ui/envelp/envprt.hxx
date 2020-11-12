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

#ifndef INCLUDED_SW_SOURCE_UI_ENVELP_ENVPRT_HXX
#define INCLUDED_SW_SOURCE_UI_ENVELP_ENVPRT_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/print.hxx>
#include <vcl/weld.hxx>

#include <envimg.hxx>
#include <envlop.hxx>

class SwEnvDlg;

class SwEnvPrtPage : public SfxTabPage
{
    std::unique_ptr<weld::Widget> m_xUpper;
    std::unique_ptr<weld::Widget> m_xLower;
    std::unique_ptr<weld::RadioButton> m_xTopButton;
    std::unique_ptr<weld::RadioButton> m_xBottomButton;
    std::unique_ptr<weld::MetricSpinButton> m_xRightField;
    std::unique_ptr<weld::MetricSpinButton> m_xDownField;
    std::unique_ptr<weld::Label> m_xPrinterInfo;
    std::unique_ptr<weld::Button> m_xPrtSetup;

    std::unique_ptr<weld::RadioButton> m_aIdsL[ENV_VER_RGHT - ENV_HOR_LEFT + 1];
    std::unique_ptr<weld::RadioButton> m_aIdsU[ENV_VER_RGHT - ENV_HOR_LEFT + 1];

    VclPtr<Printer> m_xPrt;

    DECL_LINK(LowerHdl, weld::ToggleButton&, void);
    DECL_LINK(UpperHdl, weld::ToggleButton&, void);
    DECL_LINK(ClickHdl, weld::ToggleButton&, void);
    DECL_LINK(ButtonHdl, weld::Button&, void);

    SwEnvDlg* GetParentSwEnvDlg() { return static_cast<SwEnvDlg*>(GetDialogController()); }

public:
    SwEnvPrtPage(weld::Container* pPage, weld::DialogController* pController,
                 const SfxItemSet& rSet);
    virtual ~SwEnvPrtPage() override;

    static std::unique_ptr<SfxTabPage>
    Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rSet);

    virtual void ActivatePage(const SfxItemSet& rSet) override;
    virtual DeactivateRC DeactivatePage(SfxItemSet* pSet) override;
    void FillItem(SwEnvItem& rItem);
    virtual bool FillItemSet(SfxItemSet* rSet) override;
    virtual void Reset(const SfxItemSet* rSet) override;

    void SetPrt(Printer* pPrinter) { m_xPrt = pPrinter; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
