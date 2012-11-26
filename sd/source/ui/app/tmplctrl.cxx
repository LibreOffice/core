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
#include "precompiled_sd.hxx"

// include ---------------------------------------------------------------


#include <vcl/menu.hxx>
#include <vcl/status.hxx>
#include <svl/style.hxx>
#include <svl/stritem.hxx>
#include <sfx2/dispatch.hxx>

#include "tmplctrl.hxx"
#include "ViewShellBase.hxx"
#include "drawdoc.hxx"
#include "sdattr.hrc"
#include "app.hrc"

SFX_IMPL_STATUSBAR_CONTROL( SdTemplateControl, SfxStringItem );

// class TemplatePopup_Impl --------------------------------------------------

class TemplatePopup_Impl : public PopupMenu
{
public:
    TemplatePopup_Impl();

    sal_uInt16          GetCurId() const { return nCurId; }

private:
    sal_uInt16          nCurId;

    virtual void    Select();
};

// -----------------------------------------------------------------------

TemplatePopup_Impl::TemplatePopup_Impl() :
    PopupMenu(),
    nCurId(USHRT_MAX)
{
}

// -----------------------------------------------------------------------

void TemplatePopup_Impl::Select()
{
    nCurId = GetCurItemId();
}

// class SdTemplateControl ------------------------------------------

SdTemplateControl::SdTemplateControl( sal_uInt16 _nSlotId,
                                      sal_uInt16 _nId,
                                      StatusBar& rStb ) :
    SfxStatusBarControl( _nSlotId, _nId, rStb )
{
}

// -----------------------------------------------------------------------

SdTemplateControl::~SdTemplateControl()
{
}

// -----------------------------------------------------------------------

void SdTemplateControl::StateChanged(
    sal_uInt16 /*nSID*/, SfxItemState eState, const SfxPoolItem* pState )
{
    if( eState != SFX_ITEM_AVAILABLE || dynamic_cast< const SfxVoidItem* >(pState) )
    {
        GetStatusBar().SetItemText( GetId(), String() );
    }
    else
    {
        const SfxStringItem* pSfxStringItem = dynamic_cast< const SfxStringItem* >(pState);

        if ( pSfxStringItem )
        {
            msTemplate = pSfxStringItem->GetValue();
            GetStatusBar().SetItemText( GetId(), msTemplate );
        }
    }
}

// -----------------------------------------------------------------------

void SdTemplateControl::Paint( const UserDrawEvent&  )
{
    GetStatusBar().SetItemText( GetId(), msTemplate );
}

// -----------------------------------------------------------------------

void SdTemplateControl::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU && GetStatusBar().GetItemText( GetId() ).Len() )
    {
        SfxViewFrame* pViewFrame = SfxViewFrame::Current();

        sd::ViewShellBase* pViewShellBase = sd::ViewShellBase::GetViewShellBase( pViewFrame );
        if( !pViewShellBase )
            return;

        SdDrawDocument* pDoc = pViewShellBase->GetDocument();
        if( !pDoc )
            return;

        CaptureMouse();
        TemplatePopup_Impl aPop;
        {
            const sal_uInt32 nMasterCount = pDoc->GetMasterSdPageCount(PK_STANDARD);

            sal_uInt32 nCount = 0;
            for( sal_uInt32 nPage = 0; nPage < nMasterCount; ++nPage )
            {
                SdPage* pMaster = pDoc->GetMasterSdPage(nPage, PK_STANDARD);
                if( pMaster )
                    aPop.InsertItem( ++nCount, pMaster->GetName() );
            }
            aPop.Execute( &GetStatusBar(), rCEvt.GetMousePosPixel());

            sal_uInt32 nCurrId = aPop.GetCurId()-1;
            if( nCurrId < nMasterCount )
            {
                SdPage* pMaster = pDoc->GetMasterSdPage(nCurrId, PK_STANDARD);
                SfxStringItem aStyle( ATTR_PRESLAYOUT_NAME, pMaster->GetName() );
                pViewFrame->GetDispatcher()->Execute(SID_PRESENTATION_LAYOUT,SFX_CALLMODE_SLOT, &aStyle, 0L );
                pViewFrame->GetBindings().Invalidate(SID_PRESENTATION_LAYOUT);
                pViewFrame->GetBindings().Invalidate(SID_STATUS_LAYOUT);
            }
        }

        ReleaseMouse();
    }
}



