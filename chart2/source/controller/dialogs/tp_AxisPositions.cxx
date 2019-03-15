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

#include "tp_AxisPositions.hxx"

#include <chartview/ChartSfxItemIds.hxx>
#include <AxisHelper.hxx>

#include <rtl/math.hxx>
#include <svx/chrtitem.hxx>
#include <svl/intitem.hxx>

using namespace ::com::sun::star;

namespace chart
{

AxisPositionsTabPage::AxisPositionsTabPage(TabPageParent pWindow,const SfxItemSet& rInAttrs)
    : SfxTabPage(pWindow ,"modules/schart/ui/tp_AxisPositions.ui" ,"tp_AxisPositions" , &rInAttrs)
    , m_pNumFormatter(nullptr)
    , m_bCrossingAxisIsCategoryAxis(false)
    , m_aCategories()
    , m_bSupportAxisPositioning(false)
    , m_xFL_AxisLine(m_xBuilder->weld_frame("FL_AXIS_LINE"))
    , m_xLB_CrossesAt(m_xBuilder->weld_combo_box("LB_CROSSES_OTHER_AXIS_AT"))
    , m_xED_CrossesAt(m_xBuilder->weld_formatted_spin_button("EDT_CROSSES_OTHER_AXIS_AT"))
    , m_xED_CrossesAtCategory(m_xBuilder->weld_combo_box( "EDT_CROSSES_OTHER_AXIS_AT_CATEGORY"))
    , m_xCB_AxisBetweenCategories(m_xBuilder->weld_check_button("CB_AXIS_BETWEEN_CATEGORIES"))
    , m_xFL_Labels(m_xBuilder->weld_frame("FL_LABELS"))
    , m_xLB_PlaceLabels(m_xBuilder->weld_combo_box("LB_PLACE_LABELS"))
    , m_xED_LabelDistance(m_xBuilder->weld_formatted_spin_button("EDT_AXIS_LABEL_DISTANCE"))
    , m_xCB_TicksInner(m_xBuilder->weld_check_button("CB_TICKS_INNER"))
    , m_xCB_TicksOuter(m_xBuilder->weld_check_button("CB_TICKS_OUTER"))
    , m_xCB_MinorInner(m_xBuilder->weld_check_button("CB_MINOR_INNER"))
    , m_xCB_MinorOuter(m_xBuilder->weld_check_button("CB_MINOR_OUTER"))
    , m_xBxPlaceTicks(m_xBuilder->weld_widget("boxPLACE_TICKS"))
    , m_xLB_PlaceTicks(m_xBuilder->weld_combo_box("LB_PLACE_TICKS"))
{
    m_xLB_CrossesAt->connect_changed(LINK(this, AxisPositionsTabPage, CrossesAtSelectHdl));
    m_xLB_PlaceLabels->connect_changed(LINK(this, AxisPositionsTabPage, PlaceLabelsSelectHdl));
}

AxisPositionsTabPage::~AxisPositionsTabPage()
{
    disposeOnce();
}

VclPtr<SfxTabPage> AxisPositionsTabPage::Create(TabPageParent pParent, const SfxItemSet* rOutAttrs)
{
    return VclPtr<AxisPositionsTabPage>::Create(pParent, *rOutAttrs);
}

bool AxisPositionsTabPage::FillItemSet(SfxItemSet* rOutAttrs)
{
    // axis line
    sal_Int32 nPos = m_xLB_CrossesAt->get_active();
    rOutAttrs->Put( SfxInt32Item( SCHATTR_AXIS_POSITION, nPos+1 ));
    if( nPos==2 )
    {
        double fCrossover = m_xED_CrossesAt->get_value();
        if( m_bCrossingAxisIsCategoryAxis )
            fCrossover = m_xED_CrossesAtCategory->get_active()+1;
        rOutAttrs->Put(SvxDoubleItem(fCrossover,SCHATTR_AXIS_POSITION_VALUE));
    }

    // labels
    sal_Int32 nLabelPos = m_xLB_PlaceLabels->get_active();
    if (nLabelPos != -1)
        rOutAttrs->Put( SfxInt32Item( SCHATTR_AXIS_LABEL_POSITION, nLabelPos ));

    // tick marks
    long nTicks=0;
    long nMinorTicks=0;

    if(m_xCB_MinorInner->get_active())
        nMinorTicks|=CHAXIS_MARK_INNER;
    if(m_xCB_MinorOuter->get_active())
        nMinorTicks|=CHAXIS_MARK_OUTER;
    if(m_xCB_TicksInner->get_active())
        nTicks|=CHAXIS_MARK_INNER;
    if(m_xCB_TicksOuter->get_active())
        nTicks|=CHAXIS_MARK_OUTER;

    rOutAttrs->Put(SfxInt32Item(SCHATTR_AXIS_TICKS,nTicks));
    rOutAttrs->Put(SfxInt32Item(SCHATTR_AXIS_HELPTICKS,nMinorTicks));

    sal_Int32 nMarkPos = m_xLB_PlaceTicks->get_active();
    if (nMarkPos != -1)
        rOutAttrs->Put( SfxInt32Item( SCHATTR_AXIS_MARK_POSITION, nMarkPos ));

    return true;
}

void AxisPositionsTabPage::Reset(const SfxItemSet* rInAttrs)
{
    //init and enable controls
    m_xED_CrossesAt->set_visible( !m_bCrossingAxisIsCategoryAxis );
    m_xED_CrossesAtCategory->set_visible( m_bCrossingAxisIsCategoryAxis );
    if (m_bCrossingAxisIsCategoryAxis)
    {
        for( sal_Int32 nN=0; nN<m_aCategories.getLength(); nN++ )
            m_xED_CrossesAtCategory->append_text(m_aCategories[nN]);

        sal_Int32 nCount = m_xED_CrossesAtCategory->get_count();
        if( nCount>30 )
            nCount=30;
    }

    if( m_xLB_CrossesAt->get_count() > 3 )
    {
        if( m_bCrossingAxisIsCategoryAxis )
            m_xLB_CrossesAt->remove(2);
        else
            m_xLB_CrossesAt->remove(3);
    }

    //fill controls
    const SfxPoolItem *pPoolItem = nullptr;

    //axis line
    if(rInAttrs->GetItemState(SCHATTR_AXIS_POSITION,true, &pPoolItem)== SfxItemState::SET)
    {
        bool bZero = false;
        sal_Int32 nPos = static_cast< const SfxInt32Item * >( pPoolItem )->GetValue();
        if(nPos==0)
        {
            //switch to value
            bZero = true;
            nPos = 2;
        }
        else
            nPos--;

        if( nPos < m_xLB_CrossesAt->get_count() )
            m_xLB_CrossesAt->set_active( nPos );
        CrossesAtSelectHdl( *m_xLB_CrossesAt );

        if( rInAttrs->GetItemState(SCHATTR_AXIS_POSITION_VALUE,true, &pPoolItem)== SfxItemState::SET || bZero )
        {
            double fCrossover = 0.0;
            if( !bZero )
                fCrossover = static_cast<const SvxDoubleItem*>(pPoolItem)->GetValue();
            if( m_bCrossingAxisIsCategoryAxis )
                m_xED_CrossesAtCategory->set_active( static_cast<sal_uInt16>(::rtl::math::round(fCrossover-1.0)) );
            else
                m_xED_CrossesAt->set_value(fCrossover);
        }
        else
        {
            m_xED_CrossesAtCategory->set_active(-1);
            m_xED_CrossesAt->set_text("");
        }
    }
    else
    {
        m_xLB_CrossesAt->set_active(-1);
        m_xED_CrossesAt->set_sensitive( false );
    }

    // Labels
    if( rInAttrs->GetItemState( SCHATTR_AXIS_LABEL_POSITION, false, &pPoolItem ) == SfxItemState::SET )
    {
        sal_Int32 nPos = static_cast< const SfxInt32Item * >( pPoolItem )->GetValue();
        if( nPos < m_xLB_PlaceLabels->get_count() )
            m_xLB_PlaceLabels->set_active( nPos );
    }
    else
        m_xLB_PlaceLabels->set_active(-1);
    PlaceLabelsSelectHdl( *m_xLB_PlaceLabels );

    // Tick marks
    long nTicks = 0, nMinorTicks = 0;
    if (rInAttrs->GetItemState(SCHATTR_AXIS_TICKS,true, &pPoolItem)== SfxItemState::SET)
        nTicks = static_cast<const SfxInt32Item*>(pPoolItem)->GetValue();
    if (rInAttrs->GetItemState(SCHATTR_AXIS_HELPTICKS,true, &pPoolItem)== SfxItemState::SET)
        nMinorTicks = static_cast<const SfxInt32Item*>(pPoolItem)->GetValue();

    m_xCB_TicksInner->set_active(bool(nTicks&CHAXIS_MARK_INNER));
    m_xCB_TicksOuter->set_active(bool(nTicks&CHAXIS_MARK_OUTER));
    m_xCB_MinorInner->set_active(bool(nMinorTicks&CHAXIS_MARK_INNER));
    m_xCB_MinorOuter->set_active(bool(nMinorTicks&CHAXIS_MARK_OUTER));

    // Tick position
    if( rInAttrs->GetItemState( SCHATTR_AXIS_MARK_POSITION, false, &pPoolItem ) == SfxItemState::SET )
    {
        sal_Int32 nPos = static_cast< const SfxInt32Item * >( pPoolItem )->GetValue();
        if( nPos < m_xLB_PlaceTicks->get_count() )
            m_xLB_PlaceTicks->set_active( nPos );
    }
    else
        m_xLB_PlaceTicks->set_active(-1);

    if( !m_bSupportAxisPositioning )
    {
        m_xFL_AxisLine->hide();
        m_xFL_Labels->hide();
        m_xBxPlaceTicks->hide();
    }
    else if( !AxisHelper::isAxisPositioningEnabled() )
    {
        m_xFL_AxisLine->set_sensitive(false);
        m_xFL_Labels->set_sensitive(false);
        m_xBxPlaceTicks->set_sensitive(false);
        //todo: maybe set a special help id to all those controls
    }
}

DeactivateRC AxisPositionsTabPage::DeactivatePage(SfxItemSet* pItemSet)
{
    if( pItemSet )
        FillItemSet( pItemSet );

    return DeactivateRC::LeavePage;
}

void AxisPositionsTabPage::SetNumFormatter( SvNumberFormatter* pFormatter )
{
    m_pNumFormatter = pFormatter;
    m_xED_CrossesAt->set_formatter(m_pNumFormatter);

    const SfxPoolItem *pPoolItem = nullptr;
    if( GetItemSet().GetItemState( SCHATTR_AXIS_CROSSING_MAIN_AXIS_NUMBERFORMAT, true, &pPoolItem ) == SfxItemState::SET )
    {
        sal_uLong nFmt = static_cast<const SfxUInt32Item*>(pPoolItem)->GetValue();
        m_xED_CrossesAt->set_format_key( nFmt );
    }
}

void AxisPositionsTabPage::SetCrossingAxisIsCategoryAxis( bool bCrossingAxisIsCategoryAxis )
{
    m_bCrossingAxisIsCategoryAxis = bCrossingAxisIsCategoryAxis;
}

void AxisPositionsTabPage::SetCategories( const css::uno::Sequence< OUString >& rCategories )
{
    m_aCategories = rCategories;
}

void AxisPositionsTabPage::SupportAxisPositioning( bool bSupportAxisPositioning )
{
    m_bSupportAxisPositioning = bSupportAxisPositioning;
}

IMPL_LINK_NOARG(AxisPositionsTabPage, CrossesAtSelectHdl, weld::ComboBox&, void)
{
    sal_Int32 nPos = m_xLB_CrossesAt->get_active();
    m_xED_CrossesAt->set_visible( (nPos==2) && !m_bCrossingAxisIsCategoryAxis );
    m_xED_CrossesAtCategory->set_visible( (nPos==2) && m_bCrossingAxisIsCategoryAxis );

    if (m_xED_CrossesAt->get_text().isEmpty())
        m_xED_CrossesAt->set_value(0.0);
    if (m_xED_CrossesAtCategory->get_active() == -1)
        m_xED_CrossesAtCategory->set_active(0);

    PlaceLabelsSelectHdl(*m_xLB_PlaceLabels);
}

IMPL_LINK_NOARG(AxisPositionsTabPage, PlaceLabelsSelectHdl, weld::ComboBox&, void)
{
    sal_Int32 nLabelPos = m_xLB_PlaceLabels->get_active();

    bool bEnableTickmarkPlacement = (nLabelPos>1);
    if( bEnableTickmarkPlacement )
    {
        sal_Int32 nAxisPos = m_xLB_CrossesAt->get_active();
        if( nLabelPos-2 == nAxisPos )
            bEnableTickmarkPlacement=false;
    }
    m_xBxPlaceTicks->set_sensitive(bEnableTickmarkPlacement);
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
