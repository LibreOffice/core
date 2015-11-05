/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <sal/config.h>

#include <osl/mutex.hxx>
#include <osl/thread.h>
#include <cppuhelper/factory.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>

#include "seinitializer_mscryptimpl.hxx"
#include "xmlsignature_mscryptimpl.hxx"
#include "xmlencryption_mscryptimpl.hxx"
#include "xmlsecuritycontext_mscryptimpl.hxx"
#include "securityenvironment_mscryptimpl.hxx"

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
            xFactory.set( createSingleFactory(
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
