/*************************************************************************
 *
 *  $RCSfile: xsec_mscrypt.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: mt $ $Date: 2004-07-12 13:15:22 $
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

#include "seinitializer_mscryptimpl.hxx"
#include "xmlsignature_mscryptimpl.hxx"
#include "xmlencryption_mscryptimpl.hxx"
#include "xmlsecuritycontext_mscryptimpl.hxx"
#include "securityenvironment_mscryptimpl.hxx"

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

extern "C"
{

sal_Bool SAL_CALL mscrypt_component_writeInfo( void* pServiceManager , void* pRegistryKey )
{
    sal_Bool result = sal_False;
    sal_Int32 i ;
    OUString sKeyName ;
    Reference< XRegistryKey > xNewKey ;
    Sequence< OUString > seqServices ;
    Reference< XRegistryKey > xKey( reinterpret_cast< XRegistryKey* >( pRegistryKey ) ) ;

    if( xKey.is() ) {
        //  try {
        // XMLSignature_MSCryptImpl
        sKeyName = OUString( RTL_CONSTASCII_USTRINGPARAM( "/" ) ) ;
        sKeyName += XMLSignature_MSCryptImpl::impl_getImplementationName() ;
        sKeyName += OUString::createFromAscii( "/UNO/SERVICES" ) ;

        xNewKey = xKey->createKey( sKeyName ) ;
        if( xNewKey.is() ) {
            seqServices = XMLSignature_MSCryptImpl::impl_getSupportedServiceNames() ;
            for( i = seqServices.getLength() ; i -- ;  )
                xNewKey->createKey( seqServices.getConstArray()[i] ) ;
        }

        // XMLEncryption_MSCryptImpl
        sKeyName = OUString( RTL_CONSTASCII_USTRINGPARAM( "/" ) ) ;
        sKeyName += XMLEncryption_MSCryptImpl::impl_getImplementationName() ;
        sKeyName += OUString::createFromAscii( "/UNO/SERVICES" ) ;

        xNewKey = xKey->createKey( sKeyName ) ;
        if( xNewKey.is() ) {
            seqServices = XMLEncryption_MSCryptImpl::impl_getSupportedServiceNames() ;
            for( i = seqServices.getLength() ; i -- ;  )
                xNewKey->createKey( seqServices.getConstArray()[i] ) ;
        }

        // XMLSecurityContext_MSCryptImpl
        sKeyName = OUString( RTL_CONSTASCII_USTRINGPARAM( "/" ) ) ;
        sKeyName += XMLSecurityContext_MSCryptImpl::impl_getImplementationName() ;
        sKeyName += OUString::createFromAscii( "/UNO/SERVICES" ) ;

        xNewKey = xKey->createKey( sKeyName ) ;
        if( xNewKey.is() ) {
            seqServices = XMLSecurityContext_MSCryptImpl::impl_getSupportedServiceNames() ;
            for( i = seqServices.getLength() ; i -- ;  )
                xNewKey->createKey( seqServices.getConstArray()[i] ) ;
        }

        // SecurityEnvironment_MSCryptImpl
        sKeyName = OUString( RTL_CONSTASCII_USTRINGPARAM( "/" ) ) ;
        sKeyName += SecurityEnvironment_MSCryptImpl::impl_getImplementationName() ;
        sKeyName += OUString::createFromAscii( "/UNO/SERVICES" ) ;

        xNewKey = xKey->createKey( sKeyName ) ;
        if( xNewKey.is() ) {
            seqServices = SecurityEnvironment_MSCryptImpl::impl_getSupportedServiceNames() ;
            for( i = seqServices.getLength() ; i -- ;  )
                xNewKey->createKey( seqServices.getConstArray()[i] ) ;
        }

        // SEInitializer_MSCryptImpl
        sKeyName = OUString( RTL_CONSTASCII_USTRINGPARAM( "/" ) ) ;
        sKeyName += SEInitializer_MSCryptImpl_getImplementationName() ;
        sKeyName += OUString::createFromAscii( "/UNO/SERVICES" ) ;

        xNewKey = xKey->createKey( sKeyName ) ;
        if( xNewKey.is() ) {
            seqServices = SEInitializer_MSCryptImpl_getSupportedServiceNames() ;
            for( i = seqServices.getLength() ; i -- ;  )
                xNewKey->createKey( seqServices.getConstArray()[i] ) ;
        }

        return sal_True;
        //} catch( InvalidRegistryException & ) {
        //  //we should not ignore exceptions
        //  return sal_False ;
        //}
    }
    return result;
}

void* SAL_CALL mscrypt_component_getFactory( const sal_Char* pImplName , void* pServiceManager , void* pRegistryKey )
{
    void* pRet = 0;
    Reference< XSingleServiceFactory > xFactory ;

    if( pImplName != NULL && pServiceManager != NULL ) {
        if( XMLSignature_MSCryptImpl::impl_getImplementationName().equals( OUString::createFromAscii( pImplName ) ) ) {
            xFactory = XMLSignature_MSCryptImpl::impl_createFactory( reinterpret_cast< XMultiServiceFactory* >( pServiceManager ) ) ;
        } else if( XMLSecurityContext_MSCryptImpl::impl_getImplementationName().equals( OUString::createFromAscii( pImplName ) ) ) {
            xFactory = XMLSecurityContext_MSCryptImpl::impl_createFactory( reinterpret_cast< XMultiServiceFactory* >( pServiceManager ) ) ;
        } else if( SecurityEnvironment_MSCryptImpl::impl_getImplementationName().equals( OUString::createFromAscii( pImplName ) ) ) {
            xFactory = SecurityEnvironment_MSCryptImpl::impl_createFactory( reinterpret_cast< XMultiServiceFactory* >( pServiceManager ) ) ;
        } else if( XMLEncryption_MSCryptImpl::impl_getImplementationName().equals( OUString::createFromAscii( pImplName ) ) ) {
            xFactory = XMLEncryption_MSCryptImpl::impl_createFactory( reinterpret_cast< XMultiServiceFactory* >( pServiceManager ) ) ;
        } else if( SEInitializer_MSCryptImpl_getImplementationName().equals( OUString::createFromAscii( pImplName ) ) ) {
            xFactory = Reference< XSingleServiceFactory >( createSingleFactory(
                reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
                OUString::createFromAscii( pImplName ),
                SEInitializer_MSCryptImpl_createInstance, SEInitializer_MSCryptImpl_getSupportedServiceNames() ) );
        }
    }

    if( xFactory.is() ) {
        xFactory->acquire() ;
        pRet = xFactory.get() ;
    }

    return pRet ;
}

}
