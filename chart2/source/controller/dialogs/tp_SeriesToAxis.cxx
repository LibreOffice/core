/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tp_SeriesToAxis.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:18:12 $
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
#include "tp_SeriesToAxis.hxx"

#include "ResId.hxx"
#include "TabPages.hrc"
#include "SchSfxItemIds.hxx"

// header for class SfxBoolItem
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
// header for SfxInt32Item
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif


/*
#include "schattr.hxx"
#include "schresid.hxx"
#include "chtmodel.hxx"
#include "attrib.hxx"
#include "attrib.hrc"

#ifndef _SVX_SVXIDS_HRC //autogen
#include <svx/svxids.hrc>
#endif
*/

//.............................................................................
namespace chart
{
//.............................................................................

SchOptionTabPage::SchOptionTabPage(Window* pWindow,const SfxItemSet& rInAttrs) :
    SfxTabPage(pWindow, SchResId(TP_OPTIONS), rInAttrs),
    aGrpAxis(this, ResId(GRP_OPT_AXIS)),
    aRbtAxis1(this,ResId(RBT_OPT_AXIS_1)),
    aRbtAxis2(this,ResId(RBT_OPT_AXIS_2)),

    aGrpBar(this, ResId(GB_BAR)),
    aFTGap(this,ResId(FT_GAP)),
    aMTGap(this,ResId(MT_GAP)),
    aFTOverlap(this,ResId(FT_OVERLAP)),
    aMTOverlap(this,ResId(MT_OVERLAP)),
    aCBConnect(this,ResId(CB_CONNECTOR))
{
    FreeResource();

}
/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/

SchOptionTabPage::~SchOptionTabPage()
{
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
        rOutAttrs.Put(SfxInt32Item(CHATTR_DIAGRAM_GAPWIDTH,aMTGap.GetValue()));

    if(aMTOverlap.IsVisible())
        rOutAttrs.Put(SfxInt32Item(CHATTR_DIAGRAM_OVERLAP,aMTOverlap.GetValue()));

    if(aCBConnect.IsVisible())
        rOutAttrs.Put(SfxBoolItem(CHATTR_BARCONNECT,aCBConnect.IsChecked()));

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
    if (rInAttrs.GetItemState(CHATTR_DIAGRAM_GAPWIDTH, TRUE, &pPoolItem) == SFX_ITEM_SET)
    {
        nTmp = (long)((const SfxInt32Item*)pPoolItem)->GetValue();
        aMTGap.SetValue(nTmp);
    }
    else
    {
        aMTGap.Show(FALSE);
        aFTGap.Show(FALSE);
    }

    if (rInAttrs.GetItemState(CHATTR_DIAGRAM_OVERLAP, TRUE, &pPoolItem) == SFX_ITEM_SET)
    {
        nTmp = (long)((const SfxInt32Item*)pPoolItem)->GetValue();
        aMTOverlap.SetValue(nTmp);
    }
    else
    {
        aMTOverlap.Show(FALSE);
        aFTOverlap.Show(FALSE);
    }
    if (rInAttrs.GetItemState(CHATTR_BARCONNECT, TRUE, &pPoolItem) == SFX_ITEM_SET)
    {
        nTmp = (long)((const SfxInt32Item*)pPoolItem)->GetValue();
        aCBConnect.Check(nTmp);
    }
    else
    {
        aCBConnect.Show(FALSE);
    }


}

//.............................................................................
} //namespace chart
//.............................................................................
