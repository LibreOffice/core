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

#include "fuprlout.hxx"
#include <vcl/wrkwin.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/smplhint.hxx>
#include <svl/itempool.hxx>
#include <sot/storage.hxx>
#include <vcl/msgbox.hxx>
#include <svx/svdundo.hxx>

#include <sfx2/viewfrm.hxx>
#include <sfx2/request.hxx>

#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "pres.hxx"
#include "DrawViewShell.hxx"
#include "FrameView.hxx"
#include "stlpool.hxx"
#include "View.hxx"
#include "glob.hrc"
#include "glob.hxx"
#include "strings.hrc"
#include "strmname.h"
#include "app.hrc"
#include "DrawDocShell.hxx"
#include "unprlout.hxx"
#include "unchss.hxx"
#include "unmovss.hxx"
#include "sdattr.hxx"
#include "sdresid.hxx"
#include "drawview.hxx"
#include <editeng/outliner.hxx>
#include <editeng/editdata.hxx>
#include "sdabstdlg.hxx"

namespace sd
{

TYPEINIT1( FuPresentationLayout, FuPoor );

#define DOCUMENT_TOKEN '#'

FuPresentationLayout::FuPresentationLayout (
    ViewShell* pViewSh,
    ::sd::Window* pWin,
    ::sd::View* pView,
    SdDrawDocument* pDoc,
    SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
}

FunctionReference FuPresentationLayout::Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq )
{
    FunctionReference xFunc( new FuPresentationLayout( pViewSh, pWin, pView, pDoc, rReq ) );
    xFunc->DoExecute(rReq);
    return xFunc;
}

void FuPresentationLayout::DoExecute( SfxRequest& rReq )
{
    // prevent selected objects or objects which are under editing from disappearing
    mpView->SdrEndTextEdit();

    if(mpView->GetSdrPageView())
    {
        mpView->UnmarkAll();
    }

    sal_Bool bError = sal_False;

    // determine the active page
    sal_uInt16 nSelectedPage = SDRPAGE_NOTFOUND;
    for (sal_uInt16 nPage = 0; nPage < mpDoc->GetSdPageCount(PK_STANDARD); nPage++)
    {
        if (mpDoc->GetSdPage(nPage, PK_STANDARD)->IsSelected())
        {
            nSelectedPage = nPage;
            break;
        }
    }

    DBG_ASSERT(nSelectedPage != SDRPAGE_NOTFOUND, "no selected page");
    SdPage* pSelectedPage = mpDoc->GetSdPage(nSelectedPage, PK_STANDARD);
    OUString aOldLayoutName(pSelectedPage->GetLayoutName());
    sal_Int32 nPos = aOldLayoutName.indexOf(SD_LT_SEPARATOR);
    if (nPos != -1)
        aOldLayoutName = aOldLayoutName.copy(0, nPos);

    /* if we are on a master page, the changes apply for all pages and notes-
       pages who are using the relevant layout */
    sal_Bool bOnMaster = sal_False;
    if( mpViewShell && mpViewShell->ISA(DrawViewShell))
    {
        EditMode eEditMode =
            static_cast<DrawViewShell*>(mpViewShell)->GetEditMode();
        if (eEditMode == EM_MASTERPAGE)
            bOnMaster = sal_True;
    }
    sal_Bool bMasterPage = bOnMaster;
    sal_Bool bCheckMasters = sal_False;

    // call dialog
    sal_Bool   bLoad = sal_False;           // appear the new master pages?
    OUString aFile;

    SfxItemSet aSet(mpDoc->GetPool(), ATTR_PRESLAYOUT_START, ATTR_PRESLAYOUT_END);

    aSet.Put( SfxBoolItem( ATTR_PRESLAYOUT_LOAD, bLoad));
    aSet.Put( SfxBoolItem( ATTR_PRESLAYOUT_MASTER_PAGE, bMasterPage ) );
    aSet.Put( SfxBoolItem( ATTR_PRESLAYOUT_CHECK_MASTERS, bCheckMasters ) );
    aSet.Put( SfxStringItem( ATTR_PRESLAYOUT_NAME, aOldLayoutName));



    const SfxItemSet *pArgs = rReq.GetArgs ();

    if (pArgs)
    {
        if (pArgs->GetItemState(ATTR_PRESLAYOUT_LOAD) == SFX_ITEM_SET)
            bLoad = ((SfxBoolItem&)pArgs->Get(ATTR_PRESLAYOUT_LOAD)).GetValue();
        if( pArgs->GetItemState( ATTR_PRESLAYOUT_MASTER_PAGE ) == SFX_ITEM_SET )
            bMasterPage = ( (SfxBoolItem&) pArgs->Get( ATTR_PRESLAYOUT_MASTER_PAGE ) ).GetValue();
        if( pArgs->GetItemState( ATTR_PRESLAYOUT_CHECK_MASTERS ) == SFX_ITEM_SET )
            bCheckMasters = ( (SfxBoolItem&) pArgs->Get( ATTR_PRESLAYOUT_CHECK_MASTERS ) ).GetValue();
        if (pArgs->GetItemState(ATTR_PRESLAYOUT_NAME) == SFX_ITEM_SET)
            aFile = ((SfxStringItem&)pArgs->Get(ATTR_PRESLAYOUT_NAME)).GetValue();
    }
    else
    {
        SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
        AbstractSdPresLayoutDlg* pDlg = pFact ? pFact->CreateSdPresLayoutDlg(mpDocSh, NULL, aSet ) : 0;

        sal_uInt16 nResult = pDlg ? pDlg->Execute() : static_cast<short>(RET_CANCEL);

        switch (nResult)
        {
            case RET_OK:
            {
                pDlg->GetAttr(aSet);
                if (aSet.GetItemState(ATTR_PRESLAYOUT_LOAD) == SFX_ITEM_SET)
                    bLoad = ((SfxBoolItem&)aSet.Get(ATTR_PRESLAYOUT_LOAD)).GetValue();
                if( aSet.GetItemState( ATTR_PRESLAYOUT_MASTER_PAGE ) == SFX_ITEM_SET )
                    bMasterPage = ( (SfxBoolItem&) aSet.Get( ATTR_PRESLAYOUT_MASTER_PAGE ) ).GetValue();
                if( aSet.GetItemState( ATTR_PRESLAYOUT_CHECK_MASTERS ) == SFX_ITEM_SET )
                    bCheckMasters = ( (SfxBoolItem&) aSet.Get( ATTR_PRESLAYOUT_CHECK_MASTERS ) ).GetValue();
                if (aSet.GetItemState(ATTR_PRESLAYOUT_NAME) == SFX_ITEM_SET)
                    aFile = ((SfxStringItem&)aSet.Get(ATTR_PRESLAYOUT_NAME)).GetValue();
            }
            break;

            default:
                bError = sal_True;
        }
        delete pDlg;
    }

    if (!bError)
    {
        mpDocSh->SetWaitCursor( sal_True );

        /* Here, we only exchange masterpages, therefore the current page
           remains the current page. To prevent calling PageOrderChangedHint
           during insertion and extraction of the masterpages, we block. */
        /* That isn't quitely right. If the masterpageview is active and you are
           removing a masterpage, it's possible that you are removing the
           current masterpage. So you have to call ResetActualPage ! */
        if( mpViewShell->ISA(DrawViewShell) && !bCheckMasters )
            static_cast<DrawView*>(mpView)->BlockPageOrderChangedHint(sal_True);

        if (bLoad)
        {
            OUString aFileName = aFile.getToken(0, DOCUMENT_TOKEN);
            SdDrawDocument* pTempDoc = mpDoc->OpenBookmarkDoc( aFileName );

            // #69581: If I chosed the standard-template I got no filename and so I get no
            //         SdDrawDocument-Pointer. But the method SetMasterPage is able to handle
            //         a NULL-pointer as a Standard-template ( look at SdDrawDocument::SetMasterPage )
            OUString aLayoutName;
            if( pTempDoc )
                aLayoutName = aFile.getToken(1, DOCUMENT_TOKEN);

            mpDoc->SetMasterPage(nSelectedPage, aLayoutName, pTempDoc, bMasterPage, bCheckMasters);
            mpDoc->CloseBookmarkDoc();
        }
        else
        {
            // use master page with the layout name aFile from current Doc
            mpDoc->SetMasterPage(nSelectedPage, aFile, mpDoc, bMasterPage, bCheckMasters);
        }

        // remove blocking
        if (mpViewShell->ISA(DrawViewShell) && !bCheckMasters )
            static_cast<DrawView*>(mpView)->BlockPageOrderChangedHint(sal_False);

        // if the master page was visible, show it again
        if (!bError && nSelectedPage != SDRPAGE_NOTFOUND)
        {
            if (bOnMaster)
            {
                if (mpViewShell->ISA(DrawViewShell))
                {
                    ::sd::View* pView =
                          static_cast<DrawViewShell*>(mpViewShell)->GetView();
                    sal_uInt16 nPgNum = pSelectedPage->TRG_GetMasterPage().GetPageNum();

                    if (static_cast<DrawViewShell*>(mpViewShell)->GetPageKind() == PK_NOTES)
                        nPgNum++;

                    pView->HideSdrPage();
                    pView->ShowSdrPage(pView->GetModel()->GetMasterPage(nPgNum));
                }

                // force update of TabBar
                mpViewShell->GetViewFrame()->GetDispatcher()->Execute(SID_MASTERPAGE, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD);
            }
            else
            {
                pSelectedPage->SetAutoLayout(pSelectedPage->GetAutoLayout());
            }
        }

        // fake a mode change to repaint the page tab bar
        if( mpViewShell && mpViewShell->ISA( DrawViewShell ) )
        {
            DrawViewShell* pDrawViewSh =
                static_cast<DrawViewShell*>(mpViewShell);
            EditMode eMode = pDrawViewSh->GetEditMode();
            sal_Bool bLayer = pDrawViewSh->IsLayerModeActive();
            pDrawViewSh->ChangeEditMode( eMode, !bLayer );
            pDrawViewSh->ChangeEditMode( eMode, bLayer );
        }

        mpDocSh->SetWaitCursor( sal_False );
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
