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

#include "precompiled_configmgr.hxx"
#include "sal/config.h"

#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "osl/diagnose.h"
#include "uno/lbnames.h"
#include "sal/types.h"

#include "configurationprovider.hxx"
#include "configurationregistry.hxx"
#include "defaultprovider.hxx"
#include "update.hxx"

namespace {

namespace css = com::sun::star;

css::uno::Reference< css::uno::XInterface > SAL_CALL dummy(
    css::uno::Reference< css::uno::XComponentContext > const &)
    SAL_THROW((css::uno::Exception))
{
    OSL_ASSERT(false);
    return css::uno::Reference< css::uno::XInterface >();
}

static cppu::ImplementationEntry const services[] = {
    { &dummy, &configmgr::configuration_provider::getImplementationName,
      &configmgr::configuration_provider::getSupportedServiceNames,
      &configmgr::configuration_provider::createFactory, 0, 0 },
    { &configmgr::default_provider::create,
      &configmgr::default_provider::getImplementationName,
      &configmgr::default_provider::getSupportedServiceNames,
      &cppu::createSingleComponentFactory, 0, 0 },
    { &configmgr::configuration_registry::create,
      &configmgr::configuration_registry::getImplementationName,
      &configmgr::configuration_registry::getSupportedServiceNames,
      &cppu::createSingleComponentFactory, 0, 0 },
    { &configmgr::update::create, &configmgr::update::getImplementationName,
      &configmgr::update::getSupportedServiceNames,
      &cppu::createSingleComponentFactory, 0, 0 },
    { 0, 0, 0, 0, 0, 0 }
};

}

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    char const * pImplName, void * pServiceManager, void * pRegistryKey)
{
    return cppu::component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey, services);
}

extern "C" SAL_DLLPUBLIC_EXPORT void SAL_CALL
component_getImplementationEnvironment(
    char const ** ppEnvTypeName, uno_Environment **)
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
