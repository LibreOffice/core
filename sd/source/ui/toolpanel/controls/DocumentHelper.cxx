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

#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "sdpage.hxx"
#include "glob.hxx"
#include "unmovss.hxx"
#include "strings.hrc"
#include "sdresid.hxx"
#include "undoback.hxx"
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include "stlpool.hxx"
#include <svx/xfillit0.hxx>
#include <tools/diagnose_ex.h>

using namespace ::com::sun::star;

namespace sd { namespace toolpanel { namespace controls {

SdPage* DocumentHelper::CopyMasterPageToLocalDocument (
    SdDrawDocument& rTargetDocument,
    SdPage* pMasterPage)
{
    SdPage* pNewMasterPage = NULL;

    do
    {
        if (pMasterPage == NULL)
            break;

        // Check the presence of the source document.
        SdDrawDocument* pSourceDocument = static_cast<SdDrawDocument*>(
            pMasterPage->GetModel());
        if (pSourceDocument == NULL)
            break;

        // When the given master page already belongs to the target document
        // then there is nothing more to do.
        if (pSourceDocument == &rTargetDocument)
        {
            pNewMasterPage = pMasterPage;
            break;
        }

        // Test if the master pages of both the slide and its notes page are
        // present.  This is not the case when we are called during the
        // creation of the slide master page because then the notes master
        // page is not there.
        sal_uInt16 nSourceMasterPageCount = pSourceDocument->GetMasterPageCount();
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
            pSourceDocument->GetMasterPage(nIndex)))
            break;
        // Get the notes master page.
        SdPage* pNotesMasterPage = static_cast<SdPage*>(
            pSourceDocument->GetMasterPage(nIndex+1));
        if (pNotesMasterPage == NULL)
            break;


        // Check if a master page with the same name as that of the given
        // master page already exists.
        bool bPageExists (false);
        sal_uInt16 nMasterPageCount(rTargetDocument.GetMasterSdPageCount(PK_STANDARD));
        for (sal_uInt16 nMaster=0; nMaster<nMasterPageCount; nMaster++)
        {
            SdPage* pCandidate = static_cast<SdPage*>(
                rTargetDocument.GetMasterSdPage (nMaster, PK_STANDARD));
            if (pMasterPage!=NULL
                && pCandidate->GetName().CompareTo(pMasterPage->GetName())==0)
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
            rTargetDocument.GetSdPageCount(PK_STANDARD)-1,
            PK_STANDARD);
        if (pSlide == NULL)
            break;
        pSlide->SetAutoLayout(AUTOLAYOUT_TITLE, sal_True);

        // Create a copy of the master page and the associated notes
        // master page and insert them into our document.
        pNewMasterPage = AddMasterPage(rTargetDocument, pMasterPage);
        if (pNewMasterPage==NULL)
            break;
        SdPage* pNewNotesMasterPage
            = AddMasterPage(rTargetDocument, pNotesMasterPage);
        if (pNewNotesMasterPage==NULL)
            break;

        // Make the connection from the new slide to the master page
        // (and do the same for the notes page.)
        rTargetDocument.SetMasterPage (
            rTargetDocument.GetSdPageCount(PK_STANDARD)-1,
            pNewMasterPage->GetName(),
            &rTargetDocument,
            sal_False, // Connect the new master page with the new slide but
                   // do not modify other (master) pages.
            sal_True);
    }
    while (false);

    // We are not interested in any automatisms for our modified internal
    // document.
    rTargetDocument.SetChanged (sal_False);

    return pNewMasterPage;
}




SdPage* DocumentHelper::GetSlideForMasterPage (SdPage* pMasterPage)
{
    SdPage* pCandidate = NULL;

    SdDrawDocument* pDocument = NULL;
    if (pMasterPage != NULL)
        pDocument = dynamic_cast<SdDrawDocument*>(pMasterPage->GetModel());

    // Iterate over all pages and check if it references the given master
    // page.
    if (pDocument!=NULL && pDocument->GetSdPageCount(PK_STANDARD) > 0)
    {
        // In most cases a new slide has just been inserted so start with
        // the last page.
        sal_uInt16 nPageIndex (pDocument->GetSdPageCount(PK_STANDARD)-1);
        bool bFound (false);
        while ( ! bFound)
        {
            pCandidate = pDocument->GetSdPage(
                nPageIndex,
                PK_STANDARD);
            if (pCandidate != NULL)
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

        // If no page was found that refernced the given master page reset
        // the pointer that is returned.
        if ( ! bFound)
            pCandidate = NULL;
    }

    return pCandidate;
}




SdPage* DocumentHelper::AddMasterPage (
    SdDrawDocument& rTargetDocument,
    SdPage* pMasterPage)
{
    SdPage* pClonedMasterPage = NULL;

    if (pMasterPage!=NULL)
    {
        try
        {
            // Duplicate the master page.
            pClonedMasterPage = static_cast<SdPage*>(pMasterPage->Clone());

            // Copy the necessary styles.
            SdDrawDocument* pSourceDocument
                = static_cast<SdDrawDocument*>(pMasterPage->GetModel());
            if (pSourceDocument != NULL)
                ProvideStyles (*pSourceDocument, rTargetDocument, pClonedMasterPage);

            // Copy the precious flag.
            pClonedMasterPage->SetPrecious(pMasterPage->IsPrecious());

            // Now that the styles are available we can insert the cloned
            // master page.
            rTargetDocument.InsertMasterPage (pClonedMasterPage);
        }
        catch(const uno::Exception&)
        {
            pClonedMasterPage = NULL;
            DBG_UNHANDLED_EXCEPTION();
        }
        catch(const ::std::exception&)
        {
            pClonedMasterPage = NULL;
            OSL_TRACE ("caught general exception");
        }
        catch(...)
        {
            pClonedMasterPage = NULL;
            OSL_TRACE ("caught general exception");
        }
    }

    return pClonedMasterPage;
}




void DocumentHelper::ProvideStyles (
    SdDrawDocument& rSourceDocument,
    SdDrawDocument& rTargetDocument,
    SdPage* pPage)
{
    // Get the layout name of the given page.
    String sLayoutName (pPage->GetLayoutName());
    sLayoutName.Erase (sLayoutName.SearchAscii (SD_LT_SEPARATOR));

    // Copy the style sheet from source to target document.
    SdStyleSheetPool* pSourceStyleSheetPool =
        static_cast<SdStyleSheetPool*>(rSourceDocument.GetStyleSheetPool());
    SdStyleSheetPool* pTargetStyleSheetPool =
        static_cast<SdStyleSheetPool*>(rTargetDocument.GetStyleSheetPool());
    SdStyleSheetVector aCreatedStyles;
    pTargetStyleSheetPool->CopyLayoutSheets (
        sLayoutName,
        *pSourceStyleSheetPool,
        aCreatedStyles);

    // Add an undo action for the copied style sheets.
    if( !aCreatedStyles.empty() )
    {
         ::svl::IUndoManager* pUndoManager = rTargetDocument.GetDocSh()->GetUndoManager();
       if (pUndoManager != NULL)
       {
           SdMoveStyleSheetsUndoAction* pMovStyles =
               new SdMoveStyleSheetsUndoAction (
                   &rTargetDocument,
                   aCreatedStyles,
                   sal_True);
           pUndoManager->AddUndoAction (pMovStyles);
       }
    }
}




void DocumentHelper::AssignMasterPageToPageList (
    SdDrawDocument& rTargetDocument,
    SdPage* pMasterPage,
    const ::boost::shared_ptr<std::vector<SdPage*> >& rpPageList)
{
    if (pMasterPage == NULL || !pMasterPage->IsMasterPage())
        return;

    // Make the layout name by stripping ouf the layout postfix from the
    // layout name of the given master page.
    OUString sFullLayoutName(pMasterPage->GetLayoutName());
    String sBaseLayoutName (sFullLayoutName);
    sBaseLayoutName.Erase (sBaseLayoutName.SearchAscii (SD_LT_SEPARATOR));

    if (rpPageList->empty())
        return;

    // Create a second list that contains only the valid pointers to
    // pages for which an assignment is necessary.
    ::std::vector<SdPage*>::const_iterator iPage;
    ::std::vector<SdPage*> aCleanedList;
    for (iPage=rpPageList->begin(); iPage!=rpPageList->end(); ++iPage)
    {
        OSL_ASSERT(*iPage!=NULL && (*iPage)->GetModel() == &rTargetDocument);
        if (*iPage != NULL && (*iPage)->GetLayoutName() != sFullLayoutName)
        {
            aCleanedList.push_back(*iPage);
        }
    }
        if (aCleanedList.empty() )
        return;

    ::svl::IUndoManager* pUndoMgr = rTargetDocument.GetDocSh()->GetUndoManager();
    if( pUndoMgr )
        pUndoMgr->EnterListAction(SD_RESSTR(STR_UNDO_SET_PRESLAYOUT), String());

    SdPage* pMasterPageInDocument = ProvideMasterPage(rTargetDocument,pMasterPage,rpPageList);
    if (pMasterPageInDocument == NULL)
        return;

    // Assign the master pages to the given list of pages.
    for (iPage=aCleanedList.begin();
            iPage!=aCleanedList.end();
            ++iPage)
    {
        AssignMasterPageToPage (
            pMasterPageInDocument,
            sBaseLayoutName,
            *iPage);
    }

    if( pUndoMgr )
        pUndoMgr->LeaveListAction();
}




SdPage* DocumentHelper::AddMasterPage (
    SdDrawDocument& rTargetDocument,
    SdPage* pMasterPage,
    sal_uInt16 nInsertionIndex)
{
    SdPage* pClonedMasterPage = NULL;

    if (pMasterPage!=NULL)
    {
        // Duplicate the master page.
        pClonedMasterPage = static_cast<SdPage*>(pMasterPage->Clone());

        // Copy the precious flag.
        pClonedMasterPage->SetPrecious(pMasterPage->IsPrecious());

        // Copy the necessary styles.
        SdDrawDocument* pSourceDocument
            = static_cast<SdDrawDocument*>(pMasterPage->GetModel());
        if (pSourceDocument != NULL)
        {
            ProvideStyles (*pSourceDocument, rTargetDocument, pClonedMasterPage);

            // Now that the styles are available we can insert the cloned
            // master page.
            rTargetDocument.InsertMasterPage (pClonedMasterPage, nInsertionIndex);

            // Adapt the size of the new master page to that of the pages in
            // the document.
            Size aNewSize (rTargetDocument.GetSdPage(0, pMasterPage->GetPageKind())->GetSize());
            Rectangle aBorders (
                pClonedMasterPage->GetLftBorder(),
                pClonedMasterPage->GetUppBorder(),
                pClonedMasterPage->GetRgtBorder(),
                pClonedMasterPage->GetLwrBorder());
            pClonedMasterPage->ScaleObjects(aNewSize, aBorders, sal_True);
            pClonedMasterPage->SetSize(aNewSize);
            pClonedMasterPage->CreateTitleAndLayout(sal_True);
        }
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
    SdPage* pMasterPage,
    const String& rsBaseLayoutName,
    SdPage* pPage)
{
    // Leave early when the parameters are invalid.
    if (pPage == NULL || pMasterPage == NULL)
        return;
    SdDrawDocument* pDocument = dynamic_cast<SdDrawDocument*>(pPage->GetModel());
    if (pDocument == NULL)
        return;

    if ( ! pPage->IsMasterPage())
    {
        // 1. Remove the background object (so that that, if it exists, does
        // not override the new master page) and assign the master page to
        // the regular slide.
        pDocument->GetDocSh()->GetUndoManager()->AddUndoAction(
            new SdBackgroundObjUndoAction(
                *pDocument, *pPage, pPage->getSdrPageProperties().GetItemSet()),
            sal_True);
        pPage->getSdrPageProperties().PutItem(XFillStyleItem(XFILL_NONE));

        pDocument->SetMasterPage (
            (pPage->GetPageNum()-1)/2,
            rsBaseLayoutName,
            pDocument,
            sal_False,
            sal_False);
    }
    else
    {
        // Find first slide that uses the master page.
        SdPage* pSlide = NULL;
        sal_uInt16 nPageCount = pDocument->GetSdPageCount(PK_STANDARD);
        for (sal_uInt16 nPage=0; nPage<nPageCount&&pSlide==NULL; nPage++)
        {
            SdrPage* pCandidate = pDocument->GetSdPage(nPage,PK_STANDARD);
            if (pCandidate != NULL
                && pCandidate->TRG_HasMasterPage()
                && &(pCandidate->TRG_GetMasterPage()) == pPage)
            {
                pSlide = static_cast<SdPage*>(pCandidate);
            }
        }

        if (pSlide != NULL)
        {
            // 2. Assign the given master pages to the first slide that was
            // found above that uses the master page.
            pDocument->SetMasterPage (
                (pSlide->GetPageNum()-1)/2,
                rsBaseLayoutName,
                pDocument,
                sal_False,
                sal_False);
        }
        else
        {
            // 3. Replace the master page A by a copy of the given master
            // page B.
            pDocument->RemoveUnnecessaryMasterPages (
                pPage, sal_False);
        }
    }
}




SdPage* DocumentHelper::ProvideMasterPage (
    SdDrawDocument& rTargetDocument,
    SdPage* pMasterPage,
    const ::boost::shared_ptr<std::vector<SdPage*> >& rpPageList)
{
    // Make sure that both the master page and its notes master exist
    // in the source document.  If one is missing then return without
    // making any changes.
    if (pMasterPage == NULL)
    {
        // The caller should make sure that the master page is valid.
        OSL_ASSERT(pMasterPage != NULL);
        return NULL;
    }
    SdDrawDocument* pSourceDocument = static_cast<SdDrawDocument*>(pMasterPage->GetModel());
    if (pSourceDocument == NULL)
        return NULL;
    SdPage* pNotesMasterPage = static_cast<SdPage*>(
        pSourceDocument->GetMasterPage(pMasterPage->GetPageNum()+1));
    if (pNotesMasterPage == NULL)
    {
        // The model is not in a valid state.  Maybe a new master page
        // is being (not finished yet) created?  Return without making
        // any changes.
        return NULL;
    }

    SdPage* pMasterPageInDocument = NULL;
    // Search for a master page with the same name as the given one in
    // the target document.
    const OUString sMasterPageLayoutName (pMasterPage->GetLayoutName());
    for (sal_uInt16 nIndex=0,nCount=rTargetDocument.GetMasterPageCount(); nIndex<nCount; ++nIndex)
    {
        SdPage* pCandidate = static_cast<SdPage*>(rTargetDocument.GetMasterPage(nIndex));
        if (pCandidate && sMasterPageLayoutName.equals(pCandidate->GetLayoutName()))
        {
            // The requested master page does already exist in the
            // target document, return it.
            return pCandidate;
        }
    }

    // The given master page does not already belong to the target
    // document so we have to create copies and insert them into the
    // targer document.

    // Determine the position where the new master pages are inserted.
    // By default they are inserted at the end.  When we assign to a
    // master page then insert after the last of the (selected) pages.
    sal_uInt16 nInsertionIndex = rTargetDocument.GetMasterPageCount();
    if (rpPageList->front()->IsMasterPage())
    {
        nInsertionIndex = rpPageList->back()->GetPageNum();
    }

    // Clone the master page.
    if (pMasterPage->GetModel() != &rTargetDocument)
    {
        pMasterPageInDocument = AddMasterPage (rTargetDocument, pMasterPage, nInsertionIndex);
        if( rTargetDocument.IsUndoEnabled() )
                rTargetDocument.AddUndo(
                    rTargetDocument.GetSdrUndoFactory().CreateUndoNewPage(*pMasterPageInDocument));
    }
    else
        pMasterPageInDocument = pMasterPage;

    // Clone the notes master.
    if (pNotesMasterPage->GetModel() != &rTargetDocument)
    {
        SdPage* pClonedNotesMasterPage
            = AddMasterPage (rTargetDocument, pNotesMasterPage, nInsertionIndex+1);
        if( rTargetDocument.IsUndoEnabled() )
            rTargetDocument.AddUndo(
                rTargetDocument.GetSdrUndoFactory().CreateUndoNewPage(*pClonedNotesMasterPage));
    }

    return pMasterPageInDocument;
}





} } } // end of namespace ::sd::toolpanel::controls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
