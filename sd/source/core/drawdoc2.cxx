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

#include <com/sun/star/embed/XVisualObject.hpp>
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <vcl/wrkwin.hxx>
#include <vcl/settings.hxx>

#include <sfx2/printer.hxx>
#include <sfx2/app.hxx>
#include "Outliner.hxx"
#include <editeng/paperinf.hxx>
#include <svx/svdopage.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdundo.hxx>
#include <vcl/svapp.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/langitem.hxx>
#include <svl/itempool.hxx>
#include <svx/svdpool.hxx>
#include <editeng/flditem.hxx>

#include <sfx2/linkmgr.hxx>
#include <editeng/editdata.hxx>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>

#include <editeng/outliner.hxx>
#include <svx/svditer.hxx>
#include <svtools/imapobj.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <boost/property_tree/json_parser.hpp>
#include <comphelper/lok.hxx>

#include "sdresid.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "pglink.hxx"
#include "glob.hrc"
#include "glob.hxx"
#include "stlpool.hxx"
#include "sdiocmpt.hxx"
#include "anminfo.hxx"
#include "imapinfo.hxx"
#include "cusshow.hxx"
#include "undo/undomanager.hxx"

#include "../ui/inc/DrawDocShell.hxx"
#include "../ui/inc/FrameView.hxx"
#include "../ui/inc/cfgids.hxx"
#include "../ui/inc/strings.hrc"

#include "PageListWatcher.hxx"
#include <vcl/virdev.hxx>
#include "customshowlist.hxx"

using namespace ::sd;

const long PRINT_OFFSET = 30;       // see /svx/source/dialog/page.cxx

using namespace com::sun::star;

// Looks up an object by name
SdrObject* SdDrawDocument::GetObj(const OUString& rObjName) const
{
    SdrObject* pObj = nullptr;
    SdrObject* pObjFound = nullptr;
    const SdPage* pPage = nullptr;

    // First search in all pages
    sal_uInt16 nPage = 0;
    const sal_uInt16 nMaxPages = GetPageCount();

    while (nPage < nMaxPages && !pObjFound)
    {
        pPage = static_cast<const SdPage*>( GetPage(nPage) );
        SdrObjListIter aIter(*pPage, IM_DEEPWITHGROUPS);

        while (aIter.IsMore() && !pObjFound)
        {
            pObj = aIter.Next();

            if( ( pObj->GetName().equals(rObjName) ) ||
                ( SdrInventor == pObj->GetObjInventor() &&
                  OBJ_OLE2 == pObj->GetObjIdentifier() &&
                  rObjName == static_cast< SdrOle2Obj* >( pObj )->GetPersistName() ) )
            {
                pObjFound = pObj;
            }
        }

        nPage++;
    }

    // If it couldn't be found, look through all master pages
    nPage = 0;
    const sal_uInt16 nMaxMasterPages = GetMasterPageCount();

    while (nPage < nMaxMasterPages && !pObjFound)
    {
        pPage = static_cast<const SdPage*>( GetMasterPage(nPage) );
        SdrObjListIter aIter(*pPage, IM_DEEPWITHGROUPS);

        while (aIter.IsMore() && !pObjFound)
        {
            pObj = aIter.Next();

            if( ( pObj->GetName().equals(rObjName) ) ||
                ( SdrInventor == pObj->GetObjInventor() &&
                  OBJ_OLE2 == pObj->GetObjIdentifier() &&
                  rObjName == static_cast< SdrOle2Obj* >( pObj )->GetPersistName() ) )
            {
                pObjFound = pObj;
            }
        }

        nPage++;
    }

    return pObjFound;
}

// Find SdPage by name
sal_uInt16 SdDrawDocument::GetPageByName(const OUString& rPgName, bool& rbIsMasterPage) const
{
    SdPage* pPage = nullptr;
    sal_uInt16 nPage = 0;
    const sal_uInt16 nMaxPages = GetPageCount();
    sal_uInt16 nPageNum = SDRPAGE_NOTFOUND;

    rbIsMasterPage = false;

    // Search all regular pages and all notes pages (handout pages are
    // ignored)
    while (nPage < nMaxPages && nPageNum == SDRPAGE_NOTFOUND)
    {
        pPage = const_cast<SdPage*>(static_cast<const SdPage*>(
            GetPage(nPage)));

        if (pPage != nullptr
            && pPage->GetPageKind() != PK_HANDOUT
            && pPage->GetName() == rPgName)
        {
            nPageNum = nPage;
        }

        nPage++;
    }

    // Search all master pages when not found among non-master pages
    const sal_uInt16 nMaxMasterPages = GetMasterPageCount();
    nPage = 0;

    while (nPage < nMaxMasterPages && nPageNum == SDRPAGE_NOTFOUND)
    {
        pPage = const_cast<SdPage*>(static_cast<const SdPage*>(
            GetMasterPage(nPage)));

        if (pPage && pPage->GetName() == rPgName)
        {
            nPageNum = nPage;
            rbIsMasterPage = true;
        }

        nPage++;
    }

    return nPageNum;
}

SdPage* SdDrawDocument::GetSdPage(sal_uInt16 nPgNum, PageKind ePgKind) const
{
    return mpDrawPageListWatcher->GetSdPage(ePgKind, sal_uInt32(nPgNum));
}

sal_uInt16 SdDrawDocument::GetSdPageCount(PageKind ePgKind) const
{
    return (sal_uInt16)mpDrawPageListWatcher->GetSdPageCount(ePgKind);
}

SdPage* SdDrawDocument::GetMasterSdPage(sal_uInt16 nPgNum, PageKind ePgKind)
{
    return mpMasterPageListWatcher->GetSdPage(ePgKind, sal_uInt32(nPgNum));
}

sal_uInt16 SdDrawDocument::GetMasterSdPageCount(PageKind ePgKind) const
{
    return (sal_uInt16)mpMasterPageListWatcher->GetSdPageCount(ePgKind);
}

sal_uInt16 SdDrawDocument::GetActiveSdPageCount() const
{
    return (sal_uInt16)mpDrawPageListWatcher->GetVisibleSdPageCount();
}

// Adapt the page numbers that are registered in the page objects of the notes
// pages
void SdDrawDocument::UpdatePageObjectsInNotes(sal_uInt16 nStartPos)
{
    sal_uInt16  nPageCount  = GetPageCount();
    SdPage* pPage       = nullptr;

    for (sal_uInt16 nPage = nStartPos; nPage < nPageCount; nPage++)
    {
        pPage = static_cast<SdPage*>( GetPage(nPage) );

        // If this is a notes page, find its page object and correct the page
        // number
        if (pPage && pPage->GetPageKind() == PK_NOTES)
        {
            const size_t nObjCount = pPage->GetObjCount();
            for (size_t nObj = 0; nObj < nObjCount; ++nObj)
            {
                SdrObject* pObj = pPage->GetObj(nObj);
                if (pObj->GetObjIdentifier() == OBJ_PAGE &&
                    pObj->GetObjInventor() == SdrInventor)
                {
                    // The page object is the preceding page (drawing page)
                    SAL_WARN_IF(!nStartPos, "sd", "Position of notes page must not be 0.");

                    SAL_WARN_IF(nPage <= 1, "sd", "Page object must not be a handout.");

                    if (nStartPos > 0 && nPage > 1)
                        static_cast<SdrPageObj*>(pObj)->SetReferencedPage(GetPage(nPage - 1));
                }
            }
        }
    }
}

void SdDrawDocument::UpdatePageRelativeURLs(const OUString& rOldName, const OUString& rNewName)
{
    if (rNewName.isEmpty())
        return;

    SfxItemPool& rPool(GetPool());
    sal_uInt32 nCount = rPool.GetItemCount2(EE_FEATURE_FIELD);
    for (sal_uInt32 nOff = 0; nOff < nCount; nOff++)
    {
        const SfxPoolItem *pItem = rPool.GetItem2(EE_FEATURE_FIELD, nOff);
        const SvxFieldItem* pFldItem = dynamic_cast< const SvxFieldItem * > (pItem);

        if(pFldItem)
        {
            SvxURLField* pURLField = const_cast< SvxURLField* >( dynamic_cast<const SvxURLField*>( pFldItem->GetField() ) );

            if(pURLField)
            {
                OUString aURL = pURLField->GetURL();

                if (!aURL.isEmpty() && (aURL[0] == 35) && (aURL.indexOf(rOldName, 1) == 1))
                {
                    if (aURL.getLength() == rOldName.getLength() + 1) // standard page name
                    {
                        aURL = aURL.replaceAt(1, aURL.getLength() - 1, "");
                        aURL += rNewName;
                        pURLField->SetURL(aURL);
                    }
                    else
                    {
                        const OUString sNotes(SD_RESSTR(STR_NOTES));
                        if (aURL.getLength() == rOldName.getLength() + 2 + sNotes.getLength()
                            && aURL.indexOf(sNotes, rOldName.getLength() + 2) == rOldName.getLength() + 2)
                        {
                            aURL = aURL.replaceAt(1, aURL.getLength() - 1, "");
                            aURL += rNewName + " " + sNotes;
                            pURLField->SetURL(aURL);
                        }
                    }
                }
            }
        }
    }
}

void SdDrawDocument::UpdatePageRelativeURLs(SdPage* pPage, sal_uInt16 nPos, sal_Int32 nIncrement)
{
    bool bNotes = (pPage->GetPageKind() == PK_NOTES);

    SfxItemPool& rPool(GetPool());
    sal_uInt32 nCount = rPool.GetItemCount2(EE_FEATURE_FIELD);
    for (sal_uInt32 nOff = 0; nOff < nCount; nOff++)
    {
        const SfxPoolItem *pItem = rPool.GetItem2(EE_FEATURE_FIELD, nOff);
        const SvxFieldItem* pFldItem;

        if ((pFldItem = dynamic_cast< const SvxFieldItem * > (pItem)) != nullptr)
        {
            SvxURLField* pURLField = const_cast< SvxURLField* >( dynamic_cast<const SvxURLField*>( pFldItem->GetField() ) );

            if(pURLField)
            {
                OUString aURL = pURLField->GetURL();

                if (!aURL.isEmpty() && (aURL[0] == 35))
                {
                    OUString aHashSlide("#");
                    aHashSlide += SD_RESSTR(STR_PAGE);

                    if (aURL.startsWith(aHashSlide))
                    {
                        OUString aURLCopy = aURL;
                        const OUString sNotes(SD_RESSTR(STR_NOTES));

                        aURLCopy = aURLCopy.replaceAt(0, aHashSlide.getLength(), "");

                        bool bNotesLink = ( aURLCopy.getLength() >= sNotes.getLength() + 3
                            && aURLCopy.endsWith(sNotes) );

                        if (bNotesLink != bNotes)
                            continue; // no compatible link and page

                        if (bNotes)
                            aURLCopy = aURLCopy.replaceAt(aURLCopy.getLength() - sNotes.getLength(), sNotes.getLength(), "");

                        sal_Int32 number = aURLCopy.toInt32();
                        sal_uInt16 realPageNumber = (nPos + 1)/ 2;

                        if ( number >= realPageNumber )
                        {
                            // update link page number
                            number += nIncrement;
                            aURL = aURL.replaceAt(aHashSlide.getLength() + 1, aURL.getLength() - aHashSlide.getLength() - 1, "");
                            aURL += OUString::number(number);
                            if (bNotes)
                            {
                                aURL += " " + sNotes;
                            }
                            pURLField->SetURL(aURL);
                        }
                    }
                }
            }
        }
    }
}

// Move page
void SdDrawDocument::MovePage(sal_uInt16 nPgNum, sal_uInt16 nNewPos)
{
    FmFormModel::MovePage(nPgNum, nNewPos);

    sal_uInt16 nMin = std::min(nPgNum, nNewPos);

    UpdatePageObjectsInNotes(nMin);
}

// Insert page
void SdDrawDocument::InsertPage(SdrPage* pPage, sal_uInt16 nPos)
{
    bool bLast = (nPos == GetPageCount());

    FmFormModel::InsertPage(pPage, nPos);

    static_cast<SdPage*>(pPage)->ConnectLink();

    UpdatePageObjectsInNotes(nPos);

    if (!bLast)
        UpdatePageRelativeURLs(static_cast<SdPage*>( pPage ), nPos, 1);

    if (comphelper::LibreOfficeKit::isActive() &&
        static_cast<SdPage*>(pPage)->GetPageKind() == PK_STANDARD)
    {
        libreOfficeKitCallback(LOK_CALLBACK_DOCUMENT_SIZE_CHANGED, "");
    }
}

// Delete page
void SdDrawDocument::DeletePage(sal_uInt16 nPgNum)
{
    FmFormModel::DeletePage(nPgNum);

    UpdatePageObjectsInNotes(nPgNum);
}

// Remove page
SdrPage* SdDrawDocument::RemovePage(sal_uInt16 nPgNum)
{
    SdrPage* pPage = FmFormModel::RemovePage(nPgNum);

    bool bLast = ((nPgNum+1)/2 == (GetPageCount()+1)/2);

    static_cast<SdPage*>(pPage)->DisconnectLink();
    ReplacePageInCustomShows( dynamic_cast< SdPage* >( pPage ), nullptr );
    UpdatePageObjectsInNotes(nPgNum);

    if (!bLast)
        UpdatePageRelativeURLs(static_cast<SdPage*>(pPage), nPgNum, -1);

    if (comphelper::LibreOfficeKit::isActive() &&
        static_cast<SdPage*>(pPage)->GetPageKind() == PK_STANDARD)
    {
        libreOfficeKitCallback(LOK_CALLBACK_DOCUMENT_SIZE_CHANGED, "");
    }

    return pPage;
}

// Warning: This is not called for new master pages created from SdrModel::Merge,
// you also have to modify code in SdDrawDocument::Merge!
void SdDrawDocument::InsertMasterPage(SdrPage* pPage, sal_uInt16 nPos )
{
    FmFormModel::InsertMasterPage( pPage, nPos );
    if( pPage->IsMasterPage() && (static_cast<SdPage*>(pPage)->GetPageKind() == PK_STANDARD) )
    {
        // new master page created, add its style family
        SdStyleSheetPool* pStylePool = static_cast<SdStyleSheetPool*>( GetStyleSheetPool() );
        if( pStylePool )
            pStylePool->AddStyleFamily( static_cast<SdPage*>(pPage) );
    }
}

SdrPage* SdDrawDocument::RemoveMasterPage(sal_uInt16 nPgNum)
{
    SdPage* pPage = static_cast<SdPage*>(GetMasterPage(nPgNum ));
    if( pPage && pPage->IsMasterPage() && (pPage->GetPageKind() == PK_STANDARD) )
    {
        // master page removed, remove its style family
        SdStyleSheetPool* pStylePool = static_cast<SdStyleSheetPool*>( GetStyleSheetPool() );
        if( pStylePool )
            pStylePool->RemoveStyleFamily( pPage );
    }

    return FmFormModel::RemoveMasterPage(nPgNum);
}

//Select pages
void SdDrawDocument::SetSelected(SdPage* pPage, bool bSelect)
{
    PageKind ePageKind = pPage->GetPageKind();

    if (ePageKind == PK_STANDARD)
    {
        pPage->SetSelected(bSelect);

        const sal_uInt16 nDestPageNum(pPage->GetPageNum() + 1);
        SdPage* pNotesPage = nullptr;

        if(nDestPageNum < GetPageCount())
        {
            pNotesPage = static_cast<SdPage*>(GetPage(nDestPageNum));
        }

        if (pNotesPage && pNotesPage->GetPageKind() == PK_NOTES)
        {
            pNotesPage->SetSelected(bSelect);
        }
    }
    else if (ePageKind == PK_NOTES)
    {
        pPage->SetSelected(bSelect);
        SdPage* pStandardPage = static_cast<SdPage*>( GetPage( pPage->GetPageNum() - 1 ) );

        if (pStandardPage && pStandardPage->GetPageKind() == PK_STANDARD)
            pStandardPage->SetSelected(bSelect);
    }
}

// If no pages exist yet, create them now
void SdDrawDocument::CreateFirstPages( SdDrawDocument* pRefDocument /* = 0 */ )
{
    // If no page exists yet in the model, (File -> New), insert a page
    sal_uInt16 nPageCount = GetPageCount();

    if (nPageCount <= 1)
    {
        // #i57181# Paper size depends on Language, like in Writer
        Size aDefSize = SvxPaperInfo::GetDefaultPaperSize( MAP_100TH_MM );

        // Insert handout page
        SdPage* pHandoutPage = AllocSdPage(false);

        SdPage* pRefPage = nullptr;

        if( pRefDocument )
            pRefPage = pRefDocument->GetSdPage( 0, PK_HANDOUT );

        if( pRefPage )
        {
            pHandoutPage->SetSize(pRefPage->GetSize());
            pHandoutPage->SetBorder( pRefPage->GetLftBorder(), pRefPage->GetUppBorder(), pRefPage->GetRgtBorder(), pRefPage->GetLwrBorder() );
        }
        else
        {
            pHandoutPage->SetSize(aDefSize);
            pHandoutPage->SetBorder(0, 0, 0, 0);
        }

        pHandoutPage->SetPageKind(PK_HANDOUT);
        pHandoutPage->SetName( SD_RESSTR(STR_HANDOUT) );
        InsertPage(pHandoutPage, 0);

        // Insert master page and register this with the handout page
        SdPage* pHandoutMPage = AllocSdPage(true);
        pHandoutMPage->SetSize( pHandoutPage->GetSize() );
        pHandoutMPage->SetPageKind(PK_HANDOUT);
        pHandoutMPage->SetBorder( pHandoutPage->GetLftBorder(),
                                  pHandoutPage->GetUppBorder(),
                                  pHandoutPage->GetRgtBorder(),
                                  pHandoutPage->GetLwrBorder() );
        InsertMasterPage(pHandoutMPage, 0);
        pHandoutPage->TRG_SetMasterPage( *pHandoutMPage );

        // Insert page
        // If nPageCount==1 is, the model for the clipboard was created, thus a
        // default page must already exist
        SdPage* pPage;
        bool bClipboard = false;

        if( pRefDocument )
            pRefPage = pRefDocument->GetSdPage( 0, PK_STANDARD );

        if (nPageCount == 0)
        {
            pPage = AllocSdPage(false);

            if( pRefPage )
            {
                pPage->SetSize( pRefPage->GetSize() );
                pPage->SetBorder( pRefPage->GetLftBorder(), pRefPage->GetUppBorder(), pRefPage->GetRgtBorder(), pRefPage->GetLwrBorder() );
            }
            else if (meDocType == DOCUMENT_TYPE_DRAW)
            {
                // Draw: always use default size with margins
                pPage->SetSize(aDefSize);

                SfxPrinter* pPrinter = mpDocSh->GetPrinter(false);
                if (pPrinter && pPrinter->IsValid())
                {
                    Size aOutSize(pPrinter->GetOutputSize());
                    Point aPageOffset(pPrinter->GetPageOffset());
                    aPageOffset -= pPrinter->PixelToLogic( Point() );
                    long nOffset = !aPageOffset.X() && !aPageOffset.Y() ? 0 : PRINT_OFFSET;

                    sal_uLong nTop    = aPageOffset.Y();
                    sal_uLong nLeft   = aPageOffset.X();
                    sal_uLong nBottom = std::max((long)(aDefSize.Height() - aOutSize.Height() - nTop + nOffset), 0L);
                    sal_uLong nRight  = std::max((long)(aDefSize.Width() - aOutSize.Width() - nLeft + nOffset), 0L);

                    pPage->SetBorder(nLeft, nTop, nRight, nBottom);
                }
                else
                {
                    // The printer is not available.  Use a border of 10mm
                    // on each side instead.
                    // This has to be kept synchronized with the border
                    // width set in the
                    // SvxPageDescPage::PaperSizeSelect_Impl callback.
                    pPage->SetBorder(1000, 1000, 1000, 1000);
                }
            }
            else
            {
                // Impress: always use screen format, landscape.
                Size aSz( SvxPaperInfo::GetPaperSize(PAPER_SCREEN_4_3, MAP_100TH_MM) );
                pPage->SetSize( Size( aSz.Height(), aSz.Width() ) );
                pPage->SetBorder(0, 0, 0, 0);
            }

            InsertPage(pPage, 1);
        }
        else
        {
            bClipboard = true;
            pPage = static_cast<SdPage*>( GetPage(1) );
        }

        // Insert master page, then register this with the page
        SdPage* pMPage = AllocSdPage(true);
        pMPage->SetSize( pPage->GetSize() );
        pMPage->SetBorder( pPage->GetLftBorder(),
                           pPage->GetUppBorder(),
                           pPage->GetRgtBorder(),
                           pPage->GetLwrBorder() );
        InsertMasterPage(pMPage, 1);
        pPage->TRG_SetMasterPage( *pMPage );
        if( bClipboard )
            pMPage->SetLayoutName( pPage->GetLayoutName() );

        // Insert notes page
        SdPage* pNotesPage = AllocSdPage(false);

        if( pRefDocument )
            pRefPage = pRefDocument->GetSdPage( 0, PK_NOTES );

        if( pRefPage )
        {
            pNotesPage->SetSize( pRefPage->GetSize() );
            pNotesPage->SetBorder( pRefPage->GetLftBorder(), pRefPage->GetUppBorder(), pRefPage->GetRgtBorder(), pRefPage->GetLwrBorder() );
        }
        else
        {
            // Always use portrait format
            if (aDefSize.Height() >= aDefSize.Width())
            {
                pNotesPage->SetSize(aDefSize);
            }
            else
            {
                pNotesPage->SetSize( Size(aDefSize.Height(), aDefSize.Width()) );
            }

            pNotesPage->SetBorder(0, 0, 0, 0);
        }
        pNotesPage->SetPageKind(PK_NOTES);
        InsertPage(pNotesPage, 2);
        if( bClipboard )
            pNotesPage->SetLayoutName( pPage->GetLayoutName() );

        // Insert master page, then register this with the notes page
        SdPage* pNotesMPage = AllocSdPage(true);
        pNotesMPage->SetSize( pNotesPage->GetSize() );
        pNotesMPage->SetPageKind(PK_NOTES);
        pNotesMPage->SetBorder( pNotesPage->GetLftBorder(),
                                pNotesPage->GetUppBorder(),
                                pNotesPage->GetRgtBorder(),
                                pNotesPage->GetLwrBorder() );
        InsertMasterPage(pNotesMPage, 2);
        pNotesPage->TRG_SetMasterPage( *pNotesMPage );
        if( bClipboard )
            pNotesMPage->SetLayoutName( pPage->GetLayoutName() );

        if( !pRefPage && (meDocType != DOCUMENT_TYPE_DRAW) )
            pPage->SetAutoLayout( AUTOLAYOUT_TITLE, true, true );

        mpWorkStartupTimer = new Timer("DrawWorkStartupTimer");
        mpWorkStartupTimer->SetTimeoutHdl( LINK(this, SdDrawDocument, WorkStartupHdl) );
        mpWorkStartupTimer->SetTimeout(2000);
        mpWorkStartupTimer->Start();

        SetChanged(false);
    }
}

// Creates missing notes and handout pages (after PowerPoint import).
// We assume that at least one default page and one default master page exist.

bool SdDrawDocument::CreateMissingNotesAndHandoutPages()
{
    bool bOK = false;
    sal_uInt16 nPageCount = GetPageCount();

    if (nPageCount != 0)
    {
        // Set PageKind
        SdPage* pHandoutMPage = static_cast<SdPage*>( GetMasterPage(0) );
        pHandoutMPage->SetPageKind(PK_HANDOUT);

        SdPage* pHandoutPage = static_cast<SdPage*>( GetPage(0) );
        pHandoutPage->SetPageKind(PK_HANDOUT);
        pHandoutPage->TRG_SetMasterPage( *pHandoutMPage );

        for (sal_uInt16 i = 1; i < nPageCount; i = i + 2)
        {
            SdPage* pPage = static_cast<SdPage*>( GetPage(i) );

            if(!pPage->TRG_HasMasterPage())
            {
                // No master page set -> use first default master page
                // (If there was no default page in the PPT)
                pPage->TRG_SetMasterPage(*GetMasterPage(1));
            }

            SdPage* pNotesPage = static_cast<SdPage*>( GetPage(i+1) );
            pNotesPage->SetPageKind(PK_NOTES);

            // Set notes master page
            sal_uInt16 nMasterPageAfterPagesMasterPage = (pPage->TRG_GetMasterPage()).GetPageNum() + 1;
            pNotesPage->TRG_SetMasterPage(*GetMasterPage(nMasterPageAfterPagesMasterPage));
        }

        bOK = true;
        StopWorkStartupDelay();
        SetChanged(false);
    }

    return bOK;
}

// + Move selected pages after said page
//   (nTargetPage = (sal_uInt16)-1  --> move before first page)
// + Returns sal_True when the page has been moved
bool SdDrawDocument::MovePages(sal_uInt16 nTargetPage)
{
    SdPage* pPage              = nullptr;
    sal_uInt16  nPage;
    sal_uInt16  nNoOfPages         = GetSdPageCount(PK_STANDARD);
    bool    bSomethingHappened = false;

    const bool bUndo = IsUndoEnabled();

    if( bUndo )
        BegUndo(SD_RESSTR(STR_UNDO_MOVEPAGES));

    // List of selected pages
    std::vector<SdPage*> aPageList;
    for (nPage = 0; nPage < nNoOfPages; nPage++)
    {
        pPage = GetSdPage(nPage, PK_STANDARD);

        if (pPage->IsSelected()) {
            aPageList.push_back(pPage);
        }
    }

    // If necessary, look backwards, until we find a page that wasn't selected
    nPage = nTargetPage;

    if (nPage != (sal_uInt16)-1)
    {
        pPage = GetSdPage(nPage, PK_STANDARD);
        while (nPage > 0 && pPage->IsSelected())
        {
            nPage--;
            pPage = GetSdPage(nPage, PK_STANDARD);
        }

        if (pPage->IsSelected())
        {
            nPage = (sal_uInt16)-1;
        }
    }

    // Insert before the first page
    if (nPage == (sal_uInt16)-1)
    {
        std::vector<SdPage*>::reverse_iterator iter;
        for (iter = aPageList.rbegin(); iter != aPageList.rend(); ++iter)
        {
            nPage = (*iter)->GetPageNum();
            if (nPage != 0)
            {
                SdrPage* pPg = GetPage(nPage);
                if( bUndo )
                    AddUndo(GetSdrUndoFactory().CreateUndoSetPageNum(*pPg, nPage, 1));
                MovePage(nPage, 1);
                pPg = GetPage(nPage+1);
                if( bUndo )
                    AddUndo(GetSdrUndoFactory().CreateUndoSetPageNum(*pPg, nPage+1, 2));
                MovePage(nPage+1, 2);
                bSomethingHappened = true;
            }
        }
    }
    // Insert after <nPage>
    else
    {
        nTargetPage = nPage;
        nTargetPage = 2 * nTargetPage + 1;    // PK_STANDARD --> absolute

        std::vector<SdPage*>::iterator iter;
        for (iter = aPageList.begin(); iter != aPageList.end(); ++iter)
        {
            pPage = *iter;
            nPage = pPage->GetPageNum();
            if (nPage > nTargetPage)
            {
                nTargetPage += 2;        // Insert _after_ the page

                if (nPage != nTargetPage)
                {
                    SdrPage* pPg = GetPage(nPage);
                    if( bUndo )
                        AddUndo(GetSdrUndoFactory().CreateUndoSetPageNum(*pPg, nPage, nTargetPage));
                    MovePage(nPage, nTargetPage);
                    pPg = GetPage(nPage+1);
                    if( bUndo )
                        AddUndo(GetSdrUndoFactory().CreateUndoSetPageNum(*pPg, nPage+1, nTargetPage+1));
                    MovePage(nPage+1, nTargetPage+1);
                    bSomethingHappened = true;
                }
            }
            else
            {
                if (nPage != nTargetPage)
                {
                    SdrPage* pPg = GetPage(nPage+1);
                    if( bUndo )
                        AddUndo(GetSdrUndoFactory().CreateUndoSetPageNum(*pPg, nPage+1, nTargetPage+1));
                    MovePage(nPage+1, nTargetPage+1);
                    pPg = GetPage(nPage);
                    if( bUndo )
                        AddUndo(GetSdrUndoFactory().CreateUndoSetPageNum(*pPg, nPage, nTargetPage));
                    MovePage(nPage, nTargetPage);
                    bSomethingHappened = true;
                }
            }
            nTargetPage = pPage->GetPageNum();
        }
    }

    if( bUndo )
        EndUndo();

    return bSomethingHappened;
}

// Return number of links in sfx2::LinkManager
sal_uLong SdDrawDocument::GetLinkCount()
{
    return pLinkManager->GetLinks().size();
}

// Set Language
void SdDrawDocument::SetLanguage( const LanguageType eLang, const sal_uInt16 nId )
{
    bool bChanged = false;

    if( nId == EE_CHAR_LANGUAGE && meLanguage != eLang )
    {
        meLanguage = eLang;
        bChanged = true;
    }
    else if( nId == EE_CHAR_LANGUAGE_CJK && meLanguageCJK != eLang )
    {
        meLanguageCJK = eLang;
        bChanged = true;
    }
    else if( nId == EE_CHAR_LANGUAGE_CTL && meLanguageCTL != eLang )
    {
        meLanguageCTL = eLang;
        bChanged = true;
    }

    if( bChanged )
    {
        GetDrawOutliner().SetDefaultLanguage( Application::GetSettings().GetLanguageTag().getLanguageType() );
        pHitTestOutliner->SetDefaultLanguage( Application::GetSettings().GetLanguageTag().getLanguageType() );
        pItemPool->SetPoolDefaultItem( SvxLanguageItem( eLang, nId ) );
        SetChanged( bChanged );
    }
}

// Return language
LanguageType SdDrawDocument::GetLanguage( const sal_uInt16 nId ) const
{
    LanguageType eLangType = meLanguage;

    if( nId == EE_CHAR_LANGUAGE_CJK )
        eLangType = meLanguageCJK;
    else if( nId == EE_CHAR_LANGUAGE_CTL )
        eLangType = meLanguageCTL;

    return eLangType;
}

// Initiate WorkStartup
IMPL_LINK_NOARG_TYPED(SdDrawDocument, WorkStartupHdl, Timer *, void)
{
    if (IsTransportContainer())
        return;

    if( mpDocSh )
        mpDocSh->SetWaitCursor( true );

    bool bChanged = IsChanged();        // remember this

    // Initialize Autolayouts
    SdPage* pHandoutMPage = GetMasterSdPage(0, PK_HANDOUT);

    if (pHandoutMPage->GetAutoLayout() == AUTOLAYOUT_NONE)
    {
        // No AutoLayout yet -> initialize
        pHandoutMPage->SetAutoLayout(AUTOLAYOUT_HANDOUT6, true, true);
    }

    SdPage* pPage = GetSdPage(0, PK_STANDARD);

    if (pPage->GetAutoLayout() == AUTOLAYOUT_NONE)
    {
        // No AutoLayout yet -> initialize
        pPage->SetAutoLayout(AUTOLAYOUT_NONE, true, true);
    }

    SdPage* pNotesPage = GetSdPage(0, PK_NOTES);

    if (pNotesPage->GetAutoLayout() == AUTOLAYOUT_NONE)
    {
        // No AutoLayout yet -> initialize
        pNotesPage->SetAutoLayout(AUTOLAYOUT_NOTES, true, true);
    }

    SetChanged(bChanged);

    if( mpDocSh )
        mpDocSh->SetWaitCursor( false );
}

// When the WorkStartupTimer has been created (this only happens in
// SdDrawViewShell::Construct() ), the timer may be stopped and the WorkStartup
// may be initiated.
void SdDrawDocument::StopWorkStartupDelay()
{
    if (mpWorkStartupTimer)
    {
        if ( mpWorkStartupTimer->IsActive() )
        {
            // Timer not yet expired -> initiate WorkStartup
            mpWorkStartupTimer->Stop();
            WorkStartupHdl(nullptr);
        }

        delete mpWorkStartupTimer;
        mpWorkStartupTimer = nullptr;
    }
}

// When the WorkStartupTimer has been created (this only happens in
// SdDrawViewShell::Construct() ), the timer may be stopped and the WorkStartup
// may be initiated.
SdAnimationInfo* SdDrawDocument::GetAnimationInfo(SdrObject* pObject) const
{
    DBG_ASSERT(pObject, "sd::SdDrawDocument::GetAnimationInfo(), invalid argument!");
    if( pObject )
        return GetShapeUserData( *pObject );
    else
        return nullptr;
}

SdAnimationInfo* SdDrawDocument::GetShapeUserData(SdrObject& rObject, bool bCreate /* = false */ )
{
    sal_uInt16 nUD          = 0;
    sal_uInt16 nUDCount     = rObject.GetUserDataCount();
    SdAnimationInfo* pRet = nullptr;

    // Can we find animation information within the user data?
    for (nUD = 0; nUD < nUDCount; nUD++)
    {
        SdrObjUserData* pUD = rObject.GetUserData(nUD);
        if((pUD->GetInventor() == SdUDInventor) && (pUD->GetId() == SD_ANIMATIONINFO_ID))
        {
            pRet = dynamic_cast<SdAnimationInfo*>(pUD);
            break;
        }
    }

    if( (pRet == nullptr) && bCreate )
    {
        pRet = new SdAnimationInfo( rObject );
        rObject.AppendUserData( pRet);
    }

    return pRet;
}

SdIMapInfo* SdDrawDocument::GetIMapInfo( SdrObject* pObject ) const
{
    DBG_ASSERT(pObject, "Without an object there is no IMapInfo");

    SdIMapInfo*     pIMapInfo = nullptr;
    sal_uInt16          nCount = pObject->GetUserDataCount();

    // Can we find IMap information within the user data?
    for ( sal_uInt16 i = 0; i < nCount; i++ )
    {
        SdrObjUserData* pUserData = pObject->GetUserData( i );

        if ( ( pUserData->GetInventor() == SdUDInventor ) && ( pUserData->GetId() == SD_IMAPINFO_ID ) )
            pIMapInfo = static_cast<SdIMapInfo*>(pUserData);
    }

    return pIMapInfo;
}

IMapObject* SdDrawDocument::GetHitIMapObject( SdrObject* pObj,
                                              const Point& rWinPoint,
                                              const vcl::Window& /* rCmpWnd */ )
{
    SdIMapInfo* pIMapInfo = GetIMapInfo( pObj );
    IMapObject* pIMapObj = nullptr;

    if ( pIMapInfo )
    {
        const MapMode       aMap100( MAP_100TH_MM );
        Size                aGraphSize;
        Point               aRelPoint( rWinPoint );
        ImageMap&           rImageMap = (ImageMap&) pIMapInfo->GetImageMap();
        const Rectangle&    rRect = pObj->GetLogicRect();
        bool                bObjSupported = false;

        // execute HitTest
        if ( dynamic_cast< const SdrGrafObj *>( pObj ) !=  nullptr  ) // simple graphics object
        {
            const SdrGrafObj*   pGrafObj = static_cast<const SdrGrafObj*>(pObj);
            const GeoStat&      rGeo = pGrafObj->GetGeoStat();
            SdrGrafObjGeoData*  pGeoData = static_cast<SdrGrafObjGeoData*>( pGrafObj->GetGeoData() );

            // Undo rotation
            if ( rGeo.nRotationAngle )
                RotatePoint( aRelPoint, rRect.TopLeft(), -rGeo.nSin, rGeo.nCos );

            // Undo mirroring
            if ( pGeoData->bMirrored )
                aRelPoint.X() = rRect.Right() + rRect.Left() - aRelPoint.X();

            // Undo shearing
            if ( rGeo.nShearAngle )
                ShearPoint( aRelPoint, rRect.TopLeft(), -rGeo.nTan );

            if ( pGrafObj->GetGrafPrefMapMode().GetMapUnit() == MAP_PIXEL )
                aGraphSize = Application::GetDefaultDevice()->PixelToLogic( pGrafObj->GetGrafPrefSize(), aMap100 );
            else
                aGraphSize = OutputDevice::LogicToLogic( pGrafObj->GetGrafPrefSize(),
                                                         pGrafObj->GetGrafPrefMapMode(), aMap100 );

            delete pGeoData;
            bObjSupported = true;
        }
        else if ( dynamic_cast<const SdrOle2Obj* >(pObj) !=  nullptr ) // OLE object
        {
            aGraphSize = static_cast<SdrOle2Obj*>( pObj )->GetOrigObjSize();
            bObjSupported = true;
        }

        // Everything worked out well, thus execute HitTest
        if ( bObjSupported )
        {
            // Calculate relative position of mouse cursor
            aRelPoint -= rRect.TopLeft();
            pIMapObj = rImageMap.GetHitIMapObject( aGraphSize, rRect.GetSize(), aRelPoint );

            // We don't care about deactivated objects
            if ( pIMapObj && !pIMapObj->IsActive() )
                pIMapObj = nullptr;
        }
    }

    return pIMapObj;
}

ImageMap* SdDrawDocument::GetImageMapForObject(SdrObject* pObj)
{
    SdIMapInfo* pIMapInfo = GetIMapInfo( pObj );
    if ( pIMapInfo )
    {
        return const_cast<ImageMap*>( &(pIMapInfo->GetImageMap()) );
    }
    return nullptr;
}

/** this method enforces that the masterpages are in the correct order,
    that is at position 1 is a PK_STANDARD masterpage followed by a
    PK_NOTES masterpage and so on. #
*/
void SdDrawDocument::CheckMasterPages()
{
    sal_uInt16 nMaxPages = GetMasterPageCount();

    // we need at least a handout master and one master page
    if( nMaxPages < 2 )
    {
        return;
    }

    SdPage* pPage = nullptr;

    sal_uInt16 nPage;

    // first see if the page order is correct
    for( nPage = 1; nPage < nMaxPages; nPage++ )
    {
        pPage = static_cast<SdPage*> (GetMasterPage( nPage ));
        // if an odd page is not a standard page or an even page is not a notes page
        if( ((1 == (nPage & 1)) && (pPage->GetPageKind() != PK_STANDARD) ) ||
            ((0 == (nPage & 1)) && (pPage->GetPageKind() != PK_NOTES) ) )
            break; // then we have a fatal error
    }

    if( nPage < nMaxPages )
    {
        SdPage* pNotesPage = nullptr;

        // there is a fatal error in the master page order,
        // we need to repair the document
        bool bChanged = false;

        nPage = 1;
        while( nPage < nMaxPages )
        {
            pPage = static_cast<SdPage*> (GetMasterPage( nPage ));
            if( pPage->GetPageKind() != PK_STANDARD )
            {
                bChanged = true;
                sal_uInt16 nFound = nPage + 1;
                while( nFound < nMaxPages )
                {
                    pPage = static_cast<SdPage*>(GetMasterPage( nFound ));
                    if( PK_STANDARD == pPage->GetPageKind() )
                    {
                        MoveMasterPage( nFound, nPage );
                        pPage->SetInserted();
                        break;

                    }

                    nFound++;
                }

                // if we don't have any more standard pages, were done
                if( nMaxPages == nFound )
                    break;
            }

            nPage++;

            if( nPage < nMaxPages )
                pNotesPage = static_cast<SdPage*>(GetMasterPage( nPage ));
            else
                pNotesPage = nullptr;

            if( (nullptr == pNotesPage) || (pNotesPage->GetPageKind() != PK_NOTES) || ( pPage->GetLayoutName() != pNotesPage->GetLayoutName() ) )
            {
                bChanged = true;

                sal_uInt16 nFound = nPage + 1;
                while( nFound < nMaxPages )
                {
                    pNotesPage = static_cast<SdPage*>(GetMasterPage( nFound ));
                    if( (PK_NOTES == pNotesPage->GetPageKind()) && ( pPage->GetLayoutName() == pNotesPage->GetLayoutName() ) )
                    {
                        MoveMasterPage( nFound, nPage );
                        pNotesPage->SetInserted();
                        break;
                    }

                    nFound++;
                }

                // looks like we lost a notes page
                if( nMaxPages == nFound )
                {
                    // so create one

                    // first find a reference notes page for size
                    SdPage* pRefNotesPage = nullptr;
                    nFound = 0;
                    while( nFound < nMaxPages )
                    {
                        pRefNotesPage = static_cast<SdPage*>(GetMasterPage( nFound ));
                        if( PK_NOTES == pRefNotesPage->GetPageKind() )
                            break;
                        nFound++;
                    }
                    if( nFound == nMaxPages )
                        pRefNotesPage = nullptr;

                    SdPage* pNewNotesPage = AllocSdPage(true);
                    pNewNotesPage->SetPageKind(PK_NOTES);
                    if( pRefNotesPage )
                    {
                        pNewNotesPage->SetSize( pRefNotesPage->GetSize() );
                        pNewNotesPage->SetBorder( pRefNotesPage->GetLftBorder(),
                                                pRefNotesPage->GetUppBorder(),
                                                pRefNotesPage->GetRgtBorder(),
                                                pRefNotesPage->GetLwrBorder() );
                    }
                    InsertMasterPage(pNewNotesPage,  nPage );
                    pNewNotesPage->SetLayoutName( pPage->GetLayoutName() );
                    pNewNotesPage->SetAutoLayout(AUTOLAYOUT_NOTES, true, true );
                    nMaxPages++;
                }
            }

            nPage++;
        }

        // now remove all remaining and unused non PK_STANDARD slides
        while( nPage < nMaxPages )
        {
            bChanged = true;

            RemoveMasterPage( nPage );
            nMaxPages--;
        }

        if( bChanged )
        {
            OSL_FAIL( "master pages where in a wrong order" );
            RecalcPageNums( true);
        }
    }
}

sal_uInt16 SdDrawDocument::CreatePage (
    SdPage* pActualPage,
    PageKind ePageKind,
    const OUString& sStandardPageName,
    const OUString& sNotesPageName,
    AutoLayout eStandardLayout,
    AutoLayout eNotesLayout,
    bool bIsPageBack,
    bool bIsPageObj,
    const sal_Int32 nInsertPosition)
{
    SdPage* pPreviousStandardPage;
    SdPage* pPreviousNotesPage;
    SdPage* pStandardPage;
    SdPage* pNotesPage;

    // From the given page determine the standard page and notes page of which
    // to take the layout and the position where to insert the new pages.
    if (ePageKind == PK_NOTES)
    {
        pPreviousNotesPage = pActualPage;
        sal_uInt16 nNotesPageNum = pPreviousNotesPage->GetPageNum() + 2;
        pPreviousStandardPage = static_cast<SdPage*>( GetPage(nNotesPageNum - 3) );
        eStandardLayout = pPreviousStandardPage->GetAutoLayout();
    }
    else
    {
        pPreviousStandardPage = pActualPage;
        sal_uInt16 nStandardPageNum = pPreviousStandardPage->GetPageNum() + 2;
        pPreviousNotesPage = static_cast<SdPage*>( GetPage(nStandardPageNum - 1) );
        eNotesLayout = pPreviousNotesPage->GetAutoLayout();
    }

    // Create new standard page and set it up
    pStandardPage = AllocSdPage(false);

    // Set the size here since else the presobj autolayout
    // will be wrong.
    pStandardPage->SetSize( pPreviousStandardPage->GetSize() );
    pStandardPage->SetBorder( pPreviousStandardPage->GetLftBorder(),
                              pPreviousStandardPage->GetUppBorder(),
                              pPreviousStandardPage->GetRgtBorder(),
                              pPreviousStandardPage->GetLwrBorder() );

    // Use master page of current page.
    pStandardPage->TRG_SetMasterPage(pPreviousStandardPage->TRG_GetMasterPage());

    // User layout of current standard page
    pStandardPage->SetLayoutName( pPreviousStandardPage->GetLayoutName() );
    pStandardPage->SetAutoLayout(eStandardLayout, true);
    pStandardPage->setHeaderFooterSettings( pPreviousStandardPage->getHeaderFooterSettings() );

    // transition settings of current page
    pStandardPage->setTransitionType( pPreviousStandardPage->getTransitionType() );
    pStandardPage->setTransitionSubtype( pPreviousStandardPage->getTransitionSubtype() );
    pStandardPage->setTransitionDirection( pPreviousStandardPage->getTransitionDirection() );
    pStandardPage->setTransitionFadeColor( pPreviousStandardPage->getTransitionFadeColor() );
    pStandardPage->setTransitionDuration( pPreviousStandardPage->getTransitionDuration() );

    // apply previous animation timing
    pStandardPage->SetPresChange( pPreviousStandardPage->GetPresChange() );
    pStandardPage->SetTime( pPreviousStandardPage->GetTime() );

    // Create new notes page and set it up
    pNotesPage = AllocSdPage(false);
    pNotesPage->SetPageKind(PK_NOTES);

    // Use master page of current page
    pNotesPage->TRG_SetMasterPage(pPreviousNotesPage->TRG_GetMasterPage());

    // Use layout of current notes page
    pNotesPage->SetLayoutName( pPreviousNotesPage->GetLayoutName() );
    pNotesPage->SetAutoLayout(eNotesLayout, true);
    pNotesPage->setHeaderFooterSettings( pPreviousNotesPage->getHeaderFooterSettings() );

    return InsertPageSet (
        pActualPage,
        ePageKind,
        sStandardPageName,
        sNotesPageName,
        bIsPageBack,
        bIsPageObj,
        pStandardPage,
        pNotesPage,
        nInsertPosition);
}

sal_uInt16 SdDrawDocument::DuplicatePage (sal_uInt16 nPageNum)
{
    PageKind ePageKind = PK_STANDARD;

    // Get current page
    SdPage* pActualPage = GetSdPage(nPageNum, ePageKind);

    // Get background flags
    SdrLayerAdmin& rLayerAdmin = GetLayerAdmin();
    sal_uInt8 aBckgrnd = rLayerAdmin.GetLayerID(SD_RESSTR(STR_LAYER_BCKGRND), false);
    sal_uInt8 aBckgrndObj = rLayerAdmin.GetLayerID(SD_RESSTR(STR_LAYER_BCKGRNDOBJ), false);
    SetOfByte aVisibleLayers = pActualPage->TRG_GetMasterPageVisibleLayers();

    return DuplicatePage (
        pActualPage, ePageKind,
        // No names for the new slides
        OUString(), OUString(),
        aVisibleLayers.IsSet(aBckgrnd),
        aVisibleLayers.IsSet(aBckgrndObj));
}

sal_uInt16 SdDrawDocument::DuplicatePage (
    SdPage* pActualPage,
    PageKind ePageKind,
    const OUString& sStandardPageName,
    const OUString& sNotesPageName,
    bool bIsPageBack,
    bool bIsPageObj,
    const sal_Int32 nInsertPosition)
{
    SdPage* pPreviousStandardPage;
    SdPage* pPreviousNotesPage;
    SdPage* pStandardPage;
    SdPage* pNotesPage;

    // From the given page determine the standard page and the notes page
    // of which to make copies.
    if (ePageKind == PK_NOTES)
    {
        pPreviousNotesPage = pActualPage;
        sal_uInt16 nNotesPageNum = pPreviousNotesPage->GetPageNum() + 2;
        pPreviousStandardPage = static_cast<SdPage*>( GetPage(nNotesPageNum - 3) );
    }
    else
    {
        pPreviousStandardPage = pActualPage;
        sal_uInt16 nStandardPageNum = pPreviousStandardPage->GetPageNum() + 2;
        pPreviousNotesPage = static_cast<SdPage*>( GetPage(nStandardPageNum - 1) );
    }

    // Create duplicates of a standard page and the associated notes page
    pStandardPage = static_cast<SdPage*>( pPreviousStandardPage->Clone() );
    pNotesPage = static_cast<SdPage*>( pPreviousNotesPage->Clone() );

    return InsertPageSet (
        pActualPage,
        ePageKind,
        sStandardPageName,
        sNotesPageName,
        bIsPageBack,
        bIsPageObj,
        pStandardPage,
        pNotesPage,
        nInsertPosition);
}

sal_uInt16 SdDrawDocument::InsertPageSet (
    SdPage* pActualPage,
    PageKind ePageKind,
    const OUString& sStandardPageName,
    const OUString& sNotesPageName,
    bool bIsPageBack,
    bool bIsPageObj,
    SdPage* pStandardPage,
    SdPage* pNotesPage,
    sal_Int32 nInsertPosition)
{
    SdPage* pPreviousStandardPage;
    SdPage* pPreviousNotesPage;
    sal_uInt16 nStandardPageNum;
    sal_uInt16 nNotesPageNum;
    OUString aNotesPageName(sNotesPageName);

    // Gather some information about the standard page and the notes page
    // that are to be inserted. This makes sure that there is always one
    // standard page followed by one notes page.
    if (ePageKind == PK_NOTES)
    {
        pPreviousNotesPage = pActualPage;
        nNotesPageNum = pPreviousNotesPage->GetPageNum() + 2;
        pPreviousStandardPage = static_cast<SdPage*>( GetPage(nNotesPageNum - 3) );
        nStandardPageNum = nNotesPageNum - 1;
    }
    else
    {
        pPreviousStandardPage = pActualPage;
        nStandardPageNum = pPreviousStandardPage->GetPageNum() + 2;
        pPreviousNotesPage = static_cast<SdPage*>( GetPage(nStandardPageNum - 1) );
        nNotesPageNum = nStandardPageNum + 1;
        aNotesPageName = sStandardPageName;
    }

    OSL_ASSERT(nNotesPageNum==nStandardPageNum+1);
    if (nInsertPosition < 0)
        nInsertPosition = nStandardPageNum;

    // Set up and insert the standard page
    SetupNewPage (
        pPreviousStandardPage,
        pStandardPage,
        sStandardPageName,
        nInsertPosition,
        bIsPageBack,
        bIsPageObj);

    // Set up and insert the notes page
    pNotesPage->SetPageKind(PK_NOTES);
    SetupNewPage (
        pPreviousNotesPage,
        pNotesPage,
        aNotesPageName,
        nInsertPosition+1,
        bIsPageBack,
        bIsPageObj);

    // Return an index that allows the caller to access the newly inserted
    // pages by using GetSdPage()
    return pStandardPage->GetPageNum() / 2;
}

void SdDrawDocument::SetupNewPage (
    SdPage* pPreviousPage,
    SdPage* pPage,
    const OUString& sPageName,
    sal_uInt16 nInsertionPoint,
    bool bIsPageBack,
    bool bIsPageObj)
{
    if (pPreviousPage != nullptr)
    {
        pPage->SetSize( pPreviousPage->GetSize() );
        pPage->SetBorder( pPreviousPage->GetLftBorder(),
            pPreviousPage->GetUppBorder(),
            pPreviousPage->GetRgtBorder(),
            pPreviousPage->GetLwrBorder() );
    }
    pPage->SetName(sPageName);

    InsertPage(pPage, nInsertionPoint);

    if (pPreviousPage != nullptr)
    {
        SdrLayerAdmin& rLayerAdmin = GetLayerAdmin();
        sal_uInt8 aBckgrnd = rLayerAdmin.GetLayerID(SD_RESSTR(STR_LAYER_BCKGRND), false);
        sal_uInt8 aBckgrndObj = rLayerAdmin.GetLayerID(SD_RESSTR(STR_LAYER_BCKGRNDOBJ), false);
        SetOfByte aVisibleLayers = pPreviousPage->TRG_GetMasterPageVisibleLayers();
        aVisibleLayers.Set(aBckgrnd, bIsPageBack);
        aVisibleLayers.Set(aBckgrndObj, bIsPageObj);
        pPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);
    }
}

sd::UndoManager* SdDrawDocument::GetUndoManager() const
{
    return mpDocSh ? dynamic_cast< sd::UndoManager* >(mpDocSh->GetUndoManager()) : nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
