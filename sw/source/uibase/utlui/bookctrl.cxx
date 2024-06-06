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

#include <svl/intitem.hxx>
#include <svl/slstitm.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <vcl/status.hxx>
#include <vcl/weldutils.hxx>
#include <cmdid.h>
#include <swmodule.hxx>
#include <wrtsh.hxx>
#include <IMark.hxx>
#include <bookctrl.hxx>
#include <map>

SFX_IMPL_STATUSBAR_CONTROL(SwBookmarkControl, SfxStringListItem);

SwBookmarkControl::SwBookmarkControl( sal_uInt16 _nSlotId,
                                      sal_uInt16 _nId,
                                      StatusBar& rStb ) :
    SfxStatusBarControl( _nSlotId, _nId, rStb )
{
}

SwBookmarkControl::~SwBookmarkControl()
{
}

void SwBookmarkControl::StateChangedAtStatusBarControl(
    sal_uInt16 /*nSID*/, SfxItemState eState, const SfxPoolItem* pState )
{
    if (eState != SfxItemState::DEFAULT || SfxItemState::DISABLED == eState)
    {
        GetStatusBar().SetItemText(GetId(), OUString());
        GetStatusBar().SetQuickHelpText(GetId(), OUString());
    }
    else if (auto pStringListItem = dynamic_cast<const SfxStringListItem*>(pState))
    {
        const std::vector<OUString>& rStringList(pStringListItem->GetList());
        GetStatusBar().SetItemText(GetId(), rStringList[0]);
        GetStatusBar().SetQuickHelpText(GetId(), rStringList[1]);
    }
}

void SwBookmarkControl::Paint( const UserDrawEvent&  )
{
}

void SwBookmarkControl::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() != CommandEventId::ContextMenu ||
            GetStatusBar().GetItemText( GetId() ).isEmpty())
        return;

    SwWrtShell* pWrtShell = ::GetActiveWrtShell();
    if( !(pWrtShell && pWrtShell->getIDocumentMarkAccess()->getAllMarksCount() > 0) )
        return;

    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    if (!pViewFrm)
        return;

    std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(nullptr, u"modules/swriter/ui/bookmarkmenu.ui"_ustr));
    std::unique_ptr<weld::Menu> xPopup(xBuilder->weld_menu(u"menu"_ustr));

    IDocumentMarkAccess* const pMarkAccess = pWrtShell->getIDocumentMarkAccess();
    IDocumentMarkAccess::const_iterator_t ppBookmarkStart = pMarkAccess->getBookmarksBegin();
    sal_uInt32 nPopupId = 1;
    std::map<sal_Int32, sal_uInt16> aBookmarkIdx;
    for(IDocumentMarkAccess::const_iterator_t ppBookmark = ppBookmarkStart;
        ppBookmark != pMarkAccess->getBookmarksEnd();
        ++ppBookmark)
    {
        if(IDocumentMarkAccess::MarkType::BOOKMARK == IDocumentMarkAccess::GetType(**ppBookmark))
        {
            xPopup->append(OUString::number(nPopupId), (*ppBookmark)->GetName());
            aBookmarkIdx[nPopupId] = o3tl::narrowing<sal_uInt16>(ppBookmark - ppBookmarkStart);
            nPopupId++;
        }
    }
    ::tools::Rectangle aRect(rCEvt.GetMousePosPixel(), Size(1, 1));
    weld::Window* pParent = weld::GetPopupParent(GetStatusBar(), aRect);
    OUString sResult = xPopup->popup_at_rect(pParent, aRect);
    if (!sResult.isEmpty())
    {
        SfxUInt16Item aBookmark( FN_STAT_BOOKMARK, aBookmarkIdx[sResult.toUInt32()] );
        pViewFrm->GetDispatcher()->ExecuteList(FN_STAT_BOOKMARK,
            SfxCallMode::ASYNCHRON|SfxCallMode::RECORD,
            { &aBookmark });
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
