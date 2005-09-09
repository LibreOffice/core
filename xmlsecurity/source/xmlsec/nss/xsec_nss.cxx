/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xsec_nss.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 17:36:50 $
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

#include <sal/config.h>
#include <stdio.h>

#include <osl/mutex.hxx>
#include <osl/thread.h>
#include <cppuhelper/factory.hxx>

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

#include "seinitializer_nssimpl.hxx"
#include "xmlsignature_nssimpl.hxx"
#include "xmlencryption_nssimpl.hxx"
#include "xmlsecuritycontext_nssimpl.hxx"
#include "securityenvironment_nssimpl.hxx"

using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

extern "C"
{

sal_Bool SAL_CALL nss_component_writeInfo( void* pServiceManager , void* pRegistryKey )
{
    sal_Bool result = sal_False;
    sal_Int32 i ;
    OUString sKeyName ;
    Reference< XRegistryKey > xNewKey ;
    Sequence< OUString > seqServices ;
    Reference< XRegistryKey > xKey( reinterpret_cast< XRegistryKey* >( pRegistryKey ) ) ;

    if( xKey.is() ) {
        //  try {
        // XMLSignature_NssImpl
        sKeyName = OUString( RTL_CONSTASCII_USTRINGPARAM( "/" ) ) ;
        sKeyName += XMLSignature_NssImpl::impl_getImplementationName() ;
        sKeyName += OUString::createFromAscii( "/UNO/SERVICES" ) ;

        xNewKey = xKey->createKey( sKeyName ) ;
        if( xNewKey.is() ) {
            seqServices = XMLSignature_NssImpl::impl_getSupportedServiceNames() ;
            for( i = seqServices.getLength() ; i -- ;  )
                xNewKey->createKey( seqServices.getConstArray()[i] ) ;
        }

        // XMLEncryption_NssImpl
        sKeyName = OUString( RTL_CONSTASCII_USTRINGPARAM( "/" ) ) ;
        sKeyName += XMLEncryption_NssImpl::impl_getImplementationName() ;
        sKeyName += OUString::createFromAscii( "/UNO/SERVICES" ) ;

        xNewKey = xKey->createKey( sKeyName ) ;
        if( xNewKey.is() ) {
            seqServices = XMLEncryption_NssImpl::impl_getSupportedServiceNames() ;
            for( i = seqServices.getLength() ; i -- ;  )
                xNewKey->createKey( seqServices.getConstArray()[i] ) ;
        }

        // XMLSecurityContext_NssImpl
        sKeyName = OUString( RTL_CONSTASCII_USTRINGPARAM( "/" ) ) ;
        sKeyName += XMLSecurityContext_NssImpl::impl_getImplementationName() ;
        sKeyName += OUString::createFromAscii( "/UNO/SERVICES" ) ;

        xNewKey = xKey->createKey( sKeyName ) ;
        if( xNewKey.is() ) {
            seqServices = XMLSecurityContext_NssImpl::impl_getSupportedServiceNames() ;
            for( i = seqServices.getLength() ; i -- ;  )
                xNewKey->createKey( seqServices.getConstArray()[i] ) ;
        }

        // SecurityEnvironment_NssImpl
        sKeyName = OUString( RTL_CONSTASCII_USTRINGPARAM( "/" ) ) ;
        sKeyName += SecurityEnvironment_NssImpl::impl_getImplementationName() ;
        sKeyName += OUString::createFromAscii( "/UNO/SERVICES" ) ;

        xNewKey = xKey->createKey( sKeyName ) ;
        if( xNewKey.is() ) {
            seqServices = SecurityEnvironment_NssImpl::impl_getSupportedServiceNames() ;
            for( i = seqServices.getLength() ; i -- ;  )
                xNewKey->createKey( seqServices.getConstArray()[i] ) ;
        }

        // SEInitializer_NssImpl
        sKeyName = OUString( RTL_CONSTASCII_USTRINGPARAM( "/" ) ) ;
        sKeyName += SEInitializer_NssImpl_getImplementationName() ;
        sKeyName += OUString::createFromAscii( "/UNO/SERVICES" ) ;

        xNewKey = xKey->createKey( sKeyName ) ;
        if( xNewKey.is() ) {
            seqServices = SEInitializer_NssImpl_getSupportedServiceNames() ;
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

void* SAL_CALL nss_component_getFactory( const sal_Char* pImplName , void* pServiceManager , void* pRegistryKey )
{
    void* pRet = 0;
    Reference< XSingleServiceFactory > xFactory ;

    if( pImplName != NULL && pServiceManager != NULL ) {
        if( XMLSignature_NssImpl::impl_getImplementationName().equals( OUString::createFromAscii( pImplName ) ) ) {
            xFactory = XMLSignature_NssImpl::impl_createFactory( reinterpret_cast< XMultiServiceFactory* >( pServiceManager ) ) ;
        } else if( XMLSecurityContext_NssImpl::impl_getImplementationName().equals( OUString::createFromAscii( pImplName ) ) ) {
            xFactory = XMLSecurityContext_NssImpl::impl_createFactory( reinterpret_cast< XMultiServiceFactory* >( pServiceManager ) ) ;
        } else if( SecurityEnvironment_NssImpl::impl_getImplementationName().equals( OUString::createFromAscii( pImplName ) ) ) {
            xFactory = SecurityEnvironment_NssImpl::impl_createFactory( reinterpret_cast< XMultiServiceFactory* >( pServiceManager ) ) ;
        } else if( XMLEncryption_NssImpl::impl_getImplementationName().equals( OUString::createFromAscii( pImplName ) ) ) {
            xFactory = XMLEncryption_NssImpl::impl_createFactory( reinterpret_cast< XMultiServiceFactory* >( pServiceManager ) ) ;
        } else if( SEInitializer_NssImpl_getImplementationName().equals( OUString::createFromAscii( pImplName ) ) ) {
            xFactory = Reference< XSingleServiceFactory >( createSingleFactory(
                reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
                OUString::createFromAscii( pImplName ),
                SEInitializer_NssImpl_createInstance, SEInitializer_NssImpl_getSupportedServiceNames() ) );
        }
    }

    if( xFactory.is() ) {
        xFactory->acquire() ;
        pRet = xFactory.get() ;
    }

    return pRet ;
}

}

