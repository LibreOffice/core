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
#include "precompiled_dbui.hxx"

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _DBU_REGHELPER_HXX_
#include "dbu_reghelper.hxx"
#endif
#ifndef INCLUDED_DBACCESSDLLAPI_H
#include "dbaccessdllapi.h"
#endif

/********************************************************************************************/

using namespace ::dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

//***************************************************************************************
//
// registry functions
extern "C" void SAL_CALL createRegistryInfo_OTableFilterDialog();
extern "C" void SAL_CALL createRegistryInfo_ODataSourcePropertyDialog();
extern "C" void SAL_CALL createRegistryInfo_OSQLMessageDialog();
extern "C" void SAL_CALL createRegistryInfo_OBrowser();
extern "C" void SAL_CALL createRegistryInfo_OFormGridView();
extern "C" void SAL_CALL createRegistryInfo_DBContentLoader();
extern "C" void SAL_CALL writeDBLoaderInfo(void* pRegistryKey);
extern "C" void SAL_CALL createRegistryInfo_OInteractionHandler();
extern "C" void SAL_CALL createRegistryInfo_SbaXGridControl();
extern "C" void SAL_CALL createRegistryInfo_OQueryControl();
extern "C" void SAL_CALL createRegistryInfo_OViewControl();
extern "C" void SAL_CALL createRegistryInfo_OTableControl();
extern "C" void SAL_CALL createRegistryInfo_ORelationControl();
extern "C" void SAL_CALL createRegistryInfo_ComposerDialogs();
extern "C" void SAL_CALL createRegistryInfo_ODBApplication();
extern "C" void SAL_CALL createRegistryInfo_ODirectSQLDialog();
extern "C" void SAL_CALL createRegistryInfo_OAdvancedSettingsDialog();
extern "C" void SAL_CALL createRegistryInfo_ODBTypeWizDialog();
extern "C" void SAL_CALL createRegistryInfo_OUserSettingsDialog();
extern "C" void SAL_CALL createRegistryInfo_ODBTypeWizDialogSetup();
extern "C" void SAL_CALL createRegistryInfo_OColumnControlModel();
extern "C" void SAL_CALL createRegistryInfo_OColumnControl();
extern "C" void SAL_CALL createRegistryInfo_OToolboxController();
extern "C" void SAL_CALL createRegistryInfo_CopyTableWizard();
extern "C" void SAL_CALL createRegistryInfo_OTextConnectionSettingsDialog();

//***************************************************************************************
extern "C" void SAL_CALL createRegistryInfo_DBU()
{
    static sal_Bool bInit = sal_False;
    if (!bInit)
    {
        createRegistryInfo_OTableFilterDialog();
        createRegistryInfo_ODataSourcePropertyDialog();
        createRegistryInfo_OSQLMessageDialog();
        createRegistryInfo_OBrowser();
        createRegistryInfo_OFormGridView();
        createRegistryInfo_DBContentLoader();
        createRegistryInfo_OInteractionHandler();
        createRegistryInfo_SbaXGridControl();
        createRegistryInfo_OQueryControl();
        createRegistryInfo_OViewControl();
        createRegistryInfo_OTableControl();
        createRegistryInfo_ORelationControl();
        createRegistryInfo_ComposerDialogs();
        createRegistryInfo_ODBApplication();
        createRegistryInfo_ODirectSQLDialog();
        createRegistryInfo_OAdvancedSettingsDialog();
        createRegistryInfo_ODBTypeWizDialog();
        createRegistryInfo_ODBTypeWizDialogSetup();
        createRegistryInfo_OUserSettingsDialog();
        createRegistryInfo_OColumnControlModel();
        createRegistryInfo_OColumnControl();
        createRegistryInfo_OToolboxController();
        createRegistryInfo_CopyTableWizard();
        createRegistryInfo_OTextConnectionSettingsDialog();
        bInit = sal_True;
    }
}

//---------------------------------------------------------------------------------------

extern "C" DBACCESS_DLLPUBLIC void SAL_CALL component_getImplementationEnvironment(
                const sal_Char  **ppEnvTypeName,
                uno_Environment **
            )
{
    createRegistryInfo_DBU();
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

//---------------------------------------------------------------------------------------
extern "C" DBACCESS_DLLPUBLIC void* SAL_CALL component_getFactory(
                    const sal_Char* pImplementationName,
                    void* pServiceManager,
                    void* /*pRegistryKey*/)
{
    Reference< XInterface > xRet;
    if (pServiceManager && pImplementationName)
    {
        xRet = ::dbaui::OModuleRegistration::getComponentFactory(
            ::rtl::OUString::createFromAscii(pImplementationName),
            static_cast< XMultiServiceFactory* >(pServiceManager));
    }

    if (xRet.is())
        xRet->acquire();
    return xRet.get();
};
