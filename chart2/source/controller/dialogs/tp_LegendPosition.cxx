/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tp_LegendPosition.cxx,v $
 * $Revision: 1.8.72.1 $
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
#include "tp_LegendPosition.hxx"
#include "ResId.hxx"
#include "TabPages.hrc"
#include "res_LegendPosition.hxx"
#include "chartview/ChartSfxItemIds.hxx"
#include "NoWarningThisInCTOR.hxx"
#include <svx/chrtitem.hxx>
#include <svx/eeitem.hxx>
#include <svx/frmdiritem.hxx>

//.............................................................................
namespace chart
{
//.............................................................................

SchLegendPosTabPage::SchLegendPosTabPage(Window* pWindow,
                                         const SfxItemSet& rInAttrs)
    : SfxTabPage( pWindow, SchResId(TP_LEGEND_POS), rInAttrs )
    , aGrpLegend( this, SchResId(GRP_LEGEND) )
    , m_apLegendPositionResources( new LegendPositionResources(this) )
    , m_aFlTextOrient( this, SchResId( FL_LEGEND_TEXTORIENT ) )
    , m_aFtTextDirection( this, SchResId( FT_LEGEND_TEXTDIR ) )
    , m_aLbTextDirection( this, SchResId( LB_LEGEND_TEXTDIR ), &m_aFlTextOrient, &m_aFtTextDirection )
{
    FreeResource();
}

SchLegendPosTabPage::~SchLegendPosTabPage()
{
}

SfxTabPage* SchLegendPosTabPage::Create(Window* pWindow,
                                        const SfxItemSet& rOutAttrs)
{
    return new SchLegendPosTabPage(pWindow, rOutAttrs);
}

BOOL SchLegendPosTabPage::FillItemSet(SfxItemSet& rOutAttrs)
{
    m_apLegendPositionResources->writeToItemSet(rOutAttrs);

    if( m_aLbTextDirection.GetSelectEntryCount() > 0 )
        rOutAttrs.Put( SfxInt32Item( EE_PARA_WRITINGDIR, m_aLbTextDirection.GetSelectEntryValue() ) );

    return TRUE;
}

void SchLegendPosTabPage::Reset(const SfxItemSet& rInAttrs)
{
    m_apLegendPositionResources->initFromItemSet(rInAttrs);

    const SfxPoolItem* pPoolItem = 0;
    if( rInAttrs.GetItemState( EE_PARA_WRITINGDIR, TRUE, &pPoolItem ) == SFX_ITEM_SET )
        m_aLbTextDirection.SelectEntryValue( SvxFrameDirection(((const SvxFrameDirectionItem*)pPoolItem)->GetValue()) );
}

//.............................................................................
} //namespace chart
//.............................................................................
