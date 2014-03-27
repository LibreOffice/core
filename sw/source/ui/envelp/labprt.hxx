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

// class SwLabPrtPage -------------------------------------------------------

class SwLabPrtPage : public SfxTabPage
{
    Printer*      pPrinter;             // for the shaft setting - unfortunately

    RadioButton*  m_pPageButton;
    RadioButton*  m_pSingleButton;
    VclContainer* m_pSingleGrid;
    VclContainer* m_pPrinterFrame;
    NumericField* m_pColField;
    NumericField* m_pRowField;
    CheckBox*     m_pSynchronCB;

    FixedText*    m_pPrinterInfo;
    PushButton*   m_pPrtSetup;

     SwLabPrtPage(Window* pParent, const SfxItemSet& rSet);
    ~SwLabPrtPage();

    DECL_LINK( CountHdl, Button * );

    SwLabDlg* GetParentSwLabDlg() {return (SwLabDlg*)GetParentDialog();}

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:

    static SfxTabPage* Create(Window* pParent, const SfxItemSet& rSet);

    virtual void ActivatePage(const SfxItemSet& rSet) SAL_OVERRIDE;
    virtual int  DeactivatePage(SfxItemSet* pSet = 0) SAL_OVERRIDE;
            void FillItem(SwLabItem& rItem);
    virtual bool FillItemSet(SfxItemSet& rSet) SAL_OVERRIDE;
    virtual void Reset(const SfxItemSet& rSet) SAL_OVERRIDE;
    inline Printer* GetPrt() { return (pPrinter); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
