/*************************************************************************
 *
 *  $RCSfile: facreg.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 08:06:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
SERVICE( XMLImpressImportOOO );
SERVICE( XMLImpressStylesImportOOO );
SERVICE( XMLImpressContentImportOOO );
SERVICE( XMLImpressMetaImportOOO );
SERVICE( XMLImpressSettingsImportOOO );

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
SERVICE( XMLDrawImportOOO );
SERVICE( XMLDrawStylesImportOOO );
SERVICE( XMLDrawContentImportOOO );
SERVICE( XMLDrawMetaImportOOO );
SERVICE( XMLDrawSettingsImportOOO );

// draw OOo export
SERVICE( XMLDrawExportOOO );
SERVICE( XMLDrawStylesExportOOO );
SERVICE( XMLDrawContentExportOOO );
SERVICE( XMLDrawMetaExportOOO );
SERVICE( XMLDrawSettingsExportOOO );

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
SERVICE( XMLChartImportOOO );
SERVICE( XMLChartStylesImportOOO );
SERVICE( XMLChartContentImportOOO );

// chart OOo export
SERVICE( SchXMLExport );
SERVICE( SchXMLExport_Styles );
SERVICE( SchXMLExport_Content );

// Writer OOo import
SERVICE( XMLWriterImportOOO );
SERVICE( XMLWriterStylesImportOOO );
SERVICE( XMLWriterContentImportOOO );
SERVICE( XMLWriterMetaImportOOO );
SERVICE( XMLWriterSettingsImportOOO );

// Calc OOo import
SERVICE( XMLCalcImportOOO );
SERVICE( XMLCalcStylesImportOOO );
SERVICE( XMLCalcContentImportOOO );
SERVICE( XMLCalcMetaImportOOO );
SERVICE( XMLCalcSettingsImportOOO );

// Math OOo import
SERVICE( XMLMathMetaImportOOO );
SERVICE( XMLMathSettingsImportOOO );

// meta export
SERVICE( XMLMetaExportComponent );

// meta import
SERVICE( XMLMetaImportComponent );

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

void SAL_CALL component_getImplementationEnvironment( const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
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

sal_Bool SAL_CALL component_writeInfo( void * pServiceManager, void * pRegistryKey )
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

            // impress OOo importers
            WRITEINFO( XMLImpressImportOOO );
            WRITEINFO( XMLImpressStylesImportOOO );
            WRITEINFO( XMLImpressContentImportOOO );
            WRITEINFO( XMLImpressMetaImportOOO );
            WRITEINFO( XMLImpressSettingsImportOOO );

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

            // draw OOo importers
            WRITEINFO( XMLDrawImportOOO );
            WRITEINFO( XMLDrawStylesImportOOO );
            WRITEINFO( XMLDrawContentImportOOO );
            WRITEINFO( XMLDrawMetaImportOOO );
            WRITEINFO( XMLDrawSettingsImportOOO );

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

            // chart OOo import
            WRITEINFO( XMLChartImportOOO );
            WRITEINFO( XMLChartStylesImportOOO );
            WRITEINFO( XMLChartContentImportOOO );

            // chart OOo export
            WRITEINFO( SchXMLExport );
            WRITEINFO( SchXMLExport_Styles );
            WRITEINFO( SchXMLExport_Content );

            // writer OOo importers
            WRITEINFO( XMLWriterImportOOO );
            WRITEINFO( XMLWriterStylesImportOOO );
            WRITEINFO( XMLWriterContentImportOOO );
            WRITEINFO( XMLWriterMetaImportOOO );
            WRITEINFO( XMLWriterSettingsImportOOO );

            // calc OOo importers
            WRITEINFO( XMLCalcImportOOO );
            WRITEINFO( XMLCalcStylesImportOOO );
            WRITEINFO( XMLCalcContentImportOOO );
            WRITEINFO( XMLCalcMetaImportOOO );
            WRITEINFO( XMLCalcSettingsImportOOO );

            // Math OOo import
            WRITEINFO( XMLMathMetaImportOOO );
            WRITEINFO( XMLMathSettingsImportOOO );

            // meta
            WRITEINFO( XMLMetaImportComponent );
            WRITEINFO( XMLMetaExportComponent );
            WRITEINFO( XMLMetaExportComponent );

            // writer auto text events
            WRITEINFO( XMLAutoTextEventExport );
            WRITEINFO( XMLAutoTextEventImport );
            WRITEINFO( XMLAutoTextEventExportOOO );
            WRITEINFO( XMLAutoTextEventImportOOO );

            // Oasis2OOo transformer
            WRITEINFO( Oasis2OOoTransformer );

            // OOo2Oasis transformer
            WRITEINFO( OOo2OasisTransformer );
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

void * SAL_CALL component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
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

        // impress OOo importers
        else SINGLEFACTORY( XMLImpressImportOOO )
        else SINGLEFACTORY( XMLImpressStylesImportOOO )
        else SINGLEFACTORY( XMLImpressContentImportOOO )
        else SINGLEFACTORY( XMLImpressMetaImportOOO )
        else SINGLEFACTORY( XMLImpressSettingsImportOOO )

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

        // draw OOo importers
        else SINGLEFACTORY( XMLDrawImportOOO )
        else SINGLEFACTORY( XMLDrawStylesImportOOO )
        else SINGLEFACTORY( XMLDrawContentImportOOO )
        else SINGLEFACTORY( XMLDrawMetaImportOOO )
        else SINGLEFACTORY( XMLDrawSettingsImportOOO )

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

        // chart OOo import
        else SINGLEFACTORY( XMLChartImportOOO )
        else SINGLEFACTORY( XMLChartStylesImportOOO )
        else SINGLEFACTORY( XMLChartContentImportOOO )

        // chart OOo export
        else SINGLEFACTORY( SchXMLExport )
        else SINGLEFACTORY( SchXMLExport_Styles )
        else SINGLEFACTORY( SchXMLExport_Content )

        // writer OOo import
        else SINGLEFACTORY( XMLWriterImportOOO )
        else SINGLEFACTORY( XMLWriterStylesImportOOO )
        else SINGLEFACTORY( XMLWriterContentImportOOO )
        else SINGLEFACTORY( XMLWriterMetaImportOOO )
        else SINGLEFACTORY( XMLWriterSettingsImportOOO )

        // calc OOo import
        else SINGLEFACTORY( XMLCalcImportOOO )
        else SINGLEFACTORY( XMLCalcStylesImportOOO )
        else SINGLEFACTORY( XMLCalcContentImportOOO )
        else SINGLEFACTORY( XMLCalcMetaImportOOO )
        else SINGLEFACTORY( XMLCalcSettingsImportOOO )

        // math OOo import
        else SINGLEFACTORY( XMLMathMetaImportOOO )
        else SINGLEFACTORY( XMLMathSettingsImportOOO )

        // meta import/export
        else SINGLEFACTORY( XMLMetaExportComponent )
        else SINGLEFACTORY( XMLMetaImportComponent )

        // auto text import/export
        else SINGLEFACTORY( XMLAutoTextEventExport )
        else SINGLEFACTORY( XMLAutoTextEventImport )
        else SINGLEFACTORY( XMLAutoTextEventExportOOO )
        else SINGLEFACTORY( XMLAutoTextEventImportOOO )

        // transformers
        else SINGLEFACTORY( Oasis2OOoTransformer )
        else SINGLEFACTORY( OOo2OasisTransformer )

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

