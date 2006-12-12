/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SdUnoSlideView.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 18:55:35 $
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

#include "SdUnoSlideView.hxx"

#include "SlideSorterViewShell.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsPageSelector.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "sdpage.hxx"

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sd {


SdUnoSlideView::SdUnoSlideView (
    DrawController& rController,
    slidesorter::SlideSorterViewShell& rViewShell,
    View& rView) throw()
    : DrawSubController(),
      mrController(rController),
      mrSlideSorterViewShell(rViewShell),
      mrView(rView)
{
}




SdUnoSlideView::~SdUnoSlideView (void) throw()
{
}




//===== XSelectionSupplier ====================================================

sal_Bool SAL_CALL SdUnoSlideView::select (const Any& aSelection)
      throw(lang::IllegalArgumentException, RuntimeException)
{
    bool bOk = true;

    slidesorter::controller::SlideSorterController& rSlideSorterController
        = mrSlideSorterViewShell.GetSlideSorterController();
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
                Any aNumber = xSet->getPropertyValue(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Number")));
                sal_Int32 nPageNumber;
                aNumber >>= nPageNumber;
                nPageNumber -=1; // Transform 1-based page numbers to 0-based ones.
                rSelector.SelectPage(nPageNumber);
            }
            catch(RuntimeException e)
            {
            }
        }
    }
    rSlideSorterController.MakeSelectionVisible();

    return bOk;
}




Any SAL_CALL SdUnoSlideView::getSelection (void)
      throw(RuntimeException)
{
    Any aResult;

    slidesorter::controller::SlideSorterController& rSlideSorterController
        = mrSlideSorterViewShell.GetSlideSorterController();
    slidesorter::model::SlideSorterModel::Enumeration aSelectedPages (
        rSlideSorterController.GetModel().GetSelectedPagesEnumeration());
    int nSelectedPageCount (rSlideSorterController.GetPageSelector().GetSelectedPageCount());

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




//===== XDraw View ============================================================

void SAL_CALL SdUnoSlideView::setCurrentPage (
    const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& )
    throw(::com::sun::star::uno::RuntimeException)
{
}




::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage > SAL_CALL
    SdUnoSlideView::getCurrentPage (void)
    throw(::com::sun::star::uno::RuntimeException)
{
    return Reference<drawing::XDrawPage>();
}



void SdUnoSlideView::FillPropertyTable (
    ::std::vector< ::com::sun::star::beans::Property>& )
{
    /* no additional properties */
}




sal_Bool SAL_CALL SdUnoSlideView::convertFastPropertyValue(
    ::com::sun::star::uno::Any & ,
    ::com::sun::star::uno::Any & ,
    sal_Int32 ,
    const ::com::sun::star::uno::Any&  )
    throw (::com::sun::star::lang::IllegalArgumentException)
{
    return sal_False;
}




void SAL_CALL SdUnoSlideView::setFastPropertyValue_NoBroadcast(
    sal_Int32 ,
    const ::com::sun::star::uno::Any&  )
    throw (::com::sun::star::uno::Exception)
{
}




void SAL_CALL SdUnoSlideView::getFastPropertyValue( ::com::sun::star::uno::Any&, sal_Int32  ) const
{
}



} // end of namespace sd
