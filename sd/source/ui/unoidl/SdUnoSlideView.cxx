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


#include <comphelper/serviceinfohelper.hxx>

#include "DrawController.hxx"
#include "SdUnoSlideView.hxx"

#include "SlideSorter.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsPageSelector.hxx"
#include "controller/SlsCurrentSlideManager.hxx"
#include "model/SlsPageEnumerationProvider.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "sdpage.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>

using ::rtl::OUString;

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sd {


SdUnoSlideView::SdUnoSlideView (
    slidesorter::SlideSorter& rSlideSorter) throw()
    : DrawSubControllerInterfaceBase(m_aMutex),
      mrSlideSorter(rSlideSorter)
{
}




SdUnoSlideView::~SdUnoSlideView (void) throw()
{
}




//----- XSelectionSupplier ----------------------------------------------------

sal_Bool SAL_CALL SdUnoSlideView::select (const Any& aSelection)
      throw(lang::IllegalArgumentException, RuntimeException)
{
    bool bOk = true;

    slidesorter::controller::SlideSorterController& rSlideSorterController
        = mrSlideSorter.GetController();
    slidesorter::controller::PageSelector& rSelector (rSlideSorterController.GetPageSelector());
    rSelector.DeselectAllPages();
    Sequence<Reference<drawing::XDrawPage> > xPages;
    aSelection >>= xPages;
    const sal_uInt32 nCount = xPages.getLength();
    for (sal_uInt32 nIndex=0; nIndex<nCount; ++nIndex)
    {
        Reference<beans::XPropertySet> xSet (xPages[nIndex], UNO_QUERY);
        if (xSet.is())
        {
            try
            {
                Any aNumber = xSet->getPropertyValue("Number");
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

    return bOk;
}




Any SAL_CALL SdUnoSlideView::getSelection (void)
      throw(RuntimeException)
{
    Any aResult;

    slidesorter::model::PageEnumeration aSelectedPages (
        slidesorter::model::PageEnumerationProvider::CreateSelectedPagesEnumeration(
            mrSlideSorter.GetModel()));
    int nSelectedPageCount (
        mrSlideSorter.GetController().GetPageSelector().GetSelectedPageCount());

    Sequence<Reference<XInterface> > aPages(nSelectedPageCount);
    int nIndex = 0;
    while (aSelectedPages.HasMoreElements() && nIndex<nSelectedPageCount)
    {
        slidesorter::model::SharedPageDescriptor pDescriptor (aSelectedPages.GetNextElement());
        aPages[nIndex++] = pDescriptor->GetPage()->getUnoPage();
    }
    aResult <<= aPages;

    return aResult;
}




void SAL_CALL SdUnoSlideView::addSelectionChangeListener (
    const css::uno::Reference<css::view::XSelectionChangeListener>& rxListener)
    throw(css::uno::RuntimeException)
{
    (void)rxListener;
}




void SAL_CALL SdUnoSlideView::removeSelectionChangeListener (
    const css::uno::Reference<css::view::XSelectionChangeListener>& rxListener)
    throw(css::uno::RuntimeException)
{
    (void)rxListener;
}




//----- XDrawView -------------------------------------------------------------

void SAL_CALL SdUnoSlideView::setCurrentPage (
    const css::uno::Reference<css::drawing::XDrawPage>& rxDrawPage)
    throw(css::uno::RuntimeException)
{
    Reference<beans::XPropertySet> xProperties (rxDrawPage, UNO_QUERY);
    if (xProperties.is())
    {
        sal_uInt16 nPageNumber(0);
        if (xProperties->getPropertyValue("Number") >>= nPageNumber)
        {
            mrSlideSorter.GetController().GetCurrentSlideManager()->SwitchCurrentSlide(
                nPageNumber-1,
                true);
        }
    }
}




css::uno::Reference<css::drawing::XDrawPage > SAL_CALL
    SdUnoSlideView::getCurrentPage (void)
    throw(css::uno::RuntimeException)
{
    return mrSlideSorter.GetController().GetCurrentSlideManager()->GetCurrentSlide()->GetXDrawPage();
}




//----- XFastPropertySet ------------------------------------------------------

void SdUnoSlideView::setFastPropertyValue (
    sal_Int32 nHandle,
        const Any& rValue)
    throw(css::beans::UnknownPropertyException,
        css::beans::PropertyVetoException,
        css::lang::IllegalArgumentException,
        css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    (void)nHandle;
    (void)rValue;

    throw beans::UnknownPropertyException();
}




Any SAL_CALL SdUnoSlideView::getFastPropertyValue (
    sal_Int32 nHandle)
    throw(css::beans::UnknownPropertyException,
        css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    (void)nHandle;

    if( nHandle != DrawController::PROPERTY_VIEWOFFSET )
        throw beans::UnknownPropertyException();

    return Any();
}


// XServiceInfo
OUString SAL_CALL SdUnoSlideView::getImplementationName(  ) throw (RuntimeException)
{
    return OUString( "com.sun.star.comp.sd.SdUnoSlideView" );
}

sal_Bool SAL_CALL SdUnoSlideView::supportsService( const OUString& ServiceName ) throw (RuntimeException)
{
    return comphelper::ServiceInfoHelper::supportsService( ServiceName, getSupportedServiceNames() );
}

Sequence< OUString > SAL_CALL SdUnoSlideView::getSupportedServiceNames(  ) throw (RuntimeException)
{
    OUString aSN( "com.sun.star.presentation.SlidesView" );
    uno::Sequence< OUString > aSeq( &aSN, 1 );
    return aSeq;
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
