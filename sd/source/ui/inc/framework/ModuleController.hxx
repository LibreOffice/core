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

#include <com/sun/star/drawing/framework/XModuleController.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <cppuhelper/compbase.hxx>

#include <memory>

namespace com::sun::star::frame { class XController; }
namespace com::sun::star::uno { class XComponentContext; }

namespace sd::framework {

typedef ::cppu::WeakComponentImplHelper <
    css::drawing::framework::XModuleController,
    css::lang::XInitialization
    > ModuleControllerInterfaceBase;

/** The ModuleController has two tasks:

    1. It reads the
    org.openoffice.Office.Impress/MultiPaneGUI/Framework/ResourceFactories
    configuration data that maps from resource URLs to service names of
    factories that can create resources for the URLs.  When the
    configuration controller wants to create a resource for which it does
    not have a factory, it asks the ModuleController to provide one.  The
    ModuleController looks up the service name registered for the URL of the
    resource and instantiates this service.  The service is expected to
    register on its creation a factory for the resource in question.

    2. The ModuleController reads on its creation
    org.openoffice.Office.Impress/MultiPaneGUI/Framework/StartupServices
    configuration data and instantiates all listed services.  These services
    can then register as listeners at the ConfigurationController or do
    whatever they like.
*/
class ModuleController
    : private sd::MutexOwner,
      public ModuleControllerInterfaceBase
{
public:
    static css::uno::Reference<
        css::drawing::framework::XModuleController>
        CreateInstance (
            const css::uno::Reference<css::uno::XComponentContext>&
            rxContext);

    virtual void SAL_CALL disposing() override;

    // XModuleController

    virtual void SAL_CALL requestResource(const OUString& rsResourceURL) override;

    // XInitialization

    virtual void SAL_CALL initialize(
        const css::uno::Sequence<css::uno::Any>& aArguments) override;

private:
    css::uno::Reference<
        css::frame::XController> mxController;

    class ResourceToFactoryMap;
    std::unique_ptr<ResourceToFactoryMap> mpResourceToFactoryMap;
    class LoadedFactoryContainer;
    std::unique_ptr<LoadedFactoryContainer> mpLoadedFactories;

    /// @throws std::exception
    ModuleController (
        const css::uno::Reference<css::uno::XComponentContext>& rxContext);
    ModuleController (const ModuleController&) = delete;
    virtual ~ModuleController() throw() override;

    /** Called for every entry in the ResourceFactories configuration entry.
    */
    void ProcessFactory (const ::std::vector<css::uno::Any>& rValues);

    /** Instantiate all startup services that are found in the
        /org.openoffice.Office.Impress/MultiPaneGUI/Framework/StartupServices
        configuration entry.  This method is called once when a new
        ModuleController object is created.
    */
    void InstantiateStartupServices();

    /** Called for one entry in the StartupServices configuration list this
        method instantiates the service described by the entry.  It does not
        hold references to the new object so that the object will be
        destroyed on function exit when it does not register itself
        somewhere.  It typically will register as
        XConfigurationChangeListener at the configuration controller.
    */
    void ProcessStartupService (const ::std::vector<css::uno::Any>& rValues);
};

} // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
