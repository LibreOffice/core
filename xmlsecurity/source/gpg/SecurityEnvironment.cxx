/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "SecurityEnvironment.hxx"

#include <cppuhelper/supportsservice.hxx>
#include <comphelper/servicehelper.hxx>
#include <gpgme.h>

using namespace css;
using namespace css::security;
using namespace css::uno;
using namespace css::lang;

SecurityEnvironment::SecurityEnvironment()
{
}

SecurityEnvironment::~SecurityEnvironment()
{
}

/* XServiceInfo */
OUString SAL_CALL SecurityEnvironment::getImplementationName()
    throw( RuntimeException, std::exception )
{
    return impl_getImplementationName();
}

/* XServiceInfo */
sal_Bool SAL_CALL SecurityEnvironment::supportsService( const OUString& serviceName)
    throw( RuntimeException, std::exception )
{
    return cppu::supportsService(this, serviceName);
}

/* XServiceInfo */
Sequence< OUString > SAL_CALL SecurityEnvironment::getSupportedServiceNames()
    throw( RuntimeException, std::exception )
{
    return impl_getSupportedServiceNames() ;
}

//Helper for XServiceInfo
Sequence< OUString > SecurityEnvironment::impl_getSupportedServiceNames()
{
    return { "com.sun.star.xml.crypto.SecurityEnvironment" } ;
}

OUString SecurityEnvironment::impl_getImplementationName()
    throw( RuntimeException )
{
    return "com.sun.star.xml.security.SecurityEnvironment_Gpg";
}

//Helper for registry
Reference< XInterface > SAL_CALL SecurityEnvironment::impl_createInstance( const Reference< XMultiServiceFactory >& )
    throw( RuntimeException )
{
    return Reference< XInterface >( *new SecurityEnvironment ) ;
}

Reference< XSingleServiceFactory > SecurityEnvironment::impl_createFactory( const Reference< XMultiServiceFactory >& aServiceManager ) {
    return ::cppu::createSingleFactory( aServiceManager , impl_getImplementationName() , impl_createInstance , impl_getSupportedServiceNames() ) ;
}

/* XUnoTunnel */
sal_Int64 SAL_CALL SecurityEnvironment::getSomething( const Sequence< sal_Int8 >& aIdentifier )
    throw( RuntimeException, std::exception )
{
    if( aIdentifier.getLength() == 16 && 0 == memcmp( getUnoTunnelId().getConstArray(), aIdentifier.getConstArray(), 16 ) ) {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_uIntPtr>(this));
    }
    return 0 ;
}

/* XUnoTunnel extension */

namespace
{
    class theSecurityEnvironmentUnoTunnelId  : public rtl::Static< UnoTunnelIdInit, theSecurityEnvironmentUnoTunnelId > {};
}

const Sequence< sal_Int8>& SecurityEnvironment::getUnoTunnelId() {
    return theSecurityEnvironmentUnoTunnelId::get().getSeq();
}

OUString SecurityEnvironment::getSecurityEnvironmentInformation()
    throw( css::uno::RuntimeException, std::exception )
{
    return "";
}

Sequence< Reference < XCertificate > > SecurityEnvironment::getPersonalCertificates()
    throw( SecurityException , RuntimeException, std::exception )
{
    /*gpgme_check_version(0);
    gpgme_ctx_t ctx = 0;
    gpgme_new(&ctx);
    gpgme_set_protocol(ctx, GPGME_PROTOCOL_OpenPGP);
    gpgme_error_t error = gpgme_op_keylist_start(ctx, nullptr, 0);
    gpgme_key_t key;
    while((gpgme_op_keylist_next(ctx, &key) != GPG_ERR_EOF) && key) {
        fprintf(stderr, "key found:\n");
        fprintf(stderr, "issuer   : %s\n", key->issuer_name);
        fprintf(stderr, "serial   : %s\n", key->issuer_serial);
        fprintf(stderr, "chain id : %s\n", key->chain_id);
    }*/
    Sequence<Reference < XCertificate >>* seq = new Sequence< Reference < XCertificate > >(0);
    return *seq;
}

Reference< XCertificate > SecurityEnvironment::getCertificate( const OUString& /*issuerName*/, const Sequence< sal_Int8 >& /*serialNumber*/ )
    throw( SecurityException , RuntimeException, std::exception )
{
    return nullptr;
}

Sequence< Reference < XCertificate > > SecurityEnvironment::buildCertificatePath( const Reference< XCertificate >& /*begin*/ )
    throw( SecurityException , RuntimeException, std::exception ) {
    Sequence<Reference < XCertificate >>* seq = new Sequence< Reference < XCertificate > >(0);
    return *seq;
}

Reference< XCertificate > SecurityEnvironment::createCertificateFromRaw( const Sequence< sal_Int8 >& /*rawCertificate*/ )
    throw( SecurityException , RuntimeException, std::exception )
{
    return nullptr;
}

Reference< XCertificate > SecurityEnvironment::createCertificateFromAscii( const OUString& /*asciiCertificate*/ )
    throw( SecurityException , RuntimeException, std::exception )
{
    return nullptr;
}

sal_Int32 SecurityEnvironment::verifyCertificate( const Reference< XCertificate >& /*aCert*/,
                                                  const Sequence< Reference< XCertificate > >&  /*intermediateCerts*/ )
    throw( css::uno::SecurityException, css::uno::RuntimeException, std::exception )
{
    return 0;
}

sal_Int32 SecurityEnvironment::getCertificateCharacters(
    const css::uno::Reference< css::security::XCertificate >& /*aCert*/ )
    throw( css::uno::SecurityException, css::uno::RuntimeException, std::exception )
{
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
