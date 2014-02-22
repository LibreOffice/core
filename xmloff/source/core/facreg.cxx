/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include <sal/config.h>

#include <xmloff/dllapi.h>

#include <string.h>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <osl/diagnose.h>

#include <cppuhelper/factory.hxx>
#include <xmloff/xmlreg.hxx>

using namespace com::sun::star;


#define SERVICE( className )                                        \
extern OUString SAL_CALL className##_getImplementationName() throw();   \
extern uno::Sequence< OUString > SAL_CALL className##_getSupportedServiceNames() throw();\
extern uno::Reference< uno::XInterface > SAL_CALL className##_createInstance(           \
        const uno::Reference< lang::XMultiServiceFactory > & rSMgr )                \
    throw( uno::Exception )


SERVICE( XMLImpressImportOasis );
SERVICE( XMLImpressStylesImportOasis );
SERVICE( XMLImpressContentImportOasis );
SERVICE( XMLImpressMetaImportOasis );
SERVICE( XMLImpressSettingsImportOasis );


SERVICE( XMLImpressExportOasis );
SERVICE( XMLImpressStylesExportOasis );
SERVICE( XMLImpressContentExportOasis );
SERVICE( XMLImpressMetaExportOasis );
SERVICE( XMLImpressSettingsExportOasis );




SERVICE( XMLImpressExportOOO );
SERVICE( XMLImpressStylesExportOOO );
SERVICE( XMLImpressContentExportOOO );
SERVICE( XMLImpressMetaExportOOO );
SERVICE( XMLImpressSettingsExportOOO );


SERVICE( XMLDrawImportOasis );
SERVICE( XMLDrawStylesImportOasis );
SERVICE( XMLDrawContentImportOasis );
SERVICE( XMLDrawMetaImportOasis );
SERVICE( XMLDrawSettingsImportOasis );


SERVICE( XMLDrawExportOasis );
SERVICE( XMLDrawStylesExportOasis );
SERVICE( XMLDrawContentExportOasis );
SERVICE( XMLDrawMetaExportOasis );
SERVICE( XMLDrawSettingsExportOasis );




SERVICE( XMLDrawExportOOO );
SERVICE( XMLDrawStylesExportOOO );
SERVICE( XMLDrawContentExportOOO );
SERVICE( XMLDrawMetaExportOOO );
SERVICE( XMLDrawSettingsExportOOO );


SERVICE( AnimationsImport );


SERVICE( XMLDrawingLayerExport );


SERVICE( XMLImpressClipboardExport );


SERVICE( SchXMLImport );
SERVICE( SchXMLImport_Meta );
SERVICE( SchXMLImport_Styles );
SERVICE( SchXMLImport_Content );


SERVICE( SchXMLExport_Oasis );
SERVICE( SchXMLExport_Oasis_Meta );
SERVICE( SchXMLExport_Oasis_Styles );
SERVICE( SchXMLExport_Oasis_Content );




SERVICE( SchXMLExport );
SERVICE( SchXMLExport_Styles );
SERVICE( SchXMLExport_Content );


SERVICE( XMLMetaExportComponent );


SERVICE( XMLMetaImportComponent );


SERVICE( XMLMetaExportOOO );


SERVICE( XMLAutoTextEventExport );


SERVICE( XMLAutoTextEventImport );


SERVICE( XMLAutoTextEventExportOOO );


SERVICE( XMLAutoTextEventImportOOO );


SERVICE( Oasis2OOoTransformer );


SERVICE( OOo2OasisTransformer );

//
#ifdef __cplusplus
extern "C"
{
#endif

#define SINGLEFACTORY(classname)\
        if( classname##_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )\
        {\
            xFactory = ::cppu::createSingleFactory( xMSF,\
                classname##_getImplementationName(),\
                classname##_createInstance,\
                classname##_getSupportedServiceNames() );\
        }

XMLOFF_DLLPUBLIC void * SAL_CALL xo_component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
{
    void * pRet = 0;
    if( pServiceManager )
    {
        uno::Reference< lang::XMultiServiceFactory > xMSF( reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ) );

        uno::Reference< lang::XSingleServiceFactory > xFactory;

        const sal_Int32 nImplNameLen = strlen( pImplName );

        
        SINGLEFACTORY( XMLImpressImportOasis )
        else SINGLEFACTORY( XMLImpressStylesImportOasis )
        else SINGLEFACTORY( XMLImpressContentImportOasis )
        else SINGLEFACTORY( XMLImpressMetaImportOasis )
        else SINGLEFACTORY( XMLImpressSettingsImportOasis )

        
        else SINGLEFACTORY( XMLImpressExportOasis )
        else SINGLEFACTORY( XMLImpressStylesExportOasis )
        else SINGLEFACTORY( XMLImpressContentExportOasis )
        else SINGLEFACTORY( XMLImpressMetaExportOasis )
        else SINGLEFACTORY( XMLImpressSettingsExportOasis )

        else SINGLEFACTORY( AnimationsImport )

        
        else SINGLEFACTORY( XMLImpressExportOOO )
        else SINGLEFACTORY( XMLImpressStylesExportOOO )
        else SINGLEFACTORY( XMLImpressContentExportOOO )
        else SINGLEFACTORY( XMLImpressMetaExportOOO )
        else SINGLEFACTORY( XMLImpressSettingsExportOOO )

        
        else SINGLEFACTORY( XMLDrawImportOasis )
        else SINGLEFACTORY( XMLDrawStylesImportOasis )
        else SINGLEFACTORY( XMLDrawContentImportOasis )
        else SINGLEFACTORY( XMLDrawMetaImportOasis )
        else SINGLEFACTORY( XMLDrawSettingsImportOasis )

        
        else SINGLEFACTORY( XMLDrawExportOasis )
        else SINGLEFACTORY( XMLDrawStylesExportOasis )
        else SINGLEFACTORY( XMLDrawContentExportOasis )
        else SINGLEFACTORY( XMLDrawMetaExportOasis )
        else SINGLEFACTORY( XMLDrawSettingsExportOasis )

        
        else SINGLEFACTORY( XMLDrawExportOOO )
        else SINGLEFACTORY( XMLDrawStylesExportOOO )
        else SINGLEFACTORY( XMLDrawContentExportOOO )
        else SINGLEFACTORY( XMLDrawMetaExportOOO )
        else SINGLEFACTORY( XMLDrawSettingsExportOOO )

        
        else SINGLEFACTORY( XMLDrawingLayerExport )

        
        else SINGLEFACTORY( XMLImpressClipboardExport )

        
        else SINGLEFACTORY( SchXMLImport )
        else SINGLEFACTORY( SchXMLImport_Meta )
        else SINGLEFACTORY( SchXMLImport_Styles )
        else SINGLEFACTORY( SchXMLImport_Content )

        
        else SINGLEFACTORY( SchXMLExport_Oasis )
        else SINGLEFACTORY( SchXMLExport_Oasis_Meta )
        else SINGLEFACTORY( SchXMLExport_Oasis_Styles )
        else SINGLEFACTORY( SchXMLExport_Oasis_Content )

        
        else SINGLEFACTORY( SchXMLExport )
        else SINGLEFACTORY( SchXMLExport_Styles )
        else SINGLEFACTORY( SchXMLExport_Content )

        
        else SINGLEFACTORY( XMLMetaExportComponent )
        else SINGLEFACTORY( XMLMetaImportComponent )

        
        else SINGLEFACTORY( XMLMetaExportOOO )

        
        else SINGLEFACTORY( XMLAutoTextEventExport )
        else SINGLEFACTORY( XMLAutoTextEventImport )
        else SINGLEFACTORY( XMLAutoTextEventExportOOO )

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
