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
#ifndef _SVX_SIDEBAR_CHARACTER_SPACING_CONTROL_HXX_
#define _SVX_SIDEBAR_CHARACTER_SPACING_CONTROL_HXX_

#include "svx/sidebar/PopupControl.hxx"
#include "svx/sidebar/ValueSetWithTextControl.hxx"
#include <sfx2/bindings.hxx>
#include <svtools/ctrlbox.hxx>
#include <svtools/ctrltool.hxx>
#include "TextPropertyPanel.hxx"
#include <vcl/fixed.hxx>


namespace svx { namespace sidebar {
#define SPACING_NOCUSTOM                0
#define SPACING_CLOSE_BY_CLICK_ICON     -1
#define SPACING_CLOSE_BY_CUS_EDIT       1

#define SIDEBAR_SPACING_GLOBAL_VALUE    String("PopupPanal_Spacing", 18, RTL_TEXTENCODING_ASCII_US)

#define SIDEBAR_SPACE_NORMAL    0
#define SIDEBAR_SPACE_EXPAND    1
#define SIDEBAR_SPACE_CONDENSED 2
class TextCharacterSpacingControl:public svx::sidebar::PopupControl
{
public:
    TextCharacterSpacingControl(Window* pParent, svx::sidebar::TextPropertyPanel& rPanel);
    ~TextCharacterSpacingControl();
    void ToGetFocus();
    void Rearrange(bool bLBAvailable,bool bAvailable, long nKerning);
    //virtual void Paint(const Rectangle& rect);

    //add
    short GetLastCustomState();
    long  GetLastCustomValue();
    //add end

private:
    svx::sidebar::TextPropertyPanel&     mrTextPropertyPanel;
    SfxBindings*        mpBindings;

    ValueSetWithTextControl maVSSpacing;

    FixedText           maLastCus;
//  Control             maBorder;

    FixedText           maFTSpacing;
    ListBox             maLBKerning;
    FixedText           maFTBy;
    MetricField         maEditKerning;

    Image*              mpImg;
    Image*              mpImgSel;
    XubString*          mpStr;
    XubString*          mpStrTip;

    Image               maImgCus;
    Image               maImgCusGrey;
    XubString           maStrCus;
    XubString           maStrCusE;      //add
    XubString           maStrCusC;      //add
    XubString           maStrCusN;      //add
    XubString           maStrUnit;      //add

    long                mnCustomKern;
    short               mnLastCus;
    bool                mbCusEnable;
    bool                mbVS;

    void initial();
    DECL_LINK(VSSelHdl, void*);
    DECL_LINK(KerningSelectHdl, ListBox*);
    DECL_LINK(KerningModifyHdl,  MetricField*);
};
}}

#endif
