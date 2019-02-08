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

#include "DocumentHelper.hxx"

#include <drawdoc.hxx>
#include <DrawDocShell.hxx>
#include <sdpage.hxx>
#include <glob.hxx>
#include <unmovss.hxx>
#include <strings.hrc>
#include <sdresid.hxx>
#include <undoback.hxx>
#include <ViewShell.hxx>
#include <ViewShellBase.hxx>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <stlpool.hxx>
#include <svx/xfillit0.hxx>
#include <svx/svdundo.hxx>
#include <tools/diagnose_ex.h>
#include <xmloff/autolayout.hxx>
#include <sal/log.hxx>

using namespace ::com::sun::star;

namespace sd { namespace sidebar {

SdPage* DocumentHelper::CopyMasterPageToLocalDocument (
    SdDrawDocument& rTargetDocument,
    SdPage* pMasterPage)
{
    SdPage* pNewMasterPage = nullptr;

    do
    {
        if (pMasterPage == nullptr)
            break;

        // Check the presence of the source document.
        SdDrawDocument& rSourceDocument(static_cast< SdDrawDocument& >(pMasterPage->getSdrModelFromSdrPage()));

        // When the given master page already belongs to the target document
        // then there is nothing more to do.
        if (&rSourceDocument == &rTargetDocument)
        {
            pNewMasterPage = pMasterPage;
            break;
        }

        // Test if the master pages of both the slide and its notes page are
        // present.  This is not the case when we are called during the
        // creation of the slide master page because then the notes master
        // page is not there.
        sal_uInt16 nSourceMasterPageCount = rSourceDocument.GetMasterPageCount();
        if (nSourceMasterPageCount%2 == 0)
            // There should be 1 handout page + n slide masters + n notes
            // masters = 2*n+1.  An even value indicates that a new slide
            // master but not yet the notes master has been inserted.
            break;
        sal_uInt16 nIndex = pMasterPage->GetPageNum();
        if (nSourceMasterPageCount <= nIndex+1)
            break;
        // Get the slide master page.
        if (pMasterPage != static_cast<SdPage*>(
            rSourceDocument.GetMasterPage(nIndex)))
            break;
        // Get the notes master page.
        SdPage* pNotesMasterPage = static_cast<SdPage*>(
            rSourceDocument.GetMasterPage(nIndex+1));
        if (pNotesMasterPage == nullptr)
            break;

        // Check if a master page with the same name as that of the given
        // master page already exists.
        bool bPageExists (false);
        sal_uInt16 nMasterPageCount(rTargetDocument.GetMasterSdPageCount(PageKind::Standard));
        for (sal_uInt16 nMaster=0; nMaster<nMasterPageCount; nMaster++)
        {
            SdPage* pCandidate = rTargetDocument.GetMasterSdPage (nMaster, PageKind::Standard);
            if (pCandidate->GetName() == pMasterPage->GetName())
            {
                bPageExists = true;
                pNewMasterPage = pCandidate;
                break;
            }
        }
        if (bPageExists)
            break;

        // Create a new slide (and its notes page.)
        uno::Reference<drawing::XDrawPagesSupplier> xSlideSupplier (
            rTargetDocument.getUnoModel(), uno::UNO_QUERY);
        if ( ! xSlideSupplier.is())
            break;
        uno::Reference<drawing::XDrawPages> xSlides (
            xSlideSupplier->getDrawPages(), uno::UNO_QUERY);
        if ( ! xSlides.is())
            break;
        xSlides->insertNewByIndex (xSlides->getCount());

        // Set a layout.
        SdPage* pSlide = rTargetDocument.GetSdPage(
            rTargetDocument.GetSdPageCount(PageKind::Standard)-1,
            PageKind::Standard);
        if (pSlide == nullptr)
            break;
        pSlide->SetAutoLayout(AUTOLAYOUT_TITLE, true);

        // Create a copy of the master page and the associated notes
        // master page and insert them into our document.
        pNewMasterPage = AddMasterPage(rTargetDocument, pMasterPage);
        if (pNewMasterPage==nullptr)
            break;
        SdPage* pNewNotesMasterPage
            = AddMasterPage(rTargetDocument, pNotesMasterPage);
        if (pNewNotesMasterPage==nullptr)
            break;

        // Make the connection from the new slide to the master page
        // (and do the same for the notes page.)
        rTargetDocument.SetMasterPage (
            rTargetDocument.GetSdPageCount(PageKind::Standard)-1,
            pNewMasterPage->GetName(),
            &rTargetDocument,
            false, // Connect the new master page with the new slide but
                   // do not modify other (master) pages.
            true);
    }
    while (false);

    // We are not interested in any automatisms for our modified internal
    // document.
    rTargetDocument.SetChanged(false);

    return pNewMasterPage;
}

SdPage* DocumentHelper::GetSlideForMasterPage (SdPage const * pMasterPage)
{
    SdPage* pCandidate = nullptr;

    SdDrawDocument* pDocument = nullptr;
    if (pMasterPage != nullptr)
        pDocument = dynamic_cast< SdDrawDocument* >(&pMasterPage->getSdrModelFromSdrPage());

    // Iterate over all pages and check if it references the given master
    // page.
    if (pDocument!=nullptr && pDocument->GetSdPageCount(PageKind::Standard) > 0)
    {
        // In most cases a new slide has just been inserted so start with
        // the last page.
        sal_uInt16 nPageIndex (pDocument->GetSdPageCount(PageKind::Standard)-1);
        bool bFound (false);
        while ( ! bFound)
        {
            pCandidate = pDocument->GetSdPage(
                nPageIndex,
                PageKind::Standard);
            if (pCandidate != nullptr)
            {
                if (static_cast<SdPage*>(&pCandidate->TRG_GetMasterPage())
                    == pMasterPage)
                {
                    bFound = true;
                    break;
                }
            }

            if (nPageIndex == 0)
                break;
            else
                nPageIndex --;
        }

        // If no page was found, that referenced the given master page, reset
        // the pointer that is returned.
        if ( ! bFound)
            pCandidate = nullptr;
    }

    return pCandidate;
}

SdPage* DocumentHelper::AddMasterPage (
    SdDrawDocument& rTargetDocument,
    SdPage const * pMasterPage)
{
    SdPage* pClonedMasterPage = nullptr;

    if (pMasterPage!=nullptr)
    {
        try
        {
            // Duplicate the master page.
            pClonedMasterPage = static_cast<SdPage*>(pMasterPage->CloneSdrPage(rTargetDocument));

            // Copy the necessary styles.
            SdDrawDocument& rSourceDocument(static_cast< SdDrawDocument& >(pMasterPage->getSdrModelFromSdrPage()));
            ProvideStyles(rSourceDocument, rTargetDocument, pClonedMasterPage);

            // Copy the precious flag.
            pClonedMasterPage->SetPrecious(pMasterPage->IsPrecious());

            // Now that the styles are available we can insert the cloned
            // master page.
            rTargetDocument.InsertMasterPage (pClonedMasterPage);
        }
        catch(const uno::Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("sd");
            pClonedMasterPage = nullptr;
        }
        catch(const ::std::exception& e)
        {
            pClonedMasterPage = nullptr;
            SAL_WARN("sd", "caught general exception " << e.what());
        }
        catch(...)
        {
            pClonedMasterPage = nullptr;
            SAL_WARN("sd", "caught general exception");
        }
    }

    return pClonedMasterPage;
}

void DocumentHelper::ProvideStyles (
    SdDrawDocument const & rSourceDocument,
    SdDrawDocument& rTargetDocument,
    SdPage const * pPage)
{
    // Get the layout name of the given page.
    OUString sLayoutName (pPage->GetLayoutName());
    sal_Int32 nIndex = sLayoutName.indexOf(SD_LT_SEPARATOR);
    if( nIndex != -1 )
        sLayoutName = sLayoutName.copy(0, nIndex);

    // Copy the style sheet from source to target document.
    SdStyleSheetPool* pSourceStyleSheetPool =
        static_cast<SdStyleSheetPool*>(rSourceDocument.GetStyleSheetPool());
    SdStyleSheetPool* pTargetStyleSheetPool =
        static_cast<SdStyleSheetPool*>(rTargetDocument.GetStyleSheetPool());
    StyleSheetCopyResultVector aCreatedStyles;
    pTargetStyleSheetPool->CopyLayoutSheets (
        sLayoutName,
        *pSourceStyleSheetPool,
        aCreatedStyles);

    // Add an undo action for the copied style sheets.
    if( !aCreatedStyles.empty() )
    {
        SfxUndoManager* pUndoManager = rTargetDocument.GetDocSh()->GetUndoManager();
        if (pUndoManager != nullptr)
        {
            pUndoManager->AddUndoAction (
               std::make_unique<SdMoveStyleSheetsUndoAction>(
                   &rTargetDocument,
                   aCreatedStyles,
                   true));
        }
    }
}

void DocumentHelper::AssignMasterPageToPageList (
    SdDrawDocument& rTargetDocument,
    SdPage* pMasterPage,
    const std::shared_ptr<std::vector<SdPage*> >& rpPageList)
{
    if (pMasterPage == nullptr || !pMasterPage->IsMasterPage())
        return;

    // Make the layout name by stripping ouf the layout postfix from the
    // layout name of the given master page.
    OUString sFullLayoutName(pMasterPage->GetLayoutName());
    OUString sBaseLayoutName (sFullLayoutName);
    sal_Int32 nIndex = sBaseLayoutName.indexOf(SD_LT_SEPARATOR);
    if( nIndex != -1 )
        sBaseLayoutName = sBaseLayoutName.copy(0, nIndex);

    if (rpPageList->empty())
        return;

    // Create a second list that contains only the valid pointers to
    // pages for which an assignment is necessary.
    ::std::vector<SdPage*> aCleanedList;
    for (const auto& rpPage : *rpPageList)
    {
        OSL_ASSERT(rpPage!=nullptr && &rpPage->getSdrModelFromSdrPage() == &rTargetDocument);
        if (rpPage != nullptr && rpPage->GetLayoutName() != sFullLayoutName)
        {
            aCleanedList.push_back(rpPage);
        }
    }
    if (aCleanedList.empty() )
        return;

    SfxUndoManager* pUndoMgr = rTargetDocument.GetDocSh()->GetUndoManager();
    if( pUndoMgr )
        pUndoMgr->EnterListAction(SdResId(STR_UNDO_SET_PRESLAYOUT), OUString(), 0, rTargetDocument.GetDocSh()->GetViewShell()->GetViewShellBase().GetViewShellId());

    SdPage* pMasterPageInDocument = ProvideMasterPage(rTargetDocument,pMasterPage,rpPageList);
    if (pMasterPageInDocument == nullptr)
        return;

    // Assign the master pages to the given list of pages.
    for (const auto& rpPage : aCleanedList)
    {
        AssignMasterPageToPage (
            pMasterPageInDocument,
            sBaseLayoutName,
            rpPage);
    }

    if( pUndoMgr )
        pUndoMgr->LeaveListAction();
}

SdPage* DocumentHelper::AddMasterPage (
    SdDrawDocument& rTargetDocument,
    SdPage const * pMasterPage,
    sal_uInt16 nInsertionIndex)
{
    SdPage* pClonedMasterPage = nullptr;

    if (pMasterPage!=nullptr)
    {
        // Duplicate the master page.
        pClonedMasterPage = static_cast<SdPage*>(pMasterPage->CloneSdrPage(rTargetDocument));

        // Copy the precious flag.
        pClonedMasterPage->SetPrecious(pMasterPage->IsPrecious());

        // Copy the necessary styles.
        SdDrawDocument& rSourceDocument(static_cast< SdDrawDocument& >(pMasterPage->getSdrModelFromSdrPage()));
        ProvideStyles(rSourceDocument, rTargetDocument, pClonedMasterPage);

        // Now that the styles are available we can insert the cloned
        // master page.
        rTargetDocument.InsertMasterPage (pClonedMasterPage, nInsertionIndex);

        // Adapt the size of the new master page to that of the pages in
        // the document.
        Size aNewSize (rTargetDocument.GetSdPage(0, pMasterPage->GetPageKind())->GetSize());
        ::tools::Rectangle aBorders (
            pClonedMasterPage->GetLeftBorder(),
            pClonedMasterPage->GetUpperBorder(),
            pClonedMasterPage->GetRightBorder(),
            pClonedMasterPage->GetLowerBorder());
        pClonedMasterPage->ScaleObjects(aNewSize, aBorders, true);
        pClonedMasterPage->SetSize(aNewSize);
        pClonedMasterPage->CreateTitleAndLayout(true);
    }

    return pClonedMasterPage;
}

/** In here we have to handle three cases:
    1. pPage is a normal slide.  We can use SetMasterPage to assign the
    master pages to it.
    2. pPage is a master page that is used by at least one slide.  We can
    assign the master page to these slides.
    3. pPage is a master page that is currently not used by any slide.
    We can delete that page and add copies of the given master pages
    instead.

    For points 2 and 3 where one master page A is assigned to another B we have
    to keep in mind that the master page that page A has already been
    inserted into the target document.
*/
void DocumentHelper::AssignMasterPageToPage (
    SdPage const * pMasterPage,
    const OUString& rsBaseLayoutName,
    SdPage* pPage)
{
    // Leave early when the parameters are invalid.
    if (pPage == nullptr || pMasterPage == nullptr)
        return;

    SdDrawDocument& rDocument(dynamic_cast< SdDrawDocument& >(pPage->getSdrModelFromSdrPage()));

    if ( ! pPage->IsMasterPage())
    {
        // 1. Remove the background object (so that, if it exists, does
        // not override the new master page) and assign the master page to
        // the regular slide.
        rDocument.GetDocSh()->GetUndoManager()->AddUndoAction(
            std::make_unique<SdBackgroundObjUndoAction>(
                rDocument, *pPage, pPage->getSdrPageProperties().GetItemSet()),
                true);
        pPage->getSdrPageProperties().PutItem(XFillStyleItem(drawing::FillStyle_NONE));

        rDocument.SetMasterPage (
            (pPage->GetPageNum()-1)/2,
            rsBaseLayoutName,
            &rDocument,
            false,
            false);
    }
    else
    {
        // Find first slide that uses the master page.
        SdPage* pSlide = nullptr;
        sal_uInt16 nPageCount = rDocument.GetSdPageCount(PageKind::Standard);
        for (sal_uInt16 nPage=0; nPage<nPageCount&&pSlide==nullptr; nPage++)
        {
            SdrPage* pCandidate = rDocument.GetSdPage(nPage,PageKind::Standard);
            if (pCandidate != nullptr
                && pCandidate->TRG_HasMasterPage()
                && &(pCandidate->TRG_GetMasterPage()) == pPage)
            {
                pSlide = static_cast<SdPage*>(pCandidate);
            }
        }

        if (pSlide != nullptr)
        {
            // 2. Assign the given master pages to the first slide that was
            // found above that uses the master page.
            rDocument.SetMasterPage (
                (pSlide->GetPageNum()-1)/2,
                rsBaseLayoutName,
                &rDocument,
                false,
                false);
        }
        else
        {
            // 3. Replace the master page A by a copy of the given master
            // page B.
            rDocument.RemoveUnnecessaryMasterPages (
                pPage);
        }
    }
}

SdPage* DocumentHelper::ProvideMasterPage (
    SdDrawDocument& rTargetDocument,
    SdPage* pMasterPage,
    const std::shared_ptr<std::vector<SdPage*> >& rpPageList)
{
    // Make sure that both the master page and its notes master exist
    // in the source document.  If one is missing then return without
    // making any changes.
    if (pMasterPage == nullptr)
    {
        // The caller should make sure that the master page is valid.
        OSL_ASSERT(pMasterPage != nullptr);
        return nullptr;
    }
    SdDrawDocument& rSourceDocument(static_cast< SdDrawDocument& >(pMasterPage->getSdrModelFromSdrPage()));
    SdPage* pNotesMasterPage = static_cast<SdPage*>(
        rSourceDocument.GetMasterPage(pMasterPage->GetPageNum()+1));
    if (pNotesMasterPage == nullptr)
    {
        // The model is not in a valid state.  Maybe a new master page
        // is being (not finished yet) created?  Return without making
        // any changes.
        return nullptr;
    }

    SdPage* pMasterPageInDocument = nullptr;
    // Search for a master page with the same name as the given one in
    // the target document.
    const OUString sMasterPageLayoutName (pMasterPage->GetLayoutName());
    for (sal_uInt16 nIndex=0,nCount=rTargetDocument.GetMasterPageCount(); nIndex<nCount; ++nIndex)
    {
        SdPage* pCandidate = static_cast<SdPage*>(rTargetDocument.GetMasterPage(nIndex));
        if (pCandidate && sMasterPageLayoutName == pCandidate->GetLayoutName())
        {
            // The requested master page does already exist in the
            // target document, return it.
            return pCandidate;
        }
    }

    // The given master page does not already belong to the target
    // document so we have to create copies and insert them into the
    // target document.

    // Determine the position where the new master pages are inserted.
    // By default they are inserted at the end.  When we assign to a
    // master page then insert after the last of the (selected) pages.
    sal_uInt16 nInsertionIndex = rTargetDocument.GetMasterPageCount();
    if (rpPageList->front()->IsMasterPage())
    {
        nInsertionIndex = rpPageList->back()->GetPageNum();
    }

    // Clone the master page.
    if (&pMasterPage->getSdrModelFromSdrPage() != &rTargetDocument)
    {
        pMasterPageInDocument = AddMasterPage (rTargetDocument, pMasterPage, nInsertionIndex);
        if( rTargetDocument.IsUndoEnabled() )
                rTargetDocument.AddUndo(
                    rTargetDocument.GetSdrUndoFactory().CreateUndoNewPage(*pMasterPageInDocument));
    }
    else
        pMasterPageInDocument = pMasterPage;

    // Clone the notes master.
    if (&pNotesMasterPage->getSdrModelFromSdrPage() != &rTargetDocument)
    {
        SdPage* pClonedNotesMasterPage
            = AddMasterPage (rTargetDocument, pNotesMasterPage, nInsertionIndex+1);
        if( rTargetDocument.IsUndoEnabled() )
            rTargetDocument.AddUndo(
                rTargetDocument.GetSdrUndoFactory().CreateUndoNewPage(*pClonedNotesMasterPage));
    }

    return pMasterPageInDocument;
}

} } // end of namespace sd::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
