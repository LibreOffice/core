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

#include <com/sun/star/security/CertificateCharacters.hpp>
#include <com/sun/star/security/CertificateValidity.hpp>

#include <vector>
#include <rtl/ref.hxx>

#ifdef _WIN32
#include <config_folders.h>
#include <osl/file.hxx>
#include <osl/process.h>
#include <rtl/bootstrap.hxx>
#include <tools/urlobj.hxx>
#endif

#include <key.h>
#include <keylistresult.h>
#include <libxml/xmlstring.h>
#include <xmlsec/base64.h>
#include <xmlsec/xmlsec.h>

#if defined _MSC_VER && defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundef"
#endif
#include <gpgme.h>
#if defined _MSC_VER && defined __clang__
#pragma clang diagnostic pop
#endif
#include <context.h>

using namespace css;
using namespace css::security;
using namespace css::uno;
using namespace css::lang;

SecurityEnvironmentGpg::SecurityEnvironmentGpg()
{
#ifdef _WIN32
    // On Windows, gpgme expects gpgme-w32spawn.exe to be in the same directory as the current
    // process executable. This assumption might be wrong, e.g., for bundled python, which is
    // in instdir/program/python-core-x.y.z/bin, while gpgme-w32spawn.exe is in instdir/program.
    // If we can't find gpgme-w32spawn.exe in the current executable location, then try to find
    // the spawn executable, and inform gpgme about actual location using gpgme_set_global_flag.
    [[maybe_unused]] static bool bSpawnPathInitialized = [] {
        auto accessUrl = [](const INetURLObject& url) {
            osl::File file(url.GetMainURL(INetURLObject::DecodeMechanism::NONE));
            return file.open(osl_File_OpenFlag_Read) == osl::FileBase::E_None;
        };
        OUString sPath;
        osl_getExecutableFile(&sPath.pData);
        INetURLObject aPathUrl(sPath);
        aPathUrl.setName(u"gpgme-w32spawn.exe");
        if (!accessUrl(aPathUrl))
        {
            sPath = "$BRAND_BASE_DIR/" LIBO_LIBEXEC_FOLDER "/gpgme-w32spawn.exe";
            rtl::Bootstrap::expandMacros(sPath);
            aPathUrl.SetURL(sPath);
            if (accessUrl(aPathUrl))
            {
                aPathUrl.removeSegment();
                GpgME::setGlobalFlag("w32-inst-dir",
                                     aPathUrl.getFSysPath(FSysStyle::Dos).toUtf8().getStr());
            }
        }
        return true;
    }();
#endif
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

OUString SecurityEnvironmentGpg::getSecurityEnvironmentInformation()
{
    return OUString();
}

Sequence< Reference < XCertificate > > SecurityEnvironmentGpg::getCertificatesImpl( bool bPrivateOnly )
{
    std::vector< GpgME::Key > keyList;

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

    Sequence< Reference< XCertificate > > xCertificateSequence(keyList.size());
    auto xCertificateSequenceRange = asNonConstRange(xCertificateSequence);
    int i = 0;
    for (auto const& key : keyList) {
        rtl::Reference<CertificateImpl> xCert = new CertificateImpl();
        xCert->setCertificate(m_ctx.get(),key);
        xCertificateSequenceRange[i++] = xCert;  // fills xCertificateSequence
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
    //xmlChar* pSignatureValue=xmlNodeGetContent(cur);
    OString ostr = OUStringToOString( keyId , RTL_TEXTENCODING_UTF8 );
    const xmlChar* strKeyId = reinterpret_cast<const xmlChar*>(ostr.getStr());
    xmlSecSize nWritten;
    int nRet = xmlSecBase64Decode_ex(strKeyId, const_cast<xmlSecByte*>(strKeyId), xmlStrlen(strKeyId), &nWritten);
    if(nRet < 0)
        throw RuntimeException("Base64 decode failed");

    m_ctx->setKeyListMode(GPGME_KEYLIST_MODE_LOCAL);
    GpgME::Error err = m_ctx->startKeyListing("", false);
    while (!err) {
        GpgME::Key k = m_ctx->nextKey(err);
        if (err)
            break;
        if (!k.isInvalid() && strcmp(k.primaryFingerprint(), reinterpret_cast<const char*>(strKeyId)) == 0) {
            rtl::Reference<CertificateImpl> xCert = new CertificateImpl();
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
    if (dynamic_cast<CertificateImpl*>(aCert.get()) == nullptr)
        throw RuntimeException();

    // we only listed private keys anyway, up in
    // SecurityEnvironmentGpg::getPersonalCertificates
    return CertificateCharacters::HAS_PRIVATE_KEY;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
