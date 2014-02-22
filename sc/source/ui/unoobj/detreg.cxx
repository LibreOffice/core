/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "scdetect.hxx"
#include "exceldetect.hxx"
#include <cppuhelper/implementationentry.hxx>

namespace {

static const cppu::ImplementationEntry spServices[] =
{
    {
        ScFilterDetect::impl_createInstance,
        ScFilterDetect::impl_getStaticImplementationName,
        ScFilterDetect::impl_getStaticSupportedServiceNames,
        cppu::createSingleComponentFactory,
        0, 0
    },

    {
        ScExcelBiffDetect::impl_createInstance,
        ScExcelBiffDetect::impl_getStaticImplementationName,
        ScExcelBiffDetect::impl_getStaticSupportedServiceNames,
        cppu::createSingleComponentFactory,
        0, 0
    },

    { 0, 0, 0, 0, 0, 0 }
};

}

extern "C" {

SAL_DLLPUBLIC_EXPORT void* SAL_CALL scd_component_getFactory(
    const char* pImplName, void* pServiceManager, void* pRegistryKey )
{
    return ::cppu::component_getFactoryHelper(pImplName, pServiceManager, pRegistryKey, spServices);
}

} 



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
