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

SvxRulerItem::SvxRulerItem(sal_uInt16 _nId, SvxRuler &rRul, SfxBindings &rBindings)
: SfxControllerItem(_nId, rBindings),
  rRuler(rRul)
{
}



void SvxRulerItem::StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                 const SfxPoolItem* pState)
{
    // SfxItemState::DONTCARE => pState == -1 => PTR_CAST buff
    if ( eState != SfxItemState::DEFAULT )
        pState = 0;

    switch(nSID)
    {
        // Left / right margin
        case SID_RULER_LR_MIN_MAX:
        {
            const SfxRectangleItem *pItem = dynamic_cast<const SfxRectangleItem*>( pState );
            rRuler.UpdateFrameMinMax(pItem);
            break;
        }
        case SID_ATTR_LONG_LRSPACE:
        {
            const SvxLongLRSpaceItem *pItem = dynamic_cast<const SvxLongLRSpaceItem*>( pState );
            DBG_ASSERT(pState == nullptr || pItem != nullptr, "SvxLRSpaceItem expected");
            rRuler.UpdateFrame(pItem);
            break;
        }
        case SID_ATTR_LONG_ULSPACE:
        {
            const SvxLongULSpaceItem *pItem = dynamic_cast<const SvxLongULSpaceItem*>( pState );
            DBG_ASSERT(pState == nullptr || pItem != nullptr, "SvxULSpaceItem expected");
            rRuler.UpdateFrame(pItem);
            break;
        }
        case SID_ATTR_TABSTOP_VERTICAL:
        case SID_ATTR_TABSTOP:
        {
            const SvxTabStopItem *pItem = dynamic_cast<const SvxTabStopItem*>( pState );
            DBG_ASSERT(pState == nullptr || pItem != nullptr, "SvxTabStopItem expected");
            rRuler.Update(pItem);
            break;
        }
        case SID_ATTR_PARA_LRSPACE_VERTICAL:
        case SID_ATTR_PARA_LRSPACE:
        {
            const SvxLRSpaceItem *pItem = dynamic_cast<const SvxLRSpaceItem*>( pState );
            DBG_ASSERT(pState == nullptr || pItem != nullptr, "SvxLRSpaceItem expected");
            rRuler.UpdatePara(pItem);
            break;
        }
        case SID_RULER_BORDERS_VERTICAL:
        case SID_RULER_BORDERS:
        case SID_RULER_ROWS:
        case SID_RULER_ROWS_VERTICAL:
        {
            const SvxColumnItem *pItem = dynamic_cast<const SvxColumnItem*>( pState );
            DBG_ASSERT(pState == nullptr || pItem != nullptr, "SvxColumnItem expected");
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
            const SvxPagePosSizeItem *pItem = dynamic_cast<const SvxPagePosSizeItem*>( pState );
            DBG_ASSERT(pState == nullptr || pItem != nullptr, "SvxPagePosSizeItem expected");
            rRuler.Update(pItem);
            break;
        }
        case SID_RULER_OBJECT:
        {   // Object selection
            const SvxObjectItem *pItem = dynamic_cast<const SvxObjectItem*>( pState );
            DBG_ASSERT(pState == nullptr || pItem != nullptr, "SvxObjectItem expected");
            rRuler.Update(pItem);
            break;
        }
        case SID_RULER_PROTECT:
        {
            const SvxProtectItem *pItem = dynamic_cast<const SvxProtectItem*>( pState );
            DBG_ASSERT(pState == nullptr || pItem != nullptr, "SvxProtectItem expected");
            rRuler.Update(pItem);
            break;
        }
        case SID_RULER_BORDER_DISTANCE:
        {
            const SvxLRSpaceItem *pItem = dynamic_cast<const SvxLRSpaceItem*>( pState );
            DBG_ASSERT(pState == nullptr || pItem != nullptr, "SvxLRSpaceItem expected");
            rRuler.UpdateParaBorder(pItem);
        }
        break;
        case SID_RULER_TEXT_RIGHT_TO_LEFT :
        {
            const SfxBoolItem *pItem = dynamic_cast<const SfxBoolItem*>( pState );
            DBG_ASSERT(pState == nullptr || pItem != nullptr, "SfxBoolItem expected");
            rRuler.UpdateTextRTL(pItem);
        }
        break;
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
