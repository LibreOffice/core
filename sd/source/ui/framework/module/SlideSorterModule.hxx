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

#include <ResourceId.hxx>
#include <framework/ConfigurationChangeListener.hxx>
#include <comphelper/compbase.hxx>
#include <rtl/ref.hxx>
#include <memory>
#include <set>

namespace sd { class DrawController; }
namespace sd { class ViewTabBar; }

namespace sd::framework
{
class ConfigurationController;
class Configuration;


/** This module is responsible for showing the slide sorter bar and the
    slide sorter view in the center pane.

    Manage the activation state of one resource depending on the view in the
    center pane.  The ResourceManager remembers in which configuration to
    activate and in which to deactivate the resource.  When the resource is
    deactivated or activated manually by the user then the ResourceManager
    detects this and remembers it for the future.
*/
class SlideSorterModule final
    : public sd::framework::ConfigurationChangeListener
{
public:
    SlideSorterModule (
        const rtl::Reference<::sd::DrawController>& rxController,
        const OUString& rsLeftPaneURL);
    virtual ~SlideSorterModule() override;

    /** Remember the given URL as one of a center pane view for which to
        activate the resource managed by the called object.
    */
    void AddActiveMainView (const OUString& rsMainViewURL);
    bool IsResourceActive (const OUString& rsMainViewURL);
    void SaveResourceState();

    virtual void disposing(std::unique_lock<std::mutex>&) override;

    // ConfigurationChangeListener
    virtual void notifyConfigurationChange (
        const sd::framework::ConfigurationChangeEvent& rEvent) override;

    // XEventListener
    virtual void SAL_CALL disposing (
        const css::lang::EventObject& rEvent) override;

private:
    rtl::Reference<ConfigurationController> mxConfigurationController;
    ::std::set<OUString> maActiveMainViewContainer;
    /// The resource managed by this class.
    rtl::Reference<sd::framework::ResourceId> mxResourceId;
    /// The anchor of the main view.
    rtl::Reference<sd::framework::ResourceId> mxMainViewAnchorId;
    OUString msCurrentMainViewURL;
    rtl::Reference<sd::framework::ResourceId> mxViewTabBarId;
    rtl::Reference<::sd::DrawController> mxControllerManager;

    void HandleMainViewSwitch (
        const OUString& rsViewURL,
        const bool bIsActivated);
    void HandleResourceRequest(
        bool bActivation,
        const rtl::Reference<sd::framework::Configuration>& rxConfiguration);
    void UpdateViewTabBar (
        const rtl::Reference<sd::ViewTabBar>& rxViewTabBar);
};

} // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
