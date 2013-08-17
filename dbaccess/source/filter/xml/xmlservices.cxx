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
#include "flt_reghelper.hxx"

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

extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL dbaxml_component_getFactory(
                    const sal_Char* pImplementationName,
                    void* pServiceManager,
                    void* /*pRegistryKey*/)
{
    createRegistryInfo_dbaxml();

    Reference< XInterface > xRet;
    if (pServiceManager && pImplementationName)
    {
        xRet = ::dbaxml::OModuleRegistration::getComponentFactory(
            OUString::createFromAscii(pImplementationName),
            static_cast< XMultiServiceFactory* >(pServiceManager));
    }

    if (xRet.is())
        xRet->acquire();
    return xRet.get();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
