/*************************************************************************
 *
 *  $RCSfile: ViewClipboard.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:52:24 $
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
            const List* pBookmarks = &rTransferable.GetPageBookmarks();
            if (pBookmarks == NULL)
                break;

            DrawDocShell* pDocShell = rTransferable.GetPageDocShell();
            if (pDocShell == NULL)
                break;

            SdDrawDocument* pDocument = pDocShell->GetDoc();
            if (pDocument == NULL)
                break;

            if (pBookmarks->Count() <= 0)
                break;

            int nBookmarkCount = pBookmarks->Count();
            for (int nIndex=0; nIndex<nBookmarkCount; nIndex++)
            {
                String sName (*(String*) pBookmarks->GetObject(nIndex));
                BOOL bIsMasterPage;

                // SdPage* GetMasterSdPage(USHORT nPgNum, PageKind ePgKind);
                // USHORT GetMasterSdPageCount(PageKind ePgKind) const;

                USHORT nBMPage = pDocument->GetPageByName (
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
    do
    {
        if (pMasterPage == NULL)
            return;

        // Get the target page to which the master page is assigned.
        SdrPageView* pPageView = mrView.GetPageViewPvNum(0);
        if (pPageView == NULL)
            break;

        SdPage* pPage = static_cast<SdPage*>(pPageView->GetPage());
        if (pPage == NULL)
            break;

        SdDrawDocument* pDocument = mrView.GetDoc();
        if (pDocument == NULL)
            break;

        if ( ! rTransferable.HasPageBookmarks())
            break;

        DrawDocShell* pDataDocShell = rTransferable.GetPageDocShell();
        if (pDataDocShell == NULL)
            break;

        SdDrawDocument* pSourceDocument = pDataDocShell->GetDoc();
        if (pSourceDocument == NULL)
            break;

        // We have to remove the layout suffix from the layout name which is
        // appended again by SetMasterPage() to the given name.  Don't ask.
        String sLayoutSuffix (RTL_CONSTASCII_STRINGPARAM(SD_LT_SEPARATOR));
        sLayoutSuffix.Append (SdResId(STR_LAYOUT_OUTLINE));
        int nLength = sLayoutSuffix.Len();
        String sLayoutName (pMasterPage->GetLayoutName());
        if (String(sLayoutName, sLayoutName.Len()-nLength, nLength).Equals (
            sLayoutSuffix))
            sLayoutName = String(sLayoutName, 0, sLayoutName.Len()-nLength);

        pDocument->SetMasterPage (
            pPage->GetPageNum() / 2,
            sLayoutName,
            pSourceDocument,
            FALSE, // Exchange the master page of only the target page.
            FALSE // Keep unused master pages.
            );
    }
    while (false);
}




USHORT ViewClipboard::DetermineInsertPosition  (
    const SdTransferable& rTransferable)
{
    SdDrawDocument* pDoc = mrView.GetDoc();
    USHORT nPgCnt = pDoc->GetSdPageCount( PK_STANDARD );

    // Insert position is the behind the last selected page or behind the
    // last page when the selection is empty.
    USHORT nInsertPos = pDoc->GetSdPageCount( PK_STANDARD ) * 2 + 1;
    for( USHORT nPage = 0; nPage < nPgCnt; nPage++ )
    {
        SdPage* pPage = pDoc->GetSdPage( nPage, PK_STANDARD );

        if( pPage->IsSelected() )
            nInsertPos = nPage * 2 + 3;
    }

    return nInsertPos;
}




USHORT ViewClipboard::InsertSlides (
    const SdTransferable& rTransferable,
    USHORT nInsertPosition)
{
    SdDrawDocument* pDoc = mrView.GetDoc();

    USHORT nInsertPgCnt = 0;
    USHORT nPgCnt = pDoc->GetSdPageCount( PK_STANDARD );
    BOOL bMergeMasterPages = !rTransferable.HasSourceDoc( pDoc );

    // Prepare the insertion.
    const List* pBookmarkList;
    DrawDocShell* pDataDocSh;
    if (rTransferable.HasPageBookmarks())
    {
        // When the transferable contains page bookmarks then the referenced
        // pages are inserted.
        pBookmarkList = &rTransferable.GetPageBookmarks();
        pDataDocSh = rTransferable.GetPageDocShell();
        nInsertPgCnt = (USHORT)pBookmarkList->Count();
    }
    else
    {
        // Otherwise all pages of the document of the transferable are
        // inserted.
        SvEmbeddedObject* pObj = rTransferable.GetDocShell();
        pDataDocSh = (DrawDocShell*) pObj;
        SdDrawDocument* pDataDoc = pDataDocSh->GetDoc();
        pBookmarkList = NULL;
        if (pDataDoc!=NULL && pDataDoc->GetSdPageCount(PK_STANDARD))
            nInsertPgCnt = pDataDoc->GetSdPageCount(PK_STANDARD);
    }
    if (nInsertPgCnt > 0)
    {
        const ::vos::OGuard aGuard( Application::GetSolarMutex() );
        ::sd::Window* pWin = mrView.GetViewShell()->GetActiveWindow();
        const BOOL bWait = pWin && pWin->IsWait();

        if( bWait )
            pWin->LeaveWait();

        pDoc->InsertBookmarkAsPage(
            const_cast<List*>(pBookmarkList),
            NULL,
            FALSE,
            FALSE,
            nInsertPosition,
            (&rTransferable == SD_MOD()->pTransferDrag),
            pDataDocSh,
            TRUE,
            bMergeMasterPages,
            FALSE);

        if( bWait )
            pWin->EnterWait();
    }

    return nInsertPgCnt;
}


} // end of namespace ::sd
