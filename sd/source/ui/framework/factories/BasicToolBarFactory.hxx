/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: BasicToolBarFactory.hxx,v $
 *
 * $Revision: 1.3 $
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

#ifndef SD_FRAMEWORK_BASIC_TOOL_BAR_FACTORY_HXX
#define SD_FRAMEWORK_BASIC_TOOL_BAR_FACTORY_HXX

#include "MutexOwner.hxx"

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/drawing/framework/XResourceFactory.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XResourceId.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <osl/mutex.hxx>
#include <cppuhelper/compbase3.hxx>
#include <cppuhelper/basemutex.hxx>


namespace css = ::com::sun::star;

namespace {

typedef ::cppu::WeakComponentImplHelper3 <
    css::drawing::framework::XResourceFactory,
    css::lang::XInitialization,
    css::lang::XEventListener
    > BasicToolBarFactoryInterfaceBase;

} // end of anonymous namespace.

namespace sd {
class ViewShellBase;
}

namespace sd { namespace framework {

/** This factory provides some of the frequently used tool bars:
        private:resource/toolbar/ViewTabBar
*/
class BasicToolBarFactory
    : protected ::cppu::BaseMutex,
      public BasicToolBarFactoryInterfaceBase
{
public:
    BasicToolBarFactory (
        const css::uno::Reference<com::sun::star::uno::XComponentContext>& rxContext);
    virtual ~BasicToolBarFactory (void);

    virtual void SAL_CALL disposing (void);


    // ToolBarFactory

    virtual css::uno::Reference<com::sun::star::drawing::framework::XResource> SAL_CALL
        createResource (
            const css::uno::Reference<
                css::drawing::framework::XResourceId>& rxToolBarId)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL
        releaseResource (
            const css::uno::Reference<com::sun::star::drawing::framework::XResource>&
                rxToolBar)
        throw (css::uno::RuntimeException);


    // XInitialization

    virtual void SAL_CALL initialize(
        const css::uno::Sequence<com::sun::star::uno::Any>& aArguments)
        throw (css::uno::Exception, css::uno::RuntimeException);


    // lang::XEventListener

    virtual void SAL_CALL disposing (
        const css::lang::EventObject& rEventObject)
        throw (css::uno::RuntimeException);

private:
    css::uno::Reference<css::drawing::framework::XConfigurationController> mxConfigurationController;
    css::uno::Reference<css::frame::XController> mxController;
    ViewShellBase* mpViewShellBase;

    void Shutdown (void);

    void ThrowIfDisposed (void) const
        throw (css::lang::DisposedException);
};

} } // end of namespace sd::framework

#endif
