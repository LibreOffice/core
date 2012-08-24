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

#include <string>
#include <vcl/timer.hxx>
#include <sfx2/app.hxx>
#include <svx/htmlmode.hxx>
#include <svl/intitem.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/toolbox.hxx>
#include <sfx2/mnumgr.hxx>


#include "cmdid.h"
#include "docsh.hxx"
#include "swtypes.hxx"
#include "swmodule.hxx"
#include "wrtsh.hxx"
#include "view.hxx"
#include "viewopt.hxx"
#include "ribbar.hrc"
#include "tbxanchr.hxx"



SFX_IMPL_TOOLBOX_CONTROL(SwTbxAnchor, SfxUInt16Item);

SwTbxAnchor::SwTbxAnchor( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx ),
    nActAnchorId(0)
{
    rTbx.SetItemBits( nId, TIB_DROPDOWNONLY | rTbx.GetItemBits( nId ) );
}

 SwTbxAnchor::~SwTbxAnchor()
{
}

void  SwTbxAnchor::StateChanged( sal_uInt16 /*nSID*/, SfxItemState eState, const SfxPoolItem* pState )
{
    GetToolBox().EnableItem( GetId(), (GetItemState(pState) != SFX_ITEM_DISABLED) );

    if( eState == SFX_ITEM_AVAILABLE )
    {
        const SfxUInt16Item* pItem = PTR_CAST( SfxUInt16Item, pState );
        if(pItem)
            nActAnchorId = pItem->GetValue();
    }

}

SfxPopupWindow* SwTbxAnchor::CreatePopupWindow()
{
    SwTbxAnchor::Click();
    return 0;
}

void  SwTbxAnchor::Click()
{
    PopupMenu aPopMenu(SW_RES(MN_ANCHOR_POPUP));

    SfxViewFrame*   pViewFrame( 0 );
    SfxDispatcher*  pDispatch( 0 );
    SfxViewShell*   pCurSh( SfxViewShell::Current() );

    if ( pCurSh )
    {
        pViewFrame = pCurSh->GetViewFrame();
        if ( pViewFrame )
            pDispatch = pViewFrame->GetDispatcher();
    }

    SwView* pActiveView = 0;
    if(pViewFrame)
    {
        const TypeId aTypeId = TYPE(SwView);
        SwView* pView = (SwView*)SfxViewShell::GetFirst(&aTypeId);
        while( pView )
        {
            if(pView->GetViewFrame() == pViewFrame)
            {
                pActiveView = pView;
                break;
            }
            pView = (SwView*)SfxViewShell::GetNext(*pView, &aTypeId);
        }
    }
    if(!pActiveView)
    {
        OSL_FAIL("No active view found");
        return;
    }
    SwWrtShell* pWrtShell = pActiveView->GetWrtShellPtr();
    aPopMenu.EnableItem( FN_TOOL_ANCHOR_FRAME, 0 != pWrtShell->IsFlyInFly() );

    Rectangle aRect(GetToolBox().GetItemRect(GetId()));

    if (pWrtShell->IsInHeaderFooter())
        aPopMenu.RemoveItem(aPopMenu.GetItemPos(FN_TOOL_ANCHOR_PAGE));

    if (nActAnchorId)
        aPopMenu.CheckItem(nActAnchorId);


    sal_uInt16 nSlotId = aPopMenu.Execute(&GetToolBox(), aRect);
    GetToolBox().EndSelection();

    if (nSlotId)
        pDispatch->Execute(nSlotId, SFX_CALLMODE_ASYNCHRON|SFX_CALLMODE_RECORD);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
