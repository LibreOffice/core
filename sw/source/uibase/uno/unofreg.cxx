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

#include <config_features.h>

#include "SwXFilterOptions.hxx"
#include "unofreg.hxx"
#include <sal/types.h>
#include <osl/diagnose.h>
#include <cppuhelper/factory.hxx>
#include <sfx2/sfxmodelfactory.hxx>

#include <string.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;

// #i73788#
#include <cppuhelper/implementationentry.hxx>
#include <finalthreadmanager.hxx>

extern "C"
{

static ::cppu::ImplementationEntry const entries[] = {
    { &comp_FinalThreadManager::_create,
      &comp_FinalThreadManager::_getImplementationName,
      &comp_FinalThreadManager::_getSupportedServiceNames,
      &::cppu::createSingleComponentFactory, 0, 0 },
    { 0, 0, 0, 0, 0, 0 }
};

SAL_DLLPUBLIC_EXPORT void * SAL_CALL sw_component_getFactory(
    const sal_Char * pImplName,
    void * pServiceManager,
    void * pRegistryKey )
{
    void * pRet = 0;
    if( pServiceManager )
    {
        uno::Reference< XMultiServiceFactory > xMSF(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ) );

        uno::Reference< XSingleServiceFactory > xFactory;

        const sal_Int32 nImplNameLen = strlen( pImplName );
        if( SwXMLImport_getImplementationName().equalsAsciiL( pImplName,
                                                              nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SwXMLImport_getImplementationName(),
                SwXMLImport_createInstance,
                SwXMLImport_getSupportedServiceNames() );
        }
        else if( SwXMLImportStyles_getImplementationName().equalsAsciiL(
                                                    pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SwXMLImportStyles_getImplementationName(),
                SwXMLImportStyles_createInstance,
                SwXMLImportStyles_getSupportedServiceNames() );
        }
        else if( SwXMLImportContent_getImplementationName().equalsAsciiL(
                                                    pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SwXMLImportContent_getImplementationName(),
                SwXMLImportContent_createInstance,
                SwXMLImportContent_getSupportedServiceNames() );
        }
        else if( SwXMLImportMeta_getImplementationName().equalsAsciiL(
                                                    pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SwXMLImportMeta_getImplementationName(),
                SwXMLImportMeta_createInstance,
                SwXMLImportMeta_getSupportedServiceNames() );
        }
        else if( SwXMLImportSettings_getImplementationName().equalsAsciiL(
                                                    pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SwXMLImportSettings_getImplementationName(),
                SwXMLImportSettings_createInstance,
                SwXMLImportSettings_getSupportedServiceNames() );
        }
        else if( SwXMLExportOOO_getImplementationName().equalsAsciiL( pImplName,
                                                            nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SwXMLExportOOO_getImplementationName(),
                SwXMLExportOOO_createInstance,
                SwXMLExportOOO_getSupportedServiceNames() );
        }
        else if( SwXMLExportStylesOOO_getImplementationName().equalsAsciiL(
                                                    pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SwXMLExportStylesOOO_getImplementationName(),
                SwXMLExportStylesOOO_createInstance,
                SwXMLExportStylesOOO_getSupportedServiceNames() );
        }
        else if( SwXMLExportContentOOO_getImplementationName().equalsAsciiL(
                                                    pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SwXMLExportContentOOO_getImplementationName(),
                SwXMLExportContentOOO_createInstance,
                SwXMLExportContentOOO_getSupportedServiceNames() );
        }
        else if( SwXMLExportMetaOOO_getImplementationName().equalsAsciiL(
                                                    pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SwXMLExportMetaOOO_getImplementationName(),
                SwXMLExportMetaOOO_createInstance,
                SwXMLExportMetaOOO_getSupportedServiceNames() );
        }
        else if( SwXMLExportSettingsOOO_getImplementationName().equalsAsciiL(
                                                    pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SwXMLExportSettingsOOO_getImplementationName(),
                SwXMLExportSettingsOOO_createInstance,
                SwXMLExportSettingsOOO_getSupportedServiceNames() );
        }
        else if( SwXMLExport_getImplementationName().equalsAsciiL( pImplName,
                                                            nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SwXMLExport_getImplementationName(),
                SwXMLExport_createInstance,
                SwXMLExport_getSupportedServiceNames() );
        }
        else if( SwXMLExportStyles_getImplementationName().equalsAsciiL(
                                                    pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SwXMLExportStyles_getImplementationName(),
                SwXMLExportStyles_createInstance,
                SwXMLExportStyles_getSupportedServiceNames() );
        }
        else if( SwXMLExportContent_getImplementationName().equalsAsciiL(
                                                    pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SwXMLExportContent_getImplementationName(),
                SwXMLExportContent_createInstance,
                SwXMLExportContent_getSupportedServiceNames() );
        }
        else if( SwXMLExportMeta_getImplementationName().equalsAsciiL(
                                                    pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SwXMLExportMeta_getImplementationName(),
                SwXMLExportMeta_createInstance,
                SwXMLExportMeta_getSupportedServiceNames() );
        }
        else if( SwXMLExportSettings_getImplementationName().equalsAsciiL(
                                                    pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SwXMLExportSettings_getImplementationName(),
                SwXMLExportSettings_createInstance,
                SwXMLExportSettings_getSupportedServiceNames() );
        }
        else if( SwXAutoTextContainer_getImplementationName().equalsAsciiL(
                                                    pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SwXAutoTextContainer_getImplementationName(),
                SwXAutoTextContainer_createInstance,
                SwXAutoTextContainer_getSupportedServiceNames() );
        }
        else if( SwXModule_getImplementationName().equalsAsciiL(
                                                    pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SwXModule_getImplementationName(),
                SwXModule_createInstance,
                SwXModule_getSupportedServiceNames() );
        }
#if HAVE_FEATURE_DBCONNECTIVITY
        else if( SwXMailMerge_getImplementationName().equalsAsciiL(
                                                    pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SwXMailMerge_getImplementationName(),
                SwXMailMerge_createInstance,
                SwXMailMerge_getSupportedServiceNames() );
        }
#endif
        else if( SwXFilterOptions::getImplementationName_Static().equalsAsciiL(
                                                    pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SwXFilterOptions::getImplementationName_Static(),
                SwXFilterOptions_createInstance,
                SwXFilterOptions::getSupportedServiceNames_Static() );
        }
        else if( SwTextDocument_getImplementationName().equalsAsciiL(
                                                    pImplName, nImplNameLen ) )
        {
            xFactory = ::sfx2::createSfxModelFactory( xMSF,
                SwTextDocument_getImplementationName(),
                SwTextDocument_createInstance,
                SwTextDocument_getSupportedServiceNames() );
        }
        else if( SwWebDocument_getImplementationName().equalsAsciiL(
                                                    pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SwWebDocument_getImplementationName(),
                SwWebDocument_createInstance,
                SwWebDocument_getSupportedServiceNames() );
        }
        else if( SwGlobalDocument_getImplementationName().equalsAsciiL(
                                                    pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SwGlobalDocument_getImplementationName(),
                SwGlobalDocument_createInstance,
                SwGlobalDocument_getSupportedServiceNames() );
        }
        else if( SwUnoModule_getImplementationName().equalsAsciiL(
                                                    pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SwUnoModule_getImplementationName(),
                SwUnoModule_createInstance,
                SwUnoModule_getSupportedServiceNames() );
        }
        else if( LayoutDumpFilter_getImplementationName().equalsAsciiL(
                                                    pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                LayoutDumpFilter_getImplementationName(),
                LayoutDumpFilter_createInstance,
                LayoutDumpFilter_getSupportedServiceNames() );
        }
        else if( comp_FinalThreadManager::_getImplementationName().equalsAsciiL(
                                                    pImplName, nImplNameLen ) )
        {
            pRet = ::cppu::component_getFactoryHelper(
                        pImplName, pServiceManager, pRegistryKey, entries);
        }

        if( xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }
    return pRet;
}

} // extern "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
