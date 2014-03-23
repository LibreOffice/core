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
#include <osl/diagnose.h>
#include "flt_reghelper.hxx"
#include "dbaccessdllapi.h"

/********************************************************************************************/

using namespace ::dbaxml;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

//***************************************************************************************
//
// registry functions
extern "C" void SAL_CALL createRegistryInfo_ODBFilter();
extern "C" void SAL_CALL createRegistryInfo_ODBFilterExport();
extern "C" void SAL_CALL createRegistryInfo_OSettingsExport();
extern "C" void SAL_CALL createRegistryInfo_OFullExport();
extern "C" void SAL_CALL createRegistryInfo_DBContentLoader2();
extern "C" void SAL_CALL createRegistryInfo_DBTypeDetection();
extern "C" void SAL_CALL writeDBLoaderInfo2(void* pRegistryKey);

//***************************************************************************************
extern "C" void SAL_CALL createRegistryInfo_dbaxml()
{
    static sal_Bool bInit = sal_False;
    if (!bInit)
    {
        createRegistryInfo_DBTypeDetection();
        createRegistryInfo_ODBFilter();
        createRegistryInfo_ODBFilterExport();
        createRegistryInfo_OSettingsExport();
        createRegistryInfo_OFullExport();
        createRegistryInfo_DBContentLoader2();
        bInit = sal_True;
    }
}

//---------------------------------------------------------------------------------------

extern "C" DBACCESS_DLLPUBLIC void component_getImplementationEnvironment(
                const sal_Char  **ppEnvTypeName,
                uno_Environment **
            )
{
    createRegistryInfo_dbaxml();
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//---------------------------------------------------------------------------------------

extern "C" DBACCESS_DLLPUBLIC void* component_getFactory(
                    const sal_Char* pImplementationName,
                    void* pServiceManager,
                    void* /*pRegistryKey*/)
{
    Reference< XInterface > xRet;
    if (pServiceManager && pImplementationName)
    {
        xRet = ::dbaxml::OModuleRegistration::getComponentFactory(
            ::rtl::OUString::createFromAscii(pImplementationName),
            static_cast< XMultiServiceFactory* >(pServiceManager));
    }

    if (xRet.is())
        xRet->acquire();
    return xRet.get();
};

