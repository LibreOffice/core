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

#ifndef SDEXT_PRESENTER_PANE_FACTORY_HXX
#define SDEXT_PRESENTER_PANE_FACTORY_HXX

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

namespace css = ::com::sun::star;

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
