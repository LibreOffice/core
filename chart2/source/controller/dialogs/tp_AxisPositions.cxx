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

#include "ResId.hxx"
#include "ResourceIds.hrc"
#include "Strings.hrc"
#include "chartview/ChartSfxItemIds.hxx"
#include "AxisHelper.hxx"

#include <svx/svxids.hrc>
#include <rtl/math.hxx>
#include <svx/chrtitem.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <vcl/msgbox.hxx>
#include <svl/zformat.hxx>
#include <svtools/controldims.hrc>

#include <com/sun/star/chart/ChartAxisPosition.hpp>
#include <com/sun/star/chart2/AxisType.hpp>

using namespace ::com::sun::star;

namespace chart
{

AxisPositionsTabPage::AxisPositionsTabPage(vcl::Window* pWindow,const SfxItemSet& rInAttrs)
    : SfxTabPage(pWindow
                ,"tp_AxisPositions"
                ,"modules/schart/ui/tp_AxisPositions.ui"
                , &rInAttrs)
    , m_pNumFormatter(NULL)
    , m_bCrossingAxisIsCategoryAxis(false)
    , m_aCategories()
    , m_bSupportAxisPositioning(false)
{
    get(m_pFL_AxisLine, "FL_AXIS_LINE");
    get(m_pLB_CrossesAt,"LB_CROSSES_OTHER_AXIS_AT");
    get(m_pED_CrossesAt,"EDT_CROSSES_OTHER_AXIS_AT");
    get(m_pED_CrossesAtCategory, "EDT_CROSSES_OTHER_AXIS_AT_CATEGORY");
    get(m_pCB_AxisBetweenCategories, "CB_AXIS_BETWEEN_CATEGORIES");

    get(m_pFL_Labels, "FL_LABELS");
    get(m_pLB_PlaceLabels,"LB_PLACE_LABELS");
    get(m_pED_LabelDistance,"EDT_AXIS_LABEL_DISTANCE");

    get(m_pCB_TicksInner, "CB_TICKS_INNER");
    get(m_pCB_TicksOuter, "CB_TICKS_OUTER");
    get(m_pCB_MinorInner, "CB_MINOR_INNER");
    get(m_pCB_MinorOuter, "CB_MINOR_OUTER");

    get(m_pBxPlaceTicks, "boxPLACE_TICKS");
    get(m_pLB_PlaceTicks, "LB_PLACE_TICKS");

    // these are not used/implemented
//     get(m_pCB_MajorGrid, "CB_MAJOR_GRID");
//     get(m_pPB_MajorGrid, "PB_MAJOR_GRID");
//     get(m_pCB_MinorGrid, "CB_MINOR_GRID");
//     get(m_pPB_MinorGrid, "PB_MINOR_GRID");

    m_pLB_CrossesAt->SetSelectHdl( LINK( this, AxisPositionsTabPage, CrossesAtSelectHdl ) );
    m_pLB_CrossesAt->SetDropDownLineCount( m_pLB_CrossesAt->GetEntryCount() );

    m_pLB_PlaceLabels->SetSelectHdl( LINK( this, AxisPositionsTabPage, PlaceLabelsSelectHdl ) );
    m_pLB_PlaceLabels->SetDropDownLineCount( m_pLB_PlaceLabels->GetEntryCount() );
    m_pLB_PlaceTicks->SetDropDownLineCount( m_pLB_PlaceTicks->GetEntryCount() );
}

AxisPositionsTabPage::~AxisPositionsTabPage()
{
    disposeOnce();
}

void AxisPositionsTabPage::dispose()
{
    m_pFL_AxisLine.clear();
    m_pLB_CrossesAt.clear();
    m_pED_CrossesAt.clear();
    m_pED_CrossesAtCategory.clear();
    m_pCB_AxisBetweenCategories.clear();
    m_pFL_Labels.clear();
    m_pLB_PlaceLabels.clear();
    m_pED_LabelDistance.clear();
    m_pCB_TicksInner.clear();
    m_pCB_TicksOuter.clear();
    m_pCB_MinorInner.clear();
    m_pCB_MinorOuter.clear();
    m_pBxPlaceTicks.clear();
    m_pLB_PlaceTicks.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> AxisPositionsTabPage::Create(vcl::Window* pWindow,const SfxItemSet* rOutAttrs)
{
    return VclPtr<AxisPositionsTabPage>::Create(pWindow, *rOutAttrs);
}

bool AxisPositionsTabPage::FillItemSet(SfxItemSet* rOutAttrs)
{
    // axis line
    sal_Int32 nPos = m_pLB_CrossesAt->GetSelectEntryPos();
    rOutAttrs->Put( SfxInt32Item( SCHATTR_AXIS_POSITION, nPos+1 ));
    if( 2==nPos )
    {
        double fCrossover = m_pED_CrossesAt->GetValue();
        if( m_bCrossingAxisIsCategoryAxis )
            fCrossover = m_pED_CrossesAtCategory->GetSelectEntryPos()+1;
        rOutAttrs->Put(SvxDoubleItem(fCrossover,SCHATTR_AXIS_POSITION_VALUE));
    }

    // labels
    sal_Int32 nLabelPos = m_pLB_PlaceLabels->GetSelectEntryPos();
    if( nLabelPos != LISTBOX_ENTRY_NOTFOUND )
        rOutAttrs->Put( SfxInt32Item( SCHATTR_AXIS_LABEL_POSITION, nLabelPos ));

    // tick marks
    long nTicks=0;
    long nMinorTicks=0;

    if(m_pCB_MinorInner->IsChecked())
        nMinorTicks|=CHAXIS_MARK_INNER;
    if(m_pCB_MinorOuter->IsChecked())
        nMinorTicks|=CHAXIS_MARK_OUTER;
    if(m_pCB_TicksInner->IsChecked())
        nTicks|=CHAXIS_MARK_INNER;
    if(m_pCB_TicksOuter->IsChecked())
        nTicks|=CHAXIS_MARK_OUTER;

    rOutAttrs->Put(SfxInt32Item(SCHATTR_AXIS_TICKS,nTicks));
    rOutAttrs->Put(SfxInt32Item(SCHATTR_AXIS_HELPTICKS,nMinorTicks));

    sal_Int32 nMarkPos = m_pLB_PlaceTicks->GetSelectEntryPos();
    if( nMarkPos != LISTBOX_ENTRY_NOTFOUND )
        rOutAttrs->Put( SfxInt32Item( SCHATTR_AXIS_MARK_POSITION, nMarkPos ));

    return true;
}

void AxisPositionsTabPage::Reset(const SfxItemSet* rInAttrs)
{
    //init and enable controls
    m_pED_CrossesAt->Show( !m_bCrossingAxisIsCategoryAxis );
    m_pED_CrossesAtCategory->Show( m_bCrossingAxisIsCategoryAxis );
    const sal_Int32 nMaxCount = LISTBOX_ENTRY_NOTFOUND;
    if( m_bCrossingAxisIsCategoryAxis )
    {
        for( sal_Int32 nN=0; nN<m_aCategories.getLength() && nN<nMaxCount; nN++ )
            m_pED_CrossesAtCategory->InsertEntry( m_aCategories[nN] );

        sal_Int32 nCount = m_pED_CrossesAtCategory->GetEntryCount();
        if( nCount>30 )
            nCount=30;
        m_pED_CrossesAtCategory->SetDropDownLineCount( nCount );
    }

    if( m_pLB_CrossesAt->GetEntryCount() > 3 )
    {
        if( m_bCrossingAxisIsCategoryAxis )
            m_pLB_CrossesAt->RemoveEntry(2);
        else
            m_pLB_CrossesAt->RemoveEntry(3);
    }

    //fill controls
    const SfxPoolItem *pPoolItem = NULL;

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

        if( nPos < m_pLB_CrossesAt->GetEntryCount() )
            m_pLB_CrossesAt->SelectEntryPos( nPos );
        CrossesAtSelectHdl( nullptr );

        if( rInAttrs->GetItemState(SCHATTR_AXIS_POSITION_VALUE,true, &pPoolItem)== SfxItemState::SET || bZero )
        {
            double fCrossover = 0.0;
            if( !bZero )
                fCrossover = static_cast<const SvxDoubleItem*>(pPoolItem)->GetValue();
            if( m_bCrossingAxisIsCategoryAxis )
                m_pED_CrossesAtCategory->SelectEntryPos( static_cast<sal_uInt16>(::rtl::math::round(fCrossover-1.0)) );
            else
                m_pED_CrossesAt->SetValue(fCrossover);
        }
        else
        {
            m_pED_CrossesAtCategory->SetNoSelection();
            m_pED_CrossesAt->SetTextValue("");
        }
    }
    else
    {
        m_pLB_CrossesAt->SetNoSelection();
        m_pED_CrossesAt->Enable( false );
    }

    // Labels
    if( rInAttrs->GetItemState( SCHATTR_AXIS_LABEL_POSITION, false, &pPoolItem ) == SfxItemState::SET )
    {
        sal_Int32 nPos = static_cast< const SfxInt32Item * >( pPoolItem )->GetValue();
        if( nPos < m_pLB_PlaceLabels->GetEntryCount() )
            m_pLB_PlaceLabels->SelectEntryPos( nPos );
    }
    else
        m_pLB_PlaceLabels->SetNoSelection();
    PlaceLabelsSelectHdl( nullptr );

    // Tick marks
    long nTicks = 0, nMinorTicks = 0;
    if(rInAttrs->GetItemState(SCHATTR_AXIS_TICKS,true, &pPoolItem)== SfxItemState::SET)
        nTicks = static_cast<const SfxInt32Item*>(pPoolItem)->GetValue();
    if(rInAttrs->GetItemState(SCHATTR_AXIS_HELPTICKS,true, &pPoolItem)== SfxItemState::SET)
        nMinorTicks = static_cast<const SfxInt32Item*>(pPoolItem)->GetValue();

    m_pCB_TicksInner->Check(bool(nTicks&CHAXIS_MARK_INNER));
    m_pCB_TicksOuter->Check(bool(nTicks&CHAXIS_MARK_OUTER));
    m_pCB_MinorInner->Check(bool(nMinorTicks&CHAXIS_MARK_INNER));
    m_pCB_MinorOuter->Check(bool(nMinorTicks&CHAXIS_MARK_OUTER));

    // Tick position
    if( rInAttrs->GetItemState( SCHATTR_AXIS_MARK_POSITION, false, &pPoolItem ) == SfxItemState::SET )
    {
        sal_Int32 nPos = static_cast< const SfxInt32Item * >( pPoolItem )->GetValue();
        if( nPos < m_pLB_PlaceTicks->GetEntryCount() )
            m_pLB_PlaceTicks->SelectEntryPos( nPos );
    }
    else
        m_pLB_PlaceTicks->SetNoSelection();

    if( !m_bSupportAxisPositioning )
    {
        m_pFL_AxisLine->Show(false);

        m_pFL_Labels->Show(false);

        m_pBxPlaceTicks->Show(false);
    }
    else if( !AxisHelper::isAxisPositioningEnabled() )
    {

        m_pFL_AxisLine->Enable(false);

        m_pFL_Labels->Enable(false);

        m_pBxPlaceTicks->Enable(false);

        //todo: maybe set a special help id to all those controls
    }
}

SfxTabPage::sfxpg AxisPositionsTabPage::DeactivatePage(SfxItemSet* pItemSet)
{
    if( pItemSet )
        FillItemSet( pItemSet );

    return LEAVE_PAGE;
}

void AxisPositionsTabPage::SetNumFormatter( SvNumberFormatter* pFormatter )
{
    m_pNumFormatter = pFormatter;
    m_pED_CrossesAt->SetFormatter( m_pNumFormatter );
    m_pED_CrossesAt->UseInputStringForFormatting();

    const SfxPoolItem *pPoolItem = NULL;
    if( GetItemSet().GetItemState( SCHATTR_AXIS_CROSSING_MAIN_AXIS_NUMBERFORMAT, true, &pPoolItem ) == SfxItemState::SET )
    {
        sal_uLong nFmt = (sal_uLong)static_cast<const SfxInt32Item*>(pPoolItem)->GetValue();
        m_pED_CrossesAt->SetFormatKey( nFmt );
    }
}

void AxisPositionsTabPage::SetCrossingAxisIsCategoryAxis( bool bCrossingAxisIsCategoryAxis )
{
    m_bCrossingAxisIsCategoryAxis = bCrossingAxisIsCategoryAxis;
}

void AxisPositionsTabPage::SetCategories( const ::com::sun::star::uno::Sequence< OUString >& rCategories )
{
    m_aCategories = rCategories;
}

void AxisPositionsTabPage::SupportAxisPositioning( bool bSupportAxisPositioning )
{
    m_bSupportAxisPositioning = bSupportAxisPositioning;
}

IMPL_LINK_NOARG(AxisPositionsTabPage, CrossesAtSelectHdl)
{
    sal_Int32 nPos = m_pLB_CrossesAt->GetSelectEntryPos();
    m_pED_CrossesAt->Show( (2==nPos) && !m_bCrossingAxisIsCategoryAxis );
    m_pED_CrossesAtCategory->Show( (2==nPos) && m_bCrossingAxisIsCategoryAxis );

    if( m_pED_CrossesAt->GetText().isEmpty() )
        m_pED_CrossesAt->SetValue(0.0);
    if( 0 == m_pED_CrossesAtCategory->GetSelectEntryCount() )
        m_pED_CrossesAtCategory->SelectEntryPos(0);

    PlaceLabelsSelectHdl( nullptr );
    return 0;
}

IMPL_LINK_NOARG(AxisPositionsTabPage, PlaceLabelsSelectHdl)
{
    sal_Int32 nLabelPos = m_pLB_PlaceLabels->GetSelectEntryPos();

    bool bEnableTickmarkPlacement = (nLabelPos>1);
    if( bEnableTickmarkPlacement )
    {
        sal_Int32 nAxisPos = m_pLB_CrossesAt->GetSelectEntryPos();
        if( nLabelPos-2 == nAxisPos )
            bEnableTickmarkPlacement=false;
    }
    m_pBxPlaceTicks->Enable(bEnableTickmarkPlacement);

    return 0;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
