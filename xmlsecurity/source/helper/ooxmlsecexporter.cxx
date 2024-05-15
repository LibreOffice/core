/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ooxmlsecexporter.hxx"

#include <algorithm>
#include <memory>
#include <string_view>

#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XHierarchicalStorageAccess.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>

#include <comphelper/attributelist.hxx>
#include <comphelper/ofopxmlhelper.hxx>
#include <o3tl/string_view.hxx>
#include <rtl/ref.hxx>
#include <sal/log.hxx>
#include <svx/xoutbmp.hxx>
#include <unotools/datetime.hxx>
#include <vcl/salctype.hxx>

#include <documentsignaturehelper.hxx>
#include <xsecctl.hxx>

using namespace com::sun::star;
using namespace css::xml::sax;

struct OOXMLSecExporter::Impl
{
private:
    const uno::Reference<uno::XComponentContext>& m_xComponentContext;
    const uno::Reference<embed::XStorage>& m_xRootStorage;
    const uno::Reference<xml::sax::XDocumentHandler>& m_xDocumentHandler;
    const SignatureInformation& m_rInformation;
    OUString m_aSignatureTimeValue;

public:
    Impl(const uno::Reference<uno::XComponentContext>& xComponentContext,
         const uno::Reference<embed::XStorage>& xRootStorage,
         const uno::Reference<xml::sax::XDocumentHandler>& xDocumentHandler,
         const SignatureInformation& rInformation)
        : m_xComponentContext(xComponentContext)
        , m_xRootStorage(xRootStorage)
        , m_xDocumentHandler(xDocumentHandler)
        , m_rInformation(rInformation)
    {
    }

    /// Should we intentionally not sign this stream?
    static bool isOOXMLDenylist(std::u16string_view rStreamName);
    /// Should we intentionally not sign this relation type?
    static bool isOOXMLRelationDenylist(const OUString& rRelationName);

    const uno::Reference<xml::sax::XDocumentHandler>& getDocumentHandler() const
    {
        return m_xDocumentHandler;
    }

    void writeSignature();
    void writeSignedInfo();
    void writeCanonicalizationMethod();
    void writeCanonicalizationTransform();
    void writeSignatureMethod();
    void writeSignedInfoReferences();
    void writeSignatureValue();
    void writeKeyInfo();
    void writePackageObject();
    void writeManifest();
    void writeRelationshipTransform(const OUString& rURI);
    /// Writes <SignatureProperties> inside idPackageObject.
    void writePackageObjectSignatureProperties();
    /// Writes a single <Reference> inside <Manifest>.
    void writeManifestReference(const SignatureReferenceInformation& rReference);
    void writeOfficeObject();
    /// Writes <SignatureInfoV1>.
    void writeSignatureInfo();
    void writePackageSignature();
    void writeSignatureLineImages();
};

bool OOXMLSecExporter::Impl::isOOXMLDenylist(std::u16string_view rStreamName)
{
    static const std::initializer_list<std::u16string_view> vDenylist
        = { u"/%5BContent_Types%5D.xml", u"/docProps/app.xml", u"/docProps/core.xml",
            // Don't attempt to sign other signatures for now.
            u"/_xmlsignatures" };
    // Just check the prefix, as we don't care about the content type part of the stream name.
    return std::any_of(vDenylist.begin(), vDenylist.end(),
                       [&](const std::u16string_view& rLiteral) {
                           return o3tl::starts_with(rStreamName, rLiteral);
                       });
}

bool OOXMLSecExporter::Impl::isOOXMLRelationDenylist(const OUString& rRelationName)
{
    static const std::initializer_list<std::u16string_view> vDenylist = {
        u"http://schemas.openxmlformats.org/officeDocument/2006/relationships/extended-properties",
        u"http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties",
        u"http://schemas.openxmlformats.org/package/2006/relationships/digital-signature/origin"
    };
    return std::find(vDenylist.begin(), vDenylist.end(), rRelationName) != vDenylist.end();
}

void OOXMLSecExporter::Impl::writeSignedInfo()
{
    m_xDocumentHandler->startElement(u"SignedInfo"_ustr, uno::Reference<xml::sax::XAttributeList>(
                                                             new comphelper::AttributeList()));

    writeCanonicalizationMethod();
    writeSignatureMethod();
    writeSignedInfoReferences();

    m_xDocumentHandler->endElement(u"SignedInfo"_ustr);
}

void OOXMLSecExporter::Impl::writeCanonicalizationMethod()
{
    rtl::Reference<comphelper::AttributeList> pAttributeList(new comphelper::AttributeList());
    pAttributeList->AddAttribute(u"Algorithm"_ustr, ALGO_C14N);
    m_xDocumentHandler->startElement(u"CanonicalizationMethod"_ustr,
                                     uno::Reference<xml::sax::XAttributeList>(pAttributeList));
    m_xDocumentHandler->endElement(u"CanonicalizationMethod"_ustr);
}

void OOXMLSecExporter::Impl::writeCanonicalizationTransform()
{
    rtl::Reference<comphelper::AttributeList> pAttributeList(new comphelper::AttributeList());
    pAttributeList->AddAttribute(u"Algorithm"_ustr, ALGO_C14N);
    m_xDocumentHandler->startElement(u"Transform"_ustr,
                                     uno::Reference<xml::sax::XAttributeList>(pAttributeList));
    m_xDocumentHandler->endElement(u"Transform"_ustr);
}

void OOXMLSecExporter::Impl::writeSignatureMethod()
{
    rtl::Reference<comphelper::AttributeList> pAttributeList(new comphelper::AttributeList());

    if (m_rInformation.eAlgorithmID == svl::crypto::SignatureMethodAlgorithm::ECDSA)
        pAttributeList->AddAttribute(u"Algorithm"_ustr, ALGO_ECDSASHA256);
    else
        pAttributeList->AddAttribute(u"Algorithm"_ustr, ALGO_RSASHA256);

    m_xDocumentHandler->startElement(u"SignatureMethod"_ustr,
                                     uno::Reference<xml::sax::XAttributeList>(pAttributeList));
    m_xDocumentHandler->endElement(u"SignatureMethod"_ustr);
}

void OOXMLSecExporter::Impl::writeSignedInfoReferences()
{
    const SignatureReferenceInformations& rReferences = m_rInformation.vSignatureReferenceInfors;
    for (const SignatureReferenceInformation& rReference : rReferences)
    {
        if (rReference.nType == SignatureReferenceType::SAMEDOCUMENT)
        {
            {
                rtl::Reference<comphelper::AttributeList> pAttributeList(
                    new comphelper::AttributeList());
                if (!rReference.ouURI.startsWith("idSignedProperties"))
                    pAttributeList->AddAttribute(u"Type"_ustr,
                                                 u"http://www.w3.org/2000/09/xmldsig#Object"_ustr);
                else
                    pAttributeList->AddAttribute(
                        u"Type"_ustr, u"http://uri.etsi.org/01903#SignedProperties"_ustr);
                pAttributeList->AddAttribute(u"URI"_ustr, "#" + rReference.ouURI);
                m_xDocumentHandler->startElement(
                    u"Reference"_ustr, uno::Reference<xml::sax::XAttributeList>(pAttributeList));
            }
            if (rReference.ouURI.startsWith("idSignedProperties"))
            {
                m_xDocumentHandler->startElement(
                    u"Transforms"_ustr,
                    uno::Reference<xml::sax::XAttributeList>(new comphelper::AttributeList()));
                writeCanonicalizationTransform();
                m_xDocumentHandler->endElement(u"Transforms"_ustr);
            }

            DocumentSignatureHelper::writeDigestMethod(m_xDocumentHandler);
            m_xDocumentHandler->startElement(
                u"DigestValue"_ustr,
                uno::Reference<xml::sax::XAttributeList>(new comphelper::AttributeList()));
            m_xDocumentHandler->characters(rReference.ouDigestValue);
            m_xDocumentHandler->endElement(u"DigestValue"_ustr);
            m_xDocumentHandler->endElement(u"Reference"_ustr);
        }
    }
}

void OOXMLSecExporter::Impl::writeSignatureValue()
{
    m_xDocumentHandler->startElement(
        u"SignatureValue"_ustr,
        uno::Reference<xml::sax::XAttributeList>(new comphelper::AttributeList()));
    m_xDocumentHandler->characters(m_rInformation.ouSignatureValue);
    m_xDocumentHandler->endElement(u"SignatureValue"_ustr);
}

void OOXMLSecExporter::Impl::writeKeyInfo()
{
    m_xDocumentHandler->startElement(
        u"KeyInfo"_ustr, uno::Reference<xml::sax::XAttributeList>(new comphelper::AttributeList()));
    assert(m_rInformation.GetSigningCertificate());
    for (auto const& rData : m_rInformation.X509Datas)
    {
        m_xDocumentHandler->startElement(u"X509Data"_ustr, uno::Reference<xml::sax::XAttributeList>(
                                                               new comphelper::AttributeList()));
        for (auto const& it : rData)
        {
            m_xDocumentHandler->startElement(
                u"X509Certificate"_ustr,
                uno::Reference<xml::sax::XAttributeList>(new comphelper::AttributeList()));
            m_xDocumentHandler->characters(it.X509Certificate);
            m_xDocumentHandler->endElement(u"X509Certificate"_ustr);
        }
        m_xDocumentHandler->endElement(u"X509Data"_ustr);
    }
    m_xDocumentHandler->endElement(u"KeyInfo"_ustr);
}

void OOXMLSecExporter::Impl::writePackageObject()
{
    rtl::Reference<comphelper::AttributeList> pAttributeList(new comphelper::AttributeList());
    pAttributeList->AddAttribute(u"Id"_ustr, "idPackageObject_" + m_rInformation.ouSignatureId);
    m_xDocumentHandler->startElement(u"Object"_ustr,
                                     uno::Reference<xml::sax::XAttributeList>(pAttributeList));

    writeManifest();
    writePackageObjectSignatureProperties();

    m_xDocumentHandler->endElement(u"Object"_ustr);
}

void OOXMLSecExporter::Impl::writeManifest()
{
    m_xDocumentHandler->startElement(u"Manifest"_ustr, uno::Reference<xml::sax::XAttributeList>(
                                                           new comphelper::AttributeList()));
    const SignatureReferenceInformations& rReferences = m_rInformation.vSignatureReferenceInfors;
    for (const SignatureReferenceInformation& rReference : rReferences)
    {
        if (rReference.nType != SignatureReferenceType::SAMEDOCUMENT)
        {
            if (OOXMLSecExporter::Impl::isOOXMLDenylist(rReference.ouURI))
                continue;

            writeManifestReference(rReference);
        }
    }
    m_xDocumentHandler->endElement(u"Manifest"_ustr);
}

void OOXMLSecExporter::Impl::writeRelationshipTransform(const OUString& rURI)
{
    uno::Reference<embed::XHierarchicalStorageAccess> xHierarchicalStorageAccess(m_xRootStorage,
                                                                                 uno::UNO_QUERY);
    uno::Reference<io::XInputStream> xRelStream(
        xHierarchicalStorageAccess->openStreamElementByHierarchicalName(rURI,
                                                                        embed::ElementModes::READ),
        uno::UNO_QUERY);
    {
        rtl::Reference<comphelper::AttributeList> pAttributeList(new comphelper::AttributeList());
        pAttributeList->AddAttribute(u"Algorithm"_ustr, ALGO_RELATIONSHIP);
        m_xDocumentHandler->startElement(u"Transform"_ustr,
                                         uno::Reference<xml::sax::XAttributeList>(pAttributeList));
    }

    const uno::Sequence<uno::Sequence<beans::StringPair>> aRelationsInfo
        = comphelper::OFOPXMLHelper::ReadRelationsInfoSequence(xRelStream, rURI,
                                                               m_xComponentContext);
    for (const uno::Sequence<beans::StringPair>& rPairs : aRelationsInfo)
    {
        OUString aId;
        OUString aType;
        for (const beans::StringPair& rPair : rPairs)
        {
            if (rPair.First == "Id")
                aId = rPair.Second;
            else if (rPair.First == "Type")
                aType = rPair.Second;
        }

        if (OOXMLSecExporter::Impl::isOOXMLRelationDenylist(aType))
            continue;

        rtl::Reference<comphelper::AttributeList> pAttributeList(new comphelper::AttributeList());
        pAttributeList->AddAttribute(u"xmlns:mdssi"_ustr, NS_MDSSI);
        pAttributeList->AddAttribute(u"SourceId"_ustr, aId);
        m_xDocumentHandler->startElement(u"mdssi:RelationshipReference"_ustr,
                                         uno::Reference<xml::sax::XAttributeList>(pAttributeList));
        m_xDocumentHandler->endElement(u"mdssi:RelationshipReference"_ustr);
    }

    m_xDocumentHandler->endElement(u"Transform"_ustr);
}

void OOXMLSecExporter::Impl::writePackageObjectSignatureProperties()
{
    m_xDocumentHandler->startElement(
        u"SignatureProperties"_ustr,
        uno::Reference<xml::sax::XAttributeList>(new comphelper::AttributeList()));
    {
        rtl::Reference<comphelper::AttributeList> pAttributeList(new comphelper::AttributeList());
        pAttributeList->AddAttribute(u"Id"_ustr, "idSignatureTime_" + m_rInformation.ouSignatureId);
        pAttributeList->AddAttribute(u"Target"_ustr, "#" + m_rInformation.ouSignatureId);
        m_xDocumentHandler->startElement(u"SignatureProperty"_ustr,
                                         uno::Reference<xml::sax::XAttributeList>(pAttributeList));
    }
    {
        rtl::Reference<comphelper::AttributeList> pAttributeList(new comphelper::AttributeList());
        pAttributeList->AddAttribute(u"xmlns:mdssi"_ustr, NS_MDSSI);
        m_xDocumentHandler->startElement(u"mdssi:SignatureTime"_ustr,
                                         uno::Reference<xml::sax::XAttributeList>(pAttributeList));
    }
    m_xDocumentHandler->startElement(u"mdssi:Format"_ustr, uno::Reference<xml::sax::XAttributeList>(
                                                               new comphelper::AttributeList()));
    m_xDocumentHandler->characters(u"YYYY-MM-DDThh:mm:ssTZD"_ustr);
    m_xDocumentHandler->endElement(u"mdssi:Format"_ustr);

    m_xDocumentHandler->startElement(u"mdssi:Value"_ustr, uno::Reference<xml::sax::XAttributeList>(
                                                              new comphelper::AttributeList()));
    if (!m_rInformation.ouDateTime.isEmpty())
        m_aSignatureTimeValue = m_rInformation.ouDateTime;
    else
    {
        m_aSignatureTimeValue = utl::toISO8601(m_rInformation.stDateTime);
        // Ignore sub-seconds.
        sal_Int32 nCommaPos = m_aSignatureTimeValue.indexOf(',');
        if (nCommaPos != -1)
        {
            m_aSignatureTimeValue
                = OUString::Concat(m_aSignatureTimeValue.subView(0, nCommaPos)) + "Z";
        }
    }
    m_xDocumentHandler->characters(m_aSignatureTimeValue);
    m_xDocumentHandler->endElement(u"mdssi:Value"_ustr);

    m_xDocumentHandler->endElement(u"mdssi:SignatureTime"_ustr);
    m_xDocumentHandler->endElement(u"SignatureProperty"_ustr);
    m_xDocumentHandler->endElement(u"SignatureProperties"_ustr);
}

void OOXMLSecExporter::Impl::writeManifestReference(const SignatureReferenceInformation& rReference)
{
    rtl::Reference<comphelper::AttributeList> pAttributeList(new comphelper::AttributeList());
    pAttributeList->AddAttribute(u"URI"_ustr, rReference.ouURI);
    m_xDocumentHandler->startElement(u"Reference"_ustr,
                                     uno::Reference<xml::sax::XAttributeList>(pAttributeList));

    // Transforms
    if (rReference.ouURI.endsWith(
            "?ContentType=application/vnd.openxmlformats-package.relationships+xml"))
    {
        OUString aURI = rReference.ouURI;
        // Ignore leading slash.
        if (aURI.startsWith("/"))
            aURI = aURI.copy(1);
        // Ignore query part of the URI.
        sal_Int32 nQueryPos = aURI.indexOf('?');
        if (nQueryPos != -1)
            aURI = aURI.copy(0, nQueryPos);

        m_xDocumentHandler->startElement(
            u"Transforms"_ustr,
            uno::Reference<xml::sax::XAttributeList>(new comphelper::AttributeList()));

        writeRelationshipTransform(aURI);
        writeCanonicalizationTransform();

        m_xDocumentHandler->endElement(u"Transforms"_ustr);
    }

    DocumentSignatureHelper::writeDigestMethod(m_xDocumentHandler);
    m_xDocumentHandler->startElement(u"DigestValue"_ustr, uno::Reference<xml::sax::XAttributeList>(
                                                              new comphelper::AttributeList()));
    m_xDocumentHandler->characters(rReference.ouDigestValue);
    m_xDocumentHandler->endElement(u"DigestValue"_ustr);
    m_xDocumentHandler->endElement(u"Reference"_ustr);
}

void OOXMLSecExporter::Impl::writeOfficeObject()
{
    {
        rtl::Reference<comphelper::AttributeList> pAttributeList(new comphelper::AttributeList());
        pAttributeList->AddAttribute(u"Id"_ustr, "idOfficeObject_" + m_rInformation.ouSignatureId);
        m_xDocumentHandler->startElement(u"Object"_ustr,
                                         uno::Reference<xml::sax::XAttributeList>(pAttributeList));
    }
    m_xDocumentHandler->startElement(
        u"SignatureProperties"_ustr,
        uno::Reference<xml::sax::XAttributeList>(new comphelper::AttributeList()));
    {
        rtl::Reference<comphelper::AttributeList> pAttributeList(new comphelper::AttributeList());
        pAttributeList->AddAttribute(u"Id"_ustr,
                                     "idOfficeV1Details_" + m_rInformation.ouSignatureId);
        pAttributeList->AddAttribute(u"Target"_ustr, "#" + m_rInformation.ouSignatureId);
        m_xDocumentHandler->startElement(u"SignatureProperty"_ustr,
                                         uno::Reference<xml::sax::XAttributeList>(pAttributeList));
    }
    writeSignatureInfo();
    m_xDocumentHandler->endElement(u"SignatureProperty"_ustr);
    m_xDocumentHandler->endElement(u"SignatureProperties"_ustr);
    m_xDocumentHandler->endElement(u"Object"_ustr);
}

void OOXMLSecExporter::Impl::writeSignatureInfo()
{
    rtl::Reference<comphelper::AttributeList> pAttributeList(new comphelper::AttributeList());
    pAttributeList->AddAttribute(u"xmlns"_ustr,
                                 u"http://schemas.microsoft.com/office/2006/digsig"_ustr);
    m_xDocumentHandler->startElement(u"SignatureInfoV1"_ustr,
                                     uno::Reference<xml::sax::XAttributeList>(pAttributeList));

    m_xDocumentHandler->startElement(
        u"SetupID"_ustr, uno::Reference<xml::sax::XAttributeList>(new comphelper::AttributeList()));
    m_xDocumentHandler->characters(m_rInformation.ouSignatureLineId);
    m_xDocumentHandler->endElement(u"SetupID"_ustr);
    m_xDocumentHandler->startElement(
        u"SignatureText"_ustr,
        uno::Reference<xml::sax::XAttributeList>(new comphelper::AttributeList()));
    m_xDocumentHandler->endElement(u"SignatureText"_ustr);
    m_xDocumentHandler->startElement(
        u"SignatureImage"_ustr,
        uno::Reference<xml::sax::XAttributeList>(new comphelper::AttributeList()));
    m_xDocumentHandler->endElement(u"SignatureImage"_ustr);
    m_xDocumentHandler->startElement(
        u"SignatureComments"_ustr,
        uno::Reference<xml::sax::XAttributeList>(new comphelper::AttributeList()));
    m_xDocumentHandler->characters(m_rInformation.ouDescription);
    m_xDocumentHandler->endElement(u"SignatureComments"_ustr);
    // Just hardcode something valid according to [MS-OFFCRYPTO].
    m_xDocumentHandler->startElement(
        u"WindowsVersion"_ustr,
        uno::Reference<xml::sax::XAttributeList>(new comphelper::AttributeList()));
    m_xDocumentHandler->characters(u"6.1"_ustr);
    m_xDocumentHandler->endElement(u"WindowsVersion"_ustr);
    m_xDocumentHandler->startElement(
        u"OfficeVersion"_ustr,
        uno::Reference<xml::sax::XAttributeList>(new comphelper::AttributeList()));
    m_xDocumentHandler->characters(u"16.0"_ustr);
    m_xDocumentHandler->endElement(u"OfficeVersion"_ustr);
    m_xDocumentHandler->startElement(
        u"ApplicationVersion"_ustr,
        uno::Reference<xml::sax::XAttributeList>(new comphelper::AttributeList()));
    m_xDocumentHandler->characters(u"16.0"_ustr);
    m_xDocumentHandler->endElement(u"ApplicationVersion"_ustr);
    m_xDocumentHandler->startElement(u"Monitors"_ustr, uno::Reference<xml::sax::XAttributeList>(
                                                           new comphelper::AttributeList()));
    m_xDocumentHandler->characters(u"1"_ustr);
    m_xDocumentHandler->endElement(u"Monitors"_ustr);
    m_xDocumentHandler->startElement(
        u"HorizontalResolution"_ustr,
        uno::Reference<xml::sax::XAttributeList>(new comphelper::AttributeList()));
    m_xDocumentHandler->characters(u"1280"_ustr);
    m_xDocumentHandler->endElement(u"HorizontalResolution"_ustr);
    m_xDocumentHandler->startElement(
        u"VerticalResolution"_ustr,
        uno::Reference<xml::sax::XAttributeList>(new comphelper::AttributeList()));
    m_xDocumentHandler->characters(u"800"_ustr);
    m_xDocumentHandler->endElement(u"VerticalResolution"_ustr);
    m_xDocumentHandler->startElement(u"ColorDepth"_ustr, uno::Reference<xml::sax::XAttributeList>(
                                                             new comphelper::AttributeList()));
    m_xDocumentHandler->characters(u"32"_ustr);
    m_xDocumentHandler->endElement(u"ColorDepth"_ustr);
    m_xDocumentHandler->startElement(
        u"SignatureProviderId"_ustr,
        uno::Reference<xml::sax::XAttributeList>(new comphelper::AttributeList()));
    m_xDocumentHandler->characters(u"{00000000-0000-0000-0000-000000000000}"_ustr);
    m_xDocumentHandler->endElement(u"SignatureProviderId"_ustr);
    m_xDocumentHandler->startElement(
        u"SignatureProviderUrl"_ustr,
        uno::Reference<xml::sax::XAttributeList>(new comphelper::AttributeList()));
    m_xDocumentHandler->endElement(u"SignatureProviderUrl"_ustr);
    m_xDocumentHandler->startElement(
        u"SignatureProviderDetails"_ustr,
        uno::Reference<xml::sax::XAttributeList>(new comphelper::AttributeList()));
    m_xDocumentHandler->characters(
        u"9"_ustr); // This is what MSO 2016 writes, though [MS-OFFCRYPTO] doesn't document what the value means.
    m_xDocumentHandler->endElement(u"SignatureProviderDetails"_ustr);
    m_xDocumentHandler->startElement(
        u"SignatureType"_ustr,
        uno::Reference<xml::sax::XAttributeList>(new comphelper::AttributeList()));
    m_xDocumentHandler->characters(u"2"_ustr);
    m_xDocumentHandler->endElement(u"SignatureType"_ustr);

    m_xDocumentHandler->endElement(u"SignatureInfoV1"_ustr);
}

void OOXMLSecExporter::Impl::writePackageSignature()
{
    m_xDocumentHandler->startElement(
        u"Object"_ustr, uno::Reference<xml::sax::XAttributeList>(new comphelper::AttributeList()));
    {
        rtl::Reference<comphelper::AttributeList> pAttributeList(new comphelper::AttributeList());
        pAttributeList->AddAttribute(u"xmlns:xd"_ustr, NS_XD);
        pAttributeList->AddAttribute(u"Target"_ustr, "#" + m_rInformation.ouSignatureId);
        m_xDocumentHandler->startElement(u"xd:QualifyingProperties"_ustr,
                                         uno::Reference<xml::sax::XAttributeList>(pAttributeList));
    }

    DocumentSignatureHelper::writeSignedProperties(m_xDocumentHandler, m_rInformation,
                                                   m_aSignatureTimeValue, false);

    m_xDocumentHandler->endElement(u"xd:QualifyingProperties"_ustr);
    m_xDocumentHandler->endElement(u"Object"_ustr);
}

void OOXMLSecExporter::Impl::writeSignatureLineImages()
{
    if (m_rInformation.aValidSignatureImage.is())
    {
        rtl::Reference<comphelper::AttributeList> pAttributeList(new comphelper::AttributeList());
        pAttributeList->AddAttribute(u"Id"_ustr, u"idValidSigLnImg"_ustr);
        m_xDocumentHandler->startElement(u"Object"_ustr,
                                         uno::Reference<xml::sax::XAttributeList>(pAttributeList));
        OUString aGraphicInBase64;
        Graphic aGraphic(m_rInformation.aValidSignatureImage);
        if (!XOutBitmap::GraphicToBase64(aGraphic, aGraphicInBase64, false, ConvertDataFormat::EMF))
            SAL_WARN("xmlsecurity.helper", "could not convert graphic to base64");
        m_xDocumentHandler->characters(aGraphicInBase64);
        m_xDocumentHandler->endElement(u"Object"_ustr);
    }
    if (!m_rInformation.aInvalidSignatureImage.is())
        return;

    rtl::Reference<comphelper::AttributeList> pAttributeList(new comphelper::AttributeList());
    pAttributeList->AddAttribute(u"Id"_ustr, u"idInvalidSigLnImg"_ustr);
    m_xDocumentHandler->startElement(u"Object"_ustr,
                                     uno::Reference<xml::sax::XAttributeList>(pAttributeList));
    OUString aGraphicInBase64;
    Graphic aGraphic(m_rInformation.aInvalidSignatureImage);
    if (!XOutBitmap::GraphicToBase64(aGraphic, aGraphicInBase64, false, ConvertDataFormat::EMF))
        SAL_WARN("xmlsecurity.helper", "could not convert graphic to base64");
    m_xDocumentHandler->characters(aGraphicInBase64);
    m_xDocumentHandler->endElement(u"Object"_ustr);
}

void OOXMLSecExporter::Impl::writeSignature()
{
    rtl::Reference<comphelper::AttributeList> pAttributeList(new comphelper::AttributeList());
    pAttributeList->AddAttribute(u"xmlns"_ustr, NS_XMLDSIG);
    pAttributeList->AddAttribute(u"Id"_ustr, m_rInformation.ouSignatureId);
    getDocumentHandler()->startElement(u"Signature"_ustr,
                                       uno::Reference<xml::sax::XAttributeList>(pAttributeList));

    writeSignedInfo();
    writeSignatureValue();
    writeKeyInfo();
    writePackageObject();
    writeOfficeObject();
    writePackageSignature();
    writeSignatureLineImages();

    getDocumentHandler()->endElement(u"Signature"_ustr);
}

OOXMLSecExporter::OOXMLSecExporter(
    const uno::Reference<uno::XComponentContext>& xComponentContext,
    const uno::Reference<embed::XStorage>& xRootStorage,
    const uno::Reference<xml::sax::XDocumentHandler>& xDocumentHandler,
    const SignatureInformation& rInformation)
    : m_pImpl(
          std::make_unique<Impl>(xComponentContext, xRootStorage, xDocumentHandler, rInformation))
{
}

OOXMLSecExporter::~OOXMLSecExporter() = default;

void OOXMLSecExporter::writeSignature() { m_pImpl->writeSignature(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
