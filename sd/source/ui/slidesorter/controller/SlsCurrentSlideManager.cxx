/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsCurrentSlideManager.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:24:16 $
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

#include "precompiled_sd.hxx"

#include "SlideSorter.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "controller/SlsPageSelector.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsCurrentSlideManager.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsPageObjectViewObjectContact.hxx"
#include "view/SlsHighlightObject.hxx"
#include "ViewShellBase.hxx"
#include "ViewShell.hxx"
#include "DrawViewShell.hxx"
#include "sdpage.hxx"
#include "FrameView.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

using namespace ::sd::slidesorter::model;

namespace sd { namespace slidesorter { namespace controller {

CurrentSlideManager::CurrentSlideManager (SlideSorter& rSlideSorter)
    : mrSlideSorter(rSlideSorter),
      mnCurrentSlideIndex(-1),
      mpCurrentSlide()
{
}




CurrentSlideManager::~CurrentSlideManager (void)
{
}




void CurrentSlideManager::CurrentSlideHasChanged (const sal_Int32 nSlideIndex)
{
    if (mnCurrentSlideIndex != nSlideIndex)
    {
        ReleaseCurrentSlide();
        AcquireCurrentSlide(nSlideIndex);

        // Update the selection.
        mrSlideSorter.GetController().GetPageSelector().DeselectAllPages();
        if (mpCurrentSlide.get() != NULL)
            mrSlideSorter.GetController().GetPageSelector().SelectPage(mpCurrentSlide);
    }
}




void CurrentSlideManager::ReleaseCurrentSlide (void)
{
    if (mpCurrentSlide.get() != NULL)
    {
        mpCurrentSlide->SetIsCurrentPage(false);
        mrSlideSorter.GetView().RequestRepaint(mpCurrentSlide);
    }

    mpCurrentSlide.reset();
}




bool CurrentSlideManager::IsCurrentSlideIsValid (void)
{
    return mnCurrentSlideIndex >= 0 && mnCurrentSlideIndex<mrSlideSorter.GetModel().GetPageCount();
}




void CurrentSlideManager::AcquireCurrentSlide (const sal_Int32 nSlideIndex)
{
    mnCurrentSlideIndex = nSlideIndex;

    if (IsCurrentSlideIsValid())
    {
        // Get a descriptor for the XDrawPage reference.  Note that the
        // given XDrawPage may or may not be member of the slide sorter
        // document.
        mpCurrentSlide = mrSlideSorter.GetModel().GetPageDescriptor(mnCurrentSlideIndex);
        if (mpCurrentSlide.get() != NULL)
        {
            mpCurrentSlide->SetIsCurrentPage(true);
            view::HighlightObject* pObject = mrSlideSorter.GetController().GetHighlightObject();
            if (pObject != NULL)
                pObject->SetSlide(mpCurrentSlide);
            mrSlideSorter.GetView().RequestRepaint(mpCurrentSlide);
        }
    }
}




void CurrentSlideManager::SwitchCurrentSlide (const sal_Int32 nSlideIndex)
{
    SwitchCurrentSlide(mrSlideSorter.GetModel().GetPageDescriptor(nSlideIndex));
}




void CurrentSlideManager::SwitchCurrentSlide (const SharedPageDescriptor& rpDescriptor)
{
    if (rpDescriptor.get() != NULL)
    {
        mpCurrentSlide = rpDescriptor;
        mnCurrentSlideIndex = (rpDescriptor->GetPage()->GetPageNum()-1)/2;

        ViewShell* pViewShell = mrSlideSorter.GetViewShell();
        if (pViewShell != NULL && pViewShell->IsMainViewShell())
        {
            FrameView* pFrameView = pViewShell->GetFrameView();
            if (pFrameView != NULL)
                pFrameView->SetSelectedPage(sal::static_int_cast<USHORT>(mnCurrentSlideIndex));
        }
        else
        {
            // Set current page.  At the moment we have to do this in two
            // different ways.  The UNO way is the preferable one but, alas,
            // it does not work always correctly (after some kinds of model
            // changes).  Therefore, we call DrawViewShell::SwitchPage(),
            // too.
            SetCurrentSlideAtViewShellBase(rpDescriptor);
            SetCurrentSlideAtXController(rpDescriptor);
        }
    }
}




void CurrentSlideManager::SetCurrentSlideAtViewShellBase (const SharedPageDescriptor& rpDescriptor)
{
    OSL_ASSERT(rpDescriptor.get() != NULL);

    ViewShellBase* pBase = mrSlideSorter.GetViewShellBase();
    if (pBase != NULL)
    {
        DrawViewShell* pDrawViewShell = dynamic_cast<DrawViewShell*>(
            pBase->GetMainViewShell().get());
        if (pDrawViewShell != NULL)
        {
            USHORT nPageNumber = (rpDescriptor->GetPage()->GetPageNum()-1)/2;
            pDrawViewShell->SwitchPage(nPageNumber);
            pDrawViewShell->GetPageTabControl()->SetCurPageId(nPageNumber+1);
        }
        /*
        else
        {
            presenter::PresenterViewShell* pPresenterViewShell
                = dynamic_cast<presenter::PresenterViewShell*>(pBase->GetMainViewShell());
            if (pPresenterViewShell != NULL)
            {
                pPresenterViewShell->SetCurrentSlide(
                    Reference<drawing::XDrawPage>(
                        rpDescriptor->GetPage()->getUnoPage(), UNO_QUERY));
            }
        }
        */
    }
}




void CurrentSlideManager::SetCurrentSlideAtXController (const SharedPageDescriptor& rpDescriptor)
{
    OSL_ASSERT(rpDescriptor.get() != NULL);

    try
    {
        Reference<beans::XPropertySet> xSet (mrSlideSorter.GetXController(), UNO_QUERY);
        if (xSet.is())
        {
            Any aPage;
            aPage <<= rpDescriptor->GetPage()->getUnoPage();
            xSet->setPropertyValue (
                String::CreateFromAscii("CurrentPage"),
                aPage);
        }
    }
    catch (beans::UnknownPropertyException aException)
    {
        // We have not been able to set the current page at the main view.
        // This is sad but still leaves us in a valid state.  Therefore,
        // this exception is silently ignored.
    }
}




SharedPageDescriptor CurrentSlideManager::GetCurrentSlide (void)
{
    return mpCurrentSlide;
}




void CurrentSlideManager::PrepareModelChange (void)
{
    mpCurrentSlide.reset();
}




void CurrentSlideManager::HandleModelChange (void)
{
    if (mpCurrentSlide.get() != NULL)
    {
        mpCurrentSlide = mrSlideSorter.GetModel().GetPageDescriptor(
            mpCurrentSlide->GetPageIndex());
        mpCurrentSlide->SetIsCurrentPage(true);
    }
}




SdPage* GetCurrentSdPage (SlideSorter& rSlideSorter)
{
    SharedPageDescriptor pDescriptor (
        rSlideSorter.GetController().GetCurrentSlideManager()->GetCurrentSlide());
    if (pDescriptor.get() == NULL)
        return NULL;
    else
        return pDescriptor->GetPage();
}



} } } // end of namespace ::sd::slidesorter::controller
