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

#include "MasterPageDescriptor.hxx"

#include "DocumentHelper.hxx"
#include "sdpage.hxx"
#include <tools/urlobj.hxx>

namespace sd { namespace sidebar {

//===== MasterPageDescriptor ==================================================

MasterPageDescriptor::MasterPageDescriptor (
    MasterPageContainer::Origin eOrigin,
    const sal_Int32 nTemplateIndex,
    const OUString& rsURL,
    const OUString& rsPageName,
    const OUString& rsStyleName,
    const bool bIsPrecious,
    const std::shared_ptr<PageObjectProvider>& rpPageObjectProvider,
    const std::shared_ptr<PreviewProvider>& rpPreviewProvider)
    : maToken(MasterPageContainer::NIL_TOKEN),
      meOrigin(eOrigin),
      msURL(INetURLObject(rsURL).GetMainURL(INetURLObject::DECODE_UNAMBIGUOUS)),
      msPageName(rsPageName),
      msStyleName(rsStyleName),
      mbIsPrecious(bIsPrecious),
      mpMasterPage(nullptr),
      mpSlide(nullptr),
      maSmallPreview(),
      maLargePreview(),
      mpPreviewProvider(rpPreviewProvider),
      mpPageObjectProvider(rpPageObjectProvider),
      mnTemplateIndex(nTemplateIndex),
      meURLClassification(URLCLASS_UNDETERMINED),
      mnUseCount(0)
{
}

MasterPageDescriptor::MasterPageDescriptor (const MasterPageDescriptor& rDescriptor)
    : maToken(rDescriptor.maToken),
      meOrigin(rDescriptor.meOrigin),
      msURL(rDescriptor.msURL),
      msPageName(rDescriptor.msPageName),
      msStyleName(rDescriptor.msStyleName),
      mbIsPrecious(rDescriptor.mbIsPrecious),
      mpMasterPage(rDescriptor.mpMasterPage),
      mpSlide(rDescriptor.mpSlide),
      maSmallPreview(rDescriptor.maSmallPreview),
      maLargePreview(rDescriptor.maLargePreview),
      mpPreviewProvider(rDescriptor.mpPreviewProvider),
      mpPageObjectProvider(rDescriptor.mpPageObjectProvider),
      mnTemplateIndex(rDescriptor.mnTemplateIndex),
      meURLClassification(rDescriptor.meURLClassification),
      mnUseCount(rDescriptor.mnUseCount)
{
}

MasterPageDescriptor::~MasterPageDescriptor()
{
}

void MasterPageDescriptor::SetToken (MasterPageContainer::Token aToken)
{
    maToken = aToken;
}

const Image& MasterPageDescriptor::GetPreview (MasterPageContainer::PreviewSize eSize) const
{
    if (eSize == MasterPageContainer::SMALL)
        return maSmallPreview;
    else
        return maLargePreview;
}

::std::unique_ptr<std::vector<MasterPageContainerChangeEvent::EventType> >
    MasterPageDescriptor::Update (
        const MasterPageDescriptor& rDescriptor)
{
    bool bDataChanged (false);
    bool bIndexChanged (false);
    bool bPreviewChanged (false);

    if (meOrigin==MasterPageContainer::UNKNOWN
        && rDescriptor.meOrigin!=MasterPageContainer::UNKNOWN)
    {
        meOrigin = rDescriptor.meOrigin;
        bIndexChanged = true;
    }

    if (msURL.isEmpty() && !rDescriptor.msURL.isEmpty())
    {
        msURL = rDescriptor.msURL;
        bDataChanged = true;
    }

    if (msPageName.isEmpty() && !rDescriptor.msPageName.isEmpty())
    {
        msPageName = rDescriptor.msPageName;
        bDataChanged = true;
    }

    if (msStyleName.isEmpty() && !rDescriptor.msStyleName.isEmpty())
    {
        msStyleName = rDescriptor.msStyleName;
        bDataChanged = true;
    }

    if (mpPageObjectProvider.get()==nullptr && rDescriptor.mpPageObjectProvider.get()!=nullptr)
    {
        mpPageObjectProvider = rDescriptor.mpPageObjectProvider;
        bDataChanged = true;
    }

     if (mpPreviewProvider.get()==nullptr && rDescriptor.mpPreviewProvider.get()!=nullptr)
     {
         mpPreviewProvider = rDescriptor.mpPreviewProvider;
         bPreviewChanged = true;
     }

     if (mnTemplateIndex<0 && rDescriptor.mnTemplateIndex>=0)
     {
         mnTemplateIndex = rDescriptor.mnTemplateIndex;
         bIndexChanged = true;
     }

     // Prepare the list of event types that will be returned.
     ::std::unique_ptr<std::vector<MasterPageContainerChangeEvent::EventType> > pResult;
     if (bDataChanged || bIndexChanged || bPreviewChanged)
     {
         pResult.reset(new std::vector<MasterPageContainerChangeEvent::EventType>());
         if (bDataChanged)
             pResult->push_back(MasterPageContainerChangeEvent::DATA_CHANGED);
         if (bIndexChanged)
             pResult->push_back(MasterPageContainerChangeEvent::INDEX_CHANGED);
         if (bPreviewChanged)
             pResult->push_back(MasterPageContainerChangeEvent::PREVIEW_CHANGED);
     }

     return pResult;
}

int MasterPageDescriptor::UpdatePageObject (
    sal_Int32 nCostThreshold,
    SdDrawDocument* pDocument)
{
    int nModified = 0;

    // Update the page object when that is not yet known.
    if (mpMasterPage == nullptr
        && mpPageObjectProvider.get()!=nullptr
        && (nCostThreshold<0 || mpPageObjectProvider->GetCostIndex()<=nCostThreshold))
    {
        // Note that pDocument may be NULL.

        SdPage* pPage = (*mpPageObjectProvider)(pDocument);
        if (meOrigin == MasterPageContainer::MASTERPAGE)
        {
            mpMasterPage = pPage;
            if (mpMasterPage != nullptr)
                mpMasterPage->SetPrecious(mbIsPrecious);
        }
        else
        {
            // Master pages from templates are copied into the local document.
            if (pDocument != nullptr)
                mpMasterPage = DocumentHelper::CopyMasterPageToLocalDocument(*pDocument,pPage);
            mpSlide = DocumentHelper::GetSlideForMasterPage(mpMasterPage);
        }

        if (mpMasterPage != nullptr)
        {
            // Update page name and style name.
            if (msPageName.isEmpty())
                msPageName = mpMasterPage->GetName();
            msStyleName = mpMasterPage->GetName();

            // Delete an existing substitution. The next request for a preview
            // will create the real one.
            maSmallPreview = Image();
            maLargePreview = Image();
            mpPreviewProvider = std::shared_ptr<PreviewProvider>(new PagePreviewProvider());
        }
        else
        {
            DBG_ASSERT(false, "UpdatePageObject: master page is NULL");
            return -1;
        }

        nModified = 1;
    }

    return nModified;
}

bool MasterPageDescriptor::UpdatePreview (
    sal_Int32 nCostThreshold,
    const Size& rSmallSize,
    const Size& rLargeSize,
    ::sd::PreviewRenderer& rRenderer)
{
    bool bModified (false);

    // Update the preview when that is not yet known.
    if (maLargePreview.GetSizePixel().Width()==0
        && mpPreviewProvider.get()!=nullptr
        && (nCostThreshold<0 || mpPreviewProvider->GetCostIndex()<=nCostThreshold))
    {
        SdPage* pPage = mpSlide;
        if (pPage == nullptr)
        {
            pPage = mpMasterPage;
        }
        maLargePreview = (*mpPreviewProvider)(
            rLargeSize.Width(),
            pPage,
            rRenderer);
        if (maLargePreview.GetSizePixel().Width() > 0)
        {
            // Create the small preview by scaling the large one down.
            maSmallPreview = rRenderer.ScaleBitmap(
                maLargePreview.GetBitmapEx(),
                rSmallSize.Width());
            // The large preview may not have the desired width.  Scale it
            // accordingly.
            if (maLargePreview.GetSizePixel().Width() != rLargeSize.Width())
                maLargePreview = rRenderer.ScaleBitmap(
                    maLargePreview.GetBitmapEx(),
                    rLargeSize.Width());
            bModified = true;
        }
    }

    return bModified;
}

MasterPageDescriptor::URLClassification MasterPageDescriptor::GetURLClassification()
{
    if (meURLClassification == URLCLASS_UNDETERMINED)
    {
        if (msURL.isEmpty())
            meURLClassification = URLCLASS_UNKNOWN;
        else if (msURL.indexOf("presnt")>=0)
        {
            meURLClassification = URLCLASS_PRESENTATION;
        }
        else if (msURL.indexOf("layout")>=0)
        {
            meURLClassification = URLCLASS_LAYOUT;
        }
        else if (msURL.indexOf("educate")>=0)
        {
            meURLClassification = URLCLASS_OTHER;
        }
        else
        {
            meURLClassification = URLCLASS_USER;
        }
    }

    return meURLClassification;
}

//===== URLComparator =========================================================

MasterPageDescriptor::URLComparator::URLComparator (const OUString& sURL)
    : msURL(sURL)
{
}

bool MasterPageDescriptor::URLComparator::operator() (
    const SharedMasterPageDescriptor& rDescriptor)
{
    if (rDescriptor.get() == nullptr)
        return false;
    else
        return rDescriptor->msURL.equals(msURL);
}

// ===== StyleNameComparator ==================================================

MasterPageDescriptor::StyleNameComparator::StyleNameComparator (const OUString& sStyleName)
    : msStyleName(sStyleName)
{
}

bool MasterPageDescriptor::StyleNameComparator::operator() (
    const SharedMasterPageDescriptor& rDescriptor)
{
    if (rDescriptor.get() == nullptr)
        return false;
    else
        return rDescriptor->msStyleName.equals(msStyleName);
}

//===== PageObjectComparator ==================================================

MasterPageDescriptor::PageObjectComparator::PageObjectComparator (const SdPage* pPageObject)
    : mpMasterPage(pPageObject)
{
}

bool MasterPageDescriptor::PageObjectComparator::operator() (
    const SharedMasterPageDescriptor& rDescriptor)
{
    if (rDescriptor.get() == nullptr)
        return false;
    else
        return rDescriptor->mpMasterPage==mpMasterPage;
}

//===== AllComparator =========================================================

MasterPageDescriptor::AllComparator::AllComparator(const SharedMasterPageDescriptor& rDescriptor)
    : mpDescriptor(rDescriptor)
{
}

bool MasterPageDescriptor::AllComparator::operator() (const SharedMasterPageDescriptor&rDescriptor)
{
    if (rDescriptor.get() == nullptr)
        return false;
    else
    {
        // Take URL, page name, style name, and page object into account
        // when comparing two descriptors.  When two descriptors are
        // identical in any of these values then there are thought of as
        // equivalent.  Only the Origin has to be the same in both
        // descriptors.
        return
            mpDescriptor->meOrigin == rDescriptor->meOrigin
            && (
                (!mpDescriptor->msURL.isEmpty()
                    && mpDescriptor->msURL.equals(rDescriptor->msURL))
                || (!mpDescriptor->msPageName.isEmpty()
                    && mpDescriptor->msPageName.equals(rDescriptor->msPageName))
                || (!mpDescriptor->msStyleName.isEmpty()
                    && mpDescriptor->msStyleName.equals(rDescriptor->msStyleName))
                || (mpDescriptor->mpMasterPage!=nullptr
                    && mpDescriptor->mpMasterPage==rDescriptor->mpMasterPage)
                || (mpDescriptor->mpPageObjectProvider.get()!=nullptr
                    && rDescriptor->mpPageObjectProvider.get()!=nullptr
                    && mpDescriptor->mpPageObjectProvider==rDescriptor->mpPageObjectProvider));
    }
}

} } // end of namespace sd::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
