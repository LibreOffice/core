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

#include <DrawViewShell.hxx>

#include <svx/svxids.hrc>
#include <svl/stritem.hxx>
#include <sfx2/childwin.hxx>
#include <sfx2/docfile.hxx>
#include <svl/intitem.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/request.hxx>

#include <sfx2/viewfrm.hxx>

#include <app.hrc>
#include <sdmod.hxx>
#include <sdpage.hxx>
#include <drawdoc.hxx>
#include <DrawDocShell.hxx>
#include <pgjump.hxx>
#include <navigatr.hxx>
#include <drawview.hxx>

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
            SfxChildWindow* pWindow = GetViewFrame()->GetChildWindow( SID_NAVIGATOR );
            if( pWindow )
            {
                SdNavigatorFloat* pNavWin = static_cast<SdNavigatorFloat*>(pWindow->GetWindow());
                if( pNavWin )
                    pNavWin->InitTreeLB( GetDoc() );
            }
            SfxBindings& rBindings = GetViewFrame()->GetBindings();
            rBindings.Invalidate(SID_NAVIGATOR_STATE, true);
            rBindings.Invalidate(SID_NAVIGATOR_PAGENAME, true);
        }
        break;

        case SID_NAVIGATOR_PAGE:
        case SID_NAVIGATOR_OBJECT:
        {
            if (nSId == SID_NAVIGATOR_PAGE)
            {
                if ( mpDrawView->IsTextEdit() )
                    mpDrawView->SdrEndTextEdit();

                const SfxItemSet* pArgs = rReq.GetArgs();
                PageJump eJump = static_cast<PageJump>(static_cast<const SfxUInt16Item&>( pArgs->
                                  Get(SID_NAVIGATOR_PAGE)).GetValue());

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
                        // jump to last page
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
                OUString aTarget = static_cast<const SfxStringItem&>( pArgs->
                                 Get(SID_NAVIGATOR_OBJECT)).GetValue();
                aBookmarkStr += aTarget;
                SfxStringItem aStrItem(SID_FILE_NAME, aBookmarkStr);
                SfxStringItem aReferer(SID_REFERER, GetDocSh()->GetMedium()->GetName());
                SfxViewFrame* pFrame = GetViewFrame();
                SfxFrameItem aFrameItem(SID_DOCFRAME, pFrame);
                SfxBoolItem aBrowseItem(SID_BROWSE, true);
                pFrame->GetDispatcher()->
                ExecuteList(SID_OPENDOC, SfxCallMode::SYNCHRON | SfxCallMode::RECORD,
                    { &aStrItem, &aFrameItem, &aBrowseItem, &aReferer });
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
    NavState nState = NavState::NONE;
    sal_uInt16 nCurrentPage = 0;
    sal_uInt16 nLastPage;
    OUString aPageName;

    nState |= NavState::TableUpdate;

    if (mpActualPage != nullptr)
    {
        nCurrentPage = ( mpActualPage->GetPageNum() - 1 ) / 2;
        aPageName = mpActualPage->GetName();
    }
    nLastPage = GetDoc()->GetSdPageCount( mePageKind ) - 1;


    // first page / previous page
    if( nCurrentPage == 0 )
    {
        nState |= NavState::BtnFirstDisabled | NavState::BtnPrevDisabled;
    }
    else
    {
        nState |= NavState::BtnFirstEnabled | NavState::BtnPrevEnabled;
    }

    // last page / next page
    if( nCurrentPage == nLastPage )
    {
        nState |= NavState::BtnLastDisabled | NavState::BtnNextDisabled;
    }
    else
    {
        nState |= NavState::BtnLastEnabled | NavState::BtnNextEnabled;
    }

    rSet.Put( SfxUInt32Item( SID_NAVIGATOR_STATE, static_cast<sal_uInt32>(nState) ) );
    rSet.Put( SfxStringItem( SID_NAVIGATOR_PAGENAME, aPageName ) );
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
