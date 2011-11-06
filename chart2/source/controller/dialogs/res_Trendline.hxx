/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
