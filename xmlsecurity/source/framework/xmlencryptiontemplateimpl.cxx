/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include <rtl/uuid.h>
#include "xmlencryptiontemplateimpl.hxx"

using namespace ::com::sun::star::uno ;
using ::com::sun::star::lang::XMultiServiceFactory ;
using ::com::sun::star::lang::XSingleServiceFactory ;
using ::rtl::OUString ;

using ::com::sun::star::xml::wrapper::XXMLElementWrapper ;
using ::com::sun::star::xml::crypto::XXMLEncryptionTemplate ;

XMLEncryptionTemplateImpl :: XMLEncryptionTemplateImpl( const Reference< XMultiServiceFactory >& aFactory )
    : m_xTemplate( NULL ),
      m_xTarget( NULL ),
      m_xServiceManager( aFactory ),
      m_nStatus ( ::com::sun::star::xml::crypto::SecurityOperationStatus_UNKNOWN ) {
}

XMLEncryptionTemplateImpl :: ~XMLEncryptionTemplateImpl() {
}

/* XXMLEncryptionTemplate */
void SAL_CALL XMLEncryptionTemplateImpl :: setTemplate( const Reference< XXMLElementWrapper >& aTemplate )
    throw (com::sun::star::uno::RuntimeException, com::sun::star::lang::IllegalArgumentException)
{
    m_xTemplate = aTemplate ;
}

/* XXMLEncryptionTemplate */
Reference< XXMLElementWrapper > SAL_CALL XMLEncryptionTemplateImpl :: getTemplate()
throw (com::sun::star::uno::RuntimeException)
{
    return m_xTemplate ;
}

/* XXMLEncryptionTemplate */
void SAL_CALL XMLEncryptionTemplateImpl :: setTarget( const Reference< XXMLElementWrapper >& aTarget )
    throw( com::sun::star::lang::IllegalArgumentException ) {
    m_xTarget = aTarget ;
}

/* XXMLEncryptionTemplate */
Reference< XXMLElementWrapper > SAL_CALL XMLEncryptionTemplateImpl :: getTarget()
throw (com::sun::star::uno::RuntimeException)
{
    return m_xTarget ;
}

void SAL_CALL XMLEncryptionTemplateImpl::setStatus(
    ::com::sun::star::xml::crypto::SecurityOperationStatus status )
    throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    m_nStatus = status;
}

::com::sun::star::xml::crypto::SecurityOperationStatus SAL_CALL XMLEncryptionTemplateImpl::getStatus(  )
    throw (::com::sun::star::uno::RuntimeException)
{
    return m_nStatus;
}

/* XInitialization */
void SAL_CALL XMLEncryptionTemplateImpl :: initialize( const Sequence< Any >& /*aArguments*/ )
    throw( Exception, RuntimeException ) {
    // TBD
} ;

/* XServiceInfo */
OUString SAL_CALL XMLEncryptionTemplateImpl :: getImplementationName() throw( RuntimeException ) {
    return impl_getImplementationName() ;
}

/* XServiceInfo */
sal_Bool SAL_CALL XMLEncryptionTemplateImpl :: supportsService( const OUString& serviceName) throw( RuntimeException ) {
    Sequence< OUString > seqServiceNames = getSupportedServiceNames() ;
    const OUString* pArray = seqServiceNames.getConstArray() ;
    for( sal_Int32 i = 0 ; i < seqServiceNames.getLength() ; i ++ ) {
        if( *( pArray + i ) == serviceName )
            return sal_True ;
    }
    return sal_False ;
}

/* XServiceInfo */
Sequence< OUString > SAL_CALL XMLEncryptionTemplateImpl :: getSupportedServiceNames() throw( RuntimeException ) {
    return impl_getSupportedServiceNames() ;
}

//Helper for XServiceInfo
Sequence< OUString > XMLEncryptionTemplateImpl :: impl_getSupportedServiceNames() {
    ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() ) ;
    Sequence< OUString > seqServiceNames( 1 ) ;
    seqServiceNames.getArray()[0] = OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.crypto.XMLEncryptionTemplate")) ;
    return seqServiceNames ;
}

OUString XMLEncryptionTemplateImpl :: impl_getImplementationName() throw( RuntimeException ) {
    return OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.xml.security.framework.XMLEncryptionTemplateImpl")) ;
}

//Helper for registry
Reference< XInterface > SAL_CALL XMLEncryptionTemplateImpl :: impl_createInstance( const Reference< XMultiServiceFactory >& aServiceManager ) throw( RuntimeException ) {
    return Reference< XInterface >( *new XMLEncryptionTemplateImpl( aServiceManager ) ) ;
}

Reference< XSingleServiceFactory > XMLEncryptionTemplateImpl :: impl_createFactory( const Reference< XMultiServiceFactory >& aServiceManager ) {
    //Reference< XSingleServiceFactory > xFactory ;
    //xFactory = ::cppu::createSingleFactory( aServiceManager , impl_getImplementationName , impl_createInstance , impl_getSupportedServiceNames ) ;
    //return xFactory ;
    return ::cppu::createSingleFactory( aServiceManager , impl_getImplementationName() , impl_createInstance , impl_getSupportedServiceNames() ) ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
