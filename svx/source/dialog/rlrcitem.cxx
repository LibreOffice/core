/*************************************************************************
 *
 *  $RCSfile: rlrcitem.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:11 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// INCLUDE ---------------------------------------------------------------

#ifndef _SFXRECTITEM_HXX
#include <svtools/rectitem.hxx>
#endif
#pragma hdrstop

#define ITEMID_LRSPACE  0
#define ITEMID_ULSPACE  0
#define ITEMID_TABSTOP  0
#define ITEMID_PROTECT  0

#include "dialogs.hrc"

#include "ruler.hxx"
#include "lrspitem.hxx"
#include "ulspitem.hxx"
#include "tstpitem.hxx"
#include "protitem.hxx"
#include "rlrcitem.hxx"
#include "rulritem.hxx"

// class SvxRulerItem ----------------------------------------------------

SvxRulerItem::SvxRulerItem(USHORT nId, SvxRuler &rRul, SfxBindings &rBindings)
: SfxControllerItem(nId, rBindings),
  rRuler(rRul)
{
}

// -----------------------------------------------------------------------

void SvxRulerItem::StateChanged( USHORT nSID, SfxItemState eState,
                                 const SfxPoolItem* pState)
{
    // SFX_ITEM_DONTCARE => pState == -1 => PTR_CAST buff
    if ( eState != SFX_ITEM_AVAILABLE )
        pState = 0;

    switch(nSID)
    {
        // Linker / rechter Seitenrand
        case SID_RULER_LR_MIN_MAX:
        {
            const SfxRectangleItem *pItem = PTR_CAST(SfxRectangleItem, pState);
            rRuler.UpdateFrameMinMax(pItem);
            break;
        }
        case SID_ATTR_LONG_LRSPACE:
        {
            const SvxLongLRSpaceItem *pItem = PTR_CAST(SvxLongLRSpaceItem, pState);
            DBG_ASSERT(pState? 0 != pItem: TRUE, "SvxLRSpaceItem erwartet");
            rRuler.UpdateFrame(pItem);
            break;
        }
        case SID_ATTR_LONG_ULSPACE:
        {
            const SvxLongULSpaceItem *pItem = PTR_CAST(SvxLongULSpaceItem, pState);
            DBG_ASSERT(pState? 0 != pItem: TRUE, "SvxULSpaceItem erwartet");
            rRuler.UpdateFrame(pItem);
            break;
        }
        case SID_ATTR_TABSTOP:
        {
            const SvxTabStopItem *pItem = PTR_CAST(SvxTabStopItem, pState);
            DBG_ASSERT(pState?  0 != pItem: TRUE, "SvxTabStopItem erwartet");
            rRuler.Update(pItem);
            break;
        }
        case SID_ATTR_PARA_LRSPACE:
        {
            const SvxLRSpaceItem *pItem = PTR_CAST(SvxLRSpaceItem, pState);
            DBG_ASSERT(pState?  0 != pItem: TRUE, "SvxLRSpaceItem erwartet");
            rRuler.UpdatePara(pItem);
            break;
        }
        case SID_RULER_BORDERS:
        {
            const SvxColumnItem *pItem = PTR_CAST(SvxColumnItem, pState);
            DBG_ASSERT(pState?  0 != pItem: TRUE, "SvxColumnItem erwartet");
#ifdef DBG_UTIL
            if(pItem)
            {
                if(pItem->IsConsistent())
                    rRuler.Update(pItem);
                else
                    DBG_ERROR("Spaltenitem corrupted");
            }
            else
                rRuler.Update(pItem);
#else
            rRuler.Update(pItem);
#endif
            break;
        }
        case SID_RULER_PAGE_POS:
        {   // Position Seite, Seitenbreite
            const SvxPagePosSizeItem *pItem = PTR_CAST(SvxPagePosSizeItem, pState);
            DBG_ASSERT(pState?  0 != pItem: TRUE, "SvxPagePosSizeItem erwartet");
            rRuler.Update(pItem);
            break;
        }
        case SID_RULER_OBJECT:
        {   // Object-Selektion
            const SvxObjectItem *pItem = PTR_CAST(SvxObjectItem, pState);
            DBG_ASSERT(pState?  0 != pItem: TRUE, "SvxObjectItem erwartet");
            rRuler.Update(pItem);
            break;
        }
        case SID_RULER_PROTECT:
        {
            const SvxProtectItem *pItem = PTR_CAST(SvxProtectItem, pState);
            DBG_ASSERT(pState?  0 != pItem: TRUE, "SvxProtectItem erwartet");
            rRuler.Update(pItem);
            break;
        }
    }
}


