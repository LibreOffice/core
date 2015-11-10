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

/* genericfilter: mostly generic code for registering the filter */

/* "This product is not manufactured, approved, or supported by
 * Corel Corporation or Corel Corporation Limited."
 */

#include "sal/config.h"

#include "cppuhelper/factory.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "sal/types.h"

#include "CDRImportFilter.hxx"
#include "CMXImportFilter.hxx"
#include "MSPUBImportFilter.hxx"
#include "MWAWDrawImportFilter.hxx"
#include "PageMakerImportFilter.hxx"
#include "FreehandImportFilter.hxx"
#include "VisioImportFilter.hxx"
#include "WPGImportFilter.hxx"

namespace
{

static cppu::ImplementationEntry const services[] =
{
    {
        &CDRImportFilter_createInstance, &CDRImportFilter_getImplementationName,
        &CDRImportFilter_getSupportedServiceNames,
        &cppu::createSingleComponentFactory, nullptr, 0
    },
    {
        &CMXImportFilter_createInstance, &CMXImportFilter_getImplementationName,
        &CMXImportFilter_getSupportedServiceNames,
        &cppu::createSingleComponentFactory, nullptr, 0
    },
    {
        &FreehandImportFilter_createInstance,
        &FreehandImportFilter_getImplementationName,
        &FreehandImportFilter_getSupportedServiceNames,
        &cppu::createSingleComponentFactory, nullptr, 0
    },
    {
        &PageMakerImportFilter_createInstance,
        &PageMakerImportFilter_getImplementationName,
        &PageMakerImportFilter_getSupportedServiceNames,
        &cppu::createSingleComponentFactory, nullptr, 0
    },
    {
        &MSPUBImportFilter_createInstance,
        &MSPUBImportFilter_getImplementationName,
        &MSPUBImportFilter_getSupportedServiceNames,
        &cppu::createSingleComponentFactory, nullptr, 0
    },
    {
        &MWAWDrawImportFilter_createInstance,
        &MWAWDrawImportFilter_getImplementationName,
        &MWAWDrawImportFilter_getSupportedServiceNames,
        &cppu::createSingleComponentFactory, nullptr, 0
    },
    {
        &VisioImportFilter_createInstance,
        &VisioImportFilter_getImplementationName,
        &VisioImportFilter_getSupportedServiceNames,
        &cppu::createSingleComponentFactory, nullptr, 0
    },
    {
        &WPGImportFilter_createInstance, &WPGImportFilter_getImplementationName,
        &WPGImportFilter_getSupportedServiceNames,
        &cppu::createSingleComponentFactory, nullptr, 0
    },
    { nullptr, nullptr, nullptr, nullptr, nullptr, 0 }
};

}

extern "C" SAL_DLLPUBLIC_EXPORT void *SAL_CALL wpftdraw_component_getFactory(
    char const *pImplName, void *pServiceManager, void *pRegistryKey)
{
    return cppu::component_getFactoryHelper(
               pImplName, pServiceManager, pRegistryKey, services);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
