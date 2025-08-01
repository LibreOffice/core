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

#include <funavig.hxx>
#include <sfx2/viewfrm.hxx>

#include <app.hrc>
#include <sdpage.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <drawdoc.hxx>
#include <DrawViewShell.hxx>
#include <ViewShell.hxx>
#include <slideshow.hxx>

#include <svx/svxids.hrc>
#include <svx/dialog/gotodlg.hxx>
#include <strings.hrc>
#include <sdresid.hxx>

namespace sd {


FuNavigation::FuNavigation (
    ViewShell& rViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument& rDoc,
    SfxRequest& rReq)
    : FuPoor(rViewSh, pWin, pView, rDoc, rReq)
{
}

rtl::Reference<FuPoor> FuNavigation::Create( ViewShell& rViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument& rDoc, SfxRequest& rReq )
{
    rtl::Reference<FuPoor> xFunc( new FuNavigation( rViewSh, pWin, pView, rDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuNavigation::DoExecute( SfxRequest& rReq )
{
    bool bSlideShow = SlideShow::IsRunning( mrViewShell.GetViewShellBase() )
        && !SlideShow::IsInteractiveSlideshow( &mrViewShell.GetViewShellBase() ); // IASS

    switch ( rReq.GetSlot() )
    {
        case SID_GO_TO_FIRST_PAGE:
        {
            if (!mpView->IsTextEdit()
                && dynamic_cast< const DrawViewShell *>( &mrViewShell ) !=  nullptr
                && !bSlideShow)
            {
               // jump to first page
               static_cast<DrawViewShell*>(&mrViewShell)->SwitchPage(0);
            }
        }
        break;

        case SID_GO_TO_PREVIOUS_PAGE:
        {
            if( !bSlideShow)
                if( auto pDrawViewShell = dynamic_cast<DrawViewShell *>( &mrViewShell ) )
                {
                    // With no modifier pressed we move to the previous
                    // slide.
                    mpView->SdrEndTextEdit();

                    // Previous page.
                    SdPage* pPage = pDrawViewShell->GetActualPage();
                    sal_uInt16 nSdPage = (pPage->GetPageNum() - 1) / 2;

                    if (nSdPage > 0)
                    {
                        // Switch the page and send events regarding
                        // deactivation the old page and activating the new
                        // one.
                        TabControl& rPageTabControl =
                            static_cast<DrawViewShell*>(&mrViewShell)
                            ->GetPageTabControl();
                        if (rPageTabControl.IsReallyShown())
                            rPageTabControl.SendDeactivatePageEvent ();
                        static_cast<DrawViewShell*>(&mrViewShell)->SwitchPage(nSdPage - 1);
                        if (rPageTabControl.IsReallyShown())
                            rPageTabControl.SendActivatePageEvent ();
                    }
                }
        }
        break;

        case SID_GO_TO_NEXT_PAGE:
        {
            if( !bSlideShow)
                if( auto pDrawViewShell = dynamic_cast<DrawViewShell *>( &mrViewShell ))
                {
                    // With no modifier pressed we move to the next slide.
                    mpView->SdrEndTextEdit();

                    // Next page.
                    SdPage* pPage = pDrawViewShell->GetActualPage();
                    sal_uInt16 nSdPage = (pPage->GetPageNum() - 1) / 2;

                    if (nSdPage < mrDoc.GetSdPageCount(pPage->GetPageKind()) - 1)
                    {
                        // Switch the page and send events regarding
                        // deactivation the old page and activating the new
                        // one.
                        TabControl& rPageTabControl =
                            static_cast<DrawViewShell*>(&mrViewShell)->GetPageTabControl();
                        if (rPageTabControl.IsReallyShown())
                            rPageTabControl.SendDeactivatePageEvent ();
                        static_cast<DrawViewShell*>(&mrViewShell)->SwitchPage(nSdPage + 1);
                        if (rPageTabControl.IsReallyShown())
                            rPageTabControl.SendActivatePageEvent ();
                    }
                }
        }
        break;

        case SID_GO_TO_LAST_PAGE:
        {
            if (!mpView->IsTextEdit() && !bSlideShow)
                if (auto pDrawViewShell = dynamic_cast<DrawViewShell *>( &mrViewShell ))
                {
                    // jump to last page
                    SdPage* pPage = pDrawViewShell->GetActualPage();
                    pDrawViewShell->SwitchPage(mrDoc.GetSdPageCount(
                            pPage->GetPageKind()) - 1);
                }
        }
        break;

        case SID_GO_TO_PAGE:
        {
            if( !bSlideShow)
                if(auto pDrawViewShell = dynamic_cast<DrawViewShell *>( &mrViewShell ))
                {
                    OUString sTitle = SdResId(STR_GOTO_PAGE_DLG_TITLE);
                    OUString sLabel = SdResId(STR_PAGE_NAME) + ":";

                    if (mrDoc.GetDocumentType() == DocumentType::Impress)
                    {
                        sTitle = SdResId(STR_GOTO_SLIDE_DLG_TITLE);
                        sLabel = SdResId(STR_SLIDE_NAME) + ":";
                    }
                    std::shared_ptr<SfxRequest> xRequest = std::make_shared<SfxRequest>(rReq);
                    rReq.Ignore(); // the 'old' request is not relevant any more

                    auto xDialog = std::make_shared<svx::GotoPageDlg>(pDrawViewShell->GetFrameWeld(), sTitle, sLabel,
                        pDrawViewShell->GetCurPagePos() + 1,
                        mrDoc.GetSdPageCount(PageKind::Standard));

                    rtl::Reference<FuNavigation> xThis( this ); // avoid destruction within async processing
                    weld::DialogController::runAsync(xDialog, [xDialog, xRequest, xThis](sal_uInt32 nResult) {
                        if (nResult == RET_OK)
                        {
                            DrawViewShell& rDrawViewShell2 = dynamic_cast<DrawViewShell&>(xThis->mrViewShell);
                            rDrawViewShell2.SwitchPage(xDialog->GetPageSelection() - 1);
                        }
                        xThis->Finish();
                        xRequest->Done();
                    });

                    return;
                }
        }
        break;
    }

    Finish();
}

void FuNavigation::Finish()
{
    // Refresh toolbar icons
    SfxBindings& rBindings = mrViewShell.GetViewFrame()->GetBindings();
    rBindings.Invalidate(SID_GO_TO_FIRST_PAGE);
    rBindings.Invalidate(SID_GO_TO_PREVIOUS_PAGE);
    rBindings.Invalidate(SID_GO_TO_NEXT_PAGE);
    rBindings.Invalidate(SID_GO_TO_LAST_PAGE);
}


} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
