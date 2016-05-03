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

#include "framework/ViewShellWrapper.hxx"
#include "framework/Pane.hxx"
#include "sdpage.hxx"
#include "ViewShell.hxx"
#include "Window.hxx"

#include "SlideSorter.hxx"
#include "SlideSorterViewShell.hxx"
#include "controller/SlsPageSelector.hxx"
#include "controller/SlsCurrentSlideManager.hxx"
#include "controller/SlideSorterController.hxx"
#include "model/SlsPageEnumerationProvider.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"

#include <com/sun/star/drawing/framework/XPane.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <tools/diagnose_ex.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

using ::com::sun::star::awt::XWindow;
using ::com::sun::star::rendering::XCanvas;
using ::com::sun::star::lang::DisposedException;

namespace sd { namespace framework {

ViewShellWrapper::ViewShellWrapper (
    std::shared_ptr<ViewShell> pViewShell,
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

uno::Any SAL_CALL ViewShellWrapper::queryInterface( const uno::Type & rType ) throw(uno::RuntimeException, std::exception)
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
    throw (RuntimeException, std::exception)
{
    return mxViewId;
}

sal_Bool SAL_CALL ViewShellWrapper::isAnchorOnly()
    throw (RuntimeException, std::exception)
{
    return false;
}

//----- XSelectionSupplier --------------------------------------------------

sal_Bool SAL_CALL ViewShellWrapper::select( const css::uno::Any& aSelection ) throw(lang::IllegalArgumentException, uno::RuntimeException, std::exception)
{
    if (!mpSlideSorterViewShell)
        return false;

    bool bOk = true;

    ::sd::slidesorter::controller::SlideSorterController& rSlideSorterController
        = mpSlideSorterViewShell->GetSlideSorter().GetController();
    ::sd::slidesorter::controller::PageSelector& rSelector (rSlideSorterController.GetPageSelector());
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

uno::Any SAL_CALL ViewShellWrapper::getSelection()
    throw (uno::RuntimeException, std::exception)
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

void SAL_CALL ViewShellWrapper::addSelectionChangeListener( const uno::Reference< view::XSelectionChangeListener >& ) throw(uno::RuntimeException, std::exception)
{
}

void SAL_CALL ViewShellWrapper::removeSelectionChangeListener( const uno::Reference< view::XSelectionChangeListener >& ) throw(uno::RuntimeException, std::exception)
{
}

//----- XRelocatableResource --------------------------------------------------

sal_Bool SAL_CALL ViewShellWrapper::relocateToAnchor (
    const Reference<XResource>& xResource)
    throw (RuntimeException, std::exception)
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

        if (mpViewShell.get() != nullptr)
        {
            vcl::Window* pWindow = VCLUnoHelper::GetWindow(xPane->getWindow());
            if (pWindow != nullptr && mpViewShell->RelocateToParentWindow(pWindow))
            {
                bResult = true;

                // Attach to the window of the new pane.
                xWindow.set(xPane->getWindow(), UNO_QUERY);
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

namespace
{
    class theViewShellWrapperUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theViewShellWrapperUnoTunnelId> {};
}

const Sequence<sal_Int8>& ViewShellWrapper::getUnoTunnelId()
{
    return theViewShellWrapperUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL ViewShellWrapper::getSomething (const Sequence<sal_Int8>& rId)
    throw (RuntimeException, std::exception)
{
    sal_Int64 nResult = 0;

    if (rId.getLength() == 16
        && memcmp(getUnoTunnelId().getConstArray(), rId.getConstArray(), 16) == 0)
    {
        nResult = reinterpret_cast<sal_Int64>(this);
    }

    return nResult;
}

//===== awt::XWindowListener ==================================================

void SAL_CALL ViewShellWrapper::windowResized (const awt::WindowEvent& rEvent)
    throw (RuntimeException, std::exception)
{
    (void)rEvent;
    ViewShell* pViewShell (mpViewShell.get());
    if (pViewShell != nullptr)
        pViewShell->Resize();
}

void SAL_CALL ViewShellWrapper::windowMoved (const awt::WindowEvent& rEvent)
    throw (RuntimeException, std::exception)
{
    (void)rEvent;
}

void SAL_CALL ViewShellWrapper::windowShown (const lang::EventObject& rEvent)
    throw (RuntimeException, std::exception)
{
    (void)rEvent;
    ViewShell* pViewShell (mpViewShell.get());
    if (pViewShell != nullptr)
        pViewShell->Resize();
}

void SAL_CALL ViewShellWrapper::windowHidden (const lang::EventObject& rEvent)
    throw (RuntimeException, std::exception)
{
    (void)rEvent;
}

//===== XEventListener ========================================================

void SAL_CALL ViewShellWrapper::disposing (const lang::EventObject& rEvent)
    throw (RuntimeException, std::exception)
{
    if (rEvent.Source == mxWindow)
        mxWindow = nullptr;
}

} } // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
