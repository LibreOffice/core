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

#ifndef INCLUDED_SD_SOURCE_UI_FRAMEWORK_FACTORIES_BASICVIEWFACTORY_HXX
#define INCLUDED_SD_SOURCE_UI_FRAMEWORK_FACTORIES_BASICVIEWFACTORY_HXX

#include "MutexOwner.hxx"

#include <com/sun/star/drawing/framework/XResourceFactory.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XPane.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/compbase.hxx>
#include <osl/mutex.hxx>

#include <vcl/vclptr.hxx>
#include <memory>

namespace sd {
class ViewShell;
class ViewShellBase;
class FrameView;
}
class SfxViewFrame;
namespace vcl { class Window; }

namespace {

typedef ::cppu::WeakComponentImplHelper <
    css::drawing::framework::XResourceFactory,
    css::lang::XInitialization
    > BasicViewFactoryInterfaceBase;

} // end of anonymous namespace.

namespace sd { namespace framework {

/** Factory for the frequently used standard views of the drawing framework:
        private:resource/view/
        private:resource/view/ImpressView
        private:resource/view/GraphicView
        private:resource/view/OutlineView
        private:resource/view/NotesView
        private:resource/view/HandoutView
        private:resource/view/SlideSorter
        private:resource/view/PresentationView
        private:resource/view/TaskPane
    For some views in some panes this class also acts as a cache.
*/
class BasicViewFactory
    : private sd::MutexOwner,
      public BasicViewFactoryInterfaceBase
{
public:
    explicit BasicViewFactory (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext);
    virtual ~BasicViewFactory();

    virtual void SAL_CALL disposing() override;

    // XViewFactory

    virtual css::uno::Reference<css::drawing::framework::XResource>
        SAL_CALL createResource (
            const css::uno::Reference<css::drawing::framework::XResourceId>& rxViewId)
        throw(css::uno::RuntimeException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, std::exception) override;

    virtual void SAL_CALL releaseResource (
        const css::uno::Reference<css::drawing::framework::XResource>& xView)
        throw(css::uno::RuntimeException, std::exception) override;

    // XInitialization

    virtual void SAL_CALL initialize(
        const css::uno::Sequence<css::uno::Any>& aArguments)
        throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

private:
    css::uno::Reference<css::drawing::framework::XConfigurationController>
        mxConfigurationController;
    class ViewDescriptor;
    class ViewShellContainer;
    std::unique_ptr<ViewShellContainer> mpViewShellContainer;
    ViewShellBase* mpBase;
    FrameView* mpFrameView;

    class ViewCache;
    ScopedVclPtr<vcl::Window> mpWindow;
    std::shared_ptr<ViewCache> mpViewCache;

    css::uno::Reference<css::drawing::framework::XPane> mxLocalPane;

    std::shared_ptr<ViewDescriptor> CreateView (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxViewId,
        SfxViewFrame& rFrame,
        vcl::Window& rWindow,
        const css::uno::Reference<css::drawing::framework::XPane>& rxPane,
        FrameView* pFrameView,
        const bool bIsCenterView);

    std::shared_ptr<ViewShell> CreateViewShell (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxViewId,
        SfxViewFrame& rFrame,
        vcl::Window& rWindow,
        FrameView* pFrameView,
        const bool bIsCenterView);

    void ActivateCenterView (
        const std::shared_ptr<ViewDescriptor>& rpDescriptor);

    void ReleaseView (
        const std::shared_ptr<ViewDescriptor>& rpDescriptor,
        bool bDoNotCache = false);

    bool IsCacheable (
        const std::shared_ptr<ViewDescriptor>& rpDescriptor);

    std::shared_ptr<ViewDescriptor> GetViewFromCache (
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxViewId,
        const css::uno::Reference<css::drawing::framework::XPane>& rxPane);
};

} } // end of namespace sd::framework

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
