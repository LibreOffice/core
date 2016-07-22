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
#ifndef INCLUDED_SW_SOURCE_UI_ENVELP_LABPRT_HXX
#define INCLUDED_SW_SOURCE_UI_ENVELP_LABPRT_HXX

#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <vcl/group.hxx>
#include <vcl/layout.hxx>
#include <svtools/stdctrl.hxx>
#include <sfx2/tabdlg.hxx>

class SwLabDlg;
class SwLabItem;

class SwLabPrtPage : public SfxTabPage
{
    VclPtr<Printer>      pPrinter; // for the shaft setting - unfortunately

    VclPtr<RadioButton>  m_pPageButton;
    VclPtr<RadioButton>  m_pSingleButton;
    VclPtr<VclContainer> m_pSingleGrid;
    VclPtr<VclContainer> m_pPrinterFrame;
    VclPtr<NumericField> m_pColField;
    VclPtr<NumericField> m_pRowField;
    VclPtr<CheckBox>     m_pSynchronCB;

    VclPtr<FixedText>    m_pPrinterInfo;
    VclPtr<PushButton>   m_pPrtSetup;

    DECL_LINK_TYPED( CountHdl, Button *, void );

    SwLabDlg* GetParentSwLabDlg() {return static_cast<SwLabDlg*>(GetParentDialog());}

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:
    SwLabPrtPage(vcl::Window* pParent, const SfxItemSet& rSet);
    virtual ~SwLabPrtPage();
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create(vcl::Window* pParent, const SfxItemSet* rSet);

    virtual void ActivatePage(const SfxItemSet& rSet) override;
    virtual DeactivateRC DeactivatePage(SfxItemSet* pSet) override;
            void FillItem(SwLabItem& rItem);
    virtual bool FillItemSet(SfxItemSet* rSet) override;
    virtual void Reset(const SfxItemSet* rSet) override;
    inline Printer* GetPrt() { return pPrinter; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
