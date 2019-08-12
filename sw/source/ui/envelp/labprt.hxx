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

#include <sfx2/tabdlg.hxx>

#include <label.hxx>

class SwLabDlg;
class SwLabItem;

class SwLabPrtPage : public SfxTabPage
{
    VclPtr<Printer>      pPrinter; // for the shaft setting - unfortunately

    std::unique_ptr<weld::RadioButton>  m_xPageButton;
    std::unique_ptr<weld::RadioButton>  m_xSingleButton;
    std::unique_ptr<weld::Widget> m_xSingleGrid;
    std::unique_ptr<weld::Widget> m_xPrinterFrame;
    std::unique_ptr<weld::SpinButton> m_xColField;
    std::unique_ptr<weld::SpinButton> m_xRowField;
    std::unique_ptr<weld::CheckButton>  m_xSynchronCB;
    std::unique_ptr<weld::Label>  m_xPrinterInfo;
    std::unique_ptr<weld::Button> m_xPrtSetup;

    DECL_LINK( CountHdl, weld::Button&, void );

    SwLabDlg* GetParentSwLabDlg() {return static_cast<SwLabDlg*>(GetDialogController());}

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:
    SwLabPrtPage(TabPageParent pParent, const SfxItemSet& rSet);
    virtual ~SwLabPrtPage() override;
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create(TabPageParent pParent, const SfxItemSet* rSet);

    virtual void ActivatePage(const SfxItemSet& rSet) override;
    virtual DeactivateRC DeactivatePage(SfxItemSet* pSet) override;
            void FillItem(SwLabItem& rItem);
    virtual bool FillItemSet(SfxItemSet* rSet) override;
    virtual void Reset(const SfxItemSet* rSet) override;
    Printer* GetPrt() { return pPrinter; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
