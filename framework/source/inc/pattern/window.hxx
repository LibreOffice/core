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

#ifndef __FRAMEWORK_PATTERN_WINDOW_HXX_
#define __FRAMEWORK_PATTERN_WINDOW_HXX_

#include <general.h>

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>

#include <toolkit/unohlp.hxx>
#include <vcl/window.hxx>
#include <vcl/syswin.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>

//_______________________________________________
// namespaces

#ifndef css
namespace css = ::com::sun::star;
#endif

namespace framework{


class WindowHelper
{
    public:

//-----------------------------------------------
static ::rtl::OUString getWindowState(const css::uno::Reference< css::awt::XWindow >& xWindow)
{
    if (!xWindow.is())
        return ::rtl::OUString();

    rtl::OString sWindowState;
    // SOLAR SAFE -> ----------------------------
    {
        SolarMutexGuard aSolarGuard;

        Window*    pWindow     = VCLUnoHelper::GetWindow(xWindow);
        // check for system window is neccessary to guarantee correct pointer cast!
        if (pWindow!=NULL && pWindow->IsSystemWindow())
        {
            sal_uLong nMask  = WINDOWSTATE_MASK_ALL;
            nMask &= ~(WINDOWSTATE_MASK_MINIMIZED);
            sWindowState = ((SystemWindow*)pWindow)->GetWindowState(nMask);
        }
    }
    // <- SOLAR SAFE ----------------------------

    return rtl::OStringToOUString(sWindowState,RTL_TEXTENCODING_UTF8);
}

//-----------------------------------------------
static void setWindowState(const css::uno::Reference< css::awt::XWindow >& xWindow     ,
                           const ::rtl::OUString&                          sWindowState)
{
    if (
        (!xWindow.is()            ) ||
        (!sWindowState.getLength())
       )
        return;

    // SOLAR SAFE -> ----------------------------
    SolarMutexGuard aSolarGuard;

    Window* pWindow = VCLUnoHelper::GetWindow(xWindow);
    // check for system window is neccessary to guarantee correct pointer cast!
    if (
        (pWindow                  ) &&
        (pWindow->IsSystemWindow()) &&
        (
            // dont overwrite a might existing minimized mode!
            (pWindow->GetType() != WINDOW_WORKWINDOW) ||
            (!((WorkWindow*)pWindow)->IsMinimized() )
        )
       )
    {
        ((SystemWindow*)pWindow)->SetWindowState(OUStringToOString(sWindowState,RTL_TEXTENCODING_UTF8));
    }

    // <- SOLAR SAFE ----------------------------
}

//-----------------------------------------------
static ::sal_Bool isTopWindow(const css::uno::Reference< css::awt::XWindow >& xWindow)
{
    // even child frame containing top level windows (e.g. query designer of database) will be closed
    css::uno::Reference< css::awt::XTopWindow > xTopWindowCheck(xWindow, css::uno::UNO_QUERY);
    if (xTopWindowCheck.is())
    {
        // Note: Toolkit interface XTopWindow sometimes is used by real VCL-child-windows also .-)
        // Be sure that these window is realy a "top system window".
        // Attention ! Checking Window->GetParent() isnt the right approach here.
        // Because sometimes VCL create "implicit border windows" as parents even we created
        // a simple XWindow using the toolkit only .-(
        SolarMutexGuard aSolarGuard;
        Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
        if (
            (pWindow                  ) &&
            (pWindow->IsSystemWindow())
           )
            return sal_True;
    }

    return sal_False;
}

};

} // namespace framework

#endif // __FRAMEWORK_PATTERN_WINDOW_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
