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

#include "hintids.hxx"
#include <swtypes.hxx>
#include <utlui.hrc>

#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <sfx2/dispatch.hxx>
#include <vcl/event.hxx>
#include <vcl/status.hxx>
#include <vcl/menu.hxx>
#include "cmdid.h"
#include "swmodule.hxx"
#include "wrtsh.hxx"
#include "IMark.hxx"
#include "bookctrl.hxx"
#include <map>

SFX_IMPL_STATUSBAR_CONTROL( SwBookmarkControl, SfxStringItem );

class BookmarkPopup_Impl : public PopupMenu
{
public:
    BookmarkPopup_Impl();

    sal_uInt16          GetCurId() const { return nCurId; }

private:
    sal_uInt16          nCurId;

    virtual void    Select() SAL_OVERRIDE;
};

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
    if( eState != SfxItemState::DEFAULT || dynamic_cast< const SfxVoidItem *>( pState ) !=  nullptr )
        GetStatusBar().SetItemText( GetId(), OUString() );
    else if ( dynamic_cast< const SfxStringItem *>( pState ) !=  nullptr )
    {
        sPageNumber = static_cast<const SfxStringItem*>(pState)->GetValue();
        GetStatusBar().SetItemText( GetId(), sPageNumber );
    }
    else if ( dynamic_cast< const SfxBoolItem *>( pState ) !=  nullptr )
    {
        if (static_cast<const SfxBoolItem*>(pState)->GetValue()) // Indicates whether to show extended tooltip
            GetStatusBar().SetQuickHelpText(GetId(), SW_RESSTR(STR_BOOKCTRL_HINT_EXTENDED));
        else
            GetStatusBar().SetQuickHelpText(GetId(), SW_RESSTR(STR_BOOKCTRL_HINT));
    }

}

void SwBookmarkControl::Paint( const UserDrawEvent&  )
{
    GetStatusBar().SetItemText( GetId(), sPageNumber );
}

void SwBookmarkControl::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == CommandEventId::ContextMenu &&
            !GetStatusBar().GetItemText( GetId() ).isEmpty() )
    {
        BookmarkPopup_Impl aPop;
        SwWrtShell* pWrtShell = ::GetActiveWrtShell();
        if( pWrtShell && pWrtShell->getIDocumentMarkAccess()->getAllMarksCount() > 0 )
        {
            IDocumentMarkAccess* const pMarkAccess = pWrtShell->getIDocumentMarkAccess();
            IDocumentMarkAccess::const_iterator_t ppBookmarkStart = pMarkAccess->getBookmarksBegin();
            sal_uInt16 nPopupId = 1;
            ::std::map<sal_Int32, sal_uInt16> aBookmarkIdx;
            for(IDocumentMarkAccess::const_iterator_t ppBookmark = ppBookmarkStart;
                ppBookmark != pMarkAccess->getBookmarksEnd();
                ++ppBookmark)
            {
                if(IDocumentMarkAccess::MarkType::BOOKMARK == IDocumentMarkAccess::GetType(**ppBookmark))
                {
                    aPop.InsertItem( nPopupId, ppBookmark->get()->GetName() );
                    aBookmarkIdx[nPopupId] = static_cast<sal_uInt16>(ppBookmark - ppBookmarkStart);
                    nPopupId++;
                }
            }
            aPop.Execute( &GetStatusBar(), rCEvt.GetMousePosPixel());
            sal_uInt16 nCurrId = aPop.GetCurId();
            if( nCurrId != USHRT_MAX)
            {
                SfxUInt16Item aBookmark( FN_STAT_BOOKMARK, aBookmarkIdx[nCurrId] );
                SfxViewFrame::Current()->GetDispatcher()->Execute( FN_STAT_BOOKMARK,
                    SfxCallMode::ASYNCHRON|SfxCallMode::RECORD,
                                        &aBookmark, 0L );
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
