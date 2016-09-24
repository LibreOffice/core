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

#include "tp_PolarOptions.hxx"
#include "ResId.hxx"
#include "chartview/ChartSfxItemIds.hxx"

#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svtools/controldims.hrc>

namespace chart
{

PolarOptionsTabPage::PolarOptionsTabPage( vcl::Window* pWindow,const SfxItemSet& rInAttrs ) :
    SfxTabPage( pWindow
    ,"tp_PolarOptions"
    ,"modules/schart/ui/tp_PolarOptions.ui"
    ,&rInAttrs)
{
    get(m_pCB_Clockwise, "CB_CLOCKWISE");
    get(m_pFL_StartingAngle, "frameANGLE");
    get(m_pAngleDial, "CT_ANGLE_DIAL");
    get(m_pNF_StartingAngle, "NF_STARTING_ANGLE");
    get(m_pFL_PlotOptions, "framePLOT_OPTIONS");
    get(m_pCB_IncludeHiddenCells, "CB_INCLUDE_HIDDEN_CELLS_POLAR");

    m_pAngleDial->SetLinkedField( m_pNF_StartingAngle );
}

PolarOptionsTabPage::~PolarOptionsTabPage()
{
    disposeOnce();
}

void PolarOptionsTabPage::dispose()
{
    m_pCB_Clockwise.clear();
    m_pFL_StartingAngle.clear();
    m_pAngleDial.clear();
    m_pNF_StartingAngle.clear();
    m_pFL_PlotOptions.clear();
    m_pCB_IncludeHiddenCells.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> PolarOptionsTabPage::Create( vcl::Window* pWindow,const SfxItemSet* rOutAttrs )
{
    return VclPtr<PolarOptionsTabPage>::Create( pWindow, *rOutAttrs );
}

bool PolarOptionsTabPage::FillItemSet( SfxItemSet* rOutAttrs )
{
    if( m_pAngleDial->IsVisible() )
    {
        rOutAttrs->Put(SfxInt32Item(SCHATTR_STARTING_ANGLE,
            static_cast< sal_Int32 >(m_pAngleDial->GetRotation()/100)));
    }

    if( m_pCB_Clockwise->IsVisible() )
        rOutAttrs->Put(SfxBoolItem(SCHATTR_CLOCKWISE,m_pCB_Clockwise->IsChecked()));

    if (m_pCB_IncludeHiddenCells->IsVisible())
        rOutAttrs->Put(SfxBoolItem(SCHATTR_INCLUDE_HIDDEN_CELLS, m_pCB_IncludeHiddenCells->IsChecked()));

    return true;
}

void PolarOptionsTabPage::Reset(const SfxItemSet* rInAttrs)
{
    const SfxPoolItem *pPoolItem = nullptr;

    if (rInAttrs->GetItemState(SCHATTR_STARTING_ANGLE, true, &pPoolItem) == SfxItemState::SET)
    {
        long nTmp = (long)static_cast<const SfxInt32Item*>(pPoolItem)->GetValue();
        m_pAngleDial->SetRotation( nTmp*100 );
    }
    else
    {
        m_pFL_StartingAngle->Show(false);
    }
    if (rInAttrs->GetItemState(SCHATTR_CLOCKWISE, true, &pPoolItem) == SfxItemState::SET)
    {
        bool bCheck = static_cast< const SfxBoolItem * >( pPoolItem )->GetValue();
        m_pCB_Clockwise->Check(bCheck);
    }
    else
    {
        m_pCB_Clockwise->Show(false);
    }
    if (rInAttrs->GetItemState(SCHATTR_INCLUDE_HIDDEN_CELLS, true, &pPoolItem) == SfxItemState::SET)
    {
        bool bVal = static_cast<const SfxBoolItem*>(pPoolItem)->GetValue();
        m_pCB_IncludeHiddenCells->Check(bVal);
    }
    else
    {
        m_pFL_PlotOptions->Show(false);
    }
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
