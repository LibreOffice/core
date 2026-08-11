/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xmlsignature_appleimpl.hxx"
#include "securityenvironment_appleimpl.hxx"
#include "x509certificate_appleimpl.hxx"

#include <memory>
#include <string_view>
#include <vector>

#include <comphelper/scopeguard.hxx>
#include <sal/log.hxx>
#include <svl/applekeychain.hxx>

#include <xmlelementwrapper_xmlsecimpl.hxx>
#include <xmlsec/errorcallback.hxx>
#include <xmlsec/xmlstreamio.hxx>

#include <xmlsec/base64.h>
#include <xmlsec/xmldsig.h>
#include <xmlsec/xmlsec.h>
#include <xmlsec/xmltree.h>

using namespace css;
using css::xml::wrapper::XXMLElementWrapper;
using css::xml::crypto::XSecurityEnvironment;
using css::xml::crypto::XXMLSignatureTemplate;
using css::xml::crypto::XUriBinding;
using svl::crypto::CFRef;

namespace std
{
template <> struct default_delete<xmlSecDSigCtx>
{
    void operator()(xmlSecDSigCtxPtr ptr) { xmlSecDSigCtxDestroy(ptr); }
};
}

namespace
{
/// Maps an XML-DSig SignatureMethod algorithm URI to a SecKeyAlgorithm.
bool GetSecKeyAlgorithm(std::u16string_view rUri, SecKeyAlgorithm& rAlgorithm, bool& rIsEcdsa,
                        size_t& rDigestSize)
{
    struct AlgorithmMapping
    {
        std::u16string_view aUri;
        SecKeyAlgorithm eAlgorithm;
        bool bIsEcdsa;
        size_t nDigestSize;
    };
    static const AlgorithmMapping aMappings[] = {
        { u"http://www.w3.org/2000/09/xmldsig#rsa-sha1",
          kSecKeyAlgorithmRSASignatureMessagePKCS1v15SHA1, false, 20 },
        { u"http://www.w3.org/2001/04/xmldsig-more#rsa-sha256",
          kSecKeyAlgorithmRSASignatureMessagePKCS1v15SHA256, false, 32 },
        { u"http://www.w3.org/2001/04/xmldsig-more#rsa-sha384",
          kSecKeyAlgorithmRSASignatureMessagePKCS1v15SHA384, false, 48 },
        { u"http://www.w3.org/2001/04/xmldsig-more#rsa-sha512",
          kSecKeyAlgorithmRSASignatureMessagePKCS1v15SHA512, false, 64 },
        { u"http://www.w3.org/2001/04/xmldsig-more#ecdsa-sha1",
          kSecKeyAlgorithmECDSASignatureMessageX962SHA1, true, 20 },
        { u"http://www.w3.org/2001/04/xmldsig-more#ecdsa-sha256",
          kSecKeyAlgorithmECDSASignatureMessageX962SHA256, true, 32 },
        { u"http://www.w3.org/2001/04/xmldsig-more#ecdsa-sha384",
          kSecKeyAlgorithmECDSASignatureMessageX962SHA384, true, 48 },
        { u"http://www.w3.org/2001/04/xmldsig-more#ecdsa-sha512",
          kSecKeyAlgorithmECDSASignatureMessageX962SHA512, true, 64 },
    };
    for (const auto& rMapping : aMappings)
    {
        if (rUri == rMapping.aUri)
        {
            rAlgorithm = rMapping.eAlgorithm;
            rIsEcdsa = rMapping.bIsEcdsa;
            rDigestSize = rMapping.nDigestSize;
            return true;
        }
    }
    return false;
}

/// Reads one DER TLV; returns false on malformed input.
bool ReadTLV(const unsigned char* pData, size_t nSize, size_t& rPos, unsigned char& rTag,
             size_t& rContentPos, size_t& rContentSize)
{
    if (rPos + 2 > nSize)
        return false;
    rTag = pData[rPos];
    size_t nPos = rPos + 1;
    size_t nLength = pData[nPos++];
    if (nLength & 0x80)
    {
        size_t nLengthBytes = nLength & 0x7f;
        if (nLengthBytes == 0 || nLengthBytes > sizeof(size_t) || nPos + nLengthBytes > nSize)
            return false;
        nLength = 0;
        for (size_t i = 0; i < nLengthBytes; ++i)
            nLength = (nLength << 8) | pData[nPos++];
    }
    if (nPos + nLength > nSize)
        return false;
    rContentPos = nPos;
    rContentSize = nLength;
    rPos = nPos + nLength;
    return true;
}

/** Converts a DER-encoded ECDSA-Sig-Value (SEQUENCE { r INTEGER, s INTEGER }) to the raw,
    fixed-width r||s form required by XML-DSig.
*/
bool ConvertEcdsaDerToRaw(const unsigned char* pDer, size_t nDerSize, size_t nFieldSize,
                          std::vector<unsigned char>& rRaw)
{
    unsigned char nTag;
    size_t nPos = 0, nContentPos = 0, nContentSize = 0;
    if (!ReadTLV(pDer, nDerSize, nPos, nTag, nContentPos, nContentSize) || nTag != 0x30)
        return false;

    const unsigned char* pSequence = pDer + nContentPos;
    size_t nSequenceSize = nContentSize;
    nPos = 0;

    rRaw.assign(2 * nFieldSize, 0);
    for (int i = 0; i < 2; ++i)
    {
        if (!ReadTLV(pSequence, nSequenceSize, nPos, nTag, nContentPos, nContentSize)
            || nTag != 0x02)
            return false;
        const unsigned char* pInteger = pSequence + nContentPos;
        size_t nIntegerSize = nContentSize;
        // Strip leading zero bytes.
        while (nIntegerSize > 0 && *pInteger == 0)
        {
            ++pInteger;
            --nIntegerSize;
        }
        if (nIntegerSize > nFieldSize)
            return false;
        std::copy(pInteger, pInteger + nIntegerSize,
                  rRaw.begin() + (i + 1) * nFieldSize - nIntegerSize);
    }
    return true;
}

/// Finds the first child element with the given xmlsec name / dsig namespace.
xmlNodePtr FindChild(xmlNodePtr pParent, const xmlChar* pName)
{
    for (xmlNodePtr pChild = xmlSecGetNextElementNode(pParent->children); pChild;
         pChild = xmlSecGetNextElementNode(pChild->next))
    {
        if (xmlSecCheckNodeName(pChild, pName, xmlSecDSigNs))
            return pChild;
    }
    return nullptr;
}

/// Processes all Reference elements below pParent, adding them to the given list of pDsigCtx.
bool ProcessReferences(xmlSecDSigCtxPtr pDsigCtx, xmlNodePtr pParent,
                       xmlSecDSigReferenceOrigin eOrigin, xmlSecPtrListPtr pList)
{
    for (xmlNodePtr pChild = xmlSecGetNextElementNode(pParent->children); pChild;
         pChild = xmlSecGetNextElementNode(pChild->next))
    {
        if (!xmlSecCheckNodeName(pChild, xmlSecNodeReference, xmlSecDSigNs))
            continue;

        xmlSecDSigReferenceCtxPtr pDsigRefCtx = xmlSecDSigReferenceCtxCreate(pDsigCtx, eOrigin);
        if (!pDsigRefCtx)
            return false;

        if (xmlSecPtrListAdd(pList, pDsigRefCtx) < 0)
        {
            xmlSecDSigReferenceCtxDestroy(pDsigRefCtx);
            return false;
        }

        if (xmlSecDSigReferenceCtxProcessNode(pDsigRefCtx, pChild) < 0)
            return false;

        if (pDsigRefCtx->status != xmlSecDSigStatusSucceeded)
            return false;
    }
    return true;
}
}

XMLSignature_AppleImpl::XMLSignature_AppleImpl() {}

XMLSignature_AppleImpl::~XMLSignature_AppleImpl() {}

uno::Reference<XXMLSignatureTemplate>
XMLSignature_AppleImpl::generate(const uno::Reference<XXMLSignatureTemplate>& aTemplate,
                                 const uno::Reference<XSecurityEnvironment>& aEnvironment)
{
    if (!aTemplate.is() || !aEnvironment.is())
        throw cpo::uno::RuntimeException();

    SecurityEnvironment_AppleImpl* pSecEnv
        = dynamic_cast<SecurityEnvironment_AppleImpl*>(aEnvironment.get());
    if (!pSecEnv || !pSecEnv->getAppleSigningCertificate().is())
    {
        // Not signing with a Keychain identity: the NSS implementation handles it.
        return XMLSignature_NssImpl::generate(aTemplate, aEnvironment);
    }

    CFRef<SecKeyRef> aPrivateKey = pSecEnv->getAppleSigningCertificate()->copyPrivateKey();
    if (!aPrivateKey.is())
        throw cpo::uno::RuntimeException(u"Failed to access the Keychain private key"_ustr);

    //Get the xml node
    uno::Reference<XXMLElementWrapper> xElement = aTemplate->getTemplate();
    if (!xElement.is())
        throw cpo::uno::RuntimeException();

    XMLElementWrapper_XmlSecImpl* pElement
        = dynamic_cast<XMLElementWrapper_XmlSecImpl*>(xElement.get());
    if (!pElement)
        throw cpo::uno::RuntimeException();

    xmlNodePtr pNode = pElement->getNativeElement();

    //Get the stream/URI binding
    uno::Reference<XUriBinding> xUriBinding = aTemplate->getBinding();
    if (xUriBinding.is())
    {
        //Register the stream input callbacks into libxml2
        if (xmlRegisterStreamInputCallbacks(xUriBinding) < 0)
            throw cpo::uno::RuntimeException();
    }

    setErrorRecorder();
    comphelper::ScopeGuard aGuard([&xUriBinding]() {
        if (xUriBinding.is())
            xmlUnregisterStreamInputCallbacks();
        clearErrorRecorder();
    });

    aTemplate->setStatus(xml::crypto::SecurityOperationStatus_UNKNOWN);

    std::unique_ptr<xmlSecDSigCtx> pDsigCtx(xmlSecDSigCtxCreate(nullptr));
    if (!pDsigCtx)
        return aTemplate;

    // Several asserts inside libxmlsec want the intended operation set for digests/transforms.
    pDsigCtx->operation = xmlSecTransformOperationSign;

    xmlNodePtr pSignedInfo = FindChild(pNode, xmlSecNodeSignedInfo);
    if (!pSignedInfo)
    {
        SAL_WARN("xmlsecurity.xmlsec", "generate: no SignedInfo node");
        return aTemplate;
    }

    // First calculate and write the digests of Object/Manifest references (used by OOXML
    // signatures): the SignedInfo references cover the Objects, so those digests have to be
    // final by the time SignedInfo is processed.
    for (xmlNodePtr pObject = xmlSecGetNextElementNode(pNode->children); pObject;
         pObject = xmlSecGetNextElementNode(pObject->next))
    {
        if (!xmlSecCheckNodeName(pObject, xmlSecNodeObject, xmlSecDSigNs))
            continue;
        for (xmlNodePtr pManifest = xmlSecGetNextElementNode(pObject->children); pManifest;
             pManifest = xmlSecGetNextElementNode(pManifest->next))
        {
            if (!xmlSecCheckNodeName(pManifest, xmlSecNodeManifest, xmlSecDSigNs))
                continue;
            if (!ProcessReferences(pDsigCtx.get(), pManifest, xmlSecDSigReferenceOriginManifest,
                                   &pDsigCtx->manifestReferences))
            {
                SAL_WARN("xmlsecurity.xmlsec", "generate: failed to process a Manifest reference");
                return aTemplate;
            }
        }
    }

    // Then the SignedInfo references.
    if (!ProcessReferences(pDsigCtx.get(), pSignedInfo, xmlSecDSigReferenceOriginSignedInfo,
                           &pDsigCtx->signedInfoReferences))
    {
        SAL_WARN("xmlsecurity.xmlsec", "generate: failed to process a SignedInfo reference");
        return aTemplate;
    }

    // Determine the signature algorithm from the template.
    xmlNodePtr pSignatureMethod = FindChild(pSignedInfo, xmlSecNodeSignatureMethod);
    if (!pSignatureMethod)
    {
        SAL_WARN("xmlsecurity.xmlsec", "generate: no SignatureMethod node");
        return aTemplate;
    }
    xmlChar* pAlgorithm = xmlGetProp(pSignatureMethod, xmlSecAttrAlgorithm);
    if (!pAlgorithm)
    {
        SAL_WARN("xmlsecurity.xmlsec", "generate: no SignatureMethod algorithm");
        return aTemplate;
    }
    OUString aAlgorithmUri = OUString::fromUtf8(reinterpret_cast<const char*>(pAlgorithm));
    xmlFree(pAlgorithm);

    SecKeyAlgorithm eAlgorithm{};
    bool bIsEcdsa{};
    size_t nDigestSize{};
    if (!GetSecKeyAlgorithm(aAlgorithmUri, eAlgorithm, bIsEcdsa, nDigestSize))
    {
        SAL_WARN("xmlsecurity.xmlsec",
                 "generate: unsupported SignatureMethod algorithm " << aAlgorithmUri);
        return aTemplate;
    }
    (void)nDigestSize;

    // Canonicalize SignedInfo, using the CanonicalizationMethod from the template.
    xmlNodePtr pC14NMethod = FindChild(pSignedInfo, xmlSecNodeCanonicalizationMethod);
    if (pC14NMethod
        && !xmlSecTransformCtxNodeRead(&pDsigCtx->transformCtx, pC14NMethod,
                                       xmlSecTransformUsageC14NMethod))
    {
        SAL_WARN("xmlsecurity.xmlsec", "generate: failed to read the CanonicalizationMethod");
        return aTemplate;
    }

    xmlSecNodeSetPtr pNodeSet = xmlSecNodeSetGetChildren(pNode->doc, pSignedInfo, 1, 0);
    if (!pNodeSet)
        return aTemplate;

    int nRet = xmlSecTransformCtxXmlExecute(&pDsigCtx->transformCtx, pNodeSet);
    if (nRet < 0 || !pDsigCtx->transformCtx.result)
    {
        xmlSecNodeSetDestroy(pNodeSet);
        SAL_WARN("xmlsecurity.xmlsec", "generate: failed to canonicalize SignedInfo");
        return aTemplate;
    }
    xmlSecNodeSetDestroy(pNodeSet);

    if (!SecKeyIsAlgorithmSupported(aPrivateKey.get(), kSecKeyOperationTypeSign, eAlgorithm))
    {
        SAL_WARN("xmlsecurity.xmlsec",
                 "generate: algorithm " << aAlgorithmUri << " not supported by the private key");
        return aTemplate;
    }

    // Sign the canonicalized SignedInfo with the Keychain-backed key.
    CFRef<CFDataRef> aToSign(CFDataCreate(kCFAllocatorDefault,
                                          xmlSecBufferGetData(pDsigCtx->transformCtx.result),
                                          xmlSecBufferGetSize(pDsigCtx->transformCtx.result)));
    CFErrorRef pError = nullptr;
    CFRef<CFDataRef> aSignature(
        SecKeyCreateSignature(aPrivateKey.get(), eAlgorithm, aToSign.get(), &pError));
    if (pError)
        CFRelease(pError);
    if (!aSignature.is())
    {
        SAL_WARN("xmlsecurity.xmlsec", "generate: SecKeyCreateSignature failed");
        return aTemplate;
    }

    const unsigned char* pSignatureBytes = CFDataGetBytePtr(aSignature.get());
    size_t nSignatureSize = CFDataGetLength(aSignature.get());
    std::vector<unsigned char> aRawSignature;
    if (bIsEcdsa)
    {
        // XML-DSig wants the raw r||s form, the Security framework produces DER.
        int nKeyBits = 0;
        CFRef<CFDictionaryRef> aAttributes(SecKeyCopyAttributes(aPrivateKey.get()));
        if (aAttributes.is())
        {
            CFNumberRef pBits = static_cast<CFNumberRef>(
                CFDictionaryGetValue(aAttributes.get(), kSecAttrKeySizeInBits));
            if (pBits)
                CFNumberGetValue(pBits, kCFNumberIntType, &nKeyBits);
        }
        if (nKeyBits <= 0
            || !ConvertEcdsaDerToRaw(pSignatureBytes, nSignatureSize, (nKeyBits + 7) / 8,
                                     aRawSignature))
        {
            SAL_WARN("xmlsecurity.xmlsec", "generate: failed to convert the ECDSA signature");
            return aTemplate;
        }
        pSignatureBytes = aRawSignature.data();
        nSignatureSize = aRawSignature.size();
    }

    xmlChar* pEncoded = xmlSecBase64Encode(pSignatureBytes, nSignatureSize, 64);
    if (!pEncoded)
        return aTemplate;

    xmlNodePtr pSignatureValue = FindChild(pNode, xmlSecNodeSignatureValue);
    if (!pSignatureValue)
    {
        xmlFree(pEncoded);
        SAL_WARN("xmlsecurity.xmlsec", "generate: no SignatureValue node");
        return aTemplate;
    }
    xmlNodeSetContentLen(pSignatureValue, pEncoded, xmlStrlen(pEncoded));
    xmlFree(pEncoded);

    aTemplate->setStatus(xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED);
    return aTemplate;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
