/*************************************************************************
 *
 *  $RCSfile: xsec_xmlsec.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: mt $ $Date: 2004-07-12 13:15:21 $
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

#include <sal/config.h>
#include <stdio.h>

#include <osl/mutex.hxx>
#include <osl/thread.h>
#include <cppuhelper/factory.hxx>

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

#include "xmlelementwrapper_xmlsecimpl.hxx"
#include "xmldocumentwrapper_xmlsecimpl.hxx"

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

extern "C"
{

#if defined( XMLSEC_CRYPTO_NSS )
extern sal_Bool nss_component_writeInfo( void*, void* );
extern void* nss_component_getFactory( const sal_Char*, void*, void* );
#endif

#if defined( XMLSEC_CRYPTO_MSCRYPTO )
extern sal_Bool mscrypt_component_writeInfo( void*, void* );
extern void* mscrypt_component_getFactory( const sal_Char*, void*, void* );
#endif

void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}


sal_Bool SAL_CALL component_writeInfo( void* pServiceManager , void* pRegistryKey )
{
    sal_Bool result = sal_False;
    sal_Int32 i ;
    OUString sKeyName ;
    Reference< XRegistryKey > xNewKey ;
    Sequence< OUString > seqServices ;
    Reference< XRegistryKey > xKey( reinterpret_cast< XRegistryKey* >( pRegistryKey ) ) ;

    if( xKey.is() ) {
        //  try {
        // XMLElementWrapper_XmlSecImpl
        sKeyName = OUString( RTL_CONSTASCII_USTRINGPARAM( "/" ) ) ;
        sKeyName += XMLElementWrapper_XmlSecImpl_getImplementationName() ;
        sKeyName += OUString::createFromAscii( "/UNO/SERVICES" ) ;

        xNewKey = xKey->createKey( sKeyName ) ;
        if( xNewKey.is() ) {
            seqServices = XMLElementWrapper_XmlSecImpl_getSupportedServiceNames() ;
            for( i = seqServices.getLength() ; i -- ;  )
                xNewKey->createKey( seqServices.getConstArray()[i] ) ;
        }

        // XMLDocumentWrapper_XmlSecImpl
        sKeyName = OUString( RTL_CONSTASCII_USTRINGPARAM( "/" ) ) ;
        sKeyName += XMLDocumentWrapper_XmlSecImpl_getImplementationName() ;
        sKeyName += OUString::createFromAscii( "/UNO/SERVICES" ) ;

        xNewKey = xKey->createKey( sKeyName ) ;
        if( xNewKey.is() ) {
            seqServices = XMLDocumentWrapper_XmlSecImpl_getSupportedServiceNames() ;
            for( i = seqServices.getLength() ; i -- ;  )
                xNewKey->createKey( seqServices.getConstArray()[i] ) ;
        }

#if defined( XMLSEC_CRYPTO_NSS )
        result = nss_component_writeInfo( pServiceManager, pRegistryKey ) ;
        if( !result )
            return sal_False ;
#endif

#if defined( XMLSEC_CRYPTO_MSCRYPTO )
        result = mscrypt_component_writeInfo( pServiceManager, pRegistryKey ) ;
        if( !result )
            return sal_False ;
#endif

        //} catch( InvalidRegistryException & ) {
        //  //we should not ignore exceptions
        //  return sal_False ;
        //}
    }

    return result;
}

void* SAL_CALL component_getFactory( const sal_Char* pImplName , void* pServiceManager , void* pRegistryKey )
{
    void* pRet = 0;
    Reference< XSingleServiceFactory > xFactory ;

    if( pImplName != NULL && pServiceManager != NULL ) {
        if( XMLElementWrapper_XmlSecImpl_getImplementationName().equals( OUString::createFromAscii( pImplName ) ) )
        {
            xFactory = Reference< XSingleServiceFactory >( createSingleFactory(
                reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
                OUString::createFromAscii( pImplName ),
                XMLElementWrapper_XmlSecImpl_createInstance, XMLElementWrapper_XmlSecImpl_getSupportedServiceNames() ) );
        }
        else if( XMLDocumentWrapper_XmlSecImpl_getImplementationName().equals( OUString::createFromAscii( pImplName ) ) )
        {
            xFactory = Reference< XSingleServiceFactory >( createSingleFactory(
                reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
                OUString::createFromAscii( pImplName ),
                XMLDocumentWrapper_XmlSecImpl_createInstance, XMLDocumentWrapper_XmlSecImpl_getSupportedServiceNames() ) );
        }
    }

    if( xFactory.is() ) {
        xFactory->acquire() ;
        pRet = xFactory.get() ;
    } else {
#if defined( XMLSEC_CRYPTO_NSS )
        pRet = nss_component_getFactory( pImplName, pServiceManager, pRegistryKey ) ;
        if( pRet != NULL )
            return pRet ;
#endif

#if defined( XMLSEC_CRYPTO_MSCRYPTO )
        pRet = mscrypt_component_getFactory( pImplName, pServiceManager, pRegistryKey ) ;
        if( pRet != NULL )
            return pRet ;
#endif
    }

    return pRet ;
}

}

