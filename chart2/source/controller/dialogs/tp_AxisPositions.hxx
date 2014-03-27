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

// header for SfxTabPage
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
    AxisPositionsTabPage( Window* pParent, const SfxItemSet& rInAttrs );

    static SfxTabPage* Create( Window* pParent, const SfxItemSet& rInAttrs );
    virtual bool FillItemSet( SfxItemSet& rOutAttrs ) SAL_OVERRIDE;
    virtual void Reset( const SfxItemSet& rInAttrs ) SAL_OVERRIDE;
    using TabPage::DeactivatePage;
    virtual int DeactivatePage( SfxItemSet* pItemSet = NULL ) SAL_OVERRIDE;

    void SetNumFormatter( SvNumberFormatter* pFormatter );

    void SetCrossingAxisIsCategoryAxis( bool bCrossingAxisIsCategoryAxis );
    void SetCategories( const ::com::sun::star::uno::Sequence< OUString >& rCategories );

    void SupportAxisPositioning( bool bSupportAxisPositioning );

private: //methods:
    DECL_LINK( CrossesAtSelectHdl, void* );
    DECL_LINK( PlaceLabelsSelectHdl, void* );

private: //member:
    VclFrame*       m_pFL_AxisLine;
    ListBox*        m_pLB_CrossesAt;
    FormattedField* m_pED_CrossesAt;
    ComboBox*       m_pED_CrossesAtCategory;
    CheckBox*       m_pCB_AxisBetweenCategories;

    VclFrame*       m_pFL_Labels;
    ListBox*        m_pLB_PlaceLabels;
    FormattedField* m_pED_LabelDistance;

    CheckBox*       m_pCB_TicksInner;
    CheckBox*       m_pCB_TicksOuter;

    CheckBox*       m_pCB_MinorInner;
    CheckBox*       m_pCB_MinorOuter;

    VclBox*         m_pBxPlaceTicks;
    ListBox*        m_pLB_PlaceTicks;

//     Not implemented
//     CheckBox*       m_pCB_MajorGrid;
//     PushButton*     m_pPB_MajorGrid;
//     CheckBox*       m_pCB_MinorGrid;
//     PushButton*     m_pPB_MinorGrid;

    SvNumberFormatter*  m_pNumFormatter;

    bool    m_bCrossingAxisIsCategoryAxis;
    ::com::sun::star::uno::Sequence< OUString > m_aCategories;

    bool    m_bSupportAxisPositioning;
};

} //namespace chart

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
