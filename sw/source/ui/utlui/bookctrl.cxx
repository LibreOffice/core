/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_sw.hxx"

#include "hintids.hxx"

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_USHORTS
#include <svl/svstdarr.hxx>
#endif
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

// class BookmarkPopup_Impl --------------------------------------------------

class BookmarkPopup_Impl : public PopupMenu
{
public:
    BookmarkPopup_Impl();

    USHORT          GetCurId() const { return nCurId; }

private:
    USHORT          nCurId;

    virtual void    Select();
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

// class SvxZoomStatusBarControl ------------------------------------------

SwBookmarkControl::SwBookmarkControl( USHORT _nSlotId,
                                      USHORT _nId,
                                      StatusBar& rStb ) :
    SfxStatusBarControl( _nSlotId, _nId, rStb )
{
}

SwBookmarkControl::~SwBookmarkControl()
{
}

void SwBookmarkControl::StateChanged(
    USHORT /*nSID*/, SfxItemState eState, const SfxPoolItem* pState )
{
    if( eState != SFX_ITEM_AVAILABLE || pState->ISA( SfxVoidItem ) )
        GetStatusBar().SetItemText( GetId(), String() );
    else if ( pState->ISA( SfxStringItem ) )
    {
        sPageNumber = ((SfxStringItem*)pState)->GetValue();
        GetStatusBar().SetItemText( GetId(), sPageNumber );
    }
}

void SwBookmarkControl::Paint( const UserDrawEvent&  )
{
    GetStatusBar().SetItemText( GetId(), sPageNumber );
}

void SwBookmarkControl::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU &&
            GetStatusBar().GetItemText( GetId() ).Len() )
    {
        CaptureMouse();
        BookmarkPopup_Impl aPop;
        SwWrtShell* pWrtShell = ::GetActiveWrtShell();
        if( pWrtShell && pWrtShell->getIDocumentMarkAccess()->getMarksCount() > 0 )
        {
            IDocumentMarkAccess* const pMarkAccess = pWrtShell->getIDocumentMarkAccess();
            IDocumentMarkAccess::const_iterator_t ppBookmarkStart = pMarkAccess->getBookmarksBegin();
            USHORT nPopupId = 1;
            ::std::map<sal_Int32, USHORT> aBookmarkIdx;
            for(IDocumentMarkAccess::const_iterator_t ppBookmark = ppBookmarkStart;
                ppBookmark != pMarkAccess->getBookmarksEnd();
                ppBookmark++)
            {
                if(IDocumentMarkAccess::BOOKMARK == IDocumentMarkAccess::GetType(**ppBookmark))
                {
                    aPop.InsertItem( nPopupId, ppBookmark->get()->GetName() );
                    aBookmarkIdx[nPopupId] = static_cast<USHORT>(ppBookmark - ppBookmarkStart);
                    nPopupId++;
                }
            }
            aPop.Execute( &GetStatusBar(), rCEvt.GetMousePosPixel());
            USHORT nCurrId = aPop.GetCurId();
            if( nCurrId != USHRT_MAX)
            {
                SfxUInt16Item aBookmark( FN_STAT_BOOKMARK, aBookmarkIdx[nCurrId] );
                SfxViewFrame::Current()->GetDispatcher()->Execute( FN_STAT_BOOKMARK,
                    SFX_CALLMODE_ASYNCHRON|SFX_CALLMODE_RECORD,
                                        &aBookmark, 0L );
            }
        }
        ReleaseMouse();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
