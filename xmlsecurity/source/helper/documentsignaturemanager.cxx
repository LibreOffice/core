/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <documentsignaturemanager.hxx>
#include <config_gpgme.h>

#include <gpg/SEInitializer.hxx>

#include <com/sun/star/embed/StorageFormats.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/xml/crypto/SEInitializer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <comphelper/base64.hxx>
#include <comphelper/storagehelper.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <tools/datetime.hxx>
#include <o3tl/make_unique.hxx>

#include <certificate.hxx>
#include <biginteger.hxx>

#include <xmlsec/xmlsec_init.hxx>

#include <pdfsignaturehelper.hxx>

using namespace css;
using namespace css::graphic;
using namespace css::uno;

DocumentSignatureManager::DocumentSignatureManager(
    const uno::Reference<uno::XComponentContext>& xContext, DocumentSignatureMode eMode)
    : mxContext(xContext)
    , maSignatureHelper(xContext)
    , meSignatureMode(eMode)
{
}

DocumentSignatureManager::~DocumentSignatureManager() { deInitXmlSec(); }

bool DocumentSignatureManager::init()
{
    SAL_WARN_IF(mxSEInitializer.is(), "xmlsecurity.helper",
                "DocumentSignatureManager::Init - mxSEInitializer already set!");
    SAL_WARN_IF(mxSecurityContext.is(), "xmlsecurity.helper",
                "DocumentSignatureManager::Init - mxSecurityContext already set!");
    SAL_WARN_IF(mxGpgSEInitializer.is(), "xmlsecurity.helper",
                "DocumentSignatureManager::Init - mxGpgSEInitializer already set!");

    // xmlsec is needed by both services, so init before those
    initXmlSec();

    mxSEInitializer = xml::crypto::SEInitializer::create(mxContext);
#if HAVE_FEATURE_GPGME
    mxGpgSEInitializer.set(new SEInitializerGpg());
#endif

    if (mxSEInitializer.is())
        mxSecurityContext = mxSEInitializer->createSecurityContext(OUString());

#if HAVE_FEATURE_GPGME
    if (mxGpgSEInitializer.is())
        mxGpgSecurityContext = mxGpgSEInitializer->createSecurityContext(OUString());

    return mxSecurityContext.is() || mxGpgSecurityContext.is();
#else
    return mxSecurityContext.is();
#endif
}

PDFSignatureHelper& DocumentSignatureManager::getPDFSignatureHelper()
{
    bool bInit = true;
    if (!mxSecurityContext.is())
        bInit = init();

    SAL_WARN_IF(!bInit, "xmlsecurity.comp", "Error initializing security context!");

    if (!mpPDFSignatureHelper)
        mpPDFSignatureHelper = o3tl::make_unique<PDFSignatureHelper>();

    return *mpPDFSignatureHelper;
}

#if 0 // For some reason does not work
bool DocumentSignatureManager::IsXAdESRelevant()
{
    if (mxStore.is())
    {
        // ZIP-based: ODF or OOXML.
        maSignatureHelper.StartMission();

        SignatureStreamHelper aStreamHelper = ImplOpenSignatureStream(embed::ElementModes::READ, /*bUseTempStream=*/true);
        if (aStreamHelper.nStorageFormat == embed::StorageFormats::OFOPXML)
        {
            maSignatureHelper.EndMission();
            return false;
        }
        // FIXME: How to figure out if it is ODF 1.2?
        maSignatureHelper.EndMission();
        return true;
    }
    return false;
}
#endif

/* Using the zip storage, we cannot get the properties "MediaType" and "IsEncrypted"
    We use the manifest to find out if a file is xml and if it is encrypted.
    The parameter is an encoded uri. However, the manifest contains paths. Therefore
    the path is encoded as uri, so they can be compared.
*/
bool DocumentSignatureManager::isXML(const OUString& rURI)
{
    SAL_WARN_IF(!mxStore.is(), "xmlsecurity.helper", "empty storage reference");

    bool bIsXML = false;
    bool bPropsAvailable = false;
    const OUString sPropFullPath("FullPath");
    const OUString sPropMediaType("MediaType");
    const OUString sPropDigest("Digest");

    for (int i = 0; i < m_manifest.getLength(); i++)
    {
        const uno::Sequence<beans::PropertyValue>& entry = m_manifest[i];
        OUString sPath, sMediaType;
        bool bEncrypted = false;
        for (int j = 0; j < entry.getLength(); j++)
        {
            const beans::PropertyValue& prop = entry[j];

            if (prop.Name == sPropFullPath)
                prop.Value >>= sPath;
            else if (prop.Name == sPropMediaType)
                prop.Value >>= sMediaType;
            else if (prop.Name == sPropDigest)
                bEncrypted = true;
        }
        if (DocumentSignatureHelper::equalsReferenceUriManifestPath(rURI, sPath))
        {
            bIsXML = sMediaType == "text/xml" && !bEncrypted;
            bPropsAvailable = true;
            break;
        }
    }
    if (!bPropsAvailable)
    {
        //This would be the case for at least mimetype, META-INF/manifest.xml
        //META-INF/macrosignatures.xml.
        //Files can only be encrypted if they are in the manifest.xml.
        //That is, the current file cannot be encrypted, otherwise bPropsAvailable
        //would be true.
        sal_Int32 nSep = rURI.lastIndexOf('.');
        if (nSep != -1)
        {
            OUString aExt = rURI.copy(nSep + 1);
            if (aExt.equalsIgnoreAsciiCase("XML"))
                bIsXML = true;
        }
    }
    return bIsXML;
}

//If bTempStream is true, then a temporary stream is return. If it is false then, the actual
//signature stream is used.
//Every time the user presses Add a new temporary stream is created.
//We keep the temporary stream as member because ImplGetSignatureInformations
//will later access the stream to create DocumentSignatureInformation objects
//which are stored in maCurrentSignatureInformations.
SignatureStreamHelper DocumentSignatureManager::ImplOpenSignatureStream(sal_Int32 nStreamOpenMode,
                                                                        bool bTempStream)
{
    SignatureStreamHelper aHelper;
    if (mxStore.is())
    {
        uno::Reference<container::XNameAccess> xNameAccess(mxStore, uno::UNO_QUERY);
        if (xNameAccess.is() && xNameAccess->hasByName("[Content_Types].xml"))
            aHelper.nStorageFormat = embed::StorageFormats::OFOPXML;
    }

    if (bTempStream)
    {
        if (nStreamOpenMode & embed::ElementModes::TRUNCATE)
        {
            //We write always into a new temporary stream.
            mxTempSignatureStream.set(io::TempFile::create(mxContext), uno::UNO_QUERY_THROW);
            if (aHelper.nStorageFormat != embed::StorageFormats::OFOPXML)
                aHelper.xSignatureStream = mxTempSignatureStream;
            else
            {
                mxTempSignatureStorage = comphelper::OStorageHelper::GetStorageOfFormatFromStream(
                    ZIP_STORAGE_FORMAT_STRING, mxTempSignatureStream);
                aHelper.xSignatureStorage = mxTempSignatureStorage;
            }
        }
        else
        {
            //When we read from the temp stream, then we must have previously
            //created one.
            SAL_WARN_IF(!mxTempSignatureStream.is(), "xmlsecurity.helper",
                        "empty temp. signature stream reference");
        }
        aHelper.xSignatureStream = mxTempSignatureStream;
        if (aHelper.nStorageFormat == embed::StorageFormats::OFOPXML)
            aHelper.xSignatureStorage = mxTempSignatureStorage;
    }
    else
    {
        //No temporary stream
        if (!mxSignatureStream.is())
        {
            //We may not have a dedicated stream for writing the signature
            //So we take one directly from the storage
            //Or DocumentDigitalSignatures::showDocumentContentSignatures was called,
            //in which case Add/Remove is not allowed. This is done, for example, if the
            //document is readonly
            aHelper = DocumentSignatureHelper::OpenSignatureStream(mxStore, nStreamOpenMode,
                                                                   meSignatureMode);
        }
        else
        {
            aHelper.xSignatureStream = mxSignatureStream;
        }
    }

    if (nStreamOpenMode & embed::ElementModes::TRUNCATE)
    {
        if (aHelper.xSignatureStream.is()
            && aHelper.nStorageFormat != embed::StorageFormats::OFOPXML)
        {
            uno::Reference<io::XTruncate> xTruncate(aHelper.xSignatureStream, uno::UNO_QUERY_THROW);
            xTruncate->truncate();
        }
    }
    else if (bTempStream || mxSignatureStream.is())
    {
        //In case we read the signature stream from the storage directly,
        //which is the case when DocumentDigitalSignatures::showDocumentContentSignatures
        //then XSeakable is not supported
        uno::Reference<io::XSeekable> xSeek(aHelper.xSignatureStream, uno::UNO_QUERY_THROW);
        xSeek->seek(0);
    }

    return aHelper;
}

bool DocumentSignatureManager::add(
    const uno::Reference<security::XCertificate>& xCert,
    const uno::Reference<xml::crypto::XXMLSecurityContext>& xSecurityContext,
    const OUString& rDescription, sal_Int32& nSecurityId, bool bAdESCompliant,
    const OUString& rSignatureLineId, const Reference<XGraphic>& xValidGraphic,
    const Reference<XGraphic>& xInvalidGraphic)
{
    if (!xCert.is())
    {
        SAL_WARN("xmlsecurity.helper", "no certificate selected");
        return false;
    }

    // GPG or X509 key?
    uno::Reference<lang::XServiceInfo> xServiceInfo(xSecurityContext, uno::UNO_QUERY);
    if (xServiceInfo->getImplementationName()
        == "com.sun.star.xml.security.gpg.XMLSecurityContext_GpgImpl")
    {
        // GPG keys only really have PGPKeyId and PGPKeyPacket
        if (!mxStore.is())
        {
            SAL_WARN("xmlsecurity.helper", "cannot sign pdfs with GPG keys");
            return false;
        }

        maSignatureHelper.StartMission(xSecurityContext);

        nSecurityId = maSignatureHelper.GetNewSecurityId();

        OUStringBuffer aStrBuffer;
        comphelper::Base64::encode(aStrBuffer, xCert->getEncoded());

        OUString aKeyId;
        if (auto pCertificate = dynamic_cast<xmlsecurity::Certificate*>(xCert.get()))
        {
            OUStringBuffer aBuffer;
            comphelper::Base64::encode(aBuffer, pCertificate->getSHA256Thumbprint());
            aKeyId = aBuffer.makeStringAndClear();
        }
        else
            SAL_WARN("xmlsecurity.helper",
                     "XCertificate implementation without an xmlsecurity::Certificate one");

        maSignatureHelper.SetGpgCertificate(nSecurityId, aKeyId, aStrBuffer.makeStringAndClear(),
                                            xCert->getIssuerName());
    }
    else
    {
        OUString aCertSerial = xmlsecurity::bigIntegerToNumericString(xCert->getSerialNumber());
        if (aCertSerial.isEmpty())
        {
            SAL_WARN("xmlsecurity.helper", "Error in Certificate, problem with serial number!");
            return false;
        }

        if (!mxStore.is())
        {
            // Something not ZIP based, try PDF.
            nSecurityId = getPDFSignatureHelper().GetNewSecurityId();
            getPDFSignatureHelper().SetX509Certificate(xCert);
            getPDFSignatureHelper().SetDescription(rDescription);
            uno::Reference<io::XInputStream> xInputStream(mxSignatureStream, uno::UNO_QUERY);
            if (!getPDFSignatureHelper().Sign(xInputStream, bAdESCompliant))
            {
                SAL_WARN("xmlsecurity.helper", "PDFSignatureHelper::Sign() failed");
                return false;
            }
            return true;
        }

        maSignatureHelper.StartMission(xSecurityContext);

        nSecurityId = maSignatureHelper.GetNewSecurityId();

        OUStringBuffer aStrBuffer;
        comphelper::Base64::encode(aStrBuffer, xCert->getEncoded());

        OUString aCertDigest;
        svl::crypto::SignatureMethodAlgorithm eAlgorithmID
            = svl::crypto::SignatureMethodAlgorithm::RSA;
        if (auto pCertificate = dynamic_cast<xmlsecurity::Certificate*>(xCert.get()))
        {
            OUStringBuffer aBuffer;
            comphelper::Base64::encode(aBuffer, pCertificate->getSHA256Thumbprint());
            aCertDigest = aBuffer.makeStringAndClear();

            eAlgorithmID = pCertificate->getSignatureMethodAlgorithm();
        }
        else
            SAL_WARN("xmlsecurity.helper",
                     "XCertificate implementation without an xmlsecurity::Certificate one");

        maSignatureHelper.SetX509Certificate(nSecurityId, xCert->getIssuerName(), aCertSerial,
                                             aStrBuffer.makeStringAndClear(), aCertDigest,
                                             eAlgorithmID);
    }

    uno::Sequence<uno::Reference<security::XCertificate>> aCertPath
        = xSecurityContext->getSecurityEnvironment()->buildCertificatePath(xCert);
    const uno::Reference<security::XCertificate>* pCertPath = aCertPath.getConstArray();
    sal_Int32 nCnt = aCertPath.getLength();

    OUStringBuffer aStrBuffer;
    for (int i = 0; i < nCnt; i++)
    {
        comphelper::Base64::encode(aStrBuffer, pCertPath[i]->getEncoded());
        maSignatureHelper.AddEncapsulatedX509Certificate(aStrBuffer.makeStringAndClear());
    }

    std::vector<OUString> aElements = DocumentSignatureHelper::CreateElementList(
        mxStore, meSignatureMode, DocumentSignatureAlgorithm::OOo3_2);
    DocumentSignatureHelper::AppendContentTypes(mxStore, aElements);

    sal_Int32 nElements = aElements.size();
    for (sal_Int32 n = 0; n < nElements; n++)
    {
        bool bBinaryMode = !isXML(aElements[n]);
        maSignatureHelper.AddForSigning(nSecurityId, aElements[n], bBinaryMode, bAdESCompliant);
    }

    maSignatureHelper.SetDateTime(nSecurityId, DateTime(DateTime::SYSTEM));
    maSignatureHelper.SetDescription(nSecurityId, rDescription);

    if (!rSignatureLineId.isEmpty())
        maSignatureHelper.SetSignatureLineId(nSecurityId, rSignatureLineId);

    if (xValidGraphic.is())
        maSignatureHelper.SetSignatureLineValidGraphic(nSecurityId, xValidGraphic);

    if (xInvalidGraphic.is())
        maSignatureHelper.SetSignatureLineInvalidGraphic(nSecurityId, xInvalidGraphic);

    // We open a signature stream in which the existing and the new
    //signature is written. ImplGetSignatureInformation (later in this function) will
    //then read the stream and fill maCurrentSignatureInformations. The final signature
    //is written when the user presses OK. Then only maCurrentSignatureInformation and
    //a sax writer are used to write the information.
    SignatureStreamHelper aStreamHelper
        = ImplOpenSignatureStream(embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE, true);

    if (aStreamHelper.nStorageFormat != embed::StorageFormats::OFOPXML)
    {
        uno::Reference<io::XOutputStream> xOutputStream(aStreamHelper.xSignatureStream,
                                                        uno::UNO_QUERY_THROW);
        uno::Reference<xml::sax::XWriter> xSaxWriter
            = maSignatureHelper.CreateDocumentHandlerWithHeader(xOutputStream);

        // Export old signatures...
        uno::Reference<xml::sax::XDocumentHandler> xDocumentHandler(xSaxWriter,
                                                                    uno::UNO_QUERY_THROW);
        std::size_t nInfos = maCurrentSignatureInformations.size();
        for (std::size_t n = 0; n < nInfos; n++)
            XMLSignatureHelper::ExportSignature(xDocumentHandler, maCurrentSignatureInformations[n],
                                                bAdESCompliant);

        // Create a new one...
        maSignatureHelper.CreateAndWriteSignature(xDocumentHandler, bAdESCompliant);

        // That's it...
        XMLSignatureHelper::CloseDocumentHandler(xDocumentHandler);
    }
    else
    {
        // OOXML

        // Handle relations.
        maSignatureHelper.EnsureSignaturesRelation(mxStore, /*bAdd=*/true);
        // Old signatures + the new one.
        int nSignatureCount = maCurrentSignatureInformations.size() + 1;
        maSignatureHelper.ExportSignatureRelations(aStreamHelper.xSignatureStorage,
                                                   nSignatureCount);

        // Export old signatures.
        for (std::size_t i = 0; i < maCurrentSignatureInformations.size(); ++i)
            maSignatureHelper.ExportOOXMLSignature(mxStore, aStreamHelper.xSignatureStorage,
                                                   maCurrentSignatureInformations[i], i + 1);

        // Create a new signature.
        maSignatureHelper.CreateAndWriteOOXMLSignature(mxStore, aStreamHelper.xSignatureStorage,
                                                       nSignatureCount);

        // Flush objects.
        uno::Reference<embed::XTransactedObject> xTransact(aStreamHelper.xSignatureStorage,
                                                           uno::UNO_QUERY);
        xTransact->commit();
        uno::Reference<io::XOutputStream> xOutputStream(aStreamHelper.xSignatureStream,
                                                        uno::UNO_QUERY);
        xOutputStream->closeOutput();

        uno::Reference<io::XTempFile> xTempFile(aStreamHelper.xSignatureStream, uno::UNO_QUERY);
        SAL_INFO("xmlsecurity.helper",
                 "DocumentSignatureManager::add temporary storage at " << xTempFile->getUri());
    }

    maSignatureHelper.EndMission();
    return true;
}

void DocumentSignatureManager::remove(sal_uInt16 nPosition)
{
    if (!mxStore.is())
    {
        // Something not ZIP based, try PDF.
        uno::Reference<io::XInputStream> xInputStream(mxSignatureStream, uno::UNO_QUERY);
        if (!PDFSignatureHelper::RemoveSignature(xInputStream, nPosition))
        {
            SAL_WARN("xmlsecurity.helper", "PDFSignatureHelper::RemoveSignature() failed");
            return;
        }

        // Only erase when the removal was successful, it may fail for PDF.
        // Also, erase the requested and all following signatures, as PDF signatures are always chained.
        maCurrentSignatureInformations.erase(maCurrentSignatureInformations.begin() + nPosition,
                                             maCurrentSignatureInformations.end());
        return;
    }

    maCurrentSignatureInformations.erase(maCurrentSignatureInformations.begin() + nPosition);

    // Export all other signatures...
    SignatureStreamHelper aStreamHelper = ImplOpenSignatureStream(
        embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE, /*bTempStream=*/true);

    if (aStreamHelper.nStorageFormat != embed::StorageFormats::OFOPXML)
    {
        uno::Reference<io::XOutputStream> xOutputStream(aStreamHelper.xSignatureStream,
                                                        uno::UNO_QUERY_THROW);
        uno::Reference<xml::sax::XWriter> xSaxWriter
            = maSignatureHelper.CreateDocumentHandlerWithHeader(xOutputStream);

        uno::Reference<xml::sax::XDocumentHandler> xDocumentHandler(xSaxWriter,
                                                                    uno::UNO_QUERY_THROW);
        std::size_t nInfos = maCurrentSignatureInformations.size();
        for (std::size_t n = 0; n < nInfos; ++n)
            XMLSignatureHelper::ExportSignature(xDocumentHandler, maCurrentSignatureInformations[n],
                                                false /* ??? */);

        XMLSignatureHelper::CloseDocumentHandler(xDocumentHandler);
    }
    else
    {
        // OOXML

        // Handle relations.
        int nSignatureCount = maCurrentSignatureInformations.size();
        maSignatureHelper.ExportSignatureRelations(aStreamHelper.xSignatureStorage,
                                                   nSignatureCount);

        // Export old signatures.
        for (std::size_t i = 0; i < maCurrentSignatureInformations.size(); ++i)
            maSignatureHelper.ExportOOXMLSignature(mxStore, aStreamHelper.xSignatureStorage,
                                                   maCurrentSignatureInformations[i], i + 1);

        // Flush objects.
        uno::Reference<embed::XTransactedObject> xTransact(aStreamHelper.xSignatureStorage,
                                                           uno::UNO_QUERY);
        xTransact->commit();
        uno::Reference<io::XOutputStream> xOutputStream(aStreamHelper.xSignatureStream,
                                                        uno::UNO_QUERY);
        xOutputStream->closeOutput();

        uno::Reference<io::XTempFile> xTempFile(aStreamHelper.xSignatureStream, uno::UNO_QUERY);
        SAL_INFO("xmlsecurity.helper", "DocumentSignatureManager::remove: temporary storage is at "
                                           << xTempFile->getUri());
    }
}

void DocumentSignatureManager::read(bool bUseTempStream, bool bCacheLastSignature)
{
    maCurrentSignatureInformations.clear();

    if (mxStore.is())
    {
        // ZIP-based: ODF or OOXML.
        maSignatureHelper.StartMission(mxSecurityContext);

        SignatureStreamHelper aStreamHelper
            = ImplOpenSignatureStream(embed::ElementModes::READ, bUseTempStream);
        if (aStreamHelper.nStorageFormat != embed::StorageFormats::OFOPXML
            && aStreamHelper.xSignatureStream.is())
        {
            uno::Reference<io::XInputStream> xInputStream(aStreamHelper.xSignatureStream,
                                                          uno::UNO_QUERY);
            maSignatureHelper.ReadAndVerifySignature(xInputStream);
        }
        else if (aStreamHelper.nStorageFormat == embed::StorageFormats::OFOPXML
                 && aStreamHelper.xSignatureStorage.is())
            maSignatureHelper.ReadAndVerifySignatureStorage(aStreamHelper.xSignatureStorage,
                                                            bCacheLastSignature);
        maSignatureHelper.EndMission();

        maCurrentSignatureInformations = maSignatureHelper.GetSignatureInformations();
    }
    else
    {
        // Something not ZIP based, try PDF.
        uno::Reference<io::XInputStream> xInputStream(mxSignatureStream, uno::UNO_QUERY);
        if (getPDFSignatureHelper().ReadAndVerifySignature(xInputStream))
            maCurrentSignatureInformations = getPDFSignatureHelper().GetSignatureInformations();
    }
}

void DocumentSignatureManager::write(bool bXAdESCompliantIfODF)
{
    if (!mxStore.is())
    {
        // Something not ZIP based, assume PDF, which is written directly in add() already.
        return;
    }

    // Export all other signatures...
    SignatureStreamHelper aStreamHelper = ImplOpenSignatureStream(
        embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE, false);

    if (aStreamHelper.xSignatureStream.is()
        && aStreamHelper.nStorageFormat != embed::StorageFormats::OFOPXML)
    {
        // ODF
        uno::Reference<io::XOutputStream> xOutputStream(aStreamHelper.xSignatureStream,
                                                        uno::UNO_QUERY);
        uno::Reference<xml::sax::XWriter> xSaxWriter
            = maSignatureHelper.CreateDocumentHandlerWithHeader(xOutputStream);

        uno::Reference<xml::sax::XDocumentHandler> xDocumentHandler(xSaxWriter,
                                                                    uno::UNO_QUERY_THROW);
        std::size_t nInfos = maCurrentSignatureInformations.size();
        for (std::size_t n = 0; n < nInfos; ++n)
            XMLSignatureHelper::ExportSignature(xDocumentHandler, maCurrentSignatureInformations[n],
                                                bXAdESCompliantIfODF);

        XMLSignatureHelper::CloseDocumentHandler(xDocumentHandler);
    }
    else if (aStreamHelper.xSignatureStorage.is()
             && aStreamHelper.nStorageFormat == embed::StorageFormats::OFOPXML)
    {
        // OOXML
        std::size_t nSignatureCount = maCurrentSignatureInformations.size();
        maSignatureHelper.ExportSignatureContentTypes(mxStore, nSignatureCount);
        if (nSignatureCount > 0)
            maSignatureHelper.ExportSignatureRelations(aStreamHelper.xSignatureStorage,
                                                       nSignatureCount);
        else
        {
            // Removing all signatures: then need to remove the signature relation as well.
            maSignatureHelper.EnsureSignaturesRelation(mxStore, /*bAdd=*/false);
            // Also remove the whole signature sub-storage: release our read-write reference + remove the element.
            aStreamHelper = SignatureStreamHelper();
            mxStore->removeElement("_xmlsignatures");
        }

        for (std::size_t i = 0; i < nSignatureCount; ++i)
            maSignatureHelper.ExportOOXMLSignature(mxStore, aStreamHelper.xSignatureStorage,
                                                   maCurrentSignatureInformations[i], i + 1);
    }

    // If stream was not provided, we are responsible for committing it....
    if (!mxSignatureStream.is() && aStreamHelper.xSignatureStorage.is())
    {
        uno::Reference<embed::XTransactedObject> xTrans(aStreamHelper.xSignatureStorage,
                                                        uno::UNO_QUERY);
        xTrans->commit();
    }
}

uno::Reference<xml::crypto::XSecurityEnvironment> DocumentSignatureManager::getSecurityEnvironment()
{
    return mxSecurityContext.is() ? mxSecurityContext->getSecurityEnvironment()
                                  : uno::Reference<xml::crypto::XSecurityEnvironment>();
}

uno::Reference<xml::crypto::XSecurityEnvironment>
DocumentSignatureManager::getGpgSecurityEnvironment()
{
    return mxGpgSecurityContext.is() ? mxGpgSecurityContext->getSecurityEnvironment()
                                     : uno::Reference<xml::crypto::XSecurityEnvironment>();
}

uno::Reference<xml::crypto::XXMLSecurityContext> const&
DocumentSignatureManager::getSecurityContext()
{
    return mxSecurityContext;
}

uno::Reference<xml::crypto::XXMLSecurityContext> const&
DocumentSignatureManager::getGpgSecurityContext()
{
    return mxGpgSecurityContext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
