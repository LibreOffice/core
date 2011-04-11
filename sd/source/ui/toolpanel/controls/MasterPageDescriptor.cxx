/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "MasterPageDescriptor.hxx"

#include "DocumentHelper.hxx"
#include "sdpage.hxx"
#include <tools/urlobj.hxx>

namespace sd { namespace toolpanel { namespace controls {


//===== MasterPageDescriptor ==================================================

MasterPageDescriptor::MasterPageDescriptor (
    MasterPageContainer::Origin eOrigin,
    const sal_Int32 nTemplateIndex,
    const String& rsURL,
    const String& rsPageName,
    const String& rsStyleName,
    const bool bIsPrecious,
    const ::boost::shared_ptr<PageObjectProvider>& rpPageObjectProvider,
    const ::boost::shared_ptr<PreviewProvider>& rpPreviewProvider)
    : maToken(MasterPageContainer::NIL_TOKEN),
      meOrigin(eOrigin),
      msURL(INetURLObject(rsURL).GetMainURL(INetURLObject::DECODE_UNAMBIGUOUS)),
      msPageName(rsPageName),
      msStyleName(rsStyleName),
      mbIsPrecious(bIsPrecious),
      mpMasterPage(NULL),
      mpSlide(NULL),
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




MasterPageDescriptor::~MasterPageDescriptor (void)
{
}




void MasterPageDescriptor::SetToken (MasterPageContainer::Token aToken)
{
    maToken = aToken;
}




Image MasterPageDescriptor::GetPreview (MasterPageContainer::PreviewSize eSize) const
{
    if (eSize == MasterPageContainer::SMALL)
        return maSmallPreview;
    else
        return maLargePreview;
}




::std::auto_ptr<std::vector<MasterPageContainerChangeEvent::EventType> >
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

    if (msURL.getLength()==0 && rDescriptor.msURL.getLength()!=0)
    {
        msURL = rDescriptor.msURL;
        bDataChanged = true;
    }

    if (msPageName.getLength()==0 && rDescriptor.msPageName.getLength()!=0)
    {
        msPageName = rDescriptor.msPageName;
        bDataChanged = true;
    }

    if (msStyleName.getLength()==0 && rDescriptor.msStyleName.getLength()!=0)
    {
        msStyleName = rDescriptor.msStyleName;
        bDataChanged = true;
    }

    if (mpPageObjectProvider.get()==NULL && rDescriptor.mpPageObjectProvider.get()!=NULL)
    {
        mpPageObjectProvider = rDescriptor.mpPageObjectProvider;
        bDataChanged = true;
    }

     if (mpPreviewProvider.get()==NULL && rDescriptor.mpPreviewProvider.get()!=NULL)
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
     ::std::auto_ptr<std::vector<MasterPageContainerChangeEvent::EventType> > pResult;
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




bool MasterPageDescriptor::UpdatePageObject (
    sal_Int32 nCostThreshold,
    SdDrawDocument* pDocument)
{
    bool bModified (false);

    // Update the page object when that is not yet known.
    if (mpMasterPage == NULL
        && mpPageObjectProvider.get()!=NULL
        && (nCostThreshold<0 || mpPageObjectProvider->GetCostIndex()<=nCostThreshold))
    {
        // Note that pDocument may be NULL.

        SdPage* pPage = (*mpPageObjectProvider)(pDocument);
        if (meOrigin == MasterPageContainer::MASTERPAGE)
        {
            mpMasterPage = pPage;
            if (mpMasterPage != NULL)
                mpMasterPage->SetPrecious(mbIsPrecious);
        }
        else
        {
            // Master pages from templates are copied into the local document.
            if (pDocument != NULL)
                mpMasterPage = DocumentHelper::CopyMasterPageToLocalDocument(*pDocument,pPage);
            mpSlide = DocumentHelper::GetSlideForMasterPage(mpMasterPage);
        }

        if (mpMasterPage != NULL)
        {
            // Update page name and style name.
            if (msPageName.getLength() == 0)
                msPageName = mpMasterPage->GetName();
            msStyleName = mpMasterPage->GetName();

            // Delete an existing substitution. The next request for a preview
            // will create the real one.
            maSmallPreview = Image();
            maLargePreview = Image();
            mpPreviewProvider = ::boost::shared_ptr<PreviewProvider>(new PagePreviewProvider());
        }
        else
        {
            DBG_ASSERT(false, "UpdatePageObject: master page is NULL");
        }

        bModified = true;
    }

    return bModified;
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
        && mpPreviewProvider.get()!=NULL
        && (nCostThreshold<0 || mpPreviewProvider->GetCostIndex()<=nCostThreshold))
    {
        SdPage* pPage = mpSlide;
        if (pPage == NULL)
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
            // accrodingly.
            if (maLargePreview.GetSizePixel().Width() != rLargeSize.Width())
                maLargePreview = rRenderer.ScaleBitmap(
                    maLargePreview.GetBitmapEx(),
                    rLargeSize.Width());
            bModified = true;
        }
    }

    return bModified;
}




MasterPageDescriptor::URLClassification MasterPageDescriptor::GetURLClassification (void)
{
    if (meURLClassification == URLCLASS_UNDETERMINED)
    {
        if (msURL.getLength() == 0)
            meURLClassification = URLCLASS_UNKNOWN;
        else if (msURL.indexOf(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("presnt")))>=0)
        {
            meURLClassification = URLCLASS_PRESENTATION;
        }
        else if (msURL.indexOf(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("layout")))>=0)
        {
            meURLClassification = URLCLASS_LAYOUT;
        }
        else if (msURL.indexOf(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("educate")))>=0)
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

MasterPageDescriptor::URLComparator::URLComparator (const ::rtl::OUString& sURL)
    : msURL(sURL)
{
}




bool MasterPageDescriptor::URLComparator::operator() (
    const SharedMasterPageDescriptor& rDescriptor)
{
    if (rDescriptor.get() == NULL)
        return false;
    else
        return rDescriptor->msURL.equals(msURL);
}




// ===== StyleNameComparator ==================================================

MasterPageDescriptor::StyleNameComparator::StyleNameComparator (const ::rtl::OUString& sStyleName)
    : msStyleName(sStyleName)
{
}




bool MasterPageDescriptor::StyleNameComparator::operator() (
    const SharedMasterPageDescriptor& rDescriptor)
{
    if (rDescriptor.get() == NULL)
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
    if (rDescriptor.get() == NULL)
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
    if (rDescriptor.get() == NULL)
        return false;
    else
    {
        // Take URL, page name, style name, and page object into account
        // when comparing two descriptors.  When two descriptors are
        // identical in any of these values then their are thought of as
        // equivalent.  Only the Origin has to be the same in both
        // descriptors.
        return
            mpDescriptor->meOrigin == rDescriptor->meOrigin
            && (
                (mpDescriptor->msURL.getLength()>0
                    && mpDescriptor->msURL.equals(rDescriptor->msURL))
                || (mpDescriptor->msPageName.getLength()>0
                    && mpDescriptor->msPageName.equals(rDescriptor->msPageName))
                || (mpDescriptor->msStyleName.getLength()>0
                    && mpDescriptor->msStyleName.equals(rDescriptor->msStyleName))
                || (mpDescriptor->mpMasterPage!=NULL
                    && mpDescriptor->mpMasterPage==rDescriptor->mpMasterPage)
                || (mpDescriptor->mpPageObjectProvider.get()!=NULL
                    && rDescriptor->mpPageObjectProvider.get()!=NULL
                    && mpDescriptor->mpPageObjectProvider==rDescriptor->mpPageObjectProvider));
    }
}


} } } // end of namespace ::sd::toolpanel::controls

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
