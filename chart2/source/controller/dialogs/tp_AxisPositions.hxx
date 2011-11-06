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


#ifndef _CHART2_TP_AXISPOSITIONS_HXX
#define _CHART2_TP_AXISPOSITIONS_HXX

// header for SfxTabPage
#include <sfx2/tabdlg.hxx>
#include <svtools/fmtfield.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>
//class SvNumberFormatter;

//.............................................................................
namespace chart
{
//.............................................................................

class AxisPositionsTabPage : public SfxTabPage
{
public:
    AxisPositionsTabPage( Window* pParent, const SfxItemSet& rInAttrs );

    static SfxTabPage* Create( Window* pParent, const SfxItemSet& rInAttrs );
    virtual sal_Bool FillItemSet( SfxItemSet& rOutAttrs );
    virtual void Reset( const SfxItemSet& rInAttrs );
    using TabPage::DeactivatePage;
    virtual int DeactivatePage( SfxItemSet* pItemSet = NULL );

    void SetNumFormatter( SvNumberFormatter* pFormatter );

    void SetCrossingAxisIsCategoryAxis( bool bCrossingAxisIsCategoryAxis );
    void SetCategories( const ::com::sun::star::uno::Sequence< rtl::OUString >& rCategories );

    void SupportAxisPositioning( bool bSupportAxisPositioning );

private: //methods:
    DECL_LINK( CrossesAtSelectHdl, void* );
    DECL_LINK( PlaceLabelsSelectHdl, void* );

private: //member:
    FixedLine           m_aFL_AxisLine;
    FixedText           m_aFT_CrossesAt;
    ListBox             m_aLB_CrossesAt;
    FormattedField      m_aED_CrossesAt;
    ComboBox            m_aED_CrossesAtCategory;
    CheckBox            m_aCB_AxisBetweenCategories;

    FixedLine       m_aFL_Labels;
    FixedText       m_aFT_PlaceLabels;
    ListBox         m_aLB_PlaceLabels;
    FixedText       m_aFT_LabelDistance;
    FormattedField  m_aED_LabelDistance;

    FixedLine   m_aFL_Ticks;

    FixedText   m_aFT_Major;
    CheckBox    m_aCB_TicksInner;
    CheckBox    m_aCB_TicksOuter;

    FixedText   m_aFT_Minor;
    CheckBox    m_aCB_MinorInner;
    CheckBox    m_aCB_MinorOuter;

    FixedLine   m_aFL_Vertical;
    FixedText   m_aFT_PlaceTicks;
    ListBox     m_aLB_PlaceTicks;

    FixedLine   m_aFL_Grids;
    CheckBox    m_aCB_MajorGrid;
    PushButton  m_aPB_MajorGrid;
    CheckBox    m_aCB_MinorGrid;
    PushButton  m_aPB_MinorGrid;

    SvNumberFormatter*  m_pNumFormatter;

    bool    m_bCrossingAxisIsCategoryAxis;
    ::com::sun::star::uno::Sequence< rtl::OUString > m_aCategories;

    bool    m_bSupportAxisPositioning;
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif
