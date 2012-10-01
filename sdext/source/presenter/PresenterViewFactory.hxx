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

#ifndef SDEXT_PRESENTER_VIEW_FACTORY_HXX
#define SDEXT_PRESENTER_VIEW_FACTORY_HXX

#include "PresenterController.hxx"
#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XResourceFactory.hpp>
#include <com/sun/star/drawing/framework/XView.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <rtl/ref.hxx>
#include <boost/scoped_ptr.hpp>

namespace css = ::com::sun::star;

namespace sdext { namespace presenter {

namespace {
    typedef ::cppu::WeakComponentImplHelper1 <
        css::drawing::framework::XResourceFactory
    > PresenterViewFactoryInterfaceBase;
}

/** Base class for presenter views that allows the view factory to store
    them in a cache and reuse deactivated views.
*/
class CachablePresenterView
{
public:
    virtual void ActivatePresenterView (void);

    /** Called when the view is put into a cache.  The view must not paint
        itself while being deactive.
    */
    virtual void DeactivatePresenterView (void);

    /** Called before the view is disposed.  This gives the view the
        oportunity to trigger actions that may lead to (synchronous)
        callbacks that do not result in DisposedExceptions.
    */
    virtual void ReleaseView (void);

protected:
    bool mbIsPresenterViewActive;

    CachablePresenterView (void);

    ~CachablePresenterView() {}
};

/** Factory of the presenter screen specific views.  The supported set of
    views includes:
        a life view of the current slide,
        a static preview of the next slide,
        the notes of the current slide,
        a tool bar
*/
class PresenterViewFactory
    : public ::cppu::BaseMutex,
      public PresenterViewFactoryInterfaceBase
{
public:
    static const ::rtl::OUString msCurrentSlidePreviewViewURL;
    static const ::rtl::OUString msNextSlidePreviewViewURL;
    static const ::rtl::OUString msNotesViewURL;
    static const ::rtl::OUString msToolBarViewURL;
    static const ::rtl::OUString msSlideSorterURL;
    static const ::rtl::OUString msHelpViewURL;

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
    virtual ~PresenterViewFactory (void);

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
            const css::uno::Reference<css::drawing::framework::XResourceId>& rxViewId)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL
        releaseResource (
            const css::uno::Reference<css::drawing::framework::XResource>& rxPane)
        throw (css::uno::RuntimeException);

private:
    css::uno::Reference<css::uno::XComponentContext> mxComponentContext;
    css::uno::Reference<css::drawing::framework::XConfigurationController>
        mxConfigurationController;
    css::uno::WeakReference<css::frame::XController> mxControllerWeak;
    ::rtl::Reference<PresenterController> mpPresenterController;
    typedef ::std::pair<css::uno::Reference<css::drawing::framework::XView>,
        css::uno::Reference<css::drawing::framework::XPane> > ViewResourceDescriptor;
    typedef ::std::map<rtl::OUString, ViewResourceDescriptor> ResourceContainer;
    ::boost::scoped_ptr<ResourceContainer> mpResourceCache;

    PresenterViewFactory (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const css::uno::Reference<css::frame::XController>& rxController,
        const ::rtl::Reference<PresenterController>& rpPresenterController);

    void Register (const css::uno::Reference<css::frame::XController>& rxController);

    css::uno::Reference<css::drawing::framework::XView> CreateSlideShowView(
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxViewId) const;

    css::uno::Reference<css::drawing::framework::XView> CreateSlidePreviewView(
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxViewId,
        const css::uno::Reference<css::drawing::framework::XPane>& rxPane) const;

    css::uno::Reference<css::drawing::framework::XView> CreateToolBarView(
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxViewId) const;

    css::uno::Reference<css::drawing::framework::XView> CreateNotesView(
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxViewId,
        const css::uno::Reference<css::drawing::framework::XPane>& rxPane) const;

    css::uno::Reference<css::drawing::framework::XView> CreateSlideSorterView(
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxViewId) const;

    css::uno::Reference<css::drawing::framework::XView> CreateHelpView(
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxViewId) const;

    css::uno::Reference<css::drawing::framework::XResource> GetViewFromCache (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxViewId,
        const css::uno::Reference<css::drawing::framework::XPane>& rxAnchorPane) const;
    css::uno::Reference<css::drawing::framework::XResource> CreateView(
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxViewId,
        const css::uno::Reference<css::drawing::framework::XPane>& rxAnchorPane);

    void ThrowIfDisposed (void) const throw (::com::sun::star::lang::DisposedException);
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
