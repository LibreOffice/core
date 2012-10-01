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


#include "modulepcr.hxx"

//---------------------------------------------------------------------------------------

using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

//---------------------------------------------------------------------------------------

extern "C" void SAL_CALL createRegistryInfo_OPropertyBrowserController();
extern "C" void SAL_CALL createRegistryInfo_FormController();
extern "C" void SAL_CALL createRegistryInfo_DefaultFormComponentInspectorModel();
extern "C" void SAL_CALL createRegistryInfo_DefaultHelpProvider();
extern "C" void SAL_CALL createRegistryInfo_OControlFontDialog();
extern "C" void SAL_CALL createRegistryInfo_OTabOrderDialog();
extern "C" void SAL_CALL createRegistryInfo_CellBindingPropertyHandler();
extern "C" void SAL_CALL createRegistryInfo_ButtonNavigationHandler();
extern "C" void SAL_CALL createRegistryInfo_EditPropertyHandler();
extern "C" void SAL_CALL createRegistryInfo_FormComponentPropertyHandler();
extern "C" void SAL_CALL createRegistryInfo_EFormsPropertyHandler();
extern "C" void SAL_CALL createRegistryInfo_XSDValidationPropertyHandler();
extern "C" void SAL_CALL createRegistryInfo_EventHandler();
extern "C" void SAL_CALL createRegistryInfo_GenericPropertyHandler();
extern "C" void SAL_CALL createRegistryInfo_ObjectInspectorModel();
extern "C" void SAL_CALL createRegistryInfo_SubmissionPropertyHandler();
extern "C" void SAL_CALL createRegistryInfo_StringRepresentation();
extern "C" void SAL_CALL createRegistryInfo_MasterDetailLinkDialog();
extern "C" void SAL_CALL createRegistryInfo_FormGeometryHandler();

//---------------------------------------------------------------------------------------

extern "C" void SAL_CALL pcr_initializeModule()
{
    static sal_Bool s_bInit = sal_False;
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
        s_bInit = sal_True;
    }
}

//---------------------------------------------------------------------------------------
extern "C" SAL_DLLPUBLIC_EXPORT void* SAL_CALL pcr_component_getFactory(
                    const sal_Char* pImplementationName,
                    void* pServiceManager,
                    SAL_UNUSED_PARAMETER void* /*pRegistryKey*/)
{
    pcr_initializeModule();

    Reference< XInterface > xRet;
    if (pServiceManager && pImplementationName)
    {
        xRet = ::pcr::PcrModule::getInstance().getComponentFactory(
            ::rtl::OUString::createFromAscii(pImplementationName));
    }

    if (xRet.is())
        xRet->acquire();
    return xRet.get();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
