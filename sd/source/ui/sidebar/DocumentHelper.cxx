/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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

namespace sd { namespace sidebar {

SdPage* DocumentHelper::CopyMasterPageToLocalDocument (
    SdDrawDocument& rTargetDocument,
    SdPage* pMasterPage)
{
    SdPage* pNewMasterPage = NULL;

    do
    {
        if (pMasterPage == NULL)
            break;

        
        SdDrawDocument* pSourceDocument = static_cast<SdDrawDocument*>(
            pMasterPage->GetModel());
        if (pSourceDocument == NULL)
            break;

        
        
        if (pSourceDocument == &rTargetDocument)
        {
            pNewMasterPage = pMasterPage;
            break;
        }

        
        
        
        
        sal_uInt16 nSourceMasterPageCount = pSourceDocument->GetMasterPageCount();
        if (nSourceMasterPageCount%2 == 0)
            
            
            
            break;
        sal_uInt16 nIndex = pMasterPage->GetPageNum();
        if (nSourceMasterPageCount <= nIndex+1)
            break;
        
        if (pMasterPage != static_cast<SdPage*>(
            pSourceDocument->GetMasterPage(nIndex)))
            break;
        
        SdPage* pNotesMasterPage = static_cast<SdPage*>(
            pSourceDocument->GetMasterPage(nIndex+1));
        if (pNotesMasterPage == NULL)
            break;


        
        
        bool bPageExists (false);
        sal_uInt16 nMasterPageCount(rTargetDocument.GetMasterSdPageCount(PK_STANDARD));
        for (sal_uInt16 nMaster=0; nMaster<nMasterPageCount; nMaster++)
        {
            SdPage* pCandidate = static_cast<SdPage*>(
                rTargetDocument.GetMasterSdPage (nMaster, PK_STANDARD));
            if (pMasterPage!=NULL
                && pCandidate->GetName() == pMasterPage->GetName())
            {
                bPageExists = true;
                pNewMasterPage = pCandidate;
                break;
            }
        }
        if (bPageExists)
            break;

        
        uno::Reference<drawing::XDrawPagesSupplier> xSlideSupplier (
            rTargetDocument.getUnoModel(), uno::UNO_QUERY);
        if ( ! xSlideSupplier.is())
            break;
        uno::Reference<drawing::XDrawPages> xSlides (
            xSlideSupplier->getDrawPages(), uno::UNO_QUERY);
        if ( ! xSlides.is())
            break;
        xSlides->insertNewByIndex (xSlides->getCount());

        
        SdPage* pSlide = rTargetDocument.GetSdPage(
            rTargetDocument.GetSdPageCount(PK_STANDARD)-1,
            PK_STANDARD);
        if (pSlide == NULL)
            break;
        pSlide->SetAutoLayout(AUTOLAYOUT_TITLE, sal_True);

        
        
        pNewMasterPage = AddMasterPage(rTargetDocument, pMasterPage);
        if (pNewMasterPage==NULL)
            break;
        SdPage* pNewNotesMasterPage
            = AddMasterPage(rTargetDocument, pNotesMasterPage);
        if (pNewNotesMasterPage==NULL)
            break;

        
        
        rTargetDocument.SetMasterPage (
            rTargetDocument.GetSdPageCount(PK_STANDARD)-1,
            pNewMasterPage->GetName(),
            &rTargetDocument,
            sal_False, 
                   
            sal_True);
    }
    while (false);

    
    
    rTargetDocument.SetChanged(false);

    return pNewMasterPage;
}




SdPage* DocumentHelper::GetSlideForMasterPage (SdPage* pMasterPage)
{
    SdPage* pCandidate = NULL;

    SdDrawDocument* pDocument = NULL;
    if (pMasterPage != NULL)
        pDocument = dynamic_cast<SdDrawDocument*>(pMasterPage->GetModel());

    
    
    if (pDocument!=NULL && pDocument->GetSdPageCount(PK_STANDARD) > 0)
    {
        
        
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
            
            pClonedMasterPage = static_cast<SdPage*>(pMasterPage->Clone());

            
            SdDrawDocument* pSourceDocument
                = static_cast<SdDrawDocument*>(pMasterPage->GetModel());
            if (pSourceDocument != NULL)
                ProvideStyles (*pSourceDocument, rTargetDocument, pClonedMasterPage);

            
            pClonedMasterPage->SetPrecious(pMasterPage->IsPrecious());

            
            
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
    
    OUString sLayoutName (pPage->GetLayoutName());
    sal_Int32 nIndex = sLayoutName.indexOf(SD_LT_SEPARATOR);
    if( nIndex != -1 )
        sLayoutName = sLayoutName.copy(0, nIndex);

    
    SdStyleSheetPool* pSourceStyleSheetPool =
        static_cast<SdStyleSheetPool*>(rSourceDocument.GetStyleSheetPool());
    SdStyleSheetPool* pTargetStyleSheetPool =
        static_cast<SdStyleSheetPool*>(rTargetDocument.GetStyleSheetPool());
    SdStyleSheetVector aCreatedStyles;
    pTargetStyleSheetPool->CopyLayoutSheets (
        sLayoutName,
        *pSourceStyleSheetPool,
        aCreatedStyles);

    
    if( !aCreatedStyles.empty() )
    {
        ::svl::IUndoManager* pUndoManager = rTargetDocument.GetDocSh()->GetUndoManager();
       if (pUndoManager != NULL)
       {
           SdMoveStyleSheetsUndoAction* pMovStyles =
               new SdMoveStyleSheetsUndoAction (
                   &rTargetDocument,
                   aCreatedStyles,
                   true);
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

    
    
    OUString sFullLayoutName(pMasterPage->GetLayoutName());
    OUString sBaseLayoutName (sFullLayoutName);
    sal_Int32 nIndex = sBaseLayoutName.indexOf(SD_LT_SEPARATOR);
    if( nIndex != -1 )
        sBaseLayoutName = sBaseLayoutName.copy(0, nIndex);

    if (rpPageList->empty())
        return;

    
    
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
        pUndoMgr->EnterListAction(SD_RESSTR(STR_UNDO_SET_PRESLAYOUT), OUString());

    SdPage* pMasterPageInDocument = ProvideMasterPage(rTargetDocument,pMasterPage,rpPageList);
    if (pMasterPageInDocument == NULL)
        return;

    
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
        
        pClonedMasterPage = static_cast<SdPage*>(pMasterPage->Clone());

        
        pClonedMasterPage->SetPrecious(pMasterPage->IsPrecious());

        
        SdDrawDocument* pSourceDocument
            = static_cast<SdDrawDocument*>(pMasterPage->GetModel());
        if (pSourceDocument != NULL)
        {
            ProvideStyles (*pSourceDocument, rTargetDocument, pClonedMasterPage);

            
            
            rTargetDocument.InsertMasterPage (pClonedMasterPage, nInsertionIndex);

            
            
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
    const OUString& rsBaseLayoutName,
    SdPage* pPage)
{
    
    if (pPage == NULL || pMasterPage == NULL)
        return;
    SdDrawDocument* pDocument = dynamic_cast<SdDrawDocument*>(pPage->GetModel());
    if (pDocument == NULL)
        return;

    if ( ! pPage->IsMasterPage())
    {
        
        
        
        pDocument->GetDocSh()->GetUndoManager()->AddUndoAction(
            new SdBackgroundObjUndoAction(
                *pDocument, *pPage, pPage->getSdrPageProperties().GetItemSet()),
            true);
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
            
            
            pDocument->SetMasterPage (
                (pSlide->GetPageNum()-1)/2,
                rsBaseLayoutName,
                pDocument,
                sal_False,
                sal_False);
        }
        else
        {
            
            
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
    
    
    
    if (pMasterPage == NULL)
    {
        
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
        
        
        
        return NULL;
    }

    SdPage* pMasterPageInDocument = NULL;
    
    
    const OUString sMasterPageLayoutName (pMasterPage->GetLayoutName());
    for (sal_uInt16 nIndex=0,nCount=rTargetDocument.GetMasterPageCount(); nIndex<nCount; ++nIndex)
    {
        SdPage* pCandidate = static_cast<SdPage*>(rTargetDocument.GetMasterPage(nIndex));
        if (pCandidate && sMasterPageLayoutName.equals(pCandidate->GetLayoutName()))
        {
            
            
            return pCandidate;
        }
    }

    
    
    

    
    
    
    sal_uInt16 nInsertionIndex = rTargetDocument.GetMasterPageCount();
    if (rpPageList->front()->IsMasterPage())
    {
        nInsertionIndex = rpPageList->back()->GetPageNum();
    }

    
    if (pMasterPage->GetModel() != &rTargetDocument)
    {
        pMasterPageInDocument = AddMasterPage (rTargetDocument, pMasterPage, nInsertionIndex);
        if( rTargetDocument.IsUndoEnabled() )
                rTargetDocument.AddUndo(
                    rTargetDocument.GetSdrUndoFactory().CreateUndoNewPage(*pMasterPageInDocument));
    }
    else
        pMasterPageInDocument = pMasterPage;

    
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





} } 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
