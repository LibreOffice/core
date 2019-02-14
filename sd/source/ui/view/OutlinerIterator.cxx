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

#include <OutlinerIterator.hxx>
#include <OutlinerIteratorImpl.hxx>
#include <svx/svditer.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <Outliner.hxx>

#include <drawdoc.hxx>
#include <DrawViewShell.hxx>
#include <drawview.hxx>
#include <sdpage.hxx>
#include <FrameView.hxx>
#include <DrawDocShell.hxx>
#include <Window.hxx>

namespace sd { namespace outliner {

//===== IteratorPosition ======================================================

IteratorPosition::IteratorPosition()
: mnText(0)
, mnPageIndex(-1)
, mePageKind(PageKind::Standard)
, meEditMode(EditMode::Page)
{
}

bool IteratorPosition::operator== (const IteratorPosition& aPosition) const
{
    return mxObject.get() == aPosition.mxObject.get()
        && mnText == aPosition.mnText
        && mnPageIndex == aPosition.mnPageIndex
        && mePageKind == aPosition.mePageKind
        && meEditMode == aPosition.meEditMode;
}

//===== Iterator ==============================================================

Iterator::Iterator()
{
}

Iterator::Iterator (const Iterator& rIterator)
    : mxIterator(rIterator.mxIterator ? rIterator.mxIterator->Clone() : nullptr)
{
}

Iterator::Iterator (Iterator&& rIterator)
    : mxIterator(std::move(rIterator.mxIterator))
{
}

Iterator::Iterator (std::unique_ptr<IteratorImplBase> pObject)
    : mxIterator(std::move(pObject))
{
}

Iterator::~Iterator()
{
}

Iterator& Iterator::operator= (const Iterator& rIterator)
{
    if (this != &rIterator)
    {
        if (rIterator.mxIterator)
            mxIterator.reset(rIterator.mxIterator->Clone());
        else
            mxIterator.reset();
    }
    return *this;
}

Iterator& Iterator::operator= (Iterator&& rIterator)
{
    mxIterator = std::move(rIterator.mxIterator);
    return *this;
}

const IteratorPosition& Iterator::operator* () const
{
    DBG_ASSERT (mxIterator, "::sd::outliner::Iterator::operator* : missing implementation object");
    return mxIterator->GetPosition();
}

Iterator& Iterator::operator++ ()
{
    if (mxIterator)
        mxIterator->GotoNextText();
    return *this;
}

bool Iterator::operator== (const Iterator& rIterator)
{
    if (!mxIterator || !rIterator.mxIterator)
        return mxIterator.get() == rIterator.mxIterator.get();
    else
        return *mxIterator == *rIterator.mxIterator;
}

bool Iterator::operator!= (const Iterator& rIterator)
{
    return ! operator==(rIterator);
}

void Iterator::Reverse()
{
    if (mxIterator)
        mxIterator->Reverse();
}

//===== IteratorFactory =======================================================

OutlinerContainer::OutlinerContainer (SdOutliner* pOutliner)
: mpOutliner(pOutliner)
{
}

Iterator OutlinerContainer::begin()
{
    return CreateIterator (BEGIN);
}

Iterator OutlinerContainer::end()
{
    return CreateIterator (END);
}

Iterator OutlinerContainer::current()
{
    return CreateIterator (CURRENT);
}

Iterator OutlinerContainer::CreateIterator (IteratorLocation aLocation)
{
    // Decide on certain features of the outliner which kind of iterator to
    // use.
    if (mpOutliner->mbRestrictSearchToSelection)
        // There is a selection.  Search only in this.
        return CreateSelectionIterator (
            mpOutliner->maMarkListCopy,
            mpOutliner->mpDrawDocument,
            mpOutliner->mpWeakViewShell.lock(),
            mpOutliner->mbDirectionIsForward,
            aLocation);
    else
        // Search in the whole document.
        return CreateDocumentIterator (
            mpOutliner->mpDrawDocument,
            mpOutliner->mpWeakViewShell.lock(),
            mpOutliner->mbDirectionIsForward,
            aLocation);
}

Iterator OutlinerContainer::CreateSelectionIterator (
    const ::std::vector<::tools::WeakReference<SdrObject>>& rObjectList,
    SdDrawDocument* pDocument,
    const std::shared_ptr<ViewShell>& rpViewShell,
    bool bDirectionIsForward,
    IteratorLocation aLocation)
{
    OSL_ASSERT(rpViewShell.get());

    sal_Int32 nObjectIndex;

    if (bDirectionIsForward)
        switch (aLocation)
        {
            case CURRENT:
            case BEGIN:
            default:
                nObjectIndex = 0;
                break;
            case END:
                nObjectIndex = rObjectList.size();
                break;
        }
    else
        switch (aLocation)
        {
            case CURRENT:
            case BEGIN:
            default:
                nObjectIndex = rObjectList.size()-1;
                break;
            case END:
                nObjectIndex = -1;
                break;
        }

    return Iterator (std::make_unique<SelectionIteratorImpl> (
        rObjectList, nObjectIndex, pDocument, rpViewShell, bDirectionIsForward));
}

Iterator OutlinerContainer::CreateDocumentIterator (
    SdDrawDocument* pDocument,
    const std::shared_ptr<ViewShell>& rpViewShell,
    bool bDirectionIsForward,
    IteratorLocation aLocation)
{
    OSL_ASSERT(rpViewShell.get());

    PageKind ePageKind;
    EditMode eEditMode;

    switch (aLocation)
    {
        case BEGIN:
        default:
            if (bDirectionIsForward)
            {
                ePageKind = PageKind::Standard;
                eEditMode = EditMode::Page;
            }
            else
            {
                ePageKind = PageKind::Handout;
                eEditMode = EditMode::MasterPage;
            }
            break;

        case END:
            if (bDirectionIsForward)
            {
                ePageKind = PageKind::Handout;
                eEditMode = EditMode::MasterPage;
            }
            else
            {
                ePageKind = PageKind::Standard;
                eEditMode = EditMode::Page;
            }
            break;

        case CURRENT:
            const std::shared_ptr<DrawViewShell> pDrawViewShell(
                std::dynamic_pointer_cast<DrawViewShell>(rpViewShell));
            if (pDrawViewShell.get())
            {
                ePageKind = pDrawViewShell->GetPageKind();
                eEditMode = pDrawViewShell->GetEditMode();
            }
            else
            {
                ePageKind = PageKind::Standard;
                eEditMode = EditMode::Page;
            }
            break;
    }

    sal_Int32 nPageIndex = GetPageIndex (pDocument, rpViewShell,
        ePageKind, eEditMode, bDirectionIsForward, aLocation);

    return Iterator (
        std::make_unique<DocumentIteratorImpl> (nPageIndex, ePageKind, eEditMode,
            pDocument, rpViewShell, bDirectionIsForward));
}

sal_Int32 OutlinerContainer::GetPageIndex (
    SdDrawDocument const * pDocument,
    const std::shared_ptr<ViewShell>& rpViewShell,
    PageKind ePageKind,
    EditMode eEditMode,
    bool bDirectionIsForward,
    IteratorLocation aLocation)
{
    OSL_ASSERT(rpViewShell);

    sal_Int32 nPageIndex;
    sal_Int32 nPageCount;

    const std::shared_ptr<DrawViewShell> pDrawViewShell(
        std::dynamic_pointer_cast<DrawViewShell>(rpViewShell));

    switch (eEditMode)
    {
        case EditMode::Page:
            nPageCount = pDocument->GetSdPageCount (ePageKind);
            break;
        case EditMode::MasterPage:
            nPageCount = pDocument->GetMasterSdPageCount(ePageKind);
            break;
        default:
            nPageCount = 0;
    }

    switch (aLocation)
    {
        case CURRENT:
            if (pDrawViewShell.get())
                nPageIndex = pDrawViewShell->GetCurPagePos();
            else
            {
                const SdPage* pPage = rpViewShell->GetActualPage();
                if (pPage != nullptr)
                    nPageIndex = (pPage->GetPageNum()-1)/2;
                else
                    nPageIndex = 0;
            }
            break;

        case BEGIN:
        default:
            if (bDirectionIsForward)
                nPageIndex = 0;
            else
                nPageIndex = nPageCount-1;
            break;

        case END:
            if (bDirectionIsForward)
                nPageIndex = nPageCount;
            else
                nPageIndex = -1;
            break;
    }

    return nPageIndex;
}

//===== IteratorImplBase ====================================================

IteratorImplBase::IteratorImplBase(SdDrawDocument* pDocument,
    const std::weak_ptr<ViewShell>& rpViewShellWeak,
    bool bDirectionIsForward)
:   maPosition()
,   mpDocument (pDocument)
,   mpViewShellWeak (rpViewShellWeak)
,   mbDirectionIsForward (bDirectionIsForward)
{
    std::shared_ptr<DrawViewShell> pDrawViewShell;
    if ( ! mpViewShellWeak.expired())
        pDrawViewShell = std::dynamic_pointer_cast<DrawViewShell>(rpViewShellWeak.lock());

    if (pDrawViewShell.get())
    {
        maPosition.mePageKind = pDrawViewShell->GetPageKind();
        maPosition.meEditMode = pDrawViewShell->GetEditMode();
    }
    else
    {
        maPosition.mePageKind = PageKind::Standard;
        maPosition.meEditMode = EditMode::Page;
    }
}

IteratorImplBase::IteratorImplBase( SdDrawDocument* pDocument,
    const std::weak_ptr<ViewShell>& rpViewShellWeak,
    bool bDirectionIsForward, PageKind ePageKind, EditMode eEditMode)
: maPosition()
, mpDocument (pDocument)
, mpViewShellWeak (rpViewShellWeak)
, mbDirectionIsForward (bDirectionIsForward)
{
    maPosition.mePageKind = ePageKind;
    maPosition.meEditMode = eEditMode;
}

IteratorImplBase::~IteratorImplBase()
{}

bool IteratorImplBase::operator== (const IteratorImplBase& rIterator) const
{
    return maPosition == rIterator.maPosition;
}

bool IteratorImplBase::IsEqualSelection(const IteratorImplBase& rIterator) const
{
    // When this method is executed instead of the ones from derived classes
    // then the argument is of another type then the object itself.  In this
    // just compare the position objects.
    return maPosition == rIterator.maPosition;
}

const IteratorPosition& IteratorImplBase::GetPosition()
{
    return maPosition;
}

IteratorImplBase* IteratorImplBase::Clone (IteratorImplBase* pObject) const
{
    if (pObject != nullptr)
    {
        pObject->maPosition = maPosition;
        pObject->mpDocument = mpDocument;
        pObject->mpViewShellWeak = mpViewShellWeak;
        pObject->mbDirectionIsForward = mbDirectionIsForward;
    }
    return pObject;
}

void IteratorImplBase::Reverse()
{
    mbDirectionIsForward = ! mbDirectionIsForward;
}

//===== SelectionIteratorImpl ===========================================

SelectionIteratorImpl::SelectionIteratorImpl (
    const ::std::vector<::tools::WeakReference<SdrObject>>& rObjectList,
    sal_Int32 nObjectIndex,
    SdDrawDocument* pDocument,
    const std::weak_ptr<ViewShell>& rpViewShellWeak,
    bool bDirectionIsForward)
    : IteratorImplBase (pDocument, rpViewShellWeak, bDirectionIsForward),
      mrObjectList(rObjectList),
      mnObjectIndex(nObjectIndex)
{
}

SelectionIteratorImpl::~SelectionIteratorImpl()
{}

IteratorImplBase* SelectionIteratorImpl::Clone (IteratorImplBase* pObject) const
{
    SelectionIteratorImpl* pIterator = static_cast<SelectionIteratorImpl*>(pObject);
    if (pIterator == nullptr)
        pIterator = new SelectionIteratorImpl (
            mrObjectList, mnObjectIndex, mpDocument, mpViewShellWeak, mbDirectionIsForward);
    return pIterator;
}

void SelectionIteratorImpl::GotoNextText()
{
    SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( mrObjectList.at(mnObjectIndex).get() );
    if (mbDirectionIsForward)
    {
        if( pTextObj )
        {
            ++maPosition.mnText;
            if( maPosition.mnText >= pTextObj->getTextCount() )
            {
                maPosition.mnText = 0;
                ++mnObjectIndex;
            }
        }
        else
        {
            ++mnObjectIndex;
        }
    }
    else
    {
        if( pTextObj )
        {
            --maPosition.mnText;
            if( maPosition.mnText < 0 )
            {
                maPosition.mnText = -1;
                --mnObjectIndex;
            }
        }
        else
        {
            --mnObjectIndex;
            maPosition.mnText = -1;
        }

        if( (maPosition.mnText == -1) && (mnObjectIndex >= 0) )
        {
            pTextObj = dynamic_cast< SdrTextObj* >( mrObjectList.at(mnObjectIndex).get() );
            if( pTextObj )
                maPosition.mnText = pTextObj->getTextCount() - 1;
        }

        if( maPosition.mnText == -1 )
            maPosition.mnText = 0;
    }
}

const IteratorPosition& SelectionIteratorImpl::GetPosition()
{
    maPosition.mxObject = mrObjectList.at(mnObjectIndex);

    return maPosition;
}

bool SelectionIteratorImpl::operator== (const IteratorImplBase& rIterator) const
{
    return rIterator.IsEqualSelection(*this);
}

bool SelectionIteratorImpl::IsEqualSelection(const IteratorImplBase& rIterator) const
{
    const SelectionIteratorImpl* pSelectionIterator =
        static_cast<const SelectionIteratorImpl*>(&rIterator);
    return mpDocument == pSelectionIterator->mpDocument
        && mnObjectIndex == pSelectionIterator->mnObjectIndex;
}

//===== ViewIteratorImpl ================================================

ViewIteratorImpl::ViewIteratorImpl (
    sal_Int32 nPageIndex,
    SdDrawDocument* pDocument,
    const std::weak_ptr<ViewShell>& rpViewShellWeak,
    bool bDirectionIsForward)
    : IteratorImplBase (pDocument, rpViewShellWeak, bDirectionIsForward),
      mbPageChangeOccurred(false),
      mpPage(nullptr)
{
    SetPage (nPageIndex);
}

ViewIteratorImpl::ViewIteratorImpl (
    sal_Int32 nPageIndex,
    SdDrawDocument* pDocument,
    const std::weak_ptr<ViewShell>& rpViewShellWeak,
    bool bDirectionIsForward,
    PageKind ePageKind,
    EditMode eEditMode)
    : IteratorImplBase (pDocument, rpViewShellWeak, bDirectionIsForward, ePageKind, eEditMode),
      mbPageChangeOccurred(false),
      mpPage(nullptr)
{
    SetPage (nPageIndex);
}

ViewIteratorImpl::~ViewIteratorImpl()
{
}

IteratorImplBase* ViewIteratorImpl::Clone (IteratorImplBase* pObject) const
{

    ViewIteratorImpl* pIterator = static_cast<ViewIteratorImpl*>(pObject);
    if (pIterator == nullptr)
        pIterator = new ViewIteratorImpl (
            maPosition.mnPageIndex, mpDocument, mpViewShellWeak, mbDirectionIsForward);

    IteratorImplBase::Clone (pObject);

    if (mpObjectIterator != nullptr)
    {
        pIterator->mpObjectIterator.reset( new SdrObjListIter(mpPage, SdrIterMode::DeepNoGroups, !mbDirectionIsForward) );

        // No direct way to set the object iterator to the current object.
        pIterator->maPosition.mxObject.reset(nullptr);
        while (pIterator->mpObjectIterator->IsMore() && pIterator->maPosition.mxObject!=maPosition.mxObject)
            pIterator->maPosition.mxObject.reset(pIterator->mpObjectIterator->Next());
    }
    else
        pIterator->mpObjectIterator.reset();

    return pIterator;
}

void ViewIteratorImpl::GotoNextText()
{
    SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( maPosition.mxObject.get() );
    if( pTextObj )
    {
        if (mbDirectionIsForward)
        {
            ++maPosition.mnText;
            if( maPosition.mnText < pTextObj->getTextCount() )
                return;
        }
        else
        {
            --maPosition.mnText;
            if( maPosition.mnText >= 0 )
                return;
        }
    }

    if (mpObjectIterator != nullptr && mpObjectIterator->IsMore())
        maPosition.mxObject.reset(mpObjectIterator->Next());
    else
        maPosition.mxObject.reset(nullptr);

    if (!maPosition.mxObject.is() )
    {
        if (mbDirectionIsForward)
            SetPage (maPosition.mnPageIndex+1);
        else
            SetPage (maPosition.mnPageIndex-1);

        if (mpPage != nullptr)
            mpObjectIterator.reset( new SdrObjListIter(mpPage, SdrIterMode::DeepNoGroups, !mbDirectionIsForward) );
        if (mpObjectIterator!=nullptr && mpObjectIterator->IsMore())
            maPosition.mxObject.reset(mpObjectIterator->Next());
        else
            maPosition.mxObject.reset(nullptr);
    }

    maPosition.mnText = 0;
    if( !mbDirectionIsForward && maPosition.mxObject.is() )
    {
        pTextObj = dynamic_cast< SdrTextObj* >( maPosition.mxObject.get() );
        if( pTextObj )
            maPosition.mnText = pTextObj->getTextCount() - 1;
    }
}

void ViewIteratorImpl::SetPage (sal_Int32 nPageIndex)
{
    mbPageChangeOccurred = (maPosition.mnPageIndex!=nPageIndex);
    if (mbPageChangeOccurred)
    {
        maPosition.mnPageIndex = nPageIndex;

        sal_Int32 nPageCount;
        if (maPosition.meEditMode == EditMode::Page)
            nPageCount = mpDocument->GetSdPageCount(maPosition.mePageKind);
        else
            nPageCount = mpDocument->GetMasterSdPageCount(
                maPosition.mePageKind);

        // Get page pointer.  Here we have three cases: regular pages,
        // master pages and invalid page indices.  The later ones are not
        // errors but the effect of the iterator advancing to the next page
        // and going past the last one.  This dropping of the rim at the far
        // side is detected here and has to be reacted to by the caller.
        if (nPageIndex>=0 && nPageIndex < nPageCount)
        {
            if (maPosition.meEditMode == EditMode::Page)
                mpPage = mpDocument->GetSdPage (
                    static_cast<sal_uInt16>(nPageIndex),
                    maPosition.mePageKind);
            else
                mpPage = mpDocument->GetMasterSdPage (
                    static_cast<sal_uInt16>(nPageIndex),
                    maPosition.mePageKind);
        }
        else
            mpPage = nullptr;
    }

    // Set up object list iterator.
    if (mpPage != nullptr)
        mpObjectIterator.reset( new SdrObjListIter(mpPage, SdrIterMode::DeepNoGroups, ! mbDirectionIsForward) );
    else
        mpObjectIterator.reset();

    // Get object pointer.
    if (mpObjectIterator!=nullptr && mpObjectIterator->IsMore())
        maPosition.mxObject.reset( mpObjectIterator->Next() );
    else
        maPosition.mxObject.reset( nullptr );

    maPosition.mnText = 0;
    if( !mbDirectionIsForward && maPosition.mxObject.is() )
    {
        SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( maPosition.mxObject.get() );
        if( pTextObj )
            maPosition.mnText = pTextObj->getTextCount() - 1;
    }

}

void ViewIteratorImpl::Reverse()
{
    IteratorImplBase::Reverse ();

    // Create reversed object list iterator.
    if (mpPage != nullptr)
        mpObjectIterator.reset( new SdrObjListIter(mpPage, SdrIterMode::DeepNoGroups, ! mbDirectionIsForward) );
    else
        mpObjectIterator.reset();

    // Move iterator to the current object.
    ::tools::WeakReference<SdrObject> xObject = maPosition.mxObject;
    maPosition.mxObject.reset(nullptr);

    if (!mpObjectIterator)
        return;

    while (mpObjectIterator->IsMore() && maPosition.mxObject != xObject)
        maPosition.mxObject.reset(mpObjectIterator->Next());
}

//===== DocumentIteratorImpl ============================================

DocumentIteratorImpl::DocumentIteratorImpl (
    sal_Int32 nPageIndex,
    PageKind ePageKind, EditMode eEditMode,
    SdDrawDocument* pDocument,
    const std::weak_ptr<ViewShell>& rpViewShellWeak,
    bool bDirectionIsForward)
    : ViewIteratorImpl (nPageIndex, pDocument, rpViewShellWeak, bDirectionIsForward,
        ePageKind, eEditMode)
{
    if (eEditMode == EditMode::Page)
        mnPageCount = pDocument->GetSdPageCount (ePageKind);
    else
        mnPageCount = pDocument->GetMasterSdPageCount(ePageKind);
}

DocumentIteratorImpl::~DocumentIteratorImpl()
{}

IteratorImplBase* DocumentIteratorImpl::Clone (IteratorImplBase* pObject) const
{
    DocumentIteratorImpl* pIterator = static_cast<DocumentIteratorImpl*>(pObject);
    if (pIterator == nullptr)
        pIterator = new DocumentIteratorImpl (
            maPosition.mnPageIndex, maPosition.mePageKind, maPosition.meEditMode,
            mpDocument, mpViewShellWeak, mbDirectionIsForward);
    // Finish the cloning.
    return ViewIteratorImpl::Clone (pIterator);
}

void DocumentIteratorImpl::GotoNextText()
{
    bool bSetToOnePastLastPage = false;
    bool bViewChanged = false;

    ViewIteratorImpl::GotoNextText();

    if (mbDirectionIsForward)
    {
        if (maPosition.mnPageIndex >= mnPageCount)
        {
            // Switch to master page.
            if (maPosition.meEditMode == EditMode::Page)
            {
                maPosition.meEditMode = EditMode::MasterPage;
                SetPage (0);
            }

            // Switch to next view mode.
            else
            {
                if (maPosition.mePageKind == PageKind::Handout)
                    // Not really necessary but makes things more clear.
                    bSetToOnePastLastPage = true;
                else
                {
                    maPosition.meEditMode = EditMode::Page;
                    if (maPosition.mePageKind == PageKind::Standard)
                        maPosition.mePageKind = PageKind::Notes;
                    else if (maPosition.mePageKind == PageKind::Notes)
                        maPosition.mePageKind = PageKind::Handout;
                    SetPage (0);
                }
            }
            bViewChanged = true;
        }
    }
    else
        if (maPosition.mnPageIndex < 0)
        {
            // Switch from master pages to draw pages.
            if (maPosition.meEditMode == EditMode::MasterPage)
            {
                maPosition.meEditMode = EditMode::Page;
                bSetToOnePastLastPage = true;
            }

            // Switch to previous view mode.
            else
            {
                if (maPosition.mePageKind == PageKind::Standard)
                    SetPage (-1);
                else
                {
                    maPosition.meEditMode = EditMode::MasterPage;
                    if (maPosition.mePageKind == PageKind::Handout)
                        maPosition.mePageKind = PageKind::Notes;
                    else if (maPosition.mePageKind == PageKind::Notes)
                        maPosition.mePageKind = PageKind::Standard;
                    bSetToOnePastLastPage = true;
                }
            }
            bViewChanged = true;
        }

    if (!bViewChanged)
        return;

    // Get new page count;
    sal_Int32 nPageCount;
    if (maPosition.meEditMode == EditMode::Page)
        nPageCount = mpDocument->GetSdPageCount (maPosition.mePageKind);
    else
        nPageCount = mpDocument->GetMasterSdPageCount(maPosition.mePageKind);

    // Now that we know the number of pages we can set the current page index.
    if (bSetToOnePastLastPage)
        SetPage (nPageCount);
}

} } // end of namespace ::sd::outliner

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
