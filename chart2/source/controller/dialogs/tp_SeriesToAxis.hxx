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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_TP_SERIESTOAXIS_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_TP_SERIESTOAXIS_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/field.hxx>

namespace chart
{

class SchOptionTabPage : public SfxTabPage
{
public:
    SchOptionTabPage(vcl::Window* pParent, const SfxItemSet& rInAttrs);
    virtual ~SchOptionTabPage();
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create(vcl::Window* pParent, const SfxItemSet* rInAttrs);
    virtual bool FillItemSet(SfxItemSet* rOutAttrs) override;
    virtual void Reset(const SfxItemSet* rInAttrs) override;

    void Init( bool bProvidesSecondaryYAxis, bool bProvidesOverlapAndGapWidth, bool bProvidesBarConnectors );

private: //methods
    void AdaptControlPositionsAndVisibility();

private: //member
    VclPtr<VclFrame>    m_pGrpAxis;
    VclPtr<RadioButton> m_pRbtAxis1;
    VclPtr<RadioButton> m_pRbtAxis2;

    VclPtr<VclFrame>    m_pGrpBar;
    VclPtr<MetricField> m_pMTGap;
    VclPtr<MetricField> m_pMTOverlap;
    VclPtr<CheckBox>    m_pCBConnect;
    VclPtr<CheckBox>    m_pCBAxisSideBySide;

    VclPtr<VclFrame>    m_pGrpPlotOptions;
    VclPtr<VclGrid>     m_pGridPlotOptions;
    VclPtr<RadioButton> m_pRB_DontPaint;
    VclPtr<RadioButton> m_pRB_AssumeZero;
    VclPtr<RadioButton> m_pRB_ContinueLine;

    VclPtr<CheckBox>    m_pCBIncludeHiddenCells;

    DECL_LINK_TYPED(EnableHdl, Button*, void );

    sal_Int32   m_nAllSeriesAxisIndex;

    bool m_bProvidesSecondaryYAxis;
    bool m_bProvidesOverlapAndGapWidth;
    bool m_bProvidesBarConnectors;
};

} //namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
