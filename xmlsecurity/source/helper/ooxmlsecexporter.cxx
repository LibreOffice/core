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

#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XHierarchicalStorageAccess.hpp>
#include <com/sun/star/beans/StringPair.hpp>

#include <comphelper/ofopxmlhelper.hxx>
#include <config_global.h>
#include <o3tl/make_unique.hxx>
#include <rtl/ref.hxx>
#include <unotools/datetime.hxx>
#include <xmloff/attrlist.hxx>

#include <xsecctl.hxx>

using namespace com::sun::star;

struct OOXMLSecExporter::Impl
{
    const uno::Reference<uno::XComponentContext>& m_xComponentContext;
    const uno::Reference<embed::XStorage>& m_xRootStorage;
    const uno::Reference<xml::sax::XDocumentHandler>& m_xDocumentHandler;
    const SignatureInformation& m_rInformation;
    OUString m_aSignatureTimeValue;

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

    void writeSignedInfo();
    void writeCanonicalizationMethod();
    void writeCanonicalizationTransform();
    void writeDigestMethod();
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
    void writeSignedProperties();
};

bool OOXMLSecExporter::Impl::isOOXMLBlacklist(const OUString& rStreamName)
{
#if !HAVE_BROKEN_STATIC_INITIALIZER_LIST
    static
#endif
    const std::initializer_list<OUStringLiteral> vBlacklist =
    {
        OUStringLiteral("/%5BContent_Types%5D.xml"),
        OUStringLiteral("/docProps/app.xml"),
        OUStringLiteral("/docProps/core.xml"),
        // Don't attempt to sign other signatures for now.
        OUStringLiteral("/_xmlsignatures")
    };
    // Just check the prefix, as we don't care about the content type part of the stream name.
    return std::find_if(vBlacklist.begin(), vBlacklist.end(), [&](const OUStringLiteral& rLiteral)
    {
        return rStreamName.startsWith(rLiteral);
    }) != vBlacklist.end();
}

bool OOXMLSecExporter::Impl::isOOXMLRelationBlacklist(const OUString& rRelationName)
{
#if !HAVE_BROKEN_STATIC_INITIALIZER_LIST
    static
#endif
    const std::initializer_list<OUStringLiteral> vBlacklist =
    {
        OUStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/extended-properties"),
        OUStringLiteral("http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties"),
        OUStringLiteral("http://schemas.openxmlformats.org/package/2006/relationships/digital-signature/origin")
    };
    return std::find(vBlacklist.begin(), vBlacklist.end(), rRelationName) != vBlacklist.end();
}

void OOXMLSecExporter::Impl::writeSignedInfo()
{
    m_xDocumentHandler->startElement(TAG_SIGNEDINFO, uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));

    writeCanonicalizationMethod();
    writeSignatureMethod();
    writeSignedInfoReferences();

    m_xDocumentHandler->endElement(TAG_SIGNEDINFO);
}

void OOXMLSecExporter::Impl::writeCanonicalizationMethod()
{
    rtl::Reference<SvXMLAttributeList> pAttributeList(new SvXMLAttributeList());
    pAttributeList->AddAttribute(ATTR_ALGORITHM, ALGO_C14N);
    m_xDocumentHandler->startElement(TAG_CANONICALIZATIONMETHOD, uno::Reference<xml::sax::XAttributeList>(pAttributeList.get()));
    m_xDocumentHandler->endElement(TAG_CANONICALIZATIONMETHOD);

}

void OOXMLSecExporter::Impl::writeCanonicalizationTransform()
{
    rtl::Reference<SvXMLAttributeList> pAttributeList(new SvXMLAttributeList());
    pAttributeList->AddAttribute(ATTR_ALGORITHM, ALGO_C14N);
    m_xDocumentHandler->startElement(TAG_TRANSFORM, uno::Reference<xml::sax::XAttributeList>(pAttributeList.get()));
    m_xDocumentHandler->endElement(TAG_TRANSFORM);

}

void OOXMLSecExporter::Impl::writeDigestMethod()
{
    rtl::Reference<SvXMLAttributeList> pAttributeList(new SvXMLAttributeList());
    pAttributeList->AddAttribute(ATTR_ALGORITHM, ALGO_XMLDSIGSHA256);
    m_xDocumentHandler->startElement(TAG_DIGESTMETHOD, uno::Reference<xml::sax::XAttributeList>(pAttributeList.get()));
    m_xDocumentHandler->endElement(TAG_DIGESTMETHOD);
}

void OOXMLSecExporter::Impl::writeSignatureMethod()
{
    rtl::Reference<SvXMLAttributeList> pAttributeList(new SvXMLAttributeList());
    pAttributeList->AddAttribute(ATTR_ALGORITHM, ALGO_RSASHA256);
    m_xDocumentHandler->startElement(TAG_SIGNATUREMETHOD, uno::Reference<xml::sax::XAttributeList>(pAttributeList.get()));
    m_xDocumentHandler->endElement(TAG_SIGNATUREMETHOD);
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
                pAttributeList->AddAttribute(ATTR_URI, CHAR_FRAGMENT + rReference.ouURI);
                m_xDocumentHandler->startElement(TAG_REFERENCE, uno::Reference<xml::sax::XAttributeList>(pAttributeList.get()));
            }
            if (rReference.ouURI == "idSignedProperties")
            {
                m_xDocumentHandler->startElement(TAG_TRANSFORMS, uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
                writeCanonicalizationTransform();
                m_xDocumentHandler->endElement(TAG_TRANSFORMS);
            }

            writeDigestMethod();
            m_xDocumentHandler->startElement(TAG_DIGESTVALUE, uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
            m_xDocumentHandler->characters(rReference.ouDigestValue);
            m_xDocumentHandler->endElement(TAG_DIGESTVALUE);
            m_xDocumentHandler->endElement(TAG_REFERENCE);
        }
    }
}

void OOXMLSecExporter::Impl::writeSignatureValue()
{
    m_xDocumentHandler->startElement(TAG_SIGNATUREVALUE, uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    m_xDocumentHandler->characters(m_rInformation.ouSignatureValue);
    m_xDocumentHandler->endElement(TAG_SIGNATUREVALUE);
}

void OOXMLSecExporter::Impl::writeKeyInfo()
{
    m_xDocumentHandler->startElement(TAG_KEYINFO, uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    m_xDocumentHandler->startElement(TAG_X509DATA, uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    m_xDocumentHandler->startElement(TAG_X509CERTIFICATE, uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    m_xDocumentHandler->characters(m_rInformation.ouX509Certificate);
    m_xDocumentHandler->endElement(TAG_X509CERTIFICATE);
    m_xDocumentHandler->endElement(TAG_X509DATA);
    m_xDocumentHandler->endElement(TAG_KEYINFO);
}

void OOXMLSecExporter::Impl::writePackageObject()
{
    rtl::Reference<SvXMLAttributeList> pAttributeList(new SvXMLAttributeList());
    pAttributeList->AddAttribute(ATTR_ID, "idPackageObject");
    m_xDocumentHandler->startElement(TAG_OBJECT, uno::Reference<xml::sax::XAttributeList>(pAttributeList.get()));

    writeManifest();
    writePackageObjectSignatureProperties();

    m_xDocumentHandler->endElement(TAG_OBJECT);
}

void OOXMLSecExporter::Impl::writeManifest()
{
    m_xDocumentHandler->startElement(TAG_MANIFEST, uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
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
    m_xDocumentHandler->endElement(TAG_MANIFEST);
}

void OOXMLSecExporter::Impl::writeRelationshipTransform(const OUString& rURI)
{
    uno::Reference<embed::XHierarchicalStorageAccess> xHierarchicalStorageAccess(m_xRootStorage, uno::UNO_QUERY);
    uno::Reference<io::XInputStream> xRelStream(xHierarchicalStorageAccess->openStreamElementByHierarchicalName(rURI, embed::ElementModes::READ), uno::UNO_QUERY);
    {
        rtl::Reference<SvXMLAttributeList> pAttributeList(new SvXMLAttributeList());
        pAttributeList->AddAttribute(ATTR_ALGORITHM, ALGO_RELATIONSHIP);
        m_xDocumentHandler->startElement(TAG_TRANSFORM, uno::Reference<xml::sax::XAttributeList>(pAttributeList.get()));
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
        pAttributeList->AddAttribute(ATTR_XMLNS ":" NSTAG_MDSSI, NS_MDSSI);
        pAttributeList->AddAttribute(ATTR_SOURCEID, aId);
        m_xDocumentHandler->startElement(NSTAG_MDSSI ":" TAG_RELATIONSHIPREFERENCE, uno::Reference<xml::sax::XAttributeList>(pAttributeList.get()));
        m_xDocumentHandler->endElement(NSTAG_MDSSI ":" TAG_RELATIONSHIPREFERENCE);
    }

    m_xDocumentHandler->endElement(TAG_TRANSFORM);
}

void OOXMLSecExporter::Impl::writePackageObjectSignatureProperties()
{
    m_xDocumentHandler->startElement(TAG_SIGNATUREPROPERTIES, uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    {
        rtl::Reference<SvXMLAttributeList> pAttributeList(new SvXMLAttributeList());
        pAttributeList->AddAttribute(ATTR_ID, "idSignatureTime");
        pAttributeList->AddAttribute(ATTR_TARGET, "#idPackageSignature");
        m_xDocumentHandler->startElement(TAG_SIGNATUREPROPERTY, uno::Reference<xml::sax::XAttributeList>(pAttributeList.get()));
    }
    {
        rtl::Reference<SvXMLAttributeList> pAttributeList(new SvXMLAttributeList());
        pAttributeList->AddAttribute(ATTR_XMLNS ":" NSTAG_MDSSI, NS_MDSSI);
        m_xDocumentHandler->startElement(NSTAG_MDSSI ":" TAG_SIGNATURETIME, uno::Reference<xml::sax::XAttributeList>(pAttributeList.get()));
    }
    m_xDocumentHandler->startElement(NSTAG_MDSSI ":" TAG_FORMAT, uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    m_xDocumentHandler->characters("YYYY-MM-DDThh:mm:ssTZD");
    m_xDocumentHandler->endElement(NSTAG_MDSSI ":" TAG_FORMAT);

    m_xDocumentHandler->startElement(NSTAG_MDSSI ":" TAG_VALUE, uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
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
    m_xDocumentHandler->endElement(NSTAG_MDSSI ":" TAG_VALUE);

    m_xDocumentHandler->endElement(NSTAG_MDSSI ":" TAG_SIGNATURETIME);
    m_xDocumentHandler->endElement(TAG_SIGNATUREPROPERTY);
    m_xDocumentHandler->endElement(TAG_SIGNATUREPROPERTIES);
}

void OOXMLSecExporter::Impl::writeManifestReference(const SignatureReferenceInformation& rReference)
{
    rtl::Reference<SvXMLAttributeList> pAttributeList(new SvXMLAttributeList());
    pAttributeList->AddAttribute(ATTR_URI, rReference.ouURI);
    m_xDocumentHandler->startElement(TAG_REFERENCE, uno::Reference<xml::sax::XAttributeList>(pAttributeList.get()));

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

        m_xDocumentHandler->startElement(TAG_TRANSFORMS, uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));

        writeRelationshipTransform(aURI);
        writeCanonicalizationTransform();

        m_xDocumentHandler->endElement(TAG_TRANSFORMS);
    }

    writeDigestMethod();
    m_xDocumentHandler->startElement(TAG_DIGESTVALUE, uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    m_xDocumentHandler->characters(rReference.ouDigestValue);
    m_xDocumentHandler->endElement(TAG_DIGESTVALUE);
    m_xDocumentHandler->endElement(TAG_REFERENCE);
}

void OOXMLSecExporter::Impl::writeOfficeObject()
{
    {
        rtl::Reference<SvXMLAttributeList> pAttributeList(new SvXMLAttributeList());
        pAttributeList->AddAttribute(ATTR_ID, "idOfficeObject");
        m_xDocumentHandler->startElement(TAG_OBJECT, uno::Reference<xml::sax::XAttributeList>(pAttributeList.get()));
    }
    m_xDocumentHandler->startElement(TAG_SIGNATUREPROPERTIES, uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    {
        rtl::Reference<SvXMLAttributeList> pAttributeList(new SvXMLAttributeList());
        pAttributeList->AddAttribute(ATTR_ID, "idOfficeV1Details");
        pAttributeList->AddAttribute(ATTR_TARGET, "#idPackageSignature");
        m_xDocumentHandler->startElement(TAG_SIGNATUREPROPERTY, uno::Reference<xml::sax::XAttributeList>(pAttributeList.get()));
    }
    writeSignatureInfo();
    m_xDocumentHandler->endElement(TAG_SIGNATUREPROPERTY);
    m_xDocumentHandler->endElement(TAG_SIGNATUREPROPERTIES);
    m_xDocumentHandler->endElement(TAG_OBJECT);
}

void OOXMLSecExporter::Impl::writeSignatureInfo()
{
    rtl::Reference<SvXMLAttributeList> pAttributeList(new SvXMLAttributeList());
    pAttributeList->AddAttribute(ATTR_XMLNS, "http://schemas.microsoft.com/office/2006/digsig");
    m_xDocumentHandler->startElement("SignatureInfoV1", uno::Reference<xml::sax::XAttributeList>(pAttributeList.get()));

    m_xDocumentHandler->startElement("SetupId", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    m_xDocumentHandler->endElement("SetupId");
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
    m_xDocumentHandler->characters("1");
    m_xDocumentHandler->endElement("SignatureType");

    m_xDocumentHandler->endElement("SignatureInfoV1");
}

void OOXMLSecExporter::Impl::writePackageSignature()
{
    m_xDocumentHandler->startElement(TAG_OBJECT, uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    {
        rtl::Reference<SvXMLAttributeList> pAttributeList(new SvXMLAttributeList());
        pAttributeList->AddAttribute(ATTR_XMLNS ":" NSTAG_XD, NS_XD);
        pAttributeList->AddAttribute(ATTR_TARGET, "#idPackageSignature");
        m_xDocumentHandler->startElement(NSTAG_XD ":" TAG_QUALIFYINGPROPERTIES, uno::Reference<xml::sax::XAttributeList>(pAttributeList.get()));
    }

    // FIXME why does this part crash NSS when MOZILLA_CERTIFICATE_FOLDER is not set?
    static bool bTest = getenv("LO_TESTNAME");
    if (!bTest)
        writeSignedProperties();

    m_xDocumentHandler->endElement(NSTAG_XD ":" TAG_QUALIFYINGPROPERTIES);
    m_xDocumentHandler->endElement(TAG_OBJECT);
}

void OOXMLSecExporter::Impl::writeSignedProperties()
{
    {
        rtl::Reference<SvXMLAttributeList> pAttributeList(new SvXMLAttributeList());
        pAttributeList->AddAttribute(ATTR_ID, "idSignedProperties");
        m_xDocumentHandler->startElement(NSTAG_XD ":" TAG_SIGNEDPROPERTIES, uno::Reference<xml::sax::XAttributeList>(pAttributeList.get()));
    }

    m_xDocumentHandler->startElement("xd:SignedSignatureProperties", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    m_xDocumentHandler->startElement("xd:SigningTime", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    m_xDocumentHandler->characters(m_aSignatureTimeValue);
    m_xDocumentHandler->endElement("xd:SigningTime");
    m_xDocumentHandler->startElement("xd:SigningCertificate", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    m_xDocumentHandler->startElement("xd:Cert", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    m_xDocumentHandler->startElement("xd:CertDigest", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    writeDigestMethod();

    m_xDocumentHandler->startElement("DigestValue", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    assert(!m_rInformation.ouCertDigest.isEmpty());
    m_xDocumentHandler->characters(m_rInformation.ouCertDigest);
    m_xDocumentHandler->endElement("DigestValue");

    m_xDocumentHandler->endElement("xd:CertDigest");
    m_xDocumentHandler->startElement("xd:IssuerSerial", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    m_xDocumentHandler->startElement("X509IssuerName", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    m_xDocumentHandler->characters(m_rInformation.ouX509IssuerName);
    m_xDocumentHandler->endElement("X509IssuerName");
    m_xDocumentHandler->startElement("X509SerialNumber", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    m_xDocumentHandler->characters(m_rInformation.ouX509SerialNumber);
    m_xDocumentHandler->endElement("X509SerialNumber");
    m_xDocumentHandler->endElement("xd:IssuerSerial");
    m_xDocumentHandler->endElement("xd:Cert");
    m_xDocumentHandler->endElement("xd:SigningCertificate");
    m_xDocumentHandler->startElement("xd:SignaturePolicyIdentifier", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    m_xDocumentHandler->startElement("xd:SignaturePolicyImplied", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    m_xDocumentHandler->endElement("xd:SignaturePolicyImplied");
    m_xDocumentHandler->endElement("xd:SignaturePolicyIdentifier");
    m_xDocumentHandler->endElement("xd:SignedSignatureProperties");

    m_xDocumentHandler->endElement(NSTAG_XD ":" TAG_SIGNEDPROPERTIES);
}

OOXMLSecExporter::OOXMLSecExporter(const uno::Reference<uno::XComponentContext>& xComponentContext,
                                   const uno::Reference<embed::XStorage>& xRootStorage,
                                   const uno::Reference<xml::sax::XDocumentHandler>& xDocumentHandler,
                                   const SignatureInformation& rInformation)
    : m_pImpl(o3tl::make_unique<Impl>(xComponentContext, xRootStorage, xDocumentHandler, rInformation))
{
}

OOXMLSecExporter::~OOXMLSecExporter()
{
}

void OOXMLSecExporter::writeSignature()
{
    rtl::Reference<SvXMLAttributeList> pAttributeList(new SvXMLAttributeList());
    pAttributeList->AddAttribute(ATTR_XMLNS, NS_XMLDSIG);
    pAttributeList->AddAttribute(ATTR_ID, "idPackageSignature");
    m_pImpl->m_xDocumentHandler->startElement(TAG_SIGNATURE, uno::Reference<xml::sax::XAttributeList>(pAttributeList.get()));

    m_pImpl->writeSignedInfo();
    m_pImpl->writeSignatureValue();
    m_pImpl->writeKeyInfo();
    m_pImpl->writePackageObject();
    m_pImpl->writeOfficeObject();
    m_pImpl->writePackageSignature();

    m_pImpl->m_xDocumentHandler->endElement(TAG_SIGNATURE);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
