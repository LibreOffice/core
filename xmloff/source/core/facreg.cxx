/*************************************************************************
 *
 *  $RCSfile: facreg.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: sab $ $Date: 2001-02-28 17:47:49 $
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

#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#include <cppuhelper/factory.hxx>
#include <uno/lbnames.h>

using namespace rtl;
using namespace com::sun::star;

// impress import
extern uno::Sequence< OUString > SAL_CALL SdImpressXMLImport_getSupportedServiceNames() throw();
extern OUString SAL_CALL SdImpressXMLImport_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SdImpressXMLImport_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception );

// impress export
extern uno::Sequence< OUString > SAL_CALL SdImpressXMLExport_getSupportedServiceNames() throw();
extern OUString SAL_CALL SdImpressXMLExport_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SdImpressXMLExport_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception );

// draw import
extern uno::Sequence< OUString > SAL_CALL SdDrawXMLImport_getSupportedServiceNames() throw();
extern OUString SAL_CALL SdDrawXMLImport_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SdDrawXMLImport_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception );

// draw export
extern uno::Sequence< OUString > SAL_CALL SdDrawXMLExport_getSupportedServiceNames() throw();
extern OUString SAL_CALL SdDrawXMLExport_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SdDrawXMLExport_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception );

// chart import
extern uno::Sequence< OUString > SAL_CALL SchXMLImport_getSupportedServiceNames() throw();
extern OUString SAL_CALL SchXMLImport_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SchXMLImport_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception );

// chart export
extern uno::Sequence< OUString > SAL_CALL SchXMLExport_getSupportedServiceNames() throw();
extern OUString SAL_CALL SchXMLExport_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SchXMLExport_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception );

// meta export
extern uno::Sequence< OUString > SAL_CALL XMLMetaExportComponent_getSupportedServiceNames() throw();
extern OUString SAL_CALL XMLMetaExportComponent_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL XMLMetaExportComponent_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception );

// meta import
extern uno::Sequence< OUString > SAL_CALL XMLMetaImportComponent_getSupportedServiceNames() throw();
extern OUString SAL_CALL XMLMetaImportComponent_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL XMLMetaImportComponent_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception );

// writer autotext event export
extern uno::Sequence< OUString > SAL_CALL XMLAutoTextEventExport_getSupportedServiceNames() throw();
extern OUString SAL_CALL XMLAutoTextEventExport_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL XMLAutoTextEventExport_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception );

// writer autotext event import
extern uno::Sequence< OUString > SAL_CALL XMLAutoTextEventImport_getSupportedServiceNames() throw();
extern OUString SAL_CALL XMLAutoTextEventImport_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL XMLAutoTextEventImport_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception );


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

sal_Bool SAL_CALL component_writeInfo( void * pServiceManager, void * pRegistryKey )
{
    if( pRegistryKey )
    {
        try
        {
            registry::XRegistryKey *pKey = reinterpret_cast< registry::XRegistryKey * >( pRegistryKey );

            // impress
            writeInfo( pKey, SdImpressXMLImport_getImplementationName(), SdImpressXMLImport_getSupportedServiceNames() );
            writeInfo( pKey, SdImpressXMLExport_getImplementationName(), SdImpressXMLExport_getSupportedServiceNames() );

            // draw
            writeInfo( pKey, SdDrawXMLImport_getImplementationName(), SdDrawXMLImport_getSupportedServiceNames() );
            writeInfo( pKey, SdDrawXMLExport_getImplementationName(), SdDrawXMLExport_getSupportedServiceNames() );

            // chart
            writeInfo( pKey, SchXMLImport_getImplementationName(), SchXMLImport_getSupportedServiceNames() );
            writeInfo( pKey, SchXMLExport_getImplementationName(), SchXMLExport_getSupportedServiceNames() );

            // meta
            writeInfo( pKey, XMLMetaImportComponent_getImplementationName(), XMLMetaImportComponent_getSupportedServiceNames() );
            writeInfo( pKey, XMLMetaExportComponent_getImplementationName(), XMLMetaExportComponent_getSupportedServiceNames() );

            // writer auto text events
            writeInfo( pKey, XMLAutoTextEventExport_getImplementationName(), XMLAutoTextEventExport_getSupportedServiceNames() );
            writeInfo( pKey, XMLAutoTextEventImport_getImplementationName(), XMLAutoTextEventImport_getSupportedServiceNames() );
        }
        catch (registry::InvalidRegistryException &)
        {
            OSL_ENSHURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return True;
}

void * SAL_CALL component_getFactory( const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;
    if( pServiceManager )
    {
        uno::Reference< lang::XMultiServiceFactory > xMSF( reinterpret_cast< lang::XMultiServiceFactory * >( pServiceManager ) );

        uno::Reference< lang::XSingleServiceFactory > xFactory;

        const sal_Int32 nImplNameLen = strlen( pImplName );
        if( SdImpressXMLImport_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SdImpressXMLImport_getImplementationName(),
                SdImpressXMLImport_createInstance,
                SdImpressXMLImport_getSupportedServiceNames() );
        }
        else if( SdDrawXMLImport_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SdDrawXMLImport_getImplementationName(),
                SdDrawXMLImport_createInstance,
                SdDrawXMLImport_getSupportedServiceNames() );
        }
        if( SdImpressXMLExport_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SdImpressXMLExport_getImplementationName(),
                SdImpressXMLExport_createInstance,
                SdImpressXMLExport_getSupportedServiceNames() );
        }
        else if( SdDrawXMLExport_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SdDrawXMLExport_getImplementationName(),
                SdDrawXMLExport_createInstance,
                SdDrawXMLExport_getSupportedServiceNames() );
        }
        else if( SchXMLImport_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SchXMLImport_getImplementationName(),
                SchXMLImport_createInstance,
                SchXMLImport_getSupportedServiceNames() );
        }
        else if( SchXMLExport_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SchXMLExport_getImplementationName(),
                SchXMLExport_createInstance,
                SchXMLExport_getSupportedServiceNames() );
        }
        else if ( XMLMetaExportComponent_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                XMLMetaExportComponent_getImplementationName(),
                XMLMetaExportComponent_createInstance,
                XMLMetaExportComponent_getSupportedServiceNames() );
        }
        else if ( XMLMetaImportComponent_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                XMLMetaImportComponent_getImplementationName(),
                XMLMetaImportComponent_createInstance,
                XMLMetaImportComponent_getSupportedServiceNames() );
        }
        else if( XMLAutoTextEventExport_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                XMLAutoTextEventExport_getImplementationName(),
                XMLAutoTextEventExport_createInstance,
                XMLAutoTextEventExport_getSupportedServiceNames() );
        }
        else if( XMLAutoTextEventImport_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                XMLAutoTextEventImport_getImplementationName(),
                XMLAutoTextEventImport_createInstance,
                XMLAutoTextEventImport_getSupportedServiceNames() );
        }
        if( xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }
    return pRet;
}

}
