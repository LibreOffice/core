/*************************************************************************
 *
 *  $RCSfile: dlg_InsertLegend.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:25 $
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
#include "dlg_InsertLegend.hxx"
#include "dlg_InsertLegend.hrc"

#include "ResId.hxx"
#include "SchSfxItemIds.hxx"

// header for enum SvxChartLegendPos
#define ITEMID_CHARTLEGENDPOS   SCHATTR_LEGEND_POS
#ifndef _SVX_CHRTITEM_HXX
#include <svx/chrtitem.hxx>
#endif
// header for class SfxItemPool
#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

/*************************************************************************
|*
|* Dialog zur Ausrichtung der Legende
|*
\************************************************************************/

SchLegendDlg::SchLegendDlg(Window* pWindow, const SfxItemSet& rInAttrs) :
    ModalDialog(pWindow, SchResId(DLG_LEGEND)),
    aCbxShow(this, ResId(CBX_SHOW)),
    aRbtLeft(this, ResId(RBT_LEFT)),
    aRbtTop(this, ResId(RBT_TOP)),
    aRbtRight(this, ResId(RBT_RIGHT)),
    aRbtBottom(this, ResId(RBT_BOTTOM)),
    aFlLegend(this, ResId(FL_LEGEND)),
    aBtnOK(this, ResId(BTN_OK)),
    aBtnCancel(this, ResId(BTN_CANCEL)),
    aBtnHelp(this, ResId(BTN_HELP)),
    rOutAttrs(rInAttrs)
{
    FreeResource();

    aCbxShow.SetClickHdl (LINK(this, SchLegendDlg, CbxClick));

    Reset();
}

/*************************************************************************
|*
|* Dtor
|*
/************************************************************************/

SchLegendDlg::~SchLegendDlg()
{
}

/*************************************************************************
|*
|*  Initialisierung
|*
\*************************************************************************/

void SchLegendDlg::Reset()
{
    SvxChartLegendPos ePos = CHLEGEND_NONE;

    const SfxPoolItem* pPoolItem = NULL;
    if( rOutAttrs.GetItemState( SCHATTR_LEGEND_POS,
                               TRUE, &pPoolItem ) != SFX_ITEM_SET )
        pPoolItem = &(rOutAttrs.GetPool()->GetDefaultItem( SCHATTR_LEGEND_POS ));

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
        case CHLEGEND_NONE:
        case CHLEGEND_RIGHT:
            aRbtRight.Check(TRUE);
            break;
        case CHLEGEND_BOTTOM:
            aRbtBottom.Check(TRUE);
            break;
    }

    aCbxShow.Check (ePos != CHLEGEND_NONE);
    aRbtLeft.Enable (ePos != CHLEGEND_NONE);
    aRbtTop.Enable (ePos != CHLEGEND_NONE);
    aRbtRight.Enable (ePos != CHLEGEND_NONE);
    aRbtBottom.Enable (ePos != CHLEGEND_NONE);
}

/*************************************************************************
|*
|*    Fuellt uebergebenen Item-Set mit Dialogbox-Attributen
|*
\*************************************************************************/

void SchLegendDlg::GetAttr(SfxItemSet& rOutAttrs)
{
    SvxChartLegendPos ePos;

    if ( ! aCbxShow.IsChecked())
        ePos = CHLEGEND_NONE;
    else if (aRbtLeft.IsChecked())
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
}




IMPL_LINK (SchLegendDlg, CbxClick, CheckBox *, pBtn)
{
    BOOL    bChecked = aCbxShow.IsChecked();

    aRbtLeft.Enable (bChecked);
    aRbtTop.Enable (bChecked);
    aRbtRight.Enable (bChecked);
    aRbtBottom.Enable (bChecked);

    return 0;
}

//.............................................................................
} //namespace chart
//.............................................................................
