/*************************************************************************
 *
 *  $RCSfile: tp_PointGeometry.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:26 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
    aListShapes(this,ResId(LB_LY_SHAPE)),
    aFtLayout(this, ResId(FT_LY_SHAPE)),
    rOutAttrs(rInAttrs)
{
    FreeResource();
}
/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/

SchLayoutTabPage::~SchLayoutTabPage()
{
}

/*************************************************************************
|*
|* Erzeugung
|*
\*************************************************************************/

SfxTabPage* SchLayoutTabPage::Create(Window* pWindow,
                                        const SfxItemSet& rOutAttrs)
{
    return new SchLayoutTabPage(pWindow, rOutAttrs);
}

/*************************************************************************
|*
|* Fuellt uebergebenen Item-Set mit Dialogbox-Attributen
|*
\*************************************************************************/
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
//-/        rOutAttrs.Put(SfxUInt32Item(SID_ATTR_3D_HORZ_SEGS,nSegs));
        rOutAttrs.Put(Svx3DHorizontalSegmentsItem(nSegs));
    }
    return TRUE;
}
/*************************************************************************
|*
|* Initialisierung
|*
\*************************************************************************/

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
