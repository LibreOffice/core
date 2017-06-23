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

#include <key.h>
#include <keylistresult.h>

using namespace css;
using namespace css::security;
using namespace css::uno;
using namespace css::lang;

SecurityEnvironmentGpg::SecurityEnvironmentGpg()
{
    GpgME::Error err = GpgME::checkEngine(GpgME::OpenPGP);
    if (err)
        throw RuntimeException("The GpgME library failed to initialize for the OpenPGP protocol.");

    m_ctx.reset( GpgME::Context::createForProtocol(GpgME::OpenPGP) );
    if (m_ctx == nullptr)
        throw RuntimeException("The GpgME library failed to initialize for the OpenPGP protocol.");
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
    CertificateImpl* xCert;
    std::list< GpgME::Key > keyList;
    std::list< CertificateImpl* > certsList;

    m_ctx->setKeyListMode(GPGME_KEYLIST_MODE_LOCAL);
    GpgME::Error err = m_ctx->startKeyListing("", true);
    while (!err) {
        GpgME::Key k = m_ctx->nextKey(err);
        if (err)
            break;
        if (!k.isInvalid()) {
            // We can't create CertificateImpl here as CertificateImpl::setCertificate uses GpgME API
            // which interrupts our key listing here. So first get the keys from GpgME, then create the CertificateImpls
            keyList.push_back(k);
        }
    }
    m_ctx->endKeyListing();

    for (auto const& key : keyList) {
        xCert = new CertificateImpl();
        xCert->setCertificate(m_ctx.get(),key);
        certsList.push_back(xCert);
    }

    Sequence< Reference< XCertificate > > xCertificateSequence(certsList.size());
    std::list< CertificateImpl* >::iterator xcertIt;
    int i;
    for (i = 0, xcertIt = certsList.begin(); xcertIt != certsList.end(); ++xcertIt, ++i)
        xCertificateSequence[i] = *xcertIt ;

    return xCertificateSequence;
}

Reference< XCertificate > SecurityEnvironmentGpg::getCertificate( const OUString& issuerName, const Sequence< sal_Int8 >& /*serialNumber*/ )
{
    CertificateImpl* xCert=nullptr;

    m_ctx->setKeyListMode(GPGME_KEYLIST_MODE_LOCAL);
    OString ostr = OUStringToOString( issuerName , RTL_TEXTENCODING_UTF8 );
    GpgME::Error err = m_ctx->startKeyListing(ostr.getStr(), true);
    while (!err) {
        GpgME::Key k = m_ctx->nextKey(err);
        if (err)
            break;
        if (!k.isInvalid()) {
            xCert = new CertificateImpl();
            xCert->setCertificate(m_ctx.get(), k);
            m_ctx->endKeyListing();
            return xCert;
        }
    }
    m_ctx->endKeyListing();

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
