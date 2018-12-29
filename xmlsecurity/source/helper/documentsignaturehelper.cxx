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


#include <documentsignaturehelper.hxx>

#include <algorithm>
#include <functional>

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/StorageFormats.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/StringPair.hpp>

#include <comphelper/documentconstants.hxx>
#include <comphelper/ofopxmlhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <osl/diagnose.h>
#include <rtl/ref.hxx>
#include <rtl/uri.hxx>
#include <sal/log.hxx>
#include <svx/xoutbmp.hxx>
#include <xmloff/attrlist.hxx>

#include <xsecctl.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace css::xml::sax;

namespace
{
OUString getElement(OUString const & version, ::sal_Int32 * index)
{
    while (*index < version.getLength() && version[*index] == '0') {
        ++*index;
    }
    return version.getToken(0, '.', *index);
}


// Return 1 if version1 is greater then version 2, 0 if they are equal
//and -1 if version1 is less version 2
int compareVersions(
    OUString const & version1, OUString const & version2)
{
    for (::sal_Int32 i1 = 0, i2 = 0; i1 >= 0 || i2 >= 0;) {
        OUString e1(getElement(version1, &i1));
        OUString e2(getElement(version2, &i2));
        if (e1.getLength() < e2.getLength()) {
            return -1;
        } else if (e1.getLength() > e2.getLength()) {
            return 1;
        } else if (e1 < e2) {
            return -1;
        } else if (e1 > e2) {
            return 1;
        }
    }
    return 0;
}
}

static void ImplFillElementList(
    std::vector< OUString >& rList, const Reference < css::embed::XStorage >& rxStore,
    const OUString& rRootStorageName, const bool bRecursive,
    const DocumentSignatureAlgorithm mode)
{
    Reference < css::container::XNameAccess > xElements( rxStore, UNO_QUERY );
    Sequence< OUString > aElements = xElements->getElementNames();
    sal_Int32 nElements = aElements.getLength();
    const OUString* pNames = aElements.getConstArray();

    for ( sal_Int32 n = 0; n < nElements; n++ )
    {
        if (pNames[n] == "[Content_Types].xml")
            // OOXML
            continue;

        // If the user enabled validating according to OOo 3.0
        // then mimetype and all content of META-INF must be excluded.
        if (mode != DocumentSignatureAlgorithm::OOo3_2
            && (pNames[n] == "META-INF" || pNames[n] == "mimetype"))
        {
            continue;
        }
        else
        {
            OUString sEncName = ::rtl::Uri::encode(
                pNames[n], rtl_UriCharClassRelSegment,
                rtl_UriEncodeStrict, RTL_TEXTENCODING_UTF8);
            if (sEncName.isEmpty() && !pNames[n].isEmpty())
                throw css::uno::RuntimeException("Failed to encode element name of XStorage", nullptr);

            if ( rxStore->isStreamElement( pNames[n] ) )
            {
                //Exclude documentsignatures.xml!
                if (pNames[n] ==
                    DocumentSignatureHelper::GetDocumentContentSignatureDefaultStreamName())
                    continue;
                OUString aFullName( rRootStorageName + sEncName );
                rList.push_back(aFullName);
            }
            else if ( bRecursive && rxStore->isStorageElement( pNames[n] ) )
            {
                Reference < css::embed::XStorage > xSubStore = rxStore->openStorageElement( pNames[n], css::embed::ElementModes::READ );
                OUString aFullRootName( rRootStorageName + sEncName + "/"  );
                ImplFillElementList(rList, xSubStore, aFullRootName, bRecursive, mode);
            }
        }
    }
}


bool DocumentSignatureHelper::isODFPre_1_2(const OUString & sVersion)
{
    //The property version exists only if the document is at least version 1.2
    //That is, if the document has version 1.1 and sVersion is empty.
    //The constant is defined in comphelper/documentconstants.hxx
    return compareVersions(sVersion, ODFVER_012_TEXT) == -1;
}

bool DocumentSignatureHelper::isOOo3_2_Signature(const SignatureInformation & sigInfo)
{
    return std::any_of(sigInfo.vSignatureReferenceInfors.cbegin(),
                       sigInfo.vSignatureReferenceInfors.cend(),
                       [](const SignatureReferenceInformation& info) { return info.ouURI == "META-INF/manifest.xml"; });
}

DocumentSignatureAlgorithm
DocumentSignatureHelper::getDocumentAlgorithm(
    const OUString & sODFVersion, const SignatureInformation & sigInfo)
{
    OSL_ASSERT(!sODFVersion.isEmpty());
    DocumentSignatureAlgorithm mode = DocumentSignatureAlgorithm::OOo3_2;
    if (!isOOo3_2_Signature(sigInfo))
    {
        if (isODFPre_1_2(sODFVersion))
            mode = DocumentSignatureAlgorithm::OOo2;
        else
            mode = DocumentSignatureAlgorithm::OOo3_0;
    }
    return mode;
}

//The function creates a list of files which are to be signed or for which
//the signature is to be validated. The strings are UTF8 encoded URIs which
//contain '/' as path separators.
//
//The algorithm how document signatures are created and validated has
//changed over time. The change affects only which files within the document
//are changed. Document signatures created by OOo 2.x only used particular files. Since
//OOo 3.0 everything except "mimetype" and "META-INF" are signed. As of OOo 3.2 everything
//except META-INF/documentsignatures.xml is signed.
//Signatures are validated according to the algorithm which was then used for validation.
//That is, when validating a signature which was created by OOo 3.0, then mimetype and
//META-INF are not used.
//
//When a signature is created then we always use the latest algorithm. That is, we use
//that of OOo 3.2
std::vector< OUString >
DocumentSignatureHelper::CreateElementList(
    const Reference < css::embed::XStorage >& rxStore,
    DocumentSignatureMode eMode,
    const DocumentSignatureAlgorithm mode)
{
    std::vector< OUString > aElements;
    OUString aSep(  "/"  );

    switch ( eMode )
    {
        case DocumentSignatureMode::Content:
        {
            if (mode == DocumentSignatureAlgorithm::OOo2) //that is, ODF 1.0, 1.1
            {
                // 1) Main content
                ImplFillElementList(aElements, rxStore, OUString(), false, mode);

                // 2) Pictures...
                OUString aSubStorageName( "Pictures" );
                try
                {
                    Reference < css::embed::XStorage > xSubStore = rxStore->openStorageElement( aSubStorageName, css::embed::ElementModes::READ );
                    ImplFillElementList(aElements, xSubStore, aSubStorageName+aSep, true, mode);
                }
                catch(css::io::IOException& )
                {
                    ; // Doesn't have to exist...
                }
                // 3) OLE....
                aSubStorageName = "ObjectReplacements";
                try
                {
                    Reference < css::embed::XStorage > xSubStore = rxStore->openStorageElement( aSubStorageName, css::embed::ElementModes::READ );
                    ImplFillElementList(aElements, xSubStore, aSubStorageName+aSep, true, mode);
                    xSubStore.clear();

                    // Object folders...
                    Reference < css::container::XNameAccess > xElements( rxStore, UNO_QUERY );
                    Sequence< OUString > aElementNames = xElements->getElementNames();
                    sal_Int32 nElements = aElementNames.getLength();
                    const OUString* pNames = aElementNames.getConstArray();
                    for ( sal_Int32 n = 0; n < nElements; n++ )
                    {
                        if ( ( pNames[n].match( "Object " ) ) && rxStore->isStorageElement( pNames[n] ) )
                        {
                            Reference < css::embed::XStorage > xTmpSubStore = rxStore->openStorageElement( pNames[n], css::embed::ElementModes::READ );
                            ImplFillElementList(aElements, xTmpSubStore, pNames[n]+aSep, true, mode);
                        }
                    }
                }
                catch( css::io::IOException& )
                {
                    ; // Doesn't have to exist...
                }
            }
            else
            {
                // Everything except META-INF
                ImplFillElementList(aElements, rxStore, OUString(), true, mode);
            }
        }
        break;
        case DocumentSignatureMode::Macros:
        {
            // 1) Macros
            OUString aSubStorageName( "Basic" );
            try
            {
                Reference < css::embed::XStorage > xSubStore = rxStore->openStorageElement( aSubStorageName, css::embed::ElementModes::READ );
                ImplFillElementList(aElements, xSubStore, aSubStorageName+aSep, true, mode);
            }
            catch( css::io::IOException& )
            {
                ; // Doesn't have to exist...
            }

            // 2) Dialogs
            aSubStorageName = "Dialogs";
            try
            {
                Reference < css::embed::XStorage > xSubStore = rxStore->openStorageElement( aSubStorageName, css::embed::ElementModes::READ );
                ImplFillElementList(aElements, xSubStore, aSubStorageName+aSep, true, mode);
            }
            catch( css::io::IOException& )
            {
                ; // Doesn't have to exist...
            }
            // 3) Scripts
            aSubStorageName = "Scripts";
            try
            {
                Reference < css::embed::XStorage > xSubStore = rxStore->openStorageElement( aSubStorageName, css::embed::ElementModes::READ );
                ImplFillElementList(aElements, xSubStore, aSubStorageName+aSep, true, mode);
            }
            catch( css::io::IOException& )
            {
                ; // Doesn't have to exist...
            }
        }
        break;
        case DocumentSignatureMode::Package:
        {
            // Everything except META-INF
            ImplFillElementList(aElements, rxStore, OUString(), true, mode);
        }
        break;
    }

    return aElements;
}

void DocumentSignatureHelper::AppendContentTypes(const uno::Reference<embed::XStorage>& xStorage, std::vector<OUString>& rElements)
{
    uno::Reference<container::XNameAccess> xNameAccess(xStorage, uno::UNO_QUERY);
    if (!xNameAccess.is() || !xNameAccess->hasByName("[Content_Types].xml"))
        // ODF
        return;

    uno::Reference<io::XInputStream> xRelStream(xStorage->openStreamElement("[Content_Types].xml", embed::ElementModes::READ), uno::UNO_QUERY);
    uno::Sequence< uno::Sequence<beans::StringPair> > aContentTypeInfo = comphelper::OFOPXMLHelper::ReadContentTypeSequence(xRelStream, comphelper::getProcessComponentContext());
    if (aContentTypeInfo.getLength() < 2)
    {
        SAL_WARN("xmlsecurity.helper", "no defaults or overrides in aContentTypeInfo");
        return;
    }
    uno::Sequence<beans::StringPair>& rDefaults = aContentTypeInfo[0];
    uno::Sequence<beans::StringPair>& rOverrides = aContentTypeInfo[1];

    for (OUString& rElement : rElements)
    {
        auto it = std::find_if(rOverrides.begin(), rOverrides.end(), [&](const beans::StringPair& rPair)
        {
            return rPair.First == "/" + rElement;
        });

        if (it != rOverrides.end())
        {
            rElement = "/" + rElement + "?ContentType=" + it->Second;
            continue;
        }

        it = std::find_if(rDefaults.begin(), rDefaults.end(), [&](const beans::StringPair& rPair)
        {
            return rElement.endsWith("." + rPair.First);
        });

        if (it != rDefaults.end())
        {
            rElement = "/" + rElement + "?ContentType=" + it->Second;
            continue;
        }
        SAL_WARN("xmlsecurity.helper", "found no content type for " << rElement);
    }

    std::sort(rElements.begin(), rElements.end());
}

SignatureStreamHelper DocumentSignatureHelper::OpenSignatureStream(
    const Reference < css::embed::XStorage >& rxStore, sal_Int32 nOpenMode, DocumentSignatureMode eDocSigMode )
{
    sal_Int32 nSubStorageOpenMode = css::embed::ElementModes::READ;
    if ( nOpenMode & css::embed::ElementModes::WRITE )
        nSubStorageOpenMode = css::embed::ElementModes::WRITE;

    SignatureStreamHelper aHelper;

    uno::Reference<container::XNameAccess> xNameAccess(rxStore, uno::UNO_QUERY);
    if (!xNameAccess.is())
        return aHelper;

    if (xNameAccess->hasByName("META-INF"))
    {
        try
        {
            aHelper.xSignatureStorage = rxStore->openStorageElement( "META-INF", nSubStorageOpenMode );
            if ( aHelper.xSignatureStorage.is() )
            {
                OUString aSIGStreamName;
                if ( eDocSigMode == DocumentSignatureMode::Content )
                    aSIGStreamName = DocumentSignatureHelper::GetDocumentContentSignatureDefaultStreamName();
                else if ( eDocSigMode == DocumentSignatureMode::Macros )
                    aSIGStreamName = DocumentSignatureHelper::GetScriptingContentSignatureDefaultStreamName();
                else
                    aSIGStreamName = DocumentSignatureHelper::GetPackageSignatureDefaultStreamName();

                aHelper.xSignatureStream = aHelper.xSignatureStorage->openStreamElement( aSIGStreamName, nOpenMode );
            }
        }
        catch(css::io::IOException& )
        {
            // Doesn't have to exist...
            SAL_WARN_IF( nOpenMode != css::embed::ElementModes::READ, "xmlsecurity.helper", "Error creating signature stream..." );
        }
    }
    else if(xNameAccess->hasByName("[Content_Types].xml"))
    {
        try
        {
            if (xNameAccess->hasByName("_xmlsignatures") && (nOpenMode & embed::ElementModes::TRUNCATE))
                // Truncate, then all signatures will be written -> remove previous ones.
                rxStore->removeElement("_xmlsignatures");

            aHelper.xSignatureStorage = rxStore->openStorageElement("_xmlsignatures", nSubStorageOpenMode);
            aHelper.nStorageFormat = embed::StorageFormats::OFOPXML;
        }
        catch (const io::IOException& rException)
        {
            SAL_WARN_IF(nOpenMode != css::embed::ElementModes::READ, "xmlsecurity.helper", "DocumentSignatureHelper::OpenSignatureStream: " << rException);
        }
    }

    return aHelper;
}

/** Check whether the current file can be signed with GPG (only ODF >= 1.2 can currently) */
bool DocumentSignatureHelper::CanSignWithGPG(
    const Reference < css::embed::XStorage >& rxStore,
    const OUString& sOdfVersion)
{
    uno::Reference<container::XNameAccess> xNameAccess(rxStore, uno::UNO_QUERY);
    if (!xNameAccess.is())
        return false;

    if (xNameAccess->hasByName("META-INF")) // ODF
    {
        return !isODFPre_1_2(sOdfVersion);
    }

    return false;
}



//sElementList contains all files which are expected to be signed. Only those files must me signed,
//no more, no less.
//The DocumentSignatureAlgorithm indicates if the document was created with OOo 2.x. Then
//the uri s in the Reference elements in the signature, were not properly encoded.
// For example: <Reference URI="ObjectReplacements/Object 1">
bool DocumentSignatureHelper::checkIfAllFilesAreSigned(
    const ::std::vector< OUString > & sElementList,
    const SignatureInformation & sigInfo,
    const DocumentSignatureAlgorithm alg)
{
    // Can only be valid if ALL streams are signed, which means real stream count == signed stream count
    unsigned int nRealCount = 0;
    std::function<OUString(const OUString&)> fEncode = [](const OUString& rStr) { return rStr; };
    if (alg == DocumentSignatureAlgorithm::OOo2)
        //Comparing URIs is a difficult. Therefore we kind of normalize
        //it before comparing. We assume that our URI do not have a leading "./"
        //and fragments at the end (...#...)
        fEncode = [](const OUString& rStr) {
            return rtl::Uri::encode(rStr, rtl_UriCharClassPchar, rtl_UriEncodeCheckEscapes, RTL_TEXTENCODING_UTF8);
        };

    for ( int i = sigInfo.vSignatureReferenceInfors.size(); i; )
    {
        const SignatureReferenceInformation& rInf = sigInfo.vSignatureReferenceInfors[--i];
        // There is also an extra entry of type SignatureReferenceType::SAMEDOCUMENT because of signature date.
        if ( ( rInf.nType == SignatureReferenceType::BINARYSTREAM ) || ( rInf.nType == SignatureReferenceType::XMLSTREAM ) )
        {
            //find the file in the element list
            if (std::any_of(sElementList.cbegin(), sElementList.cend(),
                    [&fEncode, &rInf](const OUString& rElement) { return fEncode(rElement) == fEncode(rInf.ouURI); }))
                nRealCount++;
        }
    }
    return  sElementList.size() == nRealCount;
}

/*Compares the Uri which are obtained from CreateElementList with
  the  path obtained from the manifest.xml.
  Returns true if both strings are equal.
*/
bool DocumentSignatureHelper::equalsReferenceUriManifestPath(
    const OUString & rUri, const OUString & rPath)
{
    //split up the uri and path into segments. Both are separated by '/'
    std::vector<OUString> vUriSegments;
    for (sal_Int32 nIndex = 0; nIndex >= 0; )
        vUriSegments.push_back(rUri.getToken( 0, '/', nIndex ));

    std::vector<OUString> vPathSegments;
    for (sal_Int32 nIndex = 0; nIndex >= 0; )
        vPathSegments.push_back(rPath.getToken( 0, '/', nIndex ));

    if (vUriSegments.size() != vPathSegments.size())
        return false;

    //Now compare each segment of the uri with its counterpart from the path
    return std::equal(
        vUriSegments.cbegin(), vUriSegments.cend(), vPathSegments.cbegin(),
        [](const OUString& rUriSegment, const OUString& rPathSegment) {
            //Decode the uri segment, so that %20 becomes ' ', etc.
            OUString sDecUri = rtl::Uri::decode(rUriSegment, rtl_UriDecodeWithCharset,  RTL_TEXTENCODING_UTF8);
            return sDecUri == rPathSegment;
        });
}

OUString DocumentSignatureHelper::GetDocumentContentSignatureDefaultStreamName()
{
    return OUString("documentsignatures.xml");
}

OUString DocumentSignatureHelper::GetScriptingContentSignatureDefaultStreamName()
{
    return OUString("macrosignatures.xml");
}

OUString DocumentSignatureHelper::GetPackageSignatureDefaultStreamName()
{
    return OUString("packagesignatures.xml");
}

void DocumentSignatureHelper::writeDigestMethod(
    const uno::Reference<xml::sax::XDocumentHandler>& xDocumentHandler)
{
    rtl::Reference<SvXMLAttributeList> pAttributeList(new SvXMLAttributeList());
    pAttributeList->AddAttribute("Algorithm", ALGO_XMLDSIGSHA256);
    xDocumentHandler->startElement("DigestMethod", uno::Reference<xml::sax::XAttributeList>(pAttributeList.get()));
    xDocumentHandler->endElement("DigestMethod");
}

void DocumentSignatureHelper::writeSignedProperties(
    const uno::Reference<xml::sax::XDocumentHandler>& xDocumentHandler,
    const SignatureInformation& signatureInfo,
    const OUString& sDate, const bool bWriteSignatureLineData)
{
    {
        rtl::Reference<SvXMLAttributeList> pAttributeList(new SvXMLAttributeList());
        pAttributeList->AddAttribute("Id", "idSignedProperties");
        xDocumentHandler->startElement("xd:SignedProperties", uno::Reference<xml::sax::XAttributeList>(pAttributeList.get()));
    }

    xDocumentHandler->startElement("xd:SignedSignatureProperties", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    xDocumentHandler->startElement("xd:SigningTime", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    xDocumentHandler->characters(sDate);
    xDocumentHandler->endElement("xd:SigningTime");
    xDocumentHandler->startElement("xd:SigningCertificate", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    xDocumentHandler->startElement("xd:Cert", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    xDocumentHandler->startElement("xd:CertDigest", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    writeDigestMethod(xDocumentHandler);

    xDocumentHandler->startElement("DigestValue", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    // TODO: this is empty for gpg signatures currently
    //assert(!signatureInfo.ouCertDigest.isEmpty());
    xDocumentHandler->characters(signatureInfo.ouCertDigest);
    xDocumentHandler->endElement("DigestValue");

    xDocumentHandler->endElement("xd:CertDigest");
    xDocumentHandler->startElement("xd:IssuerSerial", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    xDocumentHandler->startElement("X509IssuerName", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    xDocumentHandler->characters(signatureInfo.ouX509IssuerName);
    xDocumentHandler->endElement("X509IssuerName");
    xDocumentHandler->startElement("X509SerialNumber", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    xDocumentHandler->characters(signatureInfo.ouX509SerialNumber);
    xDocumentHandler->endElement("X509SerialNumber");
    xDocumentHandler->endElement("xd:IssuerSerial");
    xDocumentHandler->endElement("xd:Cert");
    xDocumentHandler->endElement("xd:SigningCertificate");
    xDocumentHandler->startElement("xd:SignaturePolicyIdentifier", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    xDocumentHandler->startElement("xd:SignaturePolicyImplied", uno::Reference<xml::sax::XAttributeList>(new SvXMLAttributeList()));
    xDocumentHandler->endElement("xd:SignaturePolicyImplied");
    xDocumentHandler->endElement("xd:SignaturePolicyIdentifier");

    if (bWriteSignatureLineData && !signatureInfo.ouSignatureLineId.isEmpty()
        && signatureInfo.aValidSignatureImage.is() && signatureInfo.aInvalidSignatureImage.is())
    {
        rtl::Reference<SvXMLAttributeList> pAttributeList(new SvXMLAttributeList());
        pAttributeList->AddAttribute(
            "xmlns:loext", "urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0");
        xDocumentHandler->startElement(
            "loext:SignatureLine",
            Reference<XAttributeList>(pAttributeList.get()));

        {
            // Write SignatureLineId element
            xDocumentHandler->startElement(
                "loext:SignatureLineId",
                Reference<XAttributeList>(new SvXMLAttributeList()));
            xDocumentHandler->characters(signatureInfo.ouSignatureLineId);
            xDocumentHandler->endElement("loext:SignatureLineId");
        }

        {
            // Write SignatureLineValidImage element
            xDocumentHandler->startElement(
                "loext:SignatureLineValidImage",
                Reference<XAttributeList>(new SvXMLAttributeList()));

            OUString aGraphicInBase64;
            Graphic aGraphic(signatureInfo.aValidSignatureImage);
            if (!XOutBitmap::GraphicToBase64(aGraphic, aGraphicInBase64, false))
                SAL_WARN("xmlsecurity.helper", "could not convert graphic to base64");

            xDocumentHandler->characters(aGraphicInBase64);
            xDocumentHandler->endElement("loext:SignatureLineValidImage");
        }

        {
            // Write SignatureLineInvalidImage element
            xDocumentHandler->startElement(
                "loext:SignatureLineInvalidImage",
                Reference<XAttributeList>(new SvXMLAttributeList()));
            OUString aGraphicInBase64;
            Graphic aGraphic(signatureInfo.aInvalidSignatureImage);
            if (!XOutBitmap::GraphicToBase64(aGraphic, aGraphicInBase64, false))
                SAL_WARN("xmlsecurity.helper", "could not convert graphic to base64");
            xDocumentHandler->characters(aGraphicInBase64);
            xDocumentHandler->endElement("loext:SignatureLineInvalidImage");
        }

        xDocumentHandler->endElement("loext:SignatureLine");
    }

    xDocumentHandler->endElement("xd:SignedSignatureProperties");

    xDocumentHandler->endElement("xd:SignedProperties");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
