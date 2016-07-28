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

#include <svl/intitem.hxx>
#include <vcl/svapp.hxx>

#include <sfx2/templdlg.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/tplpitem.hxx>
#include "tplcitem.hxx"
#include "templdgi.hxx"

#include <sfx2/sfx.hrc>
#include "dialog.hrc"

// Constructor

SfxTemplateControllerItem::SfxTemplateControllerItem(
        sal_uInt16 nSlotId,                 // ID
        SfxCommonTemplateDialog_Impl &rDlg,  // Controller-Instance,
                                             // which is assigned to this item.
        SfxBindings &rBindings):
    SfxControllerItem(nSlotId, rBindings),
    rTemplateDlg(rDlg),
    nWaterCanState(0xff),
    nUserEventId(nullptr)
{
}

SfxTemplateControllerItem::~SfxTemplateControllerItem()
{
    if(nUserEventId)
        Application::RemoveUserEvent(nUserEventId);
}


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
        case SID_STYLE_FAMILY6:
        {
            bool bAvailable = SfxItemState::DEFAULT == eState;
            if ( !bAvailable )
                rTemplateDlg.SetFamilyState(GetId(), nullptr);
            else {
                const SfxTemplateItem *pStateItem = dynamic_cast< const SfxTemplateItem* >(pItem);
                DBG_ASSERT(pStateItem != nullptr, "SfxTemplateItem expected");
                rTemplateDlg.SetFamilyState( GetId(), pStateItem );
            }
            bool bDisable = eState == SfxItemState::DISABLED;
            // Disable Family
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
                case SID_STYLE_FAMILY6:
                    nFamily = 6; break;

                default: OSL_FAIL("unknown StyleFamily"); break;
            }
            rTemplateDlg.EnableFamilyItem( nFamily, !bDisable );
            break;
        }
        case SID_STYLE_WATERCAN:
        {
            if ( eState == SfxItemState::DISABLED )
                nWaterCanState = 0xff;
            else if( eState == SfxItemState::DEFAULT )
            {
                const SfxBoolItem& rStateItem = dynamic_cast<const SfxBoolItem&>(*pItem);
                nWaterCanState = rStateItem.GetValue() ? 1 : 0;
            }
            //not necessary if the last event is still on the way
            if(!nUserEventId)
                nUserEventId = Application::PostUserEvent( LINK(
                            this, SfxTemplateControllerItem, SetWaterCanStateHdl_Impl ) );
            break;
        }
        case SID_STYLE_EDIT:
            rTemplateDlg.EnableEdit( SfxItemState::DISABLED != eState );
            break;
        case SID_STYLE_DELETE:
            rTemplateDlg.EnableDel( SfxItemState::DISABLED != eState );
            break;
        case SID_STYLE_HIDE:
            rTemplateDlg.EnableHide( SfxItemState::DISABLED != eState );
            break;
        case SID_STYLE_SHOW:
            rTemplateDlg.EnableShow( SfxItemState::DISABLED != eState );
            break;
        case SID_STYLE_NEW_BY_EXAMPLE:

            rTemplateDlg.EnableExample_Impl(
                GetId(), SfxItemState::DISABLED != eState );
            break;
        case SID_STYLE_UPDATE_BY_EXAMPLE:
        {
            rTemplateDlg.EnableExample_Impl(
                GetId(), eState != SfxItemState::DISABLED );
            break;
        }
        case SID_STYLE_NEW:
        {
            rTemplateDlg.EnableNew( SfxItemState::DISABLED != eState );
            break;
        }
        case SID_STYLE_DRAGHIERARCHIE:
        {
            rTemplateDlg.EnableTreeDrag( SfxItemState::DISABLED != eState );
            break;
        }
        case SID_STYLE_FAMILY :
        {
            const SfxUInt16Item *pStateItem = dynamic_cast< const SfxUInt16Item* >(pItem);
            if (pStateItem)
                rTemplateDlg.SetFamily( pStateItem->GetValue() );
            break;
        }
    }
}

IMPL_LINK_NOARG_TYPED(SfxTemplateControllerItem, SetWaterCanStateHdl_Impl, void*, void)
{
    nUserEventId = nullptr;
    SfxBoolItem* pState = nullptr;
    switch(nWaterCanState)
    {
        case 0 :
        case 1 :
            pState = new SfxBoolItem(SID_STYLE_WATERCAN, nWaterCanState != 0);
        break;
    }
    rTemplateDlg.SetWaterCanState(pState);
    delete pState;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
