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

#include <MutexOwner.hxx>
#include <com/sun/star/drawing/framework/XConfigurationChangeListener.hpp>
#include <cppuhelper/compbase.hxx>
#include <memory>
#include <set>

namespace com::sun::star::drawing::framework { class XConfigurationController; }
namespace com::sun::star::drawing::framework { class XControllerManager; }
namespace com::sun::star::drawing::framework { class XTabBar; }
namespace com::sun::star::frame { class XController; }

namespace sd::framework {

typedef ::cppu::WeakComponentImplHelper <
    css::drawing::framework::XConfigurationChangeListener
    > SlideSorterModuleBase;

/** This module is responsible for showing the slide sorter bar and the
    slide sorter view in the center pane.

    Manage the activation state of one resource depending on the view in the
    center pane.  The ResourceManager remembers in which configuration to
    activate and in which to deactivate the resource.  When the resource is
    deactivated or activated manually by the user then the ResourceManager
    detects this and remembers it for the future.
*/
class SlideSorterModule
    : private sd::MutexOwner,
      public SlideSorterModuleBase
{
public:
    SlideSorterModule (
        const css::uno::Reference<css::frame::XController>& rxController,
        const OUString& rsLeftPaneURL);
    virtual ~SlideSorterModule() override;

    /** Remember the given URL as one of a center pane view for which to
        activate the resource managed by the called object.
    */
    void AddActiveMainView (const OUString& rsMainViewURL);
    bool IsResourceActive (const OUString& rsMainViewURL);
    void SaveResourceState();

    virtual void SAL_CALL disposing() override;

    // XConfigurationChangeListener
    virtual void SAL_CALL notifyConfigurationChange (
        const css::drawing::framework::ConfigurationChangeEvent& rEvent) override;

    // XEventListener
    virtual void SAL_CALL disposing (
        const css::lang::EventObject& rEvent) override;

private:
    css::uno::Reference<css::drawing::framework::XConfigurationController>
        mxConfigurationController;
    ::std::set<OUString> maActiveMainViewContainer;
    /// The resource managed by this class.
    css::uno::Reference<css::drawing::framework::XResourceId> mxResourceId;
    /// The anchor of the main view.
    css::uno::Reference<css::drawing::framework::XResourceId> mxMainViewAnchorId;
    OUString msCurrentMainViewURL;
    css::uno::Reference<css::drawing::framework::XResourceId> mxViewTabBarId;
    css::uno::Reference<css::drawing::framework::XControllerManager> mxControllerManager;

    void HandleMainViewSwitch (
        const OUString& rsViewURL,
        const bool bIsActivated);
    void HandleResourceRequest(
        bool bActivation,
        const css::uno::Reference<css::drawing::framework::XConfiguration>& rxConfiguration);
    void UpdateViewTabBar (
        const css::uno::Reference<css::drawing::framework::XTabBar>& rxViewTabBar);
};

} // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
