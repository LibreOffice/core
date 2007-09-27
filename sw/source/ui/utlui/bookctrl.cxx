/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bookctrl.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 12:44:36 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include "hintids.hxx"

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_USHORTS
#include <svtools/svstdarr.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
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
#include "bookmrk.hxx"
#include "bookctrl.hxx"

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

SwBookmarkControl::SwBookmarkControl( USHORT _nSlotId,
                                      USHORT _nId,
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
        USHORT nBookCnt;
        if( pWrtShell && 0 != ( nBookCnt = pWrtShell->GetBookmarkCnt() ) )
        {
            SvUShorts aBookArr;
            for( USHORT nCount = 0; nCount < nBookCnt; ++nCount )
            {
                SwBookmark& rBkmk = pWrtShell->GetBookmark( nCount );
                if( rBkmk.IsBookMark() )
                {
                    aBookArr.Insert( nCount, aBookArr.Count() );
                    aPop.InsertItem( aBookArr.Count(), rBkmk.GetName() );
                }
            }
            aPop.Execute( &GetStatusBar(), rCEvt.GetMousePosPixel());
            USHORT nCurrId = aPop.GetCurId();
            if( nCurrId != USHRT_MAX)
            {
                SfxUInt16Item aBookmark( FN_STAT_BOOKMARK, aBookArr[nCurrId-1] );
                SfxViewFrame::Current()->GetDispatcher()->Execute(  FN_STAT_BOOKMARK,
                    SFX_CALLMODE_ASYNCHRON|SFX_CALLMODE_RECORD,
                                        &aBookmark, 0L );
            }
        }
        ReleaseMouse();
    }
}


