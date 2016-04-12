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


#include <xmlsecurity/xmlsignaturehelper.hxx>
#include <xmlsecurity/documentsignaturehelper.hxx>
#include "xsecctl.hxx"

#include "xmlsignaturehelper2.hxx"

#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <tools/datetime.hxx>

#include <xmloff/attrlist.hxx>

#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/xml/crypto/SEInitializer.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/StorageFormats.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/io/XSeekable.hpp>

#include <tools/date.hxx>
#include <tools/time.hxx>
#include <comphelper/ofopxmlhelper.hxx>
#include <comphelper/sequence.hxx>

#define TAG_DOCUMENTSIGNATURES  "document-signatures"
#define NS_DOCUMENTSIGNATURES   "http://openoffice.org/2004/documentsignatures"
#define NS_DOCUMENTSIGNATURES_ODF_1_2 "urn:oasis:names:tc:opendocument:xmlns:digitalsignature:1.0"
#define OOXML_SIGNATURE_ORIGIN "http://schemas.openxmlformats.org/package/2006/relationships/digital-signature/origin"
#define OOXML_SIGNATURE_SIGNATURE "http://schemas.openxmlformats.org/package/2006/relationships/digital-signature/signature"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

XMLSignatureHelper::XMLSignatureHelper( const uno::Reference< uno::XComponentContext >& rxCtx)
    : mxCtx(rxCtx), mbODFPre1_2(false)
{
    mpXSecController = new XSecController(rxCtx);
    mxSecurityController = mpXSecController;
    mbError = false;
}

XMLSignatureHelper::~XMLSignatureHelper()
{
}

bool XMLSignatureHelper::Init()
{
    DBG_ASSERT( !mxSEInitializer.is(), "XMLSignatureHelper::Init - mxSEInitializer already set!" );
    DBG_ASSERT( !mxSecurityContext.is(), "XMLSignatureHelper::Init - mxSecurityContext already set!" );

    mxSEInitializer = css::xml::crypto::SEInitializer::create( mxCtx );

    if ( mxSEInitializer.is() )
        mxSecurityContext = mxSEInitializer->createSecurityContext( OUString() );

    return mxSecurityContext.is();
}

void XMLSignatureHelper::SetStorage(
    const Reference < css::embed::XStorage >& rxStorage,
    const OUString& sODFVersion)
{
    DBG_ASSERT( !mxUriBinding.is(), "SetStorage - UriBinding already set!" );
    mxUriBinding = new UriBindingHelper( rxStorage );
    DBG_ASSERT(rxStorage.is(), "SetStorage - empty storage!");
    mbODFPre1_2 = DocumentSignatureHelper::isODFPre_1_2(sODFVersion);
}


void XMLSignatureHelper::SetStartVerifySignatureHdl( const Link<LinkParamNone*,bool>& rLink )
{
    maStartVerifySignatureHdl = rLink;
}


void XMLSignatureHelper::StartMission()
{
    if ( !mxUriBinding.is() )
        mxUriBinding = new UriBindingHelper();

    mpXSecController->startMission( mxUriBinding, mxSecurityContext );
}

void XMLSignatureHelper::EndMission()
{
    mpXSecController->endMission();
}

sal_Int32 XMLSignatureHelper::GetNewSecurityId()
{
    return mpXSecController->getNewSecurityId();
}

void XMLSignatureHelper::SetX509Certificate(
        sal_Int32 nSecurityId,
        const OUString& ouX509IssuerName,
        const OUString& ouX509SerialNumber,
        const OUString& ouX509Cert,
        const OUString& ouX509CertDigest)
{
    mpXSecController->setX509Certificate(
        nSecurityId,
        ouX509IssuerName,
        ouX509SerialNumber,
        ouX509Cert,
        ouX509CertDigest);
}

void XMLSignatureHelper::SetDateTime( sal_Int32 nSecurityId, const ::Date& rDate, const tools::Time& rTime )
{
    css::util::DateTime stDateTime = ::DateTime(rDate, rTime).GetUNODateTime();
    mpXSecController->setDate( nSecurityId, stDateTime );
}

void XMLSignatureHelper::SetDescription(sal_Int32 nSecurityId, const OUString& rDescription)
{
    mpXSecController->setDescription(nSecurityId, rDescription);
}

void XMLSignatureHelper::AddForSigning( sal_Int32 nSecurityId, const OUString& uri, const OUString& objectURL, bool bBinary )
{
    mpXSecController->signAStream( nSecurityId, uri, objectURL, bBinary );
}


uno::Reference<xml::sax::XWriter> XMLSignatureHelper::CreateDocumentHandlerWithHeader(
    const css::uno::Reference< css::io::XOutputStream >& xOutputStream )
{
    /*
     * get SAX writer component
     */
    uno::Reference< lang::XMultiComponentFactory > xMCF( mxCtx->getServiceManager() );
    uno::Reference< xml::sax::XWriter > xSaxWriter = xml::sax::Writer::create(mxCtx);

    /*
     * connect XML writer to output stream
     */
    xSaxWriter->setOutputStream( xOutputStream );

    /*
     * write the xml context for signatures
     */
    OUString tag_AllSignatures(TAG_DOCUMENTSIGNATURES);

    SvXMLAttributeList *pAttributeList = new SvXMLAttributeList();
    OUString sNamespace;
    if (mbODFPre1_2)
        sNamespace = NS_DOCUMENTSIGNATURES;
    else
        sNamespace = NS_DOCUMENTSIGNATURES_ODF_1_2;

    pAttributeList->AddAttribute(
        ATTR_XMLNS,
        sNamespace);

    xSaxWriter->startDocument();
    xSaxWriter->startElement(
        tag_AllSignatures,
        uno::Reference< css::xml::sax::XAttributeList > (pAttributeList));

    return xSaxWriter;
}

void XMLSignatureHelper::CloseDocumentHandler( const uno::Reference<xml::sax::XDocumentHandler>& xDocumentHandler )
{
    OUString tag_AllSignatures(TAG_DOCUMENTSIGNATURES);
    xDocumentHandler->endElement( tag_AllSignatures );
    xDocumentHandler->endDocument();
}

void XMLSignatureHelper::ExportSignature(
    const uno::Reference< xml::sax::XDocumentHandler >& xDocumentHandler,
    const SignatureInformation& signatureInfo )
{
    XSecController::exportSignature(xDocumentHandler, signatureInfo);
}

void XMLSignatureHelper::ExportOOXMLSignature(const uno::Reference<embed::XStorage>& xRootStorage, const uno::Reference<embed::XStorage>& xSignatureStorage, const SignatureInformation& rInformation, int nSignatureIndex)
{
    sal_Int32 nOpenMode = embed::ElementModes::READWRITE;
    uno::Reference<io::XOutputStream> xOutputStream(xSignatureStorage->openStreamElement("sig" + OUString::number(nSignatureIndex) + ".xml", nOpenMode), uno::UNO_QUERY);

    if (rInformation.aSignatureBytes.hasElements())
        // This is a signature roundtrip, just write back the signature as-is.
        xOutputStream->writeBytes(rInformation.aSignatureBytes);
    else
    {
        uno::Reference<xml::sax::XWriter> xSaxWriter = xml::sax::Writer::create(mxCtx);
        xSaxWriter->setOutputStream(xOutputStream);
        xSaxWriter->startDocument();

        uno::Reference<xml::sax::XDocumentHandler> xDocumentHandler(xSaxWriter, uno::UNO_QUERY);
        mpXSecController->exportOOXMLSignature(xRootStorage, xDocumentHandler, rInformation);

        xSaxWriter->endDocument();
    }
}

bool XMLSignatureHelper::CreateAndWriteSignature( const uno::Reference< xml::sax::XDocumentHandler >& xDocumentHandler )
{
    mbError = false;

    /*
     * create a signature listener
     */

    /*
     * configure the signature creation listener
     */

    /*
     * write signatures
     */
    if ( !mpXSecController->WriteSignature( xDocumentHandler ) )
    {
        mbError = true;
    }

    /*
     * clear up the signature creation listener
     */

    return !mbError;
}

bool XMLSignatureHelper::ReadAndVerifySignature( const css::uno::Reference< css::io::XInputStream >& xInputStream )
{
    mbError = false;

    DBG_ASSERT(xInputStream.is(), "input stream missing");

    /*
     * prepare ParserInputSrouce
     */
    xml::sax::InputSource aParserInput;
    aParserInput.aInputStream = xInputStream;

    /*
     * get SAX parser component
     */
    uno::Reference< xml::sax::XParser > xParser = xml::sax::Parser::create(mxCtx);

    /*
     * create a signature reader
     */
    uno::Reference< xml::sax::XDocumentHandler > xHandler
        = mpXSecController->createSignatureReader( );

    /*
     * create a signature listener
     */
    ImplXMLSignatureListener* pSignatureListener = new ImplXMLSignatureListener(
                                                    LINK( this, XMLSignatureHelper, SignatureCreationResultListener ),
                                                    LINK( this, XMLSignatureHelper, SignatureVerifyResultListener ),
                                                    LINK( this, XMLSignatureHelper, StartVerifySignatureElement ) );

    /*
     * configure the signature verify listener
     */

    /*
     * setup the connection:
     * Parser -> SignatureListener -> SignatureReader
     */
    pSignatureListener->setNextHandler(xHandler);
    xParser->setDocumentHandler( pSignatureListener );

    /*
     * parser the stream
     */
    try
    {
        xParser->parseStream( aParserInput );
    }
    catch( xml::sax::SAXParseException& )
    {
        mbError = true;
    }
    catch( xml::sax::SAXException& )
    {
        mbError = true;
    }
    catch( css::io::IOException& )
    {
        mbError = true;
    }
    catch( uno::Exception& )
    {
        mbError = true;
    }

    /*
     * clear up the connection
     */
    pSignatureListener->setNextHandler( nullptr );

    /*
     * clear up the signature verify listener
     */

    /*
     * release the signature reader
     */
    mpXSecController->releaseSignatureReader( );

    return !mbError;
}

SignatureInformation XMLSignatureHelper::GetSignatureInformation( sal_Int32 nSecurityId ) const
{
    return mpXSecController->getSignatureInformation( nSecurityId );
}

SignatureInformations XMLSignatureHelper::GetSignatureInformations() const
{
    return mpXSecController->getSignatureInformations();
}

uno::Reference< css::xml::crypto::XSecurityEnvironment > XMLSignatureHelper::GetSecurityEnvironment()
{
    return (mxSecurityContext.is()?(mxSecurityContext->getSecurityEnvironment()): uno::Reference< css::xml::crypto::XSecurityEnvironment >());
}

IMPL_LINK_TYPED( XMLSignatureHelper, SignatureCreationResultListener, XMLSignatureCreationResult&, rResult, void )
{
    maCreationResults.insert( maCreationResults.begin() + maCreationResults.size(), rResult );
    if ( rResult.nSignatureCreationResult != css::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED )
        mbError = true;
}

IMPL_LINK_TYPED( XMLSignatureHelper, SignatureVerifyResultListener, XMLSignatureVerifyResult&, rResult, void )
{
    maVerifyResults.insert( maVerifyResults.begin() + maVerifyResults.size(), rResult );
    if ( rResult.nSignatureVerifyResult != css::xml::crypto::SecurityOperationStatus_OPERATION_SUCCEEDED )
        mbError = true;
}

IMPL_LINK_NOARG_TYPED( XMLSignatureHelper, StartVerifySignatureElement, LinkParamNone*, void )
{
    if ( !maStartVerifySignatureHdl.IsSet() || maStartVerifySignatureHdl.Call(nullptr) )
    {
        sal_Int32 nSignatureId = mpXSecController->getNewSecurityId();
        mpXSecController->addSignature( nSignatureId );
    }
}

namespace
{
bool lcl_isSignatureType(const beans::StringPair& rPair)
{
    return rPair.First == "Type" && rPair.Second == OOXML_SIGNATURE_SIGNATURE;
}
bool lcl_isSignatureOriginType(const beans::StringPair& rPair)
{
    return rPair.First == "Type" && rPair.Second == OOXML_SIGNATURE_ORIGIN;
}
}

bool XMLSignatureHelper::ReadAndVerifySignatureStorage(const uno::Reference<embed::XStorage>& xStorage, bool bCacheLastSignature)
{
    sal_Int32 nOpenMode = embed::ElementModes::READ;
    uno::Reference<container::XNameAccess> xNameAccess(xStorage, uno::UNO_QUERY);
    if (xNameAccess.is() && !xNameAccess->hasByName("_rels"))
    {
        SAL_WARN("xmlsecurity.helper", "expected stream, in signature storage but not found: _rels");
        return false;
    }

    uno::Reference<embed::XStorage> xSubStorage = xStorage->openStorageElement("_rels", nOpenMode);
    uno::Reference<io::XInputStream> xRelStream(xSubStorage->openStreamElement("origin.sigs.rels", nOpenMode), uno::UNO_QUERY);
    uno::Sequence< uno::Sequence<beans::StringPair> > aRelationsInfo;
    aRelationsInfo = comphelper::OFOPXMLHelper::ReadRelationsInfoSequence(xRelStream, "origin.sigs.rels", mxCtx);

    for (sal_Int32 i = 0; i < aRelationsInfo.getLength(); ++i)
    {
        const uno::Sequence<beans::StringPair>& rRelation = aRelationsInfo[i];
        auto aRelation = comphelper::sequenceToContainer< std::vector<beans::StringPair> >(rRelation);
        if (std::find_if(aRelation.begin(), aRelation.end(), lcl_isSignatureType) != aRelation.end())
        {
            std::vector<beans::StringPair>::iterator it = std::find_if(aRelation.begin(), aRelation.end(), [](const beans::StringPair& rPair) { return rPair.First == "Target"; });
            if (it != aRelation.end())
            {
                if (xNameAccess.is() && !xNameAccess->hasByName(it->Second))
                {
                    SAL_WARN("xmlsecurity.helper", "expected stream, but not found: " << it->Second);
                    continue;
                }

                uno::Reference<io::XInputStream> xInputStream(xStorage->openStreamElement(it->Second, nOpenMode), uno::UNO_QUERY);
                if (!ReadAndVerifySignatureStorageStream(xInputStream))
                    return false;

                // By default, we cache. If it's requested, then we don't cache the last signature.
                bool bCache = true;
                if (!bCacheLastSignature && i == aRelationsInfo.getLength() - 1)
                    bCache = false;

                if (bCache)
                {
                    // Store the contents of the stream as is, in case we need to write it back later.
                    xInputStream.clear();
                    xInputStream.set(xStorage->openStreamElement(it->Second, nOpenMode), uno::UNO_QUERY);
                    uno::Reference<beans::XPropertySet> xPropertySet(xInputStream, uno::UNO_QUERY);
                    if (xPropertySet.is())
                    {
                        sal_Int64 nSize = 0;
                        xPropertySet->getPropertyValue("Size") >>= nSize;
                        uno::Sequence<sal_Int8> aData;
                        xInputStream->readBytes(aData, nSize);
                        mpXSecController->setSignatureBytes(aData);
                    }
                }
            }
        }
    }

    return true;
}

bool XMLSignatureHelper::ReadAndVerifySignatureStorageStream(const css::uno::Reference<css::io::XInputStream>& xInputStream)
{
    mbError = false;

    // Create the input source.
    xml::sax::InputSource aParserInput;
    aParserInput.aInputStream = xInputStream;

    // Create the sax parser.
    uno::Reference<xml::sax::XParser> xParser = xml::sax::Parser::create(mxCtx);

    // Create the signature reader.
    uno::Reference<xml::sax::XDocumentHandler> xHandler = mpXSecController->createSignatureReader(embed::StorageFormats::OFOPXML);

    // Create the signature listener.
    ImplXMLSignatureListener* pSignatureListener = new ImplXMLSignatureListener(
        LINK(this, XMLSignatureHelper, SignatureCreationResultListener),
        LINK(this, XMLSignatureHelper, SignatureVerifyResultListener),
        LINK(this, XMLSignatureHelper, StartVerifySignatureElement));
    uno::Reference<xml::sax::XDocumentHandler> xSignatureListener(pSignatureListener);

    // Parser -> signature listener -> signature reader.
    pSignatureListener->setNextHandler(xHandler);
    xParser->setDocumentHandler(xSignatureListener);

    // Parse the stream.
    try
    {
        xParser->parseStream(aParserInput);
    }
    catch(const uno::Exception& rException)
    {
        SAL_WARN("xmlsecurity.helper", "XMLSignatureHelper::ReadAndVerifySignatureStorageStream: " << rException.Message);
    }

    pSignatureListener->setNextHandler(nullptr);
    mpXSecController->releaseSignatureReader();

    return !mbError;
}

void XMLSignatureHelper::EnsureSignaturesRelation(const css::uno::Reference<css::embed::XStorage>& xStorage, bool bAdd)
{
    sal_Int32 nOpenMode = embed::ElementModes::READWRITE;
    uno::Reference<embed::XStorage> xSubStorage = xStorage->openStorageElement("_rels", nOpenMode);
    uno::Reference<io::XInputStream> xRelStream(xSubStorage->openStreamElement(".rels", nOpenMode), uno::UNO_QUERY);
    std::vector< uno::Sequence<beans::StringPair> > aRelationsInfo;
    aRelationsInfo = comphelper::sequenceToContainer< std::vector< uno::Sequence<beans::StringPair> > >(comphelper::OFOPXMLHelper::ReadRelationsInfoSequence(xRelStream, ".rels", mxCtx));

    // Do we have a relation already?
    bool bHaveRelation = false;
    int nCount = 0;
    for (const uno::Sequence<beans::StringPair>& rRelation : aRelationsInfo)
    {
        auto aRelation = comphelper::sequenceToContainer< std::vector<beans::StringPair> >(rRelation);
        if (std::find_if(aRelation.begin(), aRelation.end(), lcl_isSignatureOriginType) != aRelation.end())
        {
            bHaveRelation = true;
            break;
        }
        ++nCount;
    }

    if (!bHaveRelation && bAdd)
    {
        // No, and have to add one.
        std::vector<beans::StringPair> aRelation;
        aRelation.push_back(beans::StringPair("Id", "rId" + OUString::number(++nCount)));
        aRelation.push_back(beans::StringPair("Type", OOXML_SIGNATURE_ORIGIN));
        aRelation.push_back(beans::StringPair("Target", "_xmlsignatures/origin.sigs"));
        aRelationsInfo.push_back(comphelper::containerToSequence(aRelation));
    }
    else if (bHaveRelation && !bAdd)
    {
        // Yes, and need to remove it.
        for (std::vector< uno::Sequence<beans::StringPair> >::iterator it = aRelationsInfo.begin(); it != aRelationsInfo.end();)
        {
            auto aRelation = comphelper::sequenceToContainer< std::vector<beans::StringPair> >(*it);
            if (std::find_if(aRelation.begin(), aRelation.end(), lcl_isSignatureOriginType) != aRelation.end())
                it = aRelationsInfo.erase(it);
            else
                ++it;
        }
    }

    // Write it back.
    uno::Reference<io::XTruncate> xTruncate(xRelStream, uno::UNO_QUERY);
    xTruncate->truncate();
    uno::Reference<io::XOutputStream> xOutputStream(xRelStream, uno::UNO_QUERY);
    comphelper::OFOPXMLHelper::WriteRelationsInfoSequence(xOutputStream, comphelper::containerToSequence(aRelationsInfo), mxCtx);

    // Commit it.
    uno::Reference<embed::XTransactedObject> xTransact(xSubStorage, uno::UNO_QUERY);
    xTransact->commit();
    xTransact.set(xStorage, uno::UNO_QUERY);
    xTransact->commit();
}

void XMLSignatureHelper::ExportSignatureRelations(const css::uno::Reference<css::embed::XStorage>& xStorage, int nSignatureCount)
{
    // Write the empty file, its relations will be the signatures.
    sal_Int32 nOpenMode = embed::ElementModes::READWRITE;
    uno::Reference<io::XOutputStream> xOriginStream(xStorage->openStreamElement("origin.sigs", nOpenMode), uno::UNO_QUERY);
    uno::Reference<io::XTruncate> xTruncate(xOriginStream, uno::UNO_QUERY);
    xTruncate->truncate();
    xOriginStream->closeOutput();

    // Write the relations.
    uno::Reference<embed::XStorage> xSubStorage(xStorage->openStorageElement("_rels", nOpenMode), uno::UNO_QUERY);
    uno::Reference<io::XOutputStream> xRelStream(xSubStorage->openStreamElement("origin.sigs.rels", nOpenMode), uno::UNO_QUERY);
    std::vector< uno::Sequence<beans::StringPair> > aRelations;
    for (int i = 0; i < nSignatureCount; ++i)
    {
        std::vector<beans::StringPair> aRelation;
        aRelation.push_back(beans::StringPair("Id", "rId" + OUString::number(i + 1)));
        aRelation.push_back(beans::StringPair("Type", OOXML_SIGNATURE_SIGNATURE));
        aRelation.push_back(beans::StringPair("Target", "sig" + OUString::number(i + 1) + ".xml"));
        aRelations.push_back(comphelper::containerToSequence(aRelation));
    }
    comphelper::OFOPXMLHelper::WriteRelationsInfoSequence(xRelStream, comphelper::containerToSequence(aRelations), mxCtx);
    uno::Reference<embed::XTransactedObject> xTransact(xSubStorage, uno::UNO_QUERY);
    xTransact->commit();
}

void XMLSignatureHelper::ExportSignatureContentTypes(const css::uno::Reference<css::embed::XStorage>& xStorage, int nSignatureCount)
{
    sal_Int32 nOpenMode = embed::ElementModes::READWRITE;
    uno::Reference<io::XStream> xStream(xStorage->openStreamElement("[Content_Types].xml", nOpenMode), uno::UNO_QUERY);
    uno::Reference<io::XInputStream> xInputStream = xStream->getInputStream();
    uno::Sequence< uno::Sequence<beans::StringPair> > aContentTypeInfo = comphelper::OFOPXMLHelper::ReadContentTypeSequence(xInputStream, mxCtx);
    if (aContentTypeInfo.getLength() < 2)
    {
        SAL_WARN("xmlsecurity.helper", "no defaults or overrides in aContentTypeInfo");
        return;
    }

    // Append rels and sigs to defaults, if it's not there already.
    uno::Sequence<beans::StringPair>& rDefaults = aContentTypeInfo[0];
    auto aDefaults = comphelper::sequenceToContainer< std::vector<beans::StringPair> >(rDefaults);
    auto it = std::find_if(rDefaults.begin(), rDefaults.end(), [](const beans::StringPair& rPair)
    {
        return rPair.First == "rels";
    });
    if (it == rDefaults.end())
        aDefaults.push_back(beans::StringPair("rels", "application/vnd.openxmlformats-package.relationships+xml"));

    it = std::find_if(rDefaults.begin(), rDefaults.end(), [](const beans::StringPair& rPair)
    {
        return rPair.First == "sigs";
    });
    if (it == rDefaults.end())
        aDefaults.push_back(beans::StringPair("sigs", "application/vnd.openxmlformats-package.digital-signature-origin"));
    rDefaults = comphelper::containerToSequence(aDefaults);

    // Remove existing signature overrides.
    uno::Sequence<beans::StringPair>& rOverrides = aContentTypeInfo[1];
    auto aOverrides = comphelper::sequenceToContainer< std::vector<beans::StringPair> >(rOverrides);
    aOverrides.erase(std::remove_if(aOverrides.begin(), aOverrides.end(), [](const beans::StringPair& rPair)
    {
        return rPair.First.startsWith("/_xmlsignatures/sig");
    }), aOverrides.end());

    // Add our signature overrides.
    for (int i = 1; i <= nSignatureCount; ++i)
        aOverrides.push_back(beans::StringPair("/_xmlsignatures/sig" + OUString::number(i) + ".xml", "application/vnd.openxmlformats-package.digital-signature-xmlsignature+xml"));

    rOverrides = comphelper::containerToSequence(aOverrides);
    uno::Reference<io::XOutputStream> xOutputStream = xStream->getOutputStream();
    uno::Reference <io::XTruncate> xTruncate(xOutputStream, uno::UNO_QUERY);
    xTruncate->truncate();
    comphelper::OFOPXMLHelper::WriteContentSequence(xOutputStream, rDefaults, rOverrides, mxCtx);
    uno::Reference<embed::XTransactedObject> xTransact(xStorage, uno::UNO_QUERY);
    xTransact->commit();
}
bool XMLSignatureHelper::CreateAndWriteOOXMLSignature(const uno::Reference<embed::XStorage>& xRootStorage, const uno::Reference<embed::XStorage>& xSignatureStorage, int nSignatureIndex)
{
    sal_Int32 nOpenMode = embed::ElementModes::READWRITE;
    uno::Reference<io::XOutputStream> xOutputStream(xSignatureStorage->openStreamElement("sig" + OUString::number(nSignatureIndex) + ".xml", nOpenMode), uno::UNO_QUERY);
    uno::Reference<xml::sax::XWriter> xSaxWriter = xml::sax::Writer::create(mxCtx);
    xSaxWriter->setOutputStream(xOutputStream);
    xSaxWriter->startDocument();

    mbError = false;
    uno::Reference<xml::sax::XDocumentHandler> xDocumentHandler(xSaxWriter, uno::UNO_QUERY);
    if (!mpXSecController->WriteOOXMLSignature(xRootStorage, xDocumentHandler))
        mbError = true;

    xSaxWriter->endDocument();

    return !mbError;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
