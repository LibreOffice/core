/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tp_LegendPosition.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:15:12 $
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
#include "tp_LegendPosition.hxx"

#include "ResId.hxx"
#include "TabPages.hrc"
#include "SchSfxItemIds.hxx"

//#include "schattr.hxx"
#define ITEMID_CHARTLEGENDPOS   SCHATTR_LEGEND_POS
#ifndef _SVX_CHRTITEM_HXX //autogen
#include <svx/chrtitem.hxx>
#endif
/*
#include "schresid.hxx"
#include "chtmodel.hxx"
#include "attrib.hxx"
#include "attrib.hrc"
*/
//.............................................................................
namespace chart
{
//.............................................................................

SchLegendPosTabPage::SchLegendPosTabPage(Window* pWindow,
                                         const SfxItemSet& rInAttrs) :
    SfxTabPage(pWindow, SchResId(TP_LEGEND_POS), rInAttrs),
    aGrpLegend(this, ResId(GRP_LEGEND)),
    aRbtLeft(this, ResId(RBT_LEFT)),
    aRbtTop(this, ResId(RBT_TOP)),
    aRbtBottom(this, ResId(RBT_BOTTOM)),
    aRbtRight(this, ResId(RBT_RIGHT))
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
    SvxChartLegendPos ePos;

    if (aRbtLeft.IsChecked())
        ePos = CHLEGEND_LEFT;
    else if (aRbtTop.IsChecked())
        ePos = CHLEGEND_TOP;
    else if (aRbtRight.IsChecked())
        ePos = CHLEGEND_RIGHT;
    else if (aRbtBottom.IsChecked())
        ePos = CHLEGEND_BOTTOM;
    else
        ePos = CHLEGEND_NONE;

    rOutAttrs.Put(SvxChartLegendPosItem(ePos));

    return TRUE;
}

void SchLegendPosTabPage::Reset(const SfxItemSet& rInAttrs)
{
    SvxChartLegendPos ePos = CHLEGEND_NONE;

    const SfxPoolItem* pPoolItem = NULL;
    if( rInAttrs.GetItemState( SCHATTR_LEGEND_POS,
                               TRUE, &pPoolItem ) != SFX_ITEM_SET )
        pPoolItem = &(rInAttrs.GetPool()->GetDefaultItem( SCHATTR_LEGEND_POS ));

    if( pPoolItem )
        ePos = ((const SvxChartLegendPosItem*)pPoolItem)->GetValue();

    switch( ePos )
    {
        case CHLEGEND_LEFT:
            aRbtLeft.Check(TRUE);
            break;
        case CHLEGEND_TOP:
            aRbtTop.Check(TRUE);
            break;
        case CHLEGEND_RIGHT:
            aRbtRight.Check(TRUE);
            break;
        case CHLEGEND_BOTTOM:
            aRbtBottom.Check(TRUE);
            break;
        default:
            break;
    }
}

//.............................................................................
} //namespace chart
//.............................................................................
