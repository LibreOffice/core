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

#include <com/sun/star/xml/crypto/SEInitializer.hpp>

#include <comphelper/sequence.hxx>
#include <tools/stream.hxx>
#include <unotools/ucbstreamhelper.hxx>

#include <pdfio/pdfdocument.hxx>

using namespace ::com::sun::star;

PDFSignatureHelper::PDFSignatureHelper(const uno::Reference<uno::XComponentContext>& xComponentContext)
    : m_xComponentContext(xComponentContext)
{
    m_xSEInitializer = xml::crypto::SEInitializer::create(m_xComponentContext);
    if (m_xSEInitializer.is())
        // This initializes nss / mscrypto.
        m_xSecurityContext = m_xSEInitializer->createSecurityContext(OUString());
}

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

    for (size_t i = 0; i < aSignatures.size(); ++i)
    {
        security::DocumentSignatureInformation aInfo;

        bool bDigestMatch;
        if (!xmlsecurity::pdfio::PDFDocument::ValidateSignature(*pStream, aSignatures[i], bDigestMatch))
        {
            SAL_WARN("xmlsecurity.helper", "failed to determine digest match");
            continue;
        }

        aInfo.SignatureIsValid = bDigestMatch;
        m_aSignatureInfos.push_back(aInfo);
    }

    return true;
}

uno::Sequence<security::DocumentSignatureInformation> PDFSignatureHelper::GetDocumentSignatureInformations()
{
    return comphelper::containerToSequence(m_aSignatureInfos);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
