/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

// INCLUDE ---------------------------------------------------------------

#include <svl/intitem.hxx>
#include <vcl/svapp.hxx>

#include <sfx2/templdlg.hxx>
#include <sfx2/bindings.hxx>
#include "sfx2/tplpitem.hxx"
#include "tplcitem.hxx"
#include "templdgi.hxx"

#include <sfx2/sfx.hrc>
#include "dialog.hrc"

// STATIC DATA -----------------------------------------------------------

// Constructor

SfxTemplateControllerItem::SfxTemplateControllerItem(
        sal_uInt16 nSlotId,                 // ID
        SfxCommonTemplateDialog_Impl &rDlg,  // Controller-Instance,
                                             // which is assigned to this item.
        SfxBindings &rBindings):
    SfxControllerItem(nSlotId, rBindings),
    rTemplateDlg(rDlg),
    nWaterCanState(0xff),
    nUserEventId(0)
{
}
// -----------------------------------------------------------------------
SfxTemplateControllerItem::~SfxTemplateControllerItem()
{
    if(nUserEventId)
        Application::RemoveUserEvent(nUserEventId);
}

// -----------------------------------------------------------------------
// Notice about change of status, is  propagated through the Controller
// passed on by the constructor

void SfxTemplateControllerItem::StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                              const SfxPoolItem* pItem )
{
    switch(nSID)
    {
        case SID_STYLE_FAMILY1:
        case SID_STYLE_FAMILY2:
        case SID_STYLE_FAMILY3:
        case SID_STYLE_FAMILY4:
        case SID_STYLE_FAMILY5:
        {
            bool bAvailable = SFX_ITEM_AVAILABLE == eState;
            if ( !bAvailable )
                rTemplateDlg.SetFamilyState(GetId(), 0);
            else {
                const SfxTemplateItem *pStateItem = PTR_CAST(
                    SfxTemplateItem, pItem);
                DBG_ASSERT(pStateItem != 0, "SfxTemplateItem expected");
                rTemplateDlg.SetFamilyState( GetId(), pStateItem );
            }
            sal_Bool bDisable = eState == SFX_ITEM_DISABLED;
            // Disable Familly
            sal_uInt16 nFamily = 0;
            switch( GetId())
            {
                case SID_STYLE_FAMILY1:
                    nFamily = 1; break;
                case SID_STYLE_FAMILY2:
                    nFamily = 2; break;
                case SID_STYLE_FAMILY3:
                    nFamily = 3; break;
                case SID_STYLE_FAMILY4:
                    nFamily = 4; break;
                case SID_STYLE_FAMILY5:
                    nFamily = 5; break;

                default: OSL_FAIL("unknown StyleFamily"); break;
            }
            rTemplateDlg.EnableFamilyItem( nFamily, !bDisable );
            break;
        }
        case SID_STYLE_WATERCAN:
        {
            if ( eState == SFX_ITEM_DISABLED )
                nWaterCanState = 0xff;
            else if( eState == SFX_ITEM_AVAILABLE )
            {
                const SfxBoolItem *pStateItem = PTR_CAST(SfxBoolItem, pItem);
                DBG_ASSERT(pStateItem != 0, "BoolItem expected");
                nWaterCanState = pStateItem->GetValue() ? 1 : 0;
            }
            //not necessary if the last event is still on the way
            if(!nUserEventId)
                nUserEventId = Application::PostUserEvent( STATIC_LINK(
                            this, SfxTemplateControllerItem, SetWaterCanStateHdl_Impl ) );
            break;
        }
        case SID_STYLE_EDIT:
            rTemplateDlg.EnableEdit( SFX_ITEM_DISABLED != eState );
            break;
        case SID_STYLE_DELETE:
            rTemplateDlg.EnableDel( SFX_ITEM_DISABLED != eState );
            break;
        case SID_STYLE_NEW_BY_EXAMPLE:

            rTemplateDlg.EnableExample_Impl(
                GetId(), SFX_ITEM_DISABLED != eState );
            break;
        case SID_STYLE_UPDATE_BY_EXAMPLE:
        {
            rTemplateDlg.EnableExample_Impl(
                GetId(), eState != SFX_ITEM_DISABLED );
            break;
        }
        case SID_STYLE_NEW:
        {
            rTemplateDlg.EnableNew( SFX_ITEM_DISABLED != eState );
            break;
        }
        case SID_STYLE_DRAGHIERARCHIE:
        {
            rTemplateDlg.EnableTreeDrag( SFX_ITEM_DISABLED != eState );
            break;
        }
        case SID_STYLE_FAMILY :
        {
            const SfxUInt16Item *pStateItem = PTR_CAST( SfxUInt16Item, pItem);
            if (pStateItem)
                rTemplateDlg.SetFamily( pStateItem->GetValue() );
            break;
        }
    }
}

IMPL_STATIC_LINK(SfxTemplateControllerItem, SetWaterCanStateHdl_Impl,
                                    SfxTemplateControllerItem*, EMPTYARG)
{
    pThis->nUserEventId = 0;
    SfxBoolItem* pState = 0;
    switch(pThis->nWaterCanState)
    {
        case 0 :
        case 1 :
            pState = new SfxBoolItem(SID_STYLE_WATERCAN, pThis->nWaterCanState ? sal_True : sal_False);
        break;
    }
    pThis->rTemplateDlg.SetWaterCanState(pState);
    delete pState;
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
