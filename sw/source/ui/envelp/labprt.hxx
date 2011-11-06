/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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

    FixedLine     aFLDontKnow;
    RadioButton   aPageButton;
    RadioButton   aSingleButton;
    FixedText     aColText;
    NumericField  aColField;
    FixedText     aRowText;
    NumericField  aRowField;
    CheckBox      aSynchronCB;

    FixedLine     aFLPrinter;
    FixedInfo     aPrinterInfo;
    PushButton    aPrtSetup;

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


