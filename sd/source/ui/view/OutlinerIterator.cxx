/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: OutlinerIterator.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:55:26 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "OutlinerIterator.hxx"
#include "OutlinerIteratorImpl.hxx"

#ifndef _SVDITER_HXX
#include <svx/svditer.hxx>
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif

#ifndef SD_OUTLINER_HXX
#include "Outliner.hxx"
#endif

#include "drawdoc.hxx"
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif
#include "sdpage.hxx"
#ifndef SD_FRAME_VIEW
#include "FrameView.hxx"
#endif
#include "DrawDocShell.hxx"
#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif

namespace sd { namespace outliner {


//===== IteratorPosition ======================================================

IteratorPosition::IteratorPosition (void)
: mnText(0)
, mnPageIndex(-1)
, mePageKind(PK_STANDARD)
, meEditMode(EM_PAGE)
{
}

IteratorPosition::IteratorPosition (const IteratorPosition& aPosition)
: mxObject(aPosition.mxObject)
, mnText(aPosition.mnText)
, mnPageIndex(aPosition.mnPageIndex)
, mePageKind(aPosition.mePageKind)
, meEditMode(aPosition.meEditMode)
{
}

IteratorPosition::IteratorPosition (SdrObject* pObject, sal_Int32 nText, sal_Int32 nPageIndex, PageKind ePageKind, EditMode eEditMode)
: mxObject(pObject)
, mnText(nText)
, mnPageIndex(nPageIndex)
, mePageKind(ePageKind)
, meEditMode(eEditMode)
{
}

IteratorPosition::~IteratorPosition (void)
{
}

IteratorPosition& IteratorPosition::operator= (const IteratorPosition& aPosition)
{
    mxObject = aPosition.mxObject;
    mnText = aPosition.mnText;
    mnPageIndex = aPosition.mnPageIndex;
    mePageKind = aPosition.mePageKind;
    meEditMode = aPosition.meEditMode;
    return *this;
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

Iterator::Iterator (void)
{
    mpIterator = NULL;
}

Iterator::Iterator (const Iterator& rIterator)
{
    mpIterator = rIterator.mpIterator->Clone();
}

Iterator::Iterator (IteratorImplBase* pObject)
{
    mpIterator = pObject;
}

Iterator::Iterator (const IteratorImplBase& rObject)
{
    mpIterator = rObject.Clone();
}

Iterator::~Iterator (void)
{
    delete mpIterator;
}

Iterator& Iterator::operator= (const Iterator& rIterator)
{
    if (this != &rIterator)
    {
        delete mpIterator;
        if (rIterator.mpIterator != NULL)
            mpIterator = rIterator.mpIterator->Clone();
        else
            mpIterator = NULL;
    }
    return *this;
}

const IteratorPosition& Iterator::operator* () const
{
    DBG_ASSERT (mpIterator!=NULL, "::sd::outliner::Iterator::operator* : missing implementation object");
    return mpIterator->GetPosition();
}

Iterator& Iterator::operator++ ()
{
    if (mpIterator!=NULL)
        mpIterator->GotoNextText();
    return *this;
}

Iterator Iterator::operator++ (int)
{
    Iterator aTmp (*this);
    if (mpIterator!=NULL)
        mpIterator->GotoNextText();
    return aTmp;
}

bool Iterator::operator== (const Iterator& rIterator)
{
    if (mpIterator == NULL || rIterator.mpIterator==NULL)
        return mpIterator == rIterator.mpIterator;
    else
        return *mpIterator == *rIterator.mpIterator;
}

bool Iterator::operator!= (const Iterator& rIterator)
{
    return ! operator==(rIterator);
}

void Iterator::Reverse (void)
{
    if (mpIterator != NULL)
        mpIterator->Reverse();
}

//===== IteratorFactory =======================================================

OutlinerContainer::OutlinerContainer (Outliner* pOutliner)
: mpOutliner(pOutliner)
{
}

Iterator OutlinerContainer::begin (void)
{
    return CreateIterator (BEGIN);
}

Iterator OutlinerContainer::end (void)
{
    return CreateIterator (END);
}

Iterator OutlinerContainer::current (void)
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
            static_cast<DrawViewShell*>(mpOutliner->mpViewShell),
            mpOutliner->mbDirectionIsForward,
            aLocation);
    else
        // Search in the whole document.
        return CreateDocumentIterator (
            mpOutliner->mpDrawDocument,
            static_cast<DrawViewShell*>(mpOutliner->mpViewShell),
            mpOutliner->mbDirectionIsForward,
            aLocation);
}

Iterator OutlinerContainer::CreateSelectionIterator (
    const ::std::vector<SdrObjectWeakRef>& rObjectList,
    SdDrawDocument* pDocument,
    DrawViewShell* pViewShell,
    bool bDirectionIsForward,
    IteratorLocation aLocation)
{
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

    return Iterator (new SelectionIteratorImpl (
        rObjectList, nObjectIndex, pDocument, pViewShell, bDirectionIsForward));
}

Iterator OutlinerContainer::CreateViewIterator (
    SdDrawDocument* pDocument,
    DrawViewShell* pViewShell,
    bool bDirectionIsForward,
    IteratorLocation aLocation)
{
    sal_Int32 nPageIndex = GetPageIndex (pDocument, pViewShell,
        pViewShell->GetPageKind(), pViewShell->GetEditMode(),
        bDirectionIsForward, aLocation);

    return Iterator (new ViewIteratorImpl (
        nPageIndex, pDocument, pViewShell, bDirectionIsForward));
}

Iterator OutlinerContainer::CreateDocumentIterator (
    SdDrawDocument* pDocument,
    DrawViewShell* pViewShell,
    bool bDirectionIsForward,
    IteratorLocation aLocation)
{
    PageKind ePageKind;
    EditMode eEditMode;

    if (bDirectionIsForward)
        switch (aLocation)
        {
            case BEGIN:
            default:
                ePageKind = PK_STANDARD;
                eEditMode = EM_PAGE;
                break;
            case END:
                ePageKind = PK_HANDOUT;
                eEditMode = EM_MASTERPAGE;
                break;
            case CURRENT:
                ePageKind = pViewShell->GetPageKind();
                eEditMode = pViewShell->GetEditMode();
                break;
        }
    else
        switch (aLocation)
        {
            case BEGIN:
            default:
                ePageKind = PK_HANDOUT;
                eEditMode = EM_MASTERPAGE;
                break;
            case END:
                ePageKind = PK_STANDARD;
                eEditMode = EM_PAGE;
                break;
            case CURRENT:
                ePageKind = pViewShell->GetPageKind();
                eEditMode = pViewShell->GetEditMode();
                break;
        }

    sal_Int32 nPageIndex = GetPageIndex (pDocument, pViewShell,
        ePageKind, eEditMode, bDirectionIsForward, aLocation);

    return Iterator (
        new DocumentIteratorImpl (nPageIndex, ePageKind, eEditMode,
            pDocument, pViewShell, bDirectionIsForward));
}

sal_Int32 OutlinerContainer::GetPageIndex (
    SdDrawDocument* pDocument,
    DrawViewShell* pViewShell,
    PageKind ePageKind,
    EditMode eEditMode,
    bool bDirectionIsForward,
    IteratorLocation aLocation)
{
    sal_Int32 nPageIndex;
    sal_Int32 nPageCount;

    switch (eEditMode)
    {
        case EM_PAGE:
            nPageCount = pDocument->GetSdPageCount (ePageKind);
            break;
        case EM_MASTERPAGE:
            nPageCount = pDocument->GetMasterSdPageCount(ePageKind);
            break;
        default:
            nPageCount = 0;
    }

    if (bDirectionIsForward)
        switch (aLocation)
        {
            case CURRENT:
                nPageIndex = pViewShell->GetCurPageId() - 1;
                break;
            case BEGIN:
            default:
                nPageIndex = 0;
                break;
            case END:
                nPageIndex = nPageCount;
                break;
        }
    else
        switch (aLocation)
        {
            case CURRENT:
                nPageIndex = pViewShell->GetCurPageId() - 1;
                break;
            case BEGIN:
            default:
                nPageIndex = nPageCount-1;
                break;
            case END:
                nPageIndex = -1;
                break;
        }

    return nPageIndex;
}




//===== IteratorImplBase ====================================================

IteratorImplBase::IteratorImplBase(SdDrawDocument* pDocument, DrawViewShell* pViewShell, bool bDirectionIsForward)
:   maPosition()
,   mpDocument (pDocument)
,   mpViewShell (pViewShell)
,   mbDirectionIsForward (bDirectionIsForward)
{
    maPosition.mePageKind = pViewShell->GetPageKind();
    maPosition.meEditMode = pViewShell->GetEditMode();
}

IteratorImplBase::IteratorImplBase( SdDrawDocument* pDocument, DrawViewShell* pViewShell,bool bDirectionIsForward, PageKind ePageKind, EditMode eEditMode)
: maPosition()
, mpDocument (pDocument)
, mpViewShell (pViewShell)
, mbDirectionIsForward (bDirectionIsForward)
{
    maPosition.mePageKind = ePageKind;
    maPosition.meEditMode = eEditMode;
}

IteratorImplBase::~IteratorImplBase (void)
{}

bool IteratorImplBase::operator== (const IteratorImplBase& rIterator) const
{
    return maPosition == rIterator.maPosition;
}

bool IteratorImplBase::IsEqual (const IteratorImplBase& rIterator, IteratorType ) const
{
    // When this method is executed instead of the ones from derived classes
    // then the argument is of another type then the object itself.  In this
    // just compare the position objects.
    return maPosition == rIterator.maPosition;
}

const IteratorPosition& IteratorImplBase::GetPosition (void)
{
    return maPosition;
}




IteratorImplBase* IteratorImplBase::Clone (IteratorImplBase* pObject) const
{
    if (pObject != NULL)
    {
        pObject->maPosition = maPosition;
        pObject->mpDocument = mpDocument;
        pObject->mpViewShell = mpViewShell;
        pObject->mbDirectionIsForward = mbDirectionIsForward;
    }
    return pObject;
}



void IteratorImplBase::Reverse (void)
{
    mbDirectionIsForward = ! mbDirectionIsForward;
}



//===== SelectionIteratorImpl ===========================================

SelectionIteratorImpl::SelectionIteratorImpl (
    const ::std::vector<SdrObjectWeakRef>& rObjectList,
    sal_Int32 nObjectIndex,
    SdDrawDocument* pDocument,
    DrawViewShell* pViewShell,
    bool bDirectionIsForward)
    : IteratorImplBase (pDocument, pViewShell, bDirectionIsForward),
      mrObjectList(rObjectList),
      mnObjectIndex(nObjectIndex)
{}

SelectionIteratorImpl::~SelectionIteratorImpl (void)
{}

IteratorImplBase* SelectionIteratorImpl::Clone (IteratorImplBase* pObject) const
{
    SelectionIteratorImpl* pIterator = static_cast<SelectionIteratorImpl*>(pObject);
    if (pIterator == NULL)
        pIterator = new SelectionIteratorImpl (
            mrObjectList, mnObjectIndex, mpDocument, mpViewShell, mbDirectionIsForward);
    return pIterator;
}


void SelectionIteratorImpl::GotoNextText (void)
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


const IteratorPosition& SelectionIteratorImpl::GetPosition (void)
{
    maPosition.mxObject = mrObjectList.at(mnObjectIndex);

    return maPosition;
}


bool SelectionIteratorImpl::operator== (const IteratorImplBase& rIterator) const
{
    return rIterator.IsEqual (*this, SELECTION);
}


bool SelectionIteratorImpl::IsEqual (
    const IteratorImplBase& rIterator,
    IteratorType aType) const
{
    if (aType == SELECTION)
    {
        const SelectionIteratorImpl* pSelectionIterator =
            static_cast<const SelectionIteratorImpl*>(&rIterator);
        return mpDocument == pSelectionIterator->mpDocument
            && mnObjectIndex == pSelectionIterator->mnObjectIndex;
    }
    else
        return false;
}




//===== ViewIteratorImpl ================================================

ViewIteratorImpl::ViewIteratorImpl (
    sal_Int32 nPageIndex,
    SdDrawDocument* pDocument,
    DrawViewShell* pViewShell,
    bool bDirectionIsForward)
    : IteratorImplBase (pDocument, pViewShell, bDirectionIsForward),
      mbPageChangeOccured(false),
      mpPage(NULL),
      mpObjectIterator(NULL)
{
    SetPage (nPageIndex);
}




ViewIteratorImpl::ViewIteratorImpl (
    sal_Int32 nPageIndex,
    SdDrawDocument* pDocument,
    DrawViewShell* pViewShell,
    bool bDirectionIsForward,
    PageKind ePageKind,
    EditMode eEditMode)
    : IteratorImplBase (pDocument, pViewShell, bDirectionIsForward, ePageKind, eEditMode),
      mbPageChangeOccured(false),
      mpPage(NULL),
      mpObjectIterator(NULL)
{
    SetPage (nPageIndex);
}




ViewIteratorImpl::~ViewIteratorImpl (void)
{
}




IteratorImplBase* ViewIteratorImpl::Clone (IteratorImplBase* pObject) const
{

    ViewIteratorImpl* pIterator = static_cast<ViewIteratorImpl*>(pObject);
    if (pIterator == NULL)
        pIterator = new ViewIteratorImpl (
            maPosition.mnPageIndex, mpDocument, mpViewShell, mbDirectionIsForward);

    IteratorImplBase::Clone (pObject);

    if (mpObjectIterator != NULL)
    {
        pIterator->mpObjectIterator = new SdrObjListIter(*mpPage, IM_DEEPNOGROUPS, !mbDirectionIsForward);

        // No direct way to set the object iterator to the current object.
        pIterator->maPosition.mxObject.reset(NULL);
        while (pIterator->mpObjectIterator->IsMore() && pIterator->maPosition.mxObject!=maPosition.mxObject)
            pIterator->maPosition.mxObject.reset(pIterator->mpObjectIterator->Next());
    }
    else
        pIterator->mpObjectIterator = NULL;

    return pIterator;
}



void ViewIteratorImpl::GotoNextText(void)
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

    if (mpObjectIterator != NULL && mpObjectIterator->IsMore())
        maPosition.mxObject.reset(mpObjectIterator->Next());
    else
        maPosition.mxObject.reset(NULL);

    if (!maPosition.mxObject.is() )
    {
        if (mbDirectionIsForward)
            SetPage (maPosition.mnPageIndex+1);
        else
            SetPage (maPosition.mnPageIndex-1);

        if (mpPage != NULL)
            mpObjectIterator = new SdrObjListIter(*mpPage, IM_DEEPNOGROUPS, !mbDirectionIsForward);
        if (mpObjectIterator!=NULL && mpObjectIterator->IsMore())
            maPosition.mxObject.reset(mpObjectIterator->Next());
        else
            maPosition.mxObject.reset(NULL);
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
    mbPageChangeOccured = (maPosition.mnPageIndex!=nPageIndex);
    if (mbPageChangeOccured)
    {
        maPosition.mnPageIndex = nPageIndex;

        sal_Int32 nPageCount;
        if (maPosition.meEditMode == EM_PAGE)
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
            if (maPosition.meEditMode == EM_PAGE)
                mpPage = mpDocument->GetSdPage (
                    (USHORT)nPageIndex,
                    maPosition.mePageKind);
            else
                mpPage = mpDocument->GetMasterSdPage (
                    (USHORT)nPageIndex,
                    maPosition.mePageKind);
        }
        else
            mpPage = NULL;
    }

    // Set up object list iterator.
    if (mpPage != NULL)
        mpObjectIterator = new SdrObjListIter(*mpPage, IM_DEEPNOGROUPS, ! mbDirectionIsForward);
    else
        mpObjectIterator = NULL;

    // Get object pointer.
    if (mpObjectIterator!=NULL && mpObjectIterator->IsMore())
        maPosition.mxObject.reset( mpObjectIterator->Next() );
    else
        maPosition.mxObject.reset( NULL );

    maPosition.mnText = 0;
    if( !mbDirectionIsForward && maPosition.mxObject.is() )
    {
        SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >( maPosition.mxObject.get() );
        if( pTextObj )
            maPosition.mnText = pTextObj->getTextCount() - 1;
    }

}




void ViewIteratorImpl::Reverse (void)
{
    IteratorImplBase::Reverse ();

    // Create reversed object list iterator.
    if (mpObjectIterator != NULL)
        delete mpObjectIterator;
    if (mpPage != NULL)
        mpObjectIterator = new SdrObjListIter(*mpPage, IM_DEEPNOGROUPS, ! mbDirectionIsForward);
    else
        mpObjectIterator = NULL;

    // Move iterator to the current object.
    SdrObjectWeakRef xObject = maPosition.mxObject;
    maPosition.mxObject.reset(NULL);
    while (mpObjectIterator->IsMore() && maPosition.mxObject != xObject)
        maPosition.mxObject.reset(mpObjectIterator->Next());
}




//===== DocumentIteratorImpl ============================================

DocumentIteratorImpl::DocumentIteratorImpl (
    sal_Int32 nPageIndex,
    PageKind ePageKind, EditMode eEditMode,
    SdDrawDocument* pDocument, DrawViewShell* pViewShell,
    bool bDirectionIsForward)
    : ViewIteratorImpl (nPageIndex, pDocument, pViewShell, bDirectionIsForward,
        ePageKind, eEditMode)
{
    if (eEditMode == EM_PAGE)
        mnPageCount = pDocument->GetSdPageCount (ePageKind);
    else
        mnPageCount = pDocument->GetMasterSdPageCount(ePageKind);
}




DocumentIteratorImpl::~DocumentIteratorImpl (void)
{}




IteratorImplBase* DocumentIteratorImpl::Clone (IteratorImplBase* pObject) const
{
    DocumentIteratorImpl* pIterator = static_cast<DocumentIteratorImpl*>(pObject);
    if (pIterator == NULL)
        pIterator = new DocumentIteratorImpl (
            maPosition.mnPageIndex, maPosition.mePageKind, maPosition.meEditMode,
            mpDocument, mpViewShell, mbDirectionIsForward);
    // Finish the cloning.
    return ViewIteratorImpl::Clone (pIterator);
}




void DocumentIteratorImpl::GotoNextText (void)
{
    bool bSetToOnePastLastPage = false;
    bool bViewChanged = false;

    ViewIteratorImpl::GotoNextText();

    if (mbDirectionIsForward)
    {
        if (maPosition.mnPageIndex >= mnPageCount)
        {
            // Switch to master page.
            if (maPosition.meEditMode == EM_PAGE)
            {
                maPosition.meEditMode = EM_MASTERPAGE;
                SetPage (0);
            }

            // Switch to next view mode.
            else
            {
                if (maPosition.mePageKind == PK_HANDOUT)
                    // Not really necessary but makes things more clear.
                    bSetToOnePastLastPage = true;
                else
                {
                    maPosition.meEditMode = EM_PAGE;
                    if (maPosition.mePageKind == PK_STANDARD)
                        maPosition.mePageKind = PK_NOTES;
                    else if (maPosition.mePageKind == PK_NOTES)
                        maPosition.mePageKind = PK_HANDOUT;
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
            if (maPosition.meEditMode == EM_MASTERPAGE)
            {
                maPosition.meEditMode = EM_PAGE;
                bSetToOnePastLastPage = true;
            }

            // Switch to previous view mode.
            else
            {
                if (maPosition.mePageKind == PK_STANDARD)
                    SetPage (-1);
                else
                {
                    maPosition.meEditMode = EM_MASTERPAGE;
                    if (maPosition.mePageKind == PK_HANDOUT)
                        maPosition.mePageKind = PK_NOTES;
                    else if (maPosition.mePageKind == PK_NOTES)
                        maPosition.mePageKind = PK_STANDARD;
                    bSetToOnePastLastPage = true;
                }
            }
            bViewChanged = true;
        }

    if (bViewChanged)
    {
        // Get new page count;
        sal_Int32 nPageCount;
        if (maPosition.meEditMode == EM_PAGE)
            nPageCount = mpDocument->GetSdPageCount (maPosition.mePageKind);
        else
            nPageCount = mpDocument->GetMasterSdPageCount(maPosition.mePageKind);

        // Now that we know the number of pages we can set the current page index.
        if (bSetToOnePastLastPage)
            SetPage (nPageCount);
    }
}


} } // end of namespace ::sd::outliner
