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

// class SdTemplatePopup_Impl --------------------------------------------------

class SdTemplatePopup_Impl : public PopupMenu
{
public:
    SdTemplatePopup_Impl();

    sal_uInt16          GetCurId() const { return nCurId; }

private:
    sal_uInt16          nCurId;

    virtual void    Select();
};

// -----------------------------------------------------------------------

SdTemplatePopup_Impl::SdTemplatePopup_Impl() :
    PopupMenu(),
    nCurId(USHRT_MAX)
{
}

// -----------------------------------------------------------------------

void SdTemplatePopup_Impl::Select()
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
        GetStatusBar().SetItemText( GetId(), OUString() );
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
    if ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU && !GetStatusBar().GetItemText( GetId() ).isEmpty() )
    {
        SfxViewFrame* pViewFrame = SfxViewFrame::Current();

        sd::ViewShellBase* pViewShellBase = sd::ViewShellBase::GetViewShellBase( pViewFrame );
        if( !pViewShellBase )
            return;

        SdDrawDocument* pDoc = pViewShellBase->GetDocument();
        if( !pDoc )
            return;

        CaptureMouse();
        SdTemplatePopup_Impl aPop;
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



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
