/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rlrcitem.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 04:36:29 $
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
#include "precompiled_svx.hxx"

// INCLUDE ---------------------------------------------------------------

#ifndef _SFXRECTITEM_HXX
#include <svtools/rectitem.hxx>
#endif

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
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif

// class SvxRulerItem ----------------------------------------------------

SvxRulerItem::SvxRulerItem(USHORT _nId, SvxRuler &rRul, SfxBindings &rBindings)
: SfxControllerItem(_nId, rBindings),
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
        case SID_ATTR_TABSTOP_VERTICAL:
        case SID_ATTR_TABSTOP:
        {
            const SvxTabStopItem *pItem = PTR_CAST(SvxTabStopItem, pState);
            DBG_ASSERT(pState?  0 != pItem: TRUE, "SvxTabStopItem erwartet");
            rRuler.Update(pItem);
            break;
        }
        case SID_ATTR_PARA_LRSPACE_VERTICAL:
        case SID_ATTR_PARA_LRSPACE:
        {
            const SvxLRSpaceItem *pItem = PTR_CAST(SvxLRSpaceItem, pState);
            DBG_ASSERT(pState?  0 != pItem: TRUE, "SvxLRSpaceItem erwartet");
            rRuler.UpdatePara(pItem);
            break;
        }
        case SID_RULER_BORDERS_VERTICAL:
        case SID_RULER_BORDERS:
        case SID_RULER_ROWS:
        case SID_RULER_ROWS_VERTICAL:
        {
            const SvxColumnItem *pItem = PTR_CAST(SvxColumnItem, pState);
            DBG_ASSERT(pState?  0 != pItem: TRUE, "SvxColumnItem erwartet");
#ifdef DBG_UTIL
            if(pItem)
            {
                if(pItem->IsConsistent())
                    rRuler.Update(pItem, nSID);
                else
                    DBG_ERROR("Spaltenitem corrupted");
            }
            else
                rRuler.Update(pItem, nSID);
#else
            rRuler.Update(pItem, nSID);
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
        case SID_RULER_BORDER_DISTANCE:
        {
            const SvxLRSpaceItem *pItem = PTR_CAST(SvxLRSpaceItem, pState);
            DBG_ASSERT(pState?  0 != pItem: TRUE, "SvxLRSpaceItem erwartet");
            rRuler.UpdateParaBorder(pItem);
        }
        break;
        case SID_RULER_TEXT_RIGHT_TO_LEFT :
        {
            const SfxBoolItem *pItem = PTR_CAST(SfxBoolItem, pState);
            DBG_ASSERT(pState?  0 != pItem: TRUE, "SfxBoolItem erwartet");
            rRuler.UpdateTextRTL(pItem);
        }
        break;
    }
}


