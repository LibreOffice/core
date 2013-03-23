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

#pragma once
#if 1

#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/drawing/XPresenterHelper.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XPane.hpp>
#include <com/sun/star/drawing/framework/XResourceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <rtl/ref.hxx>
#include <boost/scoped_ptr.hpp>
#include <map>

namespace sdext { namespace presenter {

class PresenterController;

namespace {
    typedef ::cppu::WeakComponentImplHelper1 <
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
    static const ::rtl::OUString msCurrentSlidePreviewPaneURL;
    static const ::rtl::OUString msNextSlidePreviewPaneURL;
    static const ::rtl::OUString msNotesPaneURL;
    static const ::rtl::OUString msToolBarPaneURL;
    static const ::rtl::OUString msSlideSorterPaneURL;
    static const ::rtl::OUString msHelpPaneURL;
    static const ::rtl::OUString msOverlayPaneURL;

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
    virtual ~PresenterPaneFactory (void);

    static ::rtl::OUString getImplementationName_static (void);
    static css::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_static (void);
    static css::uno::Reference<css::uno::XInterface> Create(
        const css::uno::Reference<css::uno::XComponentContext>& rxContext)
        SAL_THROW((css::uno::Exception));

    virtual void SAL_CALL disposing (void)
        throw (css::uno::RuntimeException);

    // XResourceFactory

    virtual css::uno::Reference<css::drawing::framework::XResource>
        SAL_CALL createResource (
            const ::com::sun::star::uno::Reference<
                com::sun::star::drawing::framework::XResourceId>& rxPaneId)
        throw (::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException);

    virtual void SAL_CALL
        releaseResource (
            const ::com::sun::star::uno::Reference<com::sun::star::drawing::framework::XResource>&
                rxPane)
        throw (::com::sun::star::uno::RuntimeException);

private:
    css::uno::WeakReference<css::uno::XComponentContext> mxComponentContextWeak;
    css::uno::WeakReference<css::drawing::framework::XConfigurationController>
        mxConfigurationControllerWeak;
    ::rtl::Reference<PresenterController> mpPresenterController;
    typedef ::std::map<rtl::OUString, css::uno::Reference<css::drawing::framework::XResource> >
        ResourceContainer;
    ::boost::scoped_ptr<ResourceContainer> mpResourceCache;

    PresenterPaneFactory (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const ::rtl::Reference<PresenterController>& rpPresenterController);

    void Register (const css::uno::Reference<css::frame::XController>& rxController);

    css::uno::Reference<css::drawing::framework::XResource> CreatePane (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId,
        const ::rtl::OUString& rsTitle);
    css::uno::Reference<css::drawing::framework::XResource> CreatePane (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId,
        const ::rtl::OUString& rsTitle,
        const css::uno::Reference<css::drawing::framework::XPane>& rxParentPane,
        const bool bIsSpritePane);

    void ThrowIfDisposed (void) const throw (::com::sun::star::lang::DisposedException);
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
