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
#include "MasterPageDescriptor.hxx"
#include "MasterPageContainerProviders.hxx"

#include "DocumentHelper.hxx"
#include <PreviewRenderer.hxx>
#include <sdpage.hxx>
#include <tools/urlobj.hxx>
#include <sal/log.hxx>
#include <utility>

namespace sd::sidebar {

//===== MasterPageDescriptor ==================================================

MasterPageDescriptor::MasterPageDescriptor (
    MasterPageContainer::Origin eOrigin,
    const sal_Int32 nTemplateIndex,
    std::u16string_view rsURL,
    OUString sPageName,
    OUString sStyleName,
    const bool bIsPrecious,
    std::shared_ptr<PageObjectProvider> pPageObjectProvider,
    std::shared_ptr<PreviewProvider> pPreviewProvider)
    : maToken(MasterPageContainer::NIL_TOKEN),
      meOrigin(eOrigin),
      msURL(INetURLObject(rsURL).GetMainURL(INetURLObject::DecodeMechanism::Unambiguous)),
      msPageName(std::move(sPageName)),
      msStyleName(std::move(sStyleName)),
      mbIsPrecious(bIsPrecious),
      mpMasterPage(nullptr),
      mpSlide(nullptr),
      mpPreviewProvider(std::move(pPreviewProvider)),
      mpPageObjectProvider(std::move(pPageObjectProvider)),
      mnTemplateIndex(nTemplateIndex),
      meURLClassification(URLCLASS_UNDETERMINED),
      mnUseCount(0)
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

    if (mpPageObjectProvider == nullptr && rDescriptor.mpPageObjectProvider != nullptr)
    {
        mpPageObjectProvider = rDescriptor.mpPageObjectProvider;
        bDataChanged = true;
    }

    if (mpPreviewProvider == nullptr && rDescriptor.mpPreviewProvider != nullptr)
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
         pResult.reset(new std::vector<MasterPageContainerChangeEvent::EventType>);
         if (bDataChanged)
             pResult->push_back(MasterPageContainerChangeEvent::EventType::DATA_CHANGED);
         if (bIndexChanged)
             pResult->push_back(MasterPageContainerChangeEvent::EventType::INDEX_CHANGED);
         if (bPreviewChanged)
             pResult->push_back(MasterPageContainerChangeEvent::EventType::PREVIEW_CHANGED);
    }

    return pResult;
}

int MasterPageDescriptor::UpdatePageObject (
    sal_Int32 nCostThreshold,
    SdDrawDocument* pDocument)
{
    int nModified = 0;

    // Update the page object when that is not yet known.
    if (mpMasterPage == nullptr && mpPageObjectProvider != nullptr
        && (nCostThreshold < 0 || mpPageObjectProvider->GetCostIndex() <= nCostThreshold))
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
            mpPreviewProvider = std::make_shared<PagePreviewProvider>();
        }
        else
        {
            SAL_WARN( "sd", "UpdatePageObject: master page is NULL");
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
    if (maLargePreview.GetSizePixel().Width() == 0 && mpPreviewProvider != nullptr
        && (nCostThreshold < 0 || mpPreviewProvider->GetCostIndex() <= nCostThreshold))
    {
        SdPage* pPage = mpSlide;
        if (pPage == nullptr)
        {
            pPage = mpMasterPage;
        }
        //TODO: Notify LOOL of preview updates.
        maLargePreview = (*mpPreviewProvider)(
            rLargeSize.Width(),
            pPage,
            rRenderer);
        if (maLargePreview.GetSizePixel().Width() > 0)
        {
            // Create the small preview by scaling the large one down.
            maSmallPreview = rRenderer.ScaleBitmap(
                BitmapEx(maLargePreview.GetBitmap()),
                rSmallSize.Width());
            // The large preview may not have the desired width.  Scale it
            // accordingly.
            if (maLargePreview.GetSizePixel().Width() != rLargeSize.Width())
                maLargePreview = rRenderer.ScaleBitmap(
                    BitmapEx(maLargePreview.GetBitmap()),
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

MasterPageDescriptor::URLComparator::URLComparator (OUString sURL)
    : msURL(std::move(sURL))
{
}

bool MasterPageDescriptor::URLComparator::operator() (
    const SharedMasterPageDescriptor& rDescriptor)
{
    if (!rDescriptor)
        return false;
    else
        return rDescriptor->msURL == msURL;
}

// ===== StyleNameComparator ==================================================

MasterPageDescriptor::StyleNameComparator::StyleNameComparator (OUString sStyleName)
    : msStyleName(std::move(sStyleName))
{
}

bool MasterPageDescriptor::StyleNameComparator::operator() (
    const SharedMasterPageDescriptor& rDescriptor)
{
    if (!rDescriptor)
        return false;
    else
        return rDescriptor->msStyleName == msStyleName;
}

//===== PageObjectComparator ==================================================

MasterPageDescriptor::PageObjectComparator::PageObjectComparator (const SdPage* pPageObject)
    : mpMasterPage(pPageObject)
{
}

bool MasterPageDescriptor::PageObjectComparator::operator() (
    const SharedMasterPageDescriptor& rDescriptor)
{
    if (!rDescriptor)
        return false;
    else
        return rDescriptor->mpMasterPage==mpMasterPage;
}

//===== AllComparator =========================================================

MasterPageDescriptor::AllComparator::AllComparator(SharedMasterPageDescriptor aDescriptor)
    : mpDescriptor(std::move(aDescriptor))
{
}

bool MasterPageDescriptor::AllComparator::operator() (const SharedMasterPageDescriptor&rDescriptor)
{
    if (!rDescriptor)
        return false;
    else
    {
        // Take URL, page name, style name, and page object into account
        // when comparing two descriptors.  When two descriptors are
        // identical in any of these values then there are thought of as
        // equivalent.  Only the Origin has to be the same in both
        // descriptors.
        return mpDescriptor->meOrigin == rDescriptor->meOrigin
               && ((!mpDescriptor->msURL.isEmpty() && mpDescriptor->msURL == rDescriptor->msURL)
                   || (!mpDescriptor->msPageName.isEmpty()
                       && mpDescriptor->msPageName == rDescriptor->msPageName)
                   || (!mpDescriptor->msStyleName.isEmpty()
                       && mpDescriptor->msStyleName == rDescriptor->msStyleName)
                   || (mpDescriptor->mpMasterPage != nullptr
                       && mpDescriptor->mpMasterPage == rDescriptor->mpMasterPage)
                   || (mpDescriptor->mpPageObjectProvider != nullptr
                       && rDescriptor->mpPageObjectProvider != nullptr
                       && mpDescriptor->mpPageObjectProvider == rDescriptor->mpPageObjectProvider));
    }
}

} // end of namespace sd::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
