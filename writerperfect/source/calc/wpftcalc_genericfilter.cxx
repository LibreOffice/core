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

#include "MSWorksCalcImportFilter.hxx"
#include "MWAWCalcImportFilter.hxx"
#include "NumbersImportFilter.hxx"

namespace
{

static cppu::ImplementationEntry const services[] =
{
    {
        &MSWorksCalcImportFilter_createInstance, &MSWorksCalcImportFilter_getImplementationName,
        &MSWorksCalcImportFilter_getSupportedServiceNames,
        &cppu::createSingleComponentFactory, nullptr, 0
    },
    {
        &MWAWCalcImportFilter_createInstance, &MWAWCalcImportFilter_getImplementationName,
        &MWAWCalcImportFilter_getSupportedServiceNames,
        &cppu::createSingleComponentFactory, nullptr, 0
    },
    {
        &NumbersImportFilter_createInstance, &NumbersImportFilter_getImplementationName,
        &NumbersImportFilter_getSupportedServiceNames,
        &cppu::createSingleComponentFactory, nullptr, 0
    },
    { nullptr, nullptr, nullptr, nullptr, nullptr, 0 }
};

}

extern "C" SAL_DLLPUBLIC_EXPORT void *SAL_CALL wpftcalc_component_getFactory(
    char const *pImplName, void *pServiceManager, void *pRegistryKey)
{
    return cppu::component_getFactoryHelper(
               pImplName, pServiceManager, pRegistryKey, services);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
