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
#include "precompiled_sw.hxx"

#include "hintids.hxx"

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_USHORTS
#include <svl/svstdarr.hxx>
#endif
#include <svl/intitem.hxx>
#include <svl/stritem.hxx>
#include <sfx2/dispatch.hxx>
#ifndef _EVENT_HXX //autogen
#include <vcl/event.hxx>
#endif
#ifndef _STATUS_HXX //autogen
#include <vcl/status.hxx>
#endif
#ifndef _MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif
#include "cmdid.h"
#include "errhdl.hxx"
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

    sal_uInt16          GetCurId() const { return nCurId; }

private:
    sal_uInt16          nCurId;

    virtual void    Select();
};

// -----------------------------------------------------------------------

BookmarkPopup_Impl::BookmarkPopup_Impl() :
    PopupMenu(),
    nCurId(USHRT_MAX)
{
}

// -----------------------------------------------------------------------

void BookmarkPopup_Impl::Select()
{
    nCurId = GetCurItemId();
}

// class SvxZoomStatusBarControl ------------------------------------------

SwBookmarkControl::SwBookmarkControl( sal_uInt16 _nSlotId,
                                      sal_uInt16 _nId,
                                      StatusBar& rStb ) :
    SfxStatusBarControl( _nSlotId, _nId, rStb )
{
}

// -----------------------------------------------------------------------

SwBookmarkControl::~SwBookmarkControl()
{
}

// -----------------------------------------------------------------------

void SwBookmarkControl::StateChanged(
    sal_uInt16 /*nSID*/, SfxItemState eState, const SfxPoolItem* pState )
{
    if( eState != SFX_ITEM_AVAILABLE || pState->ISA( SfxVoidItem ) )
        GetStatusBar().SetItemText( GetId(), String() );
    else if ( pState->ISA( SfxStringItem ) )
    {
        sPageNumber = ((SfxStringItem*)pState)->GetValue();
        GetStatusBar().SetItemText( GetId(), sPageNumber );
    }
}

// -----------------------------------------------------------------------

void SwBookmarkControl::Paint( const UserDrawEvent&  )
{
    GetStatusBar().SetItemText( GetId(), sPageNumber );
}

// -----------------------------------------------------------------------

void SwBookmarkControl::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == COMMAND_CONTEXTMENU &&
            GetStatusBar().GetItemText( GetId() ).Len() )
    {
        CaptureMouse();
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
                ppBookmark++)
            {
                if(IDocumentMarkAccess::BOOKMARK == IDocumentMarkAccess::GetType(**ppBookmark))
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
                    SFX_CALLMODE_ASYNCHRON|SFX_CALLMODE_RECORD,
                                        &aBookmark, 0L );
            }
        }
        ReleaseMouse();
    }
}
