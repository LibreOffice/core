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
#include <com/sun/star/security/CertificateValidity.hpp>
#include <com/sun/star/uno/SecurityException.hpp>
#include <com/sun/star/xml/crypto/SEInitializer.hpp>

#include <comphelper/sequence.hxx>
#include <tools/stream.hxx>
#include <unotools/ucbstreamhelper.hxx>

#include <pdfio/pdfdocument.hxx>

using namespace ::com::sun::star;

PDFSignatureHelper::PDFSignatureHelper() = default;

bool PDFSignatureHelper::ReadAndVerifySignature(const uno::Reference<io::XInputStream>& xInputStream)
{
    if (!xInputStream.is())
    {
        SAL_WARN("xmlsecurity.helper", "input stream missing");
        return false;
    }

    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    xmlsecurity::pdfio::PDFDocument aDocument;
    if (!aDocument.Read(*pStream))
    {
        SAL_WARN("xmlsecurity.helper", "failed to read the document");
        return false;
    }

    std::vector<xmlsecurity::pdfio::PDFObjectElement*> aSignatures = aDocument.GetSignatureWidgets();
    if (aSignatures.empty())
        return true;

    m_aSignatureInfos.clear();

    for (size_t i = 0; i < aSignatures.size(); ++i)
    {
        SignatureInformation aInfo(i);

        bool bLast = i == aSignatures.size() - 1;
        if (!xmlsecurity::pdfio::PDFDocument::ValidateSignature(*pStream, aSignatures[i], aInfo, bLast))
            SAL_WARN("xmlsecurity.helper", "failed to determine digest match");

        m_aSignatureInfos.push_back(aInfo);
    }

    return true;
}

SignatureInformations PDFSignatureHelper::GetSignatureInformations() const
{
    return m_aSignatureInfos;
}

uno::Sequence<security::DocumentSignatureInformation> PDFSignatureHelper::GetDocumentSignatureInformations(const uno::Reference<xml::crypto::XSecurityEnvironment>& xSecEnv) const
{
    uno::Sequence<security::DocumentSignatureInformation> aRet(m_aSignatureInfos.size());

    for (size_t i = 0; i < m_aSignatureInfos.size(); ++i)
    {
        const SignatureInformation& rInternal = m_aSignatureInfos[i];
        security::DocumentSignatureInformation& rExternal = aRet[i];
        rExternal.SignatureIsValid = rInternal.nStatus == xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED;
        rExternal.Signer = xSecEnv->createCertificateFromAscii(rInternal.ouX509Certificate);
        rExternal.PartialDocumentSignature = rInternal.bPartialDocumentSignature;

        // Verify certificate.
        if (rExternal.Signer.is())
        {
            try
            {
                rExternal.CertificateStatus = xSecEnv->verifyCertificate(rExternal.Signer, {});
            }
            catch (const uno::SecurityException& rException)
            {
                SAL_WARN("xmlsecurity.helper", "failed to verify certificate: " << rException.Message);
                rExternal.CertificateStatus = security::CertificateValidity::INVALID;
            }
        }
        else
            rExternal.CertificateStatus = security::CertificateValidity::INVALID;
    }

    return aRet;
}

sal_Int32 PDFSignatureHelper::GetNewSecurityId() const
{
    return m_aSignatureInfos.size();
}

void PDFSignatureHelper::SetX509Certificate(const uno::Reference<security::XCertificate>& xCertificate)
{
    m_xCertificate = xCertificate;
}

void PDFSignatureHelper::SetDescription(const OUString& rDescription)
{
    m_aDescription = rDescription;
}

bool PDFSignatureHelper::Sign(const uno::Reference<io::XInputStream>& xInputStream, bool bAdES)
{
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    xmlsecurity::pdfio::PDFDocument aDocument;
    if (!aDocument.Read(*pStream))
    {
        SAL_WARN("xmlsecurity.helper", "failed to read the document");
        return false;
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

bool PDFSignatureHelper::RemoveSignature(const uno::Reference<io::XInputStream>& xInputStream, sal_uInt16 nPosition)
{
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(xInputStream, true));
    xmlsecurity::pdfio::PDFDocument aDocument;
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
