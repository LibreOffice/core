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

#include "tp_SeriesToAxis.hxx"

#include "ResId.hxx"
#include "chartview/ChartSfxItemIds.hxx"

#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svl/ilstitem.hxx>
#include <svtools/controldims.hxx>

#include <com/sun/star/chart/MissingValueTreatment.hpp>

namespace chart
{

SchOptionTabPage::SchOptionTabPage(vcl::Window* pWindow,const SfxItemSet& rInAttrs)
    : SfxTabPage(pWindow,
               "TP_OPTIONS",
               "modules/schart/ui/tp_SeriesToAxis.ui",
               &rInAttrs)
    , m_nAllSeriesAxisIndex(0)
    , m_bProvidesSecondaryYAxis(true)
    , m_bProvidesOverlapAndGapWidth(false)
    , m_bProvidesBarConnectors(false)
{
    get(m_pGrpAxis,"frameGrpAxis");
    get(m_pRbtAxis1,"RBT_OPT_AXIS_1");
    get(m_pRbtAxis2,"RBT_OPT_AXIS_2");

    get(m_pGrpBar,"frameSettings");
    get(m_pMTGap,"MT_GAP");
    get(m_pMTOverlap,"MT_OVERLAP");
    get(m_pCBConnect,"CB_CONNECTOR");
    get(m_pCBAxisSideBySide,"CB_BARS_SIDE_BY_SIDE");

    get(m_pGrpPlotOptions,"frameFL_PLOT_OPTIONS" );
    get(m_pGridPlotOptions,"gridPLOT_OPTIONS");
    get(m_pRB_DontPaint,"RB_DONT_PAINT");
    get(m_pRB_AssumeZero,"RB_ASSUME_ZERO");
    get(m_pRB_ContinueLine,"RB_CONTINUE_LINE");
    get(m_pCBIncludeHiddenCells,"CB_INCLUDE_HIDDEN_CELLS");

    m_pRbtAxis1->SetClickHdl( LINK( this, SchOptionTabPage, EnableHdl ));
    m_pRbtAxis2->SetClickHdl( LINK( this, SchOptionTabPage, EnableHdl ));
}

SchOptionTabPage::~SchOptionTabPage()
{
    disposeOnce();
}

void SchOptionTabPage::dispose()
{
    m_pGrpAxis.clear();
    m_pRbtAxis1.clear();
    m_pRbtAxis2.clear();
    m_pGrpBar.clear();
    m_pMTGap.clear();
    m_pMTOverlap.clear();
    m_pCBConnect.clear();
    m_pCBAxisSideBySide.clear();
    m_pGrpPlotOptions.clear();
    m_pGridPlotOptions.clear();
    m_pRB_DontPaint.clear();
    m_pRB_AssumeZero.clear();
    m_pRB_ContinueLine.clear();
    m_pCBIncludeHiddenCells.clear();
    SfxTabPage::dispose();
}

IMPL_LINK_NOARG(SchOptionTabPage, EnableHdl, Button*, void)
{
    if( m_nAllSeriesAxisIndex == 0 )
        m_pCBAxisSideBySide->Enable( m_pRbtAxis2->IsChecked());
    else if( m_nAllSeriesAxisIndex == 1 )
        m_pCBAxisSideBySide->Enable( m_pRbtAxis1->IsChecked());
}

VclPtr<SfxTabPage> SchOptionTabPage::Create(vcl::Window* pWindow,
                                            const SfxItemSet* rOutAttrs)
{
    return VclPtr<SchOptionTabPage>::Create(pWindow, *rOutAttrs);
}

bool SchOptionTabPage::FillItemSet(SfxItemSet* rOutAttrs)
{
    if(m_pRbtAxis2->IsChecked())
        rOutAttrs->Put(SfxInt32Item(SCHATTR_AXIS,CHART_AXIS_SECONDARY_Y));
    else
        rOutAttrs->Put(SfxInt32Item(SCHATTR_AXIS,CHART_AXIS_PRIMARY_Y));

    if(m_pMTGap->IsVisible())
        rOutAttrs->Put(SfxInt32Item(SCHATTR_BAR_GAPWIDTH,static_cast< sal_Int32 >( m_pMTGap->GetValue())));

    if(m_pMTOverlap->IsVisible())
        rOutAttrs->Put(SfxInt32Item(SCHATTR_BAR_OVERLAP,static_cast< sal_Int32 >( m_pMTOverlap->GetValue())));

    if(m_pCBConnect->IsVisible())
        rOutAttrs->Put(SfxBoolItem(SCHATTR_BAR_CONNECT,m_pCBConnect->IsChecked()));

    // model property is "group bars per axis", UI feature is the other way
    // round: "show bars side by side"
    if(m_pCBAxisSideBySide->IsVisible())
        rOutAttrs->Put(SfxBoolItem(SCHATTR_GROUP_BARS_PER_AXIS, ! m_pCBAxisSideBySide->IsChecked()));

    if(m_pRB_DontPaint->IsChecked())
        rOutAttrs->Put(SfxInt32Item(SCHATTR_MISSING_VALUE_TREATMENT,css::chart::MissingValueTreatment::LEAVE_GAP));
    else if(m_pRB_AssumeZero->IsChecked())
        rOutAttrs->Put(SfxInt32Item(SCHATTR_MISSING_VALUE_TREATMENT,css::chart::MissingValueTreatment::USE_ZERO));
    else if(m_pRB_ContinueLine->IsChecked())
        rOutAttrs->Put(SfxInt32Item(SCHATTR_MISSING_VALUE_TREATMENT,css::chart::MissingValueTreatment::CONTINUE));

    if (m_pCBIncludeHiddenCells->IsVisible())
        rOutAttrs->Put(SfxBoolItem(SCHATTR_INCLUDE_HIDDEN_CELLS, m_pCBIncludeHiddenCells->IsChecked()));

    return true;
}

void SchOptionTabPage::Reset(const SfxItemSet* rInAttrs)
{
    const SfxPoolItem *pPoolItem = nullptr;

    m_pRbtAxis1->Check();
    m_pRbtAxis2->Check(false);
    if (rInAttrs->GetItemState(SCHATTR_AXIS,true, &pPoolItem) == SfxItemState::SET)
    {
        long nVal=static_cast<const SfxInt32Item*>(pPoolItem)->GetValue();
        if(nVal==CHART_AXIS_SECONDARY_Y)
        {
            m_pRbtAxis2->Check();
            m_pRbtAxis1->Check(false);
        }
    }

    long nTmp;
    if (rInAttrs->GetItemState(SCHATTR_BAR_GAPWIDTH, true, &pPoolItem) == SfxItemState::SET)
    {
        nTmp = (long)static_cast<const SfxInt32Item*>(pPoolItem)->GetValue();
        m_pMTGap->SetValue(nTmp);
    }

    if (rInAttrs->GetItemState(SCHATTR_BAR_OVERLAP, true, &pPoolItem) == SfxItemState::SET)
    {
        nTmp = (long)static_cast<const SfxInt32Item*>(pPoolItem)->GetValue();
        m_pMTOverlap->SetValue(nTmp);
    }

    if (rInAttrs->GetItemState(SCHATTR_BAR_CONNECT, true, &pPoolItem) == SfxItemState::SET)
    {
        bool bCheck = static_cast< const SfxBoolItem * >( pPoolItem )->GetValue();
        m_pCBConnect->Check(bCheck);
    }

    if (rInAttrs->GetItemState(SCHATTR_AXIS_FOR_ALL_SERIES, true, &pPoolItem) == SfxItemState::SET)
    {
        m_nAllSeriesAxisIndex = static_cast< const SfxInt32Item * >( pPoolItem )->GetValue();
        m_pCBAxisSideBySide->Disable();
    }
    if (rInAttrs->GetItemState(SCHATTR_GROUP_BARS_PER_AXIS, true, &pPoolItem) == SfxItemState::SET)
    {
        // model property is "group bars per axis", UI feature is the other way
        // round: "show bars side by side"
        bool bCheck = ! static_cast< const SfxBoolItem * >( pPoolItem )->GetValue();
        m_pCBAxisSideBySide->Check( bCheck );
    }
    else
    {
        m_pCBAxisSideBySide->Show(false);
    }

    //missing value treatment
    {
        std::vector< sal_Int32 > aMissingValueTreatments;
        if( rInAttrs->GetItemState(SCHATTR_AVAILABLE_MISSING_VALUE_TREATMENTS, true, &pPoolItem) == SfxItemState::SET )
            aMissingValueTreatments = static_cast<const SfxIntegerListItem*>(pPoolItem)->GetList();

        if ( aMissingValueTreatments.size()>1 && rInAttrs->GetItemState(SCHATTR_MISSING_VALUE_TREATMENT,true, &pPoolItem) == SfxItemState::SET)
        {
            m_pRB_DontPaint->Enable(false);
            m_pRB_AssumeZero->Enable(false);
            m_pRB_ContinueLine->Enable(false);

            for(int nVal : aMissingValueTreatments)
            {
                if(nVal==css::chart::MissingValueTreatment::LEAVE_GAP)
                    m_pRB_DontPaint->Enable();
                else if(nVal==css::chart::MissingValueTreatment::USE_ZERO)
                    m_pRB_AssumeZero->Enable();
                else if(nVal==css::chart::MissingValueTreatment::CONTINUE)
                    m_pRB_ContinueLine->Enable();
            }

            long nVal=static_cast<const SfxInt32Item*>(pPoolItem)->GetValue();
            if(nVal==css::chart::MissingValueTreatment::LEAVE_GAP)
                m_pRB_DontPaint->Check();
            else if(nVal==css::chart::MissingValueTreatment::USE_ZERO)
                m_pRB_AssumeZero->Check();
            else if(nVal==css::chart::MissingValueTreatment::CONTINUE)
                m_pRB_ContinueLine->Check();
        }
        else
        {
            m_pGridPlotOptions->Show(false);
        }
    }

    // Include hidden cells
    if (rInAttrs->GetItemState(SCHATTR_INCLUDE_HIDDEN_CELLS, true, &pPoolItem) == SfxItemState::SET)
    {
        bool bVal = static_cast<const SfxBoolItem*>(pPoolItem)->GetValue();
        m_pCBIncludeHiddenCells->Check(bVal);
    }
    else
    {
        m_pCBIncludeHiddenCells->Show(false);
        // check if the radiobutton guys above
        // are visible. If they aren't, we can
        // as well hide the whole frame
        if(!m_pGridPlotOptions->IsVisible())
            m_pGrpPlotOptions->Show(false);
    }

    AdaptControlPositionsAndVisibility();
}

void SchOptionTabPage::Init( bool bProvidesSecondaryYAxis, bool bProvidesOverlapAndGapWidth, bool bProvidesBarConnectors )
{
    m_bProvidesSecondaryYAxis = bProvidesSecondaryYAxis;
    m_bProvidesOverlapAndGapWidth = bProvidesOverlapAndGapWidth;
    m_bProvidesBarConnectors = bProvidesBarConnectors;

    AdaptControlPositionsAndVisibility();
}

void SchOptionTabPage::AdaptControlPositionsAndVisibility()
{
    m_pGrpAxis->Show(m_bProvidesSecondaryYAxis);

    m_pGrpBar->Show(m_bProvidesOverlapAndGapWidth);

    m_pCBConnect->Show(m_bProvidesBarConnectors);

     if( !m_pMTGap->IsVisible() && !m_pMTOverlap->IsVisible() )
     {
         m_pGrpBar->Show(false);
     }
}
} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
