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
#include "precompiled_sc.hxx"

#include <sfx2/app.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/request.hxx>
#include <avmedia/mediaitem.hxx>
#include <svl/whiter.hxx>
#include <svx/svdomedia.hxx>
#include <svx/sdr/contact/viewcontactofsdrmediaobj.hxx>
#include <sfx2/sidebar/EnumContext.hxx>

#include "mediash.hxx"
#include "sc.hrc"
#include "viewdata.hxx"
#include "drawview.hxx"
#include "scresid.hxx"

#define ScMediaShell
#include "scslots.hxx"

#define ITEMVALUE(ItemSet,Id,Cast) ((const Cast&)(ItemSet).Get(Id)).GetValue()


SFX_IMPL_INTERFACE(ScMediaShell, ScDrawShell, ScResId(SCSTR_GRAPHICSHELL) )
{
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_OBJECT, ScResId(RID_MEDIA_OBJECTBAR) );
    SFX_POPUPMENU_REGISTRATION( ScResId(RID_POPUP_MEDIA) );
}

TYPEINIT1( ScMediaShell, ScDrawShell );

ScMediaShell::ScMediaShell(ScViewData* pData) :
    ScDrawShell(pData)
{
    SetHelpId(HID_SCSHELL_MEDIA);
    SetName( String( ScResId( SCSTR_MEDIASHELL ) ) );
    SfxShell::SetContextName(sfx2::sidebar::EnumContext::GetContextName(sfx2::sidebar::EnumContext::Context_Media));
}

ScMediaShell::~ScMediaShell()
{
}

void ScMediaShell::GetMediaState( SfxItemSet& rSet )
{
    ScDrawView* pView = GetViewData()->GetScDrawView();

    if( pView )
    {
        SfxWhichIter    aIter( rSet );
        sal_uInt16          nWhich = aIter.FirstWhich();

        while( nWhich )
        {
            if( SID_AVMEDIA_TOOLBOX == nWhich )
            {
                SdrMarkList*    pMarkList = new SdrMarkList( pView->GetMarkedObjectList() );
                bool            bDisable = true;

                if( 1 == pMarkList->GetMarkCount() )
                {
                    SdrObject* pObj = pMarkList->GetMark( 0 )->GetMarkedSdrObj();

                    if( pObj && pObj->ISA( SdrMediaObj ) )
                    {
                        ::avmedia::MediaItem aItem( SID_AVMEDIA_TOOLBOX );

                        static_cast< sdr::contact::ViewContactOfSdrMediaObj& >( pObj->GetViewContact() ).updateMediaItem( aItem );
                        rSet.Put( aItem );
                        bDisable = false;
                    }
                }

                if( bDisable )
                    rSet.DisableItem( SID_AVMEDIA_TOOLBOX );

                delete pMarkList;
            }

            nWhich = aIter.NextWhich();
        }
    }
}

void ScMediaShell::ExecuteMedia( SfxRequest& rReq )
{
    ScDrawView* pView = GetViewData()->GetScDrawView();

    if( pView && SID_AVMEDIA_TOOLBOX == rReq.GetSlot() )
    {
        const SfxItemSet*   pArgs = rReq.GetArgs();
        const SfxPoolItem*  pItem;

        if( !pArgs || ( SFX_ITEM_SET != pArgs->GetItemState( SID_AVMEDIA_TOOLBOX, sal_False, &pItem ) ) )
            pItem = NULL;

        if( pItem )
        {
            SdrMarkList* pMarkList = new SdrMarkList( pView->GetMarkedObjectList() );

            if( 1 == pMarkList->GetMarkCount() )
            {
                SdrObject* pObj = pMarkList->GetMark( 0 )->GetMarkedSdrObj();

                if( pObj && pObj->ISA( SdrMediaObj ) )
                {
                    static_cast< sdr::contact::ViewContactOfSdrMediaObj& >( pObj->GetViewContact() ).executeMediaItem(
                        static_cast< const ::avmedia::MediaItem& >( *pItem ) );
                }

                delete pMarkList;
            }
        }
    }

    Invalidate();
}
