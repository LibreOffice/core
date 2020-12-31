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

#include <svtools/ctrlbox.hxx>
#include <svx/colorbox.hxx>

// footnote settings TabPage
class SwFootNotePage: public SfxTabPage
{
    static const sal_uInt16 aPageRg[];
public:
    SwFootNotePage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet &rSet);
    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet *rSet);
    virtual ~SwFootNotePage() override;

    static const sal_uInt16* GetRanges() { return aPageRg; }

    virtual bool FillItemSet(SfxItemSet *rSet) override;
    virtual void Reset(const SfxItemSet *rSet) override;

private:

    tools::Long            lMaxHeight;

    std::unique_ptr<weld::RadioButton> m_xMaxHeightPageBtn;
    std::unique_ptr<weld::RadioButton> m_xMaxHeightBtn;
    std::unique_ptr<weld::MetricSpinButton> m_xMaxHeightEdit;
    std::unique_ptr<weld::MetricSpinButton> m_xDistEdit;
    std::unique_ptr<weld::ComboBox> m_xLinePosBox;
    std::unique_ptr<SvtLineListBox> m_xLineTypeBox;
    std::unique_ptr<weld::MetricSpinButton> m_xLineWidthEdit;
    std::unique_ptr<ColorListBox> m_xLineColorBox;
    std::unique_ptr<weld::MetricSpinButton> m_xLineLengthEdit;
    std::unique_ptr<weld::MetricSpinButton> m_xLineDistEdit;

    DECL_LINK(HeightPage, weld::ToggleButton&, void);
    DECL_LINK(HeightMetric, weld::ToggleButton&, void);
    DECL_LINK(HeightModify, weld::MetricSpinButton&, void);
    DECL_LINK(LineWidthChanged_Impl, weld::MetricSpinButton&, void);
    DECL_LINK(LineColorSelected_Impl, ColorListBox&, void);

    virtual void    ActivatePage( const SfxItemSet& rSet ) override;
    virtual DeactivateRC   DeactivatePage( SfxItemSet* pSet ) override;

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
