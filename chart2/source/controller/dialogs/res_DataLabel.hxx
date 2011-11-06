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


#ifndef CHART2_RES_DATALABEL_HXX
#define CHART2_RES_DATALABEL_HXX

#include "res_TextSeparator.hxx"
// header for class CheckBox
#include <vcl/button.hxx>
// header for class SfxItemSet
#include <svl/itemset.hxx>
#include "TextDirectionListBox.hxx"

#include <svx/dialcontrol.hxx>

class SvNumberFormatter;

//.............................................................................
namespace chart
{
//.............................................................................

class DataLabelResources
{
public:
    DataLabelResources( Window* pParent, const SfxItemSet& rInAttrs );
    virtual ~DataLabelResources();

    sal_Bool FillItemSet(SfxItemSet& rOutAttrs) const;
    void Reset(const SfxItemSet& rInAttrs);

    void SetNumberFormatter( SvNumberFormatter* pFormatter );

private:
    CheckBox            m_aCBNumber;
    PushButton          m_aPB_NumberFormatForValue;
    CheckBox            m_aCBPercent;
    PushButton          m_aPB_NumberFormatForPercent;
    CheckBox            m_aCBCategory;
    CheckBox            m_aCBSymbol;

    TextSeparatorResources  m_aSeparatorResources;

    FixedText           m_aFT_LabelPlacement;
    ListBox             m_aLB_LabelPlacement;

    FixedLine           m_aFL_Rotate;
    svx::DialControl    m_aDC_Dial;
    FixedText           m_aFT_Degrees;
    NumericField        m_aNF_Degrees;

    FixedText               m_aFT_TextDirection;
    TextDirectionListBox    m_aLB_TextDirection;

    ::std::map< sal_Int32, sal_uInt16 > m_aPlacementToListBoxMap;
    ::std::map< sal_uInt16, sal_Int32 > m_aListBoxToPlacementMap;

    SvNumberFormatter*  m_pNumberFormatter;
    bool                m_bNumberFormatMixedState;
    bool                m_bPercentFormatMixedState;
    sal_uLong               m_nNumberFormatForValue;
    sal_uLong               m_nNumberFormatForPercent;

    bool                m_bSourceFormatMixedState;
    bool                m_bPercentSourceMixedState;
    bool                m_bSourceFormatForValue;
    bool                m_bSourceFormatForPercent;

    Window*             m_pWindow;
    SfxItemPool*        m_pPool;

    DECL_LINK(NumberFormatDialogHdl, PushButton * );
    DECL_LINK(CheckHdl, CheckBox* );
    void EnableControls();
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif
