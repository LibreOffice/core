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

#define SIDEBAR_SPACING_GLOBAL_VALUE   "PopupPanal_Spacing"

#define SIDEBAR_SPACE_NORMAL    0
#define SIDEBAR_SPACE_EXPAND    1
#define SIDEBAR_SPACE_CONDENSED 2

class TextCharacterSpacingControl:public svx::sidebar::PopupControl
{
public:
    TextCharacterSpacingControl (
        Window* pParent,
        svx::sidebar::TextPropertyPanel& rPanel,
        SfxBindings* pBindings);
    virtual ~TextCharacterSpacingControl();
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

    FixedText           maFTSpacing;
    ListBox             maLBKerning;
    FixedText           maFTBy;
    MetricField         maEditKerning;

    Image*              mpImg;
    Image*              mpImgSel;
    OUString*           mpStr;
    OUString*           mpStrTip;

    Image               maImgCus;
    Image               maImgCusGrey;
    OUString            maStrCus;
    OUString            maStrCusE;
    OUString            maStrCusC;
    OUString            maStrCusN;
    OUString            maStrUnit;

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
