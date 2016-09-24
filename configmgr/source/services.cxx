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

#include <sal/config.h>

#include <cassert>

#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <sal/types.h>

#include "configurationprovider.hxx"
#include "configurationregistry.hxx"
#include "defaultprovider.hxx"
#include "readonlyaccess.hxx"
#include "readwriteaccess.hxx"
#include "update.hxx"

namespace {

css::uno::Reference< css::uno::XInterface > SAL_CALL dummy(
    SAL_UNUSED_PARAMETER css::uno::Reference< css::uno::XComponentContext >
        const &)
{
    assert(false);
    return css::uno::Reference< css::uno::XInterface >();
}

static cppu::ImplementationEntry const services[] = {
    { &dummy, &configmgr::configuration_provider::getImplementationName,
      &configmgr::configuration_provider::getSupportedServiceNames,
      &configmgr::configuration_provider::createFactory, nullptr, 0 },
    { &configmgr::default_provider::create,
      &configmgr::default_provider::getImplementationName,
      &configmgr::default_provider::getSupportedServiceNames,
      &cppu::createSingleComponentFactory, nullptr, 0 },
    { &configmgr::configuration_registry::create,
      &configmgr::configuration_registry::getImplementationName,
      &configmgr::configuration_registry::getSupportedServiceNames,
      &cppu::createSingleComponentFactory, nullptr, 0 },
    { &configmgr::read_only_access::create,
      &configmgr::read_only_access::getImplementationName,
      &configmgr::read_only_access::getSupportedServiceNames,
      &cppu::createSingleComponentFactory, nullptr, 0 },
    { &configmgr::read_write_access::create,
      &configmgr::read_write_access::getImplementationName,
      &configmgr::read_write_access::getSupportedServiceNames,
      &cppu::createSingleComponentFactory, nullptr, 0 },
    { &configmgr::update::create, &configmgr::update::getImplementationName,
      &configmgr::update::getSupportedServiceNames,
      &cppu::createSingleComponentFactory, nullptr, 0 },
    { nullptr, nullptr, nullptr, nullptr, nullptr, 0 }
};

}

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL configmgr_component_getFactory(
    char const * pImplName, void * pServiceManager, void * pRegistryKey)
{
    return cppu::component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey, services);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
