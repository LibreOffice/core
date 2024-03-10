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


#include <xmlsignaturehelper.hxx>
#include <documentsignaturehelper.hxx>
#include <xsecctl.hxx>
#include <biginteger.hxx>
#include <certificate.hxx>

#include <UriBindingHelper.hxx>

#include <tools/datetime.hxx>

#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/StorageFormats.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>

#include <comphelper/attributelist.hxx>
#include <comphelper/ofopxmlhelper.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <sal/log.hxx>

#include <optional>

constexpr OUStringLiteral NS_DOCUMENTSIGNATURES = u"http://openoffice.org/2004/documentsignatures";
constexpr OUStringLiteral NS_DOCUMENTSIGNATURES_ODF_1_2 = u"urn:oasis:names:tc:opendocument:xmlns:digitalsignature:1.0";
constexpr OUString OOXML_SIGNATURE_ORIGIN = u"http://schemas.openxmlformats.org/package/2006/relationships/digital-signature/origin"_ustr;
constexpr OUString OOXML_SIGNATURE_SIGNATURE = u"http://schemas.openxmlformats.org/package/2006/relationships/digital-signature/signature"_ustr;

using namespace ::com::sun::star;
using namespace ::com::sun::star::graphic;
using namespace ::com::sun::star::uno;

XMLSignatureHelper::XMLSignatureHelper( const uno::Reference< uno::XComponentContext >& rxCtx)
    : mxCtx(rxCtx), mbODFPre1_2(false)
{
    mpXSecController = new XSecController(rxCtx);
    mbError = false;
}

XMLSignatureHelper::~XMLSignatureHelper()
{
}

void XMLSignatureHelper::SetStorage(
    const Reference < css::embed::XStorage >& rxStorage,
    std::u16string_view sODFVersion)
{
    SAL_WARN_IF( mxUriBinding.is(), "xmlsecurity.helper", "SetStorage - UriBinding already set!" );
    mxUriBinding = new UriBindingHelper( rxStorage );
    SAL_WARN_IF(!rxStorage.is(), "xmlsecurity.helper", "SetStorage - empty storage!");
    mbODFPre1_2 = DocumentSignatureHelper::isODFPre_1_2(sODFVersion);
}


void XMLSignatureHelper::SetStartVerifySignatureHdl( const Link<LinkParamNone*,bool>& rLink )
{
    maStartVerifySignatureHdl = rLink;
}


void XMLSignatureHelper::StartMission(const uno::Reference<xml::crypto::XXMLSecurityContext>& xSecurityContext)
{
    if ( !mxUriBinding.is() )
        mxUriBinding = new UriBindingHelper();

    mpXSecController->startMission(mxUriBinding, xSecurityContext);
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
        const OUString& ouX509CertDigest,
        svl::crypto::SignatureMethodAlgorithm eAlgorithmID)
{
    mpXSecController->setX509Certificate(
        nSecurityId,
        ouX509IssuerName,
        ouX509SerialNumber,
        ouX509Cert,
        ouX509CertDigest,
        eAlgorithmID);
}

void XMLSignatureHelper::AddEncapsulatedX509Certificate(const OUString& ouEncapsulatedX509Certificate)
{
    mpXSecController->addEncapsulatedX509Certificate(ouEncapsulatedX509Certificate);
}

void XMLSignatureHelper::SetGpgCertificate(sal_Int32 nSecurityId,
                                           const OUString& ouGpgCertDigest,
                                           const OUString& ouGpgCert,
                                           const OUString& ouGpgOwner)
{
    mpXSecController->setGpgCertificate(
        nSecurityId,
        ouGpgCertDigest,
        ouGpgCert,
        ouGpgOwner);
}

void XMLSignatureHelper::SetDateTime( sal_Int32 nSecurityId, const ::DateTime& rDateTime )
{
    css::util::DateTime stDateTime = rDateTime.GetUNODateTime();
    mpXSecController->setDate( nSecurityId, stDateTime );
}

void XMLSignatureHelper::SetDescription(sal_Int32 nSecurityId, const OUString& rDescription)
{
    mpXSecController->setDescription(nSecurityId, rDescription);
}

void XMLSignatureHelper::SetSignatureLineId(sal_Int32 nSecurityId, const OUString& rSignatureLineId)
{
    mpXSecController->setSignatureLineId(nSecurityId, rSignatureLineId);
}

void XMLSignatureHelper::SetSignatureLineValidGraphic(
    sal_Int32 nSecurityId, const css::uno::Reference<XGraphic>& xValidGraphic)
{
    mpXSecController->setSignatureLineValidGraphic(nSecurityId, xValidGraphic);
}

void XMLSignatureHelper::SetSignatureLineInvalidGraphic(
    sal_Int32 nSecurityId, const css::uno::Reference<XGraphic>& xInvalidGraphic)
{
    mpXSecController->setSignatureLineInvalidGraphic(nSecurityId, xInvalidGraphic);
}

void XMLSignatureHelper::AddForSigning( sal_Int32 nSecurityId, const OUString& uri, bool bBinary, bool bXAdESCompliantIfODF )
{
    mpXSecController->signAStream( nSecurityId, uri, bBinary, bXAdESCompliantIfODF );
}


uno::Reference<xml::sax::XWriter> XMLSignatureHelper::CreateDocumentHandlerWithHeader(
    const css::uno::Reference< css::io::XOutputStream >& xOutputStream )
{
    /*
     * get SAX writer component
     */
    uno::Reference< xml::sax::XWriter > xSaxWriter = xml::sax::Writer::create(mxCtx);

    /*
     * connect XML writer to output stream
     */
    xSaxWriter->setOutputStream( xOutputStream );

    /*
     * write the xml context for signatures
     */
    rtl::Reference<comphelper::AttributeList> pAttributeList = new comphelper::AttributeList();
    OUString sNamespace;
    if (mbODFPre1_2)
        sNamespace = NS_DOCUMENTSIGNATURES;
    else
        sNamespace = NS_DOCUMENTSIGNATURES_ODF_1_2;

    pAttributeList->AddAttribute(
        "xmlns",
        sNamespace);

    xSaxWriter->startDocument();
    xSaxWriter->startElement(
        "document-signatures",
        pAttributeList);

    return xSaxWriter;
}

void XMLSignatureHelper::CloseDocumentHandler( const uno::Reference<xml::sax::XDocumentHandler>& xDocumentHandler )
{
    xDocumentHandler->endElement( "document-signatures" );
    xDocumentHandler->endDocument();
}

void XMLSignatureHelper::ExportSignature(
    const uno::Reference< xml::sax::XDocumentHandler >& xDocumentHandler,
    const SignatureInformation& signatureInfo,
    bool bXAdESCompliantIfODF )
{
    XSecController::exportSignature(xDocumentHandler, signatureInfo, bXAdESCompliantIfODF);
}

void XMLSignatureHelper::ExportOOXMLSignature(const uno::Reference<embed::XStorage>& xRootStorage, const uno::Reference<embed::XStorage>& xSignatureStorage, const SignatureInformation& rInformation, int nSignatureIndex)
{
    uno::Reference<io::XOutputStream> xOutputStream(xSignatureStorage->openStreamElement("sig" + OUString::number(nSignatureIndex) + ".xml", embed::ElementModes::READWRITE), uno::UNO_QUERY);

    if (rInformation.aSignatureBytes.hasElements())
        // This is a signature roundtrip, just write back the signature as-is.
        xOutputStream->writeBytes(rInformation.aSignatureBytes);
    else
    {
        uno::Reference<xml::sax::XWriter> xSaxWriter = xml::sax::Writer::create(mxCtx);
        xSaxWriter->setOutputStream(xOutputStream);
        xSaxWriter->startDocument();

        mpXSecController->exportOOXMLSignature(xRootStorage, xSaxWriter, rInformation);

        xSaxWriter->endDocument();
    }
}

void XMLSignatureHelper::CreateAndWriteSignature( const uno::Reference< xml::sax::XDocumentHandler >& xDocumentHandler, bool bXAdESCompliantIfODF )
{
    mbError = false;

    if ( !mpXSecController->WriteSignature( xDocumentHandler, bXAdESCompliantIfODF ) )
    {
        mbError = true;
    }
}

bool XMLSignatureHelper::ReadAndVerifySignature( const css::uno::Reference< css::io::XInputStream >& xInputStream )
{
    mbError = false;

    SAL_WARN_IF(!xInputStream.is(), "xmlsecurity.helper", "input stream missing");

    // prepare ParserInputSource
    xml::sax::InputSource aParserInput;
    aParserInput.aInputStream = xInputStream;

    // get SAX parser component
    uno::Reference< xml::sax::XParser > xParser = xml::sax::Parser::create(mxCtx);

    // create a signature reader
    uno::Reference< xml::sax::XDocumentHandler > xHandler
        = mpXSecController->createSignatureReader(*this);

    // setup the connection:
    // Parser -> SignatureReader
    xParser->setDocumentHandler( xHandler );

    // Parse the stream.
    try
    {
        xParser->parseStream( aParserInput );
    }
    catch( uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("xmlsecurity.helper");
        mbError = true;
    }

    // release the signature reader
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

void XMLSignatureHelper::StartVerifySignatureElement()
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
    if (xStorage.is() && !xStorage->hasByName("_rels"))
    {
        SAL_WARN("xmlsecurity.helper", "expected stream, in signature storage but not found: _rels");
        return false;
    }

    uno::Reference<embed::XStorage> xSubStorage = xStorage->openStorageElement("_rels", nOpenMode);
    uno::Reference<io::XInputStream> xRelStream(xSubStorage->openStreamElement("origin.sigs.rels", nOpenMode), uno::UNO_QUERY);
    uno::Sequence< uno::Sequence<beans::StringPair> > aRelationsInfo = comphelper::OFOPXMLHelper::ReadRelationsInfoSequence(xRelStream, u"origin.sigs.rels", mxCtx);

    for (sal_Int32 i = 0; i < aRelationsInfo.getLength(); ++i)
    {
        const uno::Sequence<beans::StringPair>& rRelation = aRelationsInfo[i];
        if (std::any_of(rRelation.begin(), rRelation.end(), lcl_isSignatureType))
        {
            auto it = std::find_if(rRelation.begin(), rRelation.end(), [](const beans::StringPair& rPair) { return rPair.First == "Target"; });
            if (it != rRelation.end())
            {
                if (xStorage.is() && !xStorage->hasByName(it->Second))
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

                if (!bCache)
                    continue;
                // Store the contents of the stream as is, in case we need to write it back later.
                xInputStream.clear();
                xInputStream.set(xStorage->openStreamElement(it->Second, nOpenMode), uno::UNO_QUERY);
                uno::Reference<beans::XPropertySet> xPropertySet(xInputStream, uno::UNO_QUERY);
                if (!xPropertySet.is())
                    continue;

                sal_Int64 nSize = 0;
                xPropertySet->getPropertyValue("Size") >>= nSize;
                if (nSize < 0 || nSize > SAL_MAX_INT32)
                {
                    SAL_WARN("xmlsecurity.helper", "bogus signature size: " << nSize);
                    continue;
                }
                uno::Sequence<sal_Int8> aData;
                xInputStream->readBytes(aData, nSize);
                mpXSecController->setSignatureBytes(aData);
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
    uno::Reference<xml::sax::XDocumentHandler> xHandler = mpXSecController->createSignatureReader(*this, embed::StorageFormats::OFOPXML);

    // Parser -> signature reader.
    xParser->setDocumentHandler(xHandler);

    // Parse the stream.
    try
    {
        xParser->parseStream(aParserInput);
    }
    catch(const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("xmlsecurity.helper");
        mbError = true;
    }

    // release the signature reader
    mpXSecController->releaseSignatureReader();

    return !mbError;
}

void XMLSignatureHelper::EnsureSignaturesRelation(const css::uno::Reference<css::embed::XStorage>& xStorage, bool bAdd)
{
    sal_Int32 nOpenMode = embed::ElementModes::READWRITE;
    uno::Reference<embed::XStorage> xSubStorage = xStorage->openStorageElement("_rels", nOpenMode);
    uno::Reference<io::XInputStream> xRelStream(xSubStorage->openStreamElement(".rels", nOpenMode), uno::UNO_QUERY);
    std::vector< uno::Sequence<beans::StringPair> > aRelationsInfo = comphelper::sequenceToContainer< std::vector< uno::Sequence<beans::StringPair> > >(comphelper::OFOPXMLHelper::ReadRelationsInfoSequence(xRelStream, u".rels", mxCtx));

    // Do we have a relation already?
    bool bHaveRelation = false;
    int nCount = 0;
    for (const uno::Sequence<beans::StringPair>& rRelation : aRelationsInfo)
    {
        auto aRelation = comphelper::sequenceToContainer< std::vector<beans::StringPair> >(rRelation);
        if (std::any_of(aRelation.begin(), aRelation.end(), lcl_isSignatureOriginType))
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
        aRelation.emplace_back("Id", "rId" + OUString::number(++nCount));
        aRelation.emplace_back("Type", OOXML_SIGNATURE_ORIGIN);
        aRelation.emplace_back("Target", "_xmlsignatures/origin.sigs");
        aRelationsInfo.push_back(comphelper::containerToSequence(aRelation));
    }
    else if (bHaveRelation && !bAdd)
    {
        // Yes, and need to remove it.
        for (std::vector< uno::Sequence<beans::StringPair> >::iterator it = aRelationsInfo.begin(); it != aRelationsInfo.end();)
        {
            auto aRelation = comphelper::sequenceToContainer< std::vector<beans::StringPair> >(*it);
            if (std::any_of(aRelation.begin(), aRelation.end(), lcl_isSignatureOriginType))
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
    uno::Reference<embed::XStorage> xSubStorage = xStorage->openStorageElement("_rels", nOpenMode);
    uno::Reference<io::XOutputStream> xRelStream(xSubStorage->openStreamElement("origin.sigs.rels", nOpenMode), uno::UNO_QUERY);
    std::vector< uno::Sequence<beans::StringPair> > aRelations;
    for (int i = 0; i < nSignatureCount; ++i)
    {
        std::vector<beans::StringPair> aRelation;
        aRelation.emplace_back("Id", "rId" + OUString::number(i + 1));
        aRelation.emplace_back("Type", OOXML_SIGNATURE_SIGNATURE);
        aRelation.emplace_back("Target", "sig" + OUString::number(i + 1) + ".xml");
        aRelations.push_back(comphelper::containerToSequence(aRelation));
    }
    comphelper::OFOPXMLHelper::WriteRelationsInfoSequence(xRelStream, comphelper::containerToSequence(aRelations), mxCtx);
    uno::Reference<embed::XTransactedObject> xTransact(xSubStorage, uno::UNO_QUERY);
    xTransact->commit();
}

void XMLSignatureHelper::ExportSignatureContentTypes(const css::uno::Reference<css::embed::XStorage>& xStorage, int nSignatureCount)
{
    uno::Reference<io::XStream> xStream = xStorage->openStreamElement("[Content_Types].xml", embed::ElementModes::READWRITE);
    uno::Reference<io::XInputStream> xInputStream = xStream->getInputStream();
    uno::Sequence< uno::Sequence<beans::StringPair> > aContentTypeInfo = comphelper::OFOPXMLHelper::ReadContentTypeSequence(xInputStream, mxCtx);
    if (aContentTypeInfo.getLength() < 2)
    {
        SAL_WARN("xmlsecurity.helper", "no defaults or overrides in aContentTypeInfo");
        return;
    }
    auto pContentTypeInfo = aContentTypeInfo.getArray();

    // Append rels and sigs to defaults, if it's not there already.
    uno::Sequence<beans::StringPair>& rDefaults = pContentTypeInfo[0];
    auto aDefaults = comphelper::sequenceToContainer< std::vector<beans::StringPair> >(rDefaults);
    if (std::none_of(std::cbegin(rDefaults), std::cend(rDefaults), [](const beans::StringPair& rPair) { return rPair.First == "rels"; }))
        aDefaults.emplace_back("rels", "application/vnd.openxmlformats-package.relationships+xml");

    if (std::none_of(std::cbegin(rDefaults), std::cend(rDefaults), [](const beans::StringPair& rPair) { return rPair.First == "sigs"; }))
        aDefaults.emplace_back("sigs", "application/vnd.openxmlformats-package.digital-signature-origin");
    rDefaults = comphelper::containerToSequence(aDefaults);

    // Remove existing signature overrides.
    uno::Sequence<beans::StringPair>& rOverrides = pContentTypeInfo[1];
    auto aOverrides = comphelper::sequenceToContainer< std::vector<beans::StringPair> >(rOverrides);
    std::erase_if(aOverrides, [](const beans::StringPair& rPair)
    {
        return rPair.First.startsWith("/_xmlsignatures/sig");
    });

    // Add our signature overrides.
    for (int i = 1; i <= nSignatureCount; ++i)
        aOverrides.emplace_back("/_xmlsignatures/sig" + OUString::number(i) + ".xml", "application/vnd.openxmlformats-package.digital-signature-xmlsignature+xml");

    rOverrides = comphelper::containerToSequence(aOverrides);
    uno::Reference<io::XOutputStream> xOutputStream = xStream->getOutputStream();
    uno::Reference <io::XTruncate> xTruncate(xOutputStream, uno::UNO_QUERY);
    xTruncate->truncate();
    comphelper::OFOPXMLHelper::WriteContentSequence(xOutputStream, rDefaults, rOverrides, mxCtx);
    uno::Reference<embed::XTransactedObject> xTransact(xStorage, uno::UNO_QUERY);
    xTransact->commit();
}
void XMLSignatureHelper::CreateAndWriteOOXMLSignature(const uno::Reference<embed::XStorage>& xRootStorage, const uno::Reference<embed::XStorage>& xSignatureStorage, int nSignatureIndex)
{
    uno::Reference<io::XOutputStream> xOutputStream(xSignatureStorage->openStreamElement("sig" + OUString::number(nSignatureIndex) + ".xml", embed::ElementModes::READWRITE), uno::UNO_QUERY);
    uno::Reference<xml::sax::XWriter> xSaxWriter = xml::sax::Writer::create(mxCtx);
    xSaxWriter->setOutputStream(xOutputStream);
    xSaxWriter->startDocument();

    mbError = false;
    if (!mpXSecController->WriteOOXMLSignature(xRootStorage, xSaxWriter))
        mbError = true;

    xSaxWriter->endDocument();
}

/** check this constraint from xmldsig-core 4.5.4:

  All certificates appearing in an X509Data element MUST relate to the
  validation key by either containing it or being part of a certification
  chain that terminates in a certificate containing the validation key.
 */
static auto CheckX509Data(
    uno::Reference<xml::crypto::XSecurityEnvironment> const& xSecEnv,
    std::vector<SignatureInformation::X509CertInfo> const& rX509CertInfos,
    std::vector<uno::Reference<security::XCertificate>> & rCerts,
    std::vector<SignatureInformation::X509CertInfo> & rSorted) -> bool
{
    assert(rCerts.empty());
    assert(rSorted.empty());
    if (rX509CertInfos.empty())
    {
        SAL_WARN("xmlsecurity.comp", "no X509Data");
        return false;
    }
    std::vector<uno::Reference<security::XCertificate>> certs;
    for (SignatureInformation::X509CertInfo const& it : rX509CertInfos)
    {
        if (!it.X509Certificate.isEmpty())
        {
            certs.emplace_back(xSecEnv->createCertificateFromAscii(it.X509Certificate));
        }
        else
        {
            certs.emplace_back(xSecEnv->getCertificate(
                it.X509IssuerName,
                xmlsecurity::numericStringToBigInteger(it.X509SerialNumber)));
        }
        if (!certs.back().is())
        {
            SAL_WARN("xmlsecurity.comp", "X509Data cannot be parsed");
            return false;
        }
    }

    // first, search one whose issuer isn't in the list, or a self-signed one
    std::optional<size_t> start;
    for (size_t i = 0; i < certs.size(); ++i)
    {
        for (size_t j = 0; ; ++j)
        {
            if (j == certs.size())
            {
                if (start)
                {
                    SAL_WARN("xmlsecurity.comp", "X509Data do not form a chain: certificate has no issuer but already have start of chain: " << certs[i]->getSubjectName());
                    return false;
                }
                start = i; // issuer isn't in the list
                break;
            }
            if (xmlsecurity::EqualDistinguishedNames(certs[i]->getIssuerName(), certs[j]->getSubjectName(), xmlsecurity::NOCOMPAT))
            {
                if (i == j) // self signed
                {
                    if (start)
                    {
                        SAL_WARN("xmlsecurity.comp", "X509Data do not form a chain: certificate is self-signed but already have start of chain: " << certs[i]->getSubjectName());
                        return false;
                    }
                    start = i;
                }
                break;
            }
        }
    }
    std::vector<size_t> chain;
    if (!start)
    {
        // this can only be a cycle?
        SAL_WARN("xmlsecurity.comp", "X509Data do not form a chain: cycle detected");
        return false;
    }
    chain.emplace_back(*start);

    // second, check that there is a chain, no tree or cycle...
    for (size_t i = 0; i < certs.size(); ++i)
    {
        assert(chain.size() == i + 1);
        for (size_t j = 0; j < certs.size(); ++j)
        {
            if (chain[i] != j)
            {
                if (xmlsecurity::EqualDistinguishedNames(
                        certs[chain[i]]->getSubjectName(), certs[j]->getIssuerName(), xmlsecurity::NOCOMPAT))
                {
                    if (chain.size() != i + 1) // already found issue?
                    {
                        SAL_WARN("xmlsecurity.comp", "X509Data do not form a chain: certificate issued 2 others: " << certs[chain[i]]->getSubjectName());
                        return false;
                    }
                    chain.emplace_back(j);
                }
            }
        }
        if (i == certs.size() - 1)
        {   // last one: must be a leaf
            if (chain.size() != i + 1)
            {
                SAL_WARN("xmlsecurity.comp", "X509Data do not form a chain: certificate in cycle: " << certs[chain[i]]->getSubjectName());
                return false;
            }
        }
        else if (chain.size() != i + 2)
        {   // not issuer of another?
            SAL_WARN("xmlsecurity.comp", "X509Data do not form a chain: certificate issued 0 others: " << certs[chain[i]]->getSubjectName());
            return false;
        }
    }

    // success
    assert(chain.size() == rX509CertInfos.size());
    for (auto const& it : chain)
    {
        rSorted.emplace_back(rX509CertInfos[it]);
        rCerts.emplace_back(certs[it]);
    }
    return true;
}

std::vector<uno::Reference<security::XCertificate>>
XMLSignatureHelper::CheckAndUpdateSignatureInformation(
    uno::Reference<xml::crypto::XSecurityEnvironment> const& xSecEnv,
    SignatureInformation const& rInfo)
{
    // if the check fails, it's not possible to determine which X509Data
    // contained the signing certificate - the UI cannot display something
    // useful in this case, so prevent anything misleading by clearing the
    // X509Datas.

    std::vector<uno::Reference<security::XCertificate>> certs;
    std::vector<SignatureInformation::X509Data> datas;
    // TODO: for now, just merge all X509Datas together for checking...
    // (this will probably break round-trip of signature with multiple X509Data,
    // no idea if that is a problem)
    SignatureInformation::X509Data temp;
    SignatureInformation::X509Data tempResult;
    for (auto const& rData : rInfo.X509Datas)
    {
        for (auto const& it : rData)
        {
            temp.emplace_back(it);
        }
    }
    if (CheckX509Data(xSecEnv, temp, certs, tempResult))
    {
        if (rInfo.maEncapsulatedX509Certificates.empty()) // optional, XAdES
        {
            datas.emplace_back(tempResult);
        }
        else
        {
            // check for consistency between X509Data and EncapsulatedX509Certificate
            // (LO produces just the signing certificate in X509Data and
            // the entire chain in EncapsulatedX509Certificate so in this case
            // using EncapsulatedX509Certificate yields additional intermediate
            // certificates that may help in verifying)
            std::vector<SignatureInformation::X509CertInfo> encapsulatedCertInfos;
            for (OUString const& it : rInfo.maEncapsulatedX509Certificates)
            {
                encapsulatedCertInfos.emplace_back();
                encapsulatedCertInfos.back().X509Certificate = it;
            }
            std::vector<uno::Reference<security::XCertificate>> encapsulatedCerts;
            SignatureInformation::X509Data encapsulatedResult;
            if (CheckX509Data(xSecEnv, encapsulatedCertInfos, encapsulatedCerts, encapsulatedResult))
            {
                auto const pXCertificate(dynamic_cast<xmlsecurity::Certificate*>(certs.back().get()));
                auto const pECertificate(dynamic_cast<xmlsecurity::Certificate*>(encapsulatedCerts.back().get()));
                assert(pXCertificate && pECertificate); // was just created by CheckX509Data
                if (pXCertificate->getSHA256Thumbprint() == pECertificate->getSHA256Thumbprint())
                {
                    // both are chains - take the longer one
                    if (encapsulatedCerts.size() < certs.size())
                    {
                        datas.emplace_back(tempResult);
                    }
                    else
                    {
#if 0
                        // extra info needed in testSigningMultipleTimes_ODT
                        // ... but with it, it fails with BROKEN signature?
                        // fails even on the first signature, because somehow
                        // the xd:SigningCertificate element was signed
                        // containing only one certificate, but in the final
                        // file it contains all 3 certificates due to this here.
                        for (size_t i = 0; i < encapsulatedResult.size(); ++i)
                        {
                            encapsulatedResult[i].X509IssuerName = encapsulatedCerts[i]->getIssuerName();
                            encapsulatedResult[i].X509SerialNumber = xmlsecurity::bigIntegerToNumericString(encapsulatedCerts[i]->getSerialNumber());
                            encapsulatedResult[i].X509Subject = encapsulatedCerts[i]->getSubjectName();
                            auto const pCertificate(dynamic_cast<xmlsecurity::Certificate*>(encapsulatedCerts[i].get()));
                            assert(pCertificate); // this was just created by CheckX509Data
                            OUStringBuffer aBuffer;
                            comphelper::Base64::encode(aBuffer, pCertificate->getSHA256Thumbprint());
                            encapsulatedResult[i].CertDigest = aBuffer.makeStringAndClear();
                        }
                        datas.emplace_back(encapsulatedResult);
#else
                        // keep the X509Data stuff in datas but return the
                        // longer EncapsulatedX509Certificate chain
                        datas.emplace_back(tempResult);
#endif
                        certs = encapsulatedCerts; // overwrite this seems easier
                    }
                }
                else
                {
                    SAL_WARN("xmlsecurity.comp", "X509Data and EncapsulatedX509Certificate contain different certificates");
                }
            }
        }
    }

    // rInfo is a copy, update the original
    mpXSecController->UpdateSignatureInformation(rInfo.nSecurityId, std::move(datas));
    return certs;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
