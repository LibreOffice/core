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


#include "modulepcr.hxx"
#include "pcrservices.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

extern "C" void pcr_initializeModule()
{
    static bool s_bInit = false;
    if (!s_bInit)
    {
        createRegistryInfo_OPropertyBrowserController();
        createRegistryInfo_FormController();
        createRegistryInfo_DefaultFormComponentInspectorModel();
        createRegistryInfo_DefaultHelpProvider();
        createRegistryInfo_OControlFontDialog();
        createRegistryInfo_OTabOrderDialog();
        createRegistryInfo_CellBindingPropertyHandler();
        createRegistryInfo_ButtonNavigationHandler();
        createRegistryInfo_EditPropertyHandler();
        createRegistryInfo_FormComponentPropertyHandler();
        createRegistryInfo_EFormsPropertyHandler();
        createRegistryInfo_XSDValidationPropertyHandler();
        createRegistryInfo_EventHandler();
        createRegistryInfo_GenericPropertyHandler();
        createRegistryInfo_ObjectInspectorModel();
        createRegistryInfo_SubmissionPropertyHandler();
        createRegistryInfo_StringRepresentation();
        createRegistryInfo_MasterDetailLinkDialog();
        createRegistryInfo_FormGeometryHandler();
        s_bInit = true;
    }
}


extern "C" SAL_DLLPUBLIC_EXPORT void* pcr_component_getFactory(
                    const sal_Char* pImplementationName,
                    void* pServiceManager,
                    SAL_UNUSED_PARAMETER void* /*pRegistryKey*/)
{
    pcr_initializeModule();

    Reference< XInterface > xRet;
    if (pServiceManager && pImplementationName)
    {
        xRet = ::pcr::PcrModule::getInstance().getComponentFactory(
            OUString::createFromAscii(pImplementationName));
    }

    if (xRet.is())
        xRet->acquire();
    return xRet.get();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
