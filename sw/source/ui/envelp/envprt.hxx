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
#include <vcl/toolbox.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <vcl/button.hxx>
#include <svtools/stdctrl.hxx>

#include "envimg.hxx"
#include "envlop.hxx"

class SwEnvDlg;

class SwEnvPrtPage : public SfxTabPage
{
    VclPtr<ToolBox>     m_pAlignBox;
    VclPtr<RadioButton> m_pTopButton;
    VclPtr<RadioButton> m_pBottomButton;
    VclPtr<MetricField> m_pRightField;
    VclPtr<MetricField> m_pDownField;
    VclPtr<FixedText>   m_pPrinterInfo;
    VclPtr<PushButton>  m_pPrtSetup;

    sal_uInt16 m_aIds[ENV_VER_RGHT-ENV_HOR_LEFT+1];

    VclPtr<Printer>     pPrt;

    DECL_LINK_TYPED(ClickHdl, Button*, void);
    DECL_LINK_TYPED(AlignHdl, ToolBox *, void);
    DECL_LINK_TYPED(ButtonHdl, Button *, void );

    SwEnvDlg* GetParentSwEnvDlg() {return static_cast<SwEnvDlg*>( GetParentDialog());}

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:
    SwEnvPrtPage(vcl::Window* pParent, const SfxItemSet& rSet);
    virtual ~SwEnvPrtPage();
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create(vcl::Window* pParent, const SfxItemSet* rSet);

    virtual void ActivatePage(const SfxItemSet& rSet) override;
    virtual DeactivateRC DeactivatePage(SfxItemSet* pSet) override;
            void FillItem(SwEnvItem& rItem);
    virtual bool FillItemSet(SfxItemSet* rSet) override;
    virtual void Reset(const SfxItemSet* rSet) override;

    inline void SetPrt(Printer* pPrinter) { pPrt = pPrinter; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
