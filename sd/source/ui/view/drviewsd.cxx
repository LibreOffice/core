/*************************************************************************
 *
 *  $RCSfile: drviewsd.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 20:32:36 $
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

#include "DrawViewShell.hxx"

#ifndef _SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#ifndef _AEITEM_HXX //autogen
#include <svtools/aeitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif

#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif

#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif

#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif

#include <sfx2/viewfrm.hxx>

#pragma hdrstop

#include "app.hrc"

#include "sdpage.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#ifndef _SD_SLIDESHOW_HXX
#include "slideshow.hxx"
#endif
#include "pgjump.hxx"
#ifndef SD_NAVIGATOR_CHILD_WINDOW_HXX
#include "NavigatorChildWindow.hxx"
#endif
#ifndef SD_NAVIGATION_HXX
#include "navigatr.hxx"
#endif
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif
#ifndef _SD_SLIDESHOW_HXX
#include "slideshow.hxx"
#endif

namespace sd {

/*************************************************************************
|*
|* SfxRequests fuer Navigator bearbeiten
|*
\************************************************************************/

void DrawViewShell::ExecNavigatorWin( SfxRequest& rReq )
{
    CheckLineTo (rReq);

    USHORT nSId = rReq.GetSlot();

    switch( nSId )
    {
        case SID_NAVIGATOR_INIT:
        {
            USHORT nId = SID_NAVIGATOR;
            SfxChildWindow* pWindow = GetViewFrame()->GetChildWindow( nId );
            if( pWindow )
            {
                SdNavigatorWin* pNavWin = (SdNavigatorWin*)( pWindow->GetContextWindow( SD_MOD() ) );
                if( pNavWin )
                    pNavWin->InitTreeLB( GetDoc() );
            }
        }
        break;

        case SID_NAVIGATOR_PEN:
        case SID_NAVIGATOR_PAGE:
        case SID_NAVIGATOR_OBJECT:
        {
            if (mpSlideShow)
            {
                mpSlideShow->receiveRequest( rReq );
            }
            else if (nSId == SID_NAVIGATOR_PAGE)
            {
                if ( pDrView->IsTextEdit() )
                    pDrView->EndTextEdit();

                const SfxItemSet* pArgs = rReq.GetArgs();
                PageJump eJump = (PageJump)((SfxAllEnumItem&) pArgs->
                                  Get(SID_NAVIGATOR_PAGE)).GetValue();

                switch (eJump)
                {
                    case PAGE_FIRST:
                    {
                        // Sprung zu erster Seite
                        SwitchPage(0);
                    }
                    break;

                    case PAGE_LAST:
                    {
                        // Sprung zu letzter Seite
                        SwitchPage(GetDoc()->GetSdPageCount(pActualPage->GetPageKind()) - 1);
                    }
                    break;

                    case PAGE_NEXT:
                    {
                        // Sprung zu naechster Seite
                        USHORT nSdPage = (pActualPage->GetPageNum() - 1) / 2;

                        if (nSdPage < GetDoc()->GetSdPageCount(pActualPage->GetPageKind()) - 1)
                        {
                            SwitchPage(nSdPage + 1);
                        }
                    }
                    break;

                    case PAGE_PREVIOUS:
                    {
                        // Sprung zu vorheriger Seite
                        USHORT nSdPage = (pActualPage->GetPageNum() - 1) / 2;

                        if (nSdPage > 0)
                        {
                            SwitchPage(nSdPage - 1);
                        }
                    }
                    break;
                }
            }
            else if (nSId == SID_NAVIGATOR_OBJECT)
            {
                String aBookmarkStr;
                aBookmarkStr += sal_Unicode( '#' );
                const SfxItemSet* pArgs = rReq.GetArgs();
                String aTarget = ((SfxStringItem&) pArgs->
                                 Get(SID_NAVIGATOR_OBJECT)).GetValue();
                aBookmarkStr += aTarget;
                SfxStringItem aStrItem(SID_FILE_NAME, aBookmarkStr);
                SfxStringItem aReferer(SID_REFERER, GetDocSh()->GetMedium()->GetName());
                SfxViewFrame* pFrame = GetViewFrame();
                SfxFrameItem aFrameItem(SID_DOCFRAME, pFrame);
                SfxBoolItem aBrowseItem(SID_BROWSE, TRUE);
                pFrame->GetDispatcher()->
                Execute(SID_OPENDOC, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD,
                            &aStrItem, &aFrameItem, &aBrowseItem, &aReferer, 0L);
            }

            SfxBindings& rBindings = GetViewFrame()->GetBindings();
            rBindings.Invalidate( SID_NAVIGATOR_STATE );
            rBindings.Invalidate( SID_NAVIGATOR_PAGENAME );
        }
        break;

        default:
        break;
    }
}

/*************************************************************************
|*
|* Statuswerte fuer Navigator zurueckgeben
|*
\************************************************************************/

void DrawViewShell::GetNavigatorWinState( SfxItemSet& rSet )
{
    UINT32 nState = NAVSTATE_NONE;
    USHORT nCurrentPage;
    USHORT nFirstPage = 0;
    USHORT nLastPage;
    BOOL   bEndless = FALSE;
    String aPageName;

    if( mpSlideShow )
    {
        // pen activated?
        nState |= mpSlideShow->isDrawingPossible() ? NAVBTN_PEN_CHECKED : NAVBTN_PEN_UNCHECKED;

        nCurrentPage = mpSlideShow->getCurrentPageNumber();
        nFirstPage = mpSlideShow->getFirstPageNumber();
        nLastPage = mpSlideShow->getLastPageNumber();
        bEndless = mpSlideShow->isEndless();

        // Get the page for the current page number.
        SdPage* pPage = 0;
        if( (nCurrentPage >= 0) && (nCurrentPage < GetDoc()->GetSdPageCount( PK_STANDARD ) ) )
            pPage = GetDoc()->GetSdPage (nCurrentPage, PK_STANDARD);

        if(pPage)
            aPageName = pPage->GetName();
    }
    else
    {
        nState |= NAVBTN_PEN_DISABLED | NAVTLB_UPDATE;

        nCurrentPage = ( pActualPage->GetPageNum() - 1 ) / 2;
        nLastPage = GetDoc()->GetSdPageCount( ePageKind ) - 1;
        aPageName = pActualPage->GetName();
    }

    // erste Seite / vorherige Seite
    if( nCurrentPage == nFirstPage )
    {
        nState |= NAVBTN_FIRST_DISABLED;
        if( !bEndless )
            nState |= NAVBTN_PREV_DISABLED;
        else
            nState |= NAVBTN_PREV_ENABLED;
    }
    else
    {
        nState |= NAVBTN_FIRST_ENABLED | NAVBTN_PREV_ENABLED;
    }

    // letzte Seite / naechste Seite
    if( nCurrentPage == nLastPage )
    {
        nState |= NAVBTN_LAST_DISABLED;
        if( !bEndless )
            nState |= NAVBTN_NEXT_DISABLED;
        else
            nState |= NAVBTN_NEXT_ENABLED;
    }
    else
    {
        nState |= NAVBTN_LAST_ENABLED | NAVBTN_NEXT_ENABLED;
    }

    rSet.Put( SfxUInt32Item( SID_NAVIGATOR_STATE, nState ) );
    rSet.Put( SfxStringItem( SID_NAVIGATOR_PAGENAME, aPageName ) );
}

} // end of namespace sd
