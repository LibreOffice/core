/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "SecurityEnvironment.hxx"
#include "CertificateImpl.hxx"

#include <cppuhelper/supportsservice.hxx>
#include <comphelper/servicehelper.hxx>
#include <list>

#include <gpgme.h>
#include <context.h>
#include <key.h>
#include <keylistresult.h>

using namespace css;
using namespace css::security;
using namespace css::uno;
using namespace css::lang;

SecurityEnvironmentGpg::SecurityEnvironmentGpg()
{
}

SecurityEnvironmentGpg::~SecurityEnvironmentGpg()
{
}

/* XServiceInfo */
OUString SAL_CALL SecurityEnvironmentGpg::getImplementationName()
    throw( RuntimeException, std::exception )
{
    return impl_getImplementationName();
}

/* XServiceInfo */
sal_Bool SAL_CALL SecurityEnvironmentGpg::supportsService( const OUString& serviceName)
    throw( RuntimeException, std::exception )
{
    return cppu::supportsService(this, serviceName);
}

/* XServiceInfo */
Sequence< OUString > SAL_CALL SecurityEnvironmentGpg::getSupportedServiceNames()
    throw( RuntimeException, std::exception )
{
    return impl_getSupportedServiceNames() ;
}

//Helper for XServiceInfo
Sequence< OUString > SecurityEnvironmentGpg::impl_getSupportedServiceNames()
{
    return { "com.sun.star.xml.crypto.gpg.GpgSecurityEnvironment" } ;
}

OUString SecurityEnvironmentGpg::impl_getImplementationName()
    throw( RuntimeException )
{
    return OUString("com.sun.star.xml.security.SecurityEnvironment_Gpg");
}

//Helper for registry
Reference< XInterface > SAL_CALL SecurityEnvironmentGpg::impl_createInstance( const Reference< XMultiServiceFactory >& )
    throw( RuntimeException )
{
    return Reference< XInterface >( *new SecurityEnvironmentGpg ) ;
}

Reference< XSingleServiceFactory > SecurityEnvironmentGpg::impl_createFactory( const Reference< XMultiServiceFactory >& aServiceManager ) {
    return ::cppu::createSingleFactory( aServiceManager , impl_getImplementationName() , impl_createInstance , impl_getSupportedServiceNames() ) ;
}

/* XUnoTunnel */
sal_Int64 SAL_CALL SecurityEnvironmentGpg::getSomething( const Sequence< sal_Int8 >& aIdentifier )
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

const Sequence< sal_Int8>& SecurityEnvironmentGpg::getUnoTunnelId() {
    return theSecurityEnvironmentUnoTunnelId::get().getSeq();
}

OUString SecurityEnvironmentGpg::getSecurityEnvironmentInformation()
    throw( RuntimeException, std::exception )
{
    return OUString("");
}

Sequence< Reference < XCertificate > > SecurityEnvironmentGpg::getPersonalCertificates()
    throw( SecurityException , RuntimeException, std::exception )
{
    GpgME::initializeLibrary();
    GpgME::Error err = GpgME::checkEngine(GpgME::OpenPGP);
    if (err)
        throw RuntimeException("The GpgME library failed to initialize for the OpenPGP protocol.");

    std::shared_ptr<GpgME::Context> ctx(GpgME::Context::createForProtocol(GpgME::OpenPGP));
    if (ctx == nullptr)
        throw RuntimeException("The GpgME library failed to initialize for the OpenPGP protocol.");

    CertificateImpl* xCert;
    std::list< CertificateImpl* > certsList;

    ctx->setKeyListMode(GPGME_KEYLIST_MODE_LOCAL);
    err = ctx->startKeyListing();
    while (!err) {
        GpgME::Key k = ctx->nextKey(err);
        if (err)
            break;
        if (!k.isInvalid() && k.canEncrypt() && (k.ownerTrust() == GpgME::Key::Ultimate)) {
            xCert = new CertificateImpl();
            xCert->setCertificate(k);
            certsList.push_back(xCert);
        }
    }
    ctx->endKeyListing();

    Sequence< Reference< XCertificate > > xCertificateSequence(certsList.size());
    std::list< CertificateImpl* >::iterator xcertIt;
    int i;
    for (i = 0, xcertIt = certsList.begin(); xcertIt != certsList.end(); ++xcertIt, ++i)
        xCertificateSequence[i] = *xcertIt ;

    return xCertificateSequence;
}

Reference< XCertificate > SecurityEnvironmentGpg::getCertificate( const OUString& /*issuerName*/, const Sequence< sal_Int8 >& /*serialNumber*/ )
    throw( SecurityException , RuntimeException, std::exception )
{
    return nullptr;
}

Sequence< Reference < XCertificate > > SecurityEnvironmentGpg::buildCertificatePath( const Reference< XCertificate >& /*begin*/ )
    throw( SecurityException , RuntimeException, std::exception )
{
    return Sequence< Reference < XCertificate > >();
}

Reference< XCertificate > SecurityEnvironmentGpg::createCertificateFromRaw( const Sequence< sal_Int8 >& /*rawCertificate*/ )
    throw( SecurityException , RuntimeException, std::exception )
{
    return nullptr;
}

Reference< XCertificate > SecurityEnvironmentGpg::createCertificateFromAscii( const OUString& /*asciiCertificate*/ )
    throw( SecurityException , RuntimeException, std::exception )
{
    return nullptr;
}

sal_Int32 SecurityEnvironmentGpg::verifyCertificate( const Reference< XCertificate >& /*aCert*/,
                                                  const Sequence< Reference< XCertificate > >&  /*intermediateCerts*/ )
    throw( SecurityException, RuntimeException, std::exception )
{
    return 0;
}

sal_Int32 SecurityEnvironmentGpg::getCertificateCharacters(
    const Reference< XCertificate >& /*aCert*/)
    throw( SecurityException, RuntimeException, std::exception )
{
//     const CertificateImpl* xCert;
//     const GpgME::Key* key;
//
//     Reference< XUnoTunnel > xCertTunnel(aCert, UNO_QUERY_THROW) ;
//     xCert = reinterpret_cast<CertificateImpl*>(sal::static_int_cast<sal_uIntPtr>(xCertTunnel->getSomething(CertificateImpl::getUnoTunnelId()))) ;
//     if (xCert == nullptr)
//         throw RuntimeException();
//
//     key = xCert->getCertificate();
//
    sal_Int32 characters = 0x0;

    // TODO Why this ain't workin?
    //if (key->hasSecret())
        characters |= CertificateCharacters::HAS_PRIVATE_KEY;

    return characters;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
