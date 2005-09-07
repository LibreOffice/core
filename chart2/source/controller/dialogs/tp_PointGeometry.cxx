/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tp_PointGeometry.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:15:55 $
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
#include "tp_PointGeometry.hxx"

#include "ResId.hxx"
#include "TabPages.hrc"
#include "SchSfxItemIds.hxx"

// header for SfxInt32Item
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
// header for class Svx3DHorizontalSegmentsItem
#ifndef _SVX3DITEMS_HXX
#include <svx/svx3ditems.hxx>
#endif
/*
#include "schattr.hxx"
#include "chmod3d.hxx"
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

SchLayoutTabPage::SchLayoutTabPage(Window* pWindow,const SfxItemSet& rInAttrs) :
    SfxTabPage(pWindow, SchResId(TP_LAYOUT), rInAttrs),
/*  aSquare(this, ResId(RBT_LY_CUBE)),
    aCylinder(this, ResId(RBT_LY_CYLINDER)),
    aCone(this, ResId(RBT_LY_CONE)),
    aPyramid(this, ResId(RBT_LY_PYRAMID)),
    aHanoi(this,ResId(RBT_LY_HANOI)),
    */
    aFtLayout(this, ResId(FT_LY_SHAPE)),
    aListShapes(this,ResId(LB_LY_SHAPE))
{
    FreeResource();
}

SchLayoutTabPage::~SchLayoutTabPage()
{
}

SfxTabPage* SchLayoutTabPage::Create(Window* pWindow,
                                        const SfxItemSet& rOutAttrs)
{
    return new SchLayoutTabPage(pWindow, rOutAttrs);
}

BOOL SchLayoutTabPage::FillItemSet(SfxItemSet& rOutAttrs)
{

    if(aListShapes.GetSelectEntryCount())
    {
        long nShape=CHART_SHAPE3D_SQUARE;
        long nSegs=32;

        nShape = aListShapes.GetSelectEntryPos();
        if(nShape==CHART_SHAPE3D_PYRAMID)
            nSegs=4;

        rOutAttrs.Put(SfxInt32Item(SCHATTR_STYLE_SHAPE,nShape));
        rOutAttrs.Put(Svx3DHorizontalSegmentsItem(nSegs));
    }
    return TRUE;
}

void SchLayoutTabPage::Reset(const SfxItemSet& rInAttrs)
{
    const SfxPoolItem *pPoolItem = NULL;

    if (rInAttrs.GetItemState(SCHATTR_STYLE_SHAPE,TRUE, &pPoolItem) == SFX_ITEM_SET)
    {
        long nVal=((const SfxInt32Item*)pPoolItem)->GetValue();
        aListShapes.SelectEntryPos(nVal);
    }
/*  switch (nVal)
    {
        case CHART_SHAPE3D_SQUARE:
            aSquare.Check(TRUE);
            break;
        case CHART_SHAPE3D_CYLINDER:
            aCylinder.Check(TRUE);
            break;
        case CHART_SHAPE3D_CONE:
            aCone.Check(TRUE);
            break;
        case CHART_SHAPE3D_PYRAMID:
            aPyramid.Check(TRUE);
            break;
        case CHART_SHAPE3D_HANOI:
            aHanoi.Check(TRUE);
            break;
    }
*/
}

//.............................................................................
} //namespace chart
//.............................................................................
