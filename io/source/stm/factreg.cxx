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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_io.hxx"

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

extern "C"
{

sal_Bool SAL_CALL component_canUnload( TimeValue *pTime )
{
    return g_moduleCount.canUnload( &g_moduleCount , pTime );
}

//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , g_entries );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
