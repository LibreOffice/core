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

#ifndef INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERSCREEN_HXX
#define INCLUDED_SDEXT_SOURCE_PRESENTER_PRESENTERSCREEN_HXX

#include "PresenterConfigurationAccess.hxx"
#include "PresenterPaneContainer.hxx"
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel2.hpp>
#include <com/sun/star/task/XJob.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/presentation/XPresentation2.hpp>
#include <rtl/ref.hxx>

#include <map>
#include <string_view>

namespace sd { class DrawController; }

namespace sdext::presenter {

class PresenterController;

typedef ::cppu::WeakComponentImplHelper <
    css::task::XJob, css::lang::XServiceInfo
    > PresenterScreenJobInterfaceBase;

/** The PresenterScreenJob service is instantiated every time a document is
    created or loaded.  In its execute() method it then filters out all
    non-Impress documents and creates and registers a new PresenterScreen
    object.
*/
class PresenterScreenJob
    : private ::cppu::BaseMutex,
      public PresenterScreenJobInterfaceBase
{
public:
    PresenterScreenJob(const PresenterScreenJob&) = delete;
    PresenterScreenJob& operator=(const PresenterScreenJob&) = delete;

    virtual void SAL_CALL disposing() override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames () override;

    // XJob
    virtual css::uno::Any SAL_CALL execute(
        const css::uno::Sequence<css::beans::NamedValue >& Arguments) override;

    explicit PresenterScreenJob (const css::uno::Reference<css::uno::XComponentContext>& rxContext);
    virtual ~PresenterScreenJob() override;

private:
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
typedef ::cppu::WeakComponentImplHelper <
    css::lang::XEventListener
    > PresenterScreenInterfaceBase;
class PresenterScreen
    : private ::cppu::BaseMutex,
      public PresenterScreenInterfaceBase
{
public:
    PresenterScreen (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        css::uno::Reference<css::frame::XModel2> xModel);
    virtual ~PresenterScreen() override;
    PresenterScreen(const PresenterScreen&) = delete;
    PresenterScreen& operator=(const PresenterScreen&) = delete;

    virtual void SAL_CALL disposing() override;

    static bool isPresenterScreenEnabled(
        const css::uno::Reference<css::uno::XComponentContext>& rxContext);
    static bool isPresenterScreenFullScreen(
        const css::uno::Reference<css::uno::XComponentContext>& rxContext);

    /** Make the presenter screen visible.
    */
    void InitializePresenterScreen();

    /** Do not call ShutdownPresenterScreen() directly.  Call
        RequestShutdownPresenterScreen() instead.  It will issue an
        asynchronous call to ShutdownPresenterScreen() when that is safe.
    */
    void RequestShutdownPresenterScreen();

    /** Switch / converse monitors between presenter view and slide output
     */
    void SwitchMonitors();

    // XEventListener

    virtual void SAL_CALL disposing ( const css::lang::EventObject& rEvent) override;

    void CheckNextSlideUpdate(const css::uno::Reference<css::drawing::XShape>& rxShape);

private:
    css::uno::Reference<css::frame::XModel2 > mxModel;
    rtl::Reference<::sd::DrawController> mxController;
    css::uno::WeakReference<css::drawing::framework::XConfigurationController>
        mxConfigurationControllerWeak;
    css::uno::WeakReference<css::uno::XComponentContext> mxContextWeak;
    ::rtl::Reference<PresenterController> mpPresenterController;
    css::uno::Reference<css::drawing::framework::XConfiguration> mxSavedConfiguration;
    ::rtl::Reference<PresenterPaneContainer> mpPaneContainer;
    css::uno::Reference<css::drawing::framework::XResourceFactory> mxPaneFactory;
    css::uno::Reference<css::drawing::framework::XResourceFactory> mxViewFactory;

    // IASS: Flag to note if InitializePresenterScreen() was executed
    bool mbIsInitialized;

    class ViewDescriptor
    {
    public:
        OUString msTitle;
        OUString msAccessibleTitle;
        bool mbIsOpaque;
        ViewDescriptor()
            : mbIsOpaque(false)
        {
        }
    };
    typedef ::std::map<OUString,ViewDescriptor> ViewDescriptorContainer;
    ViewDescriptorContainer maViewDescriptors;

    void ShutdownPresenterScreen();

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
        std::u16string_view rsLayoutName,
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxAnchorId);

    /** Called by ProcessLayout for a single entry of a Layouts
        configuration list.
    */
    void ProcessComponent (
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
        const ::std::vector<css::uno::Any>& rValues);

    void SetupView (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext,
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxAnchorId,
        const OUString& rsPaneURL,
        const OUString& rsViewURL,
        const PresenterPaneContainer::ViewInitializationFunction& rViewInitialization);

    /** Return the built-in screen number on the presentation will normally
        display the presenter console.
        @return
            Returns -1 when the presenter screen can or shall not be
            displayed.
    */
    sal_Int32 GetPresenterScreenNumber (
                const css::uno::Reference<css::presentation::XPresentation2>& rxPresentation) const;

    static sal_Int32 GetPresenterScreenFromScreen( sal_Int32 nPresentationScreen );

    /** Create a resource id for the full screen background pane so that it
        is displayed on another screen than the full screen presentation.
    */
    css::uno::Reference<css::drawing::framework::XResourceId> GetMainPaneId (
        const css::uno::Reference<css::presentation::XPresentation2>& rxPresentation,
        const css::uno::Reference<com::sun::star::uno::XComponentContext>& xContext) const;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
