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

#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"

#include <vcl/wrkwin.hxx>
#include <sfx2/docfile.hxx>
#include <sot/storage.hxx>
#include <sfx2/app.hxx>
#include <svl/itemset.hxx>

#include <unotools/ucbstreamhelper.hxx>
#include <sfx2/fcontnr.hxx>
#include <svx/svdopath.hxx>
#include <svx/svditer.hxx>
#include <svl/style.hxx>
#include <sfx2/linkmgr.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdundo.hxx>
#include <vcl/msgbox.hxx>
#include <sot/formats.hxx>

#include "glob.hrc"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "stlpool.hxx"
#include "sdresid.hxx"
#include "sdiocmpt.hxx"
#include "strmname.h"
#include "anminfo.hxx"
#include "customshowlist.hxx"

#include "../ui/inc/unmovss.hxx"
#include "../ui/inc/unchss.hxx"
#include "../ui/inc/unprlout.hxx"
#include "../ui/inc/DrawDocShell.hxx"
#include "../ui/inc/GraphicDocShell.hxx"
#include "../ui/inc/ViewShell.hxx"
#include "../ui/inc/View.hxx"
#include "../ui/inc/cfgids.hxx"
#include "../ui/inc/strings.hrc"

using namespace ::com::sun::star;

/** Concrete incarnations get called by lcl_IterateBookmarkPages, for
    every page in the bookmark document/list
 */

class InsertBookmarkAsPage_FindDuplicateLayouts
{
public:
    InsertBookmarkAsPage_FindDuplicateLayouts( std::vector<OUString> &rLayoutsToTransfer )
        : mrLayoutsToTransfer(rLayoutsToTransfer) {}
    void operator()( SdDrawDocument&, SdPage* );
private:
    std::vector<OUString> &mrLayoutsToTransfer;
};

void InsertBookmarkAsPage_FindDuplicateLayouts::operator()( SdDrawDocument& rDoc, SdPage* pBMMPage )
{
    // now check for duplicate masterpage and layout names
    // ===================================================

    String aFullNameLayout( pBMMPage->GetLayoutName() );
    aFullNameLayout.Erase( aFullNameLayout.SearchAscii( SD_LT_SEPARATOR ));

    OUString aLayout(aFullNameLayout);

    std::vector<OUString>::const_iterator pIter =
            find(mrLayoutsToTransfer.begin(),mrLayoutsToTransfer.end(),aLayout);

    bool bFound = pIter != mrLayoutsToTransfer.end();

    const sal_uInt16 nMPageCount = rDoc.GetMasterPageCount();
    for (sal_uInt16 nMPage = 0; nMPage < nMPageCount && !bFound; nMPage++)
    {
        // Do the layouts already exist within the document?
        SdPage* pTestPage = (SdPage*) rDoc.GetMasterPage(nMPage);
        String aFullTest(pTestPage->GetLayoutName());
        aFullTest.Erase( aFullTest.SearchAscii( SD_LT_SEPARATOR ));

        OUString aTest(aFullTest);

        if (aTest == aLayout)
            bFound = true;
    }

    if (!bFound)
        mrLayoutsToTransfer.push_back(aLayout);
}

// Inserts a bookmark as a page
static void lcl_IterateBookmarkPages( SdDrawDocument &rDoc, SdDrawDocument* pBookmarkDoc,
                               const std::vector<OUString> &rBookmarkList, sal_uInt16 nBMSdPageCount,
                               InsertBookmarkAsPage_FindDuplicateLayouts& rPageIterator )
{
    //
    // Refactored copy'n'pasted layout name collection from InsertBookmarkAsPage
    //
    int nPos, nEndPos;

    if( rBookmarkList.empty() )
    {
        // no list? whole source document
        nEndPos = nBMSdPageCount;
    }
    else
    {
        // bookmark list? number of entries
        nEndPos = rBookmarkList.size();
    }

    SdPage* pBMPage;

    // iterate over number of pages to insert
    for (nPos = 0; nPos < nEndPos; ++nPos)
    {
        // the master page associated to the nPos'th page to insert
        SdPage* pBMMPage = NULL;

        if( rBookmarkList.empty() )
        {
            // simply take master page of nPos'th page in source document
            pBMMPage = (SdPage*)(&(pBookmarkDoc->GetSdPage((sal_uInt16)nPos, PK_STANDARD)->TRG_GetMasterPage()));
        }
        else
        {
            // fetch nPos'th entry from bookmark list, and determine master page
            String  aBMPgName(rBookmarkList[nPos]);
            sal_Bool  bIsMasterPage;

            sal_uInt16 nBMPage = pBookmarkDoc->GetPageByName( aBMPgName, bIsMasterPage );

            if (nBMPage != SDRPAGE_NOTFOUND)
            {
                pBMPage = (SdPage*) pBookmarkDoc->GetPage(nBMPage);
            }
            else
            {
                pBMPage = NULL;
            }

            // enforce that bookmarked page is a standard page and not already a master page
            if (pBMPage && pBMPage->GetPageKind()==PK_STANDARD && !pBMPage->IsMasterPage())
            {
                const sal_uInt16 nBMSdPage = (nBMPage - 1) / 2;
                pBMMPage = (SdPage*) (&(pBookmarkDoc->GetSdPage(nBMSdPage, PK_STANDARD)->TRG_GetMasterPage()));
            }
        }

        // successfully determined valid (bookmarked) page?
        if( pBMMPage )
        {
            // yes, call functor
            rPageIterator( rDoc, pBMMPage );
        }
    }
}

// Opens a bookmark document
SdDrawDocument* SdDrawDocument::OpenBookmarkDoc(SfxMedium& rMedium)
{
    sal_Bool bOK = sal_True;
    SdDrawDocument* pBookmarkDoc = NULL;
    String aBookmarkName = rMedium.GetName();
    const SfxFilter* pFilter = rMedium.GetFilter();
    if ( !pFilter )
    {
        rMedium.UseInteractionHandler( sal_True );
        SFX_APP()->GetFilterMatcher().GuessFilter( rMedium, &pFilter );
    }

    if ( !pFilter )
    {
        bOK = sal_False;
    }
    else if ( maBookmarkFile != aBookmarkName && aBookmarkName.Len() )
    {
        sal_Bool bCreateGraphicShell = pFilter->GetServiceName() == "com.sun.star.drawing.DrawingDocument";
        sal_Bool bCreateImpressShell = pFilter->GetServiceName() == "com.sun.star.presentation.PresentationDocument";
        if ( bCreateGraphicShell || bCreateImpressShell )
        {
            CloseBookmarkDoc();

            // Create a DocShell, as OLE objects might be contained in the
            // document. (Persist)
            // If that wasn't the case, we could load the model directly.
            if ( bCreateGraphicShell )
                // Draw
                mxBookmarkDocShRef = new ::sd::GraphicDocShell(SFX_CREATE_MODE_STANDARD, sal_True);
            else
                // Impress
                mxBookmarkDocShRef = new ::sd::DrawDocShell(SFX_CREATE_MODE_STANDARD, sal_True);

            bOK = mxBookmarkDocShRef->DoLoad(&rMedium);
            if( bOK )
            {
                maBookmarkFile = aBookmarkName;
                pBookmarkDoc = mxBookmarkDocShRef->GetDoc();
            }
        }
    }

    DBG_ASSERT(aBookmarkName.Len(), "Empty document name!");

    if (!bOK)
    {
        ErrorBox aErrorBox( NULL, (WinBits)WB_OK, SD_RESSTR(STR_READ_DATA_ERROR));
        aErrorBox.Execute();

        CloseBookmarkDoc();
        pBookmarkDoc = NULL;
    }
    else if (mxBookmarkDocShRef.Is())
    {
        pBookmarkDoc = mxBookmarkDocShRef->GetDoc();
    }

    return(pBookmarkDoc);
}

// Opens a bookmark document
SdDrawDocument* SdDrawDocument::OpenBookmarkDoc(const String& rBookmarkFile)
{
    SdDrawDocument* pBookmarkDoc = NULL;

    if (maBookmarkFile != rBookmarkFile && rBookmarkFile.Len())
    {
        SfxMedium* pMedium = new SfxMedium( rBookmarkFile, STREAM_READ );
        pBookmarkDoc = OpenBookmarkDoc(*pMedium);
    }
    else if (mxBookmarkDocShRef.Is())
    {
        pBookmarkDoc = mxBookmarkDocShRef->GetDoc();
    }

    return(pBookmarkDoc);
}

// Inserts a bookmark (page or object)
sal_Bool SdDrawDocument::InsertBookmark(
    const std::vector<OUString> &rBookmarkList,    // List of names of the bookmarks to be inserted
    std::vector<OUString> &rExchangeList,          // List of the names to be used
    sal_Bool bLink,                                     // Insert bookmarks as links?
    sal_Bool bReplace,                                  // Replace current default and notes pages?
    sal_uInt16 nInsertPos,                              // Insertion position of pages
    sal_Bool bNoDialogs,                                // Don't show dialogs
    ::sd::DrawDocShell* pBookmarkDocSh,                 // If set, this is the source document
    sal_Bool bCopy,                                     // Copy the pages?
    Point* pObjPos)                                     // Insertion position of objects
{
    sal_Bool bOK = sal_True;
    sal_Bool bInsertPages = sal_False;

    if (rBookmarkList.empty())
    {
        // Insert all pages
        bInsertPages = sal_True;
    }
    else
    {
        SdDrawDocument* pBookmarkDoc = NULL;
        String aBookmarkName;

        if (pBookmarkDocSh)
        {
            pBookmarkDoc = pBookmarkDocSh->GetDoc();
            aBookmarkName = pBookmarkDocSh->GetMedium()->GetName();
        }
        else if ( mxBookmarkDocShRef.Is() )
        {
            pBookmarkDoc = mxBookmarkDocShRef->GetDoc();
            aBookmarkName = maBookmarkFile;
        }
        else
            bOK = sal_False;

        std::vector<OUString>::const_iterator pIter;
        for ( pIter = rBookmarkList.begin(); bOK && pIter != rBookmarkList.end() && !bInsertPages; ++pIter )
        {
            // Is there a page name in the bookmark list?
            String  aBMPgName(*pIter);
            sal_Bool    bIsMasterPage;

            if( pBookmarkDoc->GetPageByName( aBMPgName, bIsMasterPage ) != SDRPAGE_NOTFOUND )
            {
                // Found the page
                bInsertPages = sal_True;
            }
        }
    }

    sal_Bool bCalcObjCount = !rExchangeList.empty();

    if ( bOK && bInsertPages )
    {
        // Insert all page bookmarks
        bOK = InsertBookmarkAsPage(rBookmarkList, &rExchangeList, bLink, bReplace,
                                   nInsertPos, bNoDialogs, pBookmarkDocSh, bCopy, sal_True, sal_False);
    }

    if ( bOK && !rBookmarkList.empty() )
    {
        // Insert all object bookmarks
        bOK = InsertBookmarkAsObject(rBookmarkList, rExchangeList, bLink,
                                     pBookmarkDocSh, pObjPos, bCalcObjCount);
    }

    return bOK;
}

namespace
{

void
lcl_removeUnusedStyles(SfxStyleSheetBasePool* const pStyleSheetPool, SdStyleSheetVector& rStyles)
{
    SdStyleSheetVector aUsedStyles;
    aUsedStyles.reserve(rStyles.size());
    for (SdStyleSheetVector::const_iterator aIt(rStyles.begin()), aLast(rStyles.end()); aIt != aLast; ++aIt)
    {
        if ((*aIt)->IsUsed())
            aUsedStyles.push_back(*aIt);
        else
            pStyleSheetPool->Remove((*aIt).get());
    }
    rStyles = aUsedStyles;
}

}

sal_Bool SdDrawDocument::InsertBookmarkAsPage(
    const std::vector<OUString> &rBookmarkList,
    std::vector<OUString> *pExchangeList,            // List of names to be used
    sal_Bool bLink,
    sal_Bool bReplace,
    sal_uInt16 nInsertPos,
    sal_Bool bNoDialogs,
    ::sd::DrawDocShell* pBookmarkDocSh,
    sal_Bool bCopy,
    sal_Bool bMergeMasterPages,
    sal_Bool bPreservePageNames)
{
    sal_Bool bOK = sal_True;
    sal_Bool bContinue = sal_True;
    sal_Bool bScaleObjects = sal_False;
    sal_uInt16 nReplacedStandardPages = 0;

    SdDrawDocument* pBookmarkDoc = NULL;
    String aBookmarkName;

    if (pBookmarkDocSh)
    {
        pBookmarkDoc = pBookmarkDocSh->GetDoc();

        if (pBookmarkDocSh->GetMedium())
        {
            aBookmarkName = pBookmarkDocSh->GetMedium()->GetName();
        }
    }
    else if ( mxBookmarkDocShRef.Is() )
    {
        pBookmarkDoc = mxBookmarkDocShRef->GetDoc();
        aBookmarkName = maBookmarkFile;
    }
    else
    {
        return sal_False;
    }

    const sal_uInt16 nSdPageCount = GetSdPageCount(PK_STANDARD);
    const sal_uInt16 nBMSdPageCount = pBookmarkDoc->GetSdPageCount(PK_STANDARD);
    const sal_uInt16 nMPageCount = GetMasterPageCount();

    if (nSdPageCount==0 || nBMSdPageCount==0 || nMPageCount==0)
    {
        bContinue = bOK = sal_False;
        return(bContinue);
    }

    // Store the size and some other properties of the first page and notes
    // page so that inserted pages can be properly scaled even when inserted
    // before the first page.
    // Note that the pointers are used later on as general page pointers.
    SdPage* pRefPage = GetSdPage(0, PK_STANDARD);
    Size  aSize(pRefPage->GetSize());
    sal_Int32 nLeft  = pRefPage->GetLftBorder();
    sal_Int32 nRight = pRefPage->GetRgtBorder();
    sal_Int32 nUpper = pRefPage->GetUppBorder();
    sal_Int32 nLower = pRefPage->GetLwrBorder();
    Orientation eOrient = pRefPage->GetOrientation();

    SdPage* pNPage = GetSdPage(0, PK_NOTES);
    Size aNSize(pNPage->GetSize());
    sal_Int32 nNLeft  = pNPage->GetLftBorder();
    sal_Int32 nNRight = pNPage->GetRgtBorder();
    sal_Int32 nNUpper = pNPage->GetUppBorder();
    sal_Int32 nNLower = pNPage->GetLwrBorder();
    Orientation eNOrient = pRefPage->GetOrientation();

    // Adapt page size and margins to those of the later pages?
    pRefPage = GetSdPage(nSdPageCount - 1, PK_STANDARD);

    if( bNoDialogs )
    {
        if( rBookmarkList.empty() )
            bScaleObjects = pRefPage->IsScaleObjects();
        else
            bScaleObjects = sal_True;
    }
    else
    {
        SdPage* pBMPage = pBookmarkDoc->GetSdPage(0,PK_STANDARD);

        if (pBMPage->GetSize()        != pRefPage->GetSize()         ||
            pBMPage->GetLftBorder()   != pRefPage->GetLftBorder()    ||
            pBMPage->GetRgtBorder()   != pRefPage->GetRgtBorder()    ||
            pBMPage->GetUppBorder()   != pRefPage->GetUppBorder()    ||
            pBMPage->GetLwrBorder()   != pRefPage->GetLwrBorder())
        {
            String aStr(SdResId(STR_SCALE_OBJECTS));
            sal_uInt16 nBut = QueryBox( NULL, WB_YES_NO_CANCEL, aStr).Execute();

            bScaleObjects = nBut == RET_YES;
            bContinue     = nBut != RET_CANCEL;

            if (!bContinue)
            {
                return(bContinue);
            }
        }
    }


    // Get the necessary presentation stylesheets and transfer them before
    // the pages, else, the text objects won't reference their styles anymore.
    ::svl::IUndoManager* pUndoMgr = NULL;
    if( mpDocSh )
    {
        pUndoMgr = mpDocSh->GetUndoManager();
        pUndoMgr->EnterListAction(SD_RESSTR(STR_UNDO_INSERTPAGES), String());
    }

    //
    // Refactored copy'n'pasted layout name collection into IterateBookmarkPages
    //
    std::vector<OUString> aLayoutsToTransfer;
    InsertBookmarkAsPage_FindDuplicateLayouts aSearchFunctor( aLayoutsToTransfer );
    lcl_IterateBookmarkPages( *this, pBookmarkDoc, rBookmarkList, nBMSdPageCount, aSearchFunctor );


    // Copy the style that we actually need.
    SdStyleSheetPool* pBookmarkStyleSheetPool = dynamic_cast<SdStyleSheetPool*>(pBookmarkDoc->GetStyleSheetPool());
    SdStyleSheetPool* pStyleSheetPool = dynamic_cast<SdStyleSheetPool*>(GetStyleSheetPool());

    // When copying styles, also copy the master pages!
    if( !aLayoutsToTransfer.empty() )
        bMergeMasterPages = sal_True;

    std::vector<OUString>::const_iterator pIter;
    for ( pIter = aLayoutsToTransfer.begin(); pIter != aLayoutsToTransfer.end(); ++pIter )
    {
        SdStyleSheetVector aCreatedStyles;
        String layoutName = *pIter;

        pStyleSheetPool->CopyLayoutSheets(layoutName, *pBookmarkStyleSheetPool,aCreatedStyles);

        if(!aCreatedStyles.empty())
        {
            if( pUndoMgr )
            {
                SdMoveStyleSheetsUndoAction* pMovStyles = new SdMoveStyleSheetsUndoAction(this, aCreatedStyles, sal_True);
                pUndoMgr->AddUndoAction(pMovStyles);
            }
        }
    }

    // Copy styles. This unconditionally copies all styles, even those
    // that are not used in any of the inserted pages. The unused styles
    // are then removed at the end of the function, where we also create
    // undo records for the inserted styles.
    SdStyleSheetVector aNewGraphicStyles;
    pStyleSheetPool->CopyGraphicSheets(*pBookmarkStyleSheetPool, aNewGraphicStyles);
    SdStyleSheetVector aNewCellStyles;
    pStyleSheetPool->CopyCellSheets(*pBookmarkStyleSheetPool, aNewCellStyles);

    // TODO handle undo of table styles too
    pStyleSheetPool->CopyTableStyles(*pBookmarkStyleSheetPool);

    // Insert document

    const bool bUndo = IsUndoEnabled();

    if( bUndo )
        BegUndo(SD_RESSTR(STR_UNDO_INSERTPAGES));

    if (rBookmarkList.empty())
    {
        if (nInsertPos >= GetPageCount())
        {
            // Add pages to the end
            nInsertPos = GetPageCount();
        }

        sal_uInt16 nActualInsertPos = nInsertPos;

        sal_uInt16 nBMSdPage;
        std::set<sal_uInt16> aRenameSet;
        std::map<sal_uInt16,OUString> aNameMap;

        for (nBMSdPage=0; nBMSdPage < nBMSdPageCount; nBMSdPage++)
        {
            SdPage* pBMPage = pBookmarkDoc->GetSdPage(nBMSdPage, PK_STANDARD);
            String  sName( pBMPage->GetName() );
            sal_Bool    bIsMasterPage;

            if (bLink)
            {
                // Remember the names of all pages
                aNameMap.insert(std::make_pair(nBMSdPage,sName));
            }

            // Have to check for duplicate names here, too
            // don't change name if source and dest model are the same!
            if( pBookmarkDoc != this &&
                GetPageByName(sName, bIsMasterPage ) != SDRPAGE_NOTFOUND )
            {
                // delay renaming *after* pages are copied (might destroy source otherwise)
                aRenameSet.insert(nBMSdPage);
            }
        }

        Merge(*pBookmarkDoc,
              1,                 // Not the handout page
              0xFFFF,            // But all others
              nActualInsertPos,  // Insert at position ...
              bMergeMasterPages, // Move master pages?
              sal_False,             // But only the master pages used
              sal_True,              // Create an undo action
              bCopy);            // Copy (or merge) pages?

        for (nBMSdPage=0; nBMSdPage < nBMSdPageCount; nBMSdPage++)
        {
            SdPage* pPage       = (SdPage*) GetPage(nActualInsertPos);
            SdPage* pNotesPage  = (SdPage*) GetPage(nActualInsertPos+1);

            // delay renaming *after* pages are copied (might destroy source otherwise)
            if( aRenameSet.find(nBMSdPage) != aRenameSet.end() )
            {
                // Page name already in use -> Use default name for default and
                // notes page
                pPage->SetName(String());
                pNotesPage->SetName(String());
            }

            if (bLink)
            {
                String aName(aNameMap[nBMSdPage]);

                // Assemble all link names
                pPage->SetFileName(aBookmarkName);
                pPage->SetBookmarkName(aName);
                pPage->SetModel(this);
            }

            nActualInsertPos += 2;
        }
    }
    else
    {
        // Insert selected pages
        SdPage* pBMPage;

        if (nInsertPos >= GetPageCount())
        {
            // Add pages to the end
            bReplace = sal_False;
            nInsertPos = GetPageCount();
        }

        sal_uInt16 nActualInsertPos = nInsertPos;

        // Collect the bookmarked pages
        ::std::vector<SdPage*> aBookmarkedPages (rBookmarkList.size(), NULL);
        for ( size_t nPos = 0, n = rBookmarkList.size(); nPos < n; ++nPos)
        {
            String  aPgName(rBookmarkList[nPos]);
            sal_Bool    bIsMasterPage;
            sal_uInt16  nBMPage = pBookmarkDoc->GetPageByName( aPgName, bIsMasterPage );

            if (nBMPage != SDRPAGE_NOTFOUND)
            {
                aBookmarkedPages[nPos] =  dynamic_cast<SdPage*>(pBookmarkDoc->GetPage(nBMPage));
            }
        }

        for ( size_t nPos = 0, n = rBookmarkList.size(); nPos < n; ++nPos)
        {
            pBMPage = aBookmarkedPages[nPos];
            sal_uInt16 nBMPage = pBMPage!=NULL ? pBMPage->GetPageNum() : SDRPAGE_NOTFOUND;

            if (pBMPage && pBMPage->GetPageKind()==PK_STANDARD && !pBMPage->IsMasterPage())
            {
                // It has to be a default page
                sal_Bool bMustRename = sal_False;

                // delay renaming *after* pages are copied (might destroy source otherwise)
                // don't change name if source and dest model are the same!
                // avoid renaming if replacing the same page
                String  aPgName(rBookmarkList[nPos]);
                sal_Bool    bIsMasterPage;
                sal_uInt16 nPageSameName = GetPageByName(aPgName, bIsMasterPage);
                if( pBookmarkDoc != this &&
                    nPageSameName != SDRPAGE_NOTFOUND &&
                    ( !bReplace ||
                      nPageSameName != nActualInsertPos ) )
                {
                    bMustRename = sal_True;
                }

                SdPage* pBookmarkPage = pBMPage;
                if (bReplace )
                {
                    ReplacePageInCustomShows( dynamic_cast< SdPage* >( GetPage( nActualInsertPos ) ), pBookmarkPage );
                }

                Merge(*pBookmarkDoc,
                      nBMPage,           // From page (default page)
                      nBMPage+1,         // To page (notes page)
                      nActualInsertPos,  // Insert at position
                      bMergeMasterPages, // Move master pages?
                      sal_False,             // But only the master pages used
                      sal_True,              // Create undo action
                      bCopy);            // Copy (or merge) pages?

                if( bReplace )
                {
                    if( GetPage( nActualInsertPos ) != pBookmarkPage )
                    {
                        // bookmark page was not moved but cloned, so update custom shows again
                        ReplacePageInCustomShows( pBookmarkPage, dynamic_cast< SdPage* >( GetPage( nActualInsertPos ) ) );
                    }
                }

                if( bMustRename )
                {
                    // Page name already in use -> use default name for default and
                    // notes page
                    SdPage* pPage = (SdPage*) GetPage(nActualInsertPos);
                    pPage->SetName(String());
                    SdPage* pNotesPage = (SdPage*) GetPage(nActualInsertPos+1);
                    pNotesPage->SetName(String());
                }

                if (bLink)
                {
                    SdPage* pPage = (SdPage*) GetPage(nActualInsertPos);
                    pPage->SetFileName(aBookmarkName);
                    pPage->SetBookmarkName(aPgName);
                    pPage->SetModel(this);
                }

                if (bReplace)
                {
                    // Remove page and notes page.
                    const sal_uInt16 nDestPageNum(nActualInsertPos + 2);
                    SdPage* pStandardPage = 0L;

                    if(nDestPageNum < GetPageCount())
                    {
                        pStandardPage = (SdPage*)GetPage(nDestPageNum);
                    }

                    if (pStandardPage)
                    {
                        if( bPreservePageNames )
                        {
                            // Take old slide names for inserted pages
                            SdPage* pPage = (SdPage*) GetPage(nActualInsertPos);
                            pPage->SetName( pStandardPage->GetRealName() );
                        }

                        if( bUndo )
                            AddUndo(GetSdrUndoFactory().CreateUndoDeletePage(*pStandardPage));

                        RemovePage(nDestPageNum);

                        if( !bUndo )
                            delete pStandardPage;
                    }

                    SdPage* pNotesPage = 0L;

                    if(nDestPageNum < GetPageCount())
                    {
                        pNotesPage = (SdPage*)GetPage(nDestPageNum);
                    }

                    if (pNotesPage)
                    {
                        if( bPreservePageNames )
                        {
                            // Take old slide names for inserted pages
                            SdPage* pNewNotesPage = (SdPage*) GetPage(nActualInsertPos+1);
                            if( pNewNotesPage )
                                pNewNotesPage->SetName( pStandardPage->GetRealName() );
                        }

                        if( bUndo )
                            AddUndo(GetSdrUndoFactory().CreateUndoDeletePage(*pNotesPage));

                        RemovePage(nDestPageNum);

                        if( !bUndo )
                            delete pNotesPage;
                    }

                    nReplacedStandardPages++;
                }

                nActualInsertPos += 2;
            }
        }
    }


    // We might have duplicate master pages now, as the drawing engine does not
    // recognize duplicates. Remove these now.
    sal_uInt16 nNewMPageCount = GetMasterPageCount();

    // Go backwards, so the numbers don't become messed up
    for (sal_uInt16 nPage = nNewMPageCount - 1; nPage >= nMPageCount; nPage--)
    {
        pRefPage = (SdPage*) GetMasterPage(nPage);
        String aMPLayout(pRefPage->GetLayoutName());
        PageKind eKind = pRefPage->GetPageKind();

        // Does this already exist?
        for (sal_uInt16 nTest = 0; nTest < nMPageCount; nTest++)
        {
            SdPage* pTest = (SdPage*) GetMasterPage(nTest);
            String aTest(pTest->GetLayoutName());

            // nInsertPos > 2 is always true when inserting into non-empty models
            if ( nInsertPos > 2 &&
                 aTest == aMPLayout &&
                 eKind == pTest->GetPageKind() )
            {
                if( bUndo )
                    AddUndo(GetSdrUndoFactory().CreateUndoDeletePage(*pRefPage));

                RemoveMasterPage(nPage);

                if( !bUndo )
                    delete pRefPage;
                nNewMPageCount--;
                break;
            }
        }
    }

    // nInsertPos > 2 is always true when inserting into non-empty models
    if (nInsertPos > 0)
    {
        sal_uInt16 nSdPageStart = (nInsertPos - 1) / 2;
        sal_uInt16 nSdPageEnd = GetSdPageCount(PK_STANDARD) - nSdPageCount +
                            nSdPageStart - 1;
        const bool bRemoveEmptyPresObj = pBookmarkDoc &&
                (pBookmarkDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS) &&
                (GetDocumentType() == DOCUMENT_TYPE_DRAW);

        if( bReplace )
        {
            nSdPageEnd = nSdPageStart + nReplacedStandardPages - 1;
        }

        std::vector<OUString>::iterator pExchangeIter;

        if (pExchangeList)
            pExchangeIter = pExchangeList->begin();

        for (sal_uInt16 nSdPage = nSdPageStart; nSdPage <= nSdPageEnd; nSdPage++)
        {
            pRefPage = GetSdPage(nSdPage, PK_STANDARD);

            if (pExchangeList && pExchangeIter != pExchangeList->end())
            {
                // Get the name to use from Exchange list
                String aExchangeName (*pExchangeIter);
                pRefPage->SetName(aExchangeName);
                SdrHint aHint(HINT_PAGEORDERCHG);
                aHint.SetPage(pRefPage);
                Broadcast(aHint);
                SdPage* pNewNotesPage = GetSdPage(nSdPage, PK_NOTES);
                pNewNotesPage->SetName(aExchangeName);
                aHint.SetPage(pNewNotesPage);
                Broadcast(aHint);

                ++pExchangeIter;
            }

            String aLayout(pRefPage->GetLayoutName());
            aLayout.Erase(aLayout.SearchAscii( SD_LT_SEPARATOR ));

            // update layout and referred master page
            pRefPage->SetPresentationLayout(aLayout);
            if( bUndo )
                AddUndo( GetSdrUndoFactory().CreateUndoPageChangeMasterPage( *pRefPage ) );

            if (bScaleObjects)
            {
                Rectangle aBorderRect(nLeft, nUpper, nRight, nLower);
                pRefPage->ScaleObjects(aSize, aBorderRect, sal_True);
            }
            pRefPage->SetSize(aSize);
            pRefPage->SetBorder(nLeft, nUpper, nRight, nLower);
            pRefPage->SetOrientation( eOrient );

            if( bRemoveEmptyPresObj )
                pRefPage->RemoveEmptyPresentationObjects();

            pRefPage = GetSdPage(nSdPage, PK_NOTES);

            // update layout and referred master page
            pRefPage->SetPresentationLayout(aLayout);
            if( bUndo )
                AddUndo( GetSdrUndoFactory().CreateUndoPageChangeMasterPage( *pRefPage ) );

            if (bScaleObjects)
            {
                Rectangle aBorderRect(nNLeft, nNUpper, nNRight, nNLower);
                pRefPage->ScaleObjects(aNSize, aBorderRect, sal_True);
            }

            pRefPage->SetSize(aNSize);
            pRefPage->SetBorder(nNLeft, nNUpper, nNRight, nNLower);
            pRefPage->SetOrientation( eNOrient );

            if( bRemoveEmptyPresObj )
                pRefPage->RemoveEmptyPresentationObjects();
        }

        ///Remove processed elements, to avoid doings hacks in InsertBookmarkAsObject
        if ( pExchangeList )
            pExchangeList->erase(pExchangeList->begin(),pExchangeIter);

        for (sal_uInt16 nPage = nMPageCount; nPage < nNewMPageCount; nPage++)
        {
            pRefPage = (SdPage*) GetMasterPage(nPage);
            if (pRefPage->GetPageKind() == PK_STANDARD)
            {
                if (bScaleObjects)
                {
                    Rectangle aBorderRect(nLeft, nUpper, nRight, nLower);
                    pRefPage->ScaleObjects(aSize, aBorderRect, sal_True);
                }
                pRefPage->SetSize(aSize);
                pRefPage->SetBorder(nLeft, nUpper, nRight, nLower);
                pRefPage->SetOrientation( eOrient );
            }
            else        // Can only be notes
            {
                if (bScaleObjects)
                {
                    Rectangle aBorderRect(nNLeft, nNUpper, nNRight, nNLower);
                    pRefPage->ScaleObjects(aNSize, aBorderRect, sal_True);
                }
                pRefPage->SetSize(aNSize);
                pRefPage->SetBorder(nNLeft, nNUpper, nNRight, nNLower);
                pRefPage->SetOrientation( eNOrient );
            }

            if( bRemoveEmptyPresObj )
                pRefPage->RemoveEmptyPresentationObjects();
        }
    }

    // Make absolutely sure no double masterpages are there
    RemoveUnnecessaryMasterPages(NULL, sal_True, sal_True);

    // remove copied styles not used on any inserted page and create
    // undo records
    // WARNING: SdMoveStyleSheetsUndoAction clears the passed list of
    // styles, so it cannot be used after this point
    lcl_removeUnusedStyles(GetStyleSheetPool(), aNewGraphicStyles);
    if (!aNewGraphicStyles.empty() && pUndoMgr)
        pUndoMgr->AddUndoAction(new SdMoveStyleSheetsUndoAction(this, aNewGraphicStyles, sal_True));
    lcl_removeUnusedStyles(GetStyleSheetPool(), aNewCellStyles);
    if (!aNewCellStyles.empty() && pUndoMgr)
        pUndoMgr->AddUndoAction(new SdMoveStyleSheetsUndoAction(this, aNewCellStyles, sal_True));

    if( bUndo )
        EndUndo();
    pUndoMgr->LeaveListAction();

    return bContinue;
}

// Inserts a bookmark as an object
sal_Bool SdDrawDocument::InsertBookmarkAsObject(
    const std::vector<OUString> &rBookmarkList,
    const std::vector<OUString> &rExchangeList,            // List of names to use
    sal_Bool /* bLink */,
    ::sd::DrawDocShell* pBookmarkDocSh,
    Point* pObjPos, bool bCalcObjCount)
{
    sal_Bool bOK = sal_True;
    sal_Bool bOLEObjFound = sal_False;
    ::sd::View* pBMView = NULL;

    SdDrawDocument* pBookmarkDoc = NULL;
    String aBookmarkName;

    if (pBookmarkDocSh)
    {
        pBookmarkDoc = pBookmarkDocSh->GetDoc();

        if (pBookmarkDocSh->GetMedium())
        {
            aBookmarkName = pBookmarkDocSh->GetMedium()->GetName();
        }
    }
    else if ( mxBookmarkDocShRef.Is() )
    {
        pBookmarkDoc = mxBookmarkDocShRef->GetDoc();
        aBookmarkName = maBookmarkFile;
    }
    else
    {
        return sal_False;
    }

    if (rBookmarkList.empty())
    {
        pBMView = new ::sd::View(*pBookmarkDoc, (OutputDevice*) NULL);
        pBMView->EndListening(*pBookmarkDoc);
        pBMView->MarkAll();
    }
    else
    {
        SdrPage* pPage;
        SdrPageView* pPV;

        std::vector<OUString>::const_iterator pIter;
        for ( pIter = rBookmarkList.begin(); pIter != rBookmarkList.end(); ++pIter )
        {
            // Get names of bookmarks from the list
            String aBMName (*pIter);

            SdrObject* pObj = pBookmarkDoc->GetObj(aBMName);

            if (pObj)
            {
                // Found an object
                if (pObj->GetObjInventor() == SdrInventor &&
                    pObj->GetObjIdentifier() == OBJ_OLE2)
                {
                    bOLEObjFound = sal_True;
                }

                if (!pBMView)
                {
                    // Create View for the first time
                    pBMView = new ::sd::View(*pBookmarkDoc, (OutputDevice*) NULL);
                    pBMView->EndListening(*pBookmarkDoc);
                }

                pPage = pObj->GetPage();

                if (pPage->IsMasterPage())
                {
                    pPV = pBMView->ShowSdrPage(pBMView->GetModel()->GetMasterPage(pPage->GetPageNum()));
                }
                else
                {
                    pPV = pBMView->GetSdrPageView();
                    if( !pPV || (pPV->GetPage() != pPage))
                        pPV = pBMView->ShowSdrPage(pPage);
                }

                pBMView->MarkObj(pObj, pPV, sal_False);
            }
        }
    }

    if (pBMView)
    {
        // Insert selected objects
        ::sd::View* pView = new ::sd::View(*this, (OutputDevice*) NULL);
        pView->EndListening(*this);

        // Look for the page into which the objects are supposed to be inserted
        SdrPage* pPage = GetSdPage(0, PK_STANDARD);

        if (mpDocSh)
        {
            ::sd::ViewShell* pViewSh = mpDocSh->GetViewShell();

            if (pViewSh)
            {
                // Which page is currently in view?
                SdrPageView* pPV = pViewSh->GetView()->GetSdrPageView();

                if (pPV)
                {
                    pPage = pPV->GetPage();
                }
                else if (pViewSh->GetActualPage())
                {
                    pPage = pViewSh->GetActualPage();
                }
            }
        }

        Point aObjPos;

        if (pObjPos)
        {
            aObjPos = *pObjPos;
        }
        else
        {
            aObjPos = Rectangle(Point(), pPage->GetSize()).Center();
        }

        sal_uLong nCountBefore = 0;

        if (!rExchangeList.empty() || bCalcObjCount)
        {
            // Sort OrdNums and get the number of objects before inserting
            pPage->RecalcObjOrdNums();
            nCountBefore = pPage->GetObjCount();
        }

        if (bOLEObjFound)
            pBMView->GetDoc().SetAllocDocSh(sal_True);

        SdDrawDocument* pTmpDoc = (SdDrawDocument*) pBMView->GetMarkedObjModel();
        bOK = pView->Paste(*pTmpDoc, aObjPos, pPage);

        if (bOLEObjFound)
            pBMView->GetDoc().SetAllocDocSh(sal_False);

        if (!bOLEObjFound)
            delete pTmpDoc;         // Would otherwise be destroyed by DocShell

        delete pView;

        if (!rExchangeList.empty())
        {
            // Get number of objects after inserting.
            sal_uLong nCount = pPage->GetObjCount();

            std::vector<OUString>::const_iterator pIter = rExchangeList.begin();
            for (sal_uLong nObj = nCountBefore; nObj < nCount; nObj++)
            {
                // Get the name to use from the Exchange list
                if (pIter != rExchangeList.end())
                {
                    String aExchangeName (*pIter);

                    if (pPage->GetObj(nObj))
                    {
                        pPage->GetObj(nObj)->SetName(aExchangeName);
                    }

                    ++pIter;
                }
            }
        }
    }

    delete pBMView;

    return bOK;
}

// Stops the bookmark insertion
void SdDrawDocument::CloseBookmarkDoc()
{
    if (mxBookmarkDocShRef.Is())
    {
        mxBookmarkDocShRef->DoClose();
    }

    mxBookmarkDocShRef.Clear();
    maBookmarkFile = String();
}

// Is this document read-only?
bool SdDrawDocument::IsReadOnly() const
{
    return sal_False;
}


// In the subsequent AllocModel() a DocShell (xAllocedDocShRef) is created.
// Any pre-existing DocShell is deleted
void SdDrawDocument::SetAllocDocSh(sal_Bool bAlloc)
{
    mbAllocDocSh = bAlloc;

    if(mxAllocedDocShRef.Is())
    {
        mxAllocedDocShRef->DoClose();
    }

    mxAllocedDocShRef.Clear();
}

// Return list of CustomShows (create it, too, if necessary)
SdCustomShowList* SdDrawDocument::GetCustomShowList(sal_Bool bCreate)
{
    if (!mpCustomShowList && bCreate)
    {
        mpCustomShowList = new SdCustomShowList;
    }

    return mpCustomShowList;
}

// Remove unused master pages and layouts
void SdDrawDocument::RemoveUnnecessaryMasterPages(SdPage* pMasterPage, sal_Bool bOnlyDuplicatePages, sal_Bool bUndo)
{
    ::sd::View* pView = NULL;
    ::svl::IUndoManager* pUndoMgr = NULL;

    if( bUndo && !IsUndoEnabled() )
        bUndo = sal_False;

    if (mpDocSh)
    {
        pUndoMgr = mpDocSh->GetUndoManager();

        if (mpDocSh->GetViewShell())
            pView = mpDocSh->GetViewShell()->GetView();
    }

    // Check all master pages
    sal_uInt16 nSdMasterPageCount = GetMasterSdPageCount( PK_STANDARD );
    for (sal_Int32 nMPage = nSdMasterPageCount - 1; nMPage >= 0; nMPage--)
    {
        SdPage* pMaster = pMasterPage;
        SdPage* pNotesMaster = NULL;

        if (!pMaster)
        {
            pMaster = (SdPage*) GetMasterSdPage( (sal_uInt16) nMPage, PK_STANDARD );
            pNotesMaster = (SdPage*) GetMasterSdPage( (sal_uInt16) nMPage, PK_NOTES );
        }
        else
        {
            for ( sal_uInt16 nMPg = 0; nMPg < GetMasterPageCount(); nMPg++ )
            {
                if ( pMaster == GetMasterPage( nMPg ) )
                {
                    pNotesMaster = (SdPage*) GetMasterPage( ++nMPg );
                    break;
                }
            }
        }

        DBG_ASSERT( pMaster->GetPageKind() == PK_STANDARD, "wrong page kind" );

        if ( pMaster->GetPageKind() == PK_STANDARD &&
             GetMasterPageUserCount( pMaster ) == 0 &&
             pNotesMaster )
        {
            // Do not delete master pages that have their precious flag set
            sal_Bool bDeleteMaster = !pMaster->IsPrecious();
            OUString aLayoutName = pMaster->GetLayoutName();

            if(bOnlyDuplicatePages )
            {
                // remove only duplicate pages
                bDeleteMaster = sal_False;
                for (sal_uInt16 i = 0; i < GetMasterSdPageCount( PK_STANDARD ); i++)
                {
                    SdPage* pMPg = (SdPage*) GetMasterSdPage( i, PK_STANDARD );
                    if( pMPg != pMaster &&
                        pMPg->GetLayoutName() == aLayoutName )
                    {
                        // duplicate page found -> remove it
                        bDeleteMaster = sal_True;
                    }
                }
            }

            if( bDeleteMaster )
            {
                if (pView)
                {
                    // if MasterPage is visible hide on pageview
                    SdrPageView* pPgView = pView->GetSdrPageView();
                    if (pPgView)
                    {
                        SdrPage* pShownPage = pPgView->GetPage();
                        if( (pShownPage == pMaster) || (pShownPage == pNotesMaster) )
                        {
                            pView->HideSdrPage();
                            pView->ShowSdrPage( GetSdPage( 0, PK_STANDARD ) );
                        }
                    }
                }

                if( bUndo )
                {
                    BegUndo();
                    AddUndo( GetSdrUndoFactory().CreateUndoDeletePage( *pNotesMaster ) );
                }

                RemoveMasterPage( pNotesMaster->GetPageNum() );

                if( !bUndo )
                    delete pNotesMaster;

                if( bUndo )
                    AddUndo(GetSdrUndoFactory().CreateUndoDeletePage(*pMaster));

                RemoveMasterPage( pMaster->GetPageNum() );

                if( !bUndo )
                    delete pMaster;

                if( bUndo )
                    EndUndo();  // do this here already, so Joe's actions happen _between_ our own

                // Delete old, unused layout stylesheets
                sal_Bool bDeleteOldStyleSheets = sal_True;
                for ( sal_uInt16 nMPg = 0;
                          nMPg < GetMasterPageCount() && bDeleteOldStyleSheets;
                       nMPg++ )
                {
                    SdPage* pMPg = (SdPage*) GetMasterPage(nMPg);
                    if (pMPg->GetLayoutName() == aLayoutName)
                    {
                        bDeleteOldStyleSheets = sal_False;
                    }
                }

                if (bDeleteOldStyleSheets)
                {
                    SdStyleSheetVector aRemove;
                    static_cast<SdStyleSheetPool*>( mxStyleSheetPool.get())->CreateLayoutSheetList( aLayoutName, aRemove );

                    if( bUndo )
                    {
                        // This list belongs to UndoAction
                        SdMoveStyleSheetsUndoAction* pMovStyles = new SdMoveStyleSheetsUndoAction( this, aRemove, false );

                        if (pUndoMgr)
                            pUndoMgr->AddUndoAction(pMovStyles);
                    }

                    for( SdStyleSheetVector::iterator iter = aRemove.begin(); iter != aRemove.end(); ++iter )
                        static_cast<SdStyleSheetPool*>( mxStyleSheetPool.get())->Remove((*iter).get());
                }
            }
        }

        if (pMasterPage)
            break;                      // Just this one master page!
    }
}


/** Exchange master page
  *
  * Either the nSdPageNum gets a new, own master page or the master page is
  * exchanged completely (which then applies to all pages).
  *
  * nSdPageNum   : page number that the new master page should get.
  * rLayoutName  : LayoutName of the new master page
  * pSourceDoc   : document (template) to get the master page from
  * bMaster      : exchange the master page of nSdPageNum
  * bCheckMasters: remove unused master pages
  *
  * If pSourceDoc == NULL, an empty master page is applied.
  * If rLayoutName is empty, the first master page is used.
  */
// #i121863# factored out functionality
bool isMasterPageLayoutNameUnique(const SdDrawDocument& rDoc, const String& rCandidate)
{
    if(!rCandidate.Len())
    {
        return false;
    }

    const sal_uInt16 nPageCount(rDoc.GetMasterPageCount());

    for(sal_uInt16 a(0); a < nPageCount; a++)
    {
        const SdrPage* pCandidate = rDoc.GetMasterPage(a);
        String aPageLayoutName(pCandidate->GetLayoutName());
        aPageLayoutName.Erase(aPageLayoutName.SearchAscii(SD_LT_SEPARATOR));

        if(aPageLayoutName == rCandidate)
        {
            return false;
        }
    }

    return true;
}

// #i121863# factored out functinality
String createNewMasterPageLayoutName(const SdDrawDocument& rDoc)
{
    const String aBaseName(SdResId(STR_LAYOUT_DEFAULT_NAME));
    String aRetval;
    sal_uInt16 nCount(0);

    while(!aRetval.Len())
    {
        aRetval = aBaseName;

        if(nCount)
        {
            aRetval += OUString::number(nCount);
        }

        nCount++;

        if(!isMasterPageLayoutNameUnique(rDoc, aRetval))
        {
            aRetval.Erase();
        }
    }

    return aRetval;
}

void SdDrawDocument::SetMasterPage(sal_uInt16 nSdPageNum,
                                   const String& rLayoutName,
                                   SdDrawDocument* pSourceDoc,
                                   sal_Bool bMaster,
                                   sal_Bool bCheckMasters)
{
    if( mpDocSh )
        mpDocSh->SetWaitCursor( sal_True );

    ::svl::IUndoManager* pUndoMgr = mpDocSh->GetUndoManager();

    const bool bUndo = IsUndoEnabled();

    if( bUndo )
    {
        pUndoMgr->EnterListAction(SD_RESSTR(STR_UNDO_SET_PRESLAYOUT), String());
    }

    SdPage* pSelectedPage   = GetSdPage(nSdPageNum, PK_STANDARD);
    SdPage* pNotes          = (SdPage*) GetPage(pSelectedPage->GetPageNum()+1);
    SdPage& rOldMaster      = (SdPage&)pSelectedPage->TRG_GetMasterPage();
    SdPage& rOldNotesMaster = (SdPage&)pNotes->TRG_GetMasterPage();
    SdPage* pMaster         = NULL;
    SdPage* pNotesMaster    = NULL;
    SdPage* pPage           = NULL;
    String aOldPageLayoutName(pSelectedPage->GetLayoutName());
    String aOldLayoutName(aOldPageLayoutName);
    aOldLayoutName.Erase(aOldLayoutName.SearchAscii( SD_LT_SEPARATOR ));

    if (pSourceDoc)
    {
        std::vector<StyleReplaceData> aReplList; // List of replaced stylesheets
        sal_Bool bLayoutReloaded = sal_False;   // Wurde ex. Layout wieder geladen?

        // LayoutName, Page and Notes page
        if (rLayoutName.Len() == 0)
        {
            // No LayoutName: take first MasterPage
            pMaster = (SdPage*) pSourceDoc->GetMasterSdPage(0, PK_STANDARD);
            pNotesMaster = (SdPage*) pSourceDoc->GetMasterSdPage(0, PK_NOTES);
        }
        else
        {
            OUStringBuffer aBuf(rLayoutName);
            aBuf.append(SD_LT_SEPARATOR).append(SdResId(STR_LAYOUT_OUTLINE).toString());
            OUString aSearchFor(aBuf.makeStringAndClear());

            for (sal_uInt16 nMP = 0; nMP < pSourceDoc->GetMasterPageCount(); ++nMP)
            {
                SdPage* pMP = (SdPage*) pSourceDoc->GetMasterPage(nMP);

                if (pMP->GetLayoutName() == aSearchFor)
                {
                    if (pMP->GetPageKind() == PK_STANDARD)
                        pMaster = pMP;
                    if (pMP->GetPageKind() == PK_NOTES)
                        pNotesMaster = pMP;
                }
                if (pMaster && pNotesMaster)
                    break;
            }
            DBG_ASSERT(pMaster, "MasterPage (Standard page) not found");
            DBG_ASSERT(pNotesMaster, "MasterPage (Notes page) not found");

            // this should not happen, but looking at crashreports, it does
            if( (pMaster == NULL) || (pNotesMaster == NULL) )
            {
                // so take the first MasterPage
                pMaster = (SdPage*) pSourceDoc->GetMasterSdPage(0, PK_STANDARD);
                pNotesMaster = (SdPage*) pSourceDoc->GetMasterSdPage(0, PK_NOTES);
            }
        }

        // we should never reach this, but one never knows...
        if( (pMaster == NULL) || (pNotesMaster == NULL) )
        {
            pUndoMgr->LeaveListAction();

            if( mpDocSh )
                mpDocSh->SetWaitCursor( sal_False );

            OSL_FAIL( "SdDrawDocument::SetMasterPage() failed!" );

            return;
        }

        const String aOriginalNewLayoutName( pMaster->GetName() );
        String aTargetNewLayoutName(aOriginalNewLayoutName);

        if (pSourceDoc != this)
        {
            // #i121863# clone masterpages, they are from another model (!)
            SdPage* pNewNotesMaster = dynamic_cast< SdPage* >(pNotesMaster->Clone(this));
            SdPage* pNewMaster = dynamic_cast< SdPage* >(pMaster->Clone(this));

            if(!pNewNotesMaster || !pNewMaster)
            {
                delete pNewNotesMaster;
                delete pNewMaster;
                OSL_ASSERT("SdDrawDocument::SetMasterPage() cloning of MasterPage/NoteAmsterPage failed!" );
                return;
            }

            pNotesMaster = pNewNotesMaster;
            pMaster = pNewMaster;

            // layout name needs to be unique
            aTargetNewLayoutName = pMaster->GetLayoutName();
            aTargetNewLayoutName.Erase(aTargetNewLayoutName.SearchAscii(SD_LT_SEPARATOR));

            if(!isMasterPageLayoutNameUnique(*this, aTargetNewLayoutName))
            {
                aTargetNewLayoutName = createNewMasterPageLayoutName(*this);

                String aTemp(aTargetNewLayoutName);
                aTemp.AppendAscii(RTL_CONSTASCII_STRINGPARAM(SD_LT_SEPARATOR));
                aTemp.Append(SD_RESSTR(STR_LAYOUT_OUTLINE));

                pMaster->SetName(aTargetNewLayoutName);
                pMaster->SetLayoutName(aTemp);

                pNotesMaster->SetName(aTargetNewLayoutName);
                pNotesMaster->SetLayoutName(aTemp);
            }
        }

        if (pSourceDoc != this)
        {
            const sal_uInt16 nMasterPageCount = GetMasterPageCount();
            for ( sal_uInt16 nMPage = 0; nMPage < nMasterPageCount; nMPage++ )
            {
                SdPage* pCheckMaster = (SdPage*)GetMasterPage(nMPage);
                if( pCheckMaster->GetName() == aTargetNewLayoutName )
                {
                    bLayoutReloaded = sal_True;
                    break;
                }
            }

            // Correct or create presentation templates --
            // only worry about presentation templates
            String aName;
            SdStyleSheetPool* pSourceStyleSheetPool = (SdStyleSheetPool*) pSourceDoc->GetStyleSheetPool();
            pSourceStyleSheetPool->SetSearchMask(SD_STYLE_FAMILY_MASTERPAGE);
            static_cast<SdStyleSheetPool*>( mxStyleSheetPool.get())->SetSearchMask(SD_STYLE_FAMILY_MASTERPAGE);

            SdStyleSheetVector aCreatedStyles;          // List of created stylesheets
            SfxStyleSheetBase* pHisSheet = pSourceStyleSheetPool->First();

            while (pHisSheet)
            {
                aName = pHisSheet->GetName();

                // #i121863# search in source styles with original style name from source of
                // evtl. cloned master (not-cloned, renamed for uniqueness)
                if( aName.Search( aOriginalNewLayoutName ) == 0 )
                {
                    // #i121863# build name of evtl. cloned master style to search for
                    if(aOriginalNewLayoutName != aTargetNewLayoutName)
                    {
                        const sal_uInt16 nPos(aName.SearchAscii(SD_LT_SEPARATOR));
                        aName.Erase(0, nPos);
                        aName.Insert(aTargetNewLayoutName, 0);
                    }

                    SfxStyleSheet* pMySheet = static_cast<SfxStyleSheet*>( mxStyleSheetPool->Find(aName, SD_STYLE_FAMILY_MASTERPAGE) );

                    if (pMySheet)
                    {
                        // A stylesheet of the same name already exists -> overwrite contents
#ifdef DBG_UTIL
                        sal_Bool bTest =
#endif
                            pMySheet->SetName(pHisSheet->GetName());
                        DBG_ASSERT(bTest, "Renaming StyleSheet failed.");
                        pMySheet->GetItemSet().ClearItem(0);  // Delete all

                        StyleSheetUndoAction* pUndoChStyle = new StyleSheetUndoAction(this,
                                                                 pMySheet, &pHisSheet->GetItemSet());
                        pUndoMgr->AddUndoAction(pUndoChStyle);
                        pMySheet->GetItemSet().Put(pHisSheet->GetItemSet());
                        pMySheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
                    }
                    else
                    {
                        // create new style
                        OUString aHelpFile;
                        pMySheet = static_cast<SfxStyleSheet*>( &mxStyleSheetPool->Make(aName, SD_STYLE_FAMILY_MASTERPAGE, pHisSheet->GetMask()) );
                        pMySheet->SetHelpId( aHelpFile, pHisSheet->GetHelpId(aHelpFile) );
                        pMySheet->GetItemSet().ClearItem(0);  // Delete all
                        pMySheet->GetItemSet().Put(pHisSheet->GetItemSet());

                        aCreatedStyles.push_back( SdStyleSheetRef( static_cast< SdStyleSheet* >( pMySheet ) ) );
                    }

                    StyleReplaceData aReplData;
                    aReplData.nNewFamily = pMySheet->GetFamily();
                    aReplData.nFamily    = pMySheet->GetFamily();
                    aReplData.aNewName   = pMySheet->GetName();

                    // #i121863# re-create original name of styte used at page where to replace with
                    // this new style
                    String aTemp(pMySheet->GetName());
                    const sal_uInt16 nPos(aTemp.SearchAscii(SD_LT_SEPARATOR));
                    aTemp.Erase(0, nPos);
                    aTemp.Insert(aOldLayoutName, 0);
                    aReplData.aName = aTemp;
                    aReplList.push_back(aReplData);
                }

                pHisSheet = (SfxStyleSheet*) pSourceStyleSheetPool->Next();
            }

            // If new styles were created: re-create parent chaining of the item
            // sets in the styles.
            if(!aCreatedStyles.empty())
            {
                std::vector<StyleReplaceData>::iterator pRDataIter;
                for ( pRDataIter = aReplList.begin(); pRDataIter != aReplList.end(); ++pRDataIter )
                {
                    SfxStyleSheetBase* pSOld = mxStyleSheetPool->Find(pRDataIter->aName);
                    SfxStyleSheetBase* pSNew = mxStyleSheetPool->Find(pRDataIter->aNewName);

                    if (pSOld && pSNew)
                    {
                        const String& rParentOfOld = pSOld->GetParent();
                        const String& rParentOfNew = pSNew->GetParent();

                        if (rParentOfOld.Len() > 0 && rParentOfNew.Len() == 0)
                        {
                            std::vector<StyleReplaceData>::iterator pRDIter;
                            for ( pRDIter = aReplList.begin(); pRDIter != aReplList.end(); ++pRDIter )
                            {
                                if ((pRDIter->aName == rParentOfOld) && (pRDIter->aName != pRDIter->aNewName))
                                {
                                    String aParentOfNew(pRDIter->aNewName);
                                    pSNew->SetParent(aParentOfNew);
                                    break;
                                }
                            }
                        }
                    }
                }

                // Now look for all of them when searching
                pSourceStyleSheetPool->SetSearchMask(SFX_STYLE_FAMILY_ALL);
                mxStyleSheetPool->SetSearchMask(SFX_STYLE_FAMILY_ALL);
            }

            if( !aCreatedStyles.empty() )
            {
                // Add UndoAction for creating and inserting the stylesheets to
                // the top of the UndoManager
                SdMoveStyleSheetsUndoAction* pMovStyles = new SdMoveStyleSheetsUndoAction( this, aCreatedStyles, sal_True);
                pUndoMgr->AddUndoAction(pMovStyles);
            }
        }

        // Create layout name based upon the name of the page layout of the
        // master page
        String aPageLayoutName(pMaster->GetLayoutName());
        String aLayoutName = aPageLayoutName;
        aLayoutName.Erase( aLayoutName.SearchAscii( SD_LT_SEPARATOR ));

        // #i121863# Do *not* remove from original document any longer, it is potentially used there
        // and would lead to crashes. Rely on the automatic process of removing unused masterpages
        // (see RemoveUnnecessaryMasterPages)
        //if (pSourceDoc != this)
        //{
        //    // Remove from the source document
        //    pSourceDoc->RemoveMasterPage(pNotesMaster->GetPageNum());
        //    pSourceDoc->RemoveMasterPage(pMaster->GetPageNum());
        //}

        // Register the new master pages with the document and then use the
        // the new presentation layout for the default and notes pages
        if (pSourceDoc != this)
        {
            // Insert the master pages:
            // Insert master pages from new layouts at the end.
            // If a layout is being replaced, however, insert them before the
            // position of the old master page, so from now on the new master
            // page will be found when searching (e.g.
            // SdPage::SetPresentationLayout).
            sal_uInt16 nInsertPos = rOldMaster.GetPageNum();
            BegUndo();

            if (!bLayoutReloaded)
                nInsertPos = 0xFFFF;
            InsertMasterPage(pMaster, nInsertPos);
            if( bUndo )
                AddUndo(GetSdrUndoFactory().CreateUndoNewPage(*pMaster));

            nInsertPos++;
            if (!bLayoutReloaded)
                nInsertPos = 0xFFFF;
            InsertMasterPage(pNotesMaster, nInsertPos);
            if( bUndo )
            {
                AddUndo(GetSdrUndoFactory().CreateUndoNewPage(*pNotesMaster));

                EndUndo(); // do this here already, so Joe's actions happen _between_ our own.
            }
        }

        // Fill list with pages
        std::vector<SdPage*> aPageList;

//      #98456, this has to be removed according to CL (KA 07/08/2002)
//      #109884# but we need them again to restore the styles of the presentation objects while undo
        aPageList.push_back(pMaster);
        aPageList.push_back(pNotesMaster);

        if (bMaster || bLayoutReloaded)
        {
            for (sal_uInt16 nPage = 1; nPage < GetPageCount(); nPage++)
            {
                pPage = (SdPage*) GetPage(nPage);
                String aTest = pPage->GetLayoutName();
                if (aTest == aOldPageLayoutName)
                {
                    aPageList.push_back(pPage);
                }
            }

        }
        else
        {
            aPageList.push_back(pSelectedPage);
            aPageList.push_back(pNotes);
        }

        for (std::vector<SdPage*>::iterator pIter = aPageList.begin(); pIter != aPageList.end(); ++pIter)
        {
            AutoLayout eAutoLayout = (*pIter)->GetAutoLayout();

            if( bUndo )
            {
                SdPresentationLayoutUndoAction * pPLUndoAction =
                    new SdPresentationLayoutUndoAction
                        (this,
                        ( pPage && pPage->IsMasterPage() ) ? aLayoutName : aOldLayoutName,
                        aLayoutName,
                         eAutoLayout, eAutoLayout, sal_False, *pIter);
                pUndoMgr->AddUndoAction(pPLUndoAction);
            }
            (*pIter)->SetPresentationLayout(aLayoutName);
            (*pIter)->SetAutoLayout(eAutoLayout);
        }

        // Adapt new master pages
        if (pSourceDoc != this)
        {
            Size aSize(rOldMaster.GetSize());
            Rectangle aBorderRect(rOldMaster.GetLftBorder(),
                                  rOldMaster.GetUppBorder(),
                                  rOldMaster.GetRgtBorder(),
                                  rOldMaster.GetLwrBorder());
            pMaster->ScaleObjects(aSize, aBorderRect, sal_True);
            pMaster->SetSize(aSize);
            pMaster->SetBorder(rOldMaster.GetLftBorder(),
                               rOldMaster.GetUppBorder(),
                               rOldMaster.GetRgtBorder(),
                               rOldMaster.GetLwrBorder());
            pMaster->SetOrientation( rOldMaster.GetOrientation() );
            pMaster->SetAutoLayout(pMaster->GetAutoLayout());

            aSize = rOldNotesMaster.GetSize();
            Rectangle aNotesBorderRect(rOldNotesMaster.GetLftBorder(),
                                       rOldNotesMaster.GetUppBorder(),
                                       rOldNotesMaster.GetRgtBorder(),
                                       rOldNotesMaster.GetLwrBorder());
            pNotesMaster->ScaleObjects(aSize, aNotesBorderRect, sal_True);
            pNotesMaster->SetSize(aSize);
            pNotesMaster->SetBorder(rOldNotesMaster.GetLftBorder(),
                                    rOldNotesMaster.GetUppBorder(),
                                    rOldNotesMaster.GetRgtBorder(),
                                    rOldNotesMaster.GetLwrBorder());
            pNotesMaster->SetOrientation( rOldNotesMaster.GetOrientation() );
            pNotesMaster->SetAutoLayout(pNotesMaster->GetAutoLayout());

            if( (pSourceDoc->GetDocumentType() == DOCUMENT_TYPE_IMPRESS) &&
                (GetDocumentType() == DOCUMENT_TYPE_DRAW) )
            {
                pMaster->RemoveEmptyPresentationObjects();
                pNotesMaster->RemoveEmptyPresentationObjects();
            }
        }
    }
    else
    {
        // Find a new name for the layout
        OUString aName(createNewMasterPageLayoutName(*this));
        String aPageLayoutName(aName);
        aPageLayoutName.AppendAscii( SD_LT_SEPARATOR );
        aPageLayoutName += SD_RESSTR(STR_LAYOUT_OUTLINE);

        // Generate new stylesheets
        static_cast<SdStyleSheetPool*>( mxStyleSheetPool.get())->CreateLayoutStyleSheets(aName);
        SdStyleSheetVector aCreatedStyles;
        static_cast<SdStyleSheetPool*>( mxStyleSheetPool.get())->CreateLayoutSheetList(aName, aCreatedStyles);

        if( bUndo )
        {
            SdMoveStyleSheetsUndoAction* pMovStyles = new SdMoveStyleSheetsUndoAction(this, aCreatedStyles, sal_True);
            pUndoMgr->AddUndoAction(pMovStyles);
        }

        // Generate new master pages and register them with the document
        if( bUndo )
            BegUndo();

        pMaster = (SdPage*) AllocPage(sal_True);
        pMaster->SetSize(pSelectedPage->GetSize());
        pMaster->SetBorder(pSelectedPage->GetLftBorder(),
                           pSelectedPage->GetUppBorder(),
                           pSelectedPage->GetRgtBorder(),
                           pSelectedPage->GetLwrBorder() );
        pMaster->SetName(aName);
        pMaster->SetLayoutName(aPageLayoutName);
        InsertMasterPage(pMaster);

        if( bUndo )
            AddUndo(GetSdrUndoFactory().CreateUndoNewPage(*pMaster));

        pMaster->SetAutoLayout(AUTOLAYOUT_NONE, true, true);

        pNotesMaster = (SdPage*) AllocPage(sal_True);
        pNotesMaster->SetPageKind(PK_NOTES);
        pNotesMaster->SetSize(pNotes->GetSize());
        pNotesMaster->SetBorder(pNotes->GetLftBorder(),
                                pNotes->GetUppBorder(),
                                pNotes->GetRgtBorder(),
                                pNotes->GetLwrBorder() );
        pNotesMaster->SetName(aName);
        pNotesMaster->SetLayoutName(aPageLayoutName);
        InsertMasterPage(pNotesMaster);

        if( bUndo )
            AddUndo(GetSdrUndoFactory().CreateUndoNewPage(*pNotesMaster));

        pNotesMaster->SetAutoLayout(AUTOLAYOUT_NOTES, true, true);

        if( bUndo )
            EndUndo();

        // Create a list of affected default and notes pages
        std::vector<SdPage*> aPageList;
        if (bMaster)
        {
            for (sal_uInt16 nPage = 1; nPage < GetPageCount(); nPage++)
            {
                pPage = (SdPage*) GetPage(nPage);
                const String s(pPage->GetLayoutName());
                if(s == aOldPageLayoutName)
                {
                    aPageList.push_back(pPage);
                }
            }
        }
        else
        {
            aPageList.push_back(pSelectedPage);
            aPageList.push_back(pNotes);
        }

        // Set presentation layout and AutoLayout for the affected pages
        for ( std::vector<SdPage*>::iterator pIter = aPageList.begin(); pIter != aPageList.end(); ++pIter )
        {
            AutoLayout eOldAutoLayout = (*pIter)->GetAutoLayout();
            AutoLayout eNewAutoLayout =
                (*pIter)->GetPageKind() == PK_STANDARD ? AUTOLAYOUT_NONE : AUTOLAYOUT_NOTES;

            if( bUndo )
            {
                SdPresentationLayoutUndoAction * pPLUndoAction =
                    new SdPresentationLayoutUndoAction
                            (this, aOldLayoutName, aName,
                             eOldAutoLayout, eNewAutoLayout, sal_True,
                             *pIter);
                pUndoMgr->AddUndoAction(pPLUndoAction);
            }

            (*pIter)->SetPresentationLayout(aName);
            (*pIter)->SetAutoLayout(eNewAutoLayout);
        }
    }

    // If the old master pages aren't used anymore, they and their styles have
    // to be removed.
    if (bCheckMasters)
    {
        // Check all
        RemoveUnnecessaryMasterPages();
    }
    else
    {
        // Check only the master page that was replaced
        RemoveUnnecessaryMasterPages(&rOldMaster);
    }

    if( bUndo )
        pUndoMgr->LeaveListAction();

    if( mpDocSh )
        mpDocSh->SetWaitCursor( sal_False );
}



void SdDrawDocument::Merge(SdrModel& rSourceModel,
               sal_uInt16 nFirstPageNum, sal_uInt16 nLastPageNum,
               sal_uInt16 nDestPos,
               bool bMergeMasterPages, bool bAllMasterPages,
               bool bUndo, bool bTreadSourceAsConst)
{
    sal_uInt16 nMasterPageCount = GetMasterPageCount();
    SdrModel::Merge( rSourceModel, nFirstPageNum, nLastPageNum, nDestPos, bMergeMasterPages, bAllMasterPages, bUndo, bTreadSourceAsConst );

    // add style family for each new master page
    for( sal_uInt16 nMaster = nMasterPageCount; nMaster < GetMasterPageCount(); nMaster++ )
    {
        SdPage* pPage = static_cast< SdPage* >( GetMasterPage( nMaster ) );
        if( pPage && pPage->IsMasterPage() && (pPage->GetPageKind() == PK_STANDARD) )
        {
            // new master page created, add its style family
            SdStyleSheetPool* pStylePool = (SdStyleSheetPool*) GetStyleSheetPool();
            if( pStylePool )
                pStylePool->AddStyleFamily( pPage );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
