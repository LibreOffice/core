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

#include "seinitializer_nssimpl.hxx"
#include "xmlsignature_nssimpl.hxx"
#include "xmlencryption_nssimpl.hxx"
#include "xmlsecuritycontext_nssimpl.hxx"
#include "xsec_xmlsec.hxx"
#include "securityenvironment_nssimpl.hxx"

using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

extern "C"
{

void* SAL_CALL nss_component_getFactory( const sal_Char* pImplName , void* pServiceManager , void* /*pRegistryKey*/ )
{
    void* pRet = nullptr;
    Reference< XSingleServiceFactory > xFactory ;

    if( pImplName != nullptr && pServiceManager != nullptr )
    {
#ifdef XMLSEC_CRYPTO_NSS
        if( SEInitializer_NssImpl_getImplementationName().equals( OUString::createFromAscii( pImplName ) ) )
        {
            xFactory.set( createSingleFactory(
                static_cast< XMultiServiceFactory * >( pServiceManager ),
                OUString::createFromAscii( pImplName ),
                SEInitializer_NssImpl_createInstance, SEInitializer_NssImpl_getSupportedServiceNames() ) );
        }
        else if( XMLSignature_NssImpl::impl_getImplementationName().equals( OUString::createFromAscii( pImplName ) ) )
        {
            xFactory = XMLSignature_NssImpl::impl_createFactory( static_cast< XMultiServiceFactory* >( pServiceManager ) ) ;
        }
        else if( XMLSecurityContext_NssImpl::impl_getImplementationName().equals( OUString::createFromAscii( pImplName ) ) )
        {
            xFactory = XMLSecurityContext_NssImpl::impl_createFactory( static_cast< XMultiServiceFactory* >( pServiceManager ) ) ;
        }
        else if( SecurityEnvironment_NssImpl::impl_getImplementationName().equals( OUString::createFromAscii( pImplName ) ) )
        {
            xFactory = SecurityEnvironment_NssImpl::impl_createFactory( static_cast< XMultiServiceFactory* >( pServiceManager ) ) ;
        }
        else if( XMLEncryption_NssImpl::impl_getImplementationName().equals( OUString::createFromAscii( pImplName ) ) )
        {
            xFactory = XMLEncryption_NssImpl::impl_createFactory( static_cast< XMultiServiceFactory* >( pServiceManager ) ) ;
        }
#else
        if( ONSSInitializer_getImplementationName().equals( OUString::createFromAscii( pImplName ) ) )
        {
            xFactory.set( createSingleFactory(
                reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
                OUString::createFromAscii( pImplName ),
                ONSSInitializer_createInstance, ONSSInitializer_getSupportedServiceNames() ) );
        }
#endif
    }

    if( xFactory.is() ) {
        xFactory->acquire() ;
        pRet = xFactory.get() ;
    }

    return pRet ;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
