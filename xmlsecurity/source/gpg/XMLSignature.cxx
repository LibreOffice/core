/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "XMLSignature.hxx"
#include "SecurityEnvironment.hxx"
#include "XMLSecurityContext.hxx"

#include <cppuhelper/supportsservice.hxx>

using namespace css::uno;
using namespace css::lang;
using namespace css::xml::wrapper;
using namespace css::xml::crypto;

XMLSignature::XMLSignature() {
}

XMLSignature::~XMLSignature() {
}

/* XXMLSignature */
Reference< XXMLSignatureTemplate >
SAL_CALL XMLSignature::generate(
    const Reference< XXMLSignatureTemplate >& aTemplate ,
    const Reference< XSecurityEnvironment >& aEnvironment
) throw( css::xml::crypto::XMLSignatureException,
         css::uno::SecurityException,
         css::uno::RuntimeException, std::exception )
{
}

/* XXMLSignature */
Reference< XXMLSignatureTemplate >
SAL_CALL XMLSignature::validate(
    const Reference< XXMLSignatureTemplate >& aTemplate ,
    const Reference< XXMLSecurityContext >& aSecurityCtx
) throw( css::uno::RuntimeException,
         css::uno::SecurityException,
         css::xml::crypto::XMLSignatureException, std::exception ) {
}

/* XServiceInfo */
OUString SAL_CALL XMLSignature::getImplementationName() throw( RuntimeException, std::exception ) {
    return impl_getImplementationName() ;
}

/* XServiceInfo */
sal_Bool SAL_CALL XMLSignature::supportsService( const OUString& serviceName) throw( RuntimeException, std::exception ) {
    return cppu::supportsService(this, serviceName);
}

/* XServiceInfo */
Sequence< OUString > SAL_CALL XMLSignature::getSupportedServiceNames() throw( RuntimeException, std::exception ) {
    return impl_getSupportedServiceNames() ;
}

//Helper for XServiceInfo
Sequence< OUString > XMLSignature::impl_getSupportedServiceNames() {
    ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() ) ;
    Sequence<OUString> seqServiceNames { "com.sun.star.xml.crypto.XMLSignature" };
    return seqServiceNames ;
}

OUString XMLSignature::impl_getImplementationName() throw( RuntimeException ) {
    return OUString("com.sun.star.xml.security.XMLSignature_Gpg") ;
}

//Helper for registry
Reference< XInterface > SAL_CALL XMLSignature::impl_createInstance( const Reference< XMultiServiceFactory >& ) throw( RuntimeException ) {
    return Reference< XInterface >( *new XMLSignature ) ;
}

Reference< XSingleServiceFactory > XMLSignature::impl_createFactory( const Reference< XMultiServiceFactory >& aServiceManager ) {
    //Reference< XSingleServiceFactory > xFactory ;
    //xFactory = ::cppu::createSingleFactory( aServiceManager , impl_getImplementationName , impl_createInstance , impl_getSupportedServiceNames ) ;
    //return xFactory ;
    return ::cppu::createSingleFactory( aServiceManager , impl_getImplementationName() , impl_createInstance , impl_getSupportedServiceNames() ) ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
