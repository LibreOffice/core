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

#include <cppuhelper/supportsservice.hxx>

#include <DrawController.hxx>
#include <SdUnoSlideView.hxx>

#include <SlideSorter.hxx>
#include <controller/SlideSorterController.hxx>
#include <controller/SlsPageSelector.hxx>
#include <controller/SlsCurrentSlideManager.hxx>
#include <model/SlsPageEnumerationProvider.hxx>
#include <model/SlsPageDescriptor.hxx>
#include <sdpage.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sd {

SdUnoSlideView::SdUnoSlideView (
    slidesorter::SlideSorter& rSlideSorter) noexcept
    : mrSlideSorter(rSlideSorter)
{
}

SdUnoSlideView::~SdUnoSlideView() noexcept
{
}

//----- XSelectionSupplier ----------------------------------------------------

sal_Bool SAL_CALL SdUnoSlideView::select (const Any& aSelection)
{
    slidesorter::controller::SlideSorterController& rSlideSorterController
        = mrSlideSorter.GetController();
    slidesorter::controller::PageSelector& rSelector (rSlideSorterController.GetPageSelector());
    rSelector.DeselectAllPages();
    Sequence<Reference<drawing::XDrawPage> > xPages;
    aSelection >>= xPages;
    for (const auto& rPage : xPages)
    {
        Reference<beans::XPropertySet> xSet (rPage, UNO_QUERY);
        if (xSet.is())
        {
            try
            {
                Any aNumber = xSet->getPropertyValue(u"Number"_ustr);
                sal_Int32 nPageNumber = 0;
                aNumber >>= nPageNumber;
                nPageNumber -=1; // Transform 1-based page numbers to 0-based ones.
                rSelector.SelectPage(nPageNumber);
            }
            catch (const RuntimeException&)
            {
            }
        }
    }

    return true;
}

Any SAL_CALL SdUnoSlideView::getSelection()
{
    Any aResult;

    slidesorter::model::PageEnumeration aSelectedPages (
        slidesorter::model::PageEnumerationProvider::CreateSelectedPagesEnumeration(
            mrSlideSorter.GetModel()));
    int nSelectedPageCount (
        mrSlideSorter.GetController().GetPageSelector().GetSelectedPageCount());

    Sequence<Reference<XInterface> > aPages(nSelectedPageCount);
    auto aPagesRange = asNonConstRange(aPages);
    int nIndex = 0;
    while (aSelectedPages.HasMoreElements() && nIndex<nSelectedPageCount)
    {
        slidesorter::model::SharedPageDescriptor pDescriptor (aSelectedPages.GetNextElement());
        aPagesRange[nIndex++] = pDescriptor->GetPage()->getUnoPage();
    }
    aResult <<= aPages;

    return aResult;
}

void SAL_CALL SdUnoSlideView::addSelectionChangeListener (
    const css::uno::Reference<css::view::XSelectionChangeListener>&)
{}

void SAL_CALL SdUnoSlideView::removeSelectionChangeListener (
    const css::uno::Reference<css::view::XSelectionChangeListener>&)
{}

//----- XDrawView -------------------------------------------------------------

void SAL_CALL SdUnoSlideView::setCurrentPage (
    const css::uno::Reference<css::drawing::XDrawPage>& rxDrawPage)
{
    Reference<beans::XPropertySet> xProperties (rxDrawPage, UNO_QUERY);
    if (xProperties.is())
    {
        sal_uInt16 nPageNumber(0);
        if (xProperties->getPropertyValue(u"Number"_ustr) >>= nPageNumber)
        {
            mrSlideSorter.GetController().GetCurrentSlideManager()->SwitchCurrentSlide(
                nPageNumber-1);
        }
    }
}

css::uno::Reference<css::drawing::XDrawPage > SAL_CALL
    SdUnoSlideView::getCurrentPage()
{
    return mrSlideSorter.GetController().GetCurrentSlideManager()->GetCurrentSlide()->GetXDrawPage();
}

//----- XFastPropertySet ------------------------------------------------------

void SdUnoSlideView::setFastPropertyValue (
    sal_Int32 nHandle,
        const Any&)
{
    throw beans::UnknownPropertyException( OUString::number(nHandle), static_cast<cppu::OWeakObject*>(this));
}

Any SAL_CALL SdUnoSlideView::getFastPropertyValue (
    sal_Int32 nHandle)
{
    if( nHandle != DrawController::PROPERTY_VIEWOFFSET )
        throw beans::UnknownPropertyException( OUString::number(nHandle), static_cast<cppu::OWeakObject*>(this));

    return Any();
}

// XServiceInfo
OUString SAL_CALL SdUnoSlideView::getImplementationName(  )
{
    return u"com.sun.star.comp.sd.SdUnoSlideView"_ustr;
}

sal_Bool SAL_CALL SdUnoSlideView::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

Sequence< OUString > SAL_CALL SdUnoSlideView::getSupportedServiceNames(  )
{
    return { u"com.sun.star.presentation.SlidesView"_ustr };
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
