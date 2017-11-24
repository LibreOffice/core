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
#include <xmlsec-wrapper.h>

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
    m_ctx->setArmor(false);
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

Sequence< Reference < XCertificate > > SecurityEnvironmentGpg::getCertificatesImpl( bool bPrivateOnly )
{
    CertificateImpl* xCert;
    std::list< GpgME::Key > keyList;
    std::list< CertificateImpl* > certsList;

    m_ctx->setKeyListMode(GPGME_KEYLIST_MODE_LOCAL);
    GpgME::Error err = m_ctx->startKeyListing("", bPrivateOnly );
    while (!err) {
        GpgME::Key k = m_ctx->nextKey(err);
        if (err)
            break;
        if (!k.isRevoked() && !k.isExpired() && !k.isDisabled() && !k.isInvalid()) {
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
    int i = 0;
    for (auto const& cert : certsList) {
        xCertificateSequence[i++] = cert;
    }

    return xCertificateSequence;
}

Sequence< Reference < XCertificate > > SecurityEnvironmentGpg::getPersonalCertificates()
{
    return getCertificatesImpl( true );
}

Sequence< Reference < XCertificate > > SecurityEnvironmentGpg::getAllCertificates()
{
    return getCertificatesImpl( false );
}

Reference< XCertificate > SecurityEnvironmentGpg::getCertificate( const OUString& keyId, const Sequence< sal_Int8 >& /*serialNumber*/ )
{
    CertificateImpl* xCert=nullptr;

    //xmlChar* pSignatureValue=xmlNodeGetContent(cur);
    OString ostr = OUStringToOString( keyId , RTL_TEXTENCODING_UTF8 );
    const xmlChar* strKeyId = reinterpret_cast<const xmlChar*>(ostr.getStr());
    if(xmlSecBase64Decode(strKeyId, const_cast<xmlSecByte*>(strKeyId), xmlStrlen(strKeyId)) < 0)
        throw RuntimeException("Base64 decode failed");

    m_ctx->addKeyListMode(GPGME_KEYLIST_MODE_LOCAL);
    GpgME::Error err = m_ctx->startKeyListing("", false);
    while (!err) {
        GpgME::Key k = m_ctx->nextKey(err);
        if (err)
            break;
        if (!k.isInvalid() && strcmp(k.primaryFingerprint(), reinterpret_cast<const char*>(strKeyId)) == 0) {
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

sal_Int32 SecurityEnvironmentGpg::verifyCertificate( const Reference< XCertificate >& aCert,
                                                  const Sequence< Reference< XCertificate > >&  /*intermediateCerts*/ )
{
    const CertificateImpl* xCert = dynamic_cast<CertificateImpl*>(aCert.get());
    if (xCert == nullptr) {
         // Can't find the key locally -> unknown owner
        return security::CertificateValidity::ISSUER_UNKNOWN;
    }

    const GpgME::Key* key = xCert->getCertificate();
    if (key->ownerTrust() == GpgME::Key::OwnerTrust::Marginal ||
        key->ownerTrust() == GpgME::Key::OwnerTrust::Full ||
        key->ownerTrust() == GpgME::Key::OwnerTrust::Ultimate)
    {
        return security::CertificateValidity::VALID;
    }

    return security::CertificateValidity::ISSUER_UNTRUSTED;
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
