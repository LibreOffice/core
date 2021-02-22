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

#include <vcl/commandevent.hxx>
#include <vcl/status.hxx>
#include <vcl/weldutils.hxx>
#include <svl/stritem.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>

#include <tmplctrl.hxx>
#include <ViewShellBase.hxx>
#include <drawdoc.hxx>
#include <sdpage.hxx>
#include <sdattr.hrc>
#include <app.hrc>
#include <sdresid.hxx>
#include <strings.hrc>

SFX_IMPL_STATUSBAR_CONTROL( SdTemplateControl, SfxStringItem );

// class SdTemplateControl ------------------------------------------
SdTemplateControl::SdTemplateControl( sal_uInt16 _nSlotId,
                                      sal_uInt16 _nId,
                                      StatusBar& rStb ) :
    SfxStatusBarControl( _nSlotId, _nId, rStb )
{
    GetStatusBar().SetQuickHelpText(GetId(), SdResId(STR_STATUSBAR_MASTERPAGE));
}

SdTemplateControl::~SdTemplateControl()
{
}

void SdTemplateControl::StateChanged(
    sal_uInt16 /*nSID*/, SfxItemState eState, const SfxPoolItem* pState )
{
    if( eState != SfxItemState::DEFAULT || pState->IsVoidItem() )
        GetStatusBar().SetItemText( GetId(), OUString() );
    else if ( auto pStringItem = dynamic_cast< const SfxStringItem *>( pState ) )
    {
        msTemplate = pStringItem->GetValue();
        GetStatusBar().SetItemText( GetId(), msTemplate );
    }
}

void SdTemplateControl::Paint( const UserDrawEvent&  )
{
}

void SdTemplateControl::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() != CommandEventId::ContextMenu || GetStatusBar().GetItemText( GetId() ).isEmpty() )
        return;

    SfxViewFrame* pViewFrame = SfxViewFrame::Current();

    sd::ViewShellBase* pViewShellBase = sd::ViewShellBase::GetViewShellBase( pViewFrame );
    if( !pViewShellBase )
        return;

    SdDrawDocument* pDoc = pViewShellBase->GetDocument();
    if( !pDoc )
        return;

    std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(nullptr, "modules/simpress/ui/masterpagemenu.ui"));
    std::unique_ptr<weld::Menu> xPopup(xBuilder->weld_menu("menu"));

    const sal_uInt16 nMasterCount = pDoc->GetMasterSdPageCount(PageKind::Standard);

    for (sal_uInt16 nPage = 0; nPage < nMasterCount; ++nPage)
    {
        SdPage* pMaster = pDoc->GetMasterSdPage(nPage, PageKind::Standard);
        if (!pMaster)
            continue;
        xPopup->append(OUString::number(nPage), pMaster->GetName());
    }

    ::tools::Rectangle aRect(rCEvt.GetMousePosPixel(), Size(1, 1));
    weld::Window* pParent = weld::GetPopupParent(GetStatusBar(), aRect);
    OString sResult = xPopup->popup_at_rect(pParent, aRect);
    if (!sResult.isEmpty())
    {
        sal_uInt16 nCurrId = sResult.toUInt32();
        SdPage* pMaster = pDoc->GetMasterSdPage(nCurrId, PageKind::Standard);
        SfxStringItem aStyle( ATTR_PRESLAYOUT_NAME, pMaster->GetName() );
        pViewFrame->GetDispatcher()->ExecuteList(
            SID_PRESENTATION_LAYOUT, SfxCallMode::SLOT, { &aStyle });
        pViewFrame->GetBindings().Invalidate(SID_PRESENTATION_LAYOUT);
        pViewFrame->GetBindings().Invalidate(SID_STATUS_LAYOUT);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
