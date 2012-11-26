/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



#include <string> // HACK: prevent conflict between STLPORT and Workshop headers
#include <vcl/timer.hxx>
#include <sfx2/app.hxx>
#include <svx/htmlmode.hxx>
#include <svl/intitem.hxx>
#include <sfx2/dispatch.hxx>
#ifndef _TOOLBOX_HXX //autogen
#include <vcl/toolbox.hxx>
#endif
#include <sfx2/mnumgr.hxx>


#include "cmdid.h"
#include "docsh.hxx"
#include "swtypes.hxx"
#include "swmodule.hxx"
#include "wrtsh.hxx"
#include "view.hxx"
#include "viewopt.hxx"
#include "errhdl.hxx"
#include "ribbar.hrc"
#include "tbxanchr.hxx"



SFX_IMPL_TOOLBOX_CONTROL(SwTbxAnchor, SfxUInt16Item);

/******************************************************************************
 *  Beschreibung:
 ******************************************************************************/

SwTbxAnchor::SwTbxAnchor( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx ),
    nActAnchorId(0)
{
    rTbx.SetItemBits( nId, TIB_DROPDOWNONLY | rTbx.GetItemBits( nId ) );
}

/******************************************************************************
 *  Beschreibung:
 ******************************************************************************/

 SwTbxAnchor::~SwTbxAnchor()
{
}

/******************************************************************************
 *  Beschreibung:
 ******************************************************************************/

void  SwTbxAnchor::StateChanged( sal_uInt16 /*nSID*/, SfxItemState eState, const SfxPoolItem* pState )
{
    GetToolBox().EnableItem( GetId(), (GetItemState(pState) != SFX_ITEM_DISABLED) );

    if( eState == SFX_ITEM_AVAILABLE )
    {
        const SfxUInt16Item* pItem = dynamic_cast< const SfxUInt16Item* >( pState );
        if(pItem)
            nActAnchorId = pItem->GetValue();
    }

}

/******************************************************************************
 *  Beschreibung:
 ******************************************************************************/

SfxPopupWindow* SwTbxAnchor::CreatePopupWindow()
{
    SwTbxAnchor::Click();
    return 0;
}

/******************************************************************************
 *  Beschreibung:
 ******************************************************************************/

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

//    SfxDispatcher* pDispatch = GetBindings().GetDispatcher();
//    SfxViewFrame* pViewFrame = pDispatch ? pDispatch->GetFrame() : 0;
    SwView* pActiveView = 0;
    if(pViewFrame)
    {
        SwView* pView = (SwView*)SfxViewShell::GetFirst( _IsSfxViewShell< SwView > );
        while( pView )
        {
            if(pView->GetViewFrame() == pViewFrame)
            {
                pActiveView = pView;
                break;
            }
            pView = (SwView*)SfxViewShell::GetNext(*pView, _IsSfxViewShell< SwView > );
        }
    }
    if(!pActiveView)
    {
        DBG_ERROR("No active view could be found");
        return;
    }
    SwWrtShell* pWrtShell = pActiveView->GetWrtShellPtr();
    aPopMenu.EnableItem( FN_TOOL_ANKER_FRAME, 0 != pWrtShell->IsFlyInFly() );

    Rectangle aRect(GetToolBox().GetItemRect(GetId()));
    sal_uInt16 nHtmlMode = ::GetHtmlMode((SwDocShell*)SfxObjectShell::Current());
    sal_Bool bHtmlModeNoAnchor = ( nHtmlMode & HTMLMODE_ON) && 0 == (nHtmlMode & HTMLMODE_SOME_ABS_POS);

    if (bHtmlModeNoAnchor || pWrtShell->IsInHeaderFooter())
        aPopMenu.RemoveItem(aPopMenu.GetItemPos(FN_TOOL_ANKER_PAGE));

    if (nActAnchorId)
        aPopMenu.CheckItem(nActAnchorId);


    sal_uInt16 nSlotId = aPopMenu.Execute(&GetToolBox(), aRect);
    GetToolBox().EndSelection();

    if (nSlotId)
        pDispatch->Execute(nSlotId, SFX_CALLMODE_ASYNCHRON|SFX_CALLMODE_RECORD);
}
