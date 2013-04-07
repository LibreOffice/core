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

#include "SwXFilterOptions.hxx"
#include "sal/types.h"
#include <osl/diagnose.h>
#include <uno/lbnames.h>
#include <cppuhelper/factory.hxx>
#include <sfx2/sfxmodelfactory.hxx>

#include <string.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;


// module
extern uno::Sequence< OUString > SAL_CALL SwUnoModule_getSupportedServiceNames() throw();
extern OUString SAL_CALL SwUnoModule_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SwUnoModule_createInstance( const uno::Reference< XMultiServiceFactory > &rSMgr ) throw( uno::Exception );

// writer documents
extern uno::Sequence< OUString > SAL_CALL SwTextDocument_getSupportedServiceNames() throw();
extern OUString SAL_CALL SwTextDocument_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SwTextDocument_createInstance( const uno::Reference< XMultiServiceFactory > &rSMgr, const sal_uInt64 _nCreationFlags ) throw( uno::Exception );
extern uno::Sequence< OUString > SAL_CALL SwWebDocument_getSupportedServiceNames() throw();
extern OUString SAL_CALL SwWebDocument_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SwWebDocument_createInstance( const uno::Reference< XMultiServiceFactory > &rSMgr ) throw( uno::Exception );
extern uno::Sequence< OUString > SAL_CALL SwGlobalDocument_getSupportedServiceNames() throw();
extern OUString SAL_CALL SwGlobalDocument_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SwGlobalDocument_createInstance( const uno::Reference< XMultiServiceFactory > &rSMgr ) throw( uno::Exception );

// xml import
extern uno::Sequence< OUString > SAL_CALL SwXMLImport_getSupportedServiceNames() throw();
extern OUString SAL_CALL SwXMLImport_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SwXMLImport_createInstance(const uno::Reference< XMultiServiceFactory > & rSMgr)  throw( uno::Exception );
extern uno::Sequence< OUString > SAL_CALL SwXMLImportStyles_getSupportedServiceNames() throw();
extern OUString SAL_CALL SwXMLImportStyles_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SwXMLImportStyles_createInstance(const uno::Reference< XMultiServiceFactory > & rSMgr)    throw( uno::Exception );
extern uno::Sequence< OUString > SAL_CALL SwXMLImportContent_getSupportedServiceNames() throw();
extern OUString SAL_CALL SwXMLImportContent_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SwXMLImportContent_createInstance(const uno::Reference< XMultiServiceFactory > & rSMgr)   throw( uno::Exception );

extern uno::Sequence< OUString > SAL_CALL     SwXMLImportMeta_getSupportedServiceNames() throw();
extern OUString SAL_CALL     SwXMLImportMeta_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SwXMLImportMeta_createInstance(const uno::Reference< XMultiServiceFactory > & rSMgr)  throw( uno::Exception );

extern uno::Sequence< OUString > SAL_CALL SwXMLImportSettings_getSupportedServiceNames() throw();
extern OUString SAL_CALL SwXMLImportSettings_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SwXMLImportSettings_createInstance(const uno::Reference< XMultiServiceFactory > & rSMgr)  throw( uno::Exception );

// xml export (OOo)
extern uno::Sequence< OUString > SAL_CALL SwXMLExportOOO_getSupportedServiceNames() throw();
extern OUString SAL_CALL SwXMLExportOOO_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SwXMLExportOOO_createInstance(const uno::Reference< XMultiServiceFactory > & rSMgr)   throw( uno::Exception );
extern uno::Sequence< OUString > SAL_CALL SwXMLExportContentOOO_getSupportedServiceNames() throw();
extern OUString SAL_CALL SwXMLExportContentOOO_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SwXMLExportContentOOO_createInstance(const uno::Reference< XMultiServiceFactory > & rSMgr)    throw( uno::Exception );
extern uno::Sequence< OUString > SAL_CALL SwXMLExportStylesOOO_getSupportedServiceNames() throw();
extern OUString SAL_CALL SwXMLExportStylesOOO_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SwXMLExportStylesOOO_createInstance(const uno::Reference< XMultiServiceFactory > & rSMgr) throw( uno::Exception );
extern uno::Sequence< OUString > SAL_CALL SwXMLExportMetaOOO_getSupportedServiceNames() throw();
extern OUString SAL_CALL SwXMLExportMetaOOO_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SwXMLExportMetaOOO_createInstance(const uno::Reference< XMultiServiceFactory > & rSMgr)   throw( uno::Exception );
extern uno::Sequence< OUString > SAL_CALL SwXMLExportSettingsOOO_getSupportedServiceNames() throw();
extern OUString SAL_CALL SwXMLExportSettingsOOO_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SwXMLExportSettingsOOO_createInstance(const uno::Reference< XMultiServiceFactory > & rSMgr)   throw( uno::Exception );

// xml export (OASIS)
extern uno::Sequence< OUString > SAL_CALL SwXMLExport_getSupportedServiceNames() throw();
extern OUString SAL_CALL SwXMLExport_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SwXMLExport_createInstance(const uno::Reference< XMultiServiceFactory > & rSMgr)  throw( uno::Exception );
extern uno::Sequence< OUString > SAL_CALL SwXMLExportContent_getSupportedServiceNames() throw();
extern OUString SAL_CALL SwXMLExportContent_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SwXMLExportContent_createInstance(const uno::Reference< XMultiServiceFactory > & rSMgr)   throw( uno::Exception );
extern uno::Sequence< OUString > SAL_CALL SwXMLExportStyles_getSupportedServiceNames() throw();
extern OUString SAL_CALL SwXMLExportStyles_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SwXMLExportStyles_createInstance(const uno::Reference< XMultiServiceFactory > & rSMgr)    throw( uno::Exception );
extern uno::Sequence< OUString > SAL_CALL SwXMLExportMeta_getSupportedServiceNames() throw();
extern OUString SAL_CALL SwXMLExportMeta_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SwXMLExportMeta_createInstance(const uno::Reference< XMultiServiceFactory > & rSMgr)  throw( uno::Exception );
extern uno::Sequence< OUString > SAL_CALL SwXMLExportSettings_getSupportedServiceNames() throw();
extern OUString SAL_CALL SwXMLExportSettings_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SwXMLExportSettings_createInstance(const uno::Reference< XMultiServiceFactory > & rSMgr)  throw( uno::Exception );

//API objects
extern uno::Sequence< OUString > SAL_CALL SwXAutoTextContainer_getSupportedServiceNames() throw();
extern OUString SAL_CALL SwXAutoTextContainer_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SwXAutoTextContainer_createInstance(const uno::Reference< XMultiServiceFactory > & rSMgr) throw( uno::Exception );

extern uno::Sequence< OUString > SAL_CALL SwXModule_getSupportedServiceNames() throw();
extern OUString SAL_CALL SwXModule_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SwXModule_createInstance(const uno::Reference< XMultiServiceFactory > & rSMgr) throw( uno::Exception );

extern uno::Sequence< OUString > SAL_CALL SwXMailMerge_getSupportedServiceNames() throw();
extern OUString SAL_CALL SwXMailMerge_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SwXMailMerge_createInstance(const uno::Reference< XMultiServiceFactory > & rSMgr) throw( uno::Exception );

extern uno::Sequence< OUString > SAL_CALL SwXMailMerge_getSupportedServiceNames() throw();
extern OUString SAL_CALL SwXMailMerge_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SwXMailMerge_createInstance(const uno::Reference< XMultiServiceFactory > & rSMgr) throw( uno::Exception );

// Layout dump filter
extern uno::Sequence< OUString > SAL_CALL LayoutDumpFilter_getSupportedServiceNames() throw();
extern OUString SAL_CALL LayoutDumpFilter_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL LayoutDumpFilter_createInstance( const uno::Reference< XMultiServiceFactory > &rSMgr ) throw( uno::Exception );

// #i73788#
#include "cppuhelper/implementationentry.hxx"
namespace comp_FinalThreadManager {

// component and service helper functions:
OUString SAL_CALL _getImplementationName();
com::sun::star::uno::Sequence< OUString > SAL_CALL _getSupportedServiceNames();
com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL _create(
    com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext > const & context );

}

#ifdef __cplusplus
extern "C"
{
#endif

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
        else if( SwXMailMerge_getImplementationName().equalsAsciiL(
                                                    pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SwXMailMerge_getImplementationName(),
                SwXMailMerge_createInstance,
                SwXMailMerge_getSupportedServiceNames() );
        }
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

#ifdef __cplusplus
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
