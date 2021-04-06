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

#include <framework/ViewShellWrapper.hxx>
#include <sdpage.hxx>
#include <ViewShell.hxx>

#include <SlideSorter.hxx>
#include <SlideSorterViewShell.hxx>
#include <controller/SlsPageSelector.hxx>
#include <controller/SlideSorterController.hxx>
#include <model/SlsPageEnumerationProvider.hxx>
#include <model/SlsPageDescriptor.hxx>

#include <com/sun/star/drawing/framework/XPane.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/servicehelper.hxx>
#include <osl/mutex.hxx>
#include <sal/log.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

using ::com::sun::star::awt::XWindow;

namespace sd::framework {

ViewShellWrapper::ViewShellWrapper (
    const std::shared_ptr<ViewShell>& pViewShell,
    const Reference<XResourceId>& rxViewId,
    const Reference<awt::XWindow>& rxWindow)
    : ViewShellWrapperInterfaceBase(MutexOwner::maMutex),
      mpViewShell(pViewShell),
      mpSlideSorterViewShell(
          std::dynamic_pointer_cast< ::sd::slidesorter::SlideSorterViewShell >( pViewShell )),
      mxViewId(rxViewId),
      mxWindow(rxWindow)
{
}

ViewShellWrapper::~ViewShellWrapper()
{
}

void SAL_CALL ViewShellWrapper::disposing()
{
    ::osl::MutexGuard aGuard( maMutex );

    SAL_INFO("sd.ui", "disposing ViewShellWrapper " << this);
    Reference<awt::XWindow> xWindow (mxWindow);
    if (xWindow.is())
    {
        SAL_INFO(
            "sd.ui",
            "removing ViewShellWrapper " << this << " from window listener at "
                << mxWindow.get());
        xWindow->removeWindowListener(this);
    }

    mpSlideSorterViewShell.reset();
    mpViewShell.reset();
}

uno::Any SAL_CALL ViewShellWrapper::queryInterface( const uno::Type & rType )
{
    if( mpSlideSorterViewShell &&
        rType == cppu::UnoType<view::XSelectionSupplier>::get() )
    {
        uno::Reference<view::XSelectionSupplier> xSupplier( this );
        return Any(xSupplier);
    }
    else
        return ViewShellWrapperInterfaceBase::queryInterface( rType );
}

//----- XResource -------------------------------------------------------------

Reference<XResourceId> SAL_CALL ViewShellWrapper::getResourceId()
{
    return mxViewId;
}

sal_Bool SAL_CALL ViewShellWrapper::isAnchorOnly()
{
    return false;
}

//----- XSelectionSupplier --------------------------------------------------

sal_Bool SAL_CALL ViewShellWrapper::select( const css::uno::Any& aSelection )
{
    if (!mpSlideSorterViewShell)
        return false;

    ::sd::slidesorter::controller::SlideSorterController& rSlideSorterController
        = mpSlideSorterViewShell->GetSlideSorter().GetController();
    ::sd::slidesorter::controller::PageSelector& rSelector (rSlideSorterController.GetPageSelector());
    rSelector.DeselectAllPages();
    Sequence<Reference<drawing::XDrawPage> > xPages;
    aSelection >>= xPages;
    for (const auto& rPage : std::as_const(xPages))
    {
        Reference<beans::XPropertySet> xSet (rPage, UNO_QUERY);
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

    return true;
}

uno::Any SAL_CALL ViewShellWrapper::getSelection()
{
    Any aResult;

    if (!mpSlideSorterViewShell)
        return aResult;

    slidesorter::model::PageEnumeration aSelectedPages (
        slidesorter::model::PageEnumerationProvider::CreateSelectedPagesEnumeration(
            mpSlideSorterViewShell->GetSlideSorter().GetModel()));
    int nSelectedPageCount (
        mpSlideSorterViewShell->GetSlideSorter().GetController().GetPageSelector().GetSelectedPageCount());

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

void SAL_CALL ViewShellWrapper::addSelectionChangeListener( const uno::Reference< view::XSelectionChangeListener >& )
{
}

void SAL_CALL ViewShellWrapper::removeSelectionChangeListener( const uno::Reference< view::XSelectionChangeListener >& )
{
}

//----- XRelocatableResource --------------------------------------------------

sal_Bool SAL_CALL ViewShellWrapper::relocateToAnchor (
    const Reference<XResource>& xResource)
{
    bool bResult (false);

    Reference<XPane> xPane (xResource, UNO_QUERY);
    if (xPane.is())
    {
        // Detach from the window of the old pane.
        Reference<awt::XWindow> xWindow (mxWindow);
        if (xWindow.is())
            xWindow->removeWindowListener(this);
        mxWindow = nullptr;

        if (mpViewShell != nullptr)
        {
            VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow(xPane->getWindow());
            if (pWindow && mpViewShell->RelocateToParentWindow(pWindow))
            {
                bResult = true;

                // Attach to the window of the new pane.
                xWindow = xPane->getWindow();
                if (xWindow.is())
                {
                    xWindow->addWindowListener(this);
                    mpViewShell->Resize();
                }
            }
        }
    }

    return bResult;
}

//----- XUnoTunnel ------------------------------------------------------------

const Sequence<sal_Int8>& ViewShellWrapper::getUnoTunnelId()
{
    static const UnoTunnelIdInit theViewShellWrapperUnoTunnelId;
    return theViewShellWrapperUnoTunnelId.getSeq();
}

sal_Int64 SAL_CALL ViewShellWrapper::getSomething (const Sequence<sal_Int8>& rId)
{
    sal_Int64 nResult = 0;

    if (isUnoTunnelId<ViewShellWrapper>(rId))
    {
        nResult = reinterpret_cast<sal_Int64>(this);
    }

    return nResult;
}

//===== awt::XWindowListener ==================================================

void SAL_CALL ViewShellWrapper::windowResized (const awt::WindowEvent&)
{
    ViewShell* pViewShell (mpViewShell.get());
    if (pViewShell != nullptr)
        pViewShell->Resize();
}

void SAL_CALL ViewShellWrapper::windowMoved (const awt::WindowEvent&) {}

void SAL_CALL ViewShellWrapper::windowShown (const lang::EventObject&)
{
    ViewShell* pViewShell (mpViewShell.get());
    if (pViewShell != nullptr)
        pViewShell->Resize();
}

void SAL_CALL ViewShellWrapper::windowHidden (const lang::EventObject&) {}

//===== XEventListener ========================================================

void SAL_CALL ViewShellWrapper::disposing (const lang::EventObject& rEvent)
{
    if (rEvent.Source == mxWindow)
        mxWindow = nullptr;
}

} // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
