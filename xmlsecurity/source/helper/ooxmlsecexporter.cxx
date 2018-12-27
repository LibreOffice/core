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

#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XHierarchicalStorageAccess.hpp>
#include <com/sun/star/beans/StringPair.hpp>

#include <comphelper/ofopxmlhelper.hxx>
#include <rtl/ref.hxx>
#include <sal/log.hxx>
#include <svx/xoutbmp.hxx>
#include <unotools/datetime.hxx>
#include <vcl/salctype.hxx>
#include <xmloff/attrlist.hxx>

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
    static bool isOOXMLBlacklist(const OUString& rStreamName);
    /// Should we intentionally not sign this relation type?
    static bool isOOXMLRelationBlacklist(const OUString& rRelationName);

    const uno::Reference<xml::sax::XDocumentHandler>& getDocumentHandler() const
    {
        return m_xDocumentHandler;
    }

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

bool OOXMLSecExporter::Impl::isOOXMLBlacklist(const OUString& rStreamName)
{
    static const std::initializer_list<OUStringLiteral> vBlacklist =
    {
        "/%5BContent_Types%5D.xml",
        "/docProps/app.xml",
        "/docProps/core.xml",
        // Don't attempt to sign other signatures for now.
        "/_xmlsignatures"
    };
    // Just check the prefix, as we don't care about the content type part of the stream name.
    return std::any_of(vBlacklist.begin(), vBlacklist.end(), [&](const OUStringLiteral& rLiteral)
    {
        return rStreamName.startsWith(rLiteral);
    });
}

bool OOXMLSecExporter::Impl::isOOXMLRelationBlacklist(const OUString& rRelationName)
{
    static const std::initializer_list<OUStringLiteral> vBlacklist =
    {
        "http://schemas.openxmlformats.org/officeDocument/2006/relationships/extended-properties",
        "http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties",
        "http://schemas.openxmlformats.org/package/2006/relationships/digital-signature/origin"
    };
    return std::find(vBlacklist.begin(), vBlacklist.end(), rRelationName) != vBlacklist.end();
}

void OOXMLSecExporter::Impl::writeSignedInfo()
{
    m_xDocumentHandler->startElement("SignedInfo", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));

    writeCanonicalizationMethod();
    writeSignatureMethod();
    writeSignedInfoReferences();

    m_xDocumentHandler->endElement("SignedInfo");
}

void OOXMLSecExporter::Impl::writeCanonicalizationMethod()
{
    rtl::Reference<SvXMLAttributeList> pAttributeList(new SvXMLAttributeList());
    pAttributeList->AddAttribute("Algorithm", ALGO_C14N);
    m_xDocumentHandler->startElement("CanonicalizationMethod", uno::Reference<xml::sax::XAttributeList>(pAttributeList.get()));
    m_xDocumentHandler->endElement("CanonicalizationMethod");

}

void OOXMLSecExporter::Impl::writeCanonicalizationTransform()
{
    rtl::Reference<SvXMLAttributeList> pAttributeList(new SvXMLAttributeList());
    pAttributeList->AddAttribute("Algorithm", ALGO_C14N);
    m_xDocumentHandler->startElement("Transform", uno::Reference<xml::sax::XAttributeList>(pAttributeList.get()));
    m_xDocumentHandler->endElement("Transform");

}

void OOXMLSecExporter::Impl::writeSignatureMethod()
{
    rtl::Reference<SvXMLAttributeList> pAttributeList(new SvXMLAttributeList());

    if (m_rInformation.eAlgorithmID == svl::crypto::SignatureMethodAlgorithm::ECDSA)
        pAttributeList->AddAttribute("Algorithm", ALGO_ECDSASHA256);
    else
        pAttributeList->AddAttribute("Algorithm", ALGO_RSASHA256);

    m_xDocumentHandler->startElement("SignatureMethod", uno::Reference<xml::sax::XAttributeList>(pAttributeList.get()));
    m_xDocumentHandler->endElement("SignatureMethod");
}

void OOXMLSecExporter::Impl::writeSignedInfoReferences()
{
    const SignatureReferenceInformations& rReferences = m_rInformation.vSignatureReferenceInfors;
    for (const SignatureReferenceInformation& rReference : rReferences)
    {
        if (rReference.nType == SignatureReferenceType::SAMEDOCUMENT)
        {
            {
                rtl::Reference<SvXMLAttributeList> pAttributeList(new SvXMLAttributeList());
                if (rReference.ouURI != "idSignedProperties")
                    pAttributeList->AddAttribute("Type", "http://www.w3.org/2000/09/xmldsig#Object");
                else
                    pAttributeList->AddAttribute("Type", "http://uri.etsi.org/01903#SignedProperties");
                pAttributeList->AddAttribute("URI", "#" + rReference.ouURI);
                m_xDocumentHandler->startElement("Reference", uno::Reference<xml::sax::XAttributeList>(pAttributeList.get()));
            }
            if (rReference.ouURI == "idSignedProperties")
            {
                m_xDocumentHandler->startElement("Transforms", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
                writeCanonicalizationTransform();
                m_xDocumentHandler->endElement("Transforms");
            }

            DocumentSignatureHelper::writeDigestMethod(m_xDocumentHandler);
            m_xDocumentHandler->startElement("DigestValue", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
            m_xDocumentHandler->characters(rReference.ouDigestValue);
            m_xDocumentHandler->endElement("DigestValue");
            m_xDocumentHandler->endElement("Reference");
        }
    }
}

void OOXMLSecExporter::Impl::writeSignatureValue()
{
    m_xDocumentHandler->startElement("SignatureValue", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    m_xDocumentHandler->characters(m_rInformation.ouSignatureValue);
    m_xDocumentHandler->endElement("SignatureValue");
}

void OOXMLSecExporter::Impl::writeKeyInfo()
{
    m_xDocumentHandler->startElement("KeyInfo", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    m_xDocumentHandler->startElement("X509Data", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    m_xDocumentHandler->startElement("X509Certificate", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    m_xDocumentHandler->characters(m_rInformation.ouX509Certificate);
    m_xDocumentHandler->endElement("X509Certificate");
    m_xDocumentHandler->endElement("X509Data");
    m_xDocumentHandler->endElement("KeyInfo");
}

void OOXMLSecExporter::Impl::writePackageObject()
{
    rtl::Reference<SvXMLAttributeList> pAttributeList(new SvXMLAttributeList());
    pAttributeList->AddAttribute("Id", "idPackageObject");
    m_xDocumentHandler->startElement("Object", uno::Reference<xml::sax::XAttributeList>(pAttributeList.get()));

    writeManifest();
    writePackageObjectSignatureProperties();

    m_xDocumentHandler->endElement("Object");
}

void OOXMLSecExporter::Impl::writeManifest()
{
    m_xDocumentHandler->startElement("Manifest", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    const SignatureReferenceInformations& rReferences = m_rInformation.vSignatureReferenceInfors;
    for (const SignatureReferenceInformation& rReference : rReferences)
    {
        if (rReference.nType != SignatureReferenceType::SAMEDOCUMENT)
        {
            if (OOXMLSecExporter::Impl::isOOXMLBlacklist(rReference.ouURI))
                continue;

            writeManifestReference(rReference);
        }
    }
    m_xDocumentHandler->endElement("Manifest");
}

void OOXMLSecExporter::Impl::writeRelationshipTransform(const OUString& rURI)
{
    uno::Reference<embed::XHierarchicalStorageAccess> xHierarchicalStorageAccess(m_xRootStorage, uno::UNO_QUERY);
    uno::Reference<io::XInputStream> xRelStream(xHierarchicalStorageAccess->openStreamElementByHierarchicalName(rURI, embed::ElementModes::READ), uno::UNO_QUERY);
    {
        rtl::Reference<SvXMLAttributeList> pAttributeList(new SvXMLAttributeList());
        pAttributeList->AddAttribute("Algorithm", ALGO_RELATIONSHIP);
        m_xDocumentHandler->startElement("Transform", uno::Reference<xml::sax::XAttributeList>(pAttributeList.get()));
    }

    uno::Sequence< uno::Sequence<beans::StringPair> > aRelationsInfo = comphelper::OFOPXMLHelper::ReadRelationsInfoSequence(xRelStream, rURI, m_xComponentContext);
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

        if (OOXMLSecExporter::Impl::isOOXMLRelationBlacklist(aType))
            continue;

        rtl::Reference<SvXMLAttributeList> pAttributeList(new SvXMLAttributeList());
        pAttributeList->AddAttribute("xmlns:mdssi", NS_MDSSI);
        pAttributeList->AddAttribute("SourceId", aId);
        m_xDocumentHandler->startElement("mdssi:RelationshipReference", uno::Reference<xml::sax::XAttributeList>(pAttributeList.get()));
        m_xDocumentHandler->endElement("mdssi:RelationshipReference");
    }

    m_xDocumentHandler->endElement("Transform");
}

void OOXMLSecExporter::Impl::writePackageObjectSignatureProperties()
{
    m_xDocumentHandler->startElement("SignatureProperties", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    {
        rtl::Reference<SvXMLAttributeList> pAttributeList(new SvXMLAttributeList());
        pAttributeList->AddAttribute("Id", "idSignatureTime");
        pAttributeList->AddAttribute("Target", "#idPackageSignature");
        m_xDocumentHandler->startElement("SignatureProperty", uno::Reference<xml::sax::XAttributeList>(pAttributeList.get()));
    }
    {
        rtl::Reference<SvXMLAttributeList> pAttributeList(new SvXMLAttributeList());
        pAttributeList->AddAttribute("xmlns:mdssi", NS_MDSSI);
        m_xDocumentHandler->startElement("mdssi:SignatureTime", uno::Reference<xml::sax::XAttributeList>(pAttributeList.get()));
    }
    m_xDocumentHandler->startElement("mdssi:Format", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    m_xDocumentHandler->characters("YYYY-MM-DDThh:mm:ssTZD");
    m_xDocumentHandler->endElement("mdssi:Format");

    m_xDocumentHandler->startElement("mdssi:Value", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    if (!m_rInformation.ouDateTime.isEmpty())
        m_aSignatureTimeValue = m_rInformation.ouDateTime;
    else
    {
        m_aSignatureTimeValue = utl::toISO8601(m_rInformation.stDateTime);
        // Ignore sub-seconds.
        sal_Int32 nCommaPos = m_aSignatureTimeValue.indexOf(',');
        if (nCommaPos != -1)
        {
            m_aSignatureTimeValue = m_aSignatureTimeValue.copy(0, nCommaPos);
            m_aSignatureTimeValue += "Z";
        }
    }
    m_xDocumentHandler->characters(m_aSignatureTimeValue);
    m_xDocumentHandler->endElement("mdssi:Value");

    m_xDocumentHandler->endElement("mdssi:SignatureTime");
    m_xDocumentHandler->endElement("SignatureProperty");
    m_xDocumentHandler->endElement("SignatureProperties");
}

void OOXMLSecExporter::Impl::writeManifestReference(const SignatureReferenceInformation& rReference)
{
    rtl::Reference<SvXMLAttributeList> pAttributeList(new SvXMLAttributeList());
    pAttributeList->AddAttribute("URI", rReference.ouURI);
    m_xDocumentHandler->startElement("Reference", uno::Reference<xml::sax::XAttributeList>(pAttributeList.get()));

    // Transforms
    if (rReference.ouURI.endsWith("?ContentType=application/vnd.openxmlformats-package.relationships+xml"))
    {
        OUString aURI = rReference.ouURI;
        // Ignore leading slash.
        if (aURI.startsWith("/"))
            aURI = aURI.copy(1);
        // Ignore query part of the URI.
        sal_Int32 nQueryPos = aURI.indexOf('?');
        if (nQueryPos != -1)
            aURI = aURI.copy(0, nQueryPos);

        m_xDocumentHandler->startElement("Transforms", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));

        writeRelationshipTransform(aURI);
        writeCanonicalizationTransform();

        m_xDocumentHandler->endElement("Transforms");
    }

    DocumentSignatureHelper::writeDigestMethod(m_xDocumentHandler);
    m_xDocumentHandler->startElement("DigestValue", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    m_xDocumentHandler->characters(rReference.ouDigestValue);
    m_xDocumentHandler->endElement("DigestValue");
    m_xDocumentHandler->endElement("Reference");
}

void OOXMLSecExporter::Impl::writeOfficeObject()
{
    {
        rtl::Reference<SvXMLAttributeList> pAttributeList(new SvXMLAttributeList());
        pAttributeList->AddAttribute("Id", "idOfficeObject");
        m_xDocumentHandler->startElement("Object", uno::Reference<xml::sax::XAttributeList>(pAttributeList.get()));
    }
    m_xDocumentHandler->startElement("SignatureProperties", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    {
        rtl::Reference<SvXMLAttributeList> pAttributeList(new SvXMLAttributeList());
        pAttributeList->AddAttribute("Id", "idOfficeV1Details");
        pAttributeList->AddAttribute("Target", "#idPackageSignature");
        m_xDocumentHandler->startElement("SignatureProperty", uno::Reference<xml::sax::XAttributeList>(pAttributeList.get()));
    }
    writeSignatureInfo();
    m_xDocumentHandler->endElement("SignatureProperty");
    m_xDocumentHandler->endElement("SignatureProperties");
    m_xDocumentHandler->endElement("Object");
}

void OOXMLSecExporter::Impl::writeSignatureInfo()
{
    rtl::Reference<SvXMLAttributeList> pAttributeList(new SvXMLAttributeList());
    pAttributeList->AddAttribute("xmlns", "http://schemas.microsoft.com/office/2006/digsig");
    m_xDocumentHandler->startElement("SignatureInfoV1", uno::Reference<xml::sax::XAttributeList>(pAttributeList.get()));

    m_xDocumentHandler->startElement("SetupID", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    m_xDocumentHandler->characters(m_rInformation.ouSignatureLineId);
    m_xDocumentHandler->endElement("SetupID");
    m_xDocumentHandler->startElement("SignatureText", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    m_xDocumentHandler->endElement("SignatureText");
    m_xDocumentHandler->startElement("SignatureImage", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    m_xDocumentHandler->endElement("SignatureImage");
    m_xDocumentHandler->startElement("SignatureComments", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    m_xDocumentHandler->characters(m_rInformation.ouDescription);
    m_xDocumentHandler->endElement("SignatureComments");
    // Just hardcode something valid according to [MS-OFFCRYPTO].
    m_xDocumentHandler->startElement("WindowsVersion", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    m_xDocumentHandler->characters("6.1");
    m_xDocumentHandler->endElement("WindowsVersion");
    m_xDocumentHandler->startElement("OfficeVersion", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    m_xDocumentHandler->characters("16.0");
    m_xDocumentHandler->endElement("OfficeVersion");
    m_xDocumentHandler->startElement("ApplicationVersion", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    m_xDocumentHandler->characters("16.0");
    m_xDocumentHandler->endElement("ApplicationVersion");
    m_xDocumentHandler->startElement("Monitors", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    m_xDocumentHandler->characters("1");
    m_xDocumentHandler->endElement("Monitors");
    m_xDocumentHandler->startElement("HorizontalResolution", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    m_xDocumentHandler->characters("1280");
    m_xDocumentHandler->endElement("HorizontalResolution");
    m_xDocumentHandler->startElement("VerticalResolution", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    m_xDocumentHandler->characters("800");
    m_xDocumentHandler->endElement("VerticalResolution");
    m_xDocumentHandler->startElement("ColorDepth", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    m_xDocumentHandler->characters("32");
    m_xDocumentHandler->endElement("ColorDepth");
    m_xDocumentHandler->startElement("SignatureProviderId", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    m_xDocumentHandler->characters("{00000000-0000-0000-0000-000000000000}");
    m_xDocumentHandler->endElement("SignatureProviderId");
    m_xDocumentHandler->startElement("SignatureProviderUrl", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    m_xDocumentHandler->endElement("SignatureProviderUrl");
    m_xDocumentHandler->startElement("SignatureProviderDetails", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    m_xDocumentHandler->characters("9"); // This is what MSO 2016 writes, though [MS-OFFCRYPTO] doesn't document what the value means.
    m_xDocumentHandler->endElement("SignatureProviderDetails");
    m_xDocumentHandler->startElement("SignatureType", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    m_xDocumentHandler->characters("2");
    m_xDocumentHandler->endElement("SignatureType");

    m_xDocumentHandler->endElement("SignatureInfoV1");
}

void OOXMLSecExporter::Impl::writePackageSignature()
{
    m_xDocumentHandler->startElement("Object", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    {
        rtl::Reference<SvXMLAttributeList> pAttributeList(new SvXMLAttributeList());
        pAttributeList->AddAttribute("xmlns:xd", NS_XD);
        pAttributeList->AddAttribute("Target", "#idPackageSignature");
        m_xDocumentHandler->startElement("xd:QualifyingProperties", uno::Reference<xml::sax::XAttributeList>(pAttributeList.get()));
    }

    DocumentSignatureHelper::writeSignedProperties(m_xDocumentHandler, m_rInformation, m_aSignatureTimeValue, false);

    m_xDocumentHandler->endElement("xd:QualifyingProperties");
    m_xDocumentHandler->endElement("Object");
}

void OOXMLSecExporter::Impl::writeSignatureLineImages()
{
    if (m_rInformation.aValidSignatureImage.is())
    {
        rtl::Reference<SvXMLAttributeList> pAttributeList(new SvXMLAttributeList());
        pAttributeList->AddAttribute("Id", "idValidSigLnImg");
        m_xDocumentHandler->startElement(
            "Object", uno::Reference<xml::sax::XAttributeList>(pAttributeList.get()));
        OUString aGraphicInBase64;
        Graphic aGraphic(m_rInformation.aValidSignatureImage);
        if (!XOutBitmap::GraphicToBase64(aGraphic, aGraphicInBase64, false, ConvertDataFormat::EMF))
            SAL_WARN("xmlsecurity.helper", "could not convert graphic to base64");
        m_xDocumentHandler->characters(aGraphicInBase64);
        m_xDocumentHandler->endElement("Object");
    }
    if (m_rInformation.aInvalidSignatureImage.is())
    {
        rtl::Reference<SvXMLAttributeList> pAttributeList(new SvXMLAttributeList());
        pAttributeList->AddAttribute("Id", "idInvalidSigLnImg");
        m_xDocumentHandler->startElement(
            "Object", uno::Reference<xml::sax::XAttributeList>(pAttributeList.get()));
        OUString aGraphicInBase64;
        Graphic aGraphic(m_rInformation.aInvalidSignatureImage);
        if (!XOutBitmap::GraphicToBase64(aGraphic, aGraphicInBase64, false, ConvertDataFormat::EMF))
            SAL_WARN("xmlsecurity.helper", "could not convert graphic to base64");
        m_xDocumentHandler->characters(aGraphicInBase64);
        m_xDocumentHandler->endElement("Object");
    }
}

OOXMLSecExporter::OOXMLSecExporter(const uno::Reference<uno::XComponentContext>& xComponentContext,
                                   const uno::Reference<embed::XStorage>& xRootStorage,
                                   const uno::Reference<xml::sax::XDocumentHandler>& xDocumentHandler,
                                   const SignatureInformation& rInformation)
    : m_pImpl(std::make_unique<Impl>(xComponentContext, xRootStorage, xDocumentHandler, rInformation))
{
}

OOXMLSecExporter::~OOXMLSecExporter() = default;

void OOXMLSecExporter::writeSignature()
{
    rtl::Reference<SvXMLAttributeList> pAttributeList(new SvXMLAttributeList());
    pAttributeList->AddAttribute("xmlns", NS_XMLDSIG);
    pAttributeList->AddAttribute("Id", "idPackageSignature");
    m_pImpl->getDocumentHandler()->startElement("Signature", uno::Reference<xml::sax::XAttributeList>(pAttributeList.get()));

    m_pImpl->writeSignedInfo();
    m_pImpl->writeSignatureValue();
    m_pImpl->writeKeyInfo();
    m_pImpl->writePackageObject();
    m_pImpl->writeOfficeObject();
    m_pImpl->writePackageSignature();
    m_pImpl->writeSignatureLineImages();

    m_pImpl->getDocumentHandler()->endElement("Signature");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
