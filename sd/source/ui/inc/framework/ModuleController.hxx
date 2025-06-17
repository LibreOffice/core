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

#include <com/sun/star/lang/XInitialization.hpp>
#include <comphelper/compbase.hxx>
#include <cppuhelper/weakref.hxx>
#include <rtl/ref.hxx>

#include <unordered_map>

namespace sd { class DrawController; }

namespace sd::framework {

typedef comphelper::WeakComponentImplHelper <> ModuleControllerInterfaceBase;

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

    2. The ModuleController instantiates PresentationFactoryProvider.
    This service
    can then register as listeners at the ConfigurationController or do
    whatever they like.
*/
class ModuleController final
    : public ModuleControllerInterfaceBase
{
public:
    /// @throws std::exception
    ModuleController(const rtl::Reference<::sd::DrawController>& rxController);

    virtual void disposing(std::unique_lock<std::mutex>&) override;

    /** When the specified resource is requested for the first time then
        create a new instance of the associated factory service.
    */
    void requestResource(const OUString& rsResourceURL);

private:
    rtl::Reference<::sd::DrawController> mxController;

    std::unordered_map<OUString, OUString> maResourceToFactoryMap;
    std::unordered_map<OUString, css::uno::WeakReference<css::uno::XInterface>> maLoadedFactories;

    ModuleController (const ModuleController&) = delete;
    virtual ~ModuleController() noexcept override;

    /** Called for every entry in the ResourceFactories configuration entry.
    */
    void ProcessFactory (const OUString& ServiceName, ::std::vector<OUString> aURLs);

    /** Instantiate startup services.  This method is called once when a new
        ModuleController object is created.
    */
    void InstantiateStartupServices();
};

} // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
