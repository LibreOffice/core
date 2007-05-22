/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tp_LegendPosition.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:45:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

#ifndef _SVX_CHRTITEM_HXX
#include <svx/chrtitem.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

SchLegendPosTabPage::SchLegendPosTabPage(Window* pWindow,
                                         const SfxItemSet& rInAttrs)
    : SfxTabPage( pWindow, SchResId(TP_LEGEND_POS), rInAttrs )
    , aGrpLegend( this, SchResId(GRP_LEGEND) )
    , m_apLegendPositionResources( new LegendPositionResources(this) )
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
    return TRUE;
}

void SchLegendPosTabPage::Reset(const SfxItemSet& rInAttrs)
{
    m_apLegendPositionResources->initFromItemSet(rInAttrs);
}

//.............................................................................
} //namespace chart
//.............................................................................
