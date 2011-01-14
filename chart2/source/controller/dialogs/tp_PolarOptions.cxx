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

#include "precompiled_chart2.hxx"
#include "tp_PolarOptions.hxx"
#include "tp_PolarOptions.hrc"
#include "ResId.hxx"
#include "chartview/ChartSfxItemIds.hxx"
#include "NoWarningThisInCTOR.hxx"

#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <svtools/controldims.hrc>

//.............................................................................
namespace chart
{
//.............................................................................

PolarOptionsTabPage::PolarOptionsTabPage( Window* pWindow,const SfxItemSet& rInAttrs ) :
    SfxTabPage( pWindow, SchResId(TP_POLAROPTIONS), rInAttrs ),
    m_aCB_Clockwise( this, SchResId( CB_CLOCKWISE ) ),
    m_aFL_StartingAngle( this, SchResId( FL_STARTING_ANGLE ) ),
    m_aAngleDial( this, SchResId( CT_ANGLE_DIAL ) ),
    m_aFT_Degrees( this, SchResId( FT_ROTATION_DEGREES ) ),
    m_aNF_StartingAngle( this, SchResId( NF_STARTING_ANGLE ) ),
    m_aFL_PlotOptions( this, SchResId( FL_PLOT_OPTIONS_POLAR ) ),
    m_aCB_IncludeHiddenCells( this, SchResId( CB_INCLUDE_HIDDEN_CELLS_POLAR ) )
{
    FreeResource();

    m_aAngleDial.SetLinkedField( &m_aNF_StartingAngle );
}

PolarOptionsTabPage::~PolarOptionsTabPage()
{
}

SfxTabPage* PolarOptionsTabPage::Create( Window* pWindow,const SfxItemSet& rOutAttrs )
{
    return new PolarOptionsTabPage( pWindow, rOutAttrs );
}

sal_Bool PolarOptionsTabPage::FillItemSet( SfxItemSet& rOutAttrs )
{
    if( m_aAngleDial.IsVisible() )
    {
        rOutAttrs.Put(SfxInt32Item(SCHATTR_STARTING_ANGLE,
            static_cast< sal_Int32 >(m_aAngleDial.GetRotation()/100)));
    }

    if( m_aCB_Clockwise.IsVisible() )
        rOutAttrs.Put(SfxBoolItem(SCHATTR_CLOCKWISE,m_aCB_Clockwise.IsChecked()));

    if (m_aCB_IncludeHiddenCells.IsVisible())
        rOutAttrs.Put(SfxBoolItem(SCHATTR_INCLUDE_HIDDEN_CELLS, m_aCB_IncludeHiddenCells.IsChecked()));

    return sal_True;
}

void PolarOptionsTabPage::Reset(const SfxItemSet& rInAttrs)
{
    const SfxPoolItem *pPoolItem = NULL;

    long nTmp;
    if (rInAttrs.GetItemState(SCHATTR_STARTING_ANGLE, sal_True, &pPoolItem) == SFX_ITEM_SET)
    {
        nTmp = (long)((const SfxInt32Item*)pPoolItem)->GetValue();

        m_aAngleDial.SetRotation( nTmp*100 );
    }
    else
    {
        m_aFL_StartingAngle.Show(sal_False);
        m_aAngleDial.Show(sal_False);
        m_aNF_StartingAngle.Show(sal_False);
        m_aFT_Degrees.Show(sal_False);
    }
    if (rInAttrs.GetItemState(SCHATTR_CLOCKWISE, sal_True, &pPoolItem) == SFX_ITEM_SET)
    {
        sal_Bool bCheck = static_cast< const SfxBoolItem * >( pPoolItem )->GetValue();
        m_aCB_Clockwise.Check(bCheck);
    }
    else
    {
        m_aCB_Clockwise.Show(sal_False);
    }
    if (rInAttrs.GetItemState(SCHATTR_INCLUDE_HIDDEN_CELLS, sal_True, &pPoolItem) == SFX_ITEM_SET)
    {
        bool bVal = static_cast<const SfxBoolItem*>(pPoolItem)->GetValue();
        m_aCB_IncludeHiddenCells.Check(bVal);
    }
    else
    {
        m_aCB_IncludeHiddenCells.Show(sal_False);
        m_aFL_PlotOptions.Show(sal_False);
    }
}

//.............................................................................
} //namespace chart
//.............................................................................
