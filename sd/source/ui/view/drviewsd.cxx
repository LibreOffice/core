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
#include "precompiled_sd.hxx"

#include "DrawViewShell.hxx"

#include <svx/svxids.hrc>
#include <svl/aeitem.hxx>
#include <svl/stritem.hxx>
#include <sfx2/docfile.hxx>
#include <svl/intitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>

#include <sfx2/viewfrm.hxx>


#include "app.hrc"

#include "sdpage.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "slideshow.hxx"
#include "pgjump.hxx"
#include "NavigatorChildWindow.hxx"
#include "navigatr.hxx"
#include "drawview.hxx"
#include "slideshow.hxx"

namespace sd {

/*************************************************************************
|*
|* SfxRequests fuer Navigator bearbeiten
|*
\************************************************************************/

void DrawViewShell::ExecNavigatorWin( SfxRequest& rReq )
{
    CheckLineTo (rReq);

    sal_uInt16 nSId = rReq.GetSlot();

    switch( nSId )
    {
        case SID_NAVIGATOR_INIT:
        {
            sal_uInt16 nId = SID_NAVIGATOR;
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
            rtl::Reference< SlideShow > xSlideshow( SlideShow::GetSlideShow( GetViewShellBase() ) );
            if (xSlideshow.is() && xSlideshow->isRunning() )
            {
                xSlideshow->receiveRequest( rReq );
            }
            else if (nSId == SID_NAVIGATOR_PAGE)
            {
                if ( mpDrawView->IsTextEdit() )
                    mpDrawView->SdrEndTextEdit();

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
                        SwitchPage(GetDoc()->GetSdPageCount(mpActualPage->GetPageKind()) - 1);
                    }
                    break;

                    case PAGE_NEXT:
                    {
                        // Sprung zu naechster Seite
                        sal_uInt16 nSdPage = (mpActualPage->GetPageNum() - 1) / 2;

                        if (nSdPage < GetDoc()->GetSdPageCount(mpActualPage->GetPageKind()) - 1)
                        {
                            SwitchPage(nSdPage + 1);
                        }
                    }
                    break;

                    case PAGE_PREVIOUS:
                    {
                        // Sprung zu vorheriger Seite
                        sal_uInt16 nSdPage = (mpActualPage->GetPageNum() - 1) / 2;

                        if (nSdPage > 0)
                        {
                            SwitchPage(nSdPage - 1);
                        }
                    }
                    break;

                    case PAGE_NONE:
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
                SfxBoolItem aBrowseItem(SID_BROWSE, sal_True);
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
    sal_uInt32 nState = NAVSTATE_NONE;
    sal_uInt16 nCurrentPage = 0;
    sal_uInt16 nFirstPage = 0;
    sal_uInt16 nLastPage;
    sal_Bool   bEndless = sal_False;
    String aPageName;

    rtl::Reference< SlideShow > xSlideshow( SlideShow::GetSlideShow( GetViewShellBase() ) );
    if( xSlideshow.is() && xSlideshow->isRunning() )
    {
        // pen activated?
        nState |= xSlideshow->isDrawingPossible() ? NAVBTN_PEN_CHECKED : NAVBTN_PEN_UNCHECKED;

        nCurrentPage = (sal_uInt16)xSlideshow->getCurrentPageNumber();
        nFirstPage = (sal_uInt16)xSlideshow->getFirstPageNumber();
        nLastPage = (sal_uInt16)xSlideshow->getLastPageNumber();
        bEndless = xSlideshow->isEndless();

        // Get the page for the current page number.
        SdPage* pPage = 0;
        if( nCurrentPage < GetDoc()->GetSdPageCount( PK_STANDARD ) )
            pPage = GetDoc()->GetSdPage (nCurrentPage, PK_STANDARD);

        if(pPage)
            aPageName = pPage->GetName();
    }
    else
    {
        nState |= NAVBTN_PEN_DISABLED | NAVTLB_UPDATE;

        if (mpActualPage != NULL)
        {
            nCurrentPage = ( mpActualPage->GetPageNum() - 1 ) / 2;
            aPageName = mpActualPage->GetName();
        }
        nLastPage = GetDoc()->GetSdPageCount( mePageKind ) - 1;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
