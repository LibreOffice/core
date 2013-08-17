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

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>
#include "module_dba.hxx"
#include <osl/diagnose.h>
#include "DatabaseDataProvider.hxx"
#include "dbadllapi.hxx"

#include <../dataaccess/databasecontext.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

// registry functions
extern "C" void SAL_CALL createRegistryInfo_OCommandDefinition();
extern "C" void SAL_CALL createRegistryInfo_OComponentDefinition();
extern "C" void SAL_CALL createRegistryInfo_ORowSet();
extern "C" void SAL_CALL createRegistryInfo_ODatabaseDocument();
extern "C" void SAL_CALL createRegistryInfo_ODatabaseSource();
extern "C" void SAL_CALL createRegistryInfo_DataAccessDescriptorFactory();

namespace dba{
    ::cppu::ImplementationEntry entries[] = {
        {
            &::dbaccess::DatabaseDataProvider::Create,
            &::dbaccess::DatabaseDataProvider::getImplementationName_Static,
            &::dbaccess::DatabaseDataProvider::getSupportedServiceNames_Static,
            &cppu::createSingleComponentFactory, 0, 0
        },

        {
            &dbaccess::ODatabaseContext::Create,
            &dbaccess::ODatabaseContext::getImplementationName_static,
            &dbaccess::ODatabaseContext::getSupportedServiceNames_static,
            &cppu::createOneInstanceComponentFactory, 0, 0
        },

        { 0, 0, 0, 0, 0, 0 }
    };
}

// The prescribed C api must be complied with
// It consists of three functions which must be exported by the module.
extern "C" void SAL_CALL createRegistryInfo_DBA()
{
    static sal_Bool bInit = sal_False;
    if (!bInit)
    {
        createRegistryInfo_OCommandDefinition();
        createRegistryInfo_OComponentDefinition();
        createRegistryInfo_ORowSet();
        createRegistryInfo_ODatabaseDocument();
        createRegistryInfo_ODatabaseSource();
        createRegistryInfo_DataAccessDescriptorFactory();
        bInit = sal_True;
    }
}

extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL dba_component_getFactory(
                    const sal_Char* pImplementationName,
                    void* pServiceManager,
                    void* pRegistryKey)
{
    createRegistryInfo_DBA();

    Reference< XInterface > xRet;
    if (pServiceManager && pImplementationName)
    {
        xRet = ::dba::DbaModule::getInstance().getComponentFactory(
            OUString::createFromAscii( pImplementationName ) );
    }

    if (xRet.is())
        xRet->acquire();
    else
        return cppu::component_getFactoryHelper(
            pImplementationName, pServiceManager, pRegistryKey, dba::entries);
    return xRet.get();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
