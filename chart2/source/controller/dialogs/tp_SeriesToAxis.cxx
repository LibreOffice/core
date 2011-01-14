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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "tp_SeriesToAxis.hxx"
#include "tp_SeriesToAxis.hrc"

#include "ResId.hxx"
#include "chartview/ChartSfxItemIds.hxx"
#include "NoWarningThisInCTOR.hxx"

// header for class SfxBoolItem
#include <svl/eitem.hxx>
// header for SfxInt32Item
#include <svl/intitem.hxx>
//SfxIntegerListItem
#include <svl/ilstitem.hxx>
#include <svtools/controldims.hrc>

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
    m_aFL_PlotOptions(this,SchResId(FL_PLOT_OPTIONS)),
    m_aFT_MissingValues(this,SchResId(FT_MISSING_VALUES)),
    m_aRB_DontPaint(this,SchResId(RB_DONT_PAINT)),
    m_aRB_AssumeZero(this,SchResId(RB_ASSUME_ZERO)),
    m_aRB_ContinueLine(this,SchResId(RB_CONTINUE_LINE)),
    m_aCBIncludeHiddenCells(this,SchResId(CB_INCLUDE_HIDDEN_CELLS)),
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

sal_Bool SchOptionTabPage::FillItemSet(SfxItemSet& rOutAttrs)
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

    if (m_aCBIncludeHiddenCells.IsVisible())
        rOutAttrs.Put(SfxBoolItem(SCHATTR_INCLUDE_HIDDEN_CELLS, m_aCBIncludeHiddenCells.IsChecked()));

    return sal_True;
}

void SchOptionTabPage::Reset(const SfxItemSet& rInAttrs)
{
    const SfxPoolItem *pPoolItem = NULL;

    aRbtAxis1.Check(sal_True);
    aRbtAxis2.Check(sal_False);
    if (rInAttrs.GetItemState(SCHATTR_AXIS,sal_True, &pPoolItem) == SFX_ITEM_SET)
    {
        long nVal=((const SfxInt32Item*)pPoolItem)->GetValue();
        if(nVal==CHART_AXIS_SECONDARY_Y)
        {
            aRbtAxis2.Check(sal_True);
            aRbtAxis1.Check(sal_False);
        }
    }

    long nTmp;
    if (rInAttrs.GetItemState(SCHATTR_BAR_GAPWIDTH, sal_True, &pPoolItem) == SFX_ITEM_SET)
    {
        nTmp = (long)((const SfxInt32Item*)pPoolItem)->GetValue();
        aMTGap.SetValue(nTmp);
    }

    if (rInAttrs.GetItemState(SCHATTR_BAR_OVERLAP, sal_True, &pPoolItem) == SFX_ITEM_SET)
    {
        nTmp = (long)((const SfxInt32Item*)pPoolItem)->GetValue();
        aMTOverlap.SetValue(nTmp);
    }

    if (rInAttrs.GetItemState(SCHATTR_BAR_CONNECT, sal_True, &pPoolItem) == SFX_ITEM_SET)
    {
        sal_Bool bCheck = static_cast< const SfxBoolItem * >( pPoolItem )->GetValue();
        aCBConnect.Check(bCheck);
    }

    if (rInAttrs.GetItemState(SCHATTR_AXIS_FOR_ALL_SERIES, sal_True, &pPoolItem) == SFX_ITEM_SET)
    {
        m_nAllSeriesAxisIndex = static_cast< const SfxInt32Item * >( pPoolItem )->GetValue();
        aCBAxisSideBySide.Disable();
    }
    if (rInAttrs.GetItemState(SCHATTR_GROUP_BARS_PER_AXIS, sal_True, &pPoolItem) == SFX_ITEM_SET)
    {
        // model property is "group bars per axis", UI feature is the other way
        // round: "show bars side by side"
        sal_Bool bCheck = ! static_cast< const SfxBoolItem * >( pPoolItem )->GetValue();
        aCBAxisSideBySide.Check( bCheck );
    }
    else
    {
        aCBAxisSideBySide.Show(sal_False);
    }


    //missing value treatment
    {
        ::com::sun::star::uno::Sequence < sal_Int32 > aMissingValueTreatments;
        if( rInAttrs.GetItemState(SCHATTR_AVAILABLE_MISSING_VALUE_TREATMENTS, sal_True, &pPoolItem) == SFX_ITEM_SET )
            aMissingValueTreatments =((const SfxIntegerListItem*)pPoolItem)->GetConstSequence();

        if ( aMissingValueTreatments.getLength()>1 && rInAttrs.GetItemState(SCHATTR_MISSING_VALUE_TREATMENT,sal_True, &pPoolItem) == SFX_ITEM_SET)
        {
            m_aRB_DontPaint.Enable(sal_False);
            m_aRB_AssumeZero.Enable(sal_False);
            m_aRB_ContinueLine.Enable(sal_False);

            for( sal_Int32 nN =0; nN<aMissingValueTreatments.getLength(); nN++ )
            {
                sal_Int32 nVal = aMissingValueTreatments[nN];
                if(nVal==::com::sun::star::chart::MissingValueTreatment::LEAVE_GAP)
                    m_aRB_DontPaint.Enable(sal_True);
                else if(nVal==::com::sun::star::chart::MissingValueTreatment::USE_ZERO)
                    m_aRB_AssumeZero.Enable(sal_True);
                else if(nVal==::com::sun::star::chart::MissingValueTreatment::CONTINUE)
                    m_aRB_ContinueLine.Enable(sal_True);
            }

            long nVal=((const SfxInt32Item*)pPoolItem)->GetValue();
            if(nVal==::com::sun::star::chart::MissingValueTreatment::LEAVE_GAP)
                m_aRB_DontPaint.Check(sal_True);
            else if(nVal==::com::sun::star::chart::MissingValueTreatment::USE_ZERO)
                m_aRB_AssumeZero.Check(sal_True);
            else if(nVal==::com::sun::star::chart::MissingValueTreatment::CONTINUE)
                m_aRB_ContinueLine.Check(sal_True);
        }
        else
        {
            m_aFT_MissingValues.Show(sal_False);
            m_aRB_DontPaint.Show(sal_False);
            m_aRB_AssumeZero.Show(sal_False);
            m_aRB_ContinueLine.Show(sal_False);
        }
    }

    // Include hidden cells
    if (rInAttrs.GetItemState(SCHATTR_INCLUDE_HIDDEN_CELLS, sal_True, &pPoolItem) == SFX_ITEM_SET)
    {
        bool bVal = static_cast<const SfxBoolItem*>(pPoolItem)->GetValue();
        m_aCBIncludeHiddenCells.Check(bVal);
    }
    else
    {
        m_aCBIncludeHiddenCells.Show(sal_False);
        if(!m_aFT_MissingValues.IsVisible())
            m_aFL_PlotOptions.Show(sal_False);
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

void lcl_offsetControl(Control& rCtrl, long nXOffset, long nYOffset )
{
    Point aPos = rCtrl.GetPosPixel();
    rCtrl.SetPosPixel( Point(aPos.getX() + nXOffset, aPos.getY() + nYOffset) );
}

void lcl_optimzeRadioButtonSize( RadioButton& rCtrl )
{
    rCtrl.SetSizePixel( rCtrl.CalcMinimumSize() );
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
        aGrpBar.Show(sal_False);
        Point aPos;
        if( !aRbtAxis1.IsVisible() && !aRbtAxis2.IsVisible() )
            aPos = aGrpAxis.GetPosPixel();
        else
            aPos = aGrpBar.GetPosPixel();

        long nYOffset = aPos.getY() - m_aFL_PlotOptions.GetPosPixel().getY();
        lcl_offsetControl(m_aFL_PlotOptions,       0, nYOffset);
        lcl_offsetControl(m_aFT_MissingValues,     0, nYOffset);
        lcl_offsetControl(m_aRB_DontPaint,         0, nYOffset);
        lcl_offsetControl(m_aRB_AssumeZero,        0, nYOffset);
        lcl_offsetControl(m_aRB_ContinueLine,      0, nYOffset);
        lcl_offsetControl(m_aCBIncludeHiddenCells, 0, nYOffset);
    }

    m_aFT_MissingValues.SetSizePixel( m_aFT_MissingValues.CalcMinimumSize() );
    lcl_optimzeRadioButtonSize( m_aRB_DontPaint );
    lcl_optimzeRadioButtonSize( m_aRB_AssumeZero );
    lcl_optimzeRadioButtonSize( m_aRB_ContinueLine );

    Size aControlDistance( m_aFT_MissingValues.LogicToPixel( Size(RSC_SP_CTRL_DESC_X,RSC_SP_CTRL_GROUP_Y), MapMode(MAP_APPFONT) ) );
    long nXOffset = m_aFT_MissingValues.GetPosPixel().getX() + m_aFT_MissingValues.GetSizePixel().getWidth() + aControlDistance.getWidth() - m_aRB_DontPaint.GetPosPixel().getX();
    lcl_offsetControl(m_aRB_DontPaint,         nXOffset, 0);
    lcl_offsetControl(m_aRB_AssumeZero,        nXOffset, 0);
    lcl_offsetControl(m_aRB_ContinueLine,      nXOffset, 0);

    if( !m_aFT_MissingValues.IsVisible() )
    {
        //for example for stock charts
        m_aCBIncludeHiddenCells.SetPosPixel( m_aFT_MissingValues.GetPosPixel() );
    }
}
//.............................................................................
} //namespace chart
//.............................................................................
