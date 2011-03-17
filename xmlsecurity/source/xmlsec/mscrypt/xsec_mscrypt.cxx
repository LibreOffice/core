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
#include "precompiled_xmlsecurity.hxx"

#include <sal/config.h>
#include <stdio.h>

#include <osl/mutex.hxx>
#include <osl/thread.h>
#include <cppuhelper/factory.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

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

void* SAL_CALL mscrypt_component_getFactory( const sal_Char* pImplName , void* pServiceManager , void* /*pRegistryKey*/ )
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
