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
#include "colex.hxx"
#include <vcl/customweld.hxx>
#include <vcl/weld.hxx>
#include <svx/colorbox.hxx>

// TabPage Format/(Styles/)Page/Text grid
class SwTextGridPage final : public SfxTabPage
{
    sal_Int32       m_nRubyUserValue;
    bool        m_bRubyUserValue;
    Size            m_aPageSize;
    bool            m_bVertical;
    bool        m_bSquaredMode;
    bool        m_bHRulerChanged;
    bool        m_bVRulerChanged;
    bool        m_bModified = false;

    SwPageGridExample m_aExampleWN;
    std::unique_ptr<weld::RadioButton> m_xNoGridRB;
    std::unique_ptr<weld::RadioButton> m_xLinesGridRB;
    std::unique_ptr<weld::RadioButton> m_xCharsGridRB;
    std::unique_ptr<weld::CheckButton> m_xSnapToCharsCB;
    std::unique_ptr<weld::CustomWeld> m_xExampleWN;
    std::unique_ptr<weld::Widget> m_xLayoutFL;
    std::unique_ptr<weld::SpinButton> m_xLinesPerPageNF;
    std::unique_ptr<weld::Label> m_xLinesRangeFT;
    std::unique_ptr<weld::MetricSpinButton> m_xTextSizeMF;
    std::unique_ptr<weld::Label> m_xCharsPerLineFT;
    std::unique_ptr<weld::SpinButton> m_xCharsPerLineNF;
    std::unique_ptr<weld::Label> m_xCharsRangeFT;
    std::unique_ptr<weld::Label> m_xCharWidthFT;
    std::unique_ptr<weld::MetricSpinButton> m_xCharWidthMF;
    std::unique_ptr<weld::Label> m_xRubySizeFT;
    std::unique_ptr<weld::MetricSpinButton> m_xRubySizeMF;
    std::unique_ptr<weld::CheckButton> m_xRubyBelowCB;
    std::unique_ptr<weld::Widget> m_xDisplayFL;
    std::unique_ptr<weld::CheckButton> m_xDisplayCB;
    std::unique_ptr<weld::CheckButton> m_xPrintCB;
    std::unique_ptr<ColorListBox> m_xColorLB;

    void UpdatePageSize(const SfxItemSet& rSet);
    void PutGridItem(SfxItemSet& rSet);
    static void SetLinesOrCharsRanges(weld::Label& rField, const sal_Int32 nValue);

    void GridModifyHdl();

    DECL_LINK(GridTypeHdl, weld::Toggleable&, void);
    DECL_LINK(CharorLineChangedHdl, weld::SpinButton&, void);
    DECL_LINK(TextSizeChangedHdl, weld::MetricSpinButton&, void);
    DECL_LINK(ColorModifyHdl, ColorListBox&, void);
    DECL_LINK(GridModifyClickHdl, weld::Toggleable&, void);
    DECL_LINK(DisplayGridHdl, weld::Toggleable&, void);

public:
    SwTextGridPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet &rSet);
    virtual ~SwTextGridPage() override;

    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet *rSet);
    static const WhichRangesContainer & GetRanges();

    virtual bool    FillItemSet(SfxItemSet *rSet) override;
    virtual void    Reset(const SfxItemSet *rSet) override;

    virtual void    ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC   DeactivatePage( SfxItemSet* pSet ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
