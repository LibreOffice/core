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
#include <osl/diagnose.h>
#include "dbu_reghelper.hxx"
#include "dbaccessdllapi.h"

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
extern "C" void SAL_CALL createRegistryInfo_OStatusbarController();
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
        createRegistryInfo_OStatusbarController();
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
