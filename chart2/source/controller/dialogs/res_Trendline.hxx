/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef CHART2_RES_TRENDLINE_HXX
#define CHART2_RES_TRENDLINE_HXX

#include <vcl/window.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <svtools/valueset.hxx>
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
