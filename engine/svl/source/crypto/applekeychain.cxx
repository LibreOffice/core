/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#ifdef MACOSX

#include <svl/applekeychain.hxx>

#include <algorithm>
#include <cstdlib>
#include <cstring>

#include <sal/log.hxx>

namespace svl::crypto
{
CFRef<CFArrayRef> CopyKeychainSearchList()
{
    const char* pTestKeychain = std::getenv("COKIT_TEST_KEYCHAIN");
    if (!pTestKeychain || !*pTestKeychain)
        return CFRef<CFArrayRef>();

    SAL_WNODEPRECATED_DECLARATIONS_PUSH // SecKeychainOpen (macOS 10.10)
    SecKeychainRef pKeychain
        = nullptr;
    OSStatus nStatus = SecKeychainOpen(pTestKeychain, &pKeychain);
    SAL_WNODEPRECATED_DECLARATIONS_POP
    if (nStatus != errSecSuccess || !pKeychain)
    {
        SAL_WARN("svl.crypto", "CopyKeychainSearchList: can't open test keychain '"
                                   << pTestKeychain << "', status " << nStatus);
        return CFRef<CFArrayRef>();
    }
    CFRef<SecKeychainRef> aKeychain(pKeychain);

    const void* pValues[] = { aKeychain.get() };
    return CFRef<CFArrayRef>(
        CFArrayCreate(kCFAllocatorDefault, pValues, 1, &kCFTypeArrayCallBacks));
}

CFRef<CFArrayRef> CopyAllKeychainIdentities()
{
    // In unit tests, only consider the keychain explicitly provided by the test: touching the
    // user's login keychain could trigger interactive prompts in a headless run.
    if (std::getenv("LO_RUNNING_UNIT_TEST") && !std::getenv("COKIT_TEST_KEYCHAIN"))
    {
        SAL_INFO("svl.crypto", "CopyAllKeychainIdentities: disabled in unit tests");
        return CFRef<CFArrayRef>();
    }

    CFRef<CFMutableDictionaryRef> aQuery(CFDictionaryCreateMutable(
        kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));
    CFDictionarySetValue(aQuery.get(), kSecClass, kSecClassIdentity);
    CFDictionarySetValue(aQuery.get(), kSecMatchLimit, kSecMatchLimitAll);
    CFDictionarySetValue(aQuery.get(), kSecReturnRef, kCFBooleanTrue);

    CFRef<CFArrayRef> aSearchList = CopyKeychainSearchList();
    if (aSearchList.is())
        CFDictionarySetValue(aQuery.get(), kSecMatchSearchList, aSearchList.get());

    CFTypeRef pResult = nullptr;
    OSStatus nStatus = SecItemCopyMatching(aQuery.get(), &pResult);
    if (nStatus == errSecItemNotFound)
    {
        SAL_INFO("svl.crypto", "CopyAllKeychainIdentities: no identities found");
        return CFRef<CFArrayRef>();
    }
    if (nStatus != errSecSuccess || !pResult)
    {
        SAL_WARN("svl.crypto",
                 "CopyAllKeychainIdentities: SecItemCopyMatching failed, status " << nStatus);
        return CFRef<CFArrayRef>();
    }
    if (CFGetTypeID(pResult) != CFArrayGetTypeID())
    {
        CFRelease(pResult);
        return CFRef<CFArrayRef>();
    }

    SAL_INFO("svl.crypto", "CopyAllKeychainIdentities: found "
                               << CFArrayGetCount(static_cast<CFArrayRef>(pResult))
                               << " identities");
    return CFRef<CFArrayRef>(static_cast<CFArrayRef>(pResult));
}

CFRef<SecIdentityRef>
CopyKeychainIdentityForCertificate(const cpo::uno::Sequence<sal_Int8>& rDerCertificate)
{
    if (!rDerCertificate.hasElements())
        return CFRef<SecIdentityRef>();

    CFRef<CFArrayRef> aIdentities = CopyAllKeychainIdentities();
    if (!aIdentities.is())
        return CFRef<SecIdentityRef>();

    for (CFIndex i = 0; i < CFArrayGetCount(aIdentities.get()); ++i)
    {
        SecIdentityRef pIdentity = static_cast<SecIdentityRef>(
            const_cast<void*>(CFArrayGetValueAtIndex(aIdentities.get(), i)));
        if (!pIdentity)
            continue;

        SecCertificateRef pCertificate = nullptr;
        if (SecIdentityCopyCertificate(pIdentity, &pCertificate) != errSecSuccess)
            continue;
        CFRef<SecCertificateRef> aCertificate(pCertificate);

        CFRef<CFDataRef> aData(SecCertificateCopyData(aCertificate.get()));
        if (!aData.is())
            continue;

        if (CFDataGetLength(aData.get()) == rDerCertificate.getLength()
            && std::memcmp(CFDataGetBytePtr(aData.get()), rDerCertificate.getConstArray(),
                           rDerCertificate.getLength())
                   == 0)
        {
            CFRetain(pIdentity);
            return CFRef<SecIdentityRef>(pIdentity);
        }
    }

    return CFRef<SecIdentityRef>();
}

std::vector<std::vector<unsigned char>> CopyKeychainCertificateChain(SecCertificateRef pLeaf)
{
    std::vector<std::vector<unsigned char>> aRet;
    auto AddCertificate = [&aRet](SecCertificateRef pCertificate) {
        if (!pCertificate)
            return;
        CFRef<CFDataRef> aData(SecCertificateCopyData(pCertificate));
        if (!aData.is())
            return;
        std::vector<unsigned char> aDer(CFDataGetBytePtr(aData.get()),
                                        CFDataGetBytePtr(aData.get())
                                            + CFDataGetLength(aData.get()));
        if (std::find(aRet.begin(), aRet.end(), aDer) == aRet.end())
            aRet.push_back(std::move(aDer));
    };

    AddCertificate(pLeaf);

    CFRef<SecPolicyRef> aPolicy(SecPolicyCreateBasicX509());
    SecTrustRef pTrust = nullptr;
    if (SecTrustCreateWithCertificates(pLeaf, aPolicy.get(), &pTrust) != errSecSuccess || !pTrust)
        return aRet;
    CFRef<SecTrustRef> aTrust(pTrust);

    // The evaluation result doesn't matter, it's only run to build the chain.
    (void)SecTrustEvaluateWithError(aTrust.get(), nullptr);

    if (__builtin_available(macOS 12.0, *))
    {
        CFRef<CFArrayRef> aChain(SecTrustCopyCertificateChain(aTrust.get()));
        if (aChain.is())
        {
            for (CFIndex i = 0; i < CFArrayGetCount(aChain.get()); ++i)
                AddCertificate(static_cast<SecCertificateRef>(
                    const_cast<void*>(CFArrayGetValueAtIndex(aChain.get(), i))));
        }
    }
    else
    {
        SAL_WNODEPRECATED_DECLARATIONS_PUSH // SecTrustGetCertificateAtIndex (macOS 12)
            for (CFIndex i = 0; i < SecTrustGetCertificateCount(aTrust.get()); ++i)
                AddCertificate(SecTrustGetCertificateAtIndex(aTrust.get(), i));
        SAL_WNODEPRECATED_DECLARATIONS_POP
    }

    return aRet;
}
}

#endif // MACOSX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
