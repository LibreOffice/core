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
#ifndef INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_TP_AXISPOSITIONS_HXX
#define INCLUDED_CHART2_SOURCE_CONTROLLER_DIALOGS_TP_AXISPOSITIONS_HXX

#include <sfx2/tabdlg.hxx>
#include <svtools/fmtfield.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>

namespace chart
{

class AxisPositionsTabPage : public SfxTabPage
{
public:
    AxisPositionsTabPage( vcl::Window* pParent, const SfxItemSet& rInAttrs );
    virtual ~AxisPositionsTabPage();
    virtual void dispose() override;

    static VclPtr<SfxTabPage> Create( vcl::Window* pParent, const SfxItemSet* rInAttrs );
    virtual bool FillItemSet( SfxItemSet* rOutAttrs ) override;
    virtual void Reset( const SfxItemSet* rInAttrs ) override;
    using TabPage::DeactivatePage;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pItemSet ) override;

    void SetNumFormatter( SvNumberFormatter* pFormatter );

    void SetCrossingAxisIsCategoryAxis( bool bCrossingAxisIsCategoryAxis );
    void SetCategories( const css::uno::Sequence< OUString >& rCategories );

    void SupportAxisPositioning( bool bSupportAxisPositioning );

private: //methods:
    DECL_LINK_TYPED( CrossesAtSelectHdl, ListBox&, void );
    DECL_LINK_TYPED( PlaceLabelsSelectHdl, ListBox&, void );

private: //member:
    VclPtr<VclFrame>       m_pFL_AxisLine;
    VclPtr<ListBox>        m_pLB_CrossesAt;
    VclPtr<FormattedField> m_pED_CrossesAt;
    VclPtr<ComboBox>       m_pED_CrossesAtCategory;
    VclPtr<CheckBox>       m_pCB_AxisBetweenCategories;

    VclPtr<VclFrame>       m_pFL_Labels;
    VclPtr<ListBox>        m_pLB_PlaceLabels;
    VclPtr<FormattedField> m_pED_LabelDistance;

    VclPtr<CheckBox>       m_pCB_TicksInner;
    VclPtr<CheckBox>       m_pCB_TicksOuter;

    VclPtr<CheckBox>       m_pCB_MinorInner;
    VclPtr<CheckBox>       m_pCB_MinorOuter;

    VclPtr<VclBox>         m_pBxPlaceTicks;
    VclPtr<ListBox>        m_pLB_PlaceTicks;

//     Not implemented
//     VclPtr<CheckBox>       m_pCB_MajorGrid;
//     VclPtr<PushButton>     m_pPB_MajorGrid;
//     VclPtr<CheckBox>       m_pCB_MinorGrid;
//     VclPtr<PushButton>     m_pPB_MinorGrid;

    SvNumberFormatter*  m_pNumFormatter;

    bool    m_bCrossingAxisIsCategoryAxis;
    css::uno::Sequence< OUString > m_aCategories;

    bool    m_bSupportAxisPositioning;
};

} //namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
