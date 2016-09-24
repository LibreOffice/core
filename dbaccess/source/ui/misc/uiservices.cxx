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
#include <osl/diagnose.h>
#include "dbu_reghelper.hxx"
#include "uiservices.hxx"

using namespace ::dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

extern "C" void SAL_CALL createRegistryInfo_DBU()
{
    static bool bInit = false;
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
        createRegistryInfo_LimitBoxController();
        bInit = true;
    }
}

extern "C"  SAL_DLLPUBLIC_EXPORT void* SAL_CALL dbu_component_getFactory(
                    const sal_Char* pImplementationName,
                    void* pServiceManager,
                    void* /*pRegistryKey*/)
{
    createRegistryInfo_DBU();

    Reference< XInterface > xRet;
    if (pServiceManager && pImplementationName)
    {
        xRet = ::dbaui::OModuleRegistration::getComponentFactory(
            OUString::createFromAscii(pImplementationName),
            static_cast< XMultiServiceFactory* >(pServiceManager));
    }

    if (xRet.is())
        xRet->acquire();
    return xRet.get();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
