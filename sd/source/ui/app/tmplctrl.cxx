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
    if( eState != SFX_ITEM_AVAILABLE || pState->ISA( SfxVoidItem ) )
        GetStatusBar().SetItemText( GetId(), String() );
    else if ( pState->ISA( SfxStringItem ) )
    {
        msTemplate = ((SfxStringItem*)pState)->GetValue();
        GetStatusBar().SetItemText( GetId(), msTemplate );
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
            const sal_uInt16 nMasterCount = pDoc->GetMasterSdPageCount(PK_STANDARD);

            sal_uInt16 nCount = 0;
            for( sal_uInt16 nPage = 0; nPage < nMasterCount; ++nPage )
            {
                SdPage* pMaster = pDoc->GetMasterSdPage(nPage, PK_STANDARD);
                if( pMaster )
                    aPop.InsertItem( ++nCount, pMaster->GetName() );
            }
            aPop.Execute( &GetStatusBar(), rCEvt.GetMousePosPixel());

            sal_uInt16 nCurrId = aPop.GetCurId()-1;
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



