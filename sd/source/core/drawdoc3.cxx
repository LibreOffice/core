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
#include <vcl/layout.hxx>
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
#include "sdxfer.hxx"

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
    explicit InsertBookmarkAsPage_FindDuplicateLayouts( std::vector<OUString> &rLayoutsToTransfer )
        : mrLayoutsToTransfer(rLayoutsToTransfer) {}
    void operator()( SdDrawDocument&, SdPage*, bool, SdDrawDocument* );
private:
    std::vector<OUString> &mrLayoutsToTransfer;
};

void InsertBookmarkAsPage_FindDuplicateLayouts::operator()( SdDrawDocument& rDoc, SdPage* pBMMPage, bool bRenameDuplicates, SdDrawDocument* pBookmarkDoc )
{
    // now check for duplicate masterpage and layout names

    OUString aFullNameLayout( pBMMPage->GetLayoutName() );
    sal_Int32 nIndex = aFullNameLayout.indexOf( SD_LT_SEPARATOR );
    if( nIndex != -1 )
        aFullNameLayout = aFullNameLayout.copy(0, nIndex);

    OUString aLayout(aFullNameLayout);

    std::vector<OUString>::const_iterator pIter =
            find(mrLayoutsToTransfer.begin(),mrLayoutsToTransfer.end(),aLayout);

    bool bFound = pIter != mrLayoutsToTransfer.end();

    const sal_uInt16 nMPageCount = rDoc.GetMasterPageCount();
    for (sal_uInt16 nMPage = 0; nMPage < nMPageCount && !bFound; nMPage++)
    {
        // Do the layouts already exist within the document?
        SdPage* pTestPage = static_cast<SdPage*>( rDoc.GetMasterPage(nMPage) );
        OUString aFullTest(pTestPage->GetLayoutName());
        sal_Int32 nIndex2 = aFullTest.indexOf( SD_LT_SEPARATOR );
        if( nIndex2 != -1 )
            aFullTest = aFullTest.copy(0, nIndex2);

        OUString aTest(aFullTest);

        if (aTest == aLayout && pBMMPage->GetPageKind() == pTestPage->GetPageKind())
        {
            // Ignore Layouts with "Default" these seem to be special - in the sense that there are lot of assumption all over Impress
            // about this
            if( bRenameDuplicates && aTest != OUString( SdResId( STR_LAYOUT_DEFAULT_NAME ) ) && pTestPage->stringify() != pBMMPage->stringify() )
            {
                pBookmarkDoc->RenameLayoutTemplate(
                    pBMMPage->GetLayoutName(), pBMMPage->GetName() + "_");
                aLayout = pBMMPage->GetName();

                break;
            }
            else
                bFound = true;
        }
    }

    if (!bFound)
        mrLayoutsToTransfer.push_back(aLayout);
}

// Inserts a bookmark as a page
static void lcl_IterateBookmarkPages( SdDrawDocument &rDoc, SdDrawDocument* pBookmarkDoc,
                               const std::vector<OUString> &rBookmarkList, sal_uInt16 nBMSdPageCount,
                               InsertBookmarkAsPage_FindDuplicateLayouts& rPageIterator, bool bRenameDuplicates = false )
{

    // Refactored copy'n'pasted layout name collection from InsertBookmarkAsPage

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
        SdPage* pBMMPage = nullptr;

        if( rBookmarkList.empty() )
        {
            // simply take master page of nPos'th page in source document
            pBMMPage = static_cast<SdPage*>(&(pBookmarkDoc->GetSdPage((sal_uInt16)nPos, PK_STANDARD)->TRG_GetMasterPage()));
        }
        else
        {
            // fetch nPos'th entry from bookmark list, and determine master page
            OUString aBMPgName(rBookmarkList[nPos]);
            bool  bIsMasterPage;

            sal_uInt16 nBMPage = pBookmarkDoc->GetPageByName( aBMPgName, bIsMasterPage );

            if (nBMPage != SDRPAGE_NOTFOUND)
            {
                pBMPage = static_cast<SdPage*>( pBookmarkDoc->GetPage(nBMPage) );
            }
            else
            {
                pBMPage = nullptr;
            }

            // enforce that bookmarked page is a standard page and not already a master page
            if (pBMPage && pBMPage->GetPageKind()==PK_STANDARD && !pBMPage->IsMasterPage())
            {
                const sal_uInt16 nBMSdPage = (nBMPage - 1) / 2;
                pBMMPage = static_cast<SdPage*> (&(pBookmarkDoc->GetSdPage(nBMSdPage, PK_STANDARD)->TRG_GetMasterPage()));
            }
        }

        // successfully determined valid (bookmarked) page?
        if( pBMMPage )
        {
            // yes, call functor
            rPageIterator( rDoc, pBMMPage, bRenameDuplicates, pBookmarkDoc );
        }
    }
}

// Opens a bookmark document
SdDrawDocument* SdDrawDocument::OpenBookmarkDoc(SfxMedium* pMedium)
{
    bool bOK = true;
    SdDrawDocument* pBookmarkDoc = nullptr;
    OUString aBookmarkName = pMedium->GetName();
    std::shared_ptr<const SfxFilter> pFilter = pMedium->GetFilter();
    if ( !pFilter )
    {
        pMedium->UseInteractionHandler( true );
        SfxGetpApp()->GetFilterMatcher().GuessFilter(*pMedium, pFilter);
    }

    if ( !pFilter )
    {
        bOK = false;
    }
    else if ( !aBookmarkName.isEmpty() && maBookmarkFile != aBookmarkName )
    {
        bool bCreateGraphicShell = pFilter->GetServiceName() == "com.sun.star.drawing.DrawingDocument";
        bool bCreateImpressShell = pFilter->GetServiceName() == "com.sun.star.presentation.PresentationDocument";
        if ( bCreateGraphicShell || bCreateImpressShell )
        {
            CloseBookmarkDoc();

            // Create a DocShell, as OLE objects might be contained in the
            // document. (Persist)
            // If that wasn't the case, we could load the model directly.
            if ( bCreateGraphicShell )
                // Draw
                mxBookmarkDocShRef = new ::sd::GraphicDocShell(SfxObjectCreateMode::STANDARD, true);
            else
                // Impress
                mxBookmarkDocShRef = new ::sd::DrawDocShell(SfxObjectCreateMode::STANDARD, true);

            bOK = mxBookmarkDocShRef->DoLoad(pMedium);
            if( bOK )
            {
                maBookmarkFile = aBookmarkName;
                pBookmarkDoc = mxBookmarkDocShRef->GetDoc();
            }
        }
    }

    DBG_ASSERT(!aBookmarkName.isEmpty(), "Empty document name!");

    if (!bOK)
    {
        ScopedVclPtrInstance< MessageDialog > aErrorBox(nullptr, SD_RESSTR(STR_READ_DATA_ERROR));
        aErrorBox->Execute();

        CloseBookmarkDoc();
        pBookmarkDoc = nullptr;
    }
    else if (mxBookmarkDocShRef.Is())
    {
        pBookmarkDoc = mxBookmarkDocShRef->GetDoc();
    }

    return pBookmarkDoc;
}

// Opens a bookmark document
SdDrawDocument* SdDrawDocument::OpenBookmarkDoc(const OUString& rBookmarkFile)
{
    SdDrawDocument* pBookmarkDoc = nullptr;

    if (!rBookmarkFile.isEmpty() && maBookmarkFile != rBookmarkFile)
    {
        std::unique_ptr<SfxMedium> xMedium(new SfxMedium(rBookmarkFile, StreamMode::READ));
        pBookmarkDoc = OpenBookmarkDoc(xMedium.release());
    }
    else if (mxBookmarkDocShRef.Is())
    {
        pBookmarkDoc = mxBookmarkDocShRef->GetDoc();
    }

    return pBookmarkDoc;
}

// Inserts a bookmark (page or object)
void SdDrawDocument::InsertBookmark(
    const std::vector<OUString> &rBookmarkList,     // List of names of the bookmarks to be inserted
    std::vector<OUString> &rExchangeList,           // List of the names to be used
    bool bLink,                                     // Insert bookmarks as links?
    bool bReplace,                                  // Replace current default and notes pages?
    sal_uInt16 nInsertPos,                          // Insertion position of pages
    ::sd::DrawDocShell* pBookmarkDocSh,             // If set, this is the source document
    Point* pObjPos)                                 // Insertion position of objects
{
    bool bOK = true;
    bool bInsertPages = false;

    if (rBookmarkList.empty())
    {
        // Insert all pages
        bInsertPages = true;
    }
    else
    {
        SdDrawDocument* pBookmarkDoc = nullptr;

        if (pBookmarkDocSh)
        {
            pBookmarkDoc = pBookmarkDocSh->GetDoc();
        }
        else if ( mxBookmarkDocShRef.Is() )
        {
            pBookmarkDoc = mxBookmarkDocShRef->GetDoc();
        }
        else
            bOK = false;

        std::vector<OUString>::const_iterator pIter;
        for ( pIter = rBookmarkList.begin(); bOK && pIter != rBookmarkList.end() && !bInsertPages; ++pIter )
        {
            // Is there a page name in the bookmark list?
            bool    bIsMasterPage;
            if( pBookmarkDoc->GetPageByName( *pIter, bIsMasterPage ) != SDRPAGE_NOTFOUND )
            {
                // Found the page
                bInsertPages = true;
            }
        }
    }

    bool bCalcObjCount = !rExchangeList.empty();

    if ( bOK && bInsertPages )
    {
        // Insert all page bookmarks
        bOK = InsertBookmarkAsPage(rBookmarkList, &rExchangeList, bLink, bReplace,
                                   nInsertPos, false/*bNoDialogs*/, pBookmarkDocSh, true/*bCopy*/, true, false);
    }

    if ( bOK && !rBookmarkList.empty() )
    {
        // Insert all object bookmarks
        bOK = InsertBookmarkAsObject(rBookmarkList, rExchangeList, bLink,
                                     pBookmarkDocSh, pObjPos, bCalcObjCount);
    }
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

SfxStyleSheet *lcl_findStyle(SdStyleSheetVector& rStyles, const OUString& aStyleName)
{
    for(SdStyleSheetVector::const_iterator aIt(rStyles.begin()), aLast(rStyles.end()); aIt != aLast; ++aIt)
    {
        if((*aIt)->GetName().startsWith(aStyleName))
            return (*aIt).get();
    }
    return nullptr;
}

}

bool SdDrawDocument::InsertBookmarkAsPage(
    const std::vector<OUString> &rBookmarkList,
    std::vector<OUString> *pExchangeList,            // List of names to be used
    bool bLink,
    bool bReplace,
    sal_uInt16 nInsertPos,
    bool bNoDialogs,
    ::sd::DrawDocShell* pBookmarkDocSh,
    bool bCopy,
    bool bMergeMasterPages,
    bool bPreservePageNames)
{
    bool bContinue = true;
    bool bScaleObjects = false;
    sal_uInt16 nReplacedStandardPages = 0;

    SdDrawDocument* pBookmarkDoc = nullptr;
    OUString aBookmarkName;

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
        return false;
    }

    const sal_uInt16 nSdPageCount = GetSdPageCount(PK_STANDARD);
    const sal_uInt32 nBMSdPageCount = pBookmarkDoc->GetSdPageCount(PK_STANDARD);
    const sal_uInt16 nMPageCount = GetMasterPageCount();

    if (nSdPageCount==0 || nBMSdPageCount==0 || nMPageCount==0)
    {
        return false;
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
    Orientation eNOrient = pNPage->GetOrientation();

    // Adapt page size and margins to those of the later pages?
    pRefPage = GetSdPage(nSdPageCount - 1, PK_STANDARD);

    if( bNoDialogs )
    {
        // If this is clipboard, then no need to scale objects:
        // this will make copied masters to differ from the originals,
        // and thus InsertBookmarkAsPage_FindDuplicateLayouts will
        // duplicate masters on insert to same document
        bTransportContainer = (SD_MOD()->pTransferClip &&
                               SD_MOD()->pTransferClip->GetWorkDocument() == this);
        if (!bTransportContainer)
        {
            if (rBookmarkList.empty())
                bScaleObjects = pRefPage->IsScaleObjects();
            else
                bScaleObjects = true;
        }
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
            OUString aStr(SD_RESSTR(STR_SCALE_OBJECTS));
            sal_uInt16 nBut = ScopedVclPtr<QueryBox>::Create( nullptr, WB_YES_NO_CANCEL, aStr)->Execute();

            bScaleObjects = nBut == RET_YES;
            bContinue     = nBut != RET_CANCEL;

            if (!bContinue)
            {
                return bContinue;
            }
        }
    }

    // Get the necessary presentation stylesheets and transfer them before
    // the pages, else, the text objects won't reference their styles anymore.
    ::svl::IUndoManager* pUndoMgr = nullptr;
    if( mpDocSh )
    {
        pUndoMgr = mpDocSh->GetUndoManager();
        pUndoMgr->EnterListAction(SD_RESSTR(STR_UNDO_INSERTPAGES), "");
    }

    // Refactored copy'n'pasted layout name collection into IterateBookmarkPages

    std::vector<OUString> aLayoutsToTransfer;
    InsertBookmarkAsPage_FindDuplicateLayouts aSearchFunctor( aLayoutsToTransfer );
    lcl_IterateBookmarkPages( *this, pBookmarkDoc, rBookmarkList, nBMSdPageCount, aSearchFunctor, ( rBookmarkList.empty() && pBookmarkDoc != this ) );

    // Copy the style that we actually need.
    SdStyleSheetPool& rBookmarkStyleSheetPool = dynamic_cast<SdStyleSheetPool&>(*pBookmarkDoc->GetStyleSheetPool());
    SdStyleSheetPool& rStyleSheetPool = dynamic_cast<SdStyleSheetPool&>(*GetStyleSheetPool());

    // When copying styles, also copy the master pages!
    if( !aLayoutsToTransfer.empty() )
        bMergeMasterPages = true;

    std::vector<OUString>::const_iterator pIter;
    for ( pIter = aLayoutsToTransfer.begin(); pIter != aLayoutsToTransfer.end(); ++pIter )
    {
        SdStyleSheetVector aCreatedStyles;
        OUString layoutName = *pIter;

        rStyleSheetPool.CopyLayoutSheets(layoutName, rBookmarkStyleSheetPool,aCreatedStyles);

        if(!aCreatedStyles.empty())
        {
            if( pUndoMgr )
            {
                SdMoveStyleSheetsUndoAction* pMovStyles = new SdMoveStyleSheetsUndoAction(this, aCreatedStyles, true);
                pUndoMgr->AddUndoAction(pMovStyles);
            }
        }
    }

    // Copy styles. This unconditionally copies all styles, even those
    // that are not used in any of the inserted pages. The unused styles
    // are then removed at the end of the function, where we also create
    // undo records for the inserted styles.
    SdStyleSheetVector aNewGraphicStyles;
    OUString aRenameStr;
    if(!bReplace && !bNoDialogs)
        aRenameStr = "_";
    rStyleSheetPool.RenameAndCopyGraphicSheets(rBookmarkStyleSheetPool, aNewGraphicStyles, aRenameStr);
    SdStyleSheetVector aNewCellStyles;
    rStyleSheetPool.CopyCellSheets(rBookmarkStyleSheetPool, aNewCellStyles);

    // TODO handle undo of table styles too
    rStyleSheetPool.CopyTableStyles(rBookmarkStyleSheetPool);

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
            OUString sName(pBMPage->GetName());
            bool    bIsMasterPage;

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
              false,             // But only the master pages used
              true,              // Create an undo action
              bCopy);            // Copy (or merge) pages?

        for (nBMSdPage=0; nBMSdPage < nBMSdPageCount; nBMSdPage++)
        {
            SdPage* pPage       = static_cast<SdPage*>( GetPage(nActualInsertPos) );
            SdPage* pNotesPage  = static_cast<SdPage*>( GetPage(nActualInsertPos+1) );

            // delay renaming *after* pages are copied (might destroy source otherwise)
            if( aRenameSet.find(nBMSdPage) != aRenameSet.end() )
            {
                // Page name already in use -> Use default name for default and
                // notes page
                pPage->SetName(OUString());
                pNotesPage->SetName(OUString());
            }

            if (bLink)
            {
                OUString aName(aNameMap[nBMSdPage]);

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
            bReplace = false;
            nInsertPos = GetPageCount();
        }

        sal_uInt16 nActualInsertPos = nInsertPos;

        // Collect the bookmarked pages
        ::std::vector<SdPage*> aBookmarkedPages (rBookmarkList.size(), nullptr);
        for ( size_t nPos = 0, n = rBookmarkList.size(); nPos < n; ++nPos)
        {
            OUString aPgName(rBookmarkList[nPos]);
            bool    bIsMasterPage;
            sal_uInt16  nBMPage = pBookmarkDoc->GetPageByName( aPgName, bIsMasterPage );

            if (nBMPage != SDRPAGE_NOTFOUND)
            {
                aBookmarkedPages[nPos] =  dynamic_cast<SdPage*>(pBookmarkDoc->GetPage(nBMPage));
            }
        }

        for ( size_t nPos = 0, n = rBookmarkList.size(); nPos < n; ++nPos)
        {
            pBMPage = aBookmarkedPages[nPos];
            sal_uInt16 nBMPage = pBMPage!=nullptr ? pBMPage->GetPageNum() : SDRPAGE_NOTFOUND;

            if (pBMPage && pBMPage->GetPageKind()==PK_STANDARD && !pBMPage->IsMasterPage())
            {
                // It has to be a default page
                bool bMustRename = false;

                // delay renaming *after* pages are copied (might destroy source otherwise)
                // don't change name if source and dest model are the same!
                // avoid renaming if replacing the same page
                OUString aPgName(rBookmarkList[nPos]);
                bool    bIsMasterPage;
                sal_uInt16 nPageSameName = GetPageByName(aPgName, bIsMasterPage);
                if( pBookmarkDoc != this &&
                    nPageSameName != SDRPAGE_NOTFOUND &&
                    ( !bReplace ||
                      nPageSameName != nActualInsertPos ) )
                {
                    bMustRename = true;
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
                      false,             // But only the master pages used
                      true,              // Create undo action
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
                    SdPage* pPage = static_cast<SdPage*>( GetPage(nActualInsertPos) );
                    pPage->SetName(OUString());
                    SdPage* pNotesPage = static_cast<SdPage*>( GetPage(nActualInsertPos+1) );
                    pNotesPage->SetName(OUString());
                }

                if (bLink)
                {
                    SdPage* pPage = static_cast<SdPage*>( GetPage(nActualInsertPos) );
                    pPage->SetFileName(aBookmarkName);
                    pPage->SetBookmarkName(aPgName);
                    pPage->SetModel(this);
                }

                if (bReplace)
                {
                    // Remove page and notes page.
                    const sal_uInt16 nDestPageNum(nActualInsertPos + 2);
                    SdPage* pStandardPage = nullptr;

                    if(nDestPageNum < GetPageCount())
                    {
                        pStandardPage = static_cast<SdPage*>(GetPage(nDestPageNum));
                    }

                    if (pStandardPage)
                    {
                        if( bPreservePageNames )
                        {
                            // Take old slide names for inserted pages
                            SdPage* pPage = static_cast<SdPage*>( GetPage(nActualInsertPos) );
                            pPage->SetName( pStandardPage->GetRealName() );
                        }

                        if( bUndo )
                            AddUndo(GetSdrUndoFactory().CreateUndoDeletePage(*pStandardPage));

                        RemovePage(nDestPageNum);

                        if( !bUndo )
                            delete pStandardPage;
                    }

                    SdPage* pNotesPage = nullptr;

                    if(nDestPageNum < GetPageCount())
                    {
                        pNotesPage = static_cast<SdPage*>(GetPage(nDestPageNum));
                    }

                    if (pNotesPage)
                    {
                        if( bPreservePageNames )
                        {
                            // Take old slide names for inserted pages
                            SdPage* pNewNotesPage = static_cast<SdPage*>( GetPage(nActualInsertPos+1));
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
        pRefPage = static_cast<SdPage*>( GetMasterPage(nPage) );
        OUString aMPLayout(pRefPage->GetLayoutName());
        PageKind eKind = pRefPage->GetPageKind();

        // Does this already exist?
        for (sal_uInt16 nTest = 0; nTest < nMPageCount; nTest++)
        {
            SdPage* pTest = static_cast<SdPage*>( GetMasterPage(nTest) );
            OUString aTest(pTest->GetLayoutName());

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
        const bool bRemoveEmptyPresObj =
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
                OUString aExchangeName(*pExchangeIter);
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

            OUString aLayout(pRefPage->GetLayoutName());
            sal_Int32 nIndex = aLayout.indexOf( SD_LT_SEPARATOR );
            if( nIndex != -1 )
                aLayout = aLayout.copy(0, nIndex);

            // update layout and referred master page
            pRefPage->SetPresentationLayout(aLayout);
            if( bUndo )
                AddUndo( GetSdrUndoFactory().CreateUndoPageChangeMasterPage( *pRefPage ) );

            if (bScaleObjects)
            {
                Rectangle aBorderRect(nLeft, nUpper, nRight, nLower);
                pRefPage->ScaleObjects(aSize, aBorderRect, true);
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
                pRefPage->ScaleObjects(aNSize, aBorderRect, true);
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
            pRefPage = static_cast<SdPage*>( GetMasterPage(nPage) );
            if (pRefPage->GetPageKind() == PK_STANDARD)
            {
                if (bScaleObjects)
                {
                    Rectangle aBorderRect(nLeft, nUpper, nRight, nLower);
                    pRefPage->ScaleObjects(aSize, aBorderRect, true);
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
                    pRefPage->ScaleObjects(aNSize, aBorderRect, true);
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
    RemoveUnnecessaryMasterPages(nullptr, true);

    // Rename object styles if necessary
    if(!aRenameStr.isEmpty())
    {
        try
        {
            for(sal_uInt32 p = nInsertPos; p < (nInsertPos + nBMSdPageCount); p++)
            {
                SdPage *pPg = static_cast<SdPage *>( GetPage(p) );
                for(size_t i = 0; pPg && (i < pPg->GetObjCount()); ++i)
                {
                    if(pPg->GetObj(i)->GetStyleSheet())
                    {
                        OUString aStyleName = pPg->GetObj(i)->GetStyleSheet()->GetName();
                        SfxStyleSheet *pSheet = lcl_findStyle(aNewGraphicStyles, aStyleName + aRenameStr);
                        if(pSheet != nullptr)
                            pPg->GetObj(i)->SetStyleSheet(pSheet, true);
                    }
                }
            }
        }
        catch(...)
        {
            OSL_FAIL("Exception while renaming styles @ SdDrawDocument::InsertBookmarkAsPage");
        }
    }
    // remove copied styles not used on any inserted page and create
    // undo records
    // WARNING: SdMoveStyleSheetsUndoAction clears the passed list of
    // styles, so it cannot be used after this point
    lcl_removeUnusedStyles(GetStyleSheetPool(), aNewGraphicStyles);
    if (!aNewGraphicStyles.empty() && pUndoMgr)
        pUndoMgr->AddUndoAction(new SdMoveStyleSheetsUndoAction(this, aNewGraphicStyles, true));
    lcl_removeUnusedStyles(GetStyleSheetPool(), aNewCellStyles);
    if (!aNewCellStyles.empty() && pUndoMgr)
        pUndoMgr->AddUndoAction(new SdMoveStyleSheetsUndoAction(this, aNewCellStyles, true));

    if( bUndo )
        EndUndo();

    if (pUndoMgr)
        pUndoMgr->LeaveListAction();

    return bContinue;
}

// Inserts a bookmark as an object
bool SdDrawDocument::InsertBookmarkAsObject(
    const std::vector<OUString> &rBookmarkList,
    const std::vector<OUString> &rExchangeList,            // List of names to use
    bool /* bLink */,
    ::sd::DrawDocShell* pBookmarkDocSh,
    Point* pObjPos,
    bool bCalcObjCount)
{
    bool bOK = true;
    bool bOLEObjFound = false;
    ::sd::View* pBMView = nullptr;

    SdDrawDocument* pBookmarkDoc = nullptr;

    if (pBookmarkDocSh)
    {
        pBookmarkDoc = pBookmarkDocSh->GetDoc();
    }
    else if ( mxBookmarkDocShRef.Is() )
    {
        pBookmarkDoc = mxBookmarkDocShRef->GetDoc();
    }
    else
    {
        return false;
    }

    if (rBookmarkList.empty())
    {
        pBMView = new ::sd::View(*pBookmarkDoc, nullptr);
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
            SdrObject* pObj = pBookmarkDoc->GetObj(*pIter);

            if (pObj)
            {
                // Found an object
                if (pObj->GetObjInventor() == SdrInventor &&
                    pObj->GetObjIdentifier() == OBJ_OLE2)
                {
                    bOLEObjFound = true;
                }

                if (!pBMView)
                {
                    // Create View for the first time
                    pBMView = new ::sd::View(*pBookmarkDoc, nullptr);
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

                pBMView->MarkObj(pObj, pPV);
            }
        }
    }

    if (pBMView)
    {
        // Insert selected objects
        ::sd::View* pView = new ::sd::View(*this, nullptr);
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

        size_t nCountBefore = 0;

        if (!rExchangeList.empty() || bCalcObjCount)
        {
            // Sort OrdNums and get the number of objects before inserting
            pPage->RecalcObjOrdNums();
            nCountBefore = pPage->GetObjCount();
        }

        if (bOLEObjFound)
            pBMView->GetDoc().SetAllocDocSh(true);

        SdDrawDocument* pTmpDoc = static_cast<SdDrawDocument*>( pBMView->GetMarkedObjModel() );
        bOK = pView->Paste(*pTmpDoc, aObjPos, pPage, SdrInsertFlags::NONE);

        if (bOLEObjFound)
            pBMView->GetDoc().SetAllocDocSh(false);

        if (!bOLEObjFound)
            delete pTmpDoc;         // Would otherwise be destroyed by DocShell

        delete pView;

        if (!rExchangeList.empty())
        {
            // Get number of objects after inserting.
            const size_t nCount = pPage->GetObjCount();

            std::vector<OUString>::const_iterator pIter = rExchangeList.begin();
            for (size_t nObj = nCountBefore; nObj < nCount; ++nObj)
            {
                // Get the name to use from the Exchange list
                if (pIter != rExchangeList.end())
                {
                    if (pPage->GetObj(nObj))
                    {
                        pPage->GetObj(nObj)->SetName(*pIter);
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
    maBookmarkFile.clear();
}

// Is this document read-only?
bool SdDrawDocument::IsReadOnly() const
{
    return false;
}

// In the subsequent AllocModel() a DocShell (xAllocedDocShRef) is created.
// Any pre-existing DocShell is deleted
void SdDrawDocument::SetAllocDocSh(bool bAlloc)
{
    mbAllocDocSh = bAlloc;

    if(mxAllocedDocShRef.Is())
    {
        mxAllocedDocShRef->DoClose();
    }

    mxAllocedDocShRef.Clear();
}

// Return list of CustomShows (create it, too, if necessary)
SdCustomShowList* SdDrawDocument::GetCustomShowList(bool bCreate)
{
    if (!mpCustomShowList && bCreate)
    {
        mpCustomShowList = new SdCustomShowList;
    }

    return mpCustomShowList;
}

// Remove unused master pages and layouts
void SdDrawDocument::RemoveUnnecessaryMasterPages(SdPage* pMasterPage, bool bOnlyDuplicatePages, bool bUndo)
{
    ::sd::View* pView = nullptr;
    ::svl::IUndoManager* pUndoMgr = nullptr;

    if( bUndo && !IsUndoEnabled() )
        bUndo = false;

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
        SdPage* pNotesMaster = nullptr;

        if (!pMaster)
        {
            pMaster = GetMasterSdPage( (sal_uInt16) nMPage, PK_STANDARD );
            pNotesMaster = GetMasterSdPage( (sal_uInt16) nMPage, PK_NOTES );
        }
        else
        {
            for ( sal_uInt16 nMPg = 0; nMPg < GetMasterPageCount(); nMPg++ )
            {
                if ( pMaster == GetMasterPage( nMPg ) )
                {
                    pNotesMaster = static_cast<SdPage*>( GetMasterPage( ++nMPg ) );
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
            bool bDeleteMaster = !pMaster->IsPrecious();
            OUString aLayoutName = pMaster->GetLayoutName();

            if(bOnlyDuplicatePages )
            {
                // remove only duplicate pages
                bDeleteMaster = false;
                for (sal_uInt16 i = 0; i < GetMasterSdPageCount( PK_STANDARD ); i++)
                {
                    SdPage* pMPg = GetMasterSdPage( i, PK_STANDARD );
                    if( pMPg != pMaster &&
                        pMPg->GetLayoutName() == aLayoutName )
                    {
                        // duplicate page found -> remove it
                        bDeleteMaster = true;
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
                bool bDeleteOldStyleSheets = true;
                for ( sal_uInt16 nMPg = 0;
                          nMPg < GetMasterPageCount() && bDeleteOldStyleSheets;
                       nMPg++ )
                {
                    SdPage* pMPg = static_cast<SdPage*>( GetMasterPage(nMPg) );
                    if (pMPg->GetLayoutName() == aLayoutName)
                    {
                        bDeleteOldStyleSheets = false;
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
bool isMasterPageLayoutNameUnique(const SdDrawDocument& rDoc, const OUString& rCandidate)
{
    if (rCandidate.isEmpty())
    {
        return false;
    }

    const sal_uInt16 nPageCount(rDoc.GetMasterPageCount());

    for(sal_uInt16 a(0); a < nPageCount; a++)
    {
        const SdrPage* pCandidate = rDoc.GetMasterPage(a);
        OUString aPageLayoutName(pCandidate->GetLayoutName());
        sal_Int32 nIndex = aPageLayoutName.indexOf(SD_LT_SEPARATOR);
        if( nIndex != -1 )
            aPageLayoutName = aPageLayoutName.copy(0, nIndex);

        if(aPageLayoutName == rCandidate)
        {
            return false;
        }
    }

    return true;
}

// #i121863# factored out functinality
OUString createNewMasterPageLayoutName(const SdDrawDocument& rDoc)
{
    const OUString aBaseName(SdResId(STR_LAYOUT_DEFAULT_NAME));
    OUString aRetval;
    sal_uInt16 nCount(0);

    while (aRetval.isEmpty())
    {
        aRetval = aBaseName;

        if(nCount)
        {
            aRetval += OUString::number(nCount);
        }

        nCount++;

        if(!isMasterPageLayoutNameUnique(rDoc, aRetval))
        {
            aRetval.clear();
        }
    }

    return aRetval;
}

void SdDrawDocument::SetMasterPage(sal_uInt16 nSdPageNum,
                                   const OUString& rLayoutName,
                                   SdDrawDocument* pSourceDoc,
                                   bool bMaster,
                                   bool bCheckMasters)
{
    ::svl::IUndoManager* pUndoMgr = nullptr;

    if( mpDocSh )
    {
        mpDocSh->SetWaitCursor( true );
        pUndoMgr = mpDocSh->GetUndoManager();
    }

    const bool bUndo = pUndoMgr && IsUndoEnabled();

    if (bUndo)
    {
        pUndoMgr->EnterListAction(SD_RESSTR(STR_UNDO_SET_PRESLAYOUT), OUString());
    }

    SdPage* pSelectedPage   = GetSdPage(nSdPageNum, PK_STANDARD);
    SdPage* pNotes          = static_cast<SdPage*>( GetPage(pSelectedPage->GetPageNum()+1) );
    SdPage& rOldMaster      = static_cast<SdPage&>(pSelectedPage->TRG_GetMasterPage());
    SdPage& rOldNotesMaster = static_cast<SdPage&>(pNotes->TRG_GetMasterPage());
    SdPage* pMaster         = nullptr;
    SdPage* pNotesMaster    = nullptr;
    OUString aOldPageLayoutName(pSelectedPage->GetLayoutName());
    OUString aOldLayoutName(aOldPageLayoutName);
    sal_Int32 nIndex = aOldLayoutName.indexOf( SD_LT_SEPARATOR );
    if( nIndex != -1 )
        aOldLayoutName = aOldLayoutName.copy(0, nIndex);

    if (pSourceDoc)
    {
        std::vector<StyleReplaceData> aReplList; // List of replaced stylesheets
        bool bLayoutReloaded = false;   // Wurde ex. Layout wieder geladen?

        // LayoutName, Page and Notes page
        if (rLayoutName.isEmpty())
        {
            // No LayoutName: take first MasterPage
            pMaster = pSourceDoc->GetMasterSdPage(0, PK_STANDARD);
            pNotesMaster = pSourceDoc->GetMasterSdPage(0, PK_NOTES);
        }
        else
        {
            OUStringBuffer aBuf(rLayoutName);
            aBuf.append(SD_LT_SEPARATOR).append(SdResId(STR_LAYOUT_OUTLINE).toString());
            OUString aSearchFor(aBuf.makeStringAndClear());

            for (sal_uInt16 nMP = 0; nMP < pSourceDoc->GetMasterPageCount(); ++nMP)
            {
                SdPage* pMP = static_cast<SdPage*>( pSourceDoc->GetMasterPage(nMP) );

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

            // this should not happen, but looking at crash reports, it does
            if( (pMaster == nullptr) || (pNotesMaster == nullptr) )
            {
                // so take the first MasterPage
                pMaster = static_cast<SdPage*>( pSourceDoc->GetMasterSdPage(0, PK_STANDARD) );
                pNotesMaster = static_cast<SdPage*>( pSourceDoc->GetMasterSdPage(0, PK_NOTES) );
            }
        }

        // we should never reach this, but one never knows...
        if( (pMaster == nullptr) || (pNotesMaster == nullptr) )
        {
            if (bUndo)
                pUndoMgr->LeaveListAction();

            if( mpDocSh )
                mpDocSh->SetWaitCursor( false );

            OSL_FAIL( "SdDrawDocument::SetMasterPage() failed!" );

            return;
        }

        const OUString aOriginalNewLayoutName( pMaster->GetName() );
        OUString aTargetNewLayoutName(aOriginalNewLayoutName);

        if (pSourceDoc != this)
        {
            // #i121863# clone masterpages, they are from another model (!)
            SdPage* pNewNotesMaster = dynamic_cast< SdPage* >(pNotesMaster->Clone(this));
            SdPage* pNewMaster = dynamic_cast< SdPage* >(pMaster->Clone(this));

            if(!pNewNotesMaster || !pNewMaster)
            {
                delete pNewNotesMaster;
                delete pNewMaster;
                OSL_FAIL("SdDrawDocument::SetMasterPage() cloning of MasterPage/NoteAmsterPage failed!" );
                return;
            }

            pNotesMaster = pNewNotesMaster;
            pMaster = pNewMaster;

            // layout name needs to be unique
            aTargetNewLayoutName = pMaster->GetLayoutName();
            sal_Int32 nIndex2 = aTargetNewLayoutName.indexOf(SD_LT_SEPARATOR);
            if( nIndex2 != -1 )
                aTargetNewLayoutName = aTargetNewLayoutName.copy(0, nIndex2);

            if(!isMasterPageLayoutNameUnique(*this, aTargetNewLayoutName))
            {
                aTargetNewLayoutName = createNewMasterPageLayoutName(*this);

                OUString aTemp(aTargetNewLayoutName);
                aTemp += SD_LT_SEPARATOR;
                aTemp += SD_RESSTR(STR_LAYOUT_OUTLINE);

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
                SdPage* pCheckMaster = static_cast<SdPage*>(GetMasterPage(nMPage));
                if( pCheckMaster->GetName() == aTargetNewLayoutName )
                {
                    bLayoutReloaded = true;
                    break;
                }
            }

            // Correct or create presentation templates --
            // only worry about presentation templates
            OUString aName;
            SdStyleSheetPool* pSourceStyleSheetPool = static_cast<SdStyleSheetPool*>( pSourceDoc->GetStyleSheetPool() );
            pSourceStyleSheetPool->SetSearchMask(SD_STYLE_FAMILY_MASTERPAGE);
            static_cast<SdStyleSheetPool*>( mxStyleSheetPool.get())->SetSearchMask(SD_STYLE_FAMILY_MASTERPAGE);

            SdStyleSheetVector aCreatedStyles;          // List of created stylesheets
            SfxStyleSheetBase* pHisSheet = pSourceStyleSheetPool->First();

            while (pHisSheet)
            {
                aName = pHisSheet->GetName();

                // #i121863# search in source styles with original style name from source of
                // evtl. cloned master (not-cloned, renamed for uniqueness)
                if( aName.startsWith( aOriginalNewLayoutName ) )
                {
                    // #i121863# build name of evtl. cloned master style to search for
                    if(aOriginalNewLayoutName != aTargetNewLayoutName)
                    {
                        const sal_Int32 nPos(aName.indexOf(SD_LT_SEPARATOR));
                        aName = aTargetNewLayoutName + aName.copy(nPos);
                    }

                    SfxStyleSheet* pMySheet = static_cast<SfxStyleSheet*>( mxStyleSheetPool->Find(aName, SD_STYLE_FAMILY_MASTERPAGE) );

                    if (pMySheet)
                    {
                        // A stylesheet of the same name already exists -> overwrite contents
                        bool bTest = pMySheet->SetName(pHisSheet->GetName());
                        DBG_ASSERT(bTest, "Renaming StyleSheet failed.");
                        pMySheet->GetItemSet().ClearItem();  // Delete all

                        if (bUndo)
                        {
                            StyleSheetUndoAction* pUndoChStyle = new StyleSheetUndoAction(this,
                                                                 pMySheet, &pHisSheet->GetItemSet());
                            pUndoMgr->AddUndoAction(pUndoChStyle);
                        }
                        pMySheet->GetItemSet().Put(pHisSheet->GetItemSet());
                        pMySheet->Broadcast(SfxSimpleHint(SFX_HINT_DATACHANGED));
                    }
                    else
                    {
                        // create new style
                        OUString aHelpFile;
                        pMySheet = static_cast<SfxStyleSheet*>( &mxStyleSheetPool->Make(aName, SD_STYLE_FAMILY_MASTERPAGE, pHisSheet->GetMask()) );
                        pMySheet->SetHelpId( aHelpFile, pHisSheet->GetHelpId(aHelpFile) );
                        pMySheet->GetItemSet().ClearItem();  // Delete all
                        pMySheet->GetItemSet().Put(pHisSheet->GetItemSet());

                        aCreatedStyles.push_back( SdStyleSheetRef( static_cast< SdStyleSheet* >( pMySheet ) ) );
                    }

                    StyleReplaceData aReplData;
                    aReplData.nNewFamily = pMySheet->GetFamily();
                    aReplData.nFamily    = pMySheet->GetFamily();
                    aReplData.aNewName   = pMySheet->GetName();

                    // #i121863# re-create original name of style used at page where to replace with
                    // this new style
                    OUString aTemp(pMySheet->GetName());
                    const sal_Int32 nPos(aTemp.indexOf(SD_LT_SEPARATOR));
                    aTemp = aOldLayoutName + aTemp.copy(nPos);
                    aReplData.aName = aTemp;
                    aReplList.push_back(aReplData);
                }

                pHisSheet = static_cast<SfxStyleSheet*>( pSourceStyleSheetPool->Next() );
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
                        const OUString& rParentOfOld = pSOld->GetParent();
                        const OUString& rParentOfNew = pSNew->GetParent();

                        if (!rParentOfOld.isEmpty() && rParentOfNew.isEmpty())
                        {
                            std::vector<StyleReplaceData>::iterator pRDIter;
                            for ( pRDIter = aReplList.begin(); pRDIter != aReplList.end(); ++pRDIter )
                            {
                                if ((pRDIter->aName == rParentOfOld) && (pRDIter->aName != pRDIter->aNewName))
                                {
                                    OUString aParentOfNew(pRDIter->aNewName);
                                    pSNew->SetParent(aParentOfNew);
                                    break;
                                }
                            }
                        }
                    }
                }

                // Now look for all of them when searching
                pSourceStyleSheetPool->SetSearchMask(SfxStyleFamily::All);
                mxStyleSheetPool->SetSearchMask(SfxStyleFamily::All);
            }

            if (bUndo && !aCreatedStyles.empty())
            {
                // Add UndoAction for creating and inserting the stylesheets to
                // the top of the UndoManager
                SdMoveStyleSheetsUndoAction* pMovStyles = new SdMoveStyleSheetsUndoAction( this, aCreatedStyles, true);
                pUndoMgr->AddUndoAction(pMovStyles);
            }
        }

        // Create layout name based upon the name of the page layout of the
        // master page
        OUString aPageLayoutName(pMaster->GetLayoutName());
        OUString aLayoutName = aPageLayoutName;
        sal_Int32 nIndex2 = aLayoutName.indexOf( SD_LT_SEPARATOR );
        if( nIndex2 != -1 )
            aLayoutName = aLayoutName.copy( 0, nIndex2);

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
                SdPage* pPage = static_cast<SdPage*>( GetPage(nPage) );
                OUString aTest = pPage->GetLayoutName();
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
            SdPage* pPage = *pIter;
            AutoLayout eAutoLayout = pPage->GetAutoLayout();

            if( bUndo )
            {
                SdPresentationLayoutUndoAction * pPLUndoAction =
                    new SdPresentationLayoutUndoAction
                        (this,
                        pPage->IsMasterPage() ? aLayoutName : aOldLayoutName,
                        aLayoutName,
                         eAutoLayout, eAutoLayout, false, *pIter);
                pUndoMgr->AddUndoAction(pPLUndoAction);
            }
            pPage->SetPresentationLayout(aLayoutName);
            pPage->SetAutoLayout(eAutoLayout);
        }

        // Adapt new master pages
        if (pSourceDoc != this)
        {
            Size aSize(rOldMaster.GetSize());
            Rectangle aBorderRect(rOldMaster.GetLftBorder(),
                                  rOldMaster.GetUppBorder(),
                                  rOldMaster.GetRgtBorder(),
                                  rOldMaster.GetLwrBorder());
            pMaster->ScaleObjects(aSize, aBorderRect, true);
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
            pNotesMaster->ScaleObjects(aSize, aNotesBorderRect, true);
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
        OUString aPageLayoutName(aName + SD_LT_SEPARATOR + SD_RESSTR(STR_LAYOUT_OUTLINE));

        // Generate new stylesheets
        static_cast<SdStyleSheetPool*>( mxStyleSheetPool.get())->CreateLayoutStyleSheets(aName);
        SdStyleSheetVector aCreatedStyles;
        static_cast<SdStyleSheetPool*>( mxStyleSheetPool.get())->CreateLayoutSheetList(aName, aCreatedStyles);

        if( bUndo )
        {
            SdMoveStyleSheetsUndoAction* pMovStyles = new SdMoveStyleSheetsUndoAction(this, aCreatedStyles, true);
            pUndoMgr->AddUndoAction(pMovStyles);
        }

        // Generate new master pages and register them with the document
        if( bUndo )
            BegUndo();

        pMaster = AllocSdPage(true);
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

        pNotesMaster = AllocSdPage(true);
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
                SdPage* pPage = static_cast<SdPage*>( GetPage(nPage) );
                if (pPage->GetLayoutName() == aOldPageLayoutName)
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
                             eOldAutoLayout, eNewAutoLayout, true,
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
        mpDocSh->SetWaitCursor( false );
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
            SdStyleSheetPool* pStylePool = static_cast<SdStyleSheetPool*>( GetStyleSheetPool() );
            if( pStylePool )
                pStylePool->AddStyleFamily( pPage );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
