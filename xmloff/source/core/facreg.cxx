/*************************************************************************
 *
 *  $RCSfile: facreg.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: cl $ $Date: 2001-03-04 16:08:56 $
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

// impress import.styles
extern uno::Sequence< OUString > SAL_CALL SdImpressXMLImport_Style_getSupportedServiceNames() throw();
extern OUString SAL_CALL SdImpressXMLImport_Style_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SdImpressXMLImport_Style_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception );

// impress import.content
extern uno::Sequence< OUString > SAL_CALL SdImpressXMLImport_Content_getSupportedServiceNames() throw();
extern OUString SAL_CALL SdImpressXMLImport_Content_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SdImpressXMLImport_Content_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception );

// impress import.meta
extern uno::Sequence< OUString > SAL_CALL SdImpressXMLImport_Meta_getSupportedServiceNames() throw();
extern OUString SAL_CALL SdImpressXMLImport_Meta_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SdImpressXMLImport_Meta_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception );

// impress export
extern uno::Sequence< OUString > SAL_CALL SdImpressXMLExport_getSupportedServiceNames() throw();
extern OUString SAL_CALL SdImpressXMLExport_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SdImpressXMLExport_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception );

// impress export.styles
extern uno::Sequence< OUString > SAL_CALL SdImpressXMLExport_Style_getSupportedServiceNames() throw();
extern OUString SAL_CALL SdImpressXMLExport_Style_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SdImpressXMLExport_Style_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception );

// impress export.content
extern uno::Sequence< OUString > SAL_CALL SdImpressXMLExport_Content_getSupportedServiceNames() throw();
extern OUString SAL_CALL SdImpressXMLExport_Content_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SdImpressXMLExport_Content_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception );

// impress export.content
extern uno::Sequence< OUString > SAL_CALL SdImpressXMLExport_Meta_getSupportedServiceNames() throw();
extern OUString SAL_CALL SdImpressXMLExport_Meta_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SdImpressXMLExport_Meta_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception );

// draw import
extern uno::Sequence< OUString > SAL_CALL SdDrawXMLImport_getSupportedServiceNames() throw();
extern OUString SAL_CALL SdDrawXMLImport_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SdDrawXMLImport_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception );

// draw import.styles
extern uno::Sequence< OUString > SAL_CALL SdDrawXMLImport_Style_getSupportedServiceNames() throw();
extern OUString SAL_CALL SdDrawXMLImport_Style_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SdDrawXMLImport_Style_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception );

// draw import.content
extern uno::Sequence< OUString > SAL_CALL SdDrawXMLImport_Content_getSupportedServiceNames() throw();
extern OUString SAL_CALL SdDrawXMLImport_Content_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SdDrawXMLImport_Content_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception );

// draw import.content
extern uno::Sequence< OUString > SAL_CALL SdDrawXMLImport_Meta_getSupportedServiceNames() throw();
extern OUString SAL_CALL SdDrawXMLImport_Meta_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SdDrawXMLImport_Meta_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception );

// draw export
extern uno::Sequence< OUString > SAL_CALL SdDrawXMLExport_getSupportedServiceNames() throw();
extern OUString SAL_CALL SdDrawXMLExport_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SdDrawXMLExport_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception );

// draw export.style
extern uno::Sequence< OUString > SAL_CALL SdDrawXMLExport_Style_getSupportedServiceNames() throw();
extern OUString SAL_CALL SdDrawXMLExport_Style_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SdDrawXMLExport_Style_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception );

// draw export.content
extern uno::Sequence< OUString > SAL_CALL SdDrawXMLExport_Content_getSupportedServiceNames() throw();
extern OUString SAL_CALL SdDrawXMLExport_Content_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SdDrawXMLExport_Content_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception );

// draw export.meta
extern uno::Sequence< OUString > SAL_CALL SdDrawXMLExport_Meta_getSupportedServiceNames() throw();
extern OUString SAL_CALL SdDrawXMLExport_Meta_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SdDrawXMLExport_Meta_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception );

// chart import
// ------------
extern uno::Sequence< OUString > SAL_CALL SchXMLImport_getSupportedServiceNames() throw();
extern OUString SAL_CALL SchXMLImport_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SchXMLImport_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception );

// styles
extern uno::Sequence< OUString > SAL_CALL SchXMLImport_Styles_getSupportedServiceNames() throw();
extern OUString SAL_CALL SchXMLImport_Styles_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SchXMLImport_Styles_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception );

// content
extern uno::Sequence< OUString > SAL_CALL SchXMLImport_Content_getSupportedServiceNames() throw();
extern OUString SAL_CALL SchXMLImport_Content_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SchXMLImport_Content_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception );

// meta
extern uno::Sequence< OUString > SAL_CALL SchXMLImport_Meta_getSupportedServiceNames() throw();
extern OUString SAL_CALL SchXMLImport_Meta_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SchXMLImport_Meta_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception );

// chart export
// ------------
extern uno::Sequence< OUString > SAL_CALL SchXMLExport_getSupportedServiceNames() throw();
extern OUString SAL_CALL SchXMLExport_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SchXMLExport_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception );

// styles
extern uno::Sequence< OUString > SAL_CALL SchXMLExport_Styles_getSupportedServiceNames() throw();
extern OUString SAL_CALL SchXMLExport_Styles_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SchXMLExport_Styles_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception );

// content
extern uno::Sequence< OUString > SAL_CALL SchXMLExport_Content_getSupportedServiceNames() throw();
extern OUString SAL_CALL SchXMLExport_Content_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SchXMLExport_Content_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception );

// meta
extern uno::Sequence< OUString > SAL_CALL SchXMLExport_Meta_getSupportedServiceNames() throw();
extern OUString SAL_CALL SchXMLExport_Meta_getImplementationName() throw();
extern uno::Reference< uno::XInterface > SAL_CALL SchXMLExport_Meta_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception );


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

            // impress.styles
            writeInfo( pKey, SdImpressXMLImport_Style_getImplementationName(), SdImpressXMLImport_Style_getSupportedServiceNames() );
            writeInfo( pKey, SdImpressXMLExport_Style_getImplementationName(), SdImpressXMLExport_Style_getSupportedServiceNames() );

            // impress.content
            writeInfo( pKey, SdImpressXMLImport_Content_getImplementationName(), SdImpressXMLImport_Content_getSupportedServiceNames() );
            writeInfo( pKey, SdImpressXMLExport_Content_getImplementationName(), SdImpressXMLExport_Content_getSupportedServiceNames() );

            // impress.meta
            writeInfo( pKey, SdImpressXMLImport_Meta_getImplementationName(), SdImpressXMLImport_Meta_getSupportedServiceNames() );
            writeInfo( pKey, SdImpressXMLExport_Meta_getImplementationName(), SdImpressXMLExport_Meta_getSupportedServiceNames() );

            // draw
            writeInfo( pKey, SdDrawXMLImport_getImplementationName(), SdDrawXMLImport_getSupportedServiceNames() );
            writeInfo( pKey, SdDrawXMLExport_getImplementationName(), SdDrawXMLExport_getSupportedServiceNames() );

            // draw.styles
            writeInfo( pKey, SdDrawXMLImport_Style_getImplementationName(), SdDrawXMLImport_Style_getSupportedServiceNames() );
            writeInfo( pKey, SdDrawXMLExport_Style_getImplementationName(), SdDrawXMLExport_Style_getSupportedServiceNames() );

            // draw.content
            writeInfo( pKey, SdDrawXMLImport_Content_getImplementationName(), SdDrawXMLImport_Content_getSupportedServiceNames() );
            writeInfo( pKey, SdDrawXMLExport_Content_getImplementationName(), SdDrawXMLExport_Content_getSupportedServiceNames() );

            // draw.meta
            writeInfo( pKey, SdDrawXMLImport_Meta_getImplementationName(), SdDrawXMLImport_Meta_getSupportedServiceNames() );
            writeInfo( pKey, SdDrawXMLExport_Meta_getImplementationName(), SdDrawXMLExport_Meta_getSupportedServiceNames() );

            // chart
            writeInfo( pKey, SchXMLImport_getImplementationName(), SchXMLImport_getSupportedServiceNames() );
            writeInfo( pKey, SchXMLExport_getImplementationName(), SchXMLExport_getSupportedServiceNames() );

            // chart.styles
            writeInfo( pKey, SchXMLImport_Styles_getImplementationName(), SchXMLImport_Styles_getSupportedServiceNames() );
            writeInfo( pKey, SchXMLExport_Styles_getImplementationName(), SchXMLExport_Styles_getSupportedServiceNames() );

            // chart.content
            writeInfo( pKey, SchXMLImport_Content_getImplementationName(), SchXMLImport_Content_getSupportedServiceNames() );
            writeInfo( pKey, SchXMLExport_Content_getImplementationName(), SchXMLExport_Content_getSupportedServiceNames() );

            // chart.meta
            writeInfo( pKey, SchXMLImport_Meta_getImplementationName(), SchXMLImport_Meta_getSupportedServiceNames() );
            writeInfo( pKey, SchXMLExport_Meta_getImplementationName(), SchXMLExport_Meta_getSupportedServiceNames() );

            // meta
            writeInfo( pKey, XMLMetaImportComponent_getImplementationName(), XMLMetaImportComponent_getSupportedServiceNames() );
            writeInfo( pKey, XMLMetaExportComponent_getImplementationName(), XMLMetaExportComponent_getSupportedServiceNames() );
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
        else if( SdImpressXMLImport_Style_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SdImpressXMLImport_Style_getImplementationName(),
                SdImpressXMLImport_Style_createInstance,
                SdImpressXMLImport_Style_getSupportedServiceNames() );
        }
        else if( SdImpressXMLImport_Content_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SdImpressXMLImport_Content_getImplementationName(),
                SdImpressXMLImport_Content_createInstance,
                SdImpressXMLImport_Content_getSupportedServiceNames() );
        }
        else if( SdImpressXMLImport_Meta_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SdImpressXMLImport_Meta_getImplementationName(),
                SdImpressXMLImport_Meta_createInstance,
                SdImpressXMLImport_Meta_getSupportedServiceNames() );
        }
        else if( SdDrawXMLImport_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SdDrawXMLImport_getImplementationName(),
                SdDrawXMLImport_createInstance,
                SdDrawXMLImport_getSupportedServiceNames() );
        }
        else if( SdDrawXMLImport_Style_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SdDrawXMLImport_Style_getImplementationName(),
                SdDrawXMLImport_Style_createInstance,
                SdDrawXMLImport_Style_getSupportedServiceNames() );
        }
        else if( SdDrawXMLImport_Content_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SdDrawXMLImport_Content_getImplementationName(),
                SdDrawXMLImport_Content_createInstance,
                SdDrawXMLImport_Content_getSupportedServiceNames() );
        }
        else if( SdDrawXMLImport_Meta_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SdDrawXMLImport_Meta_getImplementationName(),
                SdDrawXMLImport_Meta_createInstance,
                SdDrawXMLImport_Meta_getSupportedServiceNames() );
        }
        else if( SdImpressXMLExport_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SdImpressXMLExport_getImplementationName(),
                SdImpressXMLExport_createInstance,
                SdImpressXMLExport_getSupportedServiceNames() );
        }
        else if( SdImpressXMLExport_Style_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SdImpressXMLExport_Style_getImplementationName(),
                SdImpressXMLExport_Style_createInstance,
                SdImpressXMLExport_Style_getSupportedServiceNames() );
        }
        else if( SdImpressXMLExport_Content_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SdImpressXMLExport_Content_getImplementationName(),
                SdImpressXMLExport_Content_createInstance,
                SdImpressXMLExport_Content_getSupportedServiceNames() );
        }
        else if( SdImpressXMLExport_Meta_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SdImpressXMLExport_Meta_getImplementationName(),
                SdImpressXMLExport_Meta_createInstance,
                SdImpressXMLExport_Meta_getSupportedServiceNames() );
        }
        else if( SdDrawXMLExport_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SdDrawXMLExport_getImplementationName(),
                SdDrawXMLExport_createInstance,
                SdDrawXMLExport_getSupportedServiceNames() );
        }
        else if( SdDrawXMLExport_Style_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SdDrawXMLExport_Style_getImplementationName(),
                SdDrawXMLExport_Style_createInstance,
                SdDrawXMLExport_Style_getSupportedServiceNames() );
        }
        else if( SdDrawXMLExport_Content_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SdDrawXMLExport_Content_getImplementationName(),
                SdDrawXMLExport_Content_createInstance,
                SdDrawXMLExport_Content_getSupportedServiceNames() );
        }
        else if( SdDrawXMLExport_Meta_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SdDrawXMLExport_Meta_getImplementationName(),
                SdDrawXMLExport_Meta_createInstance,
                SdDrawXMLExport_Meta_getSupportedServiceNames() );
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

        else if( SchXMLImport_Styles_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SchXMLImport_Styles_getImplementationName(),
                SchXMLImport_Styles_createInstance,
                SchXMLImport_Styles_getSupportedServiceNames() );
        }
        else if( SchXMLExport_Styles_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SchXMLExport_Styles_getImplementationName(),
                SchXMLExport_Styles_createInstance,
                SchXMLExport_Styles_getSupportedServiceNames() );
        }
        else if( SchXMLImport_Content_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SchXMLImport_Content_getImplementationName(),
                SchXMLImport_Content_createInstance,
                SchXMLImport_Content_getSupportedServiceNames() );
        }
        else if( SchXMLExport_Content_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SchXMLExport_Content_getImplementationName(),
                SchXMLExport_Content_createInstance,
                SchXMLExport_Content_getSupportedServiceNames() );
        }
        else if( SchXMLImport_Meta_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SchXMLImport_Meta_getImplementationName(),
                SchXMLImport_Meta_createInstance,
                SchXMLImport_Meta_getSupportedServiceNames() );
        }
        else if( SchXMLExport_Meta_getImplementationName().equalsAsciiL( pImplName, nImplNameLen ) )
        {
            xFactory = ::cppu::createSingleFactory( xMSF,
                SchXMLExport_Meta_getImplementationName(),
                SchXMLExport_Meta_createInstance,
                SchXMLExport_Meta_getSupportedServiceNames() );
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
