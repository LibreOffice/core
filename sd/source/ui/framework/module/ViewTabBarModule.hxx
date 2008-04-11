/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ViewTabBarModule.hxx,v $
 * $Revision: 1.4 $
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

#ifndef SD_FRAMEWORK_VIEW_TAB_BAR_MODULE_HXX
#define SD_FRAMEWORK_VIEW_TAB_BAR_MODULE_HXX

#include "MutexOwner.hxx"
<<<<<<< ViewTabBarModule.hxx

=======
>>>>>>> 1.2.234.3
#include <com/sun/star/drawing/framework/XConfigurationChangeListener.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
<<<<<<< ViewTabBarModule.hxx
#include <com/sun/star/drawing/framework/XTabBar.hpp>
=======
#include <com/sun/star/drawing/framework/XToolBarController.hpp>
>>>>>>> 1.2.234.3
#include <com/sun/star/frame/XController.hpp>
#include <osl/mutex.hxx>
#include <cppuhelper/compbase1.hxx>

namespace css = ::com::sun::star;

namespace {

typedef ::cppu::WeakComponentImplHelper1 <
    ::css::drawing::framework::XConfigurationChangeListener
    > ViewTabBarModuleInterfaceBase;

} // end of anonymous namespace.




namespace sd { namespace framework {

/** This module is responsible for showing the ViewTabBar above the view in
    the center pane.
*/
class ViewTabBarModule
    : private sd::MutexOwner,
      public ViewTabBarModuleInterfaceBase
{
public:
    /** Create a new module that controlls the view tab bar above the view
        in the specified pane.
        @param rxController
            This is the access point to the drawing framework.
        @param rxViewTabBarId
            This ResourceId specifies which tab bar is to be managed by the
            new module.
    */
    ViewTabBarModule (
        const css::uno::Reference<css::frame::XController>& rxController,
        const css::uno::Reference<
            css::drawing::framework::XResourceId>& rxViewTabBarId);
    virtual ~ViewTabBarModule (void);

    virtual void SAL_CALL disposing (void);


    // XConfigurationChangeListener

    virtual void SAL_CALL notifyConfigurationChange (
        const css::drawing::framework::ConfigurationChangeEvent& rEvent)
        throw (css::uno::RuntimeException);

    // XEventListener

    virtual void SAL_CALL disposing (
        const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException);

private:
    css::uno::Reference<
        css::drawing::framework::XConfigurationController> mxConfigurationController;
    css::uno::Reference<css::drawing::framework::XResourceId> mxViewTabBarId;

    /** This is the place where the view tab bar is filled.  Only missing
        buttons are added, so it is safe to call this method multiple
        times.
    */
    void UpdateViewTabBar (
        const css::uno::Reference<css::drawing::framework::XTabBar>& rxTabBar);
};

} } // end of namespace sd::framework

#endif
