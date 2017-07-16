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

#include <svl/sigstruct.hxx>
#include <svl/cryptosign.hxx>

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

namespace xmlsecurity
{
namespace pdfio
{

bool ValidateSignature(SvStream& rStream, vcl::filter::PDFObjectElement* pSignature, SignatureInformation& rInformation, bool bLast)
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
    const bool bNonDetached = pSubFilter && pSubFilter->GetValue() == "adbe.pkcs7.sha1";
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
    size_t nByteRangeOffset = 0;
    const std::vector<vcl::filter::PDFElement*>& rByteRangeElements = pByteRange->GetElements();
    for (size_t i = 0; i < rByteRangeElements.size(); ++i)
    {
        auto pNumber = dynamic_cast<vcl::filter::PDFNumberElement*>(rByteRangeElements[i]);
        if (!pNumber)
        {
            SAL_WARN("xmlsecurity.pdfio", "ValidateSignature: signature offset and length has to be a number");
            return false;
        }

        if (i % 2 == 0)
        {
            nByteRangeOffset = pNumber->GetValue();
            continue;
        }
        size_t nByteRangeLength = pNumber->GetValue();
        aByteRanges.emplace_back(nByteRangeOffset, nByteRangeLength);
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
    rStream.Seek(STREAM_SEEK_TO_END);
    size_t nFileEnd = rStream.Tell();
    if (bLast && (aByteRanges[1].first + aByteRanges[1].second) != nFileEnd)
        // Second range end is not the end of the file.
        rInformation.bPartialDocumentSignature = true;

    // At this point there is no obviously missing info to validate the
    // signature.
    std::vector<unsigned char> aSignature = vcl::filter::PDFDocument::DecodeHexString(pContents);
    if (aSignature.empty())
    {
        SAL_WARN("xmlsecurity.pdfio", "ValidateSignature: empty contents");
        return false;
    }

    return svl::crypto::Signing::Verify(rStream, aByteRanges, bNonDetached, aSignature, rInformation);
}

} // namespace pdfio
} // namespace xmlsecurity

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
