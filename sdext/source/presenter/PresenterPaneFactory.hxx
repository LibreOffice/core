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

#ifndef INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERPANEFACTORY_HXX
#define INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERPANEFACTORY_HXX

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/drawing/XPresenterHelper.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XPane.hpp>
#include <com/sun/star/drawing/framework/XResourceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <rtl/ref.hxx>
#include <map>
#include <memory>

namespace sdext { namespace presenter {

class PresenterController;

namespace {
    typedef ::cppu::WeakComponentImplHelper <
        css::drawing::framework::XResourceFactory
    > PresenterPaneFactoryInterfaceBase;
}

/** The PresenerPaneFactory provides a fixed set of panes.

    In order to make the presener screen more easily extendable in the
    future the set of supported panes could be made extendable on demand.
*/
class PresenterPaneFactory
    : public ::cppu::BaseMutex,
      public PresenterPaneFactoryInterfaceBase
{
public:
    static const OUString msCurrentSlidePreviewPaneURL;
    static const OUString msNextSlidePreviewPaneURL;
    static const OUString msNotesPaneURL;
    static const OUString msToolBarPaneURL;
    static const OUString msSlideSorterPaneURL;
    static const OUString msHelpPaneURL;
    static const OUString msOverlayPaneURL;

    /** Create a new instance of this class and register it as resource
        factory in the drawing framework of the given controller.
        This registration keeps it alive.  When the drawing framework is
        shut down and releases its reference to the factory then the factory
        is destroyed.
    */
    static css::uno::Reference<css::drawing::framework::XResourceFactory> Create (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const css::uno::Reference<css::frame::XController>& rxController,
        const ::rtl::Reference<PresenterController>& rpPresenterController);
    virtual ~PresenterPaneFactory();

    virtual void SAL_CALL disposing()
        throw (css::uno::RuntimeException) SAL_OVERRIDE;

    // XResourceFactory

    virtual css::uno::Reference<css::drawing::framework::XResource>
        SAL_CALL createResource (
            const css::uno::Reference<
                css::drawing::framework::XResourceId>& rxPaneId)
        throw (css::uno::RuntimeException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL
        releaseResource (
            const css::uno::Reference<css::drawing::framework::XResource>&
                rxPane)
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

private:
    css::uno::WeakReference<css::uno::XComponentContext> mxComponentContextWeak;
    css::uno::WeakReference<css::drawing::framework::XConfigurationController>
        mxConfigurationControllerWeak;
    ::rtl::Reference<PresenterController> mpPresenterController;
    typedef ::std::map<OUString, css::uno::Reference<css::drawing::framework::XResource> >
        ResourceContainer;
    std::unique_ptr<ResourceContainer> mpResourceCache;

    PresenterPaneFactory (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const ::rtl::Reference<PresenterController>& rpPresenterController);

    void Register (const css::uno::Reference<css::frame::XController>& rxController);

    css::uno::Reference<css::drawing::framework::XResource> CreatePane (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId,
        const OUString& rsTitle);
    css::uno::Reference<css::drawing::framework::XResource> CreatePane (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId,
        const OUString& rsTitle,
        const css::uno::Reference<css::drawing::framework::XPane>& rxParentPane,
        const bool bIsSpritePane);

    void ThrowIfDisposed() const throw (css::lang::DisposedException);
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
