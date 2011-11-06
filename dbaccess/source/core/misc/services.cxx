/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
// Die vorgeschriebene C-Api muss erfuellt werden!
// Sie besteht aus drei Funktionen, die von dem Modul exportiert werden muessen.
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
