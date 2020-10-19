/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <pdfio/pdfdocument.hxx>

#include <memory>
#include <vector>

#include <config_features.h>

#include <vcl/filter/PDFiumLibrary.hxx>
#include <rtl/string.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <sal/types.h>

#include <svl/sigstruct.hxx>
#include <svl/cryptosign.hxx>
#include <vcl/filter/pdfdocument.hxx>
#include <vcl/bitmap.hxx>

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

/// Collects the checksum of each page of one version of the PDF.
void AnalyizeSignatureStream(SvMemoryStream& rStream, std::vector<BitmapChecksum>& rPageChecksums,
                             int nMDPPerm)
{
#if HAVE_FEATURE_PDFIUM
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    vcl::pdf::PDFiumDocument aPdfDocument(
        FPDF_LoadMemDocument(rStream.GetData(), rStream.GetSize(), /*password=*/nullptr));

    int nPageCount = aPdfDocument.getPageCount();
    for (int nPage = 0; nPage < nPageCount; ++nPage)
    {
        std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage(aPdfDocument.openPage(nPage));
        if (!pPdfPage)
        {
            return;
        }

        BitmapChecksum nPageChecksum = pPdfPage->getChecksum(nMDPPerm);
        rPageChecksums.push_back(nPageChecksum);
    }
#else
    (void)rStream;
#endif
}

/**
 * Checks if incremental updates after singing performed valid modifications only.
 * nMDPPerm decides if annotations/commenting is OK, other changes are always not.
 */
bool IsValidSignature(SvStream& rStream, vcl::filter::PDFObjectElement* pSignature, int nMDPPerm)
{
    size_t nSignatureEOF = 0;
    if (!GetEOFOfSignature(pSignature, nSignatureEOF))
    {
        return false;
    }

    SvMemoryStream aSignatureStream;
    sal_uInt64 nPos = rStream.Tell();
    rStream.Seek(0);
    aSignatureStream.WriteStream(rStream, nSignatureEOF);
    rStream.Seek(nPos);
    aSignatureStream.Seek(0);
    std::vector<BitmapChecksum> aSignedPages;
    AnalyizeSignatureStream(aSignatureStream, aSignedPages, nMDPPerm);

    SvMemoryStream aFullStream;
    nPos = rStream.Tell();
    rStream.Seek(0);
    aFullStream.WriteStream(rStream);
    rStream.Seek(nPos);
    aFullStream.Seek(0);
    std::vector<BitmapChecksum> aAllPages;
    AnalyizeSignatureStream(aFullStream, aAllPages, nMDPPerm);

    // Fail if any page looks different after signing and at the end. Annotations/commenting doesn't
    // count, though.
    return aSignedPages == aAllPages;
}
}

namespace xmlsecurity
{
namespace pdfio
{
bool ValidateSignature(SvStream& rStream, vcl::filter::PDFObjectElement* pSignature,
                       SignatureInformation& rInformation, vcl::filter::PDFDocument& rDocument,
                       int nMDPPerm)
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
    if (!pSubFilter
        || (pSubFilter->GetValue() != "adbe.pkcs7.detached" && !bNonDetached
            && pSubFilter->GetValue() != "ETSI.CAdES.detached"))
    {
        if (!pSubFilter)
            SAL_WARN("xmlsecurity.pdfio", "ValidateSignature: missing sub-filter");
        else
            SAL_WARN("xmlsecurity.pdfio", "ValidateSignature: unsupported sub-filter: '"
                                              << pSubFilter->GetValue() << "'");
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
        SAL_WARN("xmlsecurity.pdfio",
                 "ValidateSignature: second range start is not the end of the signature");
        return false;
    }
    rInformation.bPartialDocumentSignature = !IsCompleteSignature(rStream, rDocument, pSignature);
    if (!IsValidSignature(rStream, pSignature, nMDPPerm))
    {
        SAL_WARN("xmlsecurity.pdfio", "ValidateSignature: invalid incremental update detected");
        return false;
    }

    // At this point there is no obviously missing info to validate the
    // signature.
    std::vector<unsigned char> aSignature = vcl::filter::PDFDocument::DecodeHexString(pContents);
    if (aSignature.empty())
    {
        SAL_WARN("xmlsecurity.pdfio", "ValidateSignature: empty contents");
        return false;
    }

    return svl::crypto::Signing::Verify(rStream, aByteRanges, bNonDetached, aSignature,
                                        rInformation);
}

} // namespace pdfio
} // namespace xmlsecurity

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
