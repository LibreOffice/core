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

#include "funavig.hxx"
#include <sfx2/viewfrm.hxx>

#include "app.hrc"
#include "sdpage.hxx"
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include "drawdoc.hxx"
#include "DrawViewShell.hxx"
#include "ViewShell.hxx"
#include "slideshow.hxx"

namespace sd {

TYPEINIT1( FuNavigation, FuPoor );

FuNavigation::FuNavigation (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

rtl::Reference<FuPoor> FuNavigation::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuNavigation( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuNavigation::DoExecute( SfxRequest& rReq )
{
    bool bSlideShow = SlideShow::IsRunning( mpViewShell->GetViewShellBase() );

    switch ( rReq.GetSlot() )
    {
        case SID_GO_TO_FIRST_PAGE:
        {
            if (!mpView->IsTextEdit()
                && dynamic_cast< const DrawViewShell *>( mpViewShell ) !=  nullptr
                && !bSlideShow)
            {
               // jump to first page
               static_cast<DrawViewShell*>(mpViewShell)->SwitchPage(0);
            }
        }
        break;

        case SID_GO_TO_PREVIOUS_PAGE:
        {
            if( dynamic_cast< const DrawViewShell *>( mpViewShell ) !=  nullptr && !bSlideShow)
            {
                // With no modifier pressed we move to the previous
                // slide.
                mpView->SdrEndTextEdit();

                // Previous page.
                SdPage* pPage = static_cast<DrawViewShell*>(mpViewShell)->GetActualPage();
                sal_uInt16 nSdPage = (pPage->GetPageNum() - 1) / 2;

                if (nSdPage > 0)
                {
                    // Switch the page and send events regarding
                    // deactivation the old page and activating the new
                    // one.
                    TabControl& rPageTabControl =
                        static_cast<DrawViewShell*>(mpViewShell)
                        ->GetPageTabControl();
                    if (rPageTabControl.IsReallyShown())
                        rPageTabControl.SendDeactivatePageEvent ();
                    static_cast<DrawViewShell*>(mpViewShell)->SwitchPage(nSdPage - 1);
                    if (rPageTabControl.IsReallyShown())
                        rPageTabControl.SendActivatePageEvent ();
                }
            }
        }
        break;

        case SID_GO_TO_NEXT_PAGE:
        {
            if( dynamic_cast< const DrawViewShell *>( mpViewShell ) !=  nullptr && !bSlideShow)
            {
                // With no modifier pressed we move to the next slide.
                mpView->SdrEndTextEdit();

                // Next page.
                SdPage* pPage = static_cast<DrawViewShell*>(mpViewShell)->GetActualPage();
                sal_uInt16 nSdPage = (pPage->GetPageNum() - 1) / 2;

                if (nSdPage < mpDoc->GetSdPageCount(pPage->GetPageKind()) - 1)
                {
                    // Switch the page and send events regarding
                    // deactivation the old page and activating the new
                    // one.
                    TabControl& rPageTabControl =
                        static_cast<DrawViewShell*>(mpViewShell)->GetPageTabControl();
                    if (rPageTabControl.IsReallyShown())
                        rPageTabControl.SendDeactivatePageEvent ();
                    static_cast<DrawViewShell*>(mpViewShell)->SwitchPage(nSdPage + 1);
                    if (rPageTabControl.IsReallyShown())
                        rPageTabControl.SendActivatePageEvent ();
                }
            }
        }
        break;

        case SID_GO_TO_LAST_PAGE:
        {
            if (!mpView->IsTextEdit()
                && dynamic_cast< const DrawViewShell *>( mpViewShell ) !=  nullptr
                && !bSlideShow)
            {
                // jump to last page
                SdPage* pPage =
                    static_cast<DrawViewShell*>(mpViewShell)->GetActualPage();
                static_cast<DrawViewShell*>(mpViewShell)
                    ->SwitchPage(mpDoc->GetSdPageCount(
                        pPage->GetPageKind()) - 1);
            }
        }
        break;
    }
    // Refresh toolbar icons
    SfxBindings& rBindings = mpViewShell->GetViewFrame()->GetBindings();
    rBindings.Invalidate(SID_GO_TO_FIRST_PAGE);
    rBindings.Invalidate(SID_GO_TO_PREVIOUS_PAGE);
    rBindings.Invalidate(SID_GO_TO_NEXT_PAGE);
    rBindings.Invalidate(SID_GO_TO_LAST_PAGE);
}


} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
