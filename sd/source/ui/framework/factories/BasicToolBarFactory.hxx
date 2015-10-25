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

#ifndef INCLUDED_SD_SOURCE_UI_FRAMEWORK_FACTORIES_BASICTOOLBARFACTORY_HXX
#define INCLUDED_SD_SOURCE_UI_FRAMEWORK_FACTORIES_BASICTOOLBARFACTORY_HXX

#include "MutexOwner.hxx"

#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/drawing/framework/XResourceFactory.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XResourceId.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <osl/mutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>

namespace {

typedef ::cppu::WeakComponentImplHelper <
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
    explicit BasicToolBarFactory (
        const css::uno::Reference<com::sun::star::uno::XComponentContext>& rxContext);
    virtual ~BasicToolBarFactory();

    virtual void SAL_CALL disposing() override;

    // ToolBarFactory

    virtual css::uno::Reference<com::sun::star::drawing::framework::XResource> SAL_CALL
        createResource (
            const css::uno::Reference<
                css::drawing::framework::XResourceId>& rxToolBarId)
        throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, std::exception) override;

    virtual void SAL_CALL
        releaseResource (
            const css::uno::Reference<com::sun::star::drawing::framework::XResource>&
                rxToolBar)
        throw (css::uno::RuntimeException, std::exception) override;

    // XInitialization

    virtual void SAL_CALL initialize(
        const css::uno::Sequence<com::sun::star::uno::Any>& aArguments)
        throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    // lang::XEventListener

    virtual void SAL_CALL disposing (
        const css::lang::EventObject& rEventObject)
        throw (css::uno::RuntimeException, std::exception) override;

private:
    css::uno::Reference<css::drawing::framework::XConfigurationController> mxConfigurationController;
    css::uno::Reference<css::frame::XController> mxController;
    ViewShellBase* mpViewShellBase;

    void Shutdown();

    void ThrowIfDisposed() const
        throw (css::lang::DisposedException);
};

} } // end of namespace sd::framework

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
