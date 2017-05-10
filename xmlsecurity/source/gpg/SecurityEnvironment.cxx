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

/* XUnoTunnel */
sal_Int64 SAL_CALL SecurityEnvironmentGpg::getSomething( const Sequence< sal_Int8 >& aIdentifier )
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
{
    return OUString();
}

Sequence< Reference < XCertificate > > SecurityEnvironmentGpg::getPersonalCertificates()
{
    GpgME::initializeLibrary();
    GpgME::Error err = GpgME::checkEngine(GpgME::OpenPGP);
    if (err)
        throw RuntimeException("The GpgME library failed to initialize for the OpenPGP protocol.");

    GpgME::Context* ctx = GpgME::Context::createForProtocol(GpgME::OpenPGP);
    if (ctx == nullptr)
        throw RuntimeException("The GpgME library failed to initialize for the OpenPGP protocol.");

    CertificateImpl* xCert;
    std::list< CertificateImpl* > certsList;

    ctx->setKeyListMode(GPGME_KEYLIST_MODE_LOCAL);
    err = ctx->startKeyListing("", true);
    while (!err) {
        GpgME::Key k = ctx->nextKey(err);
        if (err)
            break;
        if (!k.isInvalid()) {
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
{
    return nullptr;
}

Sequence< Reference < XCertificate > > SecurityEnvironmentGpg::buildCertificatePath( const Reference< XCertificate >& /*begin*/ )
{
    return Sequence< Reference < XCertificate > >();
}

Reference< XCertificate > SecurityEnvironmentGpg::createCertificateFromRaw( const Sequence< sal_Int8 >& /*rawCertificate*/ )
{
    return nullptr;
}

Reference< XCertificate > SecurityEnvironmentGpg::createCertificateFromAscii( const OUString& /*asciiCertificate*/ )
{
    return nullptr;
}

sal_Int32 SecurityEnvironmentGpg::verifyCertificate( const Reference< XCertificate >& /*aCert*/,
                                                  const Sequence< Reference< XCertificate > >&  /*intermediateCerts*/ )
{
    return 0;
}

sal_Int32 SecurityEnvironmentGpg::getCertificateCharacters(
    const Reference< XCertificate >& aCert)
{
    const CertificateImpl* xCert;
    Reference< XUnoTunnel > xCertTunnel(aCert, UNO_QUERY_THROW) ;
    xCert = reinterpret_cast<CertificateImpl*>(sal::static_int_cast<sal_uIntPtr>(xCertTunnel->getSomething(CertificateImpl::getUnoTunnelId()))) ;
    if (xCert == nullptr)
        throw RuntimeException();

    // we only listed private keys anyway, up in
    // SecurityEnvironmentGpg::getPersonalCertificates
    return CertificateCharacters::HAS_PRIVATE_KEY;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
