/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <xmlsecurity/pdfio/pdfdocument.hxx>

#include <map>
#include <memory>
#include <vector>

#include <com/sun/star/uno/Sequence.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/string.hxx>
#include <filter/msfilter/mscodec.hxx>
#include <rtl/character.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/string.hxx>
#include <sal/log.hxx>
#include <sal/types.h>
#include <sax/tools/converter.hxx>
#include <tools/zcodec.hxx>
#include <unotools/calendarwrapper.hxx>
#include <unotools/datetime.hxx>
#include <vcl/pdfwriter.hxx>
#include <xmloff/xmluconv.hxx>
#include <o3tl/make_unique.hxx>

#include <sigstruct.hxx>

#ifdef XMLSEC_CRYPTO_NSS
#include <cert.h>
#include <cms.h>
#include <nss.h>
#include <secerr.h>
#include <sechash.h>
#endif

#ifdef XMLSEC_CRYPTO_MSCRYPTO
#include <prewin.h>
#include <wincrypt.h>
#include <postwin.h>
#include <comphelper/windowserrorstring.hxx>
#endif

using namespace com::sun::star;

namespace
{
/// Turns an array of floats into offset + length pairs.
bool GetByteRangesFromPDF(vcl::filter::PDFArrayElement& rArray,
                          std::vector<std::pair<size_t, size_t>>& rByteRanges)
{
    size_t nByteRangeOffset = 0;
    const std::vector<vcl::filter::PDFElement*>& rByteRangeElements = rArray.GetElements();
    for (size_t i = 0; i < rByteRangeElements.size(); ++i)
    {
        auto pNumber = dynamic_cast<vcl::filter::PDFNumberElement*>(rByteRangeElements[i]);
        if (!pNumber)
        {
            SAL_WARN("xmlsecurity.pdfio",
                     "ValidateSignature: signature offset and length has to be a number");
            return false;
        }

        if (i % 2 == 0)
        {
            nByteRangeOffset = pNumber->GetValue();
            continue;
        }
        size_t nByteRangeLength = pNumber->GetValue();
        rByteRanges.emplace_back(nByteRangeOffset, nByteRangeLength);
    }

    return true;
}

/// Determines the last position that is covered by a signature.
bool GetEOFOfSignature(vcl::filter::PDFObjectElement* pSignature, size_t& rEOF)
{
    vcl::filter::PDFObjectElement* pValue = pSignature->LookupObject("V");
    if (!pValue)
    {
        return false;
    }

    auto pByteRange = dynamic_cast<vcl::filter::PDFArrayElement*>(pValue->Lookup("ByteRange"));
    if (!pByteRange || pByteRange->GetElements().size() < 2)
    {
        return false;
    }

    std::vector<std::pair<size_t, size_t>> aByteRanges;
    if (!GetByteRangesFromPDF(*pByteRange, aByteRanges))
    {
        return false;
    }

    rEOF = aByteRanges[1].first + aByteRanges[1].second;
    return true;
}

/// Checks if there are unsigned incremental updates between the signatures or after the last one.
bool IsCompleteSignature(SvStream& rStream, vcl::filter::PDFDocument& rDocument,
                         vcl::filter::PDFObjectElement* pSignature)
{
    std::set<size_t> aSignedEOFs;
    for (const auto& i : rDocument.GetSignatureWidgets())
    {
        size_t nEOF = 0;
        if (!GetEOFOfSignature(i, nEOF))
        {
            return false;
        }

        aSignedEOFs.insert(nEOF);
    }

    size_t nSignatureEOF = 0;
    if (!GetEOFOfSignature(pSignature, nSignatureEOF))
    {
        return false;
    }

    const std::vector<size_t>& rAllEOFs = rDocument.GetEOFs();
    bool bFoundOwn = false;
    for (const auto& rEOF : rAllEOFs)
    {
        if (rEOF == nSignatureEOF)
        {
            bFoundOwn = true;
            continue;
        }

        if (!bFoundOwn)
        {
            continue;
        }

        if (aSignedEOFs.find(rEOF) == aSignedEOFs.end())
        {
            // Unsigned incremental update found.
            return false;
        }
    }

    // Make sure we find the incremental update of the signature itself.
    if (!bFoundOwn)
    {
        return false;
    }

    // No additional content after the last incremental update.
    rStream.Seek(STREAM_SEEK_TO_END);
    size_t nFileEnd = rStream.Tell();
    return std::find(rAllEOFs.begin(), rAllEOFs.end(), nFileEnd) != rAllEOFs.end();
}
}

namespace xmlsecurity
{
namespace pdfio
{

namespace
{
#ifdef XMLSEC_CRYPTO_NSS
/// Similar to NSS_CMSAttributeArray_FindAttrByOidTag(), but works directly with a SECOidData.
NSSCMSAttribute* CMSAttributeArray_FindAttrByOidData(NSSCMSAttribute** attrs, SECOidData* oid, PRBool only)
{
    NSSCMSAttribute* attr1, *attr2;

    if (attrs == nullptr)
        return nullptr;

    if (oid == nullptr)
        return nullptr;

    while ((attr1 = *attrs++) != nullptr)
    {
        if (attr1->type.len == oid->oid.len && PORT_Memcmp(attr1->type.data,
                oid->oid.data,
                oid->oid.len) == 0)
            break;
    }

    if (attr1 == nullptr)
        return nullptr;

    if (!only)
        return attr1;

    while ((attr2 = *attrs++) != nullptr)
    {
        if (attr2->type.len == oid->oid.len && PORT_Memcmp(attr2->type.data,
                oid->oid.data,
                oid->oid.len) == 0)
            break;
    }

    if (attr2 != nullptr)
        return nullptr;

    return attr1;
}

/// Same as SEC_StringToOID(), which is private to us.
SECStatus StringToOID(SECItem* to, const char* from, PRUint32 len)
{
    PRUint32 decimal_numbers = 0;
    PRUint32 result_bytes = 0;
    SECStatus rv;
    PRUint8 result[1024];

    static const PRUint32 max_decimal = (0xffffffff / 10);
    static const char OIDstring[] = {"OID."};

    if (!from || !to)
    {
        PORT_SetError(SEC_ERROR_INVALID_ARGS);
        return SECFailure;
    }
    if (!len)
    {
        len = PL_strlen(from);
    }
    if (len >= 4 && !PL_strncasecmp(from, OIDstring, 4))
    {
        from += 4; /* skip leading "OID." if present */
        len  -= 4;
    }
    if (!len)
    {
bad_data:
        PORT_SetError(SEC_ERROR_BAD_DATA);
        return SECFailure;
    }
    do
    {
        PRUint32 decimal = 0;
        while (len > 0 && rtl::isAsciiDigit(static_cast<unsigned char>(*from)))
        {
            PRUint32 addend = (*from++ - '0');
            --len;
            if (decimal > max_decimal)  /* overflow */
                goto bad_data;
            decimal = (decimal * 10) + addend;
            if (decimal < addend)   /* overflow */
                goto bad_data;
        }
        if (len != 0 && *from != '.')
        {
            goto bad_data;
        }
        if (decimal_numbers == 0)
        {
            if (decimal > 2)
                goto bad_data;
            result[0] = decimal * 40;
            result_bytes = 1;
        }
        else if (decimal_numbers == 1)
        {
            if (decimal > 40)
                goto bad_data;
            result[0] += decimal;
        }
        else
        {
            /* encode the decimal number,  */
            PRUint8* rp;
            PRUint32 num_bytes = 0;
            PRUint32 tmp = decimal;
            while (tmp)
            {
                num_bytes++;
                tmp >>= 7;
            }
            if (!num_bytes)
                ++num_bytes;  /* use one byte for a zero value */
            if (static_cast<size_t>(num_bytes) + result_bytes > sizeof result)
                goto bad_data;
            tmp = num_bytes;
            rp = result + result_bytes - 1;
            rp[tmp] = (PRUint8)(decimal & 0x7f);
            decimal >>= 7;
            while (--tmp > 0)
            {
                rp[tmp] = (PRUint8)(decimal | 0x80);
                decimal >>= 7;
            }
            result_bytes += num_bytes;
        }
        ++decimal_numbers;
        if (len > 0)   /* skip trailing '.' */
        {
            ++from;
            --len;
        }
    }
    while (len > 0);
    /* now result contains result_bytes of data */
    if (to->data && to->len >= result_bytes)
    {
        PORT_Memcpy(to->data, result, to->len = result_bytes);
        rv = SECSuccess;
    }
    else
    {
        SECItem result_item = {siBuffer, nullptr, 0 };
        result_item.data = result;
        result_item.len  = result_bytes;
        rv = SECITEM_CopyItem(nullptr, to, &result_item);
    }
    return rv;
}
#elif defined XMLSEC_CRYPTO_MSCRYPTO
/// Verifies a non-detached signature using CryptoAPI.
bool VerifyNonDetachedSignature(SvStream& rStream, std::vector<std::pair<size_t, size_t>>& rByteRanges, std::vector<BYTE>& rExpectedHash)
{
    HCRYPTPROV hProv = 0;
    if (!CryptAcquireContext(&hProv, nullptr, nullptr, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
    {
        SAL_WARN("xmlsecurity.pdfio", "CryptAcquireContext() failed");
        return false;
    }

    HCRYPTHASH hHash = 0;
    if (!CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash))
    {
        SAL_WARN("xmlsecurity.pdfio", "CryptCreateHash() failed");
        return false;
    }

    for (const auto& rByteRange : rByteRanges)
    {
        rStream.Seek(rByteRange.first);
        const int nChunkLen = 4096;
        std::vector<unsigned char> aBuffer(nChunkLen);
        for (size_t nByte = 0; nByte < rByteRange.second;)
        {
            size_t nRemainingSize = rByteRange.second - nByte;
            if (nRemainingSize < nChunkLen)
            {
                rStream.ReadBytes(aBuffer.data(), nRemainingSize);
                if (!CryptHashData(hHash, aBuffer.data(), nRemainingSize, 0))
                {
                    SAL_WARN("xmlsecurity.pdfio", "CryptHashData() failed");
                    return false;
                }
                nByte = rByteRange.second;
            }
            else
            {
                rStream.ReadBytes(aBuffer.data(), nChunkLen);
                if (!CryptHashData(hHash, aBuffer.data(), nChunkLen, 0))
                {
                    SAL_WARN("xmlsecurity.pdfio", "CryptHashData() failed");
                    return false;
                }
                nByte += nChunkLen;
            }
        }
    }

    DWORD nActualHash = 0;
    if (!CryptGetHashParam(hHash, HP_HASHVAL, nullptr, &nActualHash, 0))
    {
        SAL_WARN("xmlsecurity.pdfio", "CryptGetHashParam() failed to provide the hash length");
        return false;
    }

    std::vector<unsigned char> aActualHash(nActualHash);
    if (!CryptGetHashParam(hHash, HP_HASHVAL, aActualHash.data(), &nActualHash, 0))
    {
        SAL_WARN("xmlsecurity.pdfio", "CryptGetHashParam() failed to provide the hash");
        return false;
    }

    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);

    if (!std::memcmp(aActualHash.data(), rExpectedHash.data(), aActualHash.size()) && aActualHash.size() == rExpectedHash.size())
        return true;

    return false;
}
#endif
}

bool ValidateSignature(SvStream& rStream, vcl::filter::PDFObjectElement* pSignature,
                       SignatureInformation& rInformation, vcl::filter::PDFDocument& rDocument)
{
    vcl::filter::PDFObjectElement* pValue = pSignature->LookupObject("V");
    if (!pValue)
    {
        SAL_WARN("xmlsecurity.pdfio", "ValidateSignature: no value");
        return false;
    }

    auto pContents = dynamic_cast<vcl::filter::PDFHexStringElement*>(pValue->Lookup("Contents"));
    if (!pContents)
    {
        SAL_WARN("xmlsecurity.pdfio", "ValidateSignature: no contents");
        return false;
    }

    auto pByteRange = dynamic_cast<vcl::filter::PDFArrayElement*>(pValue->Lookup("ByteRange"));
    if (!pByteRange || pByteRange->GetElements().size() < 2)
    {
        SAL_WARN("xmlsecurity.pdfio", "ValidateSignature: no byte range or too few elements");
        return false;
    }

    auto pSubFilter = dynamic_cast<vcl::filter::PDFNameElement*>(pValue->Lookup("SubFilter"));
    bool bNonDetached = pSubFilter && pSubFilter->GetValue() == "adbe.pkcs7.sha1";
    if (!pSubFilter || (pSubFilter->GetValue() != "adbe.pkcs7.detached" && !bNonDetached && pSubFilter->GetValue() != "ETSI.CAdES.detached"))
    {
        if (!pSubFilter)
            SAL_WARN("xmlsecurity.pdfio", "ValidateSignature: missing sub-filter");
        else
            SAL_WARN("xmlsecurity.pdfio", "ValidateSignature: unsupported sub-filter: '"<<pSubFilter->GetValue()<<"'");
        return false;
    }

    // Reason / comment / description is optional.
    auto pReason = dynamic_cast<vcl::filter::PDFHexStringElement*>(pValue->Lookup("Reason"));
    if (pReason)
    {
        // See appendUnicodeTextString() for the export equivalent of this.
        std::vector<unsigned char> aReason = vcl::filter::PDFDocument::DecodeHexString(pReason);
        OUStringBuffer aBuffer;
        sal_uInt16 nByte = 0;
        for (size_t i = 0; i < aReason.size(); ++i)
        {
            if (i % 2 == 0)
                nByte = aReason[i];
            else
            {
                sal_Unicode nUnicode;
                nUnicode = (nByte << 8);
                nUnicode |= aReason[i];
                aBuffer.append(nUnicode);
            }
        }

        if (!aBuffer.isEmpty())
            rInformation.ouDescription = aBuffer.makeStringAndClear();
    }

    // Date: used only when the time of signing is not available in the
    // signature.
    auto pM = dynamic_cast<vcl::filter::PDFLiteralStringElement*>(pValue->Lookup("M"));
    if (pM)
    {
        // Example: "D:20161027100104".
        const OString& rM = pM->GetValue();
        if (rM.startsWith("D:") && rM.getLength() >= 16)
        {
            rInformation.stDateTime.Year = rM.copy(2, 4).toInt32();
            rInformation.stDateTime.Month = rM.copy(6, 2).toInt32();
            rInformation.stDateTime.Day = rM.copy(8, 2).toInt32();
            rInformation.stDateTime.Hours = rM.copy(10, 2).toInt32();
            rInformation.stDateTime.Minutes = rM.copy(12, 2).toInt32();
            rInformation.stDateTime.Seconds = rM.copy(14, 2).toInt32();
        }
    }

    // Build a list of offset-length pairs, representing the signed bytes.
    std::vector<std::pair<size_t, size_t>> aByteRanges;
    if (!GetByteRangesFromPDF(*pByteRange, aByteRanges))
    {
        return false;
    }

    // Detect if the byte ranges don't cover everything, but the signature itself.
    if (aByteRanges.size() < 2)
    {
        SAL_WARN("xmlsecurity.pdfio", "ValidateSignature: expected 2 byte ranges");
        return false;
    }
    if (aByteRanges[0].first != 0)
    {
        SAL_WARN("xmlsecurity.pdfio", "ValidateSignature: first range start is not 0");
        return false;
    }
    // 2 is the leading "<" and the trailing ">" around the hex string.
    size_t nSignatureLength = static_cast<size_t>(pContents->GetValue().getLength()) + 2;
    if (aByteRanges[1].first != (aByteRanges[0].second + nSignatureLength))
    {
        SAL_WARN("xmlsecurity.pdfio", "ValidateSignature: second range start is not the end of the signature");
        return false;
    }
    rInformation.bPartialDocumentSignature = !IsCompleteSignature(rStream, rDocument, pSignature);

    // At this point there is no obviously missing info to validate the
    // signature.
    std::vector<unsigned char> aSignature = vcl::filter::PDFDocument::DecodeHexString(pContents);
    if (aSignature.empty())
    {
        SAL_WARN("xmlsecurity.pdfio", "ValidateSignature: empty contents");
        return false;
    }

#ifdef XMLSEC_CRYPTO_NSS
    // Validate the signature. No need to call NSS_Init() here, assume that the
    // caller did that already.

    SECItem aSignatureItem;
    aSignatureItem.data = aSignature.data();
    aSignatureItem.len = aSignature.size();
    NSSCMSMessage* pCMSMessage = NSS_CMSMessage_CreateFromDER(&aSignatureItem,
                                 /*cb=*/nullptr,
                                 /*cb_arg=*/nullptr,
                                 /*pwfn=*/nullptr,
                                 /*pwfn_arg=*/nullptr,
                                 /*decrypt_key_cb=*/nullptr,
                                 /*decrypt_key_cb_arg=*/nullptr);
    if (!NSS_CMSMessage_IsSigned(pCMSMessage))
    {
        SAL_WARN("xmlsecurity.pdfio", "ValidateSignature: message is not signed");
        return false;
    }

    NSSCMSContentInfo* pCMSContentInfo = NSS_CMSMessage_ContentLevel(pCMSMessage, 0);
    if (!pCMSContentInfo)
    {
        SAL_WARN("xmlsecurity.pdfio", "ValidateSignature: NSS_CMSMessage_ContentLevel() failed");
        return false;
    }

    auto pCMSSignedData = static_cast<NSSCMSSignedData*>(NSS_CMSContentInfo_GetContent(pCMSContentInfo));
    if (!pCMSSignedData)
    {
        SAL_WARN("xmlsecurity.pdfio", "ValidateSignature: NSS_CMSContentInfo_GetContent() failed");
        return false;
    }

    // Import certificates from the signed data temporarily, so it'll be
    // possible to verify the signature, even if we didn't have the certificate
    // previously.
    std::vector<CERTCertificate*> aDocumentCertificates;
    for (size_t i = 0; pCMSSignedData->rawCerts[i]; ++i)
        aDocumentCertificates.push_back(CERT_NewTempCertificate(CERT_GetDefaultCertDB(), pCMSSignedData->rawCerts[i], nullptr, 0, 0));

    NSSCMSSignerInfo* pCMSSignerInfo = NSS_CMSSignedData_GetSignerInfo(pCMSSignedData, 0);
    if (!pCMSSignerInfo)
    {
        SAL_WARN("xmlsecurity.pdfio", "ValidateSignature: NSS_CMSSignedData_GetSignerInfo() failed");
        return false;
    }

    SECItem aAlgorithm = NSS_CMSSignedData_GetDigestAlgs(pCMSSignedData)[0]->algorithm;
    SECOidTag eOidTag = SECOID_FindOIDTag(&aAlgorithm);

    // Map a sign algorithm to a digest algorithm.
    // See NSS_CMSUtil_MapSignAlgs(), which is private to us.
    switch (eOidTag)
    {
    case SEC_OID_PKCS1_SHA1_WITH_RSA_ENCRYPTION:
        eOidTag = SEC_OID_SHA1;
        break;
    case SEC_OID_PKCS1_SHA256_WITH_RSA_ENCRYPTION:
        eOidTag = SEC_OID_SHA256;
        break;
    case SEC_OID_PKCS1_SHA512_WITH_RSA_ENCRYPTION:
        eOidTag = SEC_OID_SHA512;
        break;
    default:
        break;
    }

    HASH_HashType eHashType = HASH_GetHashTypeByOidTag(eOidTag);
    HASHContext* pHASHContext = HASH_Create(eHashType);
    if (!pHASHContext)
    {
        SAL_WARN("xmlsecurity.pdfio", "ValidateSignature: HASH_Create() failed");
        return false;
    }

    // We have a hash, update it with the byte ranges.
    for (const auto& rByteRange : aByteRanges)
    {
        rStream.Seek(rByteRange.first);

        // And now hash this byte range.
        const int nChunkLen = 4096;
        std::vector<unsigned char> aBuffer(nChunkLen);
        for (size_t nByte = 0; nByte < rByteRange.second;)
        {
            size_t nRemainingSize = rByteRange.second - nByte;
            if (nRemainingSize < nChunkLen)
            {
                rStream.ReadBytes(aBuffer.data(), nRemainingSize);
                HASH_Update(pHASHContext, aBuffer.data(), nRemainingSize);
                nByte = rByteRange.second;
            }
            else
            {
                rStream.ReadBytes(aBuffer.data(), nChunkLen);
                HASH_Update(pHASHContext, aBuffer.data(), nChunkLen);
                nByte += nChunkLen;
            }
        }
    }

    // Find out what is the expected length of the hash.
    unsigned int nMaxResultLen = 0;
    switch (eOidTag)
    {
    case SEC_OID_SHA1:
        nMaxResultLen = msfilter::SHA1_HASH_LENGTH;
        rInformation.nDigestID = xml::crypto::DigestID::SHA1;
        break;
    case SEC_OID_SHA256:
        nMaxResultLen = msfilter::SHA256_HASH_LENGTH;
        rInformation.nDigestID = xml::crypto::DigestID::SHA256;
        break;
    case SEC_OID_SHA512:
        nMaxResultLen = msfilter::SHA512_HASH_LENGTH;
        break;
    default:
        SAL_WARN("xmlsecurity.pdfio", "ValidateSignature: unrecognized algorithm");
        return false;
    }

    auto pActualResultBuffer = static_cast<unsigned char*>(PORT_Alloc(nMaxResultLen));
    unsigned int nActualResultLen;
    HASH_End(pHASHContext, pActualResultBuffer, &nActualResultLen, nMaxResultLen);

    CERTCertificate* pCertificate = NSS_CMSSignerInfo_GetSigningCertificate(pCMSSignerInfo, CERT_GetDefaultCertDB());
    if (!pCertificate)
    {
        SAL_WARN("xmlsecurity.pdfio", "ValidateSignature: NSS_CMSSignerInfo_GetSigningCertificate() failed");
        return false;
    }
    else
    {
        uno::Sequence<sal_Int8> aDerCert(pCertificate->derCert.len);
        for (size_t i = 0; i < pCertificate->derCert.len; ++i)
            aDerCert[i] = pCertificate->derCert.data[i];
        OUStringBuffer aBuffer;
        sax::Converter::encodeBase64(aBuffer, aDerCert);
        rInformation.ouX509Certificate = aBuffer.makeStringAndClear();
    }

    PRTime nSigningTime;
    // This may fail, in which case the date should be taken from the dictionary's "M" key.
    if (NSS_CMSSignerInfo_GetSigningTime(pCMSSignerInfo, &nSigningTime) == SECSuccess)
    {
        // First convert the UNIX timestamp to an ISO8601 string.
        OUStringBuffer aBuffer;
        uno::Reference<uno::XComponentContext> xComponentContext = comphelper::getProcessComponentContext();
        CalendarWrapper aCalendarWrapper(xComponentContext);
        // nSigningTime is in microseconds.
        SvXMLUnitConverter::convertDateTime(aBuffer, static_cast<double>(nSigningTime) / 1000000 / tools::Time::secondPerDay, aCalendarWrapper.getEpochStart().GetUNODate());

        // Then convert this string to a local UNO DateTime.
        util::DateTime aUNODateTime;
        try
        {
            utl::ISO8601parseDateTime(aBuffer.toString(), aUNODateTime);
        }
        catch (const std::length_error&)
        {
            SAL_WARN("xmlsecurity.pdfio", "ValidateSignature: failed to parse signature date string");
            return false;
        }
        DateTime aDateTime(aUNODateTime);
        aDateTime.ConvertToLocalTime();
        rInformation.stDateTime = aDateTime.GetUNODateTime();
    }

    // Check if we have a signing certificate attribute.
    SECOidData aOidData;
    aOidData.oid.data = nullptr;
    /*
     * id-aa-signingCertificateV2 OBJECT IDENTIFIER ::=
     * { iso(1) member-body(2) us(840) rsadsi(113549) pkcs(1) pkcs9(9)
     *   smime(16) id-aa(2) 47 }
     */
    if (StringToOID(&aOidData.oid, "1.2.840.113549.1.9.16.2.47", 0) != SECSuccess)
    {
        SAL_WARN("xmlsecurity.pdfio", "StringToOID() failed");
        return false;
    }
    aOidData.offset = SEC_OID_UNKNOWN;
    aOidData.desc = "id-aa-signingCertificateV2";
    aOidData.mechanism = CKM_SHA_1;
    aOidData.supportedExtension = UNSUPPORTED_CERT_EXTENSION;
    NSSCMSAttribute* pAttribute = CMSAttributeArray_FindAttrByOidData(pCMSSignerInfo->authAttr, &aOidData, PR_TRUE);
    if (pAttribute)
        rInformation.bHasSigningCertificate = true;

    SECItem* pContentInfoContentData = pCMSSignedData->contentInfo.content.data;
    if (bNonDetached && pContentInfoContentData && pContentInfoContentData->data)
    {
        // Not a detached signature.
        if (!std::memcmp(pActualResultBuffer, pContentInfoContentData->data, nMaxResultLen) && nActualResultLen == pContentInfoContentData->len)
            rInformation.nStatus = xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED;
    }
    else
    {
        // Detached, the usual case.
        SECItem aActualResultItem;
        aActualResultItem.data = pActualResultBuffer;
        aActualResultItem.len = nActualResultLen;
        if (NSS_CMSSignerInfo_Verify(pCMSSignerInfo, &aActualResultItem, nullptr) == SECSuccess)
            rInformation.nStatus = xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED;
    }

    // Everything went fine
    PORT_Free(pActualResultBuffer);
    HASH_Destroy(pHASHContext);
    NSS_CMSSignerInfo_Destroy(pCMSSignerInfo);
    for (auto pDocumentCertificate : aDocumentCertificates)
        CERT_DestroyCertificate(pDocumentCertificate);

    return true;
#elif defined XMLSEC_CRYPTO_MSCRYPTO
    // Open a message for decoding.
    HCRYPTMSG hMsg = CryptMsgOpenToDecode(PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
                                          CMSG_DETACHED_FLAG,
                                          0,
                                          NULL,
                                          nullptr,
                                          nullptr);
    if (!hMsg)
    {
        SAL_WARN("xmlsecurity.pdfio", "ValidateSignature: CryptMsgOpenToDecode() failed");
        return false;
    }

    // Update the message with the encoded header blob.
    if (!CryptMsgUpdate(hMsg, aSignature.data(), aSignature.size(), TRUE))
    {
        SAL_WARN("xmlsecurity.pdfio", "ValidateSignature, CryptMsgUpdate() for the header failed: " << WindowsErrorString(GetLastError()));
        return false;
    }

    // Update the message with the content blob.
    for (const auto& rByteRange : aByteRanges)
    {
        rStream.Seek(rByteRange.first);

        const int nChunkLen = 4096;
        std::vector<unsigned char> aBuffer(nChunkLen);
        for (size_t nByte = 0; nByte < rByteRange.second;)
        {
            size_t nRemainingSize = rByteRange.second - nByte;
            if (nRemainingSize < nChunkLen)
            {
                rStream.ReadBytes(aBuffer.data(), nRemainingSize);
                if (!CryptMsgUpdate(hMsg, aBuffer.data(), nRemainingSize, FALSE))
                {
                    SAL_WARN("xmlsecurity.pdfio", "ValidateSignature, CryptMsgUpdate() for the content failed: " << WindowsErrorString(GetLastError()));
                    return false;
                }
                nByte = rByteRange.second;
            }
            else
            {
                rStream.ReadBytes(aBuffer.data(), nChunkLen);
                if (!CryptMsgUpdate(hMsg, aBuffer.data(), nChunkLen, FALSE))
                {
                    SAL_WARN("xmlsecurity.pdfio", "ValidateSignature, CryptMsgUpdate() for the content failed: " << WindowsErrorString(GetLastError()));
                    return false;
                }
                nByte += nChunkLen;
            }
        }
    }
    if (!CryptMsgUpdate(hMsg, nullptr, 0, TRUE))
    {
        SAL_WARN("xmlsecurity.pdfio", "ValidateSignature, CryptMsgUpdate() for the last content failed: " << WindowsErrorString(GetLastError()));
        return false;
    }

    // Get the CRYPT_ALGORITHM_IDENTIFIER from the message.
    DWORD nDigestID = 0;
    if (!CryptMsgGetParam(hMsg, CMSG_SIGNER_HASH_ALGORITHM_PARAM, 0, nullptr, &nDigestID))
    {
        SAL_WARN("xmlsecurity.pdfio", "ValidateSignature: CryptMsgGetParam() failed: " << WindowsErrorString(GetLastError()));
        return false;
    }
    std::unique_ptr<BYTE[]> pDigestBytes(new BYTE[nDigestID]);
    if (!CryptMsgGetParam(hMsg, CMSG_SIGNER_HASH_ALGORITHM_PARAM, 0, pDigestBytes.get(), &nDigestID))
    {
        SAL_WARN("xmlsecurity.pdfio", "ValidateSignature: CryptMsgGetParam() failed: " << WindowsErrorString(GetLastError()));
        return false;
    }
    auto pDigestID = reinterpret_cast<CRYPT_ALGORITHM_IDENTIFIER*>(pDigestBytes.get());
    if (OString(szOID_NIST_sha256) == pDigestID->pszObjId)
        rInformation.nDigestID = xml::crypto::DigestID::SHA256;
    else if (OString(szOID_RSA_SHA1RSA) == pDigestID->pszObjId || OString(szOID_OIWSEC_sha1) == pDigestID->pszObjId)
        rInformation.nDigestID = xml::crypto::DigestID::SHA1;
    else
        // Don't error out here, we can still verify the message digest correctly, just the digest ID won't be set.
        SAL_WARN("xmlsecurity.pdfio", "ValidateSignature: unhandled algorithm identifier '"<<pDigestID->pszObjId<<"'");

    // Get the signer CERT_INFO from the message.
    DWORD nSignerCertInfo = 0;
    if (!CryptMsgGetParam(hMsg, CMSG_SIGNER_CERT_INFO_PARAM, 0, nullptr, &nSignerCertInfo))
    {
        SAL_WARN("xmlsecurity.pdfio", "ValidateSignature: CryptMsgGetParam() failed");
        return false;
    }
    std::unique_ptr<BYTE[]> pSignerCertInfoBuf(new BYTE[nSignerCertInfo]);
    if (!CryptMsgGetParam(hMsg, CMSG_SIGNER_CERT_INFO_PARAM, 0, pSignerCertInfoBuf.get(), &nSignerCertInfo))
    {
        SAL_WARN("xmlsecurity.pdfio", "ValidateSignature: CryptMsgGetParam() failed");
        return false;
    }
    PCERT_INFO pSignerCertInfo = reinterpret_cast<PCERT_INFO>(pSignerCertInfoBuf.get());

    // Open a certificate store in memory using CERT_STORE_PROV_MSG, which
    // initializes it with the certificates from the message.
    HCERTSTORE hStoreHandle = CertOpenStore(CERT_STORE_PROV_MSG,
                                            PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
                                            NULL,
                                            0,
                                            hMsg);
    if (!hStoreHandle)
    {
        SAL_WARN("xmlsecurity.pdfio", "ValidateSignature: CertOpenStore() failed");
        return false;
    }

    // Find the signer's certificate in the store.
    PCCERT_CONTEXT pSignerCertContext = CertGetSubjectCertificateFromStore(hStoreHandle,
                                        PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
                                        pSignerCertInfo);
    if (!pSignerCertContext)
    {
        SAL_WARN("xmlsecurity.pdfio", "ValidateSignature: CertGetSubjectCertificateFromStore() failed");
        return false;
    }
    else
    {
        // Write rInformation.ouX509Certificate.
        uno::Sequence<sal_Int8> aDerCert(pSignerCertContext->cbCertEncoded);
        for (size_t i = 0; i < pSignerCertContext->cbCertEncoded; ++i)
            aDerCert[i] = pSignerCertContext->pbCertEncoded[i];
        OUStringBuffer aBuffer;
        sax::Converter::encodeBase64(aBuffer, aDerCert);
        rInformation.ouX509Certificate = aBuffer.makeStringAndClear();
    }

    if (bNonDetached)
    {
        // Not a detached signature.
        DWORD nContentParam = 0;
        if (!CryptMsgGetParam(hMsg, CMSG_CONTENT_PARAM, 0, nullptr, &nContentParam))
        {
            SAL_WARN("xmlsecurity.pdfio", "ValidateSignature: CryptMsgGetParam() failed");
            return false;
        }

        std::vector<BYTE> aContentParam(nContentParam);
        if (!CryptMsgGetParam(hMsg, CMSG_CONTENT_PARAM, 0, aContentParam.data(), &nContentParam))
        {
            SAL_WARN("xmlsecurity.pdfio", "ValidateSignature: CryptMsgGetParam() failed");
            return false;
        }

        if (VerifyNonDetachedSignature(rStream, aByteRanges, aContentParam))
            rInformation.nStatus = xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED;
    }
    else
    {
        // Detached, the usual case.
        // Use the CERT_INFO from the signer certificate to verify the signature.
        if (CryptMsgControl(hMsg, 0, CMSG_CTRL_VERIFY_SIGNATURE, pSignerCertContext->pCertInfo))
            rInformation.nStatus = xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED;
    }

    // Check if we have a signing certificate attribute.
    DWORD nSignedAttributes = 0;
    if (CryptMsgGetParam(hMsg, CMSG_SIGNER_AUTH_ATTR_PARAM, 0, nullptr, &nSignedAttributes))
    {
        std::unique_ptr<BYTE[]> pSignedAttributesBuf(new BYTE[nSignedAttributes]);
        if (!CryptMsgGetParam(hMsg, CMSG_SIGNER_AUTH_ATTR_PARAM, 0, pSignedAttributesBuf.get(), &nSignedAttributes))
        {
            SAL_WARN("xmlsecurity.pdfio", "ValidateSignature: CryptMsgGetParam() failed");
            return false;
        }
        auto pSignedAttributes = reinterpret_cast<PCRYPT_ATTRIBUTES>(pSignedAttributesBuf.get());
        for (size_t nAttr = 0; nAttr < pSignedAttributes->cAttr; ++nAttr)
        {
            CRYPT_ATTRIBUTE& rAttr = pSignedAttributes->rgAttr[nAttr];
            /*
             * id-aa-signingCertificateV2 OBJECT IDENTIFIER ::=
             * { iso(1) member-body(2) us(840) rsadsi(113549) pkcs(1) pkcs9(9)
             *   smime(16) id-aa(2) 47 }
             */
            OString aOid("1.2.840.113549.1.9.16.2.47");
            if (aOid == rAttr.pszObjId)
            {
                rInformation.bHasSigningCertificate = true;
                break;
            }
        }
    }

    CertCloseStore(hStoreHandle, CERT_CLOSE_STORE_FORCE_FLAG);
    CryptMsgClose(hMsg);
    return true;
#else
    // Not implemented.
    (void)rStream;
    (void)rInformation;

    return false;
#endif
}

} // namespace pdfio
} // namespace xmlsecurity

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
