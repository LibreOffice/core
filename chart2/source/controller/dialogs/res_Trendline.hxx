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
#if 1

#include <vcl/window.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <svl/itemset.hxx>
#include <svx/chrtitem.hxx>

namespace chart
{

class TrendlineResources
{
public:
    TrendlineResources( Window* pParent, const SfxItemSet& rInAttrs, bool bNoneAvailable );
    virtual ~TrendlineResources();

    void Reset(const SfxItemSet& rInAttrs);
    sal_Bool FillItemSet(SfxItemSet& rOutAttrs) const;

    void FillValueSets();

    long adjustControlSizes();

private:
    FixedLine m_aFLType;
    RadioButton m_aRBNone;
    RadioButton m_aRBLinear;
    RadioButton m_aRBLogarithmic;
    RadioButton m_aRBExponential;
    RadioButton m_aRBPower;

    FixedImage  m_aFINone;
    FixedImage  m_aFILinear;
    FixedImage  m_aFILogarithmic;
    FixedImage  m_aFIExponential;
    FixedImage  m_aFIPower;

    FixedLine m_aFLEquation;
    CheckBox m_aCBShowEquation;
    CheckBox m_aCBShowCorrelationCoeff;

    SvxChartRegress     m_eTrendLineType;

    bool                m_bNoneAvailable;
    bool                m_bTrendLineUnique;

    void UpdateControlStates();
    DECL_LINK( SelectTrendLine, RadioButton * );
};

} //  namespace chart

// CHART2_RES_TRENDLINE_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
