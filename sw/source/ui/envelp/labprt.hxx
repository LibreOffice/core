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
#ifndef _LABPRT_HXX
#define _LABPRT_HXX

#include <vcl/group.hxx>
#include <vcl/field.hxx>
#include <svtools/stdctrl.hxx>
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#include <sfx2/tabdlg.hxx>


class SwLabDlg;
class SwLabItem;

// class SwLabPrtPage -------------------------------------------------------

class SwLabPrtPage : public SfxTabPage
{
    Printer*      pPrinter;             //Fuer die Schachteinstellug - leider.

    RadioButton   aPageButton;
    RadioButton   aSingleButton;
    FixedText     aColText;
    NumericField  aColField;
    FixedText     aRowText;
    NumericField  aRowField;
    CheckBox      aSynchronCB;
    FixedLine     aFLDontKnow;

    FixedInfo     aPrinterInfo;
    PushButton    aPrtSetup;
    FixedLine     aFLPrinter;

     SwLabPrtPage(Window* pParent, const SfxItemSet& rSet);
    ~SwLabPrtPage();

    DECL_LINK( CountHdl, Button * );

    using Window::GetParent;
    SwLabDlg* GetParent() {return (SwLabDlg*) SfxTabPage::GetParent()->GetParent();}

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:

    static SfxTabPage* Create(Window* pParent, const SfxItemSet& rSet);

    virtual void ActivatePage(const SfxItemSet& rSet);
    virtual int  DeactivatePage(SfxItemSet* pSet = 0);
            void FillItem(SwLabItem& rItem);
    virtual sal_Bool FillItemSet(SfxItemSet& rSet);
    virtual void Reset(const SfxItemSet& rSet);
    inline Printer* GetPrt() { return (pPrinter); }
};

#endif


