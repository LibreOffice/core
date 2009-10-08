/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tp_PolarOptions.cxx,v $
 * $Revision: 1.3 $
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
#include "TabPages.hrc"
#include "chartview/ChartSfxItemIds.hxx"
#include "NoWarningThisInCTOR.hxx"

#include <svtools/eitem.hxx>
#include <svtools/intitem.hxx>
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
    m_aNF_StartingAngle( this, SchResId( NF_STARTING_ANGLE ) )
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

BOOL PolarOptionsTabPage::FillItemSet( SfxItemSet& rOutAttrs )
{
    if( m_aAngleDial.IsVisible() )
    {
        rOutAttrs.Put(SfxInt32Item(SCHATTR_STARTING_ANGLE,
            static_cast< sal_Int32 >(m_aAngleDial.GetRotation()/100)));
    }

    if( m_aCB_Clockwise.IsVisible() )
        rOutAttrs.Put(SfxBoolItem(SCHATTR_CLOCKWISE,m_aCB_Clockwise.IsChecked()));

    return TRUE;
}

void PolarOptionsTabPage::Reset(const SfxItemSet& rInAttrs)
{
    const SfxPoolItem *pPoolItem = NULL;

    long nTmp;
    if (rInAttrs.GetItemState(SCHATTR_STARTING_ANGLE, TRUE, &pPoolItem) == SFX_ITEM_SET)
    {
        nTmp = (long)((const SfxInt32Item*)pPoolItem)->GetValue();

        m_aAngleDial.SetRotation( nTmp*100 );
    }
    else
    {
        m_aFL_StartingAngle.Show(FALSE);
        m_aAngleDial.Show(FALSE);
        m_aNF_StartingAngle.Show(FALSE);
        m_aFT_Degrees.Show(FALSE);
    }
    if (rInAttrs.GetItemState(SCHATTR_CLOCKWISE, TRUE, &pPoolItem) == SFX_ITEM_SET)
    {
        BOOL bCheck = static_cast< const SfxBoolItem * >( pPoolItem )->GetValue();
        m_aCB_Clockwise.Check(bCheck);
    }
    else
    {
        m_aCB_Clockwise.Show(FALSE);
    }
}

//.............................................................................
} //namespace chart
//.............................................................................
