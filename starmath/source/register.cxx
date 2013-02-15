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

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/uno/Sequence.h>
#include <rtl/ustring.hxx>

#include <sfx2/sfxmodelfactory.hxx>

#include "smdll.hxx"
#include "document.hxx"
#include "unomodel.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

//Math document
extern Sequence< OUString > SAL_CALL
        SmDocument_getSupportedServiceNames() throw();
extern OUString SAL_CALL
        SmDocument_getImplementationName() throw();
extern Reference< XInterface >SAL_CALL
        SmDocument_createInstance(const Reference< XMultiServiceFactory > & rSMgr, const sal_uInt64 _nCreationFlags) throw( Exception );

//MathML import
extern Sequence< OUString > SAL_CALL
        SmXMLImport_getSupportedServiceNames() throw();
extern OUString SAL_CALL
        SmXMLImport_getImplementationName() throw();
extern Reference< XInterface > SAL_CALL
        SmXMLImport_createInstance(const Reference< XMultiServiceFactory > & rSMgr) throw( Exception );
extern Sequence< OUString > SAL_CALL
        SmXMLImportMeta_getSupportedServiceNames() throw();
extern OUString SAL_CALL
        SmXMLImportMeta_getImplementationName() throw();
extern Reference< XInterface > SAL_CALL
        SmXMLImportMeta_createInstance(const Reference< XMultiServiceFactory > & rSMgr) throw( Exception );
extern Sequence< OUString > SAL_CALL
        SmXMLImportSettings_getSupportedServiceNames() throw();
extern OUString SAL_CALL SmXMLImportSettings_getImplementationName() throw();
extern Reference< XInterface > SAL_CALL
        SmXMLImportSettings_createInstance(const Reference< XMultiServiceFactory > & rSMgr) throw( Exception );

//MathML export
extern Sequence< OUString > SAL_CALL
        SmXMLExport_getSupportedServiceNames() throw();
extern OUString SAL_CALL
        SmXMLExport_getImplementationName() throw();
extern Reference< XInterface > SAL_CALL
        SmXMLExport_createInstance(const Reference< XMultiServiceFactory > & rSMgr) throw( Exception );
extern Sequence< OUString > SAL_CALL
        SmXMLExportMetaOOO_getSupportedServiceNames() throw();
extern OUString SAL_CALL
        SmXMLExportMetaOOO_getImplementationName() throw();
extern Reference< XInterface > SAL_CALL
        SmXMLExportMetaOOO_createInstance(const Reference< XMultiServiceFactory > & rSMgr) throw( Exception );
extern Sequence< OUString > SAL_CALL
        SmXMLExportMeta_getSupportedServiceNames() throw();
extern OUString SAL_CALL
        SmXMLExportMeta_getImplementationName() throw();
extern Reference< XInterface > SAL_CALL
        SmXMLExportMeta_createInstance(const Reference< XMultiServiceFactory > & rSMgr) throw( Exception );
extern Sequence< OUString > SAL_CALL
        SmXMLExportSettingsOOO_getSupportedServiceNames() throw();
extern OUString SAL_CALL
        SmXMLExportSettingsOOO_getImplementationName() throw();
extern Reference< XInterface > SAL_CALL
        SmXMLExportSettingsOOO_createInstance(const Reference< XMultiServiceFactory > & rSMgr) throw( Exception );
extern Sequence< OUString > SAL_CALL
        SmXMLExportSettings_getSupportedServiceNames() throw();
extern OUString SAL_CALL
        SmXMLExportSettings_getImplementationName() throw();
extern Reference< XInterface > SAL_CALL
        SmXMLExportSettings_createInstance(const Reference< XMultiServiceFactory > & rSMgr) throw( Exception );
extern Sequence< OUString > SAL_CALL
        SmXMLExportContent_getSupportedServiceNames() throw();
extern OUString SAL_CALL
        SmXMLExportContent_getImplementationName() throw();
extern Reference< XInterface > SAL_CALL
        SmXMLExportContent_createInstance(const Reference< XMultiServiceFactory > & rSMgr) throw( Exception );


extern "C" {

SAL_DLLPUBLIC_EXPORT void* SAL_CALL sm_component_getFactory( const sal_Char* pImplementationName,
                                     void* pServiceManager,
                                     void* /*pRegistryKey*/ )
{
    // Set default return value for this operation - if it failed.
    void* pReturn = NULL ;

    if  (
            ( pImplementationName   !=  NULL ) &&
            ( pServiceManager       !=  NULL )
        )
    {
        // Define variables which are used in following macros.
        Reference< XSingleServiceFactory >   xFactory                                                                                                ;
        Reference< XMultiServiceFactory >    xServiceManager( reinterpret_cast< XMultiServiceFactory* >( pServiceManager ) ) ;

        if( SmXMLImport_getImplementationName().equalsAscii( pImplementationName ))
        {
            xFactory = ::cppu::createSingleFactory( xServiceManager,
            SmXMLImport_getImplementationName(),
            SmXMLImport_createInstance,
            SmXMLImport_getSupportedServiceNames() );
        }
        else if( SmXMLExport_getImplementationName().equalsAscii( pImplementationName ))
        {
            xFactory = ::cppu::createSingleFactory( xServiceManager,
            SmXMLExport_getImplementationName(),
            SmXMLExport_createInstance,
            SmXMLExport_getSupportedServiceNames() );
        }
        else if( SmXMLImportMeta_getImplementationName().equalsAscii( pImplementationName ))
        {
            xFactory = ::cppu::createSingleFactory( xServiceManager,
            SmXMLImportMeta_getImplementationName(),
            SmXMLImportMeta_createInstance,
            SmXMLImportMeta_getSupportedServiceNames() );
        }
        else if( SmXMLExportMetaOOO_getImplementationName().equalsAscii( pImplementationName ))
        {
            xFactory = ::cppu::createSingleFactory( xServiceManager,
            SmXMLExportMetaOOO_getImplementationName(),
            SmXMLExportMetaOOO_createInstance,
            SmXMLExportMetaOOO_getSupportedServiceNames() );
        }
        else if( SmXMLExportMeta_getImplementationName().equalsAscii( pImplementationName ))
        {
            xFactory = ::cppu::createSingleFactory( xServiceManager,
            SmXMLExportMeta_getImplementationName(),
            SmXMLExportMeta_createInstance,
            SmXMLExportMeta_getSupportedServiceNames() );
        }
        else if( SmXMLImportSettings_getImplementationName().equalsAscii( pImplementationName ))
        {
            xFactory = ::cppu::createSingleFactory( xServiceManager,
            SmXMLImportSettings_getImplementationName(),
            SmXMLImportSettings_createInstance,
            SmXMLImportSettings_getSupportedServiceNames() );
        }
        else if( SmXMLExportSettingsOOO_getImplementationName().equalsAscii( pImplementationName ))
        {
            xFactory = ::cppu::createSingleFactory( xServiceManager,
            SmXMLExportSettingsOOO_getImplementationName(),
            SmXMLExportSettingsOOO_createInstance,
            SmXMLExportSettingsOOO_getSupportedServiceNames() );
        }
        else if( SmXMLExportSettings_getImplementationName().equalsAscii( pImplementationName ))
        {
            xFactory = ::cppu::createSingleFactory( xServiceManager,
            SmXMLExportSettings_getImplementationName(),
            SmXMLExportSettings_createInstance,
            SmXMLExportSettings_getSupportedServiceNames() );
        }
        else if( SmXMLExportContent_getImplementationName().equalsAscii( pImplementationName ))
        {
            xFactory = ::cppu::createSingleFactory( xServiceManager,
            SmXMLExportContent_getImplementationName(),
            SmXMLExportContent_createInstance,
            SmXMLExportContent_getSupportedServiceNames() );
        }
        else if( SmDocument_getImplementationName().equalsAscii( pImplementationName ))
        {
            xFactory = ::sfx2::createSfxModelFactory( xServiceManager,
            SmDocument_getImplementationName(),
            SmDocument_createInstance,
            SmDocument_getSupportedServiceNames() );
        }


        // Factory is valid - service was found.
        if ( xFactory.is() )
        {
            xFactory->acquire();
            pReturn = xFactory.get();
        }
    }

    // Return with result of this operation.
    return pReturn ;
}
} // extern "C"



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
