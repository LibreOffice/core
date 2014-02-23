/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <svl/rectitem.hxx>






#include <svx/dialogs.hrc>

#include <svx/ruler.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/tstpitem.hxx>
#include "editeng/protitem.hxx"
#include "rlrcitem.hxx"
#include "svx/rulritem.hxx"
#include <svl/eitem.hxx>

// class SvxRulerItem ----------------------------------------------------

SvxRulerItem::SvxRulerItem(sal_uInt16 _nId, SvxRuler &rRul, SfxBindings &rBindings)
: SfxControllerItem(_nId, rBindings),
  rRuler(rRul)
{
}



void SvxRulerItem::StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                 const SfxPoolItem* pState)
{
    // SFX_ITEM_DONTCARE => pState == -1 => PTR_CAST buff
    if ( eState != SFX_ITEM_AVAILABLE )
        pState = 0;

    switch(nSID)
    {
        // Left / right margin
        case SID_RULER_LR_MIN_MAX:
        {
            const SfxRectangleItem *pItem = PTR_CAST(SfxRectangleItem, pState);
            rRuler.UpdateFrameMinMax(pItem);
            break;
        }
        case SID_ATTR_LONG_LRSPACE:
        {
            const SvxLongLRSpaceItem *pItem = PTR_CAST(SvxLongLRSpaceItem, pState);
            DBG_ASSERT(pState? 0 != pItem: true, "SvxLRSpaceItem expected");
            rRuler.UpdateFrame(pItem);
            break;
        }
        case SID_ATTR_LONG_ULSPACE:
        {
            const SvxLongULSpaceItem *pItem = PTR_CAST(SvxLongULSpaceItem, pState);
            DBG_ASSERT(pState? 0 != pItem: true, "SvxULSpaceItem expected");
            rRuler.UpdateFrame(pItem);
            break;
        }
        case SID_ATTR_TABSTOP_VERTICAL:
        case SID_ATTR_TABSTOP:
        {
            const SvxTabStopItem *pItem = PTR_CAST(SvxTabStopItem, pState);
            DBG_ASSERT(pState?  0 != pItem: true, "SvxTabStopItem expected");
            rRuler.Update(pItem);
            break;
        }
        case SID_ATTR_PARA_LRSPACE_VERTICAL:
        case SID_ATTR_PARA_LRSPACE:
        {
            const SvxLRSpaceItem *pItem = PTR_CAST(SvxLRSpaceItem, pState);
            DBG_ASSERT(pState?  0 != pItem: true, "SvxLRSpaceItem expected");
            rRuler.UpdatePara(pItem);
            break;
        }
        case SID_RULER_BORDERS_VERTICAL:
        case SID_RULER_BORDERS:
        case SID_RULER_ROWS:
        case SID_RULER_ROWS_VERTICAL:
        {
            const SvxColumnItem *pItem = PTR_CAST(SvxColumnItem, pState);
            DBG_ASSERT(pState?  0 != pItem: true, "SvxColumnItem expected");
#ifdef DBG_UTIL
            if(pItem)
            {
                if(pItem->IsConsistent())
                    rRuler.Update(pItem, nSID);
                else
                    OSL_FAIL("Column item corrupted");
            }
            else
                rRuler.Update(pItem, nSID);
#else
            rRuler.Update(pItem, nSID);
#endif
            break;
        }
        case SID_RULER_PAGE_POS:
        {   // Position page, page width
            const SvxPagePosSizeItem *pItem = PTR_CAST(SvxPagePosSizeItem, pState);
            DBG_ASSERT(pState?  0 != pItem: true, "SvxPagePosSizeItem expected");
            rRuler.Update(pItem);
            break;
        }
        case SID_RULER_OBJECT:
        {   // Object selection
            const SvxObjectItem *pItem = PTR_CAST(SvxObjectItem, pState);
            DBG_ASSERT(pState?  0 != pItem: true, "SvxObjectItem expected");
            rRuler.Update(pItem);
            break;
        }
        case SID_RULER_PROTECT:
        {
            const SvxProtectItem *pItem = PTR_CAST(SvxProtectItem, pState);
            DBG_ASSERT(pState?  0 != pItem: true, "SvxProtectItem expected");
            rRuler.Update(pItem);
            break;
        }
        case SID_RULER_BORDER_DISTANCE:
        {
            const SvxLRSpaceItem *pItem = PTR_CAST(SvxLRSpaceItem, pState);
            DBG_ASSERT(pState?  0 != pItem: true, "SvxLRSpaceItem expected");
            rRuler.UpdateParaBorder(pItem);
        }
        break;
        case SID_RULER_TEXT_RIGHT_TO_LEFT :
        {
            const SfxBoolItem *pItem = PTR_CAST(SfxBoolItem, pState);
            DBG_ASSERT(pState?  0 != pItem: true, "SfxBoolItem expected");
            rRuler.UpdateTextRTL(pItem);
        }
        break;
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
