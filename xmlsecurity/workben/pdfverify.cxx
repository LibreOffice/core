/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <iostream>

#ifdef WNT
#include <prewin.h>
#endif
#include <fpdfview.h>
#include <fpdf_edit.h>
#ifdef WNT
#include <postwin.h>
#endif

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/xml/crypto/SEInitializer.hpp>

#include <comphelper/processfactory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <osl/file.hxx>
#include <sal/main.h>
#include <vcl/bitmap.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/pngwrite.hxx>
#include <vcl/svapp.hxx>

#include <pdfio/pdfdocument.hxx>

using namespace com::sun::star;

namespace
{
/// Convert to inch, then assume 96 DPI.
double pointToPixel(double fPoint)
{
    return fPoint / 72 * 96;
}

/// Does PDF to PNG conversion using pdfium.
void generatePreview(const OString& rPdfPath, const OString& rPngPath)
{
    FPDF_LIBRARY_CONFIG aConfig;
    aConfig.version = 2;
    aConfig.m_pUserFontPaths = nullptr;
    aConfig.m_pIsolate = nullptr;
    aConfig.m_v8EmbedderSlot = 0;
    FPDF_InitLibraryWithConfig(&aConfig);

    // Read input into a buffer.
    OUString aInURL;
    osl::FileBase::getFileURLFromSystemPath(OUString::fromUtf8(rPdfPath), aInURL);
    SvFileStream aInStream(aInURL, StreamMode::READ);
    SvMemoryStream aInBuffer;
    aInBuffer.WriteStream(aInStream);

    // Load the buffer using pdfium.
    FPDF_DOCUMENT pPdfDocument = FPDF_LoadMemDocument(aInBuffer.GetData(), aInBuffer.GetSize(), /*password=*/nullptr);
    if (!pPdfDocument)
        return;

    // Render the first page.
    FPDF_PAGE pPdfPage = FPDF_LoadPage(pPdfDocument, /*page_index=*/0);
    if (!pPdfPage)
        return;

    // Returned unit is points, convert that to pixel.
    int nPageWidth = pointToPixel(FPDF_GetPageWidth(pPdfPage));
    int nPageHeight = pointToPixel(FPDF_GetPageHeight(pPdfPage));
    FPDF_BITMAP pPdfBitmap = FPDFBitmap_Create(nPageWidth, nPageHeight, /*alpha=*/1);
    if (!pPdfBitmap)
        return;

    FPDF_DWORD nColor = FPDFPage_HasTransparency(pPdfPage) ? 0x00000000 : 0xFFFFFFFF;
    FPDFBitmap_FillRect(pPdfBitmap, 0, 0, nPageWidth, nPageHeight, nColor);
    FPDF_RenderPageBitmap(pPdfBitmap, pPdfPage, /*start_x=*/0, /*start_y=*/0, nPageWidth, nPageHeight, /*rotate=*/0, /*flags=*/0);

    // Save the buffer as a PNG file.
    Bitmap aBitmap(Size(nPageWidth, nPageHeight), 32);
    {
        Bitmap::ScopedWriteAccess pWriteAccess(aBitmap);
        const char* pPdfBuffer = static_cast<const char*>(FPDFBitmap_GetBuffer(pPdfBitmap));
#ifndef MACOSX
        std::memcpy(pWriteAccess->GetBuffer(), pPdfBuffer, nPageWidth * nPageHeight * 4);
#else
        // ARGB -> BGRA
        for (int nRow = 0; nRow < nPageHeight; ++nRow)
        {
            int nStride = FPDFBitmap_GetStride(pPdfBitmap);
            const char* pPdfLine = pPdfBuffer + (nStride * nRow);
            Scanline pRow = pWriteAccess->GetBuffer() + (nPageWidth * nRow * 4);
            for (int nCol = 0; nCol < nPageWidth; ++nCol)
            {
                pRow[nCol * 4] = pPdfLine[(nCol * 4) + 3];
                pRow[(nCol * 4) + 1] = pPdfLine[(nCol * 4) + 2];
                pRow[(nCol * 4) + 2] = pPdfLine[(nCol * 4) + 1];
                pRow[(nCol * 4) + 3] = pPdfLine[nCol * 4];
            }
        }
#endif
    }
    BitmapEx aBitmapEx(aBitmap);
#if defined(WNT) || defined(MACOSX)
    aBitmapEx.Mirror(BmpMirrorFlags::Vertical);
#endif
    vcl::PNGWriter aWriter(aBitmapEx);
    OUString aOutURL;
    osl::FileBase::getFileURLFromSystemPath(OUString::fromUtf8(rPngPath), aOutURL);
    SvFileStream aOutStream(aOutURL, StreamMode::WRITE);
    aWriter.Write(aOutStream);

    FPDFBitmap_Destroy(pPdfBitmap);
    FPDF_ClosePage(pPdfPage);
    FPDF_CloseDocument(pPdfDocument);
    FPDF_DestroyLibrary();
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
    xmlsecurity::pdfio::PDFDocument aDocument;
    if (!aDocument.Read(aStream))
    {
        SAL_WARN("xmlsecurity.pdfio", "failed to read the document");
        return 1;
    }

    if (bRemoveSignature)
    {
        std::cerr << "removing the last signature" << std::endl;
        std::vector<xmlsecurity::pdfio::PDFObjectElement*> aSignatures = aDocument.GetSignatureWidgets();
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
        std::vector<xmlsecurity::pdfio::PDFObjectElement*> aSignatures = aDocument.GetSignatureWidgets();
        if (aSignatures.empty())
            std::cerr << "found no signatures" << std::endl;
        else
        {
            std::cerr << "found " << aSignatures.size() << " signatures" << std::endl;
            for (size_t i = 0; i < aSignatures.size(); ++i)
            {
                SignatureInformation aInfo(i);
                bool bLast = i == aSignatures.size() - 1;
                if (!xmlsecurity::pdfio::PDFDocument::ValidateSignature(aStream, aSignatures[i], aInfo, bLast))
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
