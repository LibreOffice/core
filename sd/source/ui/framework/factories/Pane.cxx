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

#include "precompiled_sd.hxx"

#include "framework/Pane.hxx"

#include <rtl/uuid.h>
#include <vcl/svapp.hxx>
#include <vos/mutex.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>
#include <cppcanvas/vclfactory.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

using ::rtl::OUString;

namespace sd { namespace framework {

Pane::Pane (
    const Reference<XResourceId>& rxPaneId,
    ::Window* pWindow)
    throw ()
    : PaneInterfaceBase(MutexOwner::maMutex),
      mxPaneId(rxPaneId),
      mpWindow(pWindow),
      mxWindow(VCLUnoHelper::GetInterface(pWindow))
{
}




Pane::~Pane (void) throw()
{
}




void Pane::disposing (void)
{
    mxWindow = NULL;
    mpWindow = NULL;
}




::Window* Pane::GetWindow (void)
{
    if (mxWindow.is())
        return mpWindow;
    else
        return NULL;
}




//----- XPane -----------------------------------------------------------------

Reference<awt::XWindow> SAL_CALL Pane::getWindow (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();

    return mxWindow;
}




Reference<rendering::XCanvas> SAL_CALL Pane::getCanvas (void)
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    ThrowIfDisposed();

    if ( ! mxCanvas.is())
        mxCanvas = CreateCanvas();

    return mxCanvas;
}




//----- XPane2 ----------------------------------------------------------------

sal_Bool SAL_CALL Pane::isVisible (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();

    const ::Window* pWindow = GetWindow();
    if (pWindow != NULL)
        return pWindow->IsVisible();
    else
        return false;
}




void SAL_CALL Pane::setVisible (sal_Bool bIsVisible)
    throw (RuntimeException)
{
    ThrowIfDisposed();

    ::Window* pWindow = GetWindow();
    if (pWindow != NULL)
        pWindow->Show(bIsVisible);
}




Reference<accessibility::XAccessible> SAL_CALL Pane::getAccessible (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    ::Window* pWindow = GetWindow();
    if (pWindow != NULL)
        return pWindow->GetAccessible(sal_False);
    else
        return NULL;
}




void SAL_CALL Pane::setAccessible (
    const Reference<accessibility::XAccessible>& rxAccessible)
    throw (RuntimeException)
{
    ThrowIfDisposed();
    ::Window* pWindow = GetWindow();
    if (pWindow != NULL)
        pWindow->SetAccessible(rxAccessible);
}




//----- XResource -------------------------------------------------------------

Reference<XResourceId> SAL_CALL Pane::getResourceId (void)
    throw (RuntimeException)
{
    ThrowIfDisposed();

    return mxPaneId;
}




sal_Bool SAL_CALL Pane::isAnchorOnly (void)
    throw (RuntimeException)
{
    return true;
}




//----- XUnoTunnel ------------------------------------------------------------

const Sequence<sal_Int8>& Pane::getUnoTunnelId (void)
{
    static Sequence<sal_Int8>* pSequence = NULL;
    if (pSequence == NULL)
    {
        const ::vos::OGuard aSolarGuard (Application::GetSolarMutex());
        if (pSequence == NULL)
        {
            static ::com::sun::star::uno::Sequence<sal_Int8> aSequence (16);
            rtl_createUuid((sal_uInt8*)aSequence.getArray(), 0, sal_True);
            pSequence = &aSequence;
        }
    }
    return *pSequence;
}




sal_Int64 SAL_CALL Pane::getSomething (const Sequence<sal_Int8>& rId)
    throw (RuntimeException)
{
    sal_Int64 nResult = 0;

    if (rId.getLength() == 16
        && rtl_compareMemory(getUnoTunnelId().getConstArray(), rId.getConstArray(), 16) == 0)
    {
        nResult = reinterpret_cast<sal_Int64>(this);
    }

    return nResult;
}




//-----------------------------------------------------------------------------

Reference<rendering::XCanvas> Pane::CreateCanvas (void)
    throw (RuntimeException)
{
    Reference<rendering::XCanvas> xCanvas;

    if (mpWindow != NULL)
    {
        ::cppcanvas::SpriteCanvasSharedPtr pCanvas (
            ::cppcanvas::VCLFactory::getInstance().createSpriteCanvas(*mpWindow));
        if (pCanvas.get() != NULL)
            xCanvas = Reference<rendering::XCanvas>(pCanvas->getUNOSpriteCanvas(), UNO_QUERY);
    }

    return xCanvas;
}




void Pane::ThrowIfDisposed (void) const
    throw (lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                "Pane object has already been disposed")),
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}


} } // end of namespace sd::framework
