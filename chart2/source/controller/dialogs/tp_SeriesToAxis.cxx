/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tp_SeriesToAxis.cxx,v $
 * $Revision: 1.11 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "tp_SeriesToAxis.hxx"

#include "ResId.hxx"
#include "TabPages.hrc"
#include "chartview/ChartSfxItemIds.hxx"
#include "NoWarningThisInCTOR.hxx"

// header for class SfxBoolItem
#include <svtools/eitem.hxx>
// header for SfxInt32Item
#include <svtools/intitem.hxx>
//SfxIntegerListItem
#include <svtools/ilstitem.hxx>
#include <com/sun/star/chart/MissingValueTreatment.hpp>

//.............................................................................
namespace chart
{
//.............................................................................

SchOptionTabPage::SchOptionTabPage(Window* pWindow,const SfxItemSet& rInAttrs) :
    SfxTabPage(pWindow, SchResId(TP_OPTIONS), rInAttrs),
    aGrpAxis(this, SchResId(GRP_OPT_AXIS)),
    aRbtAxis1(this,SchResId(RBT_OPT_AXIS_1)),
    aRbtAxis2(this,SchResId(RBT_OPT_AXIS_2)),

    aGrpBar(this, SchResId(GB_BAR)),
    aFTGap(this,SchResId(FT_GAP)),
    aMTGap(this,SchResId(MT_GAP)),
    aFTOverlap(this,SchResId(FT_OVERLAP)),
    aMTOverlap(this,SchResId(MT_OVERLAP)),
    aCBConnect(this,SchResId(CB_CONNECTOR)),
    aCBAxisSideBySide(this,SchResId(CB_BARS_SIDE_BY_SIDE)),
    m_aFL_EmptyCells(this,SchResId(FL_PLOT_MISSING_VALUES)),
    m_aRB_DontPaint(this,SchResId(RB_DONT_PAINT)),
    m_aRB_AssumeZero(this,SchResId(RB_ASSUME_ZERO)),
    m_aRB_ContinueLine(this,SchResId(RB_CONTINUE_LINE)),
    m_bProvidesSecondaryYAxis(true),
    m_bProvidesOverlapAndGapWidth(false)
{
    FreeResource();

    aRbtAxis1.SetClickHdl( LINK( this, SchOptionTabPage, EnableHdl ));
    aRbtAxis2.SetClickHdl( LINK( this, SchOptionTabPage, EnableHdl ));
}

SchOptionTabPage::~SchOptionTabPage()
{
}

IMPL_LINK( SchOptionTabPage, EnableHdl, RadioButton *, EMPTYARG )
{
    if( m_nAllSeriesAxisIndex == 0 )
        aCBAxisSideBySide.Enable( aRbtAxis2.IsChecked());
    else if( m_nAllSeriesAxisIndex == 1 )
        aCBAxisSideBySide.Enable( aRbtAxis1.IsChecked());

    return 0;
}

SfxTabPage* SchOptionTabPage::Create(Window* pWindow,const SfxItemSet& rOutAttrs)
{
    return new SchOptionTabPage(pWindow, rOutAttrs);
}

BOOL SchOptionTabPage::FillItemSet(SfxItemSet& rOutAttrs)
{
    if(aRbtAxis2.IsChecked())
        rOutAttrs.Put(SfxInt32Item(SCHATTR_AXIS,CHART_AXIS_SECONDARY_Y));
    else
        rOutAttrs.Put(SfxInt32Item(SCHATTR_AXIS,CHART_AXIS_PRIMARY_Y));

    if(aMTGap.IsVisible())
        rOutAttrs.Put(SfxInt32Item(SCHATTR_BAR_GAPWIDTH,static_cast< sal_Int32 >( aMTGap.GetValue())));

    if(aMTOverlap.IsVisible())
        rOutAttrs.Put(SfxInt32Item(SCHATTR_BAR_OVERLAP,static_cast< sal_Int32 >( aMTOverlap.GetValue())));

    if(aCBConnect.IsVisible())
        rOutAttrs.Put(SfxBoolItem(SCHATTR_BAR_CONNECT,aCBConnect.IsChecked()));

    // model property is "group bars per axis", UI feature is the other way
    // round: "show bars side by side"
    if(aCBAxisSideBySide.IsVisible())
        rOutAttrs.Put(SfxBoolItem(SCHATTR_GROUP_BARS_PER_AXIS, ! aCBAxisSideBySide.IsChecked()));

    if(m_aRB_DontPaint.IsChecked())
        rOutAttrs.Put(SfxInt32Item(SCHATTR_MISSING_VALUE_TREATMENT,::com::sun::star::chart::MissingValueTreatment::LEAVE_GAP));
    else if(m_aRB_AssumeZero.IsChecked())
        rOutAttrs.Put(SfxInt32Item(SCHATTR_MISSING_VALUE_TREATMENT,::com::sun::star::chart::MissingValueTreatment::USE_ZERO));
    else if(m_aRB_ContinueLine.IsChecked())
        rOutAttrs.Put(SfxInt32Item(SCHATTR_MISSING_VALUE_TREATMENT,::com::sun::star::chart::MissingValueTreatment::CONTINUE));

    return TRUE;
}

void SchOptionTabPage::Reset(const SfxItemSet& rInAttrs)
{
    const SfxPoolItem *pPoolItem = NULL;

    aRbtAxis1.Check(TRUE);
    aRbtAxis2.Check(FALSE);
    if (rInAttrs.GetItemState(SCHATTR_AXIS,TRUE, &pPoolItem) == SFX_ITEM_SET)
    {
        long nVal=((const SfxInt32Item*)pPoolItem)->GetValue();
        if(nVal==CHART_AXIS_SECONDARY_Y)
        {
            aRbtAxis2.Check(TRUE);
            aRbtAxis1.Check(FALSE);
        }
    }

    long nTmp;
    if (rInAttrs.GetItemState(SCHATTR_BAR_GAPWIDTH, TRUE, &pPoolItem) == SFX_ITEM_SET)
    {
        nTmp = (long)((const SfxInt32Item*)pPoolItem)->GetValue();
        aMTGap.SetValue(nTmp);
    }

    if (rInAttrs.GetItemState(SCHATTR_BAR_OVERLAP, TRUE, &pPoolItem) == SFX_ITEM_SET)
    {
        nTmp = (long)((const SfxInt32Item*)pPoolItem)->GetValue();
        aMTOverlap.SetValue(nTmp);
    }

    if (rInAttrs.GetItemState(SCHATTR_BAR_CONNECT, TRUE, &pPoolItem) == SFX_ITEM_SET)
    {
        BOOL bCheck = static_cast< const SfxBoolItem * >( pPoolItem )->GetValue();
        aCBConnect.Check(bCheck);
    }

    if (rInAttrs.GetItemState(SCHATTR_AXIS_FOR_ALL_SERIES, TRUE, &pPoolItem) == SFX_ITEM_SET)
    {
        m_nAllSeriesAxisIndex = static_cast< const SfxInt32Item * >( pPoolItem )->GetValue();
        aCBAxisSideBySide.Disable();
    }
    if (rInAttrs.GetItemState(SCHATTR_GROUP_BARS_PER_AXIS, TRUE, &pPoolItem) == SFX_ITEM_SET)
    {
        // model property is "group bars per axis", UI feature is the other way
        // round: "show bars side by side"
        BOOL bCheck = ! static_cast< const SfxBoolItem * >( pPoolItem )->GetValue();
        aCBAxisSideBySide.Check( bCheck );
    }
    else
    {
        aCBAxisSideBySide.Show(FALSE);
    }


    //missing value treatment
    {
        ::com::sun::star::uno::Sequence < sal_Int32 > aMissingValueTreatments;
        if( rInAttrs.GetItemState(SCHATTR_AVAILABLE_MISSING_VALUE_TREATMENTS, TRUE, &pPoolItem) == SFX_ITEM_SET )
            aMissingValueTreatments =((const SfxIntegerListItem*)pPoolItem)->GetConstSequence();

        if ( aMissingValueTreatments.getLength()>1 && rInAttrs.GetItemState(SCHATTR_MISSING_VALUE_TREATMENT,TRUE, &pPoolItem) == SFX_ITEM_SET)
        {
            m_aRB_DontPaint.Enable(FALSE);
            m_aRB_AssumeZero.Enable(FALSE);
            m_aRB_ContinueLine.Enable(FALSE);

            for( sal_Int32 nN =0; nN<aMissingValueTreatments.getLength(); nN++ )
            {
                sal_Int32 nVal = aMissingValueTreatments[nN];
                if(nVal==::com::sun::star::chart::MissingValueTreatment::LEAVE_GAP)
                    m_aRB_DontPaint.Enable(TRUE);
                else if(nVal==::com::sun::star::chart::MissingValueTreatment::USE_ZERO)
                    m_aRB_AssumeZero.Enable(TRUE);
                else if(nVal==::com::sun::star::chart::MissingValueTreatment::CONTINUE)
                    m_aRB_ContinueLine.Enable(TRUE);
            }

            long nVal=((const SfxInt32Item*)pPoolItem)->GetValue();
            if(nVal==::com::sun::star::chart::MissingValueTreatment::LEAVE_GAP)
                m_aRB_DontPaint.Check(TRUE);
            else if(nVal==::com::sun::star::chart::MissingValueTreatment::USE_ZERO)
                m_aRB_AssumeZero.Check(TRUE);
            else if(nVal==::com::sun::star::chart::MissingValueTreatment::CONTINUE)
                m_aRB_ContinueLine.Check(TRUE);
        }
        else
        {
            m_aRB_DontPaint.Show(FALSE);
            m_aRB_AssumeZero.Show(FALSE);
            m_aRB_ContinueLine.Show(FALSE);
            m_aFL_EmptyCells.Show(FALSE);
        }
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
    aRbtAxis1.Show(m_bProvidesSecondaryYAxis);
    aRbtAxis2.Show(m_bProvidesSecondaryYAxis);
    aGrpAxis.Show(m_bProvidesSecondaryYAxis);

    aMTGap.Show(m_bProvidesOverlapAndGapWidth);
    aFTGap.Show(m_bProvidesOverlapAndGapWidth);

    aMTOverlap.Show(m_bProvidesOverlapAndGapWidth);
    aFTOverlap.Show(m_bProvidesOverlapAndGapWidth);

    aCBConnect.Show(m_bProvidesBarConnectors);

    if( !aMTGap.IsVisible() && !aMTOverlap.IsVisible() )
    {
        aGrpBar.Show(FALSE);
        Point aPos;
        if( !aRbtAxis1.IsVisible() && !aRbtAxis2.IsVisible() )
            aPos = aGrpAxis.GetPosPixel();
        else
            aPos = aGrpBar.GetPosPixel();

        long nDiffX = aRbtAxis1.GetPosPixel().getX() - aGrpAxis.GetPosPixel().getX();
        long nDiffY = aRbtAxis1.GetPosPixel().getY() - aGrpAxis.GetPosPixel().getY();
        long nDiffY1 = aRbtAxis2.GetPosPixel().getY() - aRbtAxis1.GetPosPixel().getY();

        m_aFL_EmptyCells.SetPosPixel( aPos );
        m_aRB_DontPaint.SetPosPixel( Point( aPos.getX() + nDiffX, aPos.getY() + nDiffY ) );
        m_aRB_AssumeZero.SetPosPixel( Point( aPos.getX() + nDiffX, aPos.getY() + nDiffY + nDiffY1  ) );
        m_aRB_ContinueLine.SetPosPixel( Point( aPos.getX() + nDiffX, aPos.getY() + nDiffY + nDiffY1 * 2 ) );
    }

    if( !m_aRB_DontPaint.IsVisible() )
    {
        m_aRB_ContinueLine.SetPosPixel( m_aRB_AssumeZero.GetPosPixel() );
        m_aRB_AssumeZero.SetPosPixel( m_aRB_DontPaint.GetPosPixel() );
    }
}
//.............................................................................
} //namespace chart
//.............................................................................
