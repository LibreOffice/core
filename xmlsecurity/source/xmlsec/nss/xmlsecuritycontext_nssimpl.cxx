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
#include <rtl/uuid.h>
#include "securityenvironment_nssimpl.hxx"

#include "xmlsecuritycontext_nssimpl.hxx"
#include "xmlstreamio.hxx"

#include "xmlsecurity/xmlsec-wrapper.h"

using namespace ::com::sun::star::uno ;
using namespace ::com::sun::star::lang ;
using ::com::sun::star::lang::XMultiServiceFactory ;
using ::com::sun::star::lang::XSingleServiceFactory ;
using ::rtl::OUString ;

using ::com::sun::star::xml::crypto::XSecurityEnvironment ;
using ::com::sun::star::xml::crypto::XXMLSecurityContext ;

XMLSecurityContext_NssImpl :: XMLSecurityContext_NssImpl( const Reference< XMultiServiceFactory >& aFactory )
    ://i39448 : m_pKeysMngr( NULL ) ,
    m_xServiceManager( aFactory ) ,
    m_nDefaultEnvIndex(-1)
    //m_xSecurityEnvironment( NULL )
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

XMLSecurityContext_NssImpl :: ~XMLSecurityContext_NssImpl() {
    //i39448

    xmlDisableStreamInputCallbacks() ;
    xmlSecCryptoShutdown() ;
    xmlSecShutdown() ;
}

//i39448 : new methods
sal_Int32 SAL_CALL XMLSecurityContext_NssImpl::addSecurityEnvironment(
    const ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment >& aSecurityEnvironment)
    throw (::com::sun::star::security::SecurityInfrastructureException, ::com::sun::star::uno::RuntimeException)
{
    if( !aSecurityEnvironment.is() )
    {
        throw RuntimeException() ;
    }

    m_vSecurityEnvironments.push_back( aSecurityEnvironment );

    return m_vSecurityEnvironments.size() - 1 ;
}


sal_Int32 SAL_CALL XMLSecurityContext_NssImpl::getSecurityEnvironmentNumber(  )
    throw (::com::sun::star::uno::RuntimeException)
{
    return m_vSecurityEnvironments.size();
}

::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment > SAL_CALL
    XMLSecurityContext_NssImpl::getSecurityEnvironmentByIndex( sal_Int32 index )
    throw (::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment > xSecurityEnvironment;

    if (index >= 0 && index < ( sal_Int32 )m_vSecurityEnvironments.size())
    {
        xSecurityEnvironment = m_vSecurityEnvironments[index];
    }
    else
        throw RuntimeException() ;

    return xSecurityEnvironment;
}

::com::sun::star::uno::Reference< ::com::sun::star::xml::crypto::XSecurityEnvironment > SAL_CALL
    XMLSecurityContext_NssImpl::getSecurityEnvironment(  )
    throw (::com::sun::star::uno::RuntimeException)
{
    if (m_nDefaultEnvIndex >= 0 && m_nDefaultEnvIndex < ( sal_Int32 )m_vSecurityEnvironments.size())
        return getSecurityEnvironmentByIndex(m_nDefaultEnvIndex);
    else
        throw RuntimeException() ;
}

sal_Int32 SAL_CALL XMLSecurityContext_NssImpl::getDefaultSecurityEnvironmentIndex(  )
    throw (::com::sun::star::uno::RuntimeException)
{
    return m_nDefaultEnvIndex ;
}

void SAL_CALL XMLSecurityContext_NssImpl::setDefaultSecurityEnvironmentIndex( sal_Int32 nDefaultEnvIndex )
    throw (::com::sun::star::uno::RuntimeException)
{
    m_nDefaultEnvIndex = nDefaultEnvIndex;
}

//i39448 : old methods deleted


/* XInitialization */
void SAL_CALL XMLSecurityContext_NssImpl :: initialize( const Sequence< Any >& /*aArguments*/ ) throw( Exception, RuntimeException ) {
    // TBD
} ;

/* XServiceInfo */
OUString SAL_CALL XMLSecurityContext_NssImpl :: getImplementationName() throw( RuntimeException ) {
    return impl_getImplementationName() ;
}

/* XServiceInfo */
sal_Bool SAL_CALL XMLSecurityContext_NssImpl :: supportsService( const OUString& serviceName) throw( RuntimeException ) {
    Sequence< OUString > seqServiceNames = getSupportedServiceNames() ;
    const OUString* pArray = seqServiceNames.getConstArray() ;
    for( sal_Int32 i = 0 ; i < seqServiceNames.getLength() ; i ++ ) {
        if( *( pArray + i ) == serviceName )
            return sal_True ;
    }
    return sal_False ;
}

/* XServiceInfo */
Sequence< OUString > SAL_CALL XMLSecurityContext_NssImpl :: getSupportedServiceNames() throw( RuntimeException ) {
    return impl_getSupportedServiceNames() ;
}

//Helper for XServiceInfo
Sequence< OUString > XMLSecurityContext_NssImpl :: impl_getSupportedServiceNames() {
    ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() ) ;
    Sequence< OUString > seqServiceNames( 1 ) ;
    seqServiceNames.getArray()[0] = OUString("com.sun.star.xml.crypto.XMLSecurityContext") ;
    return seqServiceNames ;
}

OUString XMLSecurityContext_NssImpl :: impl_getImplementationName() throw( RuntimeException ) {
    return OUString("com.sun.star.xml.security.bridge.xmlsec.XMLSecurityContext_NssImpl") ;
}

//Helper for registry
Reference< XInterface > SAL_CALL XMLSecurityContext_NssImpl :: impl_createInstance( const Reference< XMultiServiceFactory >& aServiceManager ) throw( RuntimeException ) {
    return Reference< XInterface >( *new XMLSecurityContext_NssImpl( aServiceManager ) ) ;
}

Reference< XSingleServiceFactory > XMLSecurityContext_NssImpl :: impl_createFactory( const Reference< XMultiServiceFactory >& aServiceManager ) {
    //Reference< XSingleServiceFactory > xFactory ;
    //xFactory = ::cppu::createSingleFactory( aServiceManager , impl_getImplementationName , impl_createInstance , impl_getSupportedServiceNames ) ;
    //return xFactory ;
    return ::cppu::createSingleFactory( aServiceManager , impl_getImplementationName() , impl_createInstance , impl_getSupportedServiceNames() ) ;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
