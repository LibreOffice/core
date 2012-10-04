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
#ifndef _SVX_DSTRIBUT_HXX
#define _SVX_DSTRIBUT_HXX

#include <svx/dlgctrl.hxx>
#include <vcl/group.hxx>
#include <vcl/fixed.hxx>
#include <svx/dstribut_enum.hxx>

class SvxDistributePage : public SvxTabPage
{
    SvxDistributeHorizontal     meDistributeHor;
    SvxDistributeVertical       meDistributeVer;

    FixedLine                   maFlHorizontal      ;
    RadioButton                 maBtnHorNone        ;
    RadioButton                 maBtnHorLeft        ;
    RadioButton                 maBtnHorCenter      ;
    RadioButton                 maBtnHorDistance    ;
    RadioButton                 maBtnHorRight       ;
    FixedImage                  maHorLow            ;
    FixedImage                  maHorCenter         ;
    FixedImage                  maHorDistance       ;
    FixedImage                  maHorHigh           ;
    FixedLine                   maFlVertical        ;
    RadioButton                 maBtnVerNone        ;
    RadioButton                 maBtnVerTop         ;
    RadioButton                 maBtnVerCenter      ;
    RadioButton                 maBtnVerDistance    ;
    RadioButton                 maBtnVerBottom      ;
    FixedImage                  maVerLow            ;
    FixedImage                  maVerCenter         ;
    FixedImage                  maVerDistance       ;
    FixedImage                  maVerHigh           ;

public:
    SvxDistributePage(Window* pWindow, const SfxItemSet& rInAttrs,
        SvxDistributeHorizontal eHor = SvxDistributeHorizontalNone,
        SvxDistributeVertical eVer = SvxDistributeVerticalNone);
    ~SvxDistributePage();

    static SfxTabPage* Create(Window*, const SfxItemSet&,
        SvxDistributeHorizontal eHor, SvxDistributeVertical eVer);
    static sal_uInt16* GetRanges();
    virtual sal_Bool FillItemSet(SfxItemSet&);
    virtual void Reset(const SfxItemSet&);
    virtual void PointChanged(Window* pWindow, RECT_POINT eRP);

    SvxDistributeHorizontal GetDistributeHor() const { return meDistributeHor; }
    SvxDistributeVertical GetDistributeVer() const { return meDistributeVer; }
};

class SvxDistributeDialog : public SfxSingleTabDialog
{
    SvxDistributePage*          mpPage;

public:
    SvxDistributeDialog(Window* pParent, const SfxItemSet& rAttr,
        SvxDistributeHorizontal eHor = SvxDistributeHorizontalNone,
        SvxDistributeVertical eVer = SvxDistributeVerticalNone);
    ~SvxDistributeDialog();

    SvxDistributeHorizontal GetDistributeHor() const { return mpPage->GetDistributeHor(); }
    SvxDistributeVertical GetDistributeVer() const { return mpPage->GetDistributeVer(); }
};


#endif // _SVX_DSTRIBUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
