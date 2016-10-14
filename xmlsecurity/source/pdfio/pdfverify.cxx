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

#include <pdfio/pdfdocument.hxx>

using namespace com::sun::star;

SAL_IMPLEMENT_MAIN_WITH_ARGS(nArgc, pArgv)
{
    if (nArgc < 2)
    {
        SAL_WARN("xmlsecurity.pdfio", "not enough parameters");
        return 1;
    }

    // Initialize nss / mscrypto.
    uno::Reference<uno::XComponentContext> xComponentContext = cppu::defaultBootstrap_InitialComponentContext();
    uno::Reference<lang::XMultiComponentFactory> xMultiComponentFactory = xComponentContext->getServiceManager();
    uno::Reference<lang::XMultiServiceFactory> xMultiServiceFactory(xMultiComponentFactory, uno::UNO_QUERY);;
    comphelper::setProcessServiceFactory(xMultiServiceFactory);
    uno::Reference<xml::crypto::XSEInitializer> xSEInitializer = xml::crypto::SEInitializer::create(xComponentContext);
    uno::Reference<xml::crypto::XXMLSecurityContext> xSecurityContext = xSEInitializer->createSecurityContext(OUString());

    OUString aURL;
    osl::FileBase::getFileURLFromSystemPath(OUString::fromUtf8(pArgv[1]), aURL);

    SvFileStream aStream(aURL, StreamMode::READ);
    xmlsecurity::pdfio::PDFDocument aDocument;
    if (!aDocument.Read(aStream))
    {
        SAL_WARN("xmlsecurity.pdfio", "failed to read the document");
        return 1;
    }

    std::vector<xmlsecurity::pdfio::PDFObjectElement*> aSignatures = aDocument.GetSignatureWidgets();
    if (aSignatures.empty())
        std::cerr << "found no signatures" << std::endl;
    else
    {
        std::cerr << "found " << aSignatures.size() << " signatures" << std::endl;
        for (size_t i = 0; i < aSignatures.size(); ++i)
        {
            SignatureInformation aInfo(i);
            if (!xmlsecurity::pdfio::PDFDocument::ValidateSignature(aStream, aSignatures[i], aInfo))
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
