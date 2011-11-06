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


#ifndef _ENVFMT_HXX
#define _ENVFMT_HXX


#ifndef _SVSTDARR_HXX
#define _SVSTDARR_USHORTS
#include <svl/svstdarr.hxx>
#endif
#include <svtools/stdctrl.hxx>
#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _MENUBTN_HXX //autogen
#include <vcl/menubtn.hxx>
#endif
#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#include "envlop.hxx"

class SwTxtFmtColl;

// class SwEnvFmtPage ---------------------------------------------------------

class SwEnvFmtPage : public SfxTabPage
{

    FixedLine    aAddrFL;
    FixedInfo    aAddrPosInfo;
    FixedText    aAddrLeftText;
    MetricField  aAddrLeftField;
    FixedText    aAddrTopText;
    MetricField  aAddrTopField;
    FixedInfo    aAddrFormatInfo;
    MenuButton   aAddrEditButton;
    FixedLine    aSendFL;
    FixedInfo    aSendPosInfo;
    FixedText    aSendLeftText;
    MetricField  aSendLeftField;
    FixedText    aSendTopText;
    MetricField  aSendTopField;
    FixedInfo    aSendFormatInfo;
    MenuButton   aSendEditButton;
    FixedLine    aSizeFL;
    FixedText    aSizeFormatText;
    ListBox      aSizeFormatBox;
    FixedText    aSizeWidthText;
    MetricField  aSizeWidthField;
    FixedText    aSizeHeightText;
    MetricField  aSizeHeightField;
    SwEnvPreview aPreview;

    SvUShorts  aIDs;

     SwEnvFmtPage(Window* pParent, const SfxItemSet& rSet);
    ~SwEnvFmtPage();

    DECL_LINK( ModifyHdl, Edit * );
    DECL_LINK( EditHdl, MenuButton * );
    DECL_LINK( FormatHdl, ListBox * );

    void SetMinMax();

    SfxItemSet  *GetCollItemSet(SwTxtFmtColl* pColl, sal_Bool bSender);

    using Window::GetParent;
    SwEnvDlg    *GetParent() {return (SwEnvDlg*) SfxTabPage::GetParent()->GetParent();}

    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:

    static SfxTabPage* Create(Window* pParent, const SfxItemSet& rSet);

    virtual void ActivatePage(const SfxItemSet& rSet);
    virtual int  DeactivatePage(SfxItemSet* pSet = 0);
            void FillItem(SwEnvItem& rItem);
    virtual sal_Bool FillItemSet(SfxItemSet& rSet);
    virtual void Reset(const SfxItemSet& rSet);
};

#endif


