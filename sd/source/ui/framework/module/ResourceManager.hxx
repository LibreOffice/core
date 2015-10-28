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

#ifndef INCLUDED_SD_SOURCE_UI_FRAMEWORK_MODULE_RESOURCEMANAGER_HXX
#define INCLUDED_SD_SOURCE_UI_FRAMEWORK_MODULE_RESOURCEMANAGER_HXX

#include "MutexOwner.hxx"
#include <com/sun/star/drawing/framework/XConfigurationChangeListener.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <cppuhelper/compbase.hxx>
#include <memory>

namespace {

typedef ::cppu::WeakComponentImplHelper <
    css::drawing::framework::XConfigurationChangeListener
    > ResourceManagerInterfaceBase;

} // end of anonymous namespace.

namespace sd { namespace framework {

/** Manage the activation state of one resource depending on the view in the
    center pane.  The ResourceManager remembers in which configuration to
    activate and in which to deactivate the resource.  When the resource is
    deactivated or activated manually by the user then the ResourceManager
    detects this and remembers it for the future.
*/
class ResourceManager
    : private sd::MutexOwner,
      public ResourceManagerInterfaceBase
{
public:
    ResourceManager (
        const css::uno::Reference<css::frame::XController>& rxController,
        const css::uno::Reference<css::drawing::framework::XResourceId>& rxResourceId);
    virtual ~ResourceManager();

    /** Remember the given URL as one of a center pane view for which to
        activate the resource managed by the called object.
    */
    void AddActiveMainView (const OUString& rsMainViewURL);
    bool IsResourceActive (const OUString& rsMainViewURL);
    virtual void SaveResourceState();

    virtual void SAL_CALL disposing() override;

    // XConfigurationChangeListener

    virtual void SAL_CALL notifyConfigurationChange (
        const css::drawing::framework::ConfigurationChangeEvent& rEvent)
        throw (css::uno::RuntimeException, std::exception) override;

    // XEventListener

    virtual void SAL_CALL disposing (
        const css::lang::EventObject& rEvent)
        throw (css::uno::RuntimeException, std::exception) override;

protected:
    css::uno::Reference<css::drawing::framework::XConfigurationController>
        mxConfigurationController;

private:
    class MainViewContainer;
    std::unique_ptr<MainViewContainer> mpActiveMainViewContainer;

    /// The resource managed by this class.
    css::uno::Reference<css::drawing::framework::XResourceId> mxResourceId;

    /// The anchor of the main view.
    css::uno::Reference<css::drawing::framework::XResourceId> mxMainViewAnchorId;

    OUString msCurrentMainViewURL;

    void HandleMainViewSwitch (
        const OUString& rsViewURL,
        const css::uno::Reference<css::drawing::framework::XConfiguration>& rxConfiguration,
        const bool bIsActivated);
    void HandleResourceRequest(
        bool bActivation,
        const css::uno::Reference<css::drawing::framework::XConfiguration>& rxConfiguration);
    void UpdateForMainViewShell();
};

} } // end of namespace sd::framework

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
