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
#ifndef INCLUDED_SVX_SOURCE_SIDEBAR_TEXT_TEXTCHARACTERSPACINGCONTROL_HXX
#define INCLUDED_SVX_SOURCE_SIDEBAR_TEXT_TEXTCHARACTERSPACINGCONTROL_HXX

#include "svx/sidebar/PopupControl.hxx"
#include "svx/sidebar/ValueSetWithTextControl.hxx"
#include <sfx2/bindings.hxx>
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
        vcl::Window* pParent,
        svx::sidebar::TextPropertyPanel& rPanel,
        SfxBindings* pBindings);
    virtual ~TextCharacterSpacingControl();
    virtual void dispose() SAL_OVERRIDE;
    void Rearrange(bool bLBAvailable,bool bAvailable, long nKerning);
    //virtual void Paint(const Rectangle& rect);

    //add
    short GetLastCustomState() { return mnLastCus;}
    long  GetLastCustomValue() { return mnCustomKern;}
    //add end

private:
    svx::sidebar::TextPropertyPanel&     mrTextPropertyPanel;
    SfxBindings*        mpBindings;

    VclPtr<ValueSetWithTextControl> maVSSpacing;

    VclPtr<FixedText>   maLastCus;

    VclPtr<FixedText>   maFTSpacing;
    VclPtr<ListBox>     maLBKerning;
    VclPtr<FixedText>   maFTBy;
    VclPtr<MetricField> maEditKerning;

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
    DECL_LINK_TYPED(VSSelHdl, ValueSet*, void);
    DECL_LINK_TYPED(KerningSelectHdl, ListBox&, void);
    DECL_LINK(KerningModifyHdl,  MetricField*);
};
}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
