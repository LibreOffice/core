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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_RES_TRENDLINE_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_RES_TRENDLINE_HXX

#include <vcl/window.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <svl/itemset.hxx>
#include <svx/chrtitem.hxx>
#include <vcl/field.hxx>
#include <svl/zformat.hxx>
#include <svtools/fmtfield.hxx>

namespace chart
{

class TrendlineResources
{
public:
    TrendlineResources( vcl::Window* pParent, const SfxItemSet& rInAttrs );
    virtual ~TrendlineResources();

    void Reset(const SfxItemSet& rInAttrs);
    bool FillItemSet(SfxItemSet* rOutAttrs) const;

    void FillValueSets();

    void SetNumFormatter( SvNumberFormatter* pFormatter );
    void SetNbPoints( sal_Int32 nNbPoints );

private:
    VclPtr<RadioButton> m_pRB_Linear;
    VclPtr<RadioButton> m_pRB_Logarithmic;
    VclPtr<RadioButton> m_pRB_Exponential;
    VclPtr<RadioButton> m_pRB_Power;
    VclPtr<RadioButton> m_pRB_Polynomial;
    VclPtr<RadioButton> m_pRB_MovingAverage;

    VclPtr<FixedImage>  m_pFI_Linear;
    VclPtr<FixedImage>  m_pFI_Logarithmic;
    VclPtr<FixedImage>  m_pFI_Exponential;
    VclPtr<FixedImage>  m_pFI_Power;
    VclPtr<FixedImage>  m_pFI_Polynomial;
    VclPtr<FixedImage>  m_pFI_MovingAverage;

    VclPtr<NumericField> m_pNF_Degree;
    VclPtr<NumericField> m_pNF_Period;
    VclPtr<Edit>         m_pEE_Name;
    VclPtr<FormattedField> m_pFmtFld_ExtrapolateForward;
    VclPtr<FormattedField> m_pFmtFld_ExtrapolateBackward;
    VclPtr<CheckBox>     m_pCB_SetIntercept;
    VclPtr<FormattedField> m_pFmtFld_InterceptValue;
    VclPtr<CheckBox>     m_pCB_ShowEquation;
    VclPtr<CheckBox>     m_pCB_ShowCorrelationCoeff;

    SvxChartRegress     m_eTrendLineType;

    bool                m_bTrendLineUnique;

    SvNumberFormatter*  m_pNumFormatter;
    sal_Int32           m_nNbPoints;

    void UpdateControlStates();
    DECL_LINK_TYPED( SelectTrendLine, Button *, void );
    DECL_LINK_TYPED( ChangeValue, Edit&, void);
};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_RES_TRENDLINE_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
