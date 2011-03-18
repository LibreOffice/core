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
#ifndef _CHART2_TP_AXISPOSITIONS_HXX
#define _CHART2_TP_AXISPOSITIONS_HXX

// header for SfxTabPage
#include <sfx2/tabdlg.hxx>
#include <svtools/fmtfield.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
