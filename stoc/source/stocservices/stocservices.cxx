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


#include "stocservices.hxx"

#include "cppuhelper/factory.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "sal/types.h"
#include "uno/environment.h"
#include "uno/lbnames.h"

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace cppu;
using namespace osl;
using namespace stoc_services;

#ifdef DISABLE_DYNLOADING

// Do we really to have non-static global variable with the same name
// g_moduleCount in *all* (more or less, it seems) modules even in the
// normal dynamic loading case? Weird. Anyway, in the DISABLE_DYNLOADING
// case we have no use for these, and they can't be the same name.

#define g_moduleCount g_stocservices_moduleCount

#endif

rtl_StandardModuleCount g_moduleCount = MODULE_COUNT_INIT;

static struct ImplementationEntry g_entries[] =
{
    // typeconv
    {
    TypeConverter_Impl_CreateInstance, tcv_getImplementationName,
    tcv_getSupportedServiceNames, createSingleComponentFactory,
    &g_moduleCount.modCnt , 0
    },
    // uriproc
    {
    ExternalUriReferenceTranslator::create,
    ExternalUriReferenceTranslator::getImplementationName,
    ExternalUriReferenceTranslator::getSupportedServiceNames,
    createSingleComponentFactory, 0, 0
    },
    {
    UriReferenceFactory::create,
    UriReferenceFactory::getImplementationName,
    UriReferenceFactory::getSupportedServiceNames,
    createSingleComponentFactory, 0, 0
    },
    {
    UriSchemeParser_vndDOTsunDOTstarDOTexpand::create,
    UriSchemeParser_vndDOTsunDOTstarDOTexpand::getImplementationName,
    UriSchemeParser_vndDOTsunDOTstarDOTexpand::getSupportedServiceNames,
    createSingleComponentFactory, 0, 0
    },
    {
    UriSchemeParser_vndDOTsunDOTstarDOTscript::create,
    UriSchemeParser_vndDOTsunDOTstarDOTscript::getImplementationName,
    UriSchemeParser_vndDOTsunDOTstarDOTscript::getSupportedServiceNames,
    createSingleComponentFactory, 0, 0
    },
    {
    VndSunStarPkgUrlReferenceFactory::create,
    VndSunStarPkgUrlReferenceFactory::getImplementationName,
    VndSunStarPkgUrlReferenceFactory::getSupportedServiceNames,
    createSingleComponentFactory, 0, 0
    },
    { 0, 0, 0, 0, 0, 0 }
};

extern "C"
{

#ifndef DISABLE_DYNLOADING

SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL component_canUnload( TimeValue *pTime )
{
    return g_moduleCount.canUnload( &g_moduleCount , pTime );
}

#endif

SAL_DLLPUBLIC_EXPORT void * SAL_CALL stocservices_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , g_entries );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
