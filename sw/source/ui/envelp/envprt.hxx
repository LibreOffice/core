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



#ifndef _ENVPRT_HXX
#define _ENVPRT_HXX


#include <sfx2/tabdlg.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
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

    DECL_LINK( ClickHdl, Button * );
    DECL_LINK( AlignHdl, ToolBox * );
    DECL_LINK( ButtonHdl, Button * );

    using Window::GetParent;
    SwEnvDlg* GetParent() {return (SwEnvDlg*) SfxTabPage::GetParent()->GetParent();}

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

