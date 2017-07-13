/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <iostream>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/xml/crypto/SEInitializer.hpp>

#include <comphelper/processfactory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <osl/file.hxx>
#include <sal/main.h>
#include <vcl/pngwrite.hxx>
#include <vcl/svapp.hxx>
#include <vcl/graphicfilter.hxx>

#include <xmlsecurity/pdfio/pdfdocument.hxx>

#include <sigstruct.hxx>

using namespace com::sun::star;

namespace
{
/// Does PDF to PNG conversion using pdfium.
void generatePreview(const OString& rPdfPath, const OString& rPngPath)
{
    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
    Graphic aGraphic;
    OUString aInURL;
    osl::FileBase::getFileURLFromSystemPath(OUString::fromUtf8(rPdfPath), aInURL);
    SvFileStream aInStream(aInURL, StreamMode::READ);
    WmfExternal* pExtHeader = nullptr;
    if (rFilter.ImportGraphic(aGraphic, OUString(), aInStream, GRFILTER_FORMAT_DONTKNOW, nullptr, GraphicFilterImportFlags::NONE, pExtHeader) != ERRCODE_NONE)
        return;

    BitmapEx aBitmapEx = aGraphic.GetBitmapEx();
    vcl::PNGWriter aWriter(aBitmapEx);
    OUString aOutURL;
    osl::FileBase::getFileURLFromSystemPath(OUString::fromUtf8(rPngPath), aOutURL);
    SvFileStream aOutStream(aOutURL, StreamMode::WRITE);
    aWriter.Write(aOutStream);
}

int pdfVerify(int nArgc, char** pArgv)
{
    if (nArgc < 2)
    {
        SAL_WARN("xmlsecurity.pdfio", "not enough parameters");
        return 1;
    }

    // Initialize nss / mscrypto.
    uno::Reference<uno::XComponentContext> xComponentContext;
    try
    {
        xComponentContext = cppu::defaultBootstrap_InitialComponentContext();
    }
    catch (const uno::RuntimeException& rException)
    {
        SAL_WARN("xmlsecurity.pdfio", "cppu::defaultBootstrap_InitialComponentContext() failed: " << rException.Message);
        return 1;
    }
    uno::Reference<lang::XMultiComponentFactory> xMultiComponentFactory = xComponentContext->getServiceManager();
    uno::Reference<lang::XMultiServiceFactory> xMultiServiceFactory(xMultiComponentFactory, uno::UNO_QUERY);
    comphelper::setProcessServiceFactory(xMultiServiceFactory);

    if (nArgc > 3 && OString(pArgv[3]) == "-p")
    {
        InitVCL();
        generatePreview(pArgv[1], pArgv[2]);
        DeInitVCL();
        return 0;
    }

    uno::Reference<xml::crypto::XSEInitializer> xSEInitializer;
    try
    {
        xSEInitializer = xml::crypto::SEInitializer::create(xComponentContext);
    }
    catch (const uno::DeploymentException& rException)
    {
        SAL_WARN("xmlsecurity.pdfio", "DeploymentException while creating SEInitializer: " << rException.Message);
        return 1;
    }
    uno::Reference<xml::crypto::XXMLSecurityContext> xSecurityContext = xSEInitializer->createSecurityContext(OUString());

    OUString aInURL;
    osl::FileBase::getFileURLFromSystemPath(OUString::fromUtf8(pArgv[1]), aInURL);
    OUString aOutURL;
    if (nArgc > 2)
        osl::FileBase::getFileURLFromSystemPath(OUString::fromUtf8(pArgv[2]), aOutURL);

    bool bRemoveSignature = false;
    if (nArgc > 3 && OString(pArgv[3]) == "-r")
        bRemoveSignature = true;

    SvFileStream aStream(aInURL, StreamMode::READ);
    vcl::filter::PDFDocument aDocument;
    if (!aDocument.Read(aStream))
    {
        SAL_WARN("xmlsecurity.pdfio", "failed to read the document");
        return 1;
    }

    if (bRemoveSignature)
    {
        std::cerr << "removing the last signature" << std::endl;
        std::vector<vcl::filter::PDFObjectElement*> aSignatures = aDocument.GetSignatureWidgets();
        if (aSignatures.empty())
        {
            std::cerr << "found no signatures" << std::endl;
            return 1;
        }

        size_t nPosition = aSignatures.size() - 1;
        if (!aDocument.RemoveSignature(nPosition))
        {
            SAL_WARN("xmlsecurity.pdfio", "failed to remove signature #" << nPosition);
            return 1;
        }

        SvFileStream aOutStream(aOutURL, StreamMode::WRITE | StreamMode::TRUNC);
        if (!aDocument.Write(aOutStream))
        {
            SAL_WARN("xmlsecurity.pdfio", "failed to write the document");
            return 1;
        }

        return 0;
    }

    if (aOutURL.isEmpty())
    {
        std::cerr << "verifying signatures" << std::endl;
        std::vector<vcl::filter::PDFObjectElement*> aSignatures = aDocument.GetSignatureWidgets();
        if (aSignatures.empty())
            std::cerr << "found no signatures" << std::endl;
        else
        {
            std::cerr << "found " << aSignatures.size() << " signatures" << std::endl;
            for (size_t i = 0; i < aSignatures.size(); ++i)
            {
                SignatureInformation aInfo(i);
                bool bLast = i == aSignatures.size() - 1;
                if (!xmlsecurity::pdfio::ValidateSignature(aStream, aSignatures[i], aInfo, bLast))
                {
                    SAL_WARN("xmlsecurity.pdfio", "failed to determine digest match");
                    return 1;
                }

                bool bSuccess = aInfo.nStatus == xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED;
                std::cerr << "signature #" << i << ": digest match? " << bSuccess << std::endl;
            }
        }

        return 0;
    }

    std::cerr << "adding a new signature" << std::endl;
    uno::Reference<xml::crypto::XSecurityEnvironment> xSecurityEnvironment = xSecurityContext->getSecurityEnvironment();
    uno::Sequence<uno::Reference<security::XCertificate>> aCertificates = xSecurityEnvironment->getPersonalCertificates();
    if (!aCertificates.hasElements())
    {
        SAL_WARN("xmlsecurity.pdfio", "no signing certificates found");
        return 1;
    }
    if (!aDocument.Sign(aCertificates[0], "pdfverify", /*bAdES=*/true))
    {
        SAL_WARN("xmlsecurity.pdfio", "failed to sign");
        return 1;
    }

    SvFileStream aOutStream(aOutURL, StreamMode::WRITE | StreamMode::TRUNC);
    if (!aDocument.Write(aOutStream))
    {
        SAL_WARN("xmlsecurity.pdfio", "failed to write the document");
        return 1;
    }

    return 0;
}
}

SAL_IMPLEMENT_MAIN_WITH_ARGS(nArgc, pArgv)
{
    try
    {
        return pdfVerify(nArgc, pArgv);
    }
    catch (...)
    {
        std::cerr << "pdfverify: uncaught exception while invoking pdfVerify()" << std::endl;
        return 1;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
