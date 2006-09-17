/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: facreg.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 10:20:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"

#include "sal/config.h"

#include "xmloff/dllapi.h"

#include <string.h>

#ifndef _COM_SUN_STAR_CONTAINER_XSET_HPP_
#include <com/sun/star/container/XSet.hpp>
#endif

#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#include <cppuhelper/factory.hxx>
#include <uno/lbnames.h>
#include "xmlreg.hxx"

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
SERVICE( SchXMLImport_Styles );
SERVICE( SchXMLImport_Content );

// chart oasis export
SERVICE( SchXMLExport_Oasis );
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

void SAL_CALL writeInfo( registry::XRegistryKey * pRegistryKey, const OUString& rImplementationName, const uno::Sequence< OUString >& rServices )
{
    uno::Reference< registry::XRegistryKey > xNewKey(
        pRegistryKey->createKey(
            OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) + rImplementationName + OUString(RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES") ) ) );

    for( sal_Int32 i = 0; i < rServices.getLength(); i++ )
        xNewKey->createKey( rServices.getConstArray()[i]);
}

#define WRITEINFO(className)\
    writeInfo( pKey, className##_getImplementationName(), className##_getSupportedServiceNames() )

XMLOFF_DLLPUBLIC sal_Bool SAL_CALL component_writeInfo( void * /*pServiceManager*/, void * pRegistryKey )
{
    if( pRegistryKey )
    {
        try
        {
            registry::XRegistryKey *pKey = reinterpret_cast< registry::XRegistryKey * >( pRegistryKey );

            // impress oasis import
            WRITEINFO( XMLImpressImportOasis );
            WRITEINFO( XMLImpressStylesImportOasis );
            WRITEINFO( XMLImpressContentImportOasis );
            WRITEINFO( XMLImpressMetaImportOasis );
            WRITEINFO( XMLImpressSettingsImportOasis );

            // impress oasis export
            WRITEINFO( XMLImpressExportOasis );
            WRITEINFO( XMLImpressStylesExportOasis );
            WRITEINFO( XMLImpressContentExportOasis );
            WRITEINFO( XMLImpressMetaExportOasis );
            WRITEINFO( XMLImpressSettingsExportOasis );

            // animation import
            WRITEINFO( AnimationsImport );

            // impress OOo export
            WRITEINFO( XMLImpressExportOOO );
            WRITEINFO( XMLImpressStylesExportOOO );
            WRITEINFO( XMLImpressContentExportOOO );
            WRITEINFO( XMLImpressMetaExportOOO );
            WRITEINFO( XMLImpressSettingsExportOOO );

            // draw oasis import
            WRITEINFO( XMLDrawImportOasis );
            WRITEINFO( XMLDrawStylesImportOasis );
            WRITEINFO( XMLDrawContentImportOasis );
            WRITEINFO( XMLDrawMetaImportOasis );
            WRITEINFO( XMLDrawSettingsImportOasis );

            // draw oasis export
            WRITEINFO( XMLDrawExportOasis );
            WRITEINFO( XMLDrawStylesExportOasis );
            WRITEINFO( XMLDrawContentExportOasis );
            WRITEINFO( XMLDrawMetaExportOasis );
            WRITEINFO( XMLDrawSettingsExportOasis );

            // draw OOo export
            WRITEINFO( XMLDrawExportOOO );
            WRITEINFO( XMLDrawStylesExportOOO );
            WRITEINFO( XMLDrawContentExportOOO );
            WRITEINFO( XMLDrawMetaExportOOO );
            WRITEINFO( XMLDrawSettingsExportOOO );

            // drawing layer export
            WRITEINFO( XMLDrawingLayerExport );

            // impress xml clipboard export
            WRITEINFO( XMLImpressClipboardExport );

            // chart oasis import
            WRITEINFO( SchXMLImport );
            WRITEINFO( SchXMLImport_Styles );
            WRITEINFO( SchXMLImport_Content );

            // chart oasis export
            WRITEINFO( SchXMLExport_Oasis );
            WRITEINFO( SchXMLExport_Oasis_Styles );
            WRITEINFO( SchXMLExport_Oasis_Content );

            // chart OOo export
            WRITEINFO( SchXMLExport );
            WRITEINFO( SchXMLExport_Styles );
            WRITEINFO( SchXMLExport_Content );

            // meta
            WRITEINFO( XMLMetaImportComponent );
            WRITEINFO( XMLMetaExportComponent );

            WRITEINFO( XMLVersionListPersistence );

            // meta OOo
            WRITEINFO( XMLMetaExportOOO );

            // writer auto text events
            WRITEINFO( XMLAutoTextEventExport );
            WRITEINFO( XMLAutoTextEventImport );
            WRITEINFO( XMLAutoTextEventExportOOO );
        }
        catch (registry::InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_True;
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
        else SINGLEFACTORY( SchXMLImport_Styles )
        else SINGLEFACTORY( SchXMLImport_Content )

        // chart oasis export
        else SINGLEFACTORY( SchXMLExport_Oasis )
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

