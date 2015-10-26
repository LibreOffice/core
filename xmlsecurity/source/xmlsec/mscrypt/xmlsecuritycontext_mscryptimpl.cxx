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
#include "securityenvironment_mscryptimpl.hxx"

#include "xmlsecuritycontext_mscryptimpl.hxx"
#include "xmlstreamio.hxx"

#include "xmlsecurity/xmlsec-wrapper.h"
#include "xmlsec/mscrypto/akmngr.h"

using namespace ::com::sun::star::uno ;
using namespace ::com::sun::star::lang ;
using ::com::sun::star::lang::XMultiServiceFactory ;
using ::com::sun::star::lang::XSingleServiceFactory ;

using ::com::sun::star::xml::crypto::XSecurityEnvironment ;
using ::com::sun::star::xml::crypto::XXMLSecurityContext ;

XMLSecurityContext_MSCryptImpl::XMLSecurityContext_MSCryptImpl()
    ://m_pKeysMngr( NULL ) ,
     m_xSecurityEnvironment( NULL )
{
    //Init xmlsec library
    if( xmlSecInit() < 0 ) {
        throw RuntimeException() ;
    }

    //Init xmlsec crypto engine library
    if( xmlSecCryptoInit() < 0 ) {
        xmlSecShutdown() ;
        throw RuntimeException() ;
    }

    //Enable external stream handlers
    if( xmlEnableStreamInputCallbacks() < 0 ) {
        xmlSecCryptoShutdown() ;
        xmlSecShutdown() ;
        throw RuntimeException() ;
    }
}

XMLSecurityContext_MSCryptImpl::~XMLSecurityContext_MSCryptImpl() {
    xmlDisableStreamInputCallbacks() ;
    xmlSecCryptoShutdown() ;
    xmlSecShutdown() ;
}

sal_Int32 SAL_CALL XMLSecurityContext_MSCryptImpl::addSecurityEnvironment(
    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment >& aSecurityEnvironment)
    throw (::com::sun::star::security::SecurityInfrastructureException, ::com::sun::star::uno::RuntimeException)
{
    if( !aSecurityEnvironment.is() )
    {
        throw RuntimeException() ;
    }

    m_xSecurityEnvironment = aSecurityEnvironment;

    return 0;
}


sal_Int32 SAL_CALL XMLSecurityContext_MSCryptImpl::getSecurityEnvironmentNumber(  )
    throw (::com::sun::star::uno::RuntimeException)
{
    return 1;
}

::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment > SAL_CALL
    XMLSecurityContext_MSCryptImpl::getSecurityEnvironmentByIndex( sal_Int32 index )
    throw (::com::sun::star::uno::RuntimeException)
{
    if (index == 0)
    {
        return m_xSecurityEnvironment;
    }
    else
        throw RuntimeException() ;
}

::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment > SAL_CALL
    XMLSecurityContext_MSCryptImpl::getSecurityEnvironment(  )
    throw (::com::sun::star::uno::RuntimeException)
{
    return m_xSecurityEnvironment;
}

sal_Int32 SAL_CALL XMLSecurityContext_MSCryptImpl::getDefaultSecurityEnvironmentIndex(  )
    throw (::com::sun::star::uno::RuntimeException)
{
    return 0;
}

void SAL_CALL XMLSecurityContext_MSCryptImpl::setDefaultSecurityEnvironmentIndex( sal_Int32 /*nDefaultEnvIndex*/ )
    throw (::com::sun::star::uno::RuntimeException)
{
    //dummy
}

/* XServiceInfo */
OUString SAL_CALL XMLSecurityContext_MSCryptImpl::getImplementationName() throw( RuntimeException ) {
    return impl_getImplementationName() ;
}

/* XServiceInfo */
sal_Bool SAL_CALL XMLSecurityContext_MSCryptImpl::supportsService( const OUString& serviceName) throw( RuntimeException ) {
    Sequence< OUString > seqServiceNames = getSupportedServiceNames() ;
    const OUString* pArray = seqServiceNames.getConstArray() ;
    for( sal_Int32 i = 0 ; i < seqServiceNames.getLength() ; i ++ ) {
        if( *( pArray + i ) == serviceName )
            return sal_True ;
    }
    return sal_False ;
}

/* XServiceInfo */
Sequence< OUString > SAL_CALL XMLSecurityContext_MSCryptImpl::getSupportedServiceNames() throw( RuntimeException ) {
    return impl_getSupportedServiceNames() ;
}

//Helper for XServiceInfo
Sequence< OUString > XMLSecurityContext_MSCryptImpl::impl_getSupportedServiceNames() {
    ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() ) ;
    Sequence< OUString > seqServiceNames( 1 ) ;
    seqServiceNames[0] = "com.sun.star.xml.crypto.XMLSecurityContext";
    return seqServiceNames ;
}

OUString XMLSecurityContext_MSCryptImpl::impl_getImplementationName() throw( RuntimeException ) {
    return OUString("com.sun.star.xml.security.bridge.xmlsec.XMLSecurityContext_MSCryptImpl") ;
}

//Helper for registry
Reference< XInterface > SAL_CALL XMLSecurityContext_MSCryptImpl::impl_createInstance( const Reference< XMultiServiceFactory >& ) throw( RuntimeException ) {
    return Reference< XInterface >( *new XMLSecurityContext_MSCryptImpl ) ;
}

Reference< XSingleServiceFactory > XMLSecurityContext_MSCryptImpl::impl_createFactory( const Reference< XMultiServiceFactory >& aServiceManager ) {
    //Reference< XSingleServiceFactory > xFactory ;
    //xFactory = ::cppu::createSingleFactory( aServiceManager , impl_getImplementationName , impl_createInstance , impl_getSupportedServiceNames ) ;
    //return xFactory ;
    return ::cppu::createSingleFactory( aServiceManager , impl_getImplementationName() , impl_createInstance , impl_getSupportedServiceNames() ) ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
