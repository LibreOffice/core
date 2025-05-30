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

#include <vcl/settings.hxx>

#include <sal/log.hxx>
#include <tools/debug.hxx>
#include <sfx2/printer.hxx>
#include <editeng/paperinf.hxx>
#include <svx/svdopage.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdundo.hxx>
#include <vcl/svapp.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/fieldupdater.hxx>
#include <editeng/langitem.hxx>
#include <svl/itempool.hxx>
#include <editeng/flditem.hxx>

#include <sfx2/linkmgr.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdlayer.hxx>

#include <svx/svditer.hxx>
#include <comphelper/lok.hxx>
#include <xmloff/autolayout.hxx>

#include <sdresid.hxx>
#include <drawdoc.hxx>
#include <sdpage.hxx>
#include <strings.hrc>
#include <glob.hxx>
#include <stlpool.hxx>
#include <anminfo.hxx>
#include <undo/undomanager.hxx>
#include <sfx2/lokhelper.hxx>
#include <unomodel.hxx>

#include <DrawDocShell.hxx>

#include "PageListWatcher.hxx"
#include <unokywds.hxx>

using namespace ::sd;

const ::tools::Long PRINT_OFFSET = 30;       // see /svx/source/dialog/page.cxx

using namespace com::sun::star;

// Looks up an object by name
SdrObject* SdDrawDocument::GetObj(std::u16string_view rObjName) const
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
        SdrObjListIter aIter(pPage, SdrIterMode::DeepWithGroups);

        while (aIter.IsMore() && !pObjFound)
        {
            pObj = aIter.Next();

            if( ( pObj->GetName() == rObjName ) ||
                ( SdrInventor::Default == pObj->GetObjInventor() &&
                  SdrObjKind::OLE2 == pObj->GetObjIdentifier() &&
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
        SdrObjListIter aIter(pPage, SdrIterMode::DeepWithGroups);

        while (aIter.IsMore() && !pObjFound)
        {
            pObj = aIter.Next();

            if( ( pObj->GetName() == rObjName ) ||
                ( SdrInventor::Default == pObj->GetObjInventor() &&
                  SdrObjKind::OLE2 == pObj->GetObjIdentifier() &&
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
sal_uInt16 SdDrawDocument::GetPageByName(std::u16string_view rPgName, bool& rbIsMasterPage) const
{
    const SdPage* pPage = nullptr;
    sal_uInt16 nPage = 0;
    const sal_uInt16 nMaxPages = GetPageCount();
    sal_uInt16 nPageNum = SDRPAGE_NOTFOUND;

    rbIsMasterPage = false;

    // Search all regular pages and all notes pages (handout pages are
    // ignored)
    while (nPage < nMaxPages && nPageNum == SDRPAGE_NOTFOUND)
    {
        pPage = static_cast<const SdPage*>(GetPage(nPage));

        if (pPage != nullptr
            && pPage->GetPageKind() != PageKind::Handout
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
        pPage = static_cast<const SdPage*>(GetMasterPage(nPage));

        if (pPage && pPage->GetName() == rPgName)
        {
            nPageNum = nPage;
            rbIsMasterPage = true;
        }

        nPage++;
    }

    return nPageNum;
}

bool SdDrawDocument::IsPageNameUnique( std::u16string_view rPgName ) const
{
    sal_uInt16 nCount = 0;
    const SdPage* pPage = nullptr;

    // Search all regular pages and all notes pages (handout pages are ignored)
    sal_uInt16 nPage = 0;
    sal_uInt16 nMaxPages = GetPageCount();
    while (nPage < nMaxPages)
    {
        pPage = static_cast<const SdPage*>(GetPage(nPage));

        if (pPage && pPage->GetName() == rPgName && pPage->GetPageKind() != PageKind::Handout)
            nCount++;

        nPage++;
    }

    // Search all master pages
    nPage = 0;
    nMaxPages = GetMasterPageCount();
    while (nPage < nMaxPages)
    {
        pPage = static_cast<const SdPage*>(GetMasterPage(nPage));

        if (pPage && pPage->GetName() == rPgName)
            nCount++;

        nPage++;
    }

    return nCount == 1;
}

SdPage* SdDrawDocument::GetSdPage(sal_uInt16 nPgNum, PageKind ePgKind) const
{
    return mpDrawPageListWatcher->GetSdPage(ePgKind, sal_uInt32(nPgNum));
}

sal_uInt16 SdDrawDocument::GetSdPageCount(PageKind ePgKind) const
{
    return static_cast<sal_uInt16>(mpDrawPageListWatcher->GetSdPageCount(ePgKind));
}

SdPage* SdDrawDocument::GetMasterSdPage(sal_uInt16 nPgNum, PageKind ePgKind)
{
    return mpMasterPageListWatcher->GetSdPage(ePgKind, sal_uInt32(nPgNum));
}

sal_uInt16 SdDrawDocument::GetMasterSdPageCount(PageKind ePgKind) const
{
    return static_cast<sal_uInt16>(mpMasterPageListWatcher->GetSdPageCount(ePgKind));
}

sal_uInt16 SdDrawDocument::GetActiveSdPageCount() const
{
    return static_cast<sal_uInt16>(mpDrawPageListWatcher->GetVisibleSdPageCount());
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
        if (pPage && pPage->GetPageKind() == PageKind::Notes)
        {
            for (const rtl::Reference<SdrObject>& pObj : *pPage)
            {
                if (pObj->GetObjIdentifier() == SdrObjKind::Page &&
                    pObj->GetObjInventor() == SdrInventor::Default)
                {
                    // The page object is the preceding page (drawing page)
                    SAL_WARN_IF(!nStartPos, "sd", "Position of notes page must not be 0.");

                    SAL_WARN_IF(nPage <= 1, "sd", "Page object must not be a handout.");

                    if (nStartPos > 0 && nPage > 1)
                        static_cast<SdrPageObj*>(pObj.get())->SetReferencedPage(GetPage(nPage - 1));
                }
            }
        }
    }
}

namespace
{
class SvxFieldItemUpdater_BaseProperties final : public editeng::SvxFieldItemUpdater
{
    sdr::properties::BaseProperties& mrProps;

public:
    SvxFieldItemUpdater_BaseProperties(sdr::properties::BaseProperties& rProps)
        : mrProps(rProps) {}

    virtual void SetItem(const SvxFieldItem& rNew) override
    {
        mrProps.SetObjectItem(rNew);
    }
};
}

static void UpdatePageRelativeURLs(SdrObject& rObj, const std::function<void(const SvxFieldItem & rFieldItem, editeng::SvxFieldItemUpdater& rFieldItemUpdater)>& rItemCallback)
{
    if (SdrObjList* pChildrenObjs = rObj.getChildrenOfSdrObject())
    {
        for (const rtl::Reference<SdrObject>& pSubObj : *pChildrenObjs)
            UpdatePageRelativeURLs(*pSubObj, rItemCallback);
    }

    // cannot call GetObjectItemSet on a group
    if (rObj.GetObjIdentifier() != SdrObjKind::Group)
    {
        sdr::properties::BaseProperties& rProps = rObj.GetProperties();
        const SfxItemSet& rSet = rProps.GetObjectItemSet();
        if (const SvxFieldItem* pFieldItem = rSet.GetItemIfSet(EE_FEATURE_FIELD))
        {
            SvxFieldItemUpdater_BaseProperties aItemUpdater(rProps);
            rItemCallback(*pFieldItem, aItemUpdater);
        }
    }

    SdrTextObj* pTxtObj = DynCastSdrTextObj(&rObj);
    if (!pTxtObj)
        return;
    OutlinerParaObject* pOutlinerParagraphObject = pTxtObj->GetOutlinerParaObject();
    if (!pOutlinerParagraphObject)
        return;
    EditTextObject& aEdit = const_cast<EditTextObject&>(pOutlinerParagraphObject->GetTextObject());
    aEdit.GetFieldUpdater().UpdatePageRelativeURLs(rItemCallback);
};

void SdDrawDocument::UpdatePageRelativeURLsImpl(const std::function<void(const SvxFieldItem & rFieldItem, editeng::SvxFieldItemUpdater& rFieldItemUpdater)>& rItemCallback)
{
    for (sal_uInt16 nPage = 0, nPageCount = GetPageCount(); nPage < nPageCount; ++nPage)
    {
        SdrPage* pPage = GetPage(nPage);
        for(size_t nObj = 0, nObjCount = pPage->GetObjCount(); nObj < nObjCount; ++nObj)
            ::UpdatePageRelativeURLs(*pPage->GetObj(nObj), rItemCallback);
    }
}

void SdDrawDocument::UpdatePageRelativeURLs(std::u16string_view aOldName, std::u16string_view aNewName)
{
    if (aNewName.empty())
        return;

    const OUString sNotes(SdResId(STR_NOTES));
    auto aItemCallback = [&sNotes, &aOldName, &aNewName](const SvxFieldItem & rFieldItem, editeng::SvxFieldItemUpdater& rFieldItemUpdater) -> void
    {
        const SvxFieldData* pFieldData = rFieldItem.GetField();
        if (pFieldData->GetClassId() != SvxURLField::CLASS_ID)
            return;
        const SvxURLField* pURLField(static_cast<const SvxURLField*>(pFieldData));
        OUString aURL(pURLField->GetURL());
        if (aURL.isEmpty() || (aURL[0] != 35) || (aURL.indexOf(aOldName, 1) != 1))
            return;

        bool bURLChange(false);

        if (aURL.getLength() == sal_Int32(aOldName.size() + 1)) // standard page name
        {
            aURL = aURL.replaceAt(1, aURL.getLength() - 1, u"") +
                aNewName;
            bURLChange = true;
        }
        else
        {
            if (aURL.getLength() == sal_Int32(aOldName.size()) + 2 + sNotes.getLength()
                && aURL.indexOf(sNotes, aOldName.size() + 2) == sal_Int32(aOldName.size() + 2))
            {
                aURL = aURL.replaceAt(1, aURL.getLength() - 1, u"") +
                    aNewName + " " + sNotes;
                bURLChange = true;
            }
        }

        if(bURLChange)
        {
            SvxFieldItem aNewFieldItem(rFieldItem);
            const_cast<SvxURLField*>(static_cast<const SvxURLField*>(aNewFieldItem.GetField()))->SetURL(aURL);
            rFieldItemUpdater.SetItem(aNewFieldItem);
        }
    };

    UpdatePageRelativeURLsImpl(aItemCallback);
}

void SdDrawDocument::UpdatePageRelativeURLs(SdPage const * pPage, sal_uInt16 nPos, sal_Int32 nIncrement)
{
    bool bNotes = (pPage->GetPageKind() == PageKind::Notes);

    auto aItemCallback = [this, nPos, bNotes, nIncrement](const SvxFieldItem & rFieldItem, editeng::SvxFieldItemUpdater& rFieldItemUpdater) -> void
    {
        const SvxURLField* pURLField(dynamic_cast<const SvxURLField*>(rFieldItem.GetField()));

        if (nullptr == pURLField)
            return;

        OUString aURL(pURLField->GetURL());

        if (aURL.isEmpty() || (aURL[0] != 35))
            return;

        OUString aHashSlide;
        if (meDocType == DocumentType::Draw)
            aHashSlide = "#" + SdResId(STR_PAGE_NAME);
        else
            aHashSlide = "#" + SdResId(STR_PAGE);

        if (!aURL.startsWith(aHashSlide))
            return;

        OUString aURLCopy = aURL;
        const OUString sNotes(SdResId(STR_NOTES));

        aURLCopy = aURLCopy.replaceAt(0, aHashSlide.getLength(), u"");

        bool bNotesLink = ( aURLCopy.getLength() >= sNotes.getLength() + 3
            && aURLCopy.endsWith(sNotes) );

        if (bNotesLink != bNotes)
            return; // no compatible link and page

        if (bNotes)
            aURLCopy = aURLCopy.replaceAt(aURLCopy.getLength() - sNotes.getLength(), sNotes.getLength(), u"");

        sal_Int32 number = aURLCopy.toInt32();
        sal_uInt16 realPageNumber = (nPos + 1)/ 2;

        if ( number < realPageNumber )
            return;

        // update link page number
        number += nIncrement;
        aURL = aURL.replaceAt(aHashSlide.getLength() + 1, aURL.getLength() - aHashSlide.getLength() - 1, u"") +
            OUString::number(number);
        if (bNotes)
        {
            aURL += " " + sNotes;
        }

        SvxFieldItem aNewFieldItem(rFieldItem);
        const_cast<SvxURLField*>(static_cast<const SvxURLField*>(aNewFieldItem.GetField()))->SetURL(aURL);
        rFieldItemUpdater.SetItem(aNewFieldItem);
    };

    UpdatePageRelativeURLsImpl(aItemCallback);
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

    if (comphelper::LibreOfficeKit::isActive() && static_cast<SdPage*>(pPage)->GetPageKind() == PageKind::Standard)
    {
        SdXImpressDocument* pDoc = getUnoModel();
        SfxLokHelper::notifyDocumentSizeChangedAllViews(pDoc);
    }
}

// Override SfxBaseModel::getUnoModel and return a more concrete type
SdXImpressDocument* SdDrawDocument::getUnoModel()
{
    return comphelper::getFromUnoTunnel<SdXImpressDocument>(FmFormModel::getUnoModel());
}

// Delete page
void SdDrawDocument::DeletePage(sal_uInt16 nPgNum)
{
    FmFormModel::DeletePage(nPgNum);

    UpdatePageObjectsInNotes(nPgNum);
}

// Remove page
rtl::Reference<SdrPage> SdDrawDocument::RemovePage(sal_uInt16 nPgNum)
{
    rtl::Reference<SdrPage> pPage = FmFormModel::RemovePage(nPgNum);

    bool bLast = ((nPgNum+1)/2 == (GetPageCount()+1)/2);

    auto pSdPage = static_cast<SdPage*>(pPage.get());
    pSdPage->DisconnectLink();
    ReplacePageInCustomShows( pSdPage, nullptr );
    UpdatePageObjectsInNotes(nPgNum);

    if (!bLast)
        UpdatePageRelativeURLs(pSdPage, nPgNum, -1);

    if (comphelper::LibreOfficeKit::isActive() && pSdPage->GetPageKind() == PageKind::Standard)
    {
        SdXImpressDocument* pDoc = getUnoModel();
        SfxLokHelper::notifyDocumentSizeChangedAllViews(pDoc);
    }

    return pPage;
}

// Warning: This is not called for new master pages created from SdrModel::Merge,
// you also have to modify code in SdDrawDocument::Merge!
void SdDrawDocument::InsertMasterPage(SdrPage* pPage, sal_uInt16 nPos )
{
    FmFormModel::InsertMasterPage( pPage, nPos );
    if( pPage->IsMasterPage() && (static_cast<SdPage*>(pPage)->GetPageKind() == PageKind::Standard) )
    {
        // new master page created, add its style family
        SdStyleSheetPool* pStylePool = static_cast<SdStyleSheetPool*>( GetStyleSheetPool() );
        if( pStylePool )
            pStylePool->AddStyleFamily( static_cast<SdPage*>(pPage) );
    }
}

rtl::Reference<SdrPage> SdDrawDocument::RemoveMasterPage(sal_uInt16 nPgNum)
{
    SdPage* pPage = static_cast<SdPage*>(GetMasterPage(nPgNum ));
    if( pPage && pPage->IsMasterPage() && (pPage->GetPageKind() == PageKind::Standard) )
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

    if (ePageKind == PageKind::Standard)
    {
        pPage->SetSelected(bSelect);

        const sal_uInt16 nDestPageNum(pPage->GetPageNum() + 1);
        SdPage* pNotesPage = nullptr;

        if(nDestPageNum < GetPageCount())
        {
            pNotesPage = static_cast<SdPage*>(GetPage(nDestPageNum));
        }

        if (pNotesPage && pNotesPage->GetPageKind() == PageKind::Notes)
        {
            pNotesPage->SetSelected(bSelect);
        }
    }
    else if (ePageKind == PageKind::Notes)
    {
        pPage->SetSelected(bSelect);
        SdPage* pStandardPage = static_cast<SdPage*>( GetPage( pPage->GetPageNum() - 1 ) );

        if (pStandardPage && pStandardPage->GetPageKind() == PageKind::Standard)
            pStandardPage->SetSelected(bSelect);
    }
}

// If no pages exist yet, create them now
void SdDrawDocument::CreateFirstPages( SdDrawDocument const * pRefDocument /* = 0 */ )
{
    // If no page exists yet in the model, (File -> New), insert a page
    sal_uInt16 nPageCount = GetPageCount();

    if (nPageCount > 1)
        return;

    // #i57181# Paper size depends on Language, like in Writer
    Size aDefSize = SvxPaperInfo::GetDefaultPaperSize( MapUnit::Map100thMM );

    // Insert handout page
    rtl::Reference<SdPage> pHandoutPage = AllocSdPage(false);

    SdPage* pRefPage = nullptr;

    if( pRefDocument )
        pRefPage = pRefDocument->GetSdPage( 0, PageKind::Handout );

    if( pRefPage )
    {
        pHandoutPage->SetSize(pRefPage->GetSize());
        pHandoutPage->SetBorder( pRefPage->GetLeftBorder(), pRefPage->GetUpperBorder(), pRefPage->GetRightBorder(), pRefPage->GetLowerBorder() );
    }
    else
    {
        pHandoutPage->SetSize(aDefSize);
        pHandoutPage->SetBorder(0, 0, 0, 0);
    }

    pHandoutPage->SetPageKind(PageKind::Handout);
    pHandoutPage->SetName( SdResId(STR_HANDOUT) );
    InsertPage(pHandoutPage.get(), 0);

    // Insert master page and register this with the handout page
    rtl::Reference<SdPage> pHandoutMPage = AllocSdPage(true);
    pHandoutMPage->SetSize( pHandoutPage->GetSize() );
    pHandoutMPage->SetPageKind(PageKind::Handout);
    pHandoutMPage->SetBorder( pHandoutPage->GetLeftBorder(),
                              pHandoutPage->GetUpperBorder(),
                              pHandoutPage->GetRightBorder(),
                              pHandoutPage->GetLowerBorder() );
    InsertMasterPage(pHandoutMPage.get(), 0);
    pHandoutPage->TRG_SetMasterPage( *pHandoutMPage );

    // Insert page
    // If nPageCount==1 is, the model for the clipboard was created, thus a
    // default page must already exist
    rtl::Reference<SdPage> pPage;
    bool bClipboard = false;

    if( pRefDocument )
        pRefPage = pRefDocument->GetSdPage( 0, PageKind::Standard );

    if (nPageCount == 0)
    {
        pPage = AllocSdPage(false);

        if( pRefPage )
        {
            pPage->SetSize( pRefPage->GetSize() );
            pPage->SetBorder( pRefPage->GetLeftBorder(), pRefPage->GetUpperBorder(), pRefPage->GetRightBorder(), pRefPage->GetLowerBorder() );
        }
        else if (meDocType == DocumentType::Draw)
        {
            // Draw: always use default size with margins
            pPage->SetSize(aDefSize);

            SfxPrinter* pPrinter = mpDocSh->GetPrinter(false);
            if (pPrinter && pPrinter->IsValid())
            {
                Size aOutSize(pPrinter->GetOutputSize());
                Point aPageOffset(pPrinter->GetPageOffset());
                aPageOffset -= pPrinter->PixelToLogic( Point() );
                ::tools::Long nOffset = !aPageOffset.X() && !aPageOffset.Y() ? 0 : PRINT_OFFSET;

                sal_uLong nTop    = aPageOffset.Y();
                sal_uLong nLeft   = aPageOffset.X();
                sal_uLong nBottom = std::max(::tools::Long(aDefSize.Height() - aOutSize.Height() - nTop + nOffset), ::tools::Long(0));
                sal_uLong nRight  = std::max(::tools::Long(aDefSize.Width() - aOutSize.Width() - nLeft + nOffset), ::tools::Long(0));

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
            Size aSz( SvxPaperInfo::GetPaperSize(PAPER_SCREEN_16_9, MapUnit::Map100thMM) );
            pPage->SetSize( Size( aSz.Height(), aSz.Width() ) );
            pPage->SetBorder(0, 0, 0, 0);
        }

        InsertPage(pPage.get(), 1);
    }
    else
    {
        bClipboard = true;
        pPage = static_cast<SdPage*>( GetPage(1) );
    }

    // Insert master page, then register this with the page
    rtl::Reference<SdPage> pMPage = AllocSdPage(true);
    pMPage->SetSize( pPage->GetSize() );
    pMPage->SetBorder( pPage->GetLeftBorder(),
                       pPage->GetUpperBorder(),
                       pPage->GetRightBorder(),
                       pPage->GetLowerBorder() );
    InsertMasterPage(pMPage.get(), 1);
    pPage->TRG_SetMasterPage( *pMPage );
    if( bClipboard )
        pMPage->SetLayoutName( pPage->GetLayoutName() );

    // Insert notes page
    rtl::Reference<SdPage> pNotesPage = AllocSdPage(false);

    if( pRefDocument )
        pRefPage = pRefDocument->GetSdPage( 0, PageKind::Notes );

    if( pRefPage )
    {
        pNotesPage->SetSize( pRefPage->GetSize() );
        pNotesPage->SetBorder( pRefPage->GetLeftBorder(), pRefPage->GetUpperBorder(), pRefPage->GetRightBorder(), pRefPage->GetLowerBorder() );
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
    pNotesPage->SetPageKind(PageKind::Notes);
    InsertPage(pNotesPage.get(), 2);
    if( bClipboard )
        pNotesPage->SetLayoutName( pPage->GetLayoutName() );

    // Insert master page, then register this with the notes page
    rtl::Reference<SdPage> pNotesMPage = AllocSdPage(true);
    pNotesMPage->SetSize( pNotesPage->GetSize() );
    pNotesMPage->SetPageKind(PageKind::Notes);
    pNotesMPage->SetBorder( pNotesPage->GetLeftBorder(),
                            pNotesPage->GetUpperBorder(),
                            pNotesPage->GetRightBorder(),
                            pNotesPage->GetLowerBorder() );
    InsertMasterPage(pNotesMPage.get(), 2);
    pNotesPage->TRG_SetMasterPage( *pNotesMPage );
    if( bClipboard )
        pNotesMPage->SetLayoutName( pPage->GetLayoutName() );

    if( !pRefPage && (meDocType != DocumentType::Draw) )
        pPage->SetAutoLayout( AUTOLAYOUT_TITLE, true, true );

    mpWorkStartupTimer.reset( new Timer("DrawWorkStartupTimer") );
    mpWorkStartupTimer->SetInvokeHandler( LINK(this, SdDrawDocument, WorkStartupHdl) );
    mpWorkStartupTimer->SetTimeout(2000);
    mpWorkStartupTimer->Start();

    SetChanged(false);
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
        pHandoutMPage->SetPageKind(PageKind::Handout);

        SdPage* pHandoutPage = static_cast<SdPage*>( GetPage(0) );
        pHandoutPage->SetPageKind(PageKind::Handout);
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
            pNotesPage->SetPageKind(PageKind::Notes);

            // Set notes master page
            sal_uInt16 nMasterPageAfterPagesMasterPage = pPage->TRG_GetMasterPage().GetPageNum() + 1;
            pNotesPage->TRG_SetMasterPage(*GetMasterPage(nMasterPageAfterPagesMasterPage));
        }

        bOK = true;
        StopWorkStartupDelay();
        SetChanged(false);
    }

    return bOK;
}

void SdDrawDocument::UnselectAllPages()
{
    sal_uInt16 nNoOfPages = GetSdPageCount(PageKind::Standard);
    for (sal_uInt16 nPage = 0; nPage < nNoOfPages; ++nPage)
    {
        SdPage* pPage = GetSdPage(nPage, PageKind::Standard);
        pPage->SetSelected(false);
    }
}

bool SdDrawDocument::MoveSelectedPages(sal_uInt16 nTargetPage)
{
    sal_uInt16 nNoOfPages = GetSdPageCount(PageKind::Standard);
    std::vector<SdPage*> aPageList;
    for (sal_uInt16 nPage = 0; nPage < nNoOfPages; nPage++)
    {
        SdPage* pPage = GetSdPage(nPage, PageKind::Standard);

        if (pPage->IsSelected())
        {
            aPageList.push_back(pPage);
        }
    }
    return MovePages(nTargetPage, aPageList);
}

// + Move selected pages after said page
//   (nTargetPage = (sal_uInt16)-1  --> move before first page)
// + Returns sal_True when the page has been moved
bool SdDrawDocument::MovePages(sal_uInt16 nTargetPage, const std::vector<SdPage*>& rSelectedPages)
{
    SdPage* pPage              = nullptr;
    sal_uInt16 nPage;
    bool    bSomethingHappened = false;

    const bool bUndo = IsUndoEnabled();

    if( bUndo )
        BegUndo(SdResId(STR_UNDO_MOVEPAGES));

    // If necessary, look backwards, until we find a page that wasn't selected
    nPage = nTargetPage;

    if (nPage != sal_uInt16(-1))
    {
        pPage = GetSdPage(nPage, PageKind::Standard);
        while (nPage > 0 && pPage->IsSelected())
        {
            nPage--;
            pPage = GetSdPage(nPage, PageKind::Standard);
        }

        if (pPage->IsSelected())
        {
            nPage = sal_uInt16(-1);
        }
    }

    // Insert before the first page
    if (nPage == sal_uInt16(-1))
    {
        for (auto iter = rSelectedPages.rbegin(); iter != rSelectedPages.rend(); ++iter)
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
        nTargetPage = 2 * nPage + 1;    // PageKind::Standard --> absolute

        for (const auto& rpPage : rSelectedPages)
        {
            nPage = rpPage->GetPageNum();
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
            nTargetPage = rpPage->GetPageNum();
        }
    }

    if( bUndo )
        EndUndo();

    return bSomethingHappened;
}

// Return number of links in sfx2::LinkManager
sal_uLong SdDrawDocument::GetLinkCount() const
{
    return m_pLinkManager->GetLinks().size();
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
        m_pHitTestOutliner->SetDefaultLanguage( Application::GetSettings().GetLanguageTag().getLanguageType() );
        m_pItemPool->SetUserDefaultItem( SvxLanguageItem( eLang, nId ) );
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
IMPL_LINK_NOARG(SdDrawDocument, WorkStartupHdl, Timer *, void)
{
    if (IsTransportContainer())
        return;

    if( mpDocSh )
        mpDocSh->SetWaitCursor( true );

    bool bChanged = IsChanged();        // remember this

    // Initialize Autolayouts
    SdPage* pHandoutMPage = GetMasterSdPage(0, PageKind::Handout);

    if (pHandoutMPage->GetAutoLayout() == AUTOLAYOUT_NONE)
    {
        // No AutoLayout yet -> initialize
        pHandoutMPage->SetAutoLayout(AUTOLAYOUT_HANDOUT6, true, true);
    }

    SdPage* pPage = GetSdPage(0, PageKind::Standard);

    if (pPage->GetAutoLayout() == AUTOLAYOUT_NONE)
    {
        // No AutoLayout yet -> initialize
        pPage->SetAutoLayout(AUTOLAYOUT_NONE, true, true);
    }

    SdPage* pNotesPage = GetSdPage(0, PageKind::Notes);

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

        mpWorkStartupTimer.reset();
    }
}

// When the WorkStartupTimer has been created (this only happens in
// SdDrawViewShell::Construct() ), the timer may be stopped and the WorkStartup
// may be initiated.
SdAnimationInfo* SdDrawDocument::GetAnimationInfo(SdrObject* pObject)
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
        if((pUD->GetInventor() == SdrInventor::StarDrawUserData) && (pUD->GetId() == SD_ANIMATIONINFO_ID))
        {
            pRet = dynamic_cast<SdAnimationInfo*>(pUD);
            break;
        }
    }

    if( (pRet == nullptr) && bCreate )
    {
        pRet = new SdAnimationInfo( rObject );
        rObject.AppendUserData( std::unique_ptr<SdrObjUserData>(pRet) );
    }

    return pRet;
}

/** this method enforces that the masterpages are in the correct order,
    that is at position 1 is a PageKind::Standard masterpage followed by a
    PageKind::Notes masterpage and so on. #
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
        if( ((1 == (nPage & 1)) && (pPage->GetPageKind() != PageKind::Standard) ) ||
            ((0 == (nPage & 1)) && (pPage->GetPageKind() != PageKind::Notes) ) )
            break; // then we have a fatal error
    }

    if( nPage >= nMaxPages )
        return;

    SdPage* pNotesPage = nullptr;

    // there is a fatal error in the master page order,
    // we need to repair the document
    bool bChanged = false;

    nPage = 1;
    while( nPage < nMaxPages )
    {
        pPage = static_cast<SdPage*> (GetMasterPage( nPage ));
        if( pPage->GetPageKind() != PageKind::Standard )
        {
            bChanged = true;
            sal_uInt16 nFound = nPage + 1;
            while( nFound < nMaxPages )
            {
                pPage = static_cast<SdPage*>(GetMasterPage( nFound ));
                if( PageKind::Standard == pPage->GetPageKind() )
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

        if( (nullptr == pNotesPage) || (pNotesPage->GetPageKind() != PageKind::Notes) || ( pPage->GetLayoutName() != pNotesPage->GetLayoutName() ) )
        {
            bChanged = true;

            sal_uInt16 nFound = nPage + 1;
            while( nFound < nMaxPages )
            {
                pNotesPage = static_cast<SdPage*>(GetMasterPage( nFound ));
                if( (PageKind::Notes == pNotesPage->GetPageKind()) && ( pPage->GetLayoutName() == pNotesPage->GetLayoutName() ) )
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
                    if( PageKind::Notes == pRefNotesPage->GetPageKind() )
                        break;
                    nFound++;
                }
                if( nFound == nMaxPages )
                    pRefNotesPage = nullptr;

                rtl::Reference<SdPage> pNewNotesPage = AllocSdPage(true);
                pNewNotesPage->SetPageKind(PageKind::Notes);
                if( pRefNotesPage )
                {
                    pNewNotesPage->SetSize( pRefNotesPage->GetSize() );
                    pNewNotesPage->SetBorder( pRefNotesPage->GetLeftBorder(),
                                            pRefNotesPage->GetUpperBorder(),
                                            pRefNotesPage->GetRightBorder(),
                                            pRefNotesPage->GetLowerBorder() );
                }
                InsertMasterPage(pNewNotesPage.get(),  nPage );
                pNewNotesPage->SetLayoutName( pPage->GetLayoutName() );
                pNewNotesPage->SetAutoLayout(AUTOLAYOUT_NOTES, true, true );
                nMaxPages++;
            }
        }

        nPage++;
    }

    // now remove all remaining and unused non PageKind::Standard slides
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
    rtl::Reference<SdPage> pStandardPage;
    rtl::Reference<SdPage> pNotesPage;

    // From the given page determine the standard page and notes page of which
    // to take the layout and the position where to insert the new pages.
    if (ePageKind == PageKind::Notes)
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
    pStandardPage->SetBorder( pPreviousStandardPage->GetLeftBorder(),
                              pPreviousStandardPage->GetUpperBorder(),
                              pPreviousStandardPage->GetRightBorder(),
                              pPreviousStandardPage->GetLowerBorder() );

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
    pNotesPage->SetPageKind(PageKind::Notes);

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
        pStandardPage.get(),
        pNotesPage.get(),
        nInsertPosition);
}

sal_uInt16 SdDrawDocument::DuplicatePage (sal_uInt16 nPageNum)
{
    PageKind ePageKind = PageKind::Standard;

    // Get current page
    SdPage* pActualPage = GetSdPage(nPageNum, ePageKind);

    // Get background flags
    SdrLayerAdmin& rLayerAdmin = GetLayerAdmin();
    SdrLayerID aBckgrnd = rLayerAdmin.GetLayerID(sUNO_LayerName_background);
    SdrLayerID aBckgrndObj = rLayerAdmin.GetLayerID(sUNO_LayerName_background_objects);
    SdrLayerIDSet aVisibleLayers = pActualPage->TRG_GetMasterPageVisibleLayers();

    return DuplicatePage (
        pActualPage, ePageKind,
        // No names for the new slides
        OUString(), OUString(),
        aVisibleLayers.IsSet(aBckgrnd),
        aVisibleLayers.IsSet(aBckgrndObj), -1);
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
    rtl::Reference<SdPage> pStandardPage;
    rtl::Reference<SdPage> pNotesPage;

    // From the given page determine the standard page and the notes page
    // of which to make copies.
    if (ePageKind == PageKind::Notes)
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
    pStandardPage = static_cast<SdPage*>( pPreviousStandardPage->CloneSdrPage(*this).get() );
    pNotesPage = static_cast<SdPage*>( pPreviousNotesPage->CloneSdrPage(*this).get() );

    return InsertPageSet (
        pActualPage,
        ePageKind,
        sStandardPageName,
        sNotesPageName,
        bIsPageBack,
        bIsPageObj,
        pStandardPage.get(),
        pNotesPage.get(),
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
    if (ePageKind == PageKind::Notes)
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
    pNotesPage->SetPageKind(PageKind::Notes);
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
    SdPage const * pPreviousPage,
    SdPage* pPage,
    const OUString& sPageName,
    sal_uInt16 nInsertionPoint,
    bool bIsPageBack,
    bool bIsPageObj)
{
    if (pPreviousPage != nullptr)
    {
        pPage->SetSize( pPreviousPage->GetSize() );
        pPage->SetBorder( pPreviousPage->GetLeftBorder(),
            pPreviousPage->GetUpperBorder(),
            pPreviousPage->GetRightBorder(),
            pPreviousPage->GetLowerBorder() );
    }
    pPage->SetName(sPageName);

    InsertPage(pPage, nInsertionPoint);

    if (pPreviousPage != nullptr)
    {
        SdrLayerAdmin& rLayerAdmin = GetLayerAdmin();
        SdrLayerID aBckgrnd = rLayerAdmin.GetLayerID(sUNO_LayerName_background);
        SdrLayerID aBckgrndObj = rLayerAdmin.GetLayerID(sUNO_LayerName_background_objects);
        SdrLayerIDSet aVisibleLayers = pPreviousPage->TRG_GetMasterPageVisibleLayers();
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
