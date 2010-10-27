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

#ifndef SDEXT_PRESENTER_PRESENTER_SCREEN_HXX
#define SDEXT_PRESENTER_PRESENTER_SCREEN_HXX

#include "PresenterConfigurationAccess.hxx"
#include "PresenterPaneContainer.hxx"
#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel2.hpp>
#include <com/sun/star/task/XJob.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XView.hpp>
#include <com/sun/star/presentation/XSlideShowController.hpp>
#include <com/sun/star/presentation/XPresentation2.hpp>
#include <rtl/ref.hxx>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

namespace css = ::com::sun::star;

namespace sdext { namespace presenter {

class PresenterWindowManager;
class PresenterController;

namespace {
    typedef ::cppu::WeakComponentImplHelper1 <
        css::task::XJob
        > PresenterScreenJobInterfaceBase;
    typedef ::cppu::WeakComponentImplHelper1 <
        css::lang::XEventListener
        > PresenterScreenInterfaceBase;
}




/** The PresenterScreenJob service is instantiated every time a document is
    created or loaded.  In its execute() method it then filters out all
    non-Impress documents and creates and registers a new PresenterScreen
    object.
*/
class PresenterScreenJob
    : private ::boost::noncopyable,
      private ::cppu::BaseMutex,
      public PresenterScreenJobInterfaceBase
{
public:
    static ::rtl::OUString getImplementationName_static (void);
    static css::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_static (void);
    static css::uno::Reference<css::uno::XInterface> Create(
        const css::uno::Reference<css::uno::XComponentContext>& rxContext)
        SAL_THROW((css::uno::Exception));

    virtual void SAL_CALL disposing (void);

    // XJob

    virtual css::uno::Any SAL_CALL execute(
        const css::uno::Sequence<css::beans::NamedValue >& Arguments)
        throw (css::lang::IllegalArgumentException,
            css::uno::Exception,
            css::uno::RuntimeException);

private:
    PresenterScreenJob (const css::uno::Reference<css::uno::XComponentContext>& rxContext);
    virtual ~PresenterScreenJob (void);

    css::uno::Reference<css::uno::XComponentContext> mxComponentContext;
};




/** This is the bootstrap class of the presenter screen.  It is registered
    as drawing framework startup service.  That means that every drawing
    framework instance creates an instance of this class.

    <p>A PresenterScreen object registers itself as listener for drawing
    framework configuration changes.  It waits for the full screen marker (a
    top level resource) to appear in the current configuration.  When that
    happens the actual presenter screen is initialized.  A new
    PresenterController is created and takes over the task of controlling
    the presenter screen.</p>
*/
class PresenterScreen
    : private ::boost::noncopyable,
      private ::cppu::BaseMutex,
      public PresenterScreenInterfaceBase
{
public:
    PresenterScreen (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const css::uno::Reference<css::frame::XModel2>& rxModel);
    virtual ~PresenterScreen (void);

    virtual void SAL_CALL disposing (void);

    /** Make the presenter screen visible.
    */
    void InitializePresenterScreen (void);

    /** Do not call ShutdownPresenterScreen() directly.  Call
        RequestShutdownPresenterScreen() instead.  It will issue an
        asynchronous call to ShutdownPresenterScreen() when that is safe.
    */
    void RequestShutdownPresenterScreen (void);


    // XEventListener

    virtual void SAL_CALL disposing ( const css::lang::EventObject& rEvent) throw (css::uno::RuntimeException);

private:
    css::uno::Reference<css::frame::XModel2 > mxModel;
    css::uno::Reference<css::frame::XController> mxController;
    css::uno::WeakReference<css::drawing::framework::XConfigurationController>
        mxConfigurationControllerWeak;
    css::uno::WeakReference<css::uno::XComponentContext> mxContextWeak;
    css::uno::WeakReference<css::presentation::XSlideShowController> mxSlideShowControllerWeak;
    ::rtl::Reference<PresenterController> mpPresenterController;
    css::uno::Reference<css::drawing::framework::XResourceId> mxSlideShowViewId;
    css::uno::Reference<css::drawing::framework::XConfiguration> mxSavedConfiguration;
    ::rtl::Reference<PresenterPaneContainer> mpPaneContainer;
    sal_Int32 mnComponentIndex;
    css::uno::Reference<css::drawing::framework::XResourceFactory> mxPaneFactory;
    css::uno::Reference<css::drawing::framework::XResourceFactory> mxViewFactory;

    class ViewDescriptor
    {
    public:
        ::rtl::OUString msTitle;
        ::rtl::OUString msAccessibleTitle;
        bool mbIsOpaque;
    };
    typedef ::std::map<rtl::OUString,ViewDescriptor> ViewDescriptorContainer;
    ViewDescriptorContainer maViewDescriptors;


    void ShutdownPresenterScreen (void);

    /** Create and initialize the factory for presenter view specific panes.
    */
    void SetupPaneFactory (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext);

    /** Create and initialize the factory for presenter view specific views.
    */
    void SetupViewFactory (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext);

    /** Read the current layout from the configuration and call
        ProcessLayout to bring it on to the screen.
    */
    void SetupConfiguration (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxAnchorId);

    /** Read one layout from the configuration and make resource activation
        requests to bring it on to the screen.  When one layout references a
        parent layout then this method calls itself recursively.
    */
    void ProcessLayout (
        PresenterConfigurationAccess& rConfiguration,
        const ::rtl::OUString& rsLayoutName,
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxAnchorId);

    /** Called by ProcessLayout for a single entry of a Layouts
        configuration list.
    */
    void ProcessComponent (
        const ::rtl::OUString& rsKey,
        const ::std::vector<css::uno::Any>& rValues,
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxAnchorId);

    /** Read the view descriptions from the configuration.
    */
    void ProcessViewDescriptions (
        PresenterConfigurationAccess& rConfiguration);

    /** Called by ProcessViewDescriptions for a single entry.
    */
    void ProcessViewDescription (
        const ::rtl::OUString& rsKey,
        const ::std::vector<css::uno::Any>& rValues);

    void SetupView (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxAnchorId,
        const ::rtl::OUString& rsPaneURL,
        const ::rtl::OUString& rsViewURL,
        const PresenterPaneContainer::ViewInitializationFunction& rViewInitialization,
        const double nLeft,
        const double nTop,
        const double nRight,
        const double nBottom);

    /** Return the screen number on which to display the presenter screen.
        @return
            Returns -1 when the presenter screen can or shall not be
            displayed.
    */
    sal_Int32 GetScreenNumber (
        const css::uno::Reference<css::presentation::XPresentation2>& rxPresentation) const;

    /** Create a resource id for the full screen background pane so that it
        is displayed on another screen than the full screen presentation.
    */
    css::uno::Reference<css::drawing::framework::XResourceId> GetMainPaneId (
        const css::uno::Reference<css::presentation::XPresentation2>& rxPresentation) const;
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
