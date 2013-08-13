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

#ifndef _ENVPRT_HXX
#define _ENVPRT_HXX


#include <sfx2/tabdlg.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <vcl/button.hxx>
#include <svtools/stdctrl.hxx>



#include "envimg.hxx"

class SwEnvDlg;

// class SwEnvPrtPage ---------------------------------------------------------

class SwEnvPrtPage : public SfxTabPage
{
    ToolBox*     m_pAlignBox;
    RadioButton* m_pTopButton;
    RadioButton* m_pBottomButton;
    MetricField* m_pRightField;
    MetricField* m_pDownField;
    FixedText*   m_pPrinterInfo;
    PushButton*  m_pPrtSetup;

    sal_uInt16 m_aIds[6];

    Printer* pPrt;

    SwEnvPrtPage(Window* pParent, const SfxItemSet& rSet);

    DECL_LINK(ClickHdl, void *);
    DECL_LINK(AlignHdl, void *);
    DECL_LINK( ButtonHdl, Button * );

    SwEnvDlg* GetParentSwEnvDlg() {return (SwEnvDlg*) GetParentDialog();}

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:

    static SfxTabPage* Create(Window* pParent, const SfxItemSet& rSet);

    virtual void ActivatePage(const SfxItemSet& rSet);
    virtual int  DeactivatePage(SfxItemSet* pSet = 0);
            void FillItem(SwEnvItem& rItem);
    virtual sal_Bool FillItemSet(SfxItemSet& rSet);
    virtual void Reset(const SfxItemSet& rSet);

    inline void SetPrt(Printer* pPrinter) { pPrt = pPrinter; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
