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


#include <osl/diagnose.h>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>

#include <com/sun/star/registry/XRegistryKey.hpp>

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

#include "factreg.hxx"

namespace io_stm
{
    rtl_StandardModuleCount g_moduleCount = MODULE_COUNT_INIT;
}

using namespace io_stm;

static struct ImplementationEntry g_entries[] =
{
    {
        OPipeImpl_CreateInstance, OPipeImpl_getImplementationName ,
        OPipeImpl_getSupportedServiceNames, createSingleComponentFactory ,
        &g_moduleCount.modCnt , 0
    },
    {
        OPumpImpl_CreateInstance, OPumpImpl_getImplementationName ,
        OPumpImpl_getSupportedServiceNames, createSingleComponentFactory ,
        &g_moduleCount.modCnt , 0
    },
    {
        ODataInputStream_CreateInstance, ODataInputStream_getImplementationName,
        ODataInputStream_getSupportedServiceNames, createSingleComponentFactory,
        &g_moduleCount.modCnt, 0
    },
    {
        ODataOutputStream_CreateInstance, ODataOutputStream_getImplementationName,
        ODataOutputStream_getSupportedServiceNames, createSingleComponentFactory,
        &g_moduleCount.modCnt, 0
    },
    {
        OObjectInputStream_CreateInstance, OObjectInputStream_getImplementationName,
        OObjectInputStream_getSupportedServiceNames, createSingleComponentFactory,
        &g_moduleCount.modCnt, 0
    },
    {
        OObjectOutputStream_CreateInstance, OObjectOutputStream_getImplementationName,
        OObjectOutputStream_getSupportedServiceNames, createSingleComponentFactory,
        &g_moduleCount.modCnt, 0
    },
    {
        OMarkableInputStream_CreateInstance, OMarkableInputStream_getImplementationName,
        OMarkableInputStream_getSupportedServiceNames, createSingleComponentFactory,
        &g_moduleCount.modCnt, 0
    },
    {
        OMarkableOutputStream_CreateInstance, OMarkableOutputStream_getImplementationName,
        OMarkableOutputStream_getSupportedServiceNames, createSingleComponentFactory,
        &g_moduleCount.modCnt, 0
    },
    { 0, 0, 0, 0, 0, 0 }

};

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL streams_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , g_entries );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
