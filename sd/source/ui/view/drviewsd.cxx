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

namespace sd {

/**
 * handle SfxRequests for navigator
 */
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
                        // jump to first page
                        SwitchPage(0);
                    }
                    break;

                    case PAGE_LAST:
                    {
                        // jumpt to last page
                        SwitchPage(GetDoc()->GetSdPageCount(mpActualPage->GetPageKind()) - 1);
                    }
                    break;

                    case PAGE_NEXT:
                    {
                        // jump to next page
                        sal_uInt16 nSdPage = (mpActualPage->GetPageNum() - 1) / 2;

                        if (nSdPage < GetDoc()->GetSdPageCount(mpActualPage->GetPageKind()) - 1)
                        {
                            SwitchPage(nSdPage + 1);
                        }
                    }
                    break;

                    case PAGE_PREVIOUS:
                    {
                        // jump to previous page
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
                OUString aBookmarkStr("#");
                const SfxItemSet* pArgs = rReq.GetArgs();
                OUString aTarget = ((SfxStringItem&) pArgs->
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


void DrawViewShell::GetNavigatorWinState( SfxItemSet& rSet )
{
    sal_uInt32 nState = NAVSTATE_NONE;
    sal_uInt16 nCurrentPage = 0;
    sal_uInt16 nFirstPage = 0;
    sal_uInt16 nLastPage;
    sal_Bool   bEndless = sal_False;
    OUString aPageName;

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

    // first page / previous page
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

    // last page / next page
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
