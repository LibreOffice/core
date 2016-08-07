/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef __FRAMEWORK_PATTERN_WINDOW_HXX_
#define __FRAMEWORK_PATTERN_WINDOW_HXX_

//_______________________________________________
// own includes

#include <general.h>

//_______________________________________________
// interface includes
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>

//_______________________________________________
// other includes

#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif
#include <vcl/window.hxx>
#include <vcl/syswin.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/svapp.hxx>
#include <vos/mutex.hxx>
#include <rtl/ustring.hxx>

//_______________________________________________
// namespaces

#ifndef css
namespace css = ::com::sun::star;
#endif

namespace framework{

//_______________________________________________
// definitions

class WindowHelper
{
    public:

//-----------------------------------------------
static ::rtl::OUString getWindowState(const css::uno::Reference< css::awt::XWindow >& xWindow)
{
    if (!xWindow.is())
        return ::rtl::OUString();

    // SOLAR SAFE -> ----------------------------
    ::vos::OClearableGuard aSolarGuard(Application::GetSolarMutex());

    ByteString sWindowState;
    Window*    pWindow     = VCLUnoHelper::GetWindow(xWindow);
    // check for system window is necessary to guarantee correct pointer cast!
    if (pWindow!=NULL && pWindow->IsSystemWindow())
    {
        sal_uLong nMask  = WINDOWSTATE_MASK_ALL;
              nMask &= ~(WINDOWSTATE_MASK_MINIMIZED);
        sWindowState = ((SystemWindow*)pWindow)->GetWindowState(nMask);
    }

    aSolarGuard.clear();
    // <- SOLAR SAFE ----------------------------

    return B2U_ENC(sWindowState,RTL_TEXTENCODING_UTF8);
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
    ::vos::OClearableGuard aSolarGuard(Application::GetSolarMutex());

    Window* pWindow = VCLUnoHelper::GetWindow(xWindow);
    // check for system window is necessary to guarantee correct pointer cast!
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
        ((SystemWindow*)pWindow)->SetWindowState(U2B_ENC(sWindowState,RTL_TEXTENCODING_UTF8));
    }

    aSolarGuard.clear();
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
        // Be sure that these window is really a "top system window".
        // Attention ! Checking Window->GetParent() isn't the right approach here.
        // Because sometimes VCL create "implicit border windows" as parents even we created
        // a simple XWindow using the toolkit only .-(
        ::vos::OGuard aSolarLock(&Application::GetSolarMutex());
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
