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

#ifndef INCLUDED_SD_SOURCE_UI_FRAMEWORK_FACTORIES_BASICPANEFACTORY_HXX
#define INCLUDED_SD_SOURCE_UI_FRAMEWORK_FACTORIES_BASICPANEFACTORY_HXX

#include <com/sun/star/drawing/framework/XResourceFactory.hpp>
#include <com/sun/star/drawing/framework/XConfigurationChangeListener.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>

#include <memory>

namespace com { namespace sun { namespace star { namespace drawing { namespace framework { class XConfigurationController; } } } } }
namespace com { namespace sun { namespace star { namespace frame { class XController; } } } }
namespace com { namespace sun { namespace star { namespace uno { class XComponentContext; } } } }

namespace sd {

class ViewShellBase;
}

namespace sd { namespace framework {

typedef ::cppu::WeakComponentImplHelper <
    css::lang::XInitialization,
    css::drawing::framework::XResourceFactory,
    css::drawing::framework::XConfigurationChangeListener
    > BasicPaneFactoryInterfaceBase;

/** This factory provides the frequently used standard panes
        private:resource/pane/CenterPane
        private:resource/pane/FullScreenPane
        private:resource/pane/LeftImpressPane
        private:resource/pane/LeftDrawPane
    There are two left panes because this is (seems to be) the only way to
    show different titles for the left pane in Draw and Impress.
*/
class BasicPaneFactory
    : private ::cppu::BaseMutex,
      public BasicPaneFactoryInterfaceBase
{
public:
    explicit BasicPaneFactory (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext);
    virtual ~BasicPaneFactory() override;

    virtual void SAL_CALL disposing() override;

    // XInitialization

    virtual void SAL_CALL initialize(
        const css::uno::Sequence<css::uno::Any>& aArguments) override;

    // XResourceFactory

    virtual css::uno::Reference<css::drawing::framework::XResource>
        SAL_CALL createResource (
            const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId) override;

    virtual void SAL_CALL
        releaseResource (
            const css::uno::Reference<css::drawing::framework::XResource>& rxPane) override;

    // XConfigurationChangeListener

    virtual void SAL_CALL notifyConfigurationChange (
        const css::drawing::framework::ConfigurationChangeEvent& rEvent) override;

    // lang::XEventListener

    virtual void SAL_CALL disposing (
        const css::lang::EventObject& rEventObject) override;

private:
    css::uno::Reference<css::uno::XComponentContext> mxComponentContext;
    css::uno::WeakReference<css::drawing::framework::XConfigurationController>
        mxConfigurationControllerWeak;
    ViewShellBase* mpViewShellBase;
    class PaneDescriptor;
    class PaneContainer;
    std::unique_ptr<PaneContainer> mpPaneContainer;

    /** Create a new instance of FrameWindowPane.
        @param rPaneId
            There is only one frame window so this id is just checked to
            have the correct value.
    */
    css::uno::Reference<css::drawing::framework::XResource>
        CreateFrameWindowPane (
            const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId);

    /** Create a new pane that represents the center pane in full screen
        mode.
    */
    css::uno::Reference<css::drawing::framework::XResource>
        CreateFullScreenPane (
            const css::uno::Reference<css::uno::XComponentContext>& rxComponentContext,
            const css::uno::Reference<css::drawing::framework::XResourceId>& rxPaneId);

    /** Create a new instance of ChildWindowPane.
        @param rPaneId
            The ResourceURL member defines which side pane to create.
    */
    css::uno::Reference<css::drawing::framework::XResource>
        CreateChildWindowPane (
            const css::uno::Reference<
                css::drawing::framework::XResourceId>& rxPaneId,
            const PaneDescriptor& rDescriptor);

    /// @throws css::lang::DisposedException
    void ThrowIfDisposed() const;
};

} } // end of namespace sd::framework

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
