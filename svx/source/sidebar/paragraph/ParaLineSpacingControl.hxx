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
#ifndef INCLUDED_SVX_SOURCE_SIDEBAR_PARAGRAPH_PARALINESPACINGCONTROL_HXX
#define INCLUDED_SVX_SOURCE_SIDEBAR_PARAGRAPH_PARALINESPACINGCONTROL_HXX

#include "svx/sidebar/ValueSetWithTextControl.hxx"
#include <sfx2/bindings.hxx>
#include <svtools/ctrlbox.hxx>
#include <svtools/ctrltool.hxx>
#include <vcl/fixed.hxx>
#include <svl/poolitem.hxx>
#include <editeng/lspcitem.hxx>
#include <sfx2/sidebar/EnumContext.hxx>
#include <sfx2/tbxctrl.hxx>

namespace svx {

class ParaLineSpacingControl : public SfxPopupWindow
{
public:
    ParaLineSpacingControl(sal_uInt16 nId, const css::uno::Reference<css::frame::XFrame>& rFrame, vcl::Window* pParentWindow, WinBits nBits);
    virtual ~ParaLineSpacingControl();

    void Rearrange(SfxItemState currSPState,FieldUnit currMetricUnit,SvxLineSpacingItem* currSPItem,const ::sfx2::sidebar::EnumContext currentContext);
    //virtual void Paint(const Rectangle& rect);

    //add
    short GetLastCustomState();
    long  GetLastCustomValue();
    //add end

    void ExecuteLineSpace();
    void SetLineSpace( SvxLineSpacingItem& rLineSpace,
                        int eSpace, long lValue = 0 );

    void ExecuteLineSpacing( bool aIsCustom, sal_uInt16 aEntry );
    void SetAllNoSel();
    void PopupModeEndCallback();

private:
    bool                    mbUseLineSPCustom;
    bool                    mbLineSPDisable;
    SfxMapUnit              m_eLNSpaceUnit;
    SfxBindings*            mpBindings;

    long                    nMinFixDist;
    Edit*                   pActLineDistFld;

    svx::sidebar::ValueSetWithTextControl maLineSpacing;

    FixedText               maCustomFT;
    FixedText               maLSpacingFT;
    ListBox                 aLineDist;

    FixedText               maOfFT;
    MetricField             aLineDistAtPercentBox;
    MetricField             aLineDistAtMetricBox;

    Image                   maSpacing1;
    Image                   maSpacing115;
    Image                   maSpacing15;
    Image                   maSpacing2;

    Image                   maSelSpacing1;
    Image                   maSelSpacing115;
    Image                   maSelSpacing15;
    Image                   maSelSpacing2;
    Image                   maSelSelHanging;
    Image                   maImgCus;
    Image                   maImgCusGrey;
    OUString                maStrCus;

    Image*                  mpImg;
    Image*                  mpImgSel;
    OUString*               mpStr;
    OUString*               mpStrTip;
    OUString                maLine;
    OUString                maOf;

    sal_Int64               maValue;
    sal_uInt16              maPos;
private:
    void initial();
    DECL_LINK(LineSPDistHdl_Impl, ListBox*);
    DECL_LINK(LineSPDistAtHdl_Impl, void*);
    DECL_LINK(VSSelHdl, void*);
};

} // namespace svx

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
