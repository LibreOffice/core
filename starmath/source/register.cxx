/*************************************************************************
 *
 *  $RCSfile: register.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2003-04-04 19:12:51 $
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

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#include <sfx2/sfxuno.hxx>

#include "smdll.hxx"
#include "document.hxx"

using namespace ::com::sun::star::lang;

//Math document
extern ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
SmDocument_getSupportedServiceNames() throw();
extern ::rtl::OUString SAL_CALL SmDocument_getImplementationName() throw();
extern ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
SAL_CALL SmDocument_createInstance(const ::com::sun::star::uno::Reference<
    ::com::sun::star::lang::XMultiServiceFactory > & rSMgr) throw(
    ::com::sun::star::uno::Exception );

//MathML import
extern ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
SmXMLImport_getSupportedServiceNames() throw();
extern ::rtl::OUString SAL_CALL SmXMLImport_getImplementationName() throw();
extern ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
SAL_CALL SmXMLImport_createInstance(const ::com::sun::star::uno::Reference<
    ::com::sun::star::lang::XMultiServiceFactory > & rSMgr) throw(
    ::com::sun::star::uno::Exception );
extern ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
SmXMLImportMeta_getSupportedServiceNames() throw();
extern ::rtl::OUString SAL_CALL SmXMLImportMeta_getImplementationName() throw();
extern ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
SAL_CALL SmXMLImportMeta_createInstance(const ::com::sun::star::uno::Reference<
    ::com::sun::star::lang::XMultiServiceFactory > & rSMgr) throw(
    ::com::sun::star::uno::Exception );
extern ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
SmXMLImportSettings_getSupportedServiceNames() throw();
extern ::rtl::OUString SAL_CALL SmXMLImportSettings_getImplementationName() throw();
extern ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
SAL_CALL SmXMLImportSettings_createInstance(const ::com::sun::star::uno::Reference<
    ::com::sun::star::lang::XMultiServiceFactory > & rSMgr) throw(
    ::com::sun::star::uno::Exception );

//MathML export
extern ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
SmXMLExport_getSupportedServiceNames() throw();
extern ::rtl::OUString SAL_CALL SmXMLExport_getImplementationName() throw();
extern ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
SAL_CALL SmXMLExport_createInstance(const ::com::sun::star::uno::Reference<
    ::com::sun::star::lang::XMultiServiceFactory > & rSMgr) throw(
    ::com::sun::star::uno::Exception );
extern ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
SmXMLExportMeta_getSupportedServiceNames() throw();
extern ::rtl::OUString SAL_CALL SmXMLExportMeta_getImplementationName() throw();
extern ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
SAL_CALL SmXMLExportMeta_createInstance(const ::com::sun::star::uno::Reference<
    ::com::sun::star::lang::XMultiServiceFactory > & rSMgr) throw(
    ::com::sun::star::uno::Exception );
extern ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
SmXMLExportSettings_getSupportedServiceNames() throw();
extern ::rtl::OUString SAL_CALL SmXMLExportSettings_getImplementationName() throw();
extern ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
SAL_CALL SmXMLExportSettings_createInstance(const ::com::sun::star::uno::Reference<
    ::com::sun::star::lang::XMultiServiceFactory > & rSMgr) throw(
    ::com::sun::star::uno::Exception );
extern ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
SmXMLExportContent_getSupportedServiceNames() throw();
extern ::rtl::OUString SAL_CALL SmXMLExportContent_getImplementationName() throw();
extern ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
SAL_CALL SmXMLExportContent_createInstance(const ::com::sun::star::uno::Reference<
    ::com::sun::star::lang::XMultiServiceFactory > & rSMgr) throw(
    ::com::sun::star::uno::Exception );

extern "C" {

void SAL_CALL component_getImplementationEnvironment(   const   sal_Char**          ppEnvironmentTypeName   ,
                                                                uno_Environment**   ppEnvironment           )
{
    *ppEnvironmentTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME ;
}

sal_Bool SAL_CALL component_writeInfo(  void*   pServiceManager ,
                                        void*   pRegistryKey    )
{
    ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey >
            xKey( reinterpret_cast< ::com::sun::star::registry::XRegistryKey* >( pRegistryKey ) ) ;

    // Eigentliche Implementierung und ihre Services registrieren
    sal_Int32 i;
    ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey >  xNewKey;

    xNewKey = xKey->createKey(::rtl::OUString(
    RTL_CONSTASCII_USTRINGPARAM("/") ) + SmXMLImport_getImplementationName() +
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES") )  );

    ::com::sun::star::uno::Sequence< rtl::OUString > &rServices =
        SmXMLImport_getSupportedServiceNames();
    for(i = 0; i < rServices.getLength(); i++ )
        xNewKey->createKey( rServices.getConstArray()[i]);

    xNewKey = xKey->createKey(::rtl::OUString(
    RTL_CONSTASCII_USTRINGPARAM("/") ) + SmXMLExport_getImplementationName() +
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES") )  );

    rServices = SmXMLExport_getSupportedServiceNames();
    for(i = 0; i < rServices.getLength(); i++ )
        xNewKey->createKey( rServices.getConstArray()[i]);

    xNewKey = xKey->createKey(::rtl::OUString(
    RTL_CONSTASCII_USTRINGPARAM("/") ) + SmXMLImportMeta_getImplementationName() +
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES") )  );

    rServices = SmXMLImportMeta_getSupportedServiceNames();
    for(i = 0; i < rServices.getLength(); i++ )
        xNewKey->createKey( rServices.getConstArray()[i]);

    xNewKey = xKey->createKey(::rtl::OUString(
    RTL_CONSTASCII_USTRINGPARAM("/") ) + SmXMLExportMeta_getImplementationName() +
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES") )  );

    rServices = SmXMLExportMeta_getSupportedServiceNames();
    for(i = 0; i < rServices.getLength(); i++ )
        xNewKey->createKey( rServices.getConstArray()[i]);

    xNewKey = xKey->createKey(::rtl::OUString(
    RTL_CONSTASCII_USTRINGPARAM("/") ) + SmXMLImportSettings_getImplementationName() +
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES") )  );

    rServices = SmXMLImportSettings_getSupportedServiceNames();
    for(i = 0; i < rServices.getLength(); i++ )
        xNewKey->createKey( rServices.getConstArray()[i]);

    xNewKey = xKey->createKey(::rtl::OUString(
    RTL_CONSTASCII_USTRINGPARAM("/") ) + SmXMLExportSettings_getImplementationName() +
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES") )  );

    rServices = SmXMLExportSettings_getSupportedServiceNames();
    for(i = 0; i < rServices.getLength(); i++ )
        xNewKey->createKey( rServices.getConstArray()[i]);

    xNewKey = xKey->createKey(::rtl::OUString(
    RTL_CONSTASCII_USTRINGPARAM("/") ) + SmXMLExportContent_getImplementationName() +
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES") )  );

    rServices = SmXMLExportContent_getSupportedServiceNames();
    for(i = 0; i < rServices.getLength(); i++ )
        xNewKey->createKey( rServices.getConstArray()[i]);

    xNewKey = xKey->createKey(::rtl::OUString(
    RTL_CONSTASCII_USTRINGPARAM("/") ) + SmDocument_getImplementationName() +
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES") )  );

    rServices = SmDocument_getSupportedServiceNames();
    for(i = 0; i < rServices.getLength(); i++ )
        xNewKey->createKey( rServices.getConstArray()[i]);


    return sal_True;
}

void* SAL_CALL component_getFactory(    const   sal_Char*   pImplementationName ,
                                                void*       pServiceManager     ,
                                                void*       pRegistryKey        )
{
    // Set default return value for this operation - if it failed.
    void* pReturn = NULL ;

    if  (
            ( pImplementationName   !=  NULL ) &&
            ( pServiceManager       !=  NULL )
        )
    {
        // Define variables which are used in following macros.
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory >   xFactory                                                                                                ;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    xServiceManager( reinterpret_cast< ::com::sun::star::lang::XMultiServiceFactory* >( pServiceManager ) ) ;

        if( SmXMLImport_getImplementationName().equalsAsciiL(
            pImplementationName, strlen(pImplementationName)) )
        {
            xFactory = ::cppu::createSingleFactory( xServiceManager,
            SmXMLImport_getImplementationName(),
            SmXMLImport_createInstance,
            SmXMLImport_getSupportedServiceNames() );
        }
        else if( SmXMLExport_getImplementationName().equalsAsciiL(
            pImplementationName, strlen(pImplementationName)) )
        {
            xFactory = ::cppu::createSingleFactory( xServiceManager,
            SmXMLExport_getImplementationName(),
            SmXMLExport_createInstance,
            SmXMLExport_getSupportedServiceNames() );
        }
        else if( SmXMLImportMeta_getImplementationName().equalsAsciiL(
            pImplementationName, strlen(pImplementationName)) )
        {
            xFactory = ::cppu::createSingleFactory( xServiceManager,
            SmXMLImportMeta_getImplementationName(),
            SmXMLImportMeta_createInstance,
            SmXMLImportMeta_getSupportedServiceNames() );
        }
        else if( SmXMLExportMeta_getImplementationName().equalsAsciiL(
            pImplementationName, strlen(pImplementationName)) )
        {
            xFactory = ::cppu::createSingleFactory( xServiceManager,
            SmXMLExportMeta_getImplementationName(),
            SmXMLExportMeta_createInstance,
            SmXMLExportMeta_getSupportedServiceNames() );
        }
        else if( SmXMLImportSettings_getImplementationName().equalsAsciiL(
            pImplementationName, strlen(pImplementationName)) )
        {
            xFactory = ::cppu::createSingleFactory( xServiceManager,
            SmXMLImportSettings_getImplementationName(),
            SmXMLImportSettings_createInstance,
            SmXMLImportSettings_getSupportedServiceNames() );
        }
        else if( SmXMLExportSettings_getImplementationName().equalsAsciiL(
            pImplementationName, strlen(pImplementationName)) )
        {
            xFactory = ::cppu::createSingleFactory( xServiceManager,
            SmXMLExportSettings_getImplementationName(),
            SmXMLExportSettings_createInstance,
            SmXMLExportSettings_getSupportedServiceNames() );
        }
        else if( SmXMLExportContent_getImplementationName().equalsAsciiL(
            pImplementationName, strlen(pImplementationName)) )
        {
            xFactory = ::cppu::createSingleFactory( xServiceManager,
            SmXMLExportContent_getImplementationName(),
            SmXMLExportContent_createInstance,
            SmXMLExportContent_getSupportedServiceNames() );
        }
        else if( SmDocument_getImplementationName().equalsAsciiL(
            pImplementationName, strlen(pImplementationName)) )
        {
            xFactory = ::cppu::createSingleFactory( xServiceManager,
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



