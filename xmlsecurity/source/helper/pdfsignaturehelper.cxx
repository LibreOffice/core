/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <pdfsignaturehelper.hxx>

#include <memory>

#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/security/CertificateValidity.hpp>
#include <com/sun/star/uno/SecurityException.hpp>
#include <com/sun/star/security/DocumentSignatureInformation.hpp>
#include <com/sun/star/xml/crypto/XSecurityEnvironment.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>

#include <comphelper/propertysequence.hxx>
#include <sal/log.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/mediadescriptor.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/filter/pdfdocument.hxx>
#include <vcl/checksum.hxx>
#include <rtl/ustrbuf.hxx>
#include <svl/cryptosign.hxx>
#include <config_features.h>
#include <vcl/filter/PDFiumLibrary.hxx>

using namespace ::com::sun::star;

namespace
{
/// Gets the current page of the current view from xModel and puts it to the 1-based rPage.
bool GetSignatureLinePage(const uno::Reference<frame::XModel>& xModel, sal_Int32& rPage)
{
    uno::Reference<drawing::XDrawView> xController(xModel->getCurrentController(), uno::UNO_QUERY);
    if (!xController.is())
    {
        return false;
    }

    uno::Reference<beans::XPropertySet> xPage(xController->getCurrentPage(), uno::UNO_QUERY);
    if (!xPage.is())
    {
        return false;
    }

    return xPage->getPropertyValue("Number") >>= rPage;
}

/// If the currently selected shape is a Draw signature line, export that to PDF.
void GetSignatureLineShape(const uno::Reference<frame::XModel>& xModel, sal_Int32& rPage,
                           std::vector<sal_Int8>& rSignatureLineShape)
{
    if (!xModel.is())
    {
        return;
    }

    if (!GetSignatureLinePage(xModel, rPage))
    {
        return;
    }

    uno::Reference<drawing::XShapes> xShapes(xModel->getCurrentSelection(), uno::UNO_QUERY);
    if (!xShapes.is() || xShapes->getCount() < 1)
    {
        return;
    }

    uno::Reference<beans::XPropertySet> xShapeProps(xShapes->getByIndex(0), uno::UNO_QUERY);
    if (!xShapeProps.is())
    {
        return;
    }

    comphelper::SequenceAsHashMap aMap(xShapeProps->getPropertyValue("InteropGrabBag"));
    auto it = aMap.find("SignatureCertificate");
    if (it == aMap.end())
    {
        return;
    }

    // We know that we add a signature line shape to an existing PDF at this point.

    uno::Reference<frame::XStorable> xStorable(xModel, uno::UNO_QUERY);
    if (!xStorable.is())
    {
        return;
    }

    // Export just the signature line.
    utl::MediaDescriptor aMediaDescriptor;
    aMediaDescriptor["FilterName"] <<= OUString("draw_pdf_Export");
    SvMemoryStream aStream;
    uno::Reference<io::XOutputStream> xStream(new utl::OStreamWrapper(aStream));
    aMediaDescriptor["OutputStream"] <<= xStream;
    uno::Sequence<beans::PropertyValue> aFilterData(
        comphelper::InitPropertySequence({ { "Selection", uno::Any(xShapes) } }));
    aMediaDescriptor["FilterData"] <<= aFilterData;
    xStorable->storeToURL("private:stream", aMediaDescriptor.getAsConstPropertyValueList());
    xStream->flush();

    aStream.Seek(0);
    rSignatureLineShape = std::vector<sal_Int8>(aStream.GetSize());
    aStream.ReadBytes(rSignatureLineShape.data(), rSignatureLineShape.size());
}

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
            SAL_WARN("xmlsecurity.helper",
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

#if HAVE_FEATURE_PDFIUM
/// Collects the checksum of each page of one version of the PDF.
void AnalyizeSignatureStream(SvMemoryStream& rStream, std::vector<BitmapChecksum>& rPageChecksums)
{
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

        BitmapChecksum nPageChecksum = pPdfPage->getChecksum();
        rPageChecksums.push_back(nPageChecksum);
    }
}
#endif

/**
 * Checks if incremental updates after singing performed valid modifications only.
 * Annotations/commenting is OK, other changes are not.
 */
bool IsValidSignature(SvStream& rStream, vcl::filter::PDFObjectElement* pSignature)
{
    size_t nSignatureEOF = 0;
    if (!GetEOFOfSignature(pSignature, nSignatureEOF))
    {
        return false;
    }

#if HAVE_FEATURE_PDFIUM
    SvMemoryStream aSignatureStream;
    sal_uInt64 nPos = rStream.Tell();
    rStream.Seek(0);
    aSignatureStream.WriteStream(rStream, nSignatureEOF);
    rStream.Seek(nPos);
    aSignatureStream.Seek(0);
    std::vector<BitmapChecksum> aSignedPages;
    AnalyizeSignatureStream(aSignatureStream, aSignedPages);

    SvMemoryStream aFullStream;
    nPos = rStream.Tell();
    rStream.Seek(0);
    aFullStream.WriteStream(rStream);
    rStream.Seek(nPos);
    aFullStream.Seek(0);
    std::vector<BitmapChecksum> aAllPages;
    AnalyizeSignatureStream(aFullStream, aAllPages);

    // Fail if any page looks different after signing and at the end. Annotations/commenting doesn't
    // count, though.
    return aSignedPages == aAllPages;
#else
    (void)rStream;
    return true;
#endif
}

/**
 * @param rInformation The actual result.
 * @param rDocument the parsed document to see if the signature is partial.
 * @return If we can determinate a result.
 */
bool ValidateSignature(SvStream& rStream, vcl::filter::PDFObjectElement* pSignature,
                       SignatureInformation& rInformation, vcl::filter::PDFDocument& rDocument)
{
    vcl::filter::PDFObjectElement* pValue = pSignature->LookupObject("V");
    if (!pValue)
    {
        SAL_WARN("xmlsecurity.helper", "ValidateSignature: no value");
        return false;
    }

    auto pContents = dynamic_cast<vcl::filter::PDFHexStringElement*>(pValue->Lookup("Contents"));
    if (!pContents)
    {
        SAL_WARN("xmlsecurity.helper", "ValidateSignature: no contents");
        return false;
    }

    auto pByteRange = dynamic_cast<vcl::filter::PDFArrayElement*>(pValue->Lookup("ByteRange"));
    if (!pByteRange || pByteRange->GetElements().size() < 2)
    {
        SAL_WARN("xmlsecurity.helper", "ValidateSignature: no byte range or too few elements");
        return false;
    }

    auto pSubFilter = dynamic_cast<vcl::filter::PDFNameElement*>(pValue->Lookup("SubFilter"));
    const bool bNonDetached = pSubFilter && pSubFilter->GetValue() == "adbe.pkcs7.sha1";
    if (!pSubFilter
        || (pSubFilter->GetValue() != "adbe.pkcs7.detached" && !bNonDetached
            && pSubFilter->GetValue() != "ETSI.CAdES.detached"))
    {
        if (!pSubFilter)
            SAL_WARN("xmlsecurity.helper", "ValidateSignature: missing sub-filter");
        else
            SAL_WARN("xmlsecurity.helper", "ValidateSignature: unsupported sub-filter: '"
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
        SAL_WARN("xmlsecurity.helper", "ValidateSignature: expected 2 byte ranges");
        return false;
    }
    if (aByteRanges[0].first != 0)
    {
        SAL_WARN("xmlsecurity.helper", "ValidateSignature: first range start is not 0");
        return false;
    }
    // 2 is the leading "<" and the trailing ">" around the hex string.
    size_t nSignatureLength = static_cast<size_t>(pContents->GetValue().getLength()) + 2;
    if (aByteRanges[1].first != (aByteRanges[0].second + nSignatureLength))
    {
        SAL_WARN("xmlsecurity.helper",
                 "ValidateSignature: second range start is not the end of the signature");
        return false;
    }
    rInformation.bPartialDocumentSignature = !IsCompleteSignature(rStream, rDocument, pSignature);
    if (!IsValidSignature(rStream, pSignature))
    {
        SAL_WARN("xmlsecurity.helper", "ValidateSignature: invalid incremental update detected");
        return false;
    }

    // At this point there is no obviously missing info to validate the
    // signature.
    std::vector<unsigned char> aSignature = vcl::filter::PDFDocument::DecodeHexString(pContents);
    if (aSignature.empty())
    {
        SAL_WARN("xmlsecurity.helper", "ValidateSignature: empty contents");
        return false;
    }

    return svl::crypto::Signing::Verify(rStream, aByteRanges, bNonDetached, aSignature,
                                        rInformation);
}
}

PDFSignatureHelper::PDFSignatureHelper() = default;

bool PDFSignatureHelper::ReadAndVerifySignature(
    const uno::Reference<io::XInputStream>& xInputStream)
{
    if (!xInputStream.is())
    {
        SAL_WARN("xmlsecurity.helper", "input stream missing");
        return false;
    }

    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    return ReadAndVerifySignatureSvStream(*pStream);
}

bool PDFSignatureHelper::ReadAndVerifySignatureSvStream(SvStream& rStream)
{
    vcl::filter::PDFDocument aDocument;
    if (!aDocument.Read(rStream))
    {
        SAL_WARN("xmlsecurity.helper", "failed to read the document");
        return false;
    }

    std::vector<vcl::filter::PDFObjectElement*> aSignatures = aDocument.GetSignatureWidgets();
    if (aSignatures.empty())
        return true;

    m_aSignatureInfos.clear();

    for (size_t i = 0; i < aSignatures.size(); ++i)
    {
        SignatureInformation aInfo(i);

        if (!ValidateSignature(rStream, aSignatures[i], aInfo, aDocument))
            SAL_WARN("xmlsecurity.helper", "failed to determine digest match");

        m_aSignatureInfos.push_back(aInfo);
    }

    return true;
}

SignatureInformations const& PDFSignatureHelper::GetSignatureInformations() const
{
    return m_aSignatureInfos;
}

uno::Sequence<security::DocumentSignatureInformation>
PDFSignatureHelper::GetDocumentSignatureInformations(
    const uno::Reference<xml::crypto::XSecurityEnvironment>& xSecEnv) const
{
    uno::Sequence<security::DocumentSignatureInformation> aRet(m_aSignatureInfos.size());

    for (size_t i = 0; i < m_aSignatureInfos.size(); ++i)
    {
        const SignatureInformation& rInternal = m_aSignatureInfos[i];
        security::DocumentSignatureInformation& rExternal = aRet[i];
        rExternal.SignatureIsValid
            = rInternal.nStatus == xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED;
        if (!rInternal.ouX509Certificate.isEmpty())
            rExternal.Signer = xSecEnv->createCertificateFromAscii(rInternal.ouX509Certificate);
        rExternal.PartialDocumentSignature = rInternal.bPartialDocumentSignature;

        // Verify certificate.
        if (rExternal.Signer.is())
        {
            try
            {
                rExternal.CertificateStatus = xSecEnv->verifyCertificate(rExternal.Signer, {});
            }
            catch (const uno::SecurityException&)
            {
                DBG_UNHANDLED_EXCEPTION("xmlsecurity.helper", "failed to verify certificate");
                rExternal.CertificateStatus = security::CertificateValidity::INVALID;
            }
        }
        else
            rExternal.CertificateStatus = security::CertificateValidity::INVALID;
    }

    return aRet;
}

sal_Int32 PDFSignatureHelper::GetNewSecurityId() const { return m_aSignatureInfos.size(); }

void PDFSignatureHelper::SetX509Certificate(
    const uno::Reference<security::XCertificate>& xCertificate)
{
    m_xCertificate = xCertificate;
}

void PDFSignatureHelper::SetDescription(const OUString& rDescription)
{
    m_aDescription = rDescription;
}

bool PDFSignatureHelper::Sign(const uno::Reference<frame::XModel>& xModel,
                              const uno::Reference<io::XInputStream>& xInputStream, bool bAdES)
{
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    vcl::filter::PDFDocument aDocument;
    if (!aDocument.Read(*pStream))
    {
        SAL_WARN("xmlsecurity.helper", "failed to read the document");
        return false;
    }

    sal_Int32 nPage = 0;
    std::vector<sal_Int8> aSignatureLineShape;
    GetSignatureLineShape(xModel, nPage, aSignatureLineShape);
    if (nPage > 0)
    {
        // UNO page number is 1-based.
        aDocument.SetSignaturePage(nPage - 1);
    }
    if (!aSignatureLineShape.empty())
    {
        aDocument.SetSignatureLine(aSignatureLineShape);
    }

    if (!aDocument.Sign(m_xCertificate, m_aDescription, bAdES))
    {
        SAL_WARN("xmlsecurity.helper", "failed to sign");
        return false;
    }

    uno::Reference<io::XStream> xStream(xInputStream, uno::UNO_QUERY);
    std::unique_ptr<SvStream> pOutStream(utl::UcbStreamHelper::CreateStream(xStream, true));
    if (!aDocument.Write(*pOutStream))
    {
        SAL_WARN("xmlsecurity.helper", "failed to write signed data");
        return false;
    }

    return true;
}

bool PDFSignatureHelper::RemoveSignature(const uno::Reference<io::XInputStream>& xInputStream,
                                         sal_uInt16 nPosition)
{
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    vcl::filter::PDFDocument aDocument;
    if (!aDocument.Read(*pStream))
    {
        SAL_WARN("xmlsecurity.helper", "failed to read the document");
        return false;
    }

    if (!aDocument.RemoveSignature(nPosition))
    {
        SAL_WARN("xmlsecurity.helper", "failed to remove signature");
        return false;
    }

    uno::Reference<io::XStream> xStream(xInputStream, uno::UNO_QUERY);
    uno::Reference<io::XTruncate> xTruncate(xStream, uno::UNO_QUERY);
    if (!xTruncate.is())
    {
        SAL_WARN("xmlsecurity.helper", "failed to truncate");
        return false;
    }
    xTruncate->truncate();
    std::unique_ptr<SvStream> pOutStream(utl::UcbStreamHelper::CreateStream(xStream, true));
    if (!aDocument.Write(*pOutStream))
    {
        SAL_WARN("xmlsecurity.helper", "failed to write without signature");
        return false;
    }

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
