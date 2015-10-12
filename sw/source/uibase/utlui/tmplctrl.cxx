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

#include <svl/style.hxx>
#include <vcl/menu.hxx>
#include <svl/stritem.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/status.hxx>

#include <swtypes.hxx>
#include <utlui.hrc>

#include "wrtsh.hxx"
#include "view.hxx"
#include "swmodule.hxx"
#include "cmdid.h"
#include "docsh.hxx"
#include "tmplctrl.hxx"

SFX_IMPL_STATUSBAR_CONTROL( SwTemplateControl, SfxStringItem );

class SwTemplatePopup_Impl : public PopupMenu
{
public:
    SwTemplatePopup_Impl();

    sal_uInt16          GetCurId() const { return nCurId; }

private:
    sal_uInt16          nCurId;

    virtual void    Select() override;
};

SwTemplatePopup_Impl::SwTemplatePopup_Impl() :
    PopupMenu(),
    nCurId(USHRT_MAX)
{
}

void SwTemplatePopup_Impl::Select()
{
    nCurId = GetCurItemId();
}

SwTemplateControl::SwTemplateControl( sal_uInt16 _nSlotId,
                                      sal_uInt16 _nId,
                                      StatusBar& rStb ) :
    SfxStatusBarControl( _nSlotId, _nId, rStb )
{
    GetStatusBar().SetQuickHelpText(GetId(), SW_RESSTR(STR_TMPLCTRL_HINT));
}

SwTemplateControl::~SwTemplateControl()
{
}

void SwTemplateControl::StateChanged(
    sal_uInt16 /*nSID*/, SfxItemState eState, const SfxPoolItem* pState )
{
    if( eState != SfxItemState::DEFAULT || dynamic_cast< const SfxVoidItem *>( pState ) !=  nullptr )
        GetStatusBar().SetItemText( GetId(), OUString() );
    else if ( dynamic_cast< const SfxStringItem *>( pState ) !=  nullptr )
    {
        sTemplate = static_cast<const SfxStringItem*>(pState)->GetValue();
        GetStatusBar().SetItemText( GetId(), sTemplate );
    }
}

void SwTemplateControl::Paint( const UserDrawEvent&  )
{
    GetStatusBar().SetItemText( GetId(), sTemplate );
}

void SwTemplateControl::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == CommandEventId::ContextMenu &&
            !GetStatusBar().GetItemText( GetId() ).isEmpty() )
    {
        SwTemplatePopup_Impl aPop;
        {
            SwView* pView = ::GetActiveView();
            SwWrtShell* pWrtShell;
            if( pView && 0 != (pWrtShell = pView->GetWrtShellPtr()) &&
                !pWrtShell->SwCrsrShell::HasSelection()&&
                !pWrtShell->IsSelFrmMode() &&
                !pWrtShell->IsObjSelected())
            {
                SfxStyleSheetBasePool* pPool = pView->GetDocShell()->
                                                            GetStyleSheetPool();
                pPool->SetSearchMask(SFX_STYLE_FAMILY_PAGE);
                if( pPool->Count() > 1 )
                {
                    sal_uInt16 nCount = 0;
                    SfxStyleSheetBase* pStyle = pPool->First();
                    while( pStyle )
                    {
                        aPop.InsertItem( ++nCount, pStyle->GetName() );
                        pStyle = pPool->Next();
                    }

                    aPop.Execute( &GetStatusBar(), rCEvt.GetMousePosPixel());
                    const sal_uInt16 nCurrId = aPop.GetCurId();
                    if( nCurrId != USHRT_MAX)
                    {
                        // looks a bit awkward, but another way is not possible
                        pStyle = pPool->operator[]( nCurrId - 1 );
                        SfxStringItem aStyle( FN_SET_PAGE_STYLE, pStyle->GetName() );
                        pWrtShell->GetView().GetViewFrame()->GetDispatcher()->Execute(
                                    FN_SET_PAGE_STYLE,
                                    SfxCallMode::SLOT|SfxCallMode::RECORD,
                                    &aStyle, 0L );
                    }
                }
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
