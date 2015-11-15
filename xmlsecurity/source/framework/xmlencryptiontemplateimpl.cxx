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
#include "xmlencryptiontemplateimpl.hxx"

using namespace ::com::sun::star::uno ;
using ::com::sun::star::lang::XMultiServiceFactory ;
using ::com::sun::star::lang::XSingleServiceFactory ;

using ::com::sun::star::xml::wrapper::XXMLElementWrapper ;
using ::com::sun::star::xml::crypto::XXMLEncryptionTemplate ;

XMLEncryptionTemplateImpl::XMLEncryptionTemplateImpl( const Reference< XMultiServiceFactory >& aFactory )
    : m_xTemplate( nullptr ),
      m_xTarget( nullptr ),
      m_xServiceManager( aFactory ),
      m_nStatus ( ::com::sun::star::xml::crypto::SecurityOperationStatus_UNKNOWN ) {
}

XMLEncryptionTemplateImpl::~XMLEncryptionTemplateImpl() {
}

/* XXMLEncryptionTemplate */
void SAL_CALL XMLEncryptionTemplateImpl::setTemplate( const Reference< XXMLElementWrapper >& aTemplate )
    throw (com::sun::star::uno::RuntimeException, com::sun::star::lang::IllegalArgumentException, std::exception)
{
    m_xTemplate = aTemplate ;
}

/* XXMLEncryptionTemplate */
Reference< XXMLElementWrapper > SAL_CALL XMLEncryptionTemplateImpl::getTemplate()
throw (com::sun::star::uno::RuntimeException, std::exception)
{
    return m_xTemplate ;
}

/* XXMLEncryptionTemplate */
void SAL_CALL XMLEncryptionTemplateImpl::setTarget( const Reference< XXMLElementWrapper >& aTarget )
    throw( com::sun::star::lang::IllegalArgumentException, std::exception ) {
    m_xTarget = aTarget ;
}

/* XXMLEncryptionTemplate */
Reference< XXMLElementWrapper > SAL_CALL XMLEncryptionTemplateImpl::getTarget()
throw (com::sun::star::uno::RuntimeException, std::exception)
{
    return m_xTarget ;
}

void SAL_CALL XMLEncryptionTemplateImpl::setStatus(
    ::com::sun::star::xml::crypto::SecurityOperationStatus status )
    throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception)
{
    m_nStatus = status;
}

::com::sun::star::xml::crypto::SecurityOperationStatus SAL_CALL XMLEncryptionTemplateImpl::getStatus(  )
    throw (::com::sun::star::uno::RuntimeException, std::exception)
{
    return m_nStatus;
}

/* XServiceInfo */
OUString SAL_CALL XMLEncryptionTemplateImpl::getImplementationName() throw( RuntimeException, std::exception ) {
    return impl_getImplementationName() ;
}

/* XServiceInfo */
sal_Bool SAL_CALL XMLEncryptionTemplateImpl::supportsService( const OUString& serviceName) throw( RuntimeException, std::exception ) {
    Sequence< OUString > seqServiceNames = getSupportedServiceNames() ;
    const OUString* pArray = seqServiceNames.getConstArray() ;
    for( sal_Int32 i = 0 ; i < seqServiceNames.getLength() ; i ++ ) {
        if( *( pArray + i ) == serviceName )
            return sal_True ;
    }
    return sal_False ;
}

/* XServiceInfo */
Sequence< OUString > SAL_CALL XMLEncryptionTemplateImpl::getSupportedServiceNames() throw( RuntimeException, std::exception ) {
    return impl_getSupportedServiceNames() ;
}

//Helper for XServiceInfo
Sequence< OUString > XMLEncryptionTemplateImpl::impl_getSupportedServiceNames() {
    ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() ) ;
    Sequence<OUString> seqServiceNames { "com.sun.star.xml.crypto.XMLEncryptionTemplate" };
    return seqServiceNames;
}

OUString XMLEncryptionTemplateImpl::impl_getImplementationName() throw( RuntimeException ) {
    return OUString("com.sun.star.xml.security.framework.XMLEncryptionTemplateImpl") ;
}

//Helper for registry
Reference< XInterface > SAL_CALL XMLEncryptionTemplateImpl::impl_createInstance( const Reference< XMultiServiceFactory >& aServiceManager ) throw( RuntimeException ) {
    return Reference< XInterface >( *new XMLEncryptionTemplateImpl( aServiceManager ) ) ;
}

Reference< XSingleServiceFactory > XMLEncryptionTemplateImpl::impl_createFactory( const Reference< XMultiServiceFactory >& aServiceManager ) {
    return ::cppu::createSingleFactory( aServiceManager , impl_getImplementationName() , impl_createInstance , impl_getSupportedServiceNames() ) ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
