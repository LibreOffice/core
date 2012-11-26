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

// Konstruktor

SfxTemplateControllerItem::SfxTemplateControllerItem(
        sal_uInt16 nSlotId,                 // ID
        SfxCommonTemplateDialog_Impl &rDlg, // Controller-Instanz, dem dieses Item zugeordnet ist.
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

// Benachrichtigung "uber Status"anderung; wird an den
// im Konstruktor "ubergebenen Controller propagiert

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
                const SfxTemplateItem *pStateItem = dynamic_cast< const SfxTemplateItem* >( pItem);
                DBG_ASSERT(pStateItem != 0, "SfxTemplateItem erwartet");
                rTemplateDlg.SetFamilyState( GetId(), pStateItem );
            }
            sal_Bool bDisable = eState == SFX_ITEM_DISABLED;
            // Familie Disablen
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
                default: DBG_ERROR("unbekannte StyleFamily"); break;
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
                const SfxBoolItem *pStateItem = dynamic_cast< const SfxBoolItem* >( pItem);
                DBG_ASSERT(pStateItem != 0, "BoolItem erwartet");
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
            // Das Select Disabled dann, falls enabled und Style Readonly
/*          String aStr = rTemplateDlg.GetSelectedEntry();
            if( aStr.Len() ) rTemplateDlg.SelectStyle( aStr ); */
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
            const SfxUInt16Item *pStateItem = dynamic_cast< const SfxUInt16Item* >( pItem);
            if (pStateItem)
                rTemplateDlg.SetFamily( pStateItem->GetValue() );
            break;
        }
    }
}
/* -----------------------------05.09.2001 10:48------------------------------

 ---------------------------------------------------------------------------*/
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

