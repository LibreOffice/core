/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "securityenvironment_appleimpl.hxx"
#include "x509certificate_appleimpl.hxx"

#include <algorithm>
#include <vector>

#include <com/sun/star/security/CertificateCharacters.hpp>
#include <com/sun/star/security/CertificateValidity.hpp>
#include <comphelper/sequence.hxx>
#include <sal/log.hxx>
#include <svl/applekeychain.hxx>

using namespace css;
using css::security::XCertificate;
using svl::crypto::CFRef;

namespace
{
/// Creates a SecCertificateRef from the DER encoding of an XCertificate.
CFRef<SecCertificateRef> CreateSecCertificate(const uno::Reference<XCertificate>& xCert)
{
    cpo::uno::Sequence<sal_Int8> aDer = xCert->getEncoded();
    if (!aDer.hasElements())
        return CFRef<SecCertificateRef>();
    CFRef<CFDataRef> aData(CFDataCreate(kCFAllocatorDefault,
                                        reinterpret_cast<const UInt8*>(aDer.getConstArray()),
                                        aDer.getLength()));
    if (!aData.is())
        return CFRef<SecCertificateRef>();
    return CFRef<SecCertificateRef>(SecCertificateCreateWithData(kCFAllocatorDefault, aData.get()));
}
}

SecurityEnvironment_AppleImpl::SecurityEnvironment_AppleImpl() {}

SecurityEnvironment_AppleImpl::~SecurityEnvironment_AppleImpl() {}

cpo::uno::Sequence<uno::Reference<XCertificate>>
SecurityEnvironment_AppleImpl::getPersonalCertificates()
{
    std::vector<uno::Reference<XCertificate>> aCertificates;

    // Keychain identities first: when a certificate is present in both stores, prefer the
    // Keychain one, so its (possibly non-exportable) key is used for signing.
    CFRef<CFArrayRef> aIdentities = svl::crypto::CopyAllKeychainIdentities();
    if (aIdentities.is())
    {
        for (CFIndex i = 0; i < CFArrayGetCount(aIdentities.get()); ++i)
        {
            SecIdentityRef pIdentity = static_cast<SecIdentityRef>(
                const_cast<void*>(CFArrayGetValueAtIndex(aIdentities.get(), i)));
            if (!pIdentity)
                continue;
            CFRetain(pIdentity);
            try
            {
                aCertificates.emplace_back(
                    new X509Certificate_AppleImpl(CFRef<SecIdentityRef>(pIdentity)));
            }
            catch (const cpo::uno::Exception&)
            {
                SAL_WARN("xmlsecurity.xmlsec",
                         "getPersonalCertificates: failed to wrap a Keychain identity");
            }
        }
    }

    SAL_INFO("xmlsecurity.xmlsec",
             "getPersonalCertificates: " << aCertificates.size() << " Keychain identities");

    // Then the NSS-database ones that aren't already in the list.
    const cpo::uno::Sequence<uno::Reference<XCertificate>> aNssCertificates
        = SecurityEnvironment_NssImpl::getPersonalCertificates();
    for (const auto& xNssCertificate : aNssCertificates)
    {
        cpo::uno::Sequence<sal_Int8> aDer = xNssCertificate->getEncoded();
        bool bDuplicate = std::any_of(aCertificates.begin(), aCertificates.end(),
                                      [&aDer](const uno::Reference<XCertificate>& xCert) {
                                          return xCert->getEncoded() == aDer;
                                      });
        if (!bDuplicate)
            aCertificates.push_back(xNssCertificate);
    }

    if (aCertificates.empty())
        return {};
    return comphelper::containerToSequence(aCertificates);
}

sal_Int32 SecurityEnvironment_AppleImpl::verifyCertificate(
    const uno::Reference<XCertificate>& xCert,
    const cpo::uno::Sequence<uno::Reference<XCertificate>>& intermediateCerts)
{
    sal_Int32 nValidity = security::CertificateValidity::INVALID;
    try
    {
        nValidity = SecurityEnvironment_NssImpl::verifyCertificate(xCert, intermediateCerts);
    }
    catch (const cpo::uno::Exception&)
    {
        SAL_INFO("xmlsecurity.xmlsec", "verifyCertificate: NSS verification failed");
    }
    if (nValidity == security::CertificateValidity::VALID)
        return nValidity;

    // NSS doesn't trust it; also ask the system trust store.
    CFRef<SecCertificateRef> aLeaf = CreateSecCertificate(xCert);
    if (!aLeaf.is())
        return nValidity;

    CFRef<CFMutableArrayRef> aChain(
        CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks));
    CFArrayAppendValue(aChain.get(), aLeaf.get());
    for (const auto& xIntermediate : intermediateCerts)
    {
        CFRef<SecCertificateRef> aIntermediate = CreateSecCertificate(xIntermediate);
        if (aIntermediate.is())
            CFArrayAppendValue(aChain.get(), aIntermediate.get());
    }

    CFRef<SecPolicyRef> aPolicy(SecPolicyCreateBasicX509());
    SecTrustRef pTrust = nullptr;
    if (SecTrustCreateWithCertificates(aChain.get(), aPolicy.get(), &pTrust) != errSecSuccess
        || !pTrust)
        return nValidity;
    CFRef<SecTrustRef> aTrust(pTrust);

    if (SecTrustEvaluateWithError(aTrust.get(), nullptr))
    {
        SAL_INFO("xmlsecurity.xmlsec", "verifyCertificate: certificate is valid per the system "
                                       "trust store");
        return security::CertificateValidity::VALID;
    }

    return nValidity;
}

sal_Int32
SecurityEnvironment_AppleImpl::getCertificateCharacters(const uno::Reference<XCertificate>& xCert)
{
    sal_Int32 nCharacters = SecurityEnvironment_NssImpl::getCertificateCharacters(xCert);
    if (dynamic_cast<X509Certificate_AppleImpl*>(xCert.get()))
        nCharacters |= security::CertificateCharacters::HAS_PRIVATE_KEY;
    return nCharacters;
}

cpo::uno::Sequence<uno::Reference<XCertificate>>
SecurityEnvironment_AppleImpl::buildCertificatePath(const uno::Reference<XCertificate>& beginCert)
{
    X509Certificate_AppleImpl* pAppleCert
        = dynamic_cast<X509Certificate_AppleImpl*>(beginCert.get());
    m_xAppleSigningCertificate = pAppleCert;
    if (!pAppleCert)
        return SecurityEnvironment_NssImpl::buildCertificatePath(beginCert);

    std::vector<uno::Reference<XCertificate>> aChain;
    CFRef<SecCertificateRef> aLeaf = CreateSecCertificate(beginCert);
    if (aLeaf.is())
    {
        for (const std::vector<unsigned char>& rDer :
             svl::crypto::CopyKeychainCertificateChain(aLeaf.get()))
        {
            cpo::uno::Sequence<sal_Int8> aDer(reinterpret_cast<const sal_Int8*>(rDer.data()),
                                              rDer.size());
            uno::Reference<XCertificate> xCert = createCertificateFromRaw(aDer);
            if (xCert.is())
                aChain.push_back(xCert);
        }
    }
    if (aChain.empty())
        aChain.push_back(beginCert);

    return comphelper::containerToSequence(aChain);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
