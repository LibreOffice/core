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

    VclPtr<RadioButton>                m_pBtnHorNone;
    VclPtr<RadioButton>                m_pBtnHorLeft;
    VclPtr<RadioButton>                m_pBtnHorCenter;
    VclPtr<RadioButton>                m_pBtnHorDistance;
    VclPtr<RadioButton>                m_pBtnHorRight;
    VclPtr<RadioButton>                m_pBtnVerNone;
    VclPtr<RadioButton>                m_pBtnVerTop;
    VclPtr<RadioButton>                m_pBtnVerCenter;
    VclPtr<RadioButton>                m_pBtnVerDistance;
    VclPtr<RadioButton>                m_pBtnVerBottom;

public:
    SvxDistributePage(vcl::Window* pWindow, const SfxItemSet& rInAttrs,
        SvxDistributeHorizontal eHor = SvxDistributeHorizontalNone,
        SvxDistributeVertical eVer = SvxDistributeVerticalNone);
    virtual ~SvxDistributePage();
    virtual void dispose() override;

    virtual bool FillItemSet(SfxItemSet*) override;
    virtual void Reset(const SfxItemSet*) override;
    virtual void PointChanged(vcl::Window* pWindow, RECT_POINT eRP) override;

    SvxDistributeHorizontal GetDistributeHor() const { return m_eDistributeHor; }
    SvxDistributeVertical GetDistributeVer() const { return m_eDistributeVer; }
};

class SvxDistributeDialog : public SfxSingleTabDialog
{
    VclPtr<SvxDistributePage> mpPage;

public:
    SvxDistributeDialog(vcl::Window* pParent, const SfxItemSet& rAttr,
        SvxDistributeHorizontal eHor = SvxDistributeHorizontalNone,
        SvxDistributeVertical eVer = SvxDistributeVerticalNone);
    virtual ~SvxDistributeDialog();
    virtual void dispose() override;

    SvxDistributeHorizontal GetDistributeHor() const { return mpPage->GetDistributeHor(); }
    SvxDistributeVertical GetDistributeVer() const { return mpPage->GetDistributeVer(); }
};


#endif // INCLUDED_CUI_SOURCE_INC_DSTRIBUT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
