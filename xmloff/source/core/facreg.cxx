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
#include "precompiled_xmloff.hxx"

#include "sal/config.h"

#include "xmloff/dllapi.h"

#include <string.h>
#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <osl/diagnose.h>
#include <tools/debug.hxx>

#include <cppuhelper/factory.hxx>
#include <uno/lbnames.h>
#include "xmloff/xmlreg.hxx"

using namespace rtl;
using namespace com::sun::star;

#define SERVICE( className )                                        \
extern OUString SAL_CALL className##_getImplementationName() throw();   \
extern uno::Sequence< OUString > SAL_CALL className##_getSupportedServiceNames() throw();\
extern uno::Reference< uno::XInterface > SAL_CALL className##_createInstance(           \
        const uno::Reference< lang::XMultiServiceFactory > & rSMgr )                \
    throw( uno::Exception )

// impress oasis import
SERVICE( XMLImpressImportOasis );
SERVICE( XMLImpressStylesImportOasis );
SERVICE( XMLImpressContentImportOasis );
SERVICE( XMLImpressMetaImportOasis );
SERVICE( XMLImpressSettingsImportOasis );

// impress oasis export
SERVICE( XMLImpressExportOasis );
SERVICE( XMLImpressStylesExportOasis );
SERVICE( XMLImpressContentExportOasis );
SERVICE( XMLImpressMetaExportOasis );
SERVICE( XMLImpressSettingsExportOasis );

// impress OOo import

// impress OOo export
SERVICE( XMLImpressExportOOO );
SERVICE( XMLImpressStylesExportOOO );
SERVICE( XMLImpressContentExportOOO );
SERVICE( XMLImpressMetaExportOOO );
SERVICE( XMLImpressSettingsExportOOO );

// draw oasis import
SERVICE( XMLDrawImportOasis );
SERVICE( XMLDrawStylesImportOasis );
SERVICE( XMLDrawContentImportOasis );
SERVICE( XMLDrawMetaImportOasis );
SERVICE( XMLDrawSettingsImportOasis );

// draw oasis export
SERVICE( XMLDrawExportOasis );
SERVICE( XMLDrawStylesExportOasis );
SERVICE( XMLDrawContentExportOasis );
SERVICE( XMLDrawMetaExportOasis );
SERVICE( XMLDrawSettingsExportOasis );

// draw OOo import

// draw OOo export
SERVICE( XMLDrawExportOOO );
SERVICE( XMLDrawStylesExportOOO );
SERVICE( XMLDrawContentExportOOO );
SERVICE( XMLDrawMetaExportOOO );
SERVICE( XMLDrawSettingsExportOOO );

// impress animation import
SERVICE( AnimationsImport );

// drawing layer export
SERVICE( XMLDrawingLayerExport );

// impress xml clipboard export
SERVICE( XMLImpressClipboardExport );

// chart oasis import
SERVICE( SchXMLImport );
SERVICE( SchXMLImport_Meta );
SERVICE( SchXMLImport_Styles );
SERVICE( SchXMLImport_Content );

// chart oasis export
SERVICE( SchXMLExport_Oasis );
SERVICE( SchXMLExport_Oasis_Meta );
SERVICE( SchXMLExport_Oasis_Styles );
SERVICE( SchXMLExport_Oasis_Content );

// chart OOo import

// chart OOo export
SERVICE( SchXMLExport );
SERVICE( SchXMLExport_Styles );
SERVICE( SchXMLExport_Content );

// version list import/export
SERVICE( XMLVersionListPersistence );

// meta export
SERVICE( XMLMetaExportComponent );

// meta import
SERVICE( XMLMetaImportComponent );

// meta export OOo
SERVICE( XMLMetaExportOOO );

// writer autotext event export
SERVICE( XMLAutoTextEventExport );

// writer autotext event import
SERVICE( XMLAutoTextEventImport );

// writer autotext event export OOo
SERVICE( XMLAutoTextEventExportOOO );

// writer autotext event import OOo
SERVICE( XMLAutoTextEventImportOOO );

// Oasis2OOo transformer
SERVICE( Oasis2OOoTransformer );

// OOo2Oasis transformer
SERVICE( OOo2OasisTransformer );

//
#ifdef __cplusplus
extern "C"
{
#endif

XMLOFF_DLLPUBLIC void SAL_CALL component_getImplementationEnvironment( const sal_Char ** ppEnvTypeName, uno_Environment ** )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

#define SINGLEFACTORY(classname)\
        if( classname##_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )\
        {\
            xFactory = ::cppu::createSingleFactory( xMSF,\
                classname##_getImplementationName(),\
                classname##_createInstance,\
                classname##_getSupportedServiceNames() );\
        }

XMLOFF_DLLPUBLIC void * SAL_CALL component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
{
    void * pRet = 0;
    if( pServiceManager )
    {
        uno::Reference< lang::XMultiServiceFactory > xMSF( reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ) );

        uno::Reference< lang::XSingleServiceFactory > xFactory;

        const sal_Int32 nImplNameLen = strlen( pImplName );

        // impress oasis import
        SINGLEFACTORY( XMLImpressImportOasis )
        else SINGLEFACTORY( XMLImpressStylesImportOasis )
        else SINGLEFACTORY( XMLImpressContentImportOasis )
        else SINGLEFACTORY( XMLImpressMetaImportOasis )
        else SINGLEFACTORY( XMLImpressSettingsImportOasis )

        // impress oasis export
        else SINGLEFACTORY( XMLImpressExportOasis )
        else SINGLEFACTORY( XMLImpressStylesExportOasis )
        else SINGLEFACTORY( XMLImpressContentExportOasis )
        else SINGLEFACTORY( XMLImpressMetaExportOasis )
        else SINGLEFACTORY( XMLImpressSettingsExportOasis )

        else SINGLEFACTORY( AnimationsImport )

        // impress OOo export
        else SINGLEFACTORY( XMLImpressExportOOO )
        else SINGLEFACTORY( XMLImpressStylesExportOOO )
        else SINGLEFACTORY( XMLImpressContentExportOOO )
        else SINGLEFACTORY( XMLImpressMetaExportOOO )
        else SINGLEFACTORY( XMLImpressSettingsExportOOO )

        // draw oasis import
        else SINGLEFACTORY( XMLDrawImportOasis )
        else SINGLEFACTORY( XMLDrawStylesImportOasis )
        else SINGLEFACTORY( XMLDrawContentImportOasis )
        else SINGLEFACTORY( XMLDrawMetaImportOasis )
        else SINGLEFACTORY( XMLDrawSettingsImportOasis )

        // draw oasis export
        else SINGLEFACTORY( XMLDrawExportOasis )
        else SINGLEFACTORY( XMLDrawStylesExportOasis )
        else SINGLEFACTORY( XMLDrawContentExportOasis )
        else SINGLEFACTORY( XMLDrawMetaExportOasis )
        else SINGLEFACTORY( XMLDrawSettingsExportOasis )

        // draw OOo export
        else SINGLEFACTORY( XMLDrawExportOOO )
        else SINGLEFACTORY( XMLDrawStylesExportOOO )
        else SINGLEFACTORY( XMLDrawContentExportOOO )
        else SINGLEFACTORY( XMLDrawMetaExportOOO )
        else SINGLEFACTORY( XMLDrawSettingsExportOOO )

        // drawing layer export
        else SINGLEFACTORY( XMLDrawingLayerExport )

        // impress xml clipboard export
        else SINGLEFACTORY( XMLImpressClipboardExport )

        // chart oasis import
        else SINGLEFACTORY( SchXMLImport )
        else SINGLEFACTORY( SchXMLImport_Meta )
        else SINGLEFACTORY( SchXMLImport_Styles )
        else SINGLEFACTORY( SchXMLImport_Content )

        // chart oasis export
        else SINGLEFACTORY( SchXMLExport_Oasis )
        else SINGLEFACTORY( SchXMLExport_Oasis_Meta )
        else SINGLEFACTORY( SchXMLExport_Oasis_Styles )
        else SINGLEFACTORY( SchXMLExport_Oasis_Content )

        // chart OOo export
        else SINGLEFACTORY( SchXMLExport )
        else SINGLEFACTORY( SchXMLExport_Styles )
        else SINGLEFACTORY( SchXMLExport_Content )

        // meta import/export
        else SINGLEFACTORY( XMLMetaExportComponent )
        else SINGLEFACTORY( XMLMetaImportComponent )

        else SINGLEFACTORY( XMLVersionListPersistence )

        // meta import/export OOo
        else SINGLEFACTORY( XMLMetaExportOOO )

        // auto text import/export
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

