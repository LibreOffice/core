/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tp_SeriesToAxis.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-22 16:48:53 $
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
#include "tp_SeriesToAxis.hxx"

#include "ResId.hxx"
#include "TabPages.hrc"
#include "chartview/ChartSfxItemIds.hxx"
#include "NoWarningThisInCTOR.hxx"

// header for class SfxBoolItem
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
// header for SfxInt32Item
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif

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
    aCBAxisSideBySide(this,SchResId(CB_BARS_SIDE_BY_SIDE))
{
    FreeResource();

    aRbtAxis1.SetClickHdl( LINK( this, SchOptionTabPage, EnableHdl ));
    aRbtAxis2.SetClickHdl( LINK( this, SchOptionTabPage, EnableHdl ));
}
/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/

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
/*************************************************************************
|*
|* Erzeugung
|*
\*************************************************************************/

SfxTabPage* SchOptionTabPage::Create(Window* pWindow,const SfxItemSet& rOutAttrs)
{
    return new SchOptionTabPage(pWindow, rOutAttrs);
}

/*************************************************************************
|*
|* Fuellt uebergebenen Item-Set mit Dialogbox-Attributen
|*
\*************************************************************************/
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

    return TRUE;
}
/*************************************************************************
|*
|* Initialisierung
|*
\*************************************************************************/

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
    else
    {
        aMTGap.Show(FALSE);
        aFTGap.Show(FALSE);
    }

    if (rInAttrs.GetItemState(SCHATTR_BAR_OVERLAP, TRUE, &pPoolItem) == SFX_ITEM_SET)
    {
        nTmp = (long)((const SfxInt32Item*)pPoolItem)->GetValue();
        aMTOverlap.SetValue(nTmp);
    }
    else
    {
        aMTOverlap.Show(FALSE);
        aFTOverlap.Show(FALSE);
    }
    if (rInAttrs.GetItemState(SCHATTR_BAR_CONNECT, TRUE, &pPoolItem) == SFX_ITEM_SET)
    {
        BOOL bCheck = static_cast< const SfxBoolItem * >( pPoolItem )->GetValue();
        aCBConnect.Check(bCheck);
    }
    else
    {
        aCBConnect.Show(FALSE);
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

}
//.............................................................................
} //namespace chart
//.............................................................................
