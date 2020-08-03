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
#pragma once

#include <sfx2/tabdlg.hxx>
#include <svx/dstribut_enum.hxx>
#include <vcl/weld.hxx>

class SvxDistributePage : public SfxTabPage
{
    SvxDistributeHorizontal     m_eDistributeHor;
    SvxDistributeVertical       m_eDistributeVer;

    std::unique_ptr<weld::RadioButton> m_xBtnHorNone;
    std::unique_ptr<weld::RadioButton> m_xBtnHorLeft;
    std::unique_ptr<weld::RadioButton> m_xBtnHorCenter;
    std::unique_ptr<weld::RadioButton> m_xBtnHorDistance;
    std::unique_ptr<weld::RadioButton> m_xBtnHorRight;
    std::unique_ptr<weld::RadioButton> m_xBtnVerNone;
    std::unique_ptr<weld::RadioButton> m_xBtnVerTop;
    std::unique_ptr<weld::RadioButton> m_xBtnVerCenter;
    std::unique_ptr<weld::RadioButton> m_xBtnVerDistance;
    std::unique_ptr<weld::RadioButton> m_xBtnVerBottom;

public:
    SvxDistributePage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs,
        SvxDistributeHorizontal eHor,
        SvxDistributeVertical eVer);
    virtual ~SvxDistributePage() override;

    virtual bool FillItemSet(SfxItemSet*) override;
    virtual void Reset(const SfxItemSet*) override;

    SvxDistributeHorizontal GetDistributeHor() const { return m_eDistributeHor; }
    SvxDistributeVertical GetDistributeVer() const { return m_eDistributeVer; }
};

class SvxDistributeDialog : public SfxSingleTabDialogController
{
    SvxDistributePage* mpPage;

public:
    SvxDistributeDialog(weld::Window* pParent, const SfxItemSet& rAttr,
        SvxDistributeHorizontal eHor,
        SvxDistributeVertical eVer);
    virtual ~SvxDistributeDialog() override;

    SvxDistributeHorizontal GetDistributeHor() const { return mpPage->GetDistributeHor(); }
    SvxDistributeVertical GetDistributeVer() const { return mpPage->GetDistributeVer(); }
};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
