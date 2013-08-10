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


#include "ViewClipboard.hxx"

#include "DrawDocShell.hxx"
#include "View.hxx"
#include "ViewShell.hxx"
#include "Window.hxx"

#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "sdxfer.hxx"
#include "sdresid.hxx"
#include "glob.hrc"

#include <svx/svdpagv.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

namespace sd {

ViewClipboard::ViewClipboard (::sd::View& rView)
    : mrView(rView)
{
}




ViewClipboard::~ViewClipboard (void)
{
}




void ViewClipboard::HandlePageDrop (const SdTransferable& rTransferable)
{
    // Determine whether to insert the given set of slides or to assign a
    // given master page.
    SdPage* pMasterPage = GetFirstMasterPage (rTransferable);
    if (pMasterPage != NULL)
        AssignMasterPage (rTransferable, pMasterPage);
    else
        InsertSlides (rTransferable, DetermineInsertPosition (rTransferable));
}




SdPage* ViewClipboard::GetFirstMasterPage (const SdTransferable& rTransferable)
{
    SdPage* pFirstMasterPage = NULL;

    if (rTransferable.HasPageBookmarks())
    {
        do
        {
            const std::vector<OUString> &rBookmarks = rTransferable.GetPageBookmarks();

            if (rBookmarks.empty())
                break;

            DrawDocShell* pDocShell = rTransferable.GetPageDocShell();
            if (pDocShell == NULL)
                break;

            SdDrawDocument* pDocument = pDocShell->GetDoc();
            if (pDocument == NULL)
                break;

            std::vector<OUString>::const_iterator pIter;
            for ( pIter = rBookmarks.begin(); pIter != rBookmarks.end(); ++pIter )
            {
                OUString sName (*pIter);
                sal_Bool bIsMasterPage;

                // SdPage* GetMasterSdPage(sal_uInt16 nPgNum, PageKind ePgKind);
                // sal_uInt16 GetMasterSdPageCount(PageKind ePgKind) const;

                sal_uInt16 nBMPage = pDocument->GetPageByName (
                    sName, bIsMasterPage);
                if ( ! bIsMasterPage)
                {
                    // At least one regular slide: return NULL to indicate
                    // that not all bookmarks point to master pages.
                    pFirstMasterPage = NULL;
                    break;
                }
                else if (pFirstMasterPage == NULL)
                {
                    // Remember the first master page for later.
                    if (nBMPage != SDRPAGE_NOTFOUND)
                        pFirstMasterPage = static_cast<SdPage*>(
                            pDocument->GetMasterPage(nBMPage));
                }
            }
        }
        while (false);
    }

    return pFirstMasterPage;
}




void ViewClipboard::AssignMasterPage (
    const SdTransferable& rTransferable,
    SdPage* pMasterPage)
{
    if (pMasterPage == NULL)
        return;

    // Get the target page to which the master page is assigned.
    SdrPageView* pPageView = mrView.GetSdrPageView();
    if (pPageView == NULL)
        return;

    SdPage* pPage = static_cast<SdPage*>(pPageView->GetPage());
    if (pPage == NULL)
        return;

    SdDrawDocument& rDocument = mrView.GetDoc();

    if ( ! rTransferable.HasPageBookmarks())
        return;

    DrawDocShell* pDataDocShell = rTransferable.GetPageDocShell();
    if (pDataDocShell == NULL)
        return;

    SdDrawDocument* pSourceDocument = pDataDocShell->GetDoc();
    if (pSourceDocument == NULL)
        return;

    // We have to remove the layout suffix from the layout name which is
    // appended again by SetMasterPage() to the given name.  Don't ask.
    OUString sLayoutSuffix = SD_LT_SEPARATOR + SD_RESSTR(STR_LAYOUT_OUTLINE);
    sal_Int32 nLength = sLayoutSuffix.getLength();
    OUString sLayoutName = pMasterPage->GetLayoutName();
    if (sLayoutName.endsWith(sLayoutSuffix))
        sLayoutName = sLayoutName.copy(0, sLayoutName.getLength() - nLength);

    rDocument.SetMasterPage (
        pPage->GetPageNum() / 2,
        sLayoutName,
        pSourceDocument,
        sal_False, // Exchange the master page of only the target page.
        sal_False // Keep unused master pages.
        );
}




sal_uInt16 ViewClipboard::DetermineInsertPosition  (
    const SdTransferable& )
{
    SdDrawDocument& rDoc = mrView.GetDoc();
    sal_uInt16 nPgCnt = rDoc.GetSdPageCount( PK_STANDARD );

    // Insert position is the behind the last selected page or behind the
    // last page when the selection is empty.
    sal_uInt16 nInsertPos = rDoc.GetSdPageCount( PK_STANDARD ) * 2 + 1;
    for( sal_uInt16 nPage = 0; nPage < nPgCnt; nPage++ )
    {
        SdPage* pPage = rDoc.GetSdPage( nPage, PK_STANDARD );

        if( pPage->IsSelected() )
            nInsertPos = nPage * 2 + 3;
    }

    return nInsertPos;
}




sal_uInt16 ViewClipboard::InsertSlides (
    const SdTransferable& rTransferable,
    sal_uInt16 nInsertPosition)
{
    SdDrawDocument& rDoc = mrView.GetDoc();

    sal_uInt16 nInsertPgCnt = 0;
    sal_Bool bMergeMasterPages = !rTransferable.HasSourceDoc( &rDoc );

    // Prepare the insertion.
    const std::vector<OUString> *pBookmarkList = NULL;
    DrawDocShell* pDataDocSh;
    if (rTransferable.HasPageBookmarks())
    {
        // When the transferable contains page bookmarks then the referenced
        // pages are inserted.
        pBookmarkList = &rTransferable.GetPageBookmarks();
        pDataDocSh = rTransferable.GetPageDocShell();
        nInsertPgCnt = (sal_uInt16)pBookmarkList->size();
    }
    else
    {
        // Otherwise all pages of the document of the transferable are
        // inserted.
        SfxObjectShell* pShell = rTransferable.GetDocShell();
        pDataDocSh = (DrawDocShell*) pShell;
        SdDrawDocument* pDataDoc = pDataDocSh->GetDoc();

        if (pDataDoc!=NULL && pDataDoc->GetSdPageCount(PK_STANDARD))
            nInsertPgCnt = pDataDoc->GetSdPageCount(PK_STANDARD);
    }
    if (nInsertPgCnt > 0)
    {
        const SolarMutexGuard aGuard;
        ::sd::Window* pWin = mrView.GetViewShell()->GetActiveWindow();
        const sal_Bool bWait = pWin && pWin->IsWait();

        if( bWait )
            pWin->LeaveWait();

        rDoc.InsertBookmarkAsPage(
            pBookmarkList ? *pBookmarkList : std::vector<OUString>(),
            NULL,
            sal_False,
            sal_False,
            nInsertPosition,
            (&rTransferable == SD_MOD()->pTransferDrag),
            pDataDocSh,
            sal_True,
            bMergeMasterPages,
            sal_False);

        if( bWait )
            pWin->EnterWait();
    }

    return nInsertPgCnt;
}


} // end of namespace ::sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
