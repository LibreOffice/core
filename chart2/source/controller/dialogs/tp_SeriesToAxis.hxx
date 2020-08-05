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

namespace weld {
    class CheckButton;
    class MetricSpinButton;
    class RadioButton;
    class ToggleButton;
    class Widget;
}

namespace chart
{

class SchOptionTabPage : public SfxTabPage
{
public:
    SchOptionTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rInAttrs);
    virtual ~SchOptionTabPage() override;

    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rInAttrs);
    virtual bool FillItemSet(SfxItemSet* rOutAttrs) override;
    virtual void Reset(const SfxItemSet* rInAttrs) override;

    void Init( bool bProvidesSecondaryYAxis, bool bProvidesOverlapAndGapWidth, bool bProvidesBarConnectors );

private: //methods
    void AdaptControlPositionsAndVisibility();

private: //member
    DECL_LINK(EnableHdl, weld::ToggleButton&, void );

    sal_Int32   m_nAllSeriesAxisIndex;

    bool m_bProvidesSecondaryYAxis;
    bool m_bProvidesOverlapAndGapWidth;
    bool m_bProvidesBarConnectors;

    std::unique_ptr<weld::Widget> m_xGrpAxis;
    std::unique_ptr<weld::RadioButton> m_xRbtAxis1;
    std::unique_ptr<weld::RadioButton> m_xRbtAxis2;
    std::unique_ptr<weld::Widget> m_xGrpBar;
    std::unique_ptr<weld::MetricSpinButton> m_xMTGap;
    std::unique_ptr<weld::MetricSpinButton> m_xMTOverlap;
    std::unique_ptr<weld::CheckButton> m_xCBConnect;
    std::unique_ptr<weld::CheckButton> m_xCBAxisSideBySide;
    std::unique_ptr<weld::Widget> m_xGrpPlotOptions;
    std::unique_ptr<weld::Widget> m_xGridPlotOptions;
    std::unique_ptr<weld::RadioButton> m_xRB_DontPaint;
    std::unique_ptr<weld::RadioButton> m_xRB_AssumeZero;
    std::unique_ptr<weld::RadioButton> m_xRB_ContinueLine;
    std::unique_ptr<weld::CheckButton> m_xCBIncludeHiddenCells;
    std::unique_ptr<weld::CheckButton> m_xCBHideLegendEntry;
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
