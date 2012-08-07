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
