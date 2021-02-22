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
#include <svl/stritem.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/status.hxx>
#include <vcl/weldutils.hxx>

#include <swtypes.hxx>
#include <strings.hrc>

#include <wrtsh.hxx>
#include <view.hxx>
#include <swmodule.hxx>
#include <cmdid.h>
#include <docsh.hxx>
#include <tmplctrl.hxx>

SFX_IMPL_STATUSBAR_CONTROL( SwTemplateControl, SfxStringItem );

SwTemplateControl::SwTemplateControl( sal_uInt16 _nSlotId,
                                      sal_uInt16 _nId,
                                      StatusBar& rStb ) :
    SfxStatusBarControl( _nSlotId, _nId, rStb )
{
    GetStatusBar().SetQuickHelpText(GetId(), SwResId(STR_TMPLCTRL_HINT));
}

SwTemplateControl::~SwTemplateControl()
{
}

void SwTemplateControl::StateChanged(
    sal_uInt16 /*nSID*/, SfxItemState eState, const SfxPoolItem* pState )
{
    const SfxStringItem* pItem = nullptr;
    if (SfxItemState::DEFAULT == eState && (pItem = dynamic_cast<const SfxStringItem*>(pState)))
    {
        sTemplate = pItem->GetValue();
        GetStatusBar().SetItemText(GetId(), sTemplate);
    }
    else
        GetStatusBar().SetItemText(GetId(), OUString());
}

void SwTemplateControl::Paint( const UserDrawEvent&  )
{
}

void SwTemplateControl::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() != CommandEventId::ContextMenu ||
            GetStatusBar().GetItemText( GetId() ).isEmpty())
        return;

    {
        SwView* pView = ::GetActiveView();
        SwWrtShell *const pWrtShell(pView ? pView->GetWrtShellPtr() : nullptr);
        if (nullptr != pWrtShell &&
            !pWrtShell->SwCursorShell::HasSelection()&&
            !pWrtShell->IsSelFrameMode() &&
            !pWrtShell->IsObjSelected())
        {
            SfxStyleSheetBasePool* pPool = pView->GetDocShell()->
                                                        GetStyleSheetPool();
            auto xIter = pPool->CreateIterator(SfxStyleFamily::Page);
            if (xIter->Count() > 1)
            {
                std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(nullptr, "modules/swriter/ui/pagestylemenu.ui"));
                std::unique_ptr<weld::Menu> xPopup(xBuilder->weld_menu("menu"));

                sal_uInt32 nCount = 0;
                SfxStyleSheetBase* pStyle = xIter->First();
                while( pStyle )
                {
                    xPopup->append(OUString::number(++nCount), pStyle->GetName());
                    pStyle = xIter->Next();
                }

                ::tools::Rectangle aRect(rCEvt.GetMousePosPixel(), Size(1, 1));
                weld::Window* pParent = weld::GetPopupParent(GetStatusBar(), aRect);
                OString sResult = xPopup->popup_at_rect(pParent, aRect);
                if (!sResult.isEmpty())
                {
                    sal_uInt32 nCurrId = sResult.toUInt32();
                    // looks a bit awkward, but another way is not possible
                    pStyle = xIter->operator[]( nCurrId - 1 );
                    SfxStringItem aStyle( FN_SET_PAGE_STYLE, pStyle->GetName() );
                    pWrtShell->GetView().GetViewFrame()->GetDispatcher()->ExecuteList(
                                FN_SET_PAGE_STYLE,
                                SfxCallMode::SLOT|SfxCallMode::RECORD,
                                { &aStyle });
                }
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
