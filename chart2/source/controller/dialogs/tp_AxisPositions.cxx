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
#include "TabPages.hrc"
#include "Strings.hrc"
#include "chartview/ChartSfxItemIds.hxx"
#include "NoWarningThisInCTOR.hxx"
#include "AxisHelper.hxx"

#include <svx/svxids.hrc>
#include <rtl/math.hxx>
// header for class SvxDoubleItem
#include <svx/chrtitem.hxx>
// header for class SfxBoolItem
#include <svl/eitem.hxx>
// header for SfxInt32Item
#include <svl/intitem.hxx>

// header for class WarningBox
#include <vcl/msgbox.hxx>

// header for class SvNumberformat
#include <svl/zformat.hxx>

#include <svtools/controldims.hrc>

#include <com/sun/star/chart/ChartAxisPosition.hpp>
#include <com/sun/star/chart2/AxisType.hpp>

using namespace ::com::sun::star;

namespace chart
{

namespace
{
void lcl_newXPos( Control& rControl, long nXPos )
{
    Point aPos( rControl.GetPosPixel() );
    aPos.X() = nXPos;
    rControl.SetPosPixel(aPos);
}

void lcl_shiftY( Control& rControl, long nYDiff )
{
    Point aPos( rControl.GetPosPixel() );
    aPos.Y() += nYDiff;
    rControl.SetPosPixel(aPos);
}

}

AxisPositionsTabPage::AxisPositionsTabPage(Window* pWindow,const SfxItemSet& rInAttrs)
    : SfxTabPage(pWindow, SchResId(TP_AXIS_POSITIONS), rInAttrs)

    , m_aFL_AxisLine(this,SchResId(FL_AXIS_LINE))
    , m_aFT_CrossesAt(this,SchResId(FT_CROSSES_OTHER_AXIS_AT))
    , m_aLB_CrossesAt(this,SchResId(LB_CROSSES_OTHER_AXIS_AT))
    , m_aED_CrossesAt(this,SchResId(EDT_CROSSES_OTHER_AXIS_AT))
    , m_aED_CrossesAtCategory(this,SchResId(EDT_CROSSES_OTHER_AXIS_AT_CATEGORY))
    , m_aCB_AxisBetweenCategories(this, SchResId(CB_AXIS_BETWEEN_CATEGORIES))

    , m_aFL_Labels(this,SchResId(FL_LABELS))
    , m_aFT_PlaceLabels( this, SchResId( FT_PLACE_LABELS ) )
    , m_aLB_PlaceLabels( this, SchResId( LB_PLACE_LABELS ) )
    , m_aFT_LabelDistance( this, SchResId( FT_AXIS_LABEL_DISTANCE ) )
    , m_aED_LabelDistance( this, SchResId( EDT_AXIS_LABEL_DISTANCE ) )

    , m_aFL_Ticks(this,SchResId(FL_TICKS))
    , m_aFT_Major(this,SchResId(FT_MAJOR))
    , m_aCB_TicksInner(this,SchResId(CB_TICKS_INNER))
    , m_aCB_TicksOuter(this,SchResId(CB_TICKS_OUTER))
    , m_aFT_Minor(this,SchResId(FT_MINOR))
    , m_aCB_MinorInner(this,SchResId(CB_MINOR_INNER))
    , m_aCB_MinorOuter(this,SchResId(CB_MINOR_OUTER))

    , m_aFL_Vertical(this,SchResId(FL_VERTICAL))
    , m_aFT_PlaceTicks(this,SchResId(FT_PLACE_TICKS))
    , m_aLB_PlaceTicks(this,SchResId(LB_PLACE_TICKS))

    , m_aFL_Grids(this,SchResId(FL_GRIDS))
    , m_aCB_MajorGrid(this,SchResId(CB_MAJOR_GRID))
    , m_aPB_MajorGrid(this,SchResId(PB_MAJOR_GRID))
    , m_aCB_MinorGrid(this,SchResId(CB_MINOR_GRID))
    , m_aPB_MinorGrid(this,SchResId(PB_MINOR_GRID))

    , m_pNumFormatter(NULL)
    , m_bCrossingAxisIsCategoryAxis(false)
    , m_aCategories()
{
    FreeResource();
    SetExchangeSupport();

    const long nDialogWidth = GetSizePixel().Width();
    const long nDistance = LogicToPixel( Size(RSC_SP_CTRL_X, 0), MapMode(MAP_APPFONT) ).Width();

    //axis line controls
    {
        if( m_aFT_CrossesAt.CalcMinimumSize().Width() < m_aFT_CrossesAt.GetSizePixel().Width() )
        {
            m_aFT_CrossesAt.SetSizePixel(m_aFT_CrossesAt.CalcMinimumSize());
            long nXPos = m_aFT_CrossesAt.GetPosPixel().X() + m_aFT_CrossesAt.GetSizePixel().Width() + nDistance;
            lcl_newXPos( m_aLB_CrossesAt, nXPos );

            nXPos += m_aLB_CrossesAt.GetSizePixel().Width() + nDistance;
            lcl_newXPos( m_aED_CrossesAt, nXPos );
            lcl_newXPos( m_aED_CrossesAtCategory, nXPos );
        }
    }

    long nFixTextHeight = m_aFT_PlaceLabels.GetSizePixel().Height();
    long nLabelPlacementWidth = m_aFT_PlaceLabels.CalcMinimumSize().Width();
    long nMarkPlacementWidth = m_aFT_PlaceTicks.CalcMinimumSize().Width();

    long nWidth_0 = ::std::max( m_aFT_Major.CalcMinimumSize().Width(), m_aFT_Minor.CalcMinimumSize().Width() );
    nWidth_0 = ::std::max( nWidth_0, nLabelPlacementWidth );
    nWidth_0 = ::std::max( nWidth_0, nMarkPlacementWidth );
    nLabelPlacementWidth = nMarkPlacementWidth = nWidth_0;

    //label placement
    {
        m_aFT_PlaceLabels.SetSizePixel( Size( nLabelPlacementWidth, nFixTextHeight ) );
        long nXPos = m_aFT_PlaceLabels.GetPosPixel().X() + nLabelPlacementWidth + nDistance;
        lcl_newXPos( m_aLB_PlaceLabels, nXPos );
        m_aLB_PlaceLabels.SetSizePixel(m_aLB_PlaceLabels.CalcMinimumSize());
    }

    //tickmark placement
    {
        m_aFT_PlaceTicks.SetSizePixel( Size( nMarkPlacementWidth, nFixTextHeight ) );
        long nXPos = m_aFT_PlaceTicks.GetPosPixel().X() + nMarkPlacementWidth + nDistance;
        lcl_newXPos( m_aLB_PlaceTicks, nXPos );
        m_aLB_PlaceTicks.SetSizePixel( m_aLB_PlaceTicks.CalcMinimumSize() );
    }

    //tickmark controls
    {
        long nWidth_1 = ::std::max( m_aCB_TicksInner.CalcMinimumSize().Width(), m_aCB_MinorInner.CalcMinimumSize().Width() );
        long nWidth_2 = ::std::max( m_aCB_TicksOuter.CalcMinimumSize().Width(), m_aCB_MinorOuter.CalcMinimumSize().Width() );

        long nLeftSpace = nDialogWidth - nWidth_0 - nWidth_1 - nWidth_2 - 3*nDistance;

        if(nLeftSpace>=0)
        {
            m_aFT_Major.SetSizePixel(m_aFT_Major.CalcMinimumSize());
            m_aFT_Minor.SetSizePixel(m_aFT_Minor.CalcMinimumSize());

            m_aCB_TicksInner.SetSizePixel( m_aCB_TicksInner.CalcMinimumSize()  );
            m_aCB_MinorInner.SetSizePixel( m_aCB_MinorInner.CalcMinimumSize()  );

            m_aCB_TicksOuter.SetSizePixel( m_aCB_TicksOuter.CalcMinimumSize() );
            m_aCB_MinorOuter.SetSizePixel( m_aCB_MinorOuter.CalcMinimumSize() );

            long nXPos = m_aFT_Major.GetPosPixel().X() + nWidth_0 + nDistance;
            lcl_newXPos( m_aCB_TicksInner, nXPos );
            lcl_newXPos( m_aCB_MinorInner, nXPos );

            nXPos += nWidth_1 + nDistance;
            lcl_newXPos( m_aCB_TicksOuter, nXPos );
            lcl_newXPos( m_aCB_MinorOuter, nXPos );

            nXPos += nWidth_2 + nDistance;
            lcl_newXPos( m_aFL_Vertical, nXPos );
        }
    }

    //right alignement of listboxes:
    {
        long nLabelRightBorder = m_aLB_PlaceLabels.GetPosPixel().X() + m_aLB_PlaceLabels.GetSizePixel().Width();
        long nTickmarksRightBorder = m_aLB_PlaceTicks.GetPosPixel().X() + m_aLB_PlaceTicks.GetSizePixel().Width();

        long nNewRightBorder = ::std::max( m_aLB_CrossesAt.GetPosPixel().X() + m_aLB_CrossesAt.GetSizePixel().Width(),  nLabelRightBorder);
        nNewRightBorder = ::std::max( nTickmarksRightBorder, nNewRightBorder );

        long nListBoxHeight = m_aLB_PlaceLabels.GetSizePixel().Height();
        m_aLB_PlaceLabels.SetSizePixel( Size( m_aLB_PlaceLabels.GetSizePixel().Width()+nNewRightBorder-nLabelRightBorder, nListBoxHeight ) );
        m_aLB_PlaceTicks.SetSizePixel( Size( m_aLB_PlaceTicks.GetSizePixel().Width()+nNewRightBorder-nTickmarksRightBorder, nListBoxHeight ) );
    }

    m_aLB_CrossesAt.SetSelectHdl( LINK( this, AxisPositionsTabPage, CrossesAtSelectHdl ) );
    m_aLB_CrossesAt.SetDropDownLineCount( m_aLB_CrossesAt.GetEntryCount() );

    m_aLB_PlaceLabels.SetSelectHdl( LINK( this, AxisPositionsTabPage, PlaceLabelsSelectHdl ) );
    m_aLB_PlaceLabels.SetDropDownLineCount( m_aLB_PlaceLabels.GetEntryCount() );
    m_aLB_PlaceTicks.SetDropDownLineCount( m_aLB_PlaceTicks.GetEntryCount() );
    m_aCB_TicksInner.SetAccessibleRelationLabeledBy(&m_aFT_Major);
    m_aCB_TicksOuter.SetAccessibleRelationLabeledBy(&m_aFT_Major);
    m_aCB_MinorInner.SetAccessibleRelationLabeledBy(&m_aFT_Minor);
    m_aCB_MinorOuter.SetAccessibleRelationLabeledBy(&m_aFT_Minor);
}

SfxTabPage* AxisPositionsTabPage::Create(Window* pWindow,const SfxItemSet& rOutAttrs)
{
    return new AxisPositionsTabPage(pWindow, rOutAttrs);
}

sal_Bool AxisPositionsTabPage::FillItemSet(SfxItemSet& rOutAttrs)
{
    // axis line
    sal_uInt16 nPos = m_aLB_CrossesAt.GetSelectEntryPos();
    rOutAttrs.Put( SfxInt32Item( SCHATTR_AXIS_POSITION, nPos+1 ));
    if( 2==nPos )
    {
        double fCrossover = m_aED_CrossesAt.GetValue();
        if( m_bCrossingAxisIsCategoryAxis )
            fCrossover = m_aED_CrossesAtCategory.GetSelectEntryPos()+1;
        rOutAttrs.Put(SvxDoubleItem(fCrossover,SCHATTR_AXIS_POSITION_VALUE));
    }

    // labels
    sal_uInt16 nLabelPos = m_aLB_PlaceLabels.GetSelectEntryPos();
    if( nLabelPos != LISTBOX_ENTRY_NOTFOUND )
        rOutAttrs.Put( SfxInt32Item( SCHATTR_AXIS_LABEL_POSITION, nLabelPos ));

    // tick marks
    long nTicks=0;
    long nMinorTicks=0;

    if(m_aCB_MinorInner.IsChecked())
        nMinorTicks|=CHAXIS_MARK_INNER;
    if(m_aCB_MinorOuter.IsChecked())
        nMinorTicks|=CHAXIS_MARK_OUTER;
    if(m_aCB_TicksInner.IsChecked())
        nTicks|=CHAXIS_MARK_INNER;
    if(m_aCB_TicksOuter.IsChecked())
        nTicks|=CHAXIS_MARK_OUTER;

    rOutAttrs.Put(SfxInt32Item(SCHATTR_AXIS_TICKS,nTicks));
    rOutAttrs.Put(SfxInt32Item(SCHATTR_AXIS_HELPTICKS,nMinorTicks));

    sal_uInt16 nMarkPos = m_aLB_PlaceTicks.GetSelectEntryPos();
    if( nMarkPos != LISTBOX_ENTRY_NOTFOUND )
        rOutAttrs.Put( SfxInt32Item( SCHATTR_AXIS_MARK_POSITION, nMarkPos ));

    return sal_True;
}

void AxisPositionsTabPage::Reset(const SfxItemSet& rInAttrs)
{
    //init and enable controls
    m_aED_CrossesAt.Show( !m_bCrossingAxisIsCategoryAxis );
    m_aED_CrossesAtCategory.Show( m_bCrossingAxisIsCategoryAxis );
    const sal_Int32 nMaxCount = LISTBOX_ENTRY_NOTFOUND;
    if( m_bCrossingAxisIsCategoryAxis )
    {
        for( sal_Int32 nN=0; nN<m_aCategories.getLength() && nN<nMaxCount; nN++ )
            m_aED_CrossesAtCategory.InsertEntry( m_aCategories[nN] );

        sal_uInt16 nCount = m_aED_CrossesAtCategory.GetEntryCount();
        if( nCount>30 )
            nCount=30;
        m_aED_CrossesAtCategory.SetDropDownLineCount( nCount );
    }

    if( m_aLB_CrossesAt.GetEntryCount() > 3 )
    {
        if( m_bCrossingAxisIsCategoryAxis )
            m_aLB_CrossesAt.RemoveEntry(2);
        else
            m_aLB_CrossesAt.RemoveEntry(3);
    }

    //fill controls
    const SfxPoolItem *pPoolItem = NULL;

    //axis line
    if(rInAttrs.GetItemState(SCHATTR_AXIS_POSITION,sal_True, &pPoolItem)== SFX_ITEM_SET)
    {
        bool bZero = false;
        sal_uInt16 nPos = (sal_uInt16)static_cast< const SfxInt32Item * >( pPoolItem )->GetValue();
        if(nPos==0)
        {
            //switch to value
            bZero = true;
            nPos = 2;
        }
        else
            nPos--;

        if( nPos < m_aLB_CrossesAt.GetEntryCount() )
            m_aLB_CrossesAt.SelectEntryPos( nPos );
        CrossesAtSelectHdl( (void*)0 );

        if( rInAttrs.GetItemState(SCHATTR_AXIS_POSITION_VALUE,sal_True, &pPoolItem)== SFX_ITEM_SET || bZero )
        {
            double fCrossover = 0.0;
            if( !bZero )
                fCrossover = (((const SvxDoubleItem*)pPoolItem)->GetValue());
            if( m_bCrossingAxisIsCategoryAxis )
                m_aED_CrossesAtCategory.SelectEntryPos( static_cast<sal_uInt16>(::rtl::math::round(fCrossover-1.0)) );
            else
                m_aED_CrossesAt.SetValue(fCrossover);
        }
        else
        {
            m_aED_CrossesAtCategory.SetNoSelection();
            m_aED_CrossesAt.SetTextValue(OUString());
        }
    }
    else
    {
        m_aLB_CrossesAt.SetNoSelection();
        m_aED_CrossesAt.Enable( sal_False );
    }

    // Labels
    if( rInAttrs.GetItemState( SCHATTR_AXIS_LABEL_POSITION, sal_False, &pPoolItem ) == SFX_ITEM_SET )
    {
        sal_uInt16 nPos = (sal_uInt16)static_cast< const SfxInt32Item * >( pPoolItem )->GetValue();
        if( nPos < m_aLB_PlaceLabels.GetEntryCount() )
            m_aLB_PlaceLabels.SelectEntryPos( nPos );
    }
    else
        m_aLB_PlaceLabels.SetNoSelection();
    PlaceLabelsSelectHdl( (void*)0 );

    // Tick marks
    long nTicks=0,nMinorTicks=0;
    if(rInAttrs.GetItemState(SCHATTR_AXIS_TICKS,sal_True, &pPoolItem)== SFX_ITEM_SET)
        nTicks=((const SfxInt32Item*)pPoolItem)->GetValue();
    if(rInAttrs.GetItemState(SCHATTR_AXIS_HELPTICKS,sal_True, &pPoolItem)== SFX_ITEM_SET)
        nMinorTicks=((const SfxInt32Item*)pPoolItem)->GetValue();

    m_aCB_TicksInner.Check(sal_Bool(nTicks&CHAXIS_MARK_INNER));
    m_aCB_TicksOuter.Check(sal_Bool(nTicks&CHAXIS_MARK_OUTER));
    m_aCB_MinorInner.Check(sal_Bool(nMinorTicks&CHAXIS_MARK_INNER));
    m_aCB_MinorOuter.Check(sal_Bool(nMinorTicks&CHAXIS_MARK_OUTER));

    // Tick position
    if( rInAttrs.GetItemState( SCHATTR_AXIS_MARK_POSITION, sal_False, &pPoolItem ) == SFX_ITEM_SET )
    {
        sal_uInt16 nPos = (sal_uInt16)static_cast< const SfxInt32Item * >( pPoolItem )->GetValue();
        if( nPos < m_aLB_PlaceTicks.GetEntryCount() )
            m_aLB_PlaceTicks.SelectEntryPos( nPos );
    }
    else
        m_aLB_PlaceTicks.SetNoSelection();

    if( !m_bSupportAxisPositioning )
    {
        m_aFL_AxisLine.Show(false);
        m_aFT_CrossesAt.Show(false);
        m_aLB_CrossesAt.Show(false);
        m_aED_CrossesAt.Show(false);
        m_aED_CrossesAtCategory.Show(false);
        m_aCB_AxisBetweenCategories.Show(false);

        m_aFL_Labels.Show(false);
        m_aFT_PlaceLabels.Show(false);
        m_aLB_PlaceLabels.Show(false);
        m_aFT_LabelDistance.Show(false);
        m_aED_LabelDistance.Show(false);

        m_aFL_Vertical.Show(false);
        m_aFT_PlaceTicks.Show(false);
        m_aLB_PlaceTicks.Show(false);

        long nYDiff = m_aFL_AxisLine.GetPosPixel().Y() - m_aFL_Ticks.GetPosPixel().Y();
        lcl_shiftY( m_aFL_Ticks, nYDiff );
        lcl_shiftY( m_aFT_Major, nYDiff );
        lcl_shiftY( m_aCB_TicksInner, nYDiff );
        lcl_shiftY( m_aCB_TicksOuter, nYDiff );

        lcl_shiftY( m_aFT_Minor, nYDiff );
        lcl_shiftY( m_aCB_MinorInner, nYDiff );
        lcl_shiftY( m_aCB_MinorOuter, nYDiff );
    }
    else if( !AxisHelper::isAxisPositioningEnabled() )
    {
        m_aFL_AxisLine.Enable(false);
        m_aFT_CrossesAt.Enable(false);
        m_aLB_CrossesAt.Enable(false);
        m_aED_CrossesAt.Enable(false);
        m_aED_CrossesAtCategory.Enable(false);
        m_aCB_AxisBetweenCategories.Enable(false);

        m_aFL_Labels.Enable(false);
        m_aFT_PlaceLabels.Enable(false);
        m_aLB_PlaceLabels.Enable(false);
        m_aFT_LabelDistance.Enable(false);
        m_aED_LabelDistance.Enable(false);

        m_aFL_Vertical.Enable(false);
        m_aFT_PlaceTicks.Enable(false);
        m_aLB_PlaceTicks.Enable(false);

        //todo: maybe set a special help id to all those controls
    }
}

int AxisPositionsTabPage::DeactivatePage(SfxItemSet* pItemSet)
{
    if( pItemSet )
        FillItemSet( *pItemSet );

    return LEAVE_PAGE;
}

void AxisPositionsTabPage::SetNumFormatter( SvNumberFormatter* pFormatter )
{
    m_pNumFormatter = pFormatter;
    m_aED_CrossesAt.SetFormatter( m_pNumFormatter );
    m_aED_CrossesAt.UseInputStringForFormatting();

    const SfxPoolItem *pPoolItem = NULL;
    if( GetItemSet().GetItemState( SCHATTR_AXIS_CROSSING_MAIN_AXIS_NUMBERFORMAT, sal_True, &pPoolItem ) == SFX_ITEM_SET )
    {
        sal_uLong nFmt = (sal_uLong)((const SfxInt32Item*)pPoolItem)->GetValue();
        m_aED_CrossesAt.SetFormatKey( nFmt );
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
    sal_uInt16 nPos = m_aLB_CrossesAt.GetSelectEntryPos();
    m_aED_CrossesAt.Show( (2==nPos) && !m_bCrossingAxisIsCategoryAxis );
    m_aED_CrossesAtCategory.Show( (2==nPos) && m_bCrossingAxisIsCategoryAxis );

    if( m_aED_CrossesAt.GetText().isEmpty() )
        m_aED_CrossesAt.SetValue(0.0);
    if( 0 == m_aED_CrossesAtCategory.GetSelectEntryCount() )
        m_aED_CrossesAtCategory.SelectEntryPos(0);

    PlaceLabelsSelectHdl( (void*)0 );
    return 0;
}

IMPL_LINK_NOARG(AxisPositionsTabPage, PlaceLabelsSelectHdl)
{
    sal_uInt16 nLabelPos = m_aLB_PlaceLabels.GetSelectEntryPos();

    bool bEnableTickmarkPlacement = (nLabelPos>1);
    if( bEnableTickmarkPlacement )
    {
        sal_uInt16 nAxisPos = m_aLB_CrossesAt.GetSelectEntryPos();
        if( nLabelPos-2 == nAxisPos )
            bEnableTickmarkPlacement=false;
    }
    m_aFT_PlaceTicks.Enable(bEnableTickmarkPlacement);
    m_aLB_PlaceTicks.Enable(bEnableTickmarkPlacement);

    return 0;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
