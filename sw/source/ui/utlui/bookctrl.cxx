/*************************************************************************
 *
 *  $RCSfile: bookctrl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:49 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

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
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
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

SwBookmarkControl::SwBookmarkControl( USHORT nId,
                                                  StatusBar& rStb,
                                                  SfxBindings& rBind ) :
    SfxStatusBarControl( nId, rStb, rBind )
{
}

// -----------------------------------------------------------------------

SwBookmarkControl::~SwBookmarkControl()
{
}

// -----------------------------------------------------------------------

void SwBookmarkControl::StateChanged( const SfxPoolItem* pState )
{
    SfxItemState eState = GetItemState(pState);

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
            USHORT nId = aPop.GetCurId();
            if( nId != USHRT_MAX)
            {
                SfxUInt16Item aBookmark( FN_STAT_BOOKMARK, aBookArr[nId-1] );
                SfxViewFrame::Current()->GetDispatcher()->Execute(  FN_STAT_BOOKMARK,
                    SFX_CALLMODE_ASYNCHRON|SFX_CALLMODE_RECORD,
                                        &aBookmark, 0L );
            }
        }
        ReleaseMouse();
    }
}

/*************************************************************************

      $Log: not supported by cvs2svn $
      Revision 1.30  2000/09/18 16:06:17  willem.vandorp
      OpenOffice header added.

      Revision 1.29  2000/09/07 15:59:34  os
      change: SFX_DISPATCHER/SFX_BINDINGS removed

      Revision 1.28  2000/02/11 15:00:22  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.27  1998/03/10 14:35:06  OS
      Textmarkenindex wieder selbst feststellen #47718#


      Rev 1.26   10 Mar 1998 15:35:06   OS
   Textmarkenindex wieder selbst feststellen #47718#

      Rev 1.25   23 Feb 1998 07:20:24   OS
   GetBookmark/ Cnt mit Bookmark-Flag

      Rev 1.24   21 Nov 1997 12:10:14   MA
   includes

      Rev 1.23   03 Nov 1997 13:59:22   MA
   precomp entfernt

      Rev 1.22   29 Aug 1997 15:57:30   OS
   PopupMenu::Execute mit Window* fuer VCL

      Rev 1.21   29 Aug 1997 13:36:12   OS
   DLL-Umbau

      Rev 1.20   08 Aug 1997 17:25:24   OM
   Headerfile-Umstellung

      Rev 1.19   09 Jul 1997 17:45:30   HJS
   includes

      Rev 1.18   14 Jan 1997 09:37:30   TRI
   includes

      Rev 1.17   23 Oct 1996 13:49:32   JP
   SVMEM -> SVSTDARR Umstellung

      Rev 1.16   28 Aug 1996 15:42:46   OS
   includes

      Rev 1.15   24 Jul 1996 13:49:56   OS
   PCH nachgeruestet

      Rev 1.14   26 Jun 1996 15:04:02   OS
   Aufruf von Dispatcher.Execute an 324 angepasst

      Rev 1.13   01 Feb 1996 19:07:02   JP
   BookmarkControl::Command: ans PopUp die Pos im Bookmark-Array uebergeben, nicht die laufende!

      Rev 1.12   17 Nov 1995 12:47:52   MA
   Segmentierung

      Rev 1.11   13 Nov 1995 20:46:10   JP
   Ptrarr gegen SvUShortArr ausgetauscht

      Rev 1.10   07 Nov 1995 18:04:16   MA
   opt/fix/chg: statics und segs

      Rev 1.9   17 Oct 1995 20:54:14   JP
   IsMark oder IsBookmark am Bookmark benutzen

      Rev 1.8   06 Oct 1995 00:20:14   HJS
   pState => eState

      Rev 1.7   05 Oct 1995 18:41:12   OM
   StateChanged umgestellt

      Rev 1.6   09 Aug 1995 19:15:18   OS
   Umstellung auf CommandHdl

*************************************************************************/


