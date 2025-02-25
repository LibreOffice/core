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


#include <memory>
#include <string_view>

#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/app.hxx>
#include <svl/itemset.hxx>
#include <tools/debug.hxx>
#include <comphelper/diagnose_ex.hxx>

#include <sfx2/fcontnr.hxx>
#include <svl/style.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdundo.hxx>
#include <vcl/stdtext.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <xmloff/autolayout.hxx>

#include <strings.hrc>
#include <drawdoc.hxx>
#include <sdmod.hxx>
#include <sdpage.hxx>
#include <stlpool.hxx>
#include <sdresid.hxx>
#include <customshowlist.hxx>
#include <sdxfer.hxx>

#include <unmovss.hxx>
#include <unchss.hxx>
#include <unprlout.hxx>
#include <DrawDocShell.hxx>
#include <GraphicDocShell.hxx>
#include <ViewShell.hxx>
#include <View.hxx>
#include <ViewShellBase.hxx>
#include <strings.hxx>

using namespace ::com::sun::star;

/** Concrete incarnations get called by lcl_IterateBookmarkPages, for
    every page in the bookmark document/list
 */

namespace {

class InsertBookmarkAsPage_FindDuplicateLayouts
{
public:
    explicit InsertBookmarkAsPage_FindDuplicateLayouts( std::vector<OUString> &rLayoutsToTransfer )
        : mrLayoutsToTransfer(rLayoutsToTransfer) {}
    void operator()( SdDrawDocument&, SdPage const *, bool bRenameDuplicates, SdDrawDocument* pBookmarkDoc, bool bMergeMasterPagesOnly);
private:
    std::vector<OUString> &mrLayoutsToTransfer;
};

}

void InsertBookmarkAsPage_FindDuplicateLayouts::operator()( SdDrawDocument& rDoc, SdPage const * pBMMPage, bool bRenameDuplicates, SdDrawDocument* pBookmarkDoc, bool bMergeMasterPagesOnly )
{
    // now check for duplicate masterpage and layout names

    OUString aLayout = SdDrawDocument::GetBaseLayoutName( pBMMPage->GetLayoutName() );

    std::vector<OUString>::const_iterator pIter =
        find(mrLayoutsToTransfer.begin(), mrLayoutsToTransfer.end(), aLayout);

    bool bFound = pIter != mrLayoutsToTransfer.end();

    const sal_uInt16 nMPageCount = rDoc.GetMasterPageCount();
    for (sal_uInt16 nMPage = 0; nMPage < nMPageCount && !bFound; nMPage++)
    {
        // Do the layouts already exist within the document?
        SdPage* pTestPage = static_cast<SdPage*>( rDoc.GetMasterPage(nMPage) );
        OUString aTest(pTestPage->GetLayoutName());
        sal_Int32 nIndex2 = aTest.indexOf( SD_LT_SEPARATOR );
        if( nIndex2 != -1 )
            aTest = aTest.copy(0, nIndex2);

        if (aTest == aLayout && pBMMPage->GetPageKind() == pTestPage->GetPageKind())
        {
            // Ignore Layouts with "Default" these seem to be special - in the sense that there are lot of assumption all over Impress
            // about this
            if (aTest != SdResId(STR_LAYOUT_DEFAULT_NAME) || bMergeMasterPagesOnly)
            {
                // If we are merging master pages only, or if we are renaming duplicates and the page is not the same, then we need to rename the layout
                if ( bMergeMasterPagesOnly ||(bRenameDuplicates && !(pTestPage->Equals(*pBMMPage))))
                {
                    OUString aOriginalName = pBMMPage->GetName();
                    OUString aNewName;
                    // Generate a new name with incremented numeric prefix
                    aNewName = SdDrawDocument::GenerateNewLayoutName(aOriginalName);
                    OUString aOldLayoutName = pBMMPage->GetLayoutName();
                    pBookmarkDoc->RenameLayoutTemplate(aOldLayoutName,aNewName);
                    aLayout = pBMMPage->GetName();
                    break;
                }
                else
                    bFound = true;
            }
        }
    }

    if (!bFound)
        mrLayoutsToTransfer.push_back(aLayout);
}

// Inserts a bookmark as a page
static void lcl_IterateBookmarkPages( SdDrawDocument &rDoc, SdDrawDocument* pBookmarkDoc,
                               const std::vector<OUString> &rBookmarkList, sal_uInt16 nCount,
                               InsertBookmarkAsPage_FindDuplicateLayouts& rPageIterator, bool bRenameDuplicates, bool bMergeMasterPagesOnly = false)
{

    // Refactored copy'n'pasted layout name collection from InsertBookmarkAsPage

    int nPos, nEndPos;

    if( rBookmarkList.empty() )
    {
        // no list? whole source document
        nEndPos = nCount;
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
            if (bMergeMasterPagesOnly)
                pBMMPage = pBookmarkDoc->GetMasterSdPage(static_cast<sal_uInt16>(nPos), PageKind::Standard);
            else
                pBMMPage = static_cast<SdPage*>(&(pBookmarkDoc->GetSdPage(static_cast<sal_uInt16>(nPos), PageKind::Standard)->TRG_GetMasterPage()));
        }
        else
        {
            // fetch nPos'th entry from bookmark list, and determine master page
            const OUString& aBMPgName(rBookmarkList[nPos]);
            bool  bIsMasterPage;

            sal_uInt16 nBMPage = pBookmarkDoc->GetPageByName( aBMPgName, bIsMasterPage );

            if (nBMPage != SDRPAGE_NOTFOUND)
            {
                if (bIsMasterPage)
                    pBMPage = static_cast<SdPage*>( pBookmarkDoc->GetMasterPage(nBMPage) );
                else
                    pBMPage = static_cast<SdPage*>( pBookmarkDoc->GetPage(nBMPage) );
            }
            else
            {
                pBMPage = nullptr;
            }

            // enforce that bookmarked page is a standard page and not already a master page
            if (pBMPage && pBMPage->GetPageKind()==PageKind::Standard && !pBMPage->IsMasterPage())
            {
                const sal_uInt16 nBMSdPage = (nBMPage - 1) / 2;
                pBMMPage = static_cast<SdPage*> (&(pBookmarkDoc->GetSdPage(nBMSdPage, PageKind::Standard)->TRG_GetMasterPage()));
            }
        }

        // successfully determined valid (bookmarked) page?
        if( pBMMPage )
        {
            // yes, call functor
            rPageIterator( rDoc, pBMMPage, bRenameDuplicates, pBookmarkDoc, bMergeMasterPagesOnly );
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
                mxBookmarkDocShRef = new ::sd::GraphicDocShell(SfxObjectCreateMode::STANDARD);
            else
                // Impress
                mxBookmarkDocShRef = new ::sd::DrawDocShell(SfxObjectCreateMode::STANDARD, true, DocumentType::Impress);

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
        std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(nullptr,
                                                       VclMessageType::Warning, VclButtonsType::Ok, SdResId(STR_READ_DATA_ERROR)));
        xErrorBox->run();

        CloseBookmarkDoc();
        pBookmarkDoc = nullptr;
    }
    else if (mxBookmarkDocShRef.is())
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
    else if (mxBookmarkDocShRef.is())
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
    sal_uInt16 nInsertPos,                          // Insertion position of pages
    ::sd::DrawDocShell* pBookmarkDocSh,             // If set, this is the source document
    Point const * pObjPos)                                 // Insertion position of objects
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
        else if ( mxBookmarkDocShRef.is() )
        {
            pBookmarkDoc = mxBookmarkDocShRef->GetDoc();
        }
        else
            bOK = false;

        bInsertPages = bOK && std::any_of(rBookmarkList.begin(), rBookmarkList.end(),
            [&pBookmarkDoc](const OUString& rBookmark) {
                // Is there a page name in the bookmark list?
                bool    bIsMasterPage;
                return pBookmarkDoc->GetPageByName(rBookmark, bIsMasterPage) != SDRPAGE_NOTFOUND;
            });
    }

    bool bCalcObjCount = !rExchangeList.empty();

    if ( bOK && bInsertPages )
    {
        // Insert all page bookmarks
        bOK = InsertFileAsPage(rBookmarkList, &rExchangeList,
                                     bLink, nInsertPos, pBookmarkDocSh);
    }

    if ( bOK && !rBookmarkList.empty() )
    {
        // Insert all object bookmarks
        InsertBookmarkAsObject(rBookmarkList, rExchangeList,
                                     pBookmarkDocSh, pObjPos, bCalcObjCount);
    }
}

namespace
{

void
lcl_removeUnusedStyles(SfxStyleSheetBasePool* const pStyleSheetPool, StyleSheetCopyResultVector& rStyles)
{
    StyleSheetCopyResultVector aUsedStyles;
    aUsedStyles.reserve(rStyles.size());
    for (const auto& a : rStyles)
    {
        if (a.m_xStyleSheet->IsUsed())
            aUsedStyles.push_back(a);
        else
            pStyleSheetPool->Remove(a.m_xStyleSheet.get());
    }
    rStyles = std::move(aUsedStyles);
}

void
lcl_removeUnusedTableStyles(SdStyleSheetPool* const pStyleSheetPool, XStyleVector const & rStyles)
{
    css::uno::Reference<css::container::XNameContainer> xTableFamily(
        pStyleSheetPool->getByName(u"table"_ustr), css::uno::UNO_QUERY);
    if (!xTableFamily)
        return;

    for (const auto& a : rStyles)
    {
        if (!a->isInUse())
            xTableFamily->removeByName(a->getName());
    }
}

SfxStyleSheet *lcl_findStyle(StyleSheetCopyResultVector& rStyles, std::u16string_view aStyleName)
{
    for (const auto& a : rStyles)
    {
        if (a.m_xStyleSheet->GetName().startsWith(aStyleName))
            return a.m_xStyleSheet.get();
    }
    return nullptr;
}

}

bool SdDrawDocument::initBookmarkDoc(
    ::sd::DrawDocShell* pBookmarkDocSh,
    SdDrawDocument*& pBookmarkDoc,
    OUString& aBookmarkName)
{
    if (pBookmarkDocSh)
    {
        pBookmarkDoc = pBookmarkDocSh->GetDoc();

        if (pBookmarkDocSh->GetMedium())
        {
            aBookmarkName = pBookmarkDocSh->GetMedium()->GetName();
        }
    }
    else if ( mxBookmarkDocShRef.is() )
    {
        pBookmarkDoc = mxBookmarkDocShRef->GetDoc();
        aBookmarkName = maBookmarkFile;
    }
    else
    {
        return false;
    }
    return true;
}

void SdDrawDocument::getPageProperties(PageProperties& mainProps, PageProperties& notesProps , sal_uInt16 nSdPageCount)
{
    // Get the properties from the first Standard page.
    mainProps.pPage = GetSdPage(0, PageKind::Standard);
    mainProps.size        = mainProps.pPage->GetSize();
    mainProps.left        = mainProps.pPage->GetLeftBorder();
    mainProps.right       = mainProps.pPage->GetRightBorder();
    mainProps.upper       = mainProps.pPage->GetUpperBorder();
    mainProps.lower       = mainProps.pPage->GetLowerBorder();
    mainProps.orientation = mainProps.pPage->GetOrientation();

    // Similarly for the first Notes page.
    notesProps.pPage = GetSdPage(0, PageKind::Notes);
    notesProps.size        = notesProps.pPage->GetSize();
    notesProps.left        = notesProps.pPage->GetLeftBorder();
    notesProps.right       = notesProps.pPage->GetRightBorder();
    notesProps.upper       = notesProps.pPage->GetUpperBorder();
    notesProps.lower       = notesProps.pPage->GetLowerBorder();
    notesProps.orientation = notesProps.pPage->GetOrientation();

    // Adapt the main page properties using the last standard page.
    mainProps.pPage = GetSdPage(nSdPageCount - 1, PageKind::Standard);
}

bool SdDrawDocument::determineScaleObjects(bool bNoDialogs,
                                           const PageNameList& rBookmarkList,
                                           PageInsertionParams& rParams)
{
    // In dialog-less mode, decide based on transfer container and page settings.
    if (bNoDialogs)
    {
        SdModule* mod = SdModule::get();
        // If this is clipboard, then no need to scale objects:
        // this will make copied masters to differ from the originals,
        // and thus InsertBookmarkAsPage_FindDuplicateLayouts will
        // duplicate masters on insert to same document
        m_bTransportContainer = (mod->pTransferClip &&
                                 mod->pTransferClip->GetWorkDocument() == this);
        if (!m_bTransportContainer)
        {
            if (rBookmarkList.empty())
                rParams.bScaleObjects = rParams.mainProps.pPage->IsScaleObjects();
            else
                rParams.bScaleObjects = true;
        }
    }
    else
    {
        // If not dialog-less, compare the first bookmark page with our reference page.
        SdPage* pBMPage = rParams.pBookmarkDoc->GetSdPage(0, PageKind::Standard);
        if (pBMPage->GetSize()      != rParams.mainProps.pPage->GetSize()       ||
            pBMPage->GetLeftBorder()  != rParams.mainProps.pPage->GetLeftBorder()  ||
            pBMPage->GetRightBorder() != rParams.mainProps.pPage->GetRightBorder() ||
            pBMPage->GetUpperBorder() != rParams.mainProps.pPage->GetUpperBorder() ||
            pBMPage->GetLowerBorder() != rParams.mainProps.pPage->GetLowerBorder())
        {
            OUString aStr(SdResId(STR_SCALE_OBJECTS));
            std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(nullptr,
                                                VclMessageType::Question, VclButtonsType::YesNo,
                                                aStr));
            xQueryBox->add_button(GetStandardText(StandardButtonType::Cancel), RET_CANCEL);
            sal_uInt16 nBut = xQueryBox->run();
            rParams.bScaleObjects = (nBut == RET_YES);
            bool bContinue = (nBut != RET_CANCEL);
            if (!bContinue)
                return bContinue;
        }
    }
    return true;
}

SfxUndoManager* SdDrawDocument::beginUndoAction()
{
    SfxUndoManager* pUndoMgr = nullptr;
    if ( mpDocSh )
    {
        pUndoMgr = mpDocSh->GetUndoManager();
        ViewShellId nViewShellId(-1);
        if (sd::ViewShell* pViewShell = mpDocSh->GetViewShell())
            nViewShellId = pViewShell->GetViewShellBase().GetViewShellId();
        pUndoMgr->EnterListAction(SdResId(STR_UNDO_INSERTPAGES), u""_ustr, 0, nViewShellId);
    }
    return pUndoMgr;
}

void SdDrawDocument::collectLayoutsToTransfer(const PageNameList& rBookmarkList,
                                              SdDrawDocument* pBookmarkDoc,
                                              SlideLayoutNameList& aLayoutsToTransfer,
                                              const DocumentPageCounts& rPageCounts,
                                              bool bMergeMasterPagesOnly)
{
    sal_uInt16 nCount = bMergeMasterPagesOnly ? rPageCounts.nMasterPageCount : rPageCounts.nSourcePageCount;// Replace by pageCounts to add the masterpagecount
    InsertBookmarkAsPage_FindDuplicateLayouts aSearchFunctor(aLayoutsToTransfer);
    lcl_IterateBookmarkPages( *this, pBookmarkDoc, rBookmarkList, nCount, aSearchFunctor, ( rBookmarkList.empty() && pBookmarkDoc != this ), bMergeMasterPagesOnly );
}

void SdDrawDocument::transferLayoutStyles(const SlideLayoutNameList& aLayoutsToTransfer,
                                          SdDrawDocument* pBookmarkDoc,
                                          SfxUndoManager* pUndoMgr,
                                          StyleTransferContext& rStyleContext)
{
    // For each layout in the list, copy the layout styles and, if the layout
    // is from a master page, also copy theme and layout ID.

    // Use the style sheet pools from the context
    SdStyleSheetPool& rBookmarkStyleSheetPool = *rStyleContext.pSourceStyleSheetPool;
    SdStyleSheetPool& rStyleSheetPool = *rStyleContext.pDestStyleSheetPool;

    for (const OUString& layoutName : aLayoutsToTransfer)
    {
        StyleSheetCopyResultVector aCreatedStyles;
        rStyleSheetPool.CopyLayoutSheets(layoutName, rBookmarkStyleSheetPool, aCreatedStyles);

        if(!aCreatedStyles.empty())
        {
            if( pUndoMgr )
            {
                pUndoMgr->AddUndoAction(std::make_unique<SdMoveStyleSheetsUndoAction>(this, aCreatedStyles, true));
            }
        }

        // copy SlideLayout and Theme of the master slide
        // If the layout belongs to a master page, extract its theme and layout ID.
        sal_Int32 nLayout = 20; // blank page - master slide layout ID
        bool bIsMasterPage = false;
        sal_uInt16 nBMPage = pBookmarkDoc->GetPageByName(layoutName, bIsMasterPage);
        if (bIsMasterPage)
        {
            uno::Reference< drawing::XDrawPage > xOldPage(pBookmarkDoc->GetMasterPage(nBMPage)->getUnoPage(), uno::UNO_QUERY_THROW);
            SdrPage* pMasterPage = SdPage::getImplementation(xOldPage);
            if (pMasterPage)
            {
                rStyleContext.aThemes.insert({ layoutName, pMasterPage->getSdrPageProperties().getTheme() });

                // Retrieve the SlideLayout property via the property set.
                uno::Reference<beans::XPropertySet> xPropSet(xOldPage, uno::UNO_QUERY_THROW);
                if (xPropSet.is())
                {
                    uno::Any aLayoutID = xPropSet->getPropertyValue(u"SlideLayout"_ustr);
                    if (aLayoutID.hasValue()) {
                        aLayoutID >>= nLayout;
                    }
                }
            }
            rStyleContext.aSlideLayouts.insert({ layoutName, nLayout });
        }
    }
}

void SdDrawDocument::copyStyles(bool bReplace, bool bNoDialogs,
                                StyleTransferContext& rStyleContext)
{
    // Use the style sheet pools from the context
    SdStyleSheetPool& rBookmarkStyleSheetPool = *rStyleContext.pSourceStyleSheetPool;
    SdStyleSheetPool& rStyleSheetPool = *rStyleContext.pDestStyleSheetPool;

    // Depending on whether pages are being replaced and dialog mode,
    // decide on a renaming string and then copy graphic, cell, and table styles.
    if (!bReplace && !bNoDialogs)
        rStyleContext.aRenameString = "_";
    rStyleSheetPool.RenameAndCopyGraphicSheets(rBookmarkStyleSheetPool, rStyleContext.aGraphicStyles, rStyleContext.aRenameString);
    rStyleSheetPool.CopyCellSheets(rBookmarkStyleSheetPool, rStyleContext.aCellStyles);
    // TODO handle undo of table styles too
    rStyleSheetPool.CopyTableStyles(rBookmarkStyleSheetPool, rStyleContext.aTableStyles);
}

void SdDrawDocument::insertAllPages(PageInsertionParams& rParams,
                                    const InsertBookmarkOptions& rOptions,
                                    const DocumentPageCounts& rPageCounts)
{
    // Adjust insertion position if needed.
    if (rParams.nInsertPos >= GetPageCount())
        rParams.nInsertPos = GetPageCount();

    sal_uInt16 nActualInsertPos = rParams.nInsertPos;

    // First, handle master pages.
    if (rOptions.bMergeMasterPagesOnly)
    {
        // Insert all master pages from the bookmark document.
        sal_uInt16 nMPageCount = rParams.pBookmarkDoc->GetMasterPageCount();
        // During the copy process, a vector holds the master page slides:
        // - default handout page,
        // - default standard page,
        // - and default notes page.
        //
        // Normally, new master slides are added, so we start targeting from index 3.
        // If the copied slide matches the existing "Default" name, no new slides are added;
        // we then start from index 1, skipping the handout.
        sal_uInt16 nStart = (nMPageCount % 2) + 2;
        if (nStart == nMPageCount)
            nStart = 1;
        // Process all master pages.
        for (sal_uInt16 nMPage = nStart; nMPage < nMPageCount; nMPage++)
        {
            SdPage* pSBMPage = static_cast<SdPage*>(rParams.pBookmarkDoc->GetMasterPage(nMPage));

            // Only process standard pages
            if (pSBMPage && pSBMPage->GetPageKind() == PageKind::Standard && pSBMPage->IsMasterPage())
            {
                OUString aLayoutName(pSBMPage->GetName());
                if (aLayoutName == SdResId(STR_LAYOUT_DEFAULT_NAME))
                {
                    // Default page -> use default name
                    aLayoutName = OUString();
                }
                AddNewMasterPageFromExisting(pSBMPage, rParams.pBookmarkDoc, rParams.bUndo, aLayoutName);
            }

        }
        return ;
    }

    sal_uInt16 nBMSdPage;
    // Build a name map and a set for pages that need renaming.
    std::set<sal_uInt16> aRenameSet;
    std::map<sal_uInt16, OUString> aNameMap;

    for (nBMSdPage = 0; nBMSdPage < rPageCounts.nSourcePageCount; nBMSdPage++)
    {
        SdPage* pBMPage = rParams.pBookmarkDoc->GetSdPage(nBMSdPage, PageKind::Standard);
        OUString sName(pBMPage->GetName());
        bool bIsMasterPage;

        if (rOptions.bLink)
        {
            // Remember the names of all pages
            aNameMap.insert(std::make_pair(nBMSdPage,sName));
        }

        // Have to check for duplicate names here, too
        // don't change name if source and dest model are the same!
        if( rParams.pBookmarkDoc != this &&
            GetPageByName(sName, bIsMasterPage ) != SDRPAGE_NOTFOUND )
        {
            // delay renaming *after* pages are copied (might destroy source otherwise)
            aRenameSet.insert(nBMSdPage);
        }
    }

    // Merge the pages from the bookmark document.
    Merge(*rParams.pBookmarkDoc,
          1,                 // Not the handout page
          SDRPAGE_NOTFOUND,  // But all others
          nActualInsertPos,  // Insert at position ...
          rOptions.bMergeMasterPages, // Move master pages?
          false,             // But only the master pages used
          true,              // Create an undo action
          rOptions.bCopy);            // Copy (or merge) pages?

    // After merging, fix names and set link info if needed.
    for ( nBMSdPage = 0; nBMSdPage < rPageCounts.nSourcePageCount; nBMSdPage++)
    {
        SdPage* pPage      = static_cast<SdPage*>( GetPage(nActualInsertPos) );
        SdPage* pNotesPage = static_cast<SdPage*>( GetPage(nActualInsertPos + 1) );

        // delay renaming *after* pages are copied (might destroy source otherwise)
        if ( aRenameSet.find(nBMSdPage) != aRenameSet.end() )
        {
            // Page name already in use -> Use default name for default and
            // notes page
            pPage->SetName(OUString());
            pNotesPage->SetName(OUString());
        }
        if (rOptions.bLink)
        {
            // Assemble all link names
            pPage->SetFileName(rParams.aBookmarkName);
            pPage->SetBookmarkName(aNameMap[nBMSdPage]);
        }

        nActualInsertPos += 2;
    }
}

void SdDrawDocument::insertSelectedPages(const PageNameList& rBookmarkList,
                                      PageInsertionParams& rParams,
                                      InsertBookmarkOptions rOptions)
{
    // Adjust insertion position if necessary.
    if (rParams.nInsertPos >= GetPageCount())
    {
        // Add pages to the end
        rOptions.bReplace = false;
        rParams.nInsertPos = GetPageCount();
    }

    sal_uInt16 nActualInsertPos = rParams.nInsertPos;

    // Build a vector of pointers to bookmarked pages.
    std::vector<SdPage*> aBookmarkedPages(rBookmarkList.size(), nullptr);
    for (size_t nPos = 0; nPos < rBookmarkList.size(); ++nPos)
    {
        const OUString& aPgName(rBookmarkList[nPos]);
        bool bIsMasterPage = false;
        sal_uInt16 nBMPage = rParams.pBookmarkDoc->GetPageByName( aPgName, bIsMasterPage);

        if (nBMPage != SDRPAGE_NOTFOUND)
        {
            if (rOptions.bMergeMasterPagesOnly)
            {
                // Only the master page
                aBookmarkedPages[nPos] = static_cast<SdPage*>(rParams.pBookmarkDoc->GetMasterPage(nBMPage));
            }
            else
            {
                // The standard page
                aBookmarkedPages[nPos] = dynamic_cast<SdPage*>(rParams.pBookmarkDoc->GetPage(nBMPage));
            }
        }
    }

    if (rOptions.bMergeMasterPagesOnly)
    {
        for (size_t nPos = 0; nPos < rBookmarkList.size(); ++nPos)
        {
            SdPage* pBMPage = aBookmarkedPages[nPos];
            if (pBMPage && pBMPage->GetPageKind() == PageKind::Standard && pBMPage->IsMasterPage())
            {
                // It has to be a default page
                AddNewMasterPageFromExisting(pBMPage, rParams.pBookmarkDoc, rParams.bUndo, pBMPage->GetName());
            }
        }
        return;
    }

    // Process each bookmarked page.
    for ( size_t nPos = 0; nPos < rBookmarkList.size(); ++nPos)
    {
        SdPage* pBMPage = aBookmarkedPages[nPos];
        sal_uInt16 nBMPage = pBMPage!=nullptr ? pBMPage->GetPageNum() : SDRPAGE_NOTFOUND;

        if (pBMPage && pBMPage->GetPageKind() == PageKind::Standard && !pBMPage->IsMasterPage())
        {
            // It has to be a default page
            bool bMustRename = false;
            // delay renaming *after* pages are copied (might destroy source otherwise)
            // don't change name if source and dest model are the same!
            // avoid renaming if replacing the same page
            const OUString& aPgName(rBookmarkList[nPos]);
            bool  bIsMasterPage;
            sal_uInt16 nPageSameName = GetPageByName(aPgName, bIsMasterPage);
            if ( rParams.pBookmarkDoc != this &&
                nPageSameName != SDRPAGE_NOTFOUND &&
                ( !rOptions.bReplace || nPageSameName != nActualInsertPos))
            {
                bMustRename = true;
            }

            SdPage* pBookmarkPage = pBMPage;
            if (rOptions.bReplace)
                ReplacePageInCustomShows(dynamic_cast<SdPage*>(GetPage(nActualInsertPos)), pBookmarkPage);

            Merge(*rParams.pBookmarkDoc,
                  nBMPage,           // From page (default page)
                  nBMPage+1,         // To page (notes page)
                  nActualInsertPos,  // Insert at position
                  rOptions.bMergeMasterPages, // Move master pages?
                  false,             // But only the master pages used
                  true,              // Create undo action
                  rOptions.bCopy);            // Copy (or merge) pages?

            if (rOptions.bReplace && GetPage(nActualInsertPos) != pBookmarkPage)
            {
                // bookmark page was not moved but cloned, so update custom shows again
                ReplacePageInCustomShows(pBookmarkPage, dynamic_cast<SdPage*>(GetPage(nActualInsertPos)));
            }
                // tdf#39519 - rename page if its name is not unique, e.g., if a slide is copied by
                // ctrl + drag and drop (DND_ACTION_COPY)
            if (bMustRename
                // tdf#164284 - prevent page name change during page move
                || (rParams.pBookmarkDoc->DoesMakePageObjectsNamesUnique()
                    && !mpDocSh->IsPageNameUnique(aPgName)))
            {
                // Page name already in use -> use default name for default and
                // notes page
                SdPage* pPage = static_cast<SdPage*>( GetPage(nActualInsertPos) );
                pPage->SetName(OUString());
                SdPage* pNotesPage = static_cast<SdPage*>( GetPage(nActualInsertPos+1) );
                pNotesPage->SetName(OUString());
            }

            if (rOptions.bLink)
            {
                SdPage* pPage = static_cast<SdPage*>( GetPage(nActualInsertPos) );
                pPage->SetFileName(rParams.aBookmarkName);
                pPage->SetBookmarkName(aPgName);
            }

            if (rOptions.bReplace)
            {
                // Remove the replaced page(s)
                const sal_uInt16 nDestPageNum(nActualInsertPos + 2);
                SdPage* pStandardPage = nullptr;

                if (nDestPageNum < GetPageCount())
                {
                    pStandardPage = static_cast<SdPage*>(GetPage(nDestPageNum));
                }

                if (pStandardPage)
                {
                    if( rOptions.bPreservePageNames )
                    {
                        // Take old slide names for inserted pages
                        SdPage* pPage = static_cast<SdPage*>( GetPage(nActualInsertPos) );
                        pPage->SetName( pStandardPage->GetRealName() );
                    }

                    if( rParams.bUndo )
                        AddUndo(GetSdrUndoFactory().CreateUndoDeletePage(*pStandardPage));

                    RemovePage(nDestPageNum);
                }

                SdPage* pNotesPage = nullptr;
                if (nDestPageNum < GetPageCount())
                {
                    pNotesPage = static_cast<SdPage*>(GetPage(nDestPageNum));
                }

                if (pNotesPage)
                {
                    if (rOptions.bPreservePageNames)
                    {
                        // Take old slide names for inserted pages
                        SdPage* pNewNotesPage = static_cast<SdPage*>( GetPage(nActualInsertPos+1));

                        if (pNewNotesPage)
                        {
                            assert(pStandardPage);
                            pNewNotesPage->SetName(pStandardPage->GetRealName());
                        }
                    }

                    if( rParams.bUndo )
                        AddUndo(GetSdrUndoFactory().CreateUndoDeletePage(*pNotesPage));

                    RemovePage(nDestPageNum);
                }
                rParams.nReplacedStandardPages++;
            }

            nActualInsertPos += 2;
        }
    }
}

void SdDrawDocument::removeDuplicateMasterPages(PageInsertionParams& rParams,
                                                DocumentPageCounts& rPageCounts)
{
    // Remove duplicate master pages created during the merge.
    // We might have duplicate master pages now, as the drawing engine does not
    // recognize duplicates. Remove these now.
    rPageCounts.nNewMPageCount = GetMasterPageCount();

    // Go backwards, so the numbers don't become messed up
    for (sal_uInt16 nPage = rPageCounts.nNewMPageCount - 1; nPage >= rPageCounts.nMasterPageCount; nPage--)
    {
        rParams.mainProps.pPage = static_cast<SdPage*>(GetMasterPage(nPage));
        OUString aMPLayout (rParams.mainProps.pPage->GetLayoutName());
        PageKind eKind = rParams.mainProps.pPage->GetPageKind();
        // Check against the original set of master pages.
        for (sal_uInt16 nTest = 0; nTest < rPageCounts.nMasterPageCount; nTest++)
        {
            SdPage* pTest = static_cast<SdPage*>( GetMasterPage(nTest));
            OUString aTest(pTest->GetLayoutName());

            if (aTest == aMPLayout && eKind == pTest->GetPageKind() && rParams.nInsertPos > 2)
            {
                if(rParams.bUndo)
                    AddUndo(GetSdrUndoFactory().CreateUndoDeletePage(*rParams.mainProps.pPage));
                RemoveMasterPage(nPage);
                rPageCounts.nNewMPageCount--;
                break;
            }
        }
    }
    // Optionally, update remaining master pages with new themes/layout IDs.
}

void SdDrawDocument::updateInsertedPages(PageInsertionParams& rParams,
                                         const InsertBookmarkOptions& rOptions,
                                         const DocumentPageCounts& rPageCounts,
                                         StyleTransferContext& rStyleContext)
{
    // If we are copying master pages only, we don't need to update the pages.
    if (rOptions.bMergeMasterPagesOnly)
        return;

    // Update page names (from pExchangeList), layouts, scaling, etc.
    sal_uInt16 nSdPageStart = (rParams.nInsertPos - 1) / 2;
    sal_uInt16 nSdPageEnd = rOptions.bReplace
        ? nSdPageStart + rParams.nReplacedStandardPages - 1 // if replacing, update only the replaced pages
        : GetSdPageCount(PageKind::Standard) - rPageCounts.nDestPageCount + nSdPageStart - 1;

    const bool bRemoveEmptyPresObj =
            (rParams.pBookmarkDoc->GetDocumentType() == DocumentType::Impress) &&
            (GetDocumentType() == DocumentType::Draw);

    std::vector<OUString>::iterator pExchangeIter;
    if (rParams.pExchangeList)
        pExchangeIter = rParams.pExchangeList->begin();

    for (sal_uInt16 nSdPage = nSdPageStart; nSdPage <= nSdPageEnd; ++nSdPage)
    {
        rParams.mainProps.pPage = GetSdPage(nSdPage, PageKind::Standard);
        if (rParams.pExchangeList && pExchangeIter != rParams.pExchangeList->end())
        {
            OUString aExchangeName(*pExchangeIter);
            rParams.mainProps.pPage->SetName(aExchangeName);
            Broadcast(SdrHint(SdrHintKind::PageOrderChange, rParams.mainProps.pPage));

            SdPage* pNotesPage = GetSdPage(nSdPage, PageKind::Notes);
            pNotesPage->SetName(aExchangeName);
            Broadcast(SdrHint(SdrHintKind::PageOrderChange, pNotesPage));

            ++pExchangeIter;
        }
        OUString aLayout = GetBaseLayoutName(rParams.mainProps.pPage->GetLayoutName());

        // update layout and referred master page
        rParams.mainProps.pPage->SetPresentationLayout(aLayout);
        if (rParams.bUndo)
            AddUndo(GetSdrUndoFactory().CreateUndoPageChangeMasterPage(*rParams.mainProps.pPage));

        if (rParams.bScaleObjects)
        {
            ::tools::Rectangle aBorderRect(rParams.mainProps.left, rParams.mainProps.upper, rParams.mainProps.right, rParams.mainProps.lower);
            rParams.mainProps.pPage->ScaleObjects(rParams.mainProps.size, aBorderRect, true);
        }
        rParams.mainProps.pPage->SetSize(rParams.mainProps.size);
        rParams.mainProps.pPage->SetBorder(rParams.mainProps.left, rParams.mainProps.upper, rParams.mainProps.right, rParams.mainProps.lower);
        rParams.mainProps.pPage->SetOrientation(rParams.mainProps.orientation);

        if (bRemoveEmptyPresObj)
            rParams.mainProps.pPage->RemoveEmptyPresentationObjects();

        rParams.notesProps.pPage = GetSdPage(nSdPage, PageKind::Notes);

        // update layout and referred master page
        rParams.notesProps.pPage->SetPresentationLayout(aLayout);
        if (rParams.bUndo)
            AddUndo(GetSdrUndoFactory().CreateUndoPageChangeMasterPage(*rParams.notesProps.pPage));

        if (rParams.bScaleObjects)
        {
            ::tools::Rectangle aBorderRect(rParams.notesProps.left, rParams.notesProps.upper, rParams.notesProps.right, rParams.notesProps.lower);
            rParams.notesProps.pPage->ScaleObjects(rParams.notesProps.size, aBorderRect, true);
        }

        rParams.notesProps.pPage->SetSize(rParams.notesProps.size);
        rParams.notesProps.pPage->SetBorder(rParams.notesProps.left, rParams.notesProps.upper, rParams.notesProps.right, rParams.notesProps.lower);
        rParams.notesProps.pPage->SetOrientation(rParams.notesProps.orientation);

        if (bRemoveEmptyPresObj)
            rParams.notesProps.pPage->RemoveEmptyPresentationObjects();
    }
    ///Remove processed elements, to avoid doing hacks in InsertBookmarkAsObject
    if (rParams.pExchangeList)
        rParams.pExchangeList->erase(rParams.pExchangeList->begin(), pExchangeIter);

    for (sal_uInt16 nPage = rPageCounts.nMasterPageCount; nPage < rPageCounts.nNewMPageCount; nPage++)
    {
        rParams.mainProps.pPage = static_cast<SdPage*>(GetMasterPage(nPage));
        if (rParams.mainProps.pPage->GetPageKind() == PageKind::Standard)
        {
            if (rParams.bScaleObjects)
            {
                ::tools::Rectangle aBorderRect(rParams.mainProps.left, rParams.mainProps.upper, rParams.mainProps.right, rParams.mainProps.lower);
                rParams.mainProps.pPage->ScaleObjects(rParams.mainProps.size, aBorderRect, true);
            }
            rParams.mainProps.pPage->SetSize(rParams.mainProps.size);
            rParams.mainProps.pPage->SetBorder(rParams.mainProps.left, rParams.mainProps.upper, rParams.mainProps.right, rParams.mainProps.lower);
            rParams.mainProps.pPage->SetOrientation(rParams.mainProps.orientation);

            uno::Reference<drawing::XDrawPage> xNewPage(GetMasterPage(nPage)->getUnoPage(), uno::UNO_QUERY_THROW);

            SdrPage* pMasterPage = SdPage::getImplementation(xNewPage);
            if (pMasterPage)
            {
                OUString aLayout(rParams.mainProps.pPage->GetName());
                if (auto it{ rStyleContext.aThemes.find(aLayout) }; it != std::end(rStyleContext.aThemes))
                {
                    pMasterPage->getSdrPageProperties().setTheme(it->second);
                }
            }

            uno::Reference<beans::XPropertySet> xNewPropSet(xNewPage, uno::UNO_QUERY_THROW);
            if (xNewPropSet.is())
            {
                OUString aLayout(rParams.mainProps.pPage->GetName());
                sal_Int32 nLayout = 20; // blank page - master slide layout ID
                if (auto it{ rStyleContext.aSlideLayouts.find(aLayout) }; it != std::end(rStyleContext.aSlideLayouts))
                {
                    nLayout = it->second;
                }
                xNewPropSet->setPropertyValue(u"SlideLayout"_ustr, uno::Any(nLayout));
            }
        }
        else // Can only be notes
        {
            if (rParams.bScaleObjects)
            {
                ::tools::Rectangle aBorderRect(rParams.notesProps.left, rParams.notesProps.upper, rParams.notesProps.right, rParams.notesProps.lower);
                rParams.notesProps.pPage->ScaleObjects(rParams.notesProps.size, aBorderRect, true);
            }
            rParams.notesProps.pPage->SetSize(rParams.notesProps.size);
            rParams.notesProps.pPage->SetBorder(rParams.notesProps.left, rParams.notesProps.upper, rParams.notesProps.right, rParams.notesProps.lower);
            rParams.notesProps.pPage->SetOrientation(rParams.notesProps.orientation);
        }

        if (bRemoveEmptyPresObj)
            rParams.mainProps.pPage->RemoveEmptyPresentationObjects();
    }
}

void SdDrawDocument::renameObjectStylesIfNeeded(sal_uInt32 nInsertPos,
                                                StyleTransferContext& rStyleContext,
                                                sal_uInt32 nBMSdPageCount)
{
    if (!rStyleContext.aRenameString.isEmpty())
    {
        try {
            for (sal_uInt32 p = nInsertPos; p < nInsertPos + nBMSdPageCount ; p++)
            {
                if (SdPage* pPage = static_cast<SdPage*>(GetPage(p))) {
                    for (const rtl::Reference<SdrObject>& pObj : *pPage)
                    {
                        if (pObj->GetStyleSheet())
                        {
                            OUString aStyleName = pObj->GetStyleSheet()->GetName();
                            SfxStyleSheet* pSheet = lcl_findStyle(rStyleContext.aGraphicStyles, Concat2View(aStyleName + rStyleContext.aRenameString));
                            if (pSheet)
                                pObj->SetStyleSheet(pSheet, true);
                        }
                    }
                }
            }
        }
        catch(...)
        {
            TOOLS_WARN_EXCEPTION( "sd", "Exception while renaming styles @ SdDrawDocument::InsertBookmarkAsPage");
        }
    }
}

void SdDrawDocument::cleanupStyles(SfxUndoManager* pUndoMgr,
                                   StyleTransferContext& rStyleContext)
{
    lcl_removeUnusedStyles(GetStyleSheetPool(), rStyleContext.aGraphicStyles);
    if (!rStyleContext.aGraphicStyles.empty() && pUndoMgr)
        pUndoMgr->AddUndoAction(std::make_unique<SdMoveStyleSheetsUndoAction>(this, rStyleContext.aGraphicStyles, true));
    lcl_removeUnusedTableStyles(static_cast<SdStyleSheetPool*>(GetStyleSheetPool()), rStyleContext.aTableStyles);
    lcl_removeUnusedStyles(GetStyleSheetPool(), rStyleContext.aCellStyles);
}

void SdDrawDocument::endUndoAction(bool bUndo,SfxUndoManager* pUndoMgr)
{
    if( bUndo )
        EndUndo();

    if (pUndoMgr)
        pUndoMgr->LeaveListAction();
}

// Inserts a bookmark as an object
bool SdDrawDocument::InsertBookmarkAsObject(
    const std::vector<OUString> &rBookmarkList,
    const std::vector<OUString> &rExchangeList,            // List of names to use
    ::sd::DrawDocShell* pBookmarkDocSh,
    Point const * pObjPos,
    bool bCalcObjCount)
{
    bool bOK = true;
    bool bOLEObjFound = false;
    std::unique_ptr<::sd::View> pBMView;

    SdDrawDocument* pBookmarkDoc = nullptr;

    if (pBookmarkDocSh)
    {
        pBookmarkDoc = pBookmarkDocSh->GetDoc();
    }
    else if ( mxBookmarkDocShRef.is() )
    {
        pBookmarkDoc = mxBookmarkDocShRef->GetDoc();
    }
    else
    {
        return false;
    }

    if (rBookmarkList.empty())
    {
        pBMView.reset(new ::sd::View(*pBookmarkDoc, nullptr));
        pBMView->EndListening(*pBookmarkDoc);
        pBMView->MarkAll();
    }
    else
    {
        SdrPage* pPage;
        SdrPageView* pPV;

        for ( const auto& rBookmark : rBookmarkList )
        {
            // Get names of bookmarks from the list
            SdrObject* pObj = pBookmarkDoc->GetObj(rBookmark);

            if (pObj)
            {
                // Found an object
                if (pObj->GetObjInventor() == SdrInventor::Default &&
                    pObj->GetObjIdentifier() == SdrObjKind::OLE2)
                {
                    bOLEObjFound = true;
                }

                if (!pBMView)
                {
                    // Create View for the first time
                    pBMView.reset(new ::sd::View(*pBookmarkDoc, nullptr));
                    pBMView->EndListening(*pBookmarkDoc);
                }

                pPage = pObj->getSdrPageFromSdrObject();

                if (pPage->IsMasterPage())
                {
                    pPV = pBMView->ShowSdrPage(pBMView->GetModel().GetMasterPage(pPage->GetPageNum()));
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
        std::optional<::sd::View> pView(std::in_place, *this, nullptr);
        pView->EndListening(*this);

        // Look for the page into which the objects are supposed to be inserted
        SdrPage* pPage = GetSdPage(0, PageKind::Standard);

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
            aObjPos = ::tools::Rectangle(Point(), pPage->GetSize()).Center();
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

        SdDrawDocument* pTmpDoc = static_cast<SdDrawDocument*>( pBMView->CreateMarkedObjModel().release() );
        bOK = pView->Paste(*pTmpDoc, aObjPos, pPage, SdrInsertFlags::NONE);

        if (bOLEObjFound)
            pBMView->GetDoc().SetAllocDocSh(false);

        if (!bOLEObjFound)
            delete pTmpDoc;         // Would otherwise be destroyed by DocShell

        pView.reset();

        // Get number of objects after inserting.
        const size_t nCount = pPage->GetObjCount();
        if (nCountBefore < nCount)
        {
            size_t nObj = nCountBefore;
            for (const auto& rExchange : rExchangeList)
            {
                // Get the name to use from the Exchange list
                if (pPage->GetObj(nObj))
                {
                    pPage->GetObj(nObj)->SetName(rExchange);
                }

                ++nObj;
                if (nObj >= nCount)
                    break;
            }
        }
    }

    return bOK;
}

OUString SdDrawDocument::GetBaseLayoutName(std::u16string_view rLayoutName)
{
    OUString aBaseName(rLayoutName);
    sal_Int32 nIndex = aBaseName.indexOf(SD_LT_SEPARATOR);
    if (nIndex != -1)
        aBaseName = aBaseName.copy(0, nIndex);

    return aBaseName;
}

OUString SdDrawDocument::GenerateNewLayoutName(std::u16string_view rOriginalName)
{
    // Generate a new name with incremented numeric prefix
    OUString aOriginalName(rOriginalName);
    OUString aNewName;
    sal_uInt32 nCount = 1;

    // Check if name already has a number prefix
    int nUnderscorePos = aOriginalName.indexOf( "_" );
    if (nUnderscorePos > 0)
    {
        // Try to extract the number part
        OUString aPrefix = aOriginalName.copy(0, nUnderscorePos);

        // Check if the prefix is a valid number
        bool bIsNumber = true;
        for (sal_Int32 i = 0; i < aPrefix.getLength(); ++i)
        {
            if (!isdigit(aPrefix[i]))
            {
                bIsNumber = false;
                break;
            }
        }
        if (bIsNumber)
        {
            // If it is a number, increment it
            nCount = aPrefix.toInt32() + 1;
            OUString aBaseName = aOriginalName.copy(nUnderscorePos + 1);
            aNewName = OUString::number(nCount) + "_" + aBaseName;
        }
        else
        {
            // Has underscore but not a number, just append "1_"
            aNewName = "1_" + aOriginalName;
        }
    }
    else
    {
        // If no number prefix, just append "1_"
        aNewName = "1_" + aOriginalName;
    }

    return aNewName;
}

// Stops the bookmark insertion
void SdDrawDocument::CloseBookmarkDoc()
{
    if (mxBookmarkDocShRef.is())
    {
        mxBookmarkDocShRef->DoClose();
    }

    mxBookmarkDocShRef.clear();
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

    if(mxAllocedDocShRef.is())
    {
        mxAllocedDocShRef->DoClose();
    }

    mxAllocedDocShRef.clear();
}

// Return list of CustomShows (create it, too, if necessary)
SdCustomShowList* SdDrawDocument::GetCustomShowList(bool bCreate)
{
    if (!mpCustomShowList && bCreate)
    {
        mpCustomShowList.reset(new SdCustomShowList);
    }

    return mpCustomShowList.get();
}

// Remove unused master pages and layouts
void SdDrawDocument::RemoveUnnecessaryMasterPages(SdPage* pMasterPage, bool bOnlyDuplicatePages, bool bUndo)
{
    ::sd::View* pView = nullptr;
    SfxUndoManager* pUndoMgr = nullptr;

    if( bUndo && !IsUndoEnabled() )
        bUndo = false;

    if (mpDocSh)
    {
        pUndoMgr = mpDocSh->GetUndoManager();

        if (mpDocSh->GetViewShell())
            pView = mpDocSh->GetViewShell()->GetView();
    }

    // Check all master pages
    sal_uInt16 nSdMasterPageCount = GetMasterSdPageCount( PageKind::Standard );
    for (sal_Int32 nMPage = nSdMasterPageCount - 1; nMPage >= 0; nMPage--)
    {
        SdPage* pMaster = pMasterPage;
        SdPage* pNotesMaster = nullptr;

        if (!pMaster)
        {
            pMaster = GetMasterSdPage( static_cast<sal_uInt16>(nMPage), PageKind::Standard );
            pNotesMaster = GetMasterSdPage( static_cast<sal_uInt16>(nMPage), PageKind::Notes );
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

        DBG_ASSERT( pMaster->GetPageKind() == PageKind::Standard, "wrong page kind" );

        if ( pMaster->GetPageKind() == PageKind::Standard &&
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
                for (sal_uInt16 i = 0; i < GetMasterSdPageCount( PageKind::Standard ); i++)
                {
                    SdPage* pMPg = GetMasterSdPage( i, PageKind::Standard );
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
                            pView->ShowSdrPage( GetSdPage( 0, PageKind::Standard ) );
                        }
                    }
                }

                if( bUndo )
                {
                    BegUndo();
                    AddUndo( GetSdrUndoFactory().CreateUndoDeletePage( *pNotesMaster ) );
                }

                RemoveMasterPage( pNotesMaster->GetPageNum() );

                if( bUndo )
                    AddUndo(GetSdrUndoFactory().CreateUndoDeletePage(*pMaster));

                RemoveMasterPage( pMaster->GetPageNum() );

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
                        StyleSheetCopyResultVector aUndoRemove;
                        aUndoRemove.reserve(aRemove.size());
                        for (const auto& a : aRemove)
                            aUndoRemove.emplace_back(a.get(), true);

                        if (pUndoMgr)
                            pUndoMgr->AddUndoAction(std::make_unique<SdMoveStyleSheetsUndoAction>(this, aUndoRemove, false));
                    }

                    for( const auto& a : aRemove )
                        static_cast<SdStyleSheetPool*>( mxStyleSheetPool.get())->Remove(a.get());
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
static bool isMasterPageLayoutNameUnique(const SdDrawDocument& rDoc, std::u16string_view rCandidate)
{
    if (rCandidate.empty())
    {
        return false;
    }

    const sal_uInt16 nPageCount(rDoc.GetMasterPageCount());

    for(sal_uInt16 a(0); a < nPageCount; a++)
    {
        const SdPage* pCandidate = static_cast<const SdPage*>(rDoc.GetMasterPage(a));
        OUString aPageLayoutName = SdDrawDocument::GetBaseLayoutName(pCandidate->GetLayoutName());

        if(aPageLayoutName == rCandidate)
        {
            return false;
        }
    }

    return true;
}

// #i121863# factored out functionality
static OUString createNewMasterPageLayoutName(const SdDrawDocument& rDoc)
{
    const OUString aBaseName(SdResId(STR_LAYOUT_DEFAULT_NAME));
    sal_uInt16 nCount(0);
    for (;;)
    {
        OUString aRetval = aBaseName;
        if(nCount)
        {
            aRetval += OUString::number(nCount);
        }
        if (isMasterPageLayoutNameUnique(rDoc, aRetval))
            return aRetval;
        nCount++;
    }
}

void SdDrawDocument::SetMasterPage(sal_uInt16 nSdPageNum,
                                   std::u16string_view rLayoutName,
                                   SdDrawDocument* pSourceDoc,
                                   bool bMaster,
                                   bool bCheckMasters)
{
    SfxUndoManager* pUndoMgr = nullptr;

    if( mpDocSh )
    {
        mpDocSh->SetWaitCursor( true );
        pUndoMgr = mpDocSh->GetUndoManager();
    }

    const bool bUndo = pUndoMgr && IsUndoEnabled();

    if (bUndo)
    {
        ViewShellId nViewShellId(-1);
        if (sd::ViewShell* pViewShell = mpDocSh->GetViewShell())
            nViewShellId = pViewShell->GetViewShellBase().GetViewShellId();
        pUndoMgr->EnterListAction(SdResId(STR_UNDO_SET_PRESLAYOUT), OUString(), 0, nViewShellId);
    }

    SdPage* pSelectedPage   = GetSdPage(nSdPageNum, PageKind::Standard);
    SdPage* pNotes          = static_cast<SdPage*>( GetPage(pSelectedPage->GetPageNum()+1) );
    SdPage& rOldMaster      = static_cast<SdPage&>(pSelectedPage->TRG_GetMasterPage());
    SdPage& rOldNotesMaster = static_cast<SdPage&>(pNotes->TRG_GetMasterPage());
    rtl::Reference<SdPage> pMaster;
    rtl::Reference<SdPage> pNotesMaster;
    OUString aOldPageLayoutName(pSelectedPage->GetLayoutName());
    OUString aOldLayoutName = GetBaseLayoutName(aOldPageLayoutName);

    if (pSourceDoc)
    {
        std::vector<StyleReplaceData> aReplList; // List of replaced stylesheets
        bool bLayoutReloaded = false;   // Was ex. layout reloaded?

        // LayoutName, Page and Notes page
        if (rLayoutName.empty())
        {
            // No LayoutName: take first MasterPage
            pMaster = pSourceDoc->GetMasterSdPage(0, PageKind::Standard);
            pNotesMaster = pSourceDoc->GetMasterSdPage(0, PageKind::Notes);
        }
        else
        {
            OUString aSearchFor
                = OUString::Concat(rLayoutName) + SD_LT_SEPARATOR + STR_LAYOUT_OUTLINE;

            for (sal_uInt16 nMP = 0; nMP < pSourceDoc->GetMasterPageCount(); ++nMP)
            {
                SdPage* pMP = static_cast<SdPage*>( pSourceDoc->GetMasterPage(nMP) );

                if (pMP->GetLayoutName() == aSearchFor)
                {
                    if (pMP->GetPageKind() == PageKind::Standard)
                        pMaster = pMP;
                    if (pMP->GetPageKind() == PageKind::Notes)
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
                pMaster = pSourceDoc->GetMasterSdPage(0, PageKind::Standard);
                pNotesMaster = pSourceDoc->GetMasterSdPage(0, PageKind::Notes);
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
            rtl::Reference<SdPage> pNewNotesMaster(dynamic_cast< SdPage* >(pNotesMaster->CloneSdrPage(*this).get()));
            rtl::Reference<SdPage> pNewMaster(dynamic_cast< SdPage* >(pMaster->CloneSdrPage(*this).get()));

            if(!pNewNotesMaster || !pNewMaster)
            {
                OSL_FAIL("SdDrawDocument::SetMasterPage() cloning of MasterPage/NoteAmsterPage failed!" );
                return;
            }

            pNotesMaster = std::move(pNewNotesMaster);
            pMaster = std::move(pNewMaster);

            // layout name needs to be unique
            aTargetNewLayoutName = pMaster->GetLayoutName();
            sal_Int32 nIndex2 = aTargetNewLayoutName.indexOf(SD_LT_SEPARATOR);
            if( nIndex2 != -1 )
                aTargetNewLayoutName = aTargetNewLayoutName.copy(0, nIndex2);

            if(!isMasterPageLayoutNameUnique(*this, aTargetNewLayoutName))
            {
                aTargetNewLayoutName = createNewMasterPageLayoutName(*this);

                OUString aTemp = aTargetNewLayoutName + SD_LT_SEPARATOR + STR_LAYOUT_OUTLINE;

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

            StyleSheetCopyResultVector aCreatedStyles;          // List of created stylesheets
            SfxStyleSheetBase* pHisSheet = pSourceStyleSheetPool->First(SfxStyleFamily::Page);

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
                        aName = aTargetNewLayoutName + aName.subView(nPos);
                    }

                    SfxStyleSheet* pMySheet = static_cast<SfxStyleSheet*>( mxStyleSheetPool->Find(aName, SfxStyleFamily::Page) );

                    if (pMySheet)
                    {
                        // A stylesheet of the same name already exists -> overwrite contents
                        bool bTest = pMySheet->SetName(pHisSheet->GetName());
                        DBG_ASSERT(bTest, "Renaming StyleSheet failed.");
                        pMySheet->GetItemSet().ClearItem();  // Delete all

                        if (bUndo)
                        {
                            pUndoMgr->AddUndoAction(std::make_unique<StyleSheetUndoAction>(this,
                                                                 *pMySheet, &pHisSheet->GetItemSet()));
                        }
                        pMySheet->GetItemSet().Put(pHisSheet->GetItemSet());
                        pMySheet->Broadcast(SfxHint(SfxHintId::DataChanged));
                    }
                    else
                    {
                        // create new style
                        OUString aHelpFile;
                        pMySheet = static_cast<SfxStyleSheet*>( &mxStyleSheetPool->Make(aName, SfxStyleFamily::Page, pHisSheet->GetMask()) );
                        pMySheet->SetHelpId( aHelpFile, pHisSheet->GetHelpId(aHelpFile) );
                        pMySheet->GetItemSet().ClearItem();  // Delete all
                        pMySheet->GetItemSet().Put(pHisSheet->GetItemSet());

                        aCreatedStyles.emplace_back(static_cast<SdStyleSheet*>(pMySheet), true);
                    }

                    StyleReplaceData aReplData;
                    aReplData.nNewFamily = pMySheet->GetFamily();
                    aReplData.nFamily    = pMySheet->GetFamily();
                    aReplData.aNewName   = pMySheet->GetName();

                    // #i121863# re-create original name of style used at page where to replace with
                    // this new style
                    OUString aTemp(pMySheet->GetName());
                    const sal_Int32 nPos(aTemp.indexOf(SD_LT_SEPARATOR));
                    aTemp = aOldLayoutName + aTemp.subView(nPos);
                    aReplData.aName = aTemp;
                    aReplList.push_back(aReplData);
                }

                pHisSheet = pSourceStyleSheetPool->Next();
            }

            // If new styles were created: re-create parent chaining of the item
            // sets in the styles.
            if(!aCreatedStyles.empty())
            {
                for ( const auto& rRData : aReplList )
                {
                    SfxStyleSheetBase* pSOld = mxStyleSheetPool->Find(rRData.aName, SfxStyleFamily::Page);
                    SfxStyleSheetBase* pSNew = mxStyleSheetPool->Find(rRData.aNewName, SfxStyleFamily::Page);

                    if (pSOld && pSNew)
                    {
                        const OUString& rParentOfOld = pSOld->GetParent();
                        const OUString& rParentOfNew = pSNew->GetParent();

                        if (!rParentOfOld.isEmpty() && rParentOfNew.isEmpty())
                        {
                            std::vector<StyleReplaceData>::iterator pRDIter = std::find_if(aReplList.begin(), aReplList.end(),
                                [&rParentOfOld](const StyleReplaceData& rRD) { return (rRD.aName == rParentOfOld) && (rRD.aName != rRD.aNewName); });
                            if (pRDIter != aReplList.end())
                            {
                                OUString aParentOfNew(pRDIter->aNewName);
                                pSNew->SetParent(aParentOfNew);
                            }
                        }
                    }
                }
            }

            if (bUndo && !aCreatedStyles.empty())
            {
                // Add UndoAction for creating and inserting the stylesheets to
                // the top of the UndoManager
                pUndoMgr->AddUndoAction(std::make_unique<SdMoveStyleSheetsUndoAction>( this, aCreatedStyles, true));
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

        // Register the new master pages with the document and then use
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
            InsertMasterPage(pMaster.get(), nInsertPos);
            if( bUndo )
                AddUndo(GetSdrUndoFactory().CreateUndoNewPage(*pMaster));

            nInsertPos++;
            if (!bLayoutReloaded)
                nInsertPos = 0xFFFF;
            InsertMasterPage(pNotesMaster.get(), nInsertPos);
            if( bUndo )
            {
                AddUndo(GetSdrUndoFactory().CreateUndoNewPage(*pNotesMaster));

                EndUndo(); // do this here already, so Joe's actions happen _between_ our own.
            }
        }

        // Fill list with pages
        std::vector<rtl::Reference<SdPage>> aPageList;

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

        for (rtl::Reference<SdPage>& pPage : aPageList)
        {
            AutoLayout eAutoLayout = pPage->GetAutoLayout();

            if( bUndo )
            {
                pUndoMgr->AddUndoAction(std::make_unique<SdPresentationLayoutUndoAction>
                        (this,
                        pPage->IsMasterPage() ? aLayoutName : aOldLayoutName,
                        aLayoutName,
                         eAutoLayout, eAutoLayout, false, pPage.get()));
            }
            pPage->SetPresentationLayout(aLayoutName);
            pPage->SetAutoLayout(eAutoLayout);
        }

        // Adapt new master pages
        if (pSourceDoc != this)
        {
            Size aSize(rOldMaster.GetSize());
            ::tools::Rectangle aBorderRect(rOldMaster.GetLeftBorder(),
                                  rOldMaster.GetUpperBorder(),
                                  rOldMaster.GetRightBorder(),
                                  rOldMaster.GetLowerBorder());
            pMaster->ScaleObjects(aSize, aBorderRect, true);
            pMaster->SetSize(aSize);
            pMaster->SetBorder(rOldMaster.GetLeftBorder(),
                               rOldMaster.GetUpperBorder(),
                               rOldMaster.GetRightBorder(),
                               rOldMaster.GetLowerBorder());
            pMaster->SetOrientation( rOldMaster.GetOrientation() );
            pMaster->SetAutoLayout(pMaster->GetAutoLayout());

            aSize = rOldNotesMaster.GetSize();
            ::tools::Rectangle aNotesBorderRect(rOldNotesMaster.GetLeftBorder(),
                                       rOldNotesMaster.GetUpperBorder(),
                                       rOldNotesMaster.GetRightBorder(),
                                       rOldNotesMaster.GetLowerBorder());
            pNotesMaster->ScaleObjects(aSize, aNotesBorderRect, true);
            pNotesMaster->SetSize(aSize);
            pNotesMaster->SetBorder(rOldNotesMaster.GetLeftBorder(),
                                    rOldNotesMaster.GetUpperBorder(),
                                    rOldNotesMaster.GetRightBorder(),
                                    rOldNotesMaster.GetLowerBorder());
            pNotesMaster->SetOrientation( rOldNotesMaster.GetOrientation() );
            pNotesMaster->SetAutoLayout(pNotesMaster->GetAutoLayout());

            if( (pSourceDoc->GetDocumentType() == DocumentType::Impress) &&
                (GetDocumentType() == DocumentType::Draw) )
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
        OUString aPageLayoutName(aName + SD_LT_SEPARATOR + STR_LAYOUT_OUTLINE);

        // Generate new stylesheets
        static_cast<SdStyleSheetPool*>( mxStyleSheetPool.get())->CreateLayoutStyleSheets(aName);
        SdStyleSheetVector aCreatedStyles;
        static_cast<SdStyleSheetPool*>( mxStyleSheetPool.get())->CreateLayoutSheetList(aName, aCreatedStyles);

        if( bUndo )
        {
            StyleSheetCopyResultVector aUndoInsert;
            aUndoInsert.reserve(aCreatedStyles.size());
            for (const auto& a : aCreatedStyles)
                aUndoInsert.emplace_back(a.get(), true);
            pUndoMgr->AddUndoAction(std::make_unique<SdMoveStyleSheetsUndoAction>(this, aUndoInsert, true));
            // Generate new master pages and register them with the document
            BegUndo();
        }

        pMaster = AllocSdPage(true);
        pMaster->SetSize(pSelectedPage->GetSize());
        pMaster->SetBorder(pSelectedPage->GetLeftBorder(),
                           pSelectedPage->GetUpperBorder(),
                           pSelectedPage->GetRightBorder(),
                           pSelectedPage->GetLowerBorder() );
        pMaster->SetName(aName);
        pMaster->SetLayoutName(aPageLayoutName);
        InsertMasterPage(pMaster.get());

        if( bUndo )
            AddUndo(GetSdrUndoFactory().CreateUndoNewPage(*pMaster));

        pMaster->SetAutoLayout(AUTOLAYOUT_NONE, true, true);

        pNotesMaster = AllocSdPage(true);
        pNotesMaster->SetPageKind(PageKind::Notes);
        pNotesMaster->SetSize(pNotes->GetSize());
        pNotesMaster->SetBorder(pNotes->GetLeftBorder(),
                                pNotes->GetUpperBorder(),
                                pNotes->GetRightBorder(),
                                pNotes->GetLowerBorder() );
        pNotesMaster->SetName(aName);
        pNotesMaster->SetLayoutName(aPageLayoutName);
        InsertMasterPage(pNotesMaster.get());

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
        for ( auto& rpPage : aPageList )
        {
            AutoLayout eOldAutoLayout = rpPage->GetAutoLayout();
            AutoLayout eNewAutoLayout =
                rpPage->GetPageKind() == PageKind::Standard ? AUTOLAYOUT_NONE : AUTOLAYOUT_NOTES;

            if( bUndo )
            {
                pUndoMgr->AddUndoAction(std::make_unique<SdPresentationLayoutUndoAction>
                            (this, aOldLayoutName, aName,
                             eOldAutoLayout, eNewAutoLayout, true,
                             rpPage));
            }

            rpPage->SetPresentationLayout(aName);
            rpPage->SetAutoLayout(eNewAutoLayout);
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
    SdrModel::Merge( rSourceModel, nFirstPageNum, nLastPageNum, nDestPos, bMergeMasterPages, bAllMasterPages, bUndo, bTreadSourceAsConst);

    // add style family for each new master page
    for( sal_uInt16 nMaster = nMasterPageCount; nMaster < GetMasterPageCount(); nMaster++ )
    {
        SdPage* pPage = static_cast< SdPage* >( GetMasterPage( nMaster ) );
        if( pPage && pPage->IsMasterPage() && (pPage->GetPageKind() == PageKind::Standard) )
        {
            // new master page created, add its style family
            SdStyleSheetPool* pStylePool = static_cast<SdStyleSheetPool*>( GetStyleSheetPool() );
            if( pStylePool )
                pStylePool->AddStyleFamily( pPage );
        }
    }
}

SdPage* SdDrawDocument::AddNewMasterPageFromExisting(
    SdPage* pSourceMasterPage,
    SdDrawDocument* pBookmarkDoc,
    bool bUndo,
    const OUString& sNewName)
{
    // Check if the source master page is valid and a master page
    assert(pSourceMasterPage && pSourceMasterPage->IsMasterPage() &&
        "AddNewMasterPageFromExisting: Source must be a valid master page");

    // If bookmark document not provided, assume it's from this document
    if (!pBookmarkDoc)
        pBookmarkDoc = this;

    // Setup undo management
    SfxUndoManager* pUndoMgr = nullptr;
    if (bUndo && IsUndoEnabled())
    {
        pUndoMgr = GetDocSh()->GetUndoManager();
        pUndoMgr->EnterListAction(SdResId(STR_INSERTPAGE), OUString(), 0, ViewShellId(-1));
    }

    // Find a new name for the layout or use provided name
    OUString aName = sNewName.isEmpty() ? createNewMasterPageLayoutName(*this) : sNewName;
    OUString aPageLayoutName(aName + SD_LT_SEPARATOR + STR_LAYOUT_OUTLINE);

    // Check if the layout name is unique
    // If not, generate a new name until it is unique
    while (!isMasterPageLayoutNameUnique(*this, aName))
    {
        aName = GenerateNewLayoutName(aName);
        aPageLayoutName = aName + SD_LT_SEPARATOR + STR_LAYOUT_OUTLINE;
    }

    // Generate new stylesheets
    SdStyleSheetPool* pStylePool = static_cast<SdStyleSheetPool*>(mxStyleSheetPool.get());
    StyleSheetCopyResultVector aCreatedStyles;

    // Copy layout sheets from the source page's name to the new name
    OUString sSourceLayoutName = GetBaseLayoutName(pSourceMasterPage->GetLayoutName());

    // Create the style transfer context
    StyleTransferContext aStyleContext(
        static_cast<SdStyleSheetPool*>(pBookmarkDoc->mxStyleSheetPool.get()),
        pStylePool);

    // Copy layout sheets
    pStylePool->CopyLayoutSheets(sSourceLayoutName, *pStylePool, aCreatedStyles);

    if(!aCreatedStyles.empty() && bUndo && pUndoMgr)
    {
        pUndoMgr->AddUndoAction(std::make_unique<SdMoveStyleSheetsUndoAction>(this, aCreatedStyles, true));
    }

    if(bUndo && pUndoMgr)
        BegUndo();

    // Clone the master slide page across documents
    rtl::Reference<SdPage> pMaster(dynamic_cast<SdPage*>(
        pSourceMasterPage->CloneSdrPage(*this).get()));

    pMaster->SetName(aName);
    pMaster->SetLayoutName(aPageLayoutName);
    InsertMasterPage(pMaster.get());

    if(bUndo && pUndoMgr)
        AddUndo(GetSdrUndoFactory().CreateUndoNewPage(*pMaster));

    // Find the notes master page that corresponds to the source master page
    SdPage* pSourceNotesMaster = nullptr;
    for (sal_uInt16 i = 0; i < pBookmarkDoc->GetMasterPageCount(); i++)
    {
        SdPage* pPage = static_cast<SdPage*>(pBookmarkDoc->GetMasterPage(i));
        if (pPage && pPage->GetPageKind() == PageKind::Notes &&
            pPage->GetLayoutName() == pSourceMasterPage->GetLayoutName())
        {
            pSourceNotesMaster = pPage;
            break;
        }
    }

    // Clone the notes master page if found
    if (pSourceNotesMaster)
    {
        rtl::Reference<SdPage> pNotesMaster(dynamic_cast<SdPage*>(
            pSourceNotesMaster->CloneSdrPage(*this).get()));

        pNotesMaster->SetName(aName);
        pNotesMaster->SetLayoutName(aPageLayoutName);
        InsertMasterPage(pNotesMaster.get());

        if(bUndo && pUndoMgr)
            AddUndo(GetSdrUndoFactory().CreateUndoNewPage(*pNotesMaster));
    }

    if(bUndo && pUndoMgr)
    {
        EndUndo();
        pUndoMgr->LeaveListAction();
    }

    return pMaster.get();
}

// Paste pages from clipboard - handles regular paste operations
bool SdDrawDocument::PasteBookmarkAsPage(
    const PageNameList &rBookmarkList,
    PageNameList *pExchangeList,
    sal_uInt16 nInsertPos,
    ::sd::DrawDocShell* pBookmarkDocSh,
    bool bMergeMasterPages,
    bool bMergeMasterPagesOnly)
{
    // Use predefined options for paste operation
    InsertBookmarkOptions options = InsertBookmarkOptions::ForPaste(bMergeMasterPages, bMergeMasterPagesOnly);

    // Create insertion parameters
    PageInsertionParams aInsertParams(nInsertPos, pExchangeList);

    // Get the bookmark document first so we can use it for scale object determination
    if (!initBookmarkDoc(pBookmarkDocSh, aInsertParams.pBookmarkDoc, aInsertParams.aBookmarkName))
        return false;

    DocumentPageCounts pageCounts(GetSdPageCount(PageKind::Standard),
                                 aInsertParams.pBookmarkDoc->GetSdPageCount(PageKind::Standard),
                                 GetMasterPageCount(),
                                 aInsertParams.pBookmarkDoc->GetMasterPageCount());


    if (!pageCounts.areValid())
    {
        return false;
    }

    // Retrieve page properties (size, borders, orientation) for main and notes pages.
    getPageProperties(aInsertParams.mainProps, aInsertParams.notesProps, pageCounts.nDestPageCount);

    // Determine if objects need to be scaled
    if (!determineScaleObjects(options.bNoDialogs, rBookmarkList, aInsertParams))
        return false;

    // Get the necessary presentation stylesheets and transfer them before
    // the pages, else, the text objects won't reference their styles anymore.
    SfxUndoManager* pUndoMgr = beginUndoAction();

    // Collect layout names that need to be transferred
    SlideLayoutNameList aLayoutsToTransfer;
    collectLayoutsToTransfer(rBookmarkList, aInsertParams.pBookmarkDoc, aLayoutsToTransfer, pageCounts, options.bMergeMasterPagesOnly);

    // Copy the style that we actually need.
    SdStyleSheetPool& rBookmarkStyleSheetPool = dynamic_cast<SdStyleSheetPool&>(*aInsertParams.pBookmarkDoc->GetStyleSheetPool());
    SdStyleSheetPool& rStyleSheetPool = dynamic_cast<SdStyleSheetPool&>(*GetStyleSheetPool());

    // When copying styles, also copy the master pages!
    if (!aLayoutsToTransfer.empty())
        options.bMergeMasterPages = true;

    // Create a StyleTransferContext with the style sheet pools
    StyleTransferContext aStyleContext(&rBookmarkStyleSheetPool, &rStyleSheetPool);

    // Transfer layout styles
    transferLayoutStyles(aLayoutsToTransfer, aInsertParams.pBookmarkDoc, pUndoMgr, aStyleContext);

    // Copy styles. This unconditionally copies all styles, even those
    // that are not used in any of the inserted pages. The unused styles
    // are then removed at the end of the function, where we also create
    // undo records for the inserted styles.
    copyStyles(options.bReplace, options.bNoDialogs, aStyleContext);

    aInsertParams.bUndo = IsUndoEnabled();

    // Insert document
    if (aInsertParams.bUndo)
        BegUndo(SdResId(STR_UNDO_INSERTPAGES));

    // Insert pages based on whether all or selected pages are bookmarked.
    if (rBookmarkList.empty())
    {
        insertAllPages(aInsertParams, options, pageCounts);//reblce by pageCounts to add the masterpagecount
    }
    else
    {
        // Insert selected pages
        insertSelectedPages(rBookmarkList, aInsertParams, options);
    }

    // Remove duplicate master pages that may have been created.
    if (!options.bMergeMasterPagesOnly)
        removeDuplicateMasterPages(aInsertParams, pageCounts);

    // nInsertPos > 2 is always true when inserting into non-empty models
    if (nInsertPos > 0) {
        updateInsertedPages(aInsertParams, options, pageCounts, aStyleContext);
    }

    // Make absolutely sure no double masterpages are there
    if (!options.bMergeMasterPagesOnly)
        RemoveUnnecessaryMasterPages(nullptr, true);

    // Rename object styles if necessary
    renameObjectStylesIfNeeded(nInsertPos, aStyleContext, pageCounts.nSourcePageCount);

    // Clean up any copied styles not used and record undo actions.
    // remove copied styles not used on any inserted page and create
    // undo records
    // WARNING: SdMoveStyleSheetsUndoAction clears the passed list of
    // styles, so it cannot be used after this point
    cleanupStyles(pUndoMgr, aStyleContext);

    // End undo action
    endUndoAction(aInsertParams.bUndo, pUndoMgr);

    return true;
}

// Resolve page links
bool SdDrawDocument::ResolvePageLinks(
    const PageNameList &rBookmarkList,
    sal_uInt16 nInsertPos,
    bool bNoDialogs,
    bool bCopy,
    bool bMergeMasterPagesOnly)
{
    // Use predefined options for page link resolution
    InsertBookmarkOptions options = InsertBookmarkOptions::ForPageLinks(bCopy, bNoDialogs);
    options.bMergeMasterPagesOnly = bMergeMasterPagesOnly;

    // Create insertion parameters
    PageInsertionParams aInsertParams(nInsertPos);

    // Get the bookmark document first so we can use it for scale object determination
    if (!initBookmarkDoc(nullptr, aInsertParams.pBookmarkDoc, aInsertParams.aBookmarkName))
        return false;

    DocumentPageCounts pageCounts(GetSdPageCount(PageKind::Standard),
                                 aInsertParams.pBookmarkDoc->GetSdPageCount(PageKind::Standard),
                                 GetMasterPageCount(),
                                 aInsertParams.pBookmarkDoc->GetMasterPageCount());

    if (!pageCounts.areValid())
    {
        return false;
    }

    // Retrieve page properties (size, borders, orientation) for main and notes pages
    getPageProperties(aInsertParams.mainProps, aInsertParams.notesProps, pageCounts.nDestPageCount);

    if (!determineScaleObjects(options.bNoDialogs, rBookmarkList, aInsertParams))
        return false;

    // Get the necessary presentation stylesheets and transfer them before
    // the pages, else, the text objects won't reference their styles anymore.
    SfxUndoManager* pUndoMgr = beginUndoAction();

    // Collect layout names that need to be transferred
    SlideLayoutNameList aLayoutsToTransfer;
    collectLayoutsToTransfer(rBookmarkList, aInsertParams.pBookmarkDoc, aLayoutsToTransfer, pageCounts, options.bMergeMasterPagesOnly);

    // Copy the style that we actually need.
    SdStyleSheetPool& rBookmarkStyleSheetPool = dynamic_cast<SdStyleSheetPool&>(*aInsertParams.pBookmarkDoc->GetStyleSheetPool());
    SdStyleSheetPool& rStyleSheetPool = dynamic_cast<SdStyleSheetPool&>(*GetStyleSheetPool());

    // Create a StyleTransferContext with the style sheet pools
    StyleTransferContext aStyleContext(&rBookmarkStyleSheetPool, &rStyleSheetPool);

    // Transfer layout styles
    transferLayoutStyles(aLayoutsToTransfer, aInsertParams.pBookmarkDoc, pUndoMgr, aStyleContext);

    // Copy styles. This unconditionally copies all styles, even those
    // that are not used in any of the inserted pages. The unused styles
    // are then removed at the end of the function, where we also create
    // undo records for the inserted styles.
    copyStyles(options.bReplace, options.bNoDialogs, aStyleContext);

    aInsertParams.bUndo = IsUndoEnabled();

    // Insert document
    if (aInsertParams.bUndo)
        BegUndo(SdResId(STR_UNDO_INSERTPAGES));

    if (rBookmarkList.empty())
    {
        insertAllPages(aInsertParams, options, pageCounts);
    }
    else
    {
        // Insert selected pages
        insertSelectedPages(rBookmarkList, aInsertParams, options);
    }

    // Remove duplicate master pages that may have been created.
    if (!options.bMergeMasterPagesOnly)
        removeDuplicateMasterPages(aInsertParams, pageCounts);

    // Update inserted pages (scaling objects if needed, etc)
    updateInsertedPages(aInsertParams, options, pageCounts, aStyleContext);

    // Make absolutely sure no double masterpages are there
    if (!options.bMergeMasterPagesOnly)
        RemoveUnnecessaryMasterPages(nullptr, true);

    // Rename object styles if necessary
    renameObjectStylesIfNeeded(nInsertPos, aStyleContext, pageCounts.nSourcePageCount);

    // Clean up any copied styles not used and record undo actions.
    // remove copied styles not used on any inserted page and create
    // undo records
    // WARNING: SdMoveStyleSheetsUndoAction clears the passed list of
    // styles, so it cannot be used after this point
    cleanupStyles(pUndoMgr, aStyleContext);

    // End undo action
    endUndoAction(aInsertParams.bUndo, pUndoMgr);

    return true;
}

// Import a whole document
bool SdDrawDocument::ImportDocumentPages(
    const PageNameList &rBookmarkList,
    sal_uInt16 nInsertPos,
    ::sd::DrawDocShell* pBookmarkDocSh,
    bool bMergeMasterPagesOnly)
{
    // Use predefined options for document import
    InsertBookmarkOptions options = InsertBookmarkOptions::ForDocumentImport(bMergeMasterPagesOnly);

    // Create parameter object for page insertion
    PageInsertionParams aInsertParams(nInsertPos);

    // Initialize bookmark document
    if (!initBookmarkDoc(pBookmarkDocSh, aInsertParams.pBookmarkDoc, aInsertParams.aBookmarkName))
        return false;

    DocumentPageCounts pageCounts(GetSdPageCount(PageKind::Standard),
                                  aInsertParams.pBookmarkDoc->GetSdPageCount(PageKind::Standard),
                                  GetMasterPageCount(),
                                  aInsertParams.pBookmarkDoc->GetMasterPageCount());

    if (!pageCounts.areValid())
    {
        return false;
    }

    // Retrieve page properties (size, borders, orientation) for main and notes pages.
    getPageProperties(aInsertParams.mainProps, aInsertParams.notesProps, pageCounts.nDestPageCount);

    // Determine if objects need to be scaled
    if (!determineScaleObjects(options.bNoDialogs, rBookmarkList, aInsertParams))
        return false;

    // Start undo action
    SfxUndoManager* pUndoMgr = beginUndoAction();

    // Collect layout names that need to be transferred
    SlideLayoutNameList aLayoutsToTransfer;
    collectLayoutsToTransfer(rBookmarkList, aInsertParams.pBookmarkDoc, aLayoutsToTransfer, pageCounts, options.bMergeMasterPagesOnly);

    // Copy the style that we actually need.
    SdStyleSheetPool& rBookmarkStyleSheetPool = dynamic_cast<SdStyleSheetPool&>(*aInsertParams.pBookmarkDoc->GetStyleSheetPool());
    SdStyleSheetPool& rStyleSheetPool = dynamic_cast<SdStyleSheetPool&>(*GetStyleSheetPool());


    // Create a StyleTransferContext with the style sheet pools
    StyleTransferContext aStyleContext(&rBookmarkStyleSheetPool, &rStyleSheetPool);

    // Transfer layout styles
    transferLayoutStyles(aLayoutsToTransfer, aInsertParams.pBookmarkDoc, pUndoMgr, aStyleContext);

    // Copy styles. This unconditionally copies all styles, even those
    // that are not used in any of the inserted pages. The unused styles
    // are then removed at the end of the function, where we also create
    // undo records for the inserted styles.
    copyStyles(options.bReplace, options.bNoDialogs, aStyleContext);

    aInsertParams.bUndo = IsUndoEnabled();

    if (aInsertParams.bUndo)
        BegUndo(SdResId(STR_UNDO_INSERTPAGES));

    // For document import, we typically import all pages
    if (rBookmarkList.empty())
    {
        insertAllPages(aInsertParams, options, pageCounts);
    }
    else
    {
        // If specific pages were selected, import only those
        insertSelectedPages(rBookmarkList, aInsertParams, options);
    }

    // Remove duplicate master pages that may have been created.
    if (!options.bMergeMasterPagesOnly)
        removeDuplicateMasterPages(aInsertParams, pageCounts);

    // nInsertPos > 2 is always true when inserting into non-empty models
    if (nInsertPos > 0) {
        updateInsertedPages(aInsertParams, options, pageCounts, aStyleContext);
    }

    // Make absolutely sure no double masterpages are there
    if (!options.bMergeMasterPagesOnly)
        RemoveUnnecessaryMasterPages(nullptr, true);

    // Rename object styles if necessary
    renameObjectStylesIfNeeded(nInsertPos, aStyleContext, pageCounts.nSourcePageCount);

    // Clean up any copied styles not used and record undo actions.
    cleanupStyles(pUndoMgr, aStyleContext);

    // End undo action
    endUndoAction(aInsertParams.bUndo, pUndoMgr);

    return true;
}

// Insert pages from external files
bool SdDrawDocument::InsertFileAsPage(
    const PageNameList &rBookmarkList,
    PageNameList *pExchangeList,
    bool bLink,
    sal_uInt16 nInsertPos,
    ::sd::DrawDocShell* pBookmarkDocSh,
    bool bMergeMasterPagesOnly)
{
    // Use predefined options for file insert operation
    InsertBookmarkOptions options = InsertBookmarkOptions::ForFileInsert(bLink);
    options.bMergeMasterPagesOnly = bMergeMasterPagesOnly;

    // Create parameter object for page insertion
    PageInsertionParams aInsertParams(nInsertPos, pExchangeList);

    // Initialize bookmark document
    if (!initBookmarkDoc(pBookmarkDocSh, aInsertParams.pBookmarkDoc, aInsertParams.aBookmarkName))
        return false;

    DocumentPageCounts pageCounts(GetSdPageCount(PageKind::Standard),
                                  aInsertParams.pBookmarkDoc->GetSdPageCount(PageKind::Standard),
                                  GetMasterPageCount(),
                                  aInsertParams.pBookmarkDoc->GetMasterPageCount());

    if (!pageCounts.areValid())
    {
        return false;
    }

    // Retrieve page properties (size, borders, orientation) for main and notes pages.
    getPageProperties(aInsertParams.mainProps, aInsertParams.notesProps, pageCounts.nDestPageCount);

    // Determine if objects need to be scaled
    if (!determineScaleObjects(options.bNoDialogs, rBookmarkList, aInsertParams))
        return false;

    // Get the necessary presentation stylesheets and transfer them before
    // the pages, else, the text objects won't reference their styles anymore.
    SfxUndoManager* pUndoMgr = beginUndoAction();

    // Collect layout names that need to be transferred
    SlideLayoutNameList aLayoutsToTransfer;
    collectLayoutsToTransfer(rBookmarkList, aInsertParams.pBookmarkDoc, aLayoutsToTransfer, pageCounts, options.bMergeMasterPagesOnly);

    // Copy the style that we actually need.
    SdStyleSheetPool& rBookmarkStyleSheetPool = dynamic_cast<SdStyleSheetPool&>(*aInsertParams.pBookmarkDoc->GetStyleSheetPool());
    SdStyleSheetPool& rStyleSheetPool = dynamic_cast<SdStyleSheetPool&>(*GetStyleSheetPool());

    // Create a StyleTransferContext with the style sheet pools
    StyleTransferContext aStyleContext(&rBookmarkStyleSheetPool, &rStyleSheetPool);

    // Transfer layout styles
    transferLayoutStyles(aLayoutsToTransfer, aInsertParams.pBookmarkDoc, pUndoMgr, aStyleContext);

    // Copy styles. This unconditionally copies all styles, even those
    // that are not used in any of the inserted pages. The unused styles
    // are then removed at the end of the function, where we also create
    // undo records for the inserted styles.
    copyStyles(options.bReplace, options.bNoDialogs, aStyleContext);

    aInsertParams.bUndo = IsUndoEnabled();

    if (aInsertParams.bUndo)
        BegUndo(SdResId(STR_UNDO_INSERTPAGES));

    if (rBookmarkList.empty())
    {
        insertAllPages(aInsertParams, options, pageCounts);
    }
    else
    {
        // Insert selected pages
        insertSelectedPages(rBookmarkList, aInsertParams, options);
    }

    // Remove duplicate master pages that may have been created.
    if (!options.bMergeMasterPagesOnly)
        removeDuplicateMasterPages(aInsertParams, pageCounts);

    // nInsertPos > 2 is always true when inserting into non-empty models
    if (nInsertPos > 0) {
        updateInsertedPages(aInsertParams, options, pageCounts, aStyleContext);
    }

    // Make absolutely sure no double masterpages are there
    if (!options.bMergeMasterPagesOnly)
        RemoveUnnecessaryMasterPages(nullptr, true);

    // Rename object styles if necessary
    renameObjectStylesIfNeeded(nInsertPos, aStyleContext, pageCounts.nSourcePageCount);

    // Clean up any copied styles not used and record undo actions.
    cleanupStyles(pUndoMgr, aStyleContext);

    // End undo action
    endUndoAction(aInsertParams.bUndo, pUndoMgr);

    return true;
}

// Handle drag and drop operations
bool SdDrawDocument::DropBookmarkAsPage(
    const PageNameList &rBookmarkList,
    sal_uInt16 nInsertPos,
    ::sd::DrawDocShell* pBookmarkDocSh,
    bool bMergeMasterPages,
    bool bMergeMasterPagesOnly)
{
    // Use predefined options for drag and drop operation
    InsertBookmarkOptions options = InsertBookmarkOptions::ForDragDrop(bMergeMasterPages, bMergeMasterPagesOnly);

    // Create parameter object for page insertion
    PageInsertionParams aInsertParams(nInsertPos);

    // Initialize bookmark document
    if (!initBookmarkDoc(pBookmarkDocSh, aInsertParams.pBookmarkDoc, aInsertParams.aBookmarkName))
        return false;

    DocumentPageCounts pageCounts(GetSdPageCount(PageKind::Standard),
                                  aInsertParams.pBookmarkDoc->GetSdPageCount(PageKind::Standard),
                                  GetMasterPageCount(),
                                  aInsertParams.pBookmarkDoc->GetMasterPageCount());

    if (!pageCounts.areValid())
    {
        return false;
    }

    // Retrieve page properties (size, borders, orientation) for main and notes pages.
    getPageProperties(aInsertParams.mainProps, aInsertParams.notesProps, pageCounts.nDestPageCount);

    // Determine if objects need to be scaled
    if (!determineScaleObjects(options.bNoDialogs, rBookmarkList, aInsertParams))
        return false;

    // Get the necessary presentation stylesheets and transfer them before
    // the pages, else, the text objects won't reference their styles anymore.
    SfxUndoManager* pUndoMgr = beginUndoAction();

    // Collect layout names that need to be transferred
    SlideLayoutNameList aLayoutsToTransfer;
    collectLayoutsToTransfer(rBookmarkList, aInsertParams.pBookmarkDoc, aLayoutsToTransfer, pageCounts, options.bMergeMasterPagesOnly);

    // Copy the style that we actually need.
    SdStyleSheetPool& rBookmarkStyleSheetPool = dynamic_cast<SdStyleSheetPool&>(*aInsertParams.pBookmarkDoc->GetStyleSheetPool());
    SdStyleSheetPool& rStyleSheetPool = dynamic_cast<SdStyleSheetPool&>(*GetStyleSheetPool());

    // Create a StyleTransferContext with the style sheet pools
    StyleTransferContext aStyleContext(&rBookmarkStyleSheetPool, &rStyleSheetPool);

    // Transfer layout styles
    transferLayoutStyles(aLayoutsToTransfer, aInsertParams.pBookmarkDoc, pUndoMgr, aStyleContext);

    // Copy styles. This unconditionally copies all styles, even those
    // that are not used in any of the inserted pages. The unused styles
    // are then removed at the end of the function, where we also create
    // undo records for the inserted styles.
    copyStyles(options.bReplace, options.bNoDialogs, aStyleContext);

    aInsertParams.bUndo = IsUndoEnabled();

    if (aInsertParams.bUndo)
        BegUndo(SdResId(STR_UNDO_INSERTPAGES));

    // Insert pages based on whether all or selected pages are bookmarked.
    if (rBookmarkList.empty())
    {
        insertAllPages(aInsertParams, options, pageCounts);
    }
    else
    {
        // Insert selected pages
        insertSelectedPages(rBookmarkList, aInsertParams, options);
    }

    // Remove duplicate master pages that may have been created.
    if (!options.bMergeMasterPagesOnly)
        removeDuplicateMasterPages(aInsertParams, pageCounts);

    // nInsertPos > 2 is always true when inserting into non-empty models
    if (nInsertPos > 0) {
        updateInsertedPages(aInsertParams, options, pageCounts, aStyleContext);
    }

    // Make absolutely sure no double masterpages are there
    if (!options.bMergeMasterPagesOnly)
        RemoveUnnecessaryMasterPages(nullptr, true);

    // Rename object styles if necessary
    renameObjectStylesIfNeeded(nInsertPos, aStyleContext, pageCounts.nSourcePageCount);

    // Clean up any copied styles not used and record undo actions.
    cleanupStyles(pUndoMgr, aStyleContext);

    // End undo action
    endUndoAction(aInsertParams.bUndo, pUndoMgr);

    return true;
}

// Copy or move pages within the same document
bool SdDrawDocument::CopyOrMovePagesWithinDocument(
    const PageNameList &rBookmarkList,
    PageNameList *pExchangeList,
    sal_uInt16 nInsertPos,
    bool bPreservePageNames,
    bool bMergeMasterPagesOnly)
{
    // Use predefined options for internal document operations
    InsertBookmarkOptions options = InsertBookmarkOptions::ForInternalOps(bPreservePageNames, bMergeMasterPagesOnly);

    // Create parameter object for page insertion
    // When copying within document, source and target are the same
    PageInsertionParams aInsertParams(nInsertPos, pExchangeList, this);

    DocumentPageCounts pageCounts(GetSdPageCount(PageKind::Standard),
                                  aInsertParams.pBookmarkDoc->GetSdPageCount(PageKind::Standard),
                                  GetMasterPageCount(),
                                  aInsertParams.pBookmarkDoc->GetMasterPageCount());

    if (!pageCounts.areValid())
    {
        return false;
    }

    getPageProperties(aInsertParams.mainProps, aInsertParams.notesProps, pageCounts.nDestPageCount);

    if (!determineScaleObjects(options.bNoDialogs, rBookmarkList, aInsertParams))
        return false;

    // Get the necessary presentation stylesheets and transfer them before
    // the pages, else, the text objects won't reference their styles anymore.
    SfxUndoManager* pUndoMgr = beginUndoAction();
    aInsertParams.bUndo = IsUndoEnabled();

    if (aInsertParams.bUndo)
        BegUndo(SdResId(STR_UNDO_INSERTPAGES));

    // Insert selected pages
    insertSelectedPages(rBookmarkList, aInsertParams, options);

    // Remove duplicate master pages that may have been created.
    if (!options.bMergeMasterPagesOnly)
        removeDuplicateMasterPages(aInsertParams, pageCounts);

    // nInsertPos > 2 is always true when inserting into non-empty models
    StyleTransferContext aStyleContext;
    if (nInsertPos > 0) {
        updateInsertedPages(aInsertParams, options, pageCounts, aStyleContext);
    }

    // No need to remove unnecessary master pages when copying within the same document
    // No need to rename object styles when copying within the same document
    // No need to clean up styles when copying within the same document

    // End undo action
    endUndoAction(aInsertParams.bUndo, pUndoMgr);

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
