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


#include "framework/ViewShellWrapper.hxx"
#include "framework/Pane.hxx"
#include "taskpane/ToolPanelViewShell.hxx"
#include "ViewShell.hxx"
#include "Window.hxx"

#include <com/sun/star/drawing/framework/XPane.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

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

using ::rtl::OUString;
using ::sd::toolpanel::ToolPanelViewShell;

namespace sd { namespace framework {

ViewShellWrapper::ViewShellWrapper (
    ::boost::shared_ptr<ViewShell> pViewShell,
    const Reference<XResourceId>& rxViewId,
    const Reference<awt::XWindow>& rxWindow)
    : ViewShellWrapperInterfaceBase(MutexOwner::maMutex),
      mpViewShell(pViewShell),
      mxViewId(rxViewId),
      mxWindow(rxWindow)
{
}




ViewShellWrapper::~ViewShellWrapper (void)
{
}




void SAL_CALL ViewShellWrapper::disposing (void)
{
    ::osl::MutexGuard aGuard( maMutex );

    OSL_TRACE("disposing ViewShellWrapper %x", this);
    Reference<awt::XWindow> xWindow (mxWindow);
    if (xWindow.is())
    {
        OSL_TRACE("removing ViewShellWrapper %x from window listener at %x", this, mxWindow.get());
        xWindow->removeWindowListener(this);
    }

    mpViewShell.reset();
}




::boost::shared_ptr<ViewShell> ViewShellWrapper::GetViewShell (void)
{
    return mpViewShell;
}




//----- XResource -------------------------------------------------------------

Reference<XResourceId> SAL_CALL ViewShellWrapper::getResourceId (void)
    throw (RuntimeException)
{
    return mxViewId;
}




sal_Bool SAL_CALL ViewShellWrapper::isAnchorOnly (void)
    throw (RuntimeException)
{
    return false;
}




//----- XRelocatableResource --------------------------------------------------

sal_Bool SAL_CALL ViewShellWrapper::relocateToAnchor (
    const Reference<XResource>& xResource)
    throw (RuntimeException)
{
    sal_Bool bResult (false);

    Reference<XPane> xPane (xResource, UNO_QUERY);
    if (xPane.is())
    {
        // Detach from the window of the old pane.
        Reference<awt::XWindow> xWindow (mxWindow);
        if (xWindow.is())
            xWindow->removeWindowListener(this);
        mxWindow = NULL;

        if (mpViewShell.get() != NULL)
        {
            ::Window* pWindow = VCLUnoHelper::GetWindow(xPane->getWindow());
            if (pWindow != NULL && mpViewShell->RelocateToParentWindow(pWindow))
            {
                bResult = sal_True;

                // Attach to the window of the new pane.
                xWindow = Reference<awt::XWindow>(xPane->getWindow(), UNO_QUERY);
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

const Sequence<sal_Int8>& ViewShellWrapper::getUnoTunnelId (void)
{
    return theViewShellWrapperUnoTunnelId::get().getSeq();
}




sal_Int64 SAL_CALL ViewShellWrapper::getSomething (const Sequence<sal_Int8>& rId)
    throw (RuntimeException)
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
    throw (RuntimeException)
{
    (void)rEvent;
    ViewShell* pViewShell (mpViewShell.get());
    if (pViewShell != NULL)
        pViewShell->Resize();
}




void SAL_CALL ViewShellWrapper::windowMoved (const awt::WindowEvent& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
}




void SAL_CALL ViewShellWrapper::windowShown (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
    ViewShell* pViewShell (mpViewShell.get());
    if (pViewShell != NULL)
        pViewShell->Resize();
}




void SAL_CALL ViewShellWrapper::windowHidden (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    (void)rEvent;
}




//===== XEventListener ========================================================

void SAL_CALL ViewShellWrapper::disposing (const lang::EventObject& rEvent)
    throw (RuntimeException)
{
    if (rEvent.Source == mxWindow)
        mxWindow = NULL;
}


} } // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
