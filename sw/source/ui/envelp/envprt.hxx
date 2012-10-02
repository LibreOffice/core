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
    ToolBox      aAlignBox;
    RadioButton  aTopButton;
    RadioButton  aBottomButton;
    FixedText    aRightText;
    MetricField  aRightField;
    FixedText    aDownText;
    MetricField  aDownField;
    FixedInfo    aPrinterInfo;
    FixedLine    aNoNameFL;
    FixedLine    aPrinterFL;
    PushButton   aPrtSetup;

    Printer* pPrt;

     SwEnvPrtPage(Window* pParent, const SfxItemSet& rSet);
    ~SwEnvPrtPage();

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
