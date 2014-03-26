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
// header for class SvNumberformat
#include <svl/zformat.hxx>
// header for class FormattedField
#include <svtools/fmtfield.hxx>
namespace chart
{

class TrendlineResources
{
public:
    TrendlineResources( Window* pParent, const SfxItemSet& rInAttrs );
    virtual ~TrendlineResources();

    void Reset(const SfxItemSet& rInAttrs);
    bool FillItemSet(SfxItemSet& rOutAttrs) const;

    void FillValueSets();

    void SetNumFormatter( SvNumberFormatter* pFormatter );

private:
    RadioButton* m_pRB_Linear;
    RadioButton* m_pRB_Logarithmic;
    RadioButton* m_pRB_Exponential;
    RadioButton* m_pRB_Power;
    RadioButton* m_pRB_Polynomial;
    RadioButton* m_pRB_MovingAverage;

    FixedImage*  m_pFI_Linear;
    FixedImage*  m_pFI_Logarithmic;
    FixedImage*  m_pFI_Exponential;
    FixedImage*  m_pFI_Power;
    FixedImage*  m_pFI_Polynomial;
    FixedImage*  m_pFI_MovingAverage;

    NumericField* m_pNF_Degree;
    NumericField* m_pNF_Period;
    Edit*         m_pEE_Name;
    FormattedField* m_pFmtFld_ExtrapolateForward;
    FormattedField* m_pFmtFld_ExtrapolateBackward;
    CheckBox*     m_pCB_SetIntercept;
    FormattedField* m_pFmtFld_InterceptValue;
    CheckBox*     m_pCB_ShowEquation;
    CheckBox*     m_pCB_ShowCorrelationCoeff;

    SvxChartRegress     m_eTrendLineType;

    bool                m_bTrendLineUnique;

    SvNumberFormatter*  m_pNumFormatter;

    void UpdateControlStates();
    DECL_LINK( SelectTrendLine, RadioButton * );
    DECL_LINK( ChangeValue, void *);
};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_RES_TRENDLINE_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
