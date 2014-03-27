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
#ifndef INCLUDED_CUI_SOURCE_INC_DSTRIBUT_HXX
#define INCLUDED_CUI_SOURCE_INC_DSTRIBUT_HXX

#include <svx/dlgctrl.hxx>
#include <vcl/group.hxx>
#include <vcl/fixed.hxx>
#include <svx/dstribut_enum.hxx>

class SvxDistributePage : public SvxTabPage
{
    SvxDistributeHorizontal     m_eDistributeHor;
    SvxDistributeVertical       m_eDistributeVer;

    RadioButton*                m_pBtnHorNone;
    RadioButton*                m_pBtnHorLeft;
    RadioButton*                m_pBtnHorCenter;
    RadioButton*                m_pBtnHorDistance;
    RadioButton*                m_pBtnHorRight;
    RadioButton*                m_pBtnVerNone;
    RadioButton*                m_pBtnVerTop;
    RadioButton*                m_pBtnVerCenter;
    RadioButton*                m_pBtnVerDistance;
    RadioButton*                m_pBtnVerBottom;

public:
    SvxDistributePage(Window* pWindow, const SfxItemSet& rInAttrs,
        SvxDistributeHorizontal eHor = SvxDistributeHorizontalNone,
        SvxDistributeVertical eVer = SvxDistributeVerticalNone);

    static SfxTabPage* Create(Window*, const SfxItemSet&,
        SvxDistributeHorizontal eHor, SvxDistributeVertical eVer);
    virtual bool FillItemSet(SfxItemSet&) SAL_OVERRIDE;
    virtual void Reset(const SfxItemSet&) SAL_OVERRIDE;
    virtual void PointChanged(Window* pWindow, RECT_POINT eRP) SAL_OVERRIDE;

    SvxDistributeHorizontal GetDistributeHor() const { return m_eDistributeHor; }
    SvxDistributeVertical GetDistributeVer() const { return m_eDistributeVer; }
};

class SvxDistributeDialog : public SfxSingleTabDialog
{
    SvxDistributePage* mpPage;

public:
    SvxDistributeDialog(Window* pParent, const SfxItemSet& rAttr,
        SvxDistributeHorizontal eHor = SvxDistributeHorizontalNone,
        SvxDistributeVertical eVer = SvxDistributeVerticalNone);

    SvxDistributeHorizontal GetDistributeHor() const { return mpPage->GetDistributeHor(); }
    SvxDistributeVertical GetDistributeVer() const { return mpPage->GetDistributeVer(); }
};


#endif // INCLUDED_CUI_SOURCE_INC_DSTRIBUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
