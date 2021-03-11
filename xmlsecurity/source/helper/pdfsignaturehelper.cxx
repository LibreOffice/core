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
#include <svl/cryptosign.hxx>
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

/// Represents a parsed signature.
struct Signature
{
    std::unique_ptr<vcl::pdf::PDFiumSignature> m_pSignature;
    /// Offset+length pairs.
    std::vector<std::pair<size_t, size_t>> m_aByteRanges;
};

/// Turns an array of floats into offset + length pairs.
void GetByteRangesFromPDF(std::unique_ptr<vcl::pdf::PDFiumSignature>& pSignature,
                          std::vector<std::pair<size_t, size_t>>& rByteRanges)
{
    std::vector<int> aByteRange = pSignature->getByteRange();
    if (aByteRange.empty())
    {
        SAL_WARN("xmlsecurity.helper", "GetByteRangesFromPDF: no byte ranges");
        return;
    }

    size_t nByteRangeOffset = 0;
    for (size_t i = 0; i < aByteRange.size(); ++i)
    {
        if (i % 2 == 0)
        {
            nByteRangeOffset = aByteRange[i];
            continue;
        }

        size_t nLength = aByteRange[i];
        rByteRanges.emplace_back(nByteRangeOffset, nLength);
    }
}

/// Determines the last position that is covered by a signature.
bool GetEOFOfSignature(const Signature& rSignature, size_t& rEOF)
{
    if (rSignature.m_aByteRanges.size() < 2)
    {
        return false;
    }

    rEOF = rSignature.m_aByteRanges[1].first + rSignature.m_aByteRanges[1].second;
    return true;
}

/**
 * Get the value of the "modification detection and prevention" permission:
 * Valid values are 1, 2 and 3: only 3 allows annotations after signing.
 */
int GetMDPPerm(const std::vector<Signature>& rSignatures)
{
    int nRet = 3;

    if (rSignatures.empty())
    {
        return nRet;
    }

    for (const auto& rSignature : rSignatures)
    {
        int nPerm = rSignature.m_pSignature->getDocMDPPermission();
        if (nPerm != 0)
        {
            return nPerm;
        }
    }

    return nRet;
}

/// Checks if there are unsigned incremental updates between the signatures or after the last one.
bool IsCompleteSignature(SvStream& rStream, const Signature& rSignature,
                         const std::set<unsigned int>& rSignedEOFs,
                         const std::vector<unsigned int>& rAllEOFs)
{
    size_t nSignatureEOF = 0;
    if (!GetEOFOfSignature(rSignature, nSignatureEOF))
    {
        return false;
    }

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

        if (rSignedEOFs.find(rEOF) == rSignedEOFs.end())
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

/**
 * Contains checksums of a PDF page, which is rendered without annotations. It also contains
 * the geometry of a few dangerous annotation types.
 */
struct PageChecksum
{
    BitmapChecksum m_nPageContent;
    std::vector<basegfx::B2DRectangle> m_aAnnotations;
    bool operator==(const PageChecksum& rChecksum) const;
};

bool PageChecksum::operator==(const PageChecksum& rChecksum) const
{
    if (m_nPageContent != rChecksum.m_nPageContent)
    {
        return false;
    }

    return m_aAnnotations == rChecksum.m_aAnnotations;
}

/// Collects the checksum of each page of one version of the PDF.
void AnalyizeSignatureStream(SvMemoryStream& rStream, std::vector<PageChecksum>& rPageChecksums,
                             int nMDPPerm)
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = pPdfium->openDocument(rStream.GetData(), rStream.GetSize());
    if (!pPdfDocument)
    {
        return;
    }

    int nPageCount = pPdfDocument->getPageCount();
    for (int nPage = 0; nPage < nPageCount; ++nPage)
    {
        std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(nPage);
        if (!pPdfPage)
        {
            return;
        }

        PageChecksum aPageChecksum;
        aPageChecksum.m_nPageContent = pPdfPage->getChecksum(nMDPPerm);
        for (int i = 0; i < pPdfPage->getAnnotationCount(); ++i)
        {
            std::unique_ptr<vcl::pdf::PDFiumAnnotation> pPdfAnnotation = pPdfPage->getAnnotation(i);
            vcl::pdf::PDFAnnotationSubType eType = pPdfAnnotation->getSubType();
            switch (eType)
            {
                case vcl::pdf::PDFAnnotationSubType::Unknown:
                case vcl::pdf::PDFAnnotationSubType::FreeText:
                case vcl::pdf::PDFAnnotationSubType::Stamp:
                case vcl::pdf::PDFAnnotationSubType::Redact:
                    aPageChecksum.m_aAnnotations.push_back(pPdfAnnotation->getRectangle());
                    break;
                default:
                    break;
            }
        }
        rPageChecksums.push_back(aPageChecksum);
    }
}

/**
 * Checks if incremental updates after singing performed valid modifications only.
 * nMDPPerm decides if annotations/commenting is OK, other changes are always not.
 */
bool IsValidSignature(SvStream& rStream, const Signature& rSignature, int nMDPPerm)
{
    size_t nSignatureEOF = 0;
    if (!GetEOFOfSignature(rSignature, nSignatureEOF))
    {
        return false;
    }

    SvMemoryStream aSignatureStream;
    sal_uInt64 nPos = rStream.Tell();
    rStream.Seek(0);
    aSignatureStream.WriteStream(rStream, nSignatureEOF);
    rStream.Seek(nPos);
    aSignatureStream.Seek(0);
    std::vector<PageChecksum> aSignedPages;
    AnalyizeSignatureStream(aSignatureStream, aSignedPages, nMDPPerm);

    SvMemoryStream aFullStream;
    nPos = rStream.Tell();
    rStream.Seek(0);
    aFullStream.WriteStream(rStream);
    rStream.Seek(nPos);
    aFullStream.Seek(0);
    std::vector<PageChecksum> aAllPages;
    AnalyizeSignatureStream(aFullStream, aAllPages, nMDPPerm);

    // Fail if any page looks different after signing and at the end. Annotations/commenting doesn't
    // count, though.
    return aSignedPages == aAllPages;
}

/**
 * @param rInformation The actual result.
 * @param rDocument the parsed document to see if the signature is partial.
 * @return If we can determinate a result.
 */
bool ValidateSignature(SvStream& rStream, const Signature& rSignature,
                       SignatureInformation& rInformation, int nMDPPerm,
                       const std::set<unsigned int>& rSignatureEOFs,
                       const std::vector<unsigned int>& rTrailerEnds)
{
    std::vector<unsigned char> aContents = rSignature.m_pSignature->getContents();
    if (aContents.empty())
    {
        SAL_WARN("xmlsecurity.helper", "ValidateSignature: no contents");
        return false;
    }

    OString aSubFilter = rSignature.m_pSignature->getSubFilter();

    const bool bNonDetached = aSubFilter == "adbe.pkcs7.sha1";
    if (aSubFilter.isEmpty()
        || (aSubFilter != "adbe.pkcs7.detached" && !bNonDetached
            && aSubFilter != "ETSI.CAdES.detached"))
    {
        if (aSubFilter.isEmpty())
            SAL_WARN("xmlsecurity.helper", "ValidateSignature: missing sub-filter");
        else
            SAL_WARN("xmlsecurity.helper",
                     "ValidateSignature: unsupported sub-filter: '" << aSubFilter << "'");
        return false;
    }

    // Reason / comment / description is optional.
    rInformation.ouDescription = rSignature.m_pSignature->getReason();

    // Date: used only when the time of signing is not available in the
    // signature.
    rInformation.stDateTime = rSignature.m_pSignature->getTime();

    // Detect if the byte ranges don't cover everything, but the signature itself.
    if (rSignature.m_aByteRanges.size() < 2)
    {
        SAL_WARN("xmlsecurity.helper", "ValidateSignature: expected 2 byte ranges");
        return false;
    }
    if (rSignature.m_aByteRanges[0].first != 0)
    {
        SAL_WARN("xmlsecurity.helper", "ValidateSignature: first range start is not 0");
        return false;
    }
    // Binary vs hex dump and 2 is the leading "<" and the trailing ">" around the hex string.
    size_t nSignatureLength = aContents.size() * 2 + 2;
    if (rSignature.m_aByteRanges[1].first
        != (rSignature.m_aByteRanges[0].second + nSignatureLength))
    {
        SAL_WARN("xmlsecurity.helper",
                 "ValidateSignature: second range start is not the end of the signature");
        return false;
    }
    rInformation.bPartialDocumentSignature
        = !IsCompleteSignature(rStream, rSignature, rSignatureEOFs, rTrailerEnds);
    if (!IsValidSignature(rStream, rSignature, nMDPPerm))
    {
        SAL_WARN("xmlsecurity.helper", "ValidateSignature: invalid incremental update detected");
        return false;
    }

    // At this point there is no obviously missing info to validate the
    // signature.
    return svl::crypto::Signing::Verify(rStream, rSignature.m_aByteRanges, bNonDetached, aContents,
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
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
    {
        return true;
    }

    SvMemoryStream aStream;
    sal_uInt64 nPos = rStream.Tell();
    rStream.Seek(0);
    aStream.WriteStream(rStream);
    rStream.Seek(nPos);
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = pPdfium->openDocument(aStream.GetData(), aStream.GetSize());
    if (!pPdfDocument)
    {
        SAL_WARN("xmlsecurity.helper", "failed to read the document");
        return false;
    }

    int nSignatureCount = pPdfDocument->getSignatureCount();
    if (nSignatureCount <= 0)
    {
        return true;
    }
    std::vector<Signature> aSignatures(nSignatureCount);
    for (int i = 0; i < nSignatureCount; ++i)
    {
        std::unique_ptr<vcl::pdf::PDFiumSignature> pSignature = pPdfDocument->getSignature(i);
        std::vector<std::pair<size_t, size_t>> aByteRanges;
        GetByteRangesFromPDF(pSignature, aByteRanges);
        aSignatures[i] = Signature{ std::move(pSignature), aByteRanges };
    }

    std::set<unsigned int> aSignatureEOFs;
    for (const auto& rSignature : aSignatures)
    {
        size_t nEOF = 0;
        if (GetEOFOfSignature(rSignature, nEOF))
        {
            aSignatureEOFs.insert(nEOF);
        }
    }

    std::vector<unsigned int> aTrailerEnds = pPdfDocument->getTrailerEnds();

    m_aSignatureInfos.clear();

    int nMDPPerm = GetMDPPerm(aSignatures);

    for (size_t i = 0; i < aSignatures.size(); ++i)
    {
        SignatureInformation aInfo(i);

        if (!ValidateSignature(rStream, aSignatures[i], aInfo, nMDPPerm, aSignatureEOFs,
                               aTrailerEnds))
        {
            SAL_WARN("xmlsecurity.helper", "failed to determine digest match");
        }

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
        if (rInternal.GetSigningCertificate()
            && !rInternal.GetSigningCertificate()->X509Certificate.isEmpty())
        {
            rExternal.Signer = xSecEnv->createCertificateFromAscii(
                rInternal.GetSigningCertificate()->X509Certificate);
        }
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
