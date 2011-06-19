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

class PresenterPaneContainer;

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
