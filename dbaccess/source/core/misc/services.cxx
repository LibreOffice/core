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
#include "precompiled_dbaccess.hxx"

#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>
#include "module_dba.hxx"
#include <osl/diagnose.h>
#include "DatabaseDataProvider.hxx"
#include "dbadllapi.hxx"

/********************************************************************************************/

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

//***************************************************************************************
//
// registry functions
extern "C" void SAL_CALL createRegistryInfo_ODatabaseContext();
extern "C" void SAL_CALL createRegistryInfo_OCommandDefinition();
extern "C" void SAL_CALL createRegistryInfo_OComponentDefinition();
extern "C" void SAL_CALL createRegistryInfo_ORowSet();
extern "C" void SAL_CALL createRegistryInfo_ODatabaseDocument();
extern "C" void SAL_CALL createRegistryInfo_ODatabaseSource();
extern "C" void SAL_CALL createRegistryInfo_DataAccessDescriptorFactory();

namespace dba{
//--------------------------------------------------------------------------
    ::cppu::ImplementationEntry entries[] = {
        { &::dbaccess::DatabaseDataProvider::Create, &::dbaccess::DatabaseDataProvider::getImplementationName_Static, &::dbaccess::DatabaseDataProvider::getSupportedServiceNames_Static,
            &cppu::createSingleComponentFactory, 0, 0 },
        { 0, 0, 0, 0, 0, 0 }
    };
}

//***************************************************************************************
//
// The prescribed C api must be complied with
// It consists of three functions which must be exported by the module.
//
extern "C" void SAL_CALL createRegistryInfo_DBA()
{
    static sal_Bool bInit = sal_False;
    if (!bInit)
    {
        createRegistryInfo_ODatabaseContext();
        createRegistryInfo_OCommandDefinition();
        createRegistryInfo_OComponentDefinition();
        createRegistryInfo_ORowSet();
        createRegistryInfo_ODatabaseDocument();
        createRegistryInfo_ODatabaseSource();
        createRegistryInfo_DataAccessDescriptorFactory();
        bInit = sal_True;
    }
}

//---------------------------------------------------------------------------------------

extern "C" OOO_DLLPUBLIC_DBA void SAL_CALL component_getImplementationEnvironment(
                const sal_Char  **ppEnvTypeName,
                uno_Environment **
            )
{
    createRegistryInfo_DBA();
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//---------------------------------------------------------------------------------------
extern "C" OOO_DLLPUBLIC_DBA void* SAL_CALL component_getFactory(
                    const sal_Char* pImplementationName,
                    void* pServiceManager,
                    void* pRegistryKey)
{
    Reference< XInterface > xRet;
    if (pServiceManager && pImplementationName)
    {
        xRet = ::dba::DbaModule::getInstance().getComponentFactory(
            ::rtl::OUString::createFromAscii( pImplementationName ),
            static_cast< XMultiServiceFactory* >( pServiceManager ) );
    }

    if (xRet.is())
        xRet->acquire();
    else
        return cppu::component_getFactoryHelper(
            pImplementationName, pServiceManager, pRegistryKey, dba::entries);
    return xRet.get();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
