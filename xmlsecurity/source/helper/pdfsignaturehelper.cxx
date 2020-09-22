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

#include <pdfio/pdfdocument.hxx>

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

        if (!xmlsecurity::pdfio::ValidateSignature(rStream, aSignatures[i], aInfo, aDocument))
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
