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

#include <swtypes.hxx>
#include <strings.hrc>

#include <svl/intitem.hxx>
#include <svl/slstitm.hxx>
#include <svl/eitem.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <vcl/status.hxx>
#include <vcl/menu.hxx>
#include <cmdid.h>
#include <swmodule.hxx>
#include <wrtsh.hxx>
#include <IMark.hxx>
#include <bookctrl.hxx>
#include <map>

SFX_IMPL_STATUSBAR_CONTROL(SwBookmarkControl, SfxStringListItem);

namespace {

class BookmarkPopup_Impl : public PopupMenu
{
public:
    BookmarkPopup_Impl();

    sal_uInt16          GetCurId() const { return nCurId; }

private:
    sal_uInt16          nCurId;

    virtual void    Select() override;
};

}

BookmarkPopup_Impl::BookmarkPopup_Impl() :
    PopupMenu(),
    nCurId(USHRT_MAX)
{
}

void BookmarkPopup_Impl::Select()
{
    nCurId = GetCurItemId();
}

SwBookmarkControl::SwBookmarkControl( sal_uInt16 _nSlotId,
                                      sal_uInt16 _nId,
                                      StatusBar& rStb ) :
    SfxStatusBarControl( _nSlotId, _nId, rStb )
{
}

SwBookmarkControl::~SwBookmarkControl()
{
}

void SwBookmarkControl::StateChanged(
    sal_uInt16 /*nSID*/, SfxItemState eState, const SfxPoolItem* pState )
{
    if( eState != SfxItemState::DEFAULT || pState->IsVoidItem() )
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

    ScopedVclPtrInstance<BookmarkPopup_Impl> aPop;
    SwWrtShell* pWrtShell = ::GetActiveWrtShell();
    if( !(pWrtShell && pWrtShell->getIDocumentMarkAccess()->getAllMarksCount() > 0) )
        return;

    IDocumentMarkAccess* const pMarkAccess = pWrtShell->getIDocumentMarkAccess();
    IDocumentMarkAccess::const_iterator_t ppBookmarkStart = pMarkAccess->getBookmarksBegin();
    sal_uInt16 nPopupId = 1;
    std::map<sal_Int32, sal_uInt16> aBookmarkIdx;
    for(IDocumentMarkAccess::const_iterator_t ppBookmark = ppBookmarkStart;
        ppBookmark != pMarkAccess->getBookmarksEnd();
        ++ppBookmark)
    {
        if(IDocumentMarkAccess::MarkType::BOOKMARK == IDocumentMarkAccess::GetType(**ppBookmark))
        {
            aPop->InsertItem( nPopupId, (*ppBookmark)->GetName() );
            aBookmarkIdx[nPopupId] = static_cast<sal_uInt16>(ppBookmark - ppBookmarkStart);
            nPopupId++;
        }
    }
    aPop->Execute( &GetStatusBar(), rCEvt.GetMousePosPixel());
    sal_uInt16 nCurrId = aPop->GetCurId();
    if( nCurrId != USHRT_MAX)
    {
        SfxUInt16Item aBookmark( FN_STAT_BOOKMARK, aBookmarkIdx[nCurrId] );
        SfxViewFrame::Current()->GetDispatcher()->ExecuteList(FN_STAT_BOOKMARK,
            SfxCallMode::ASYNCHRON|SfxCallMode::RECORD,
            { &aBookmark });
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
