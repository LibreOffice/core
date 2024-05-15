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
#include <string_view>

#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/StorageFormats.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>

#include <comphelper/attributelist.hxx>
#include <comphelper/documentconstants.hxx>
#include <comphelper/ofopxmlhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <osl/diagnose.h>
#include <rtl/ref.hxx>
#include <rtl/uri.hxx>
#include <sal/log.hxx>
#include <svx/xoutbmp.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <o3tl/string_view.hxx>

#include <xsecctl.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace css::xml::sax;

namespace
{
std::u16string_view getElement(std::u16string_view version, size_t * index)
{
    while (*index < version.size() && version[*index] == '0') {
        ++*index;
    }
    return o3tl::getToken(version, u'.', *index);
}


// Return 1 if version1 is greater than version 2, 0 if they are equal
//and -1 if version1 is less version 2
int compareVersions(
    std::u16string_view version1, std::u16string_view version2)
{
    for (size_t i1 = 0, i2 = 0; i1 != std::u16string_view::npos || i2 != std::u16string_view::npos;) {
        std::u16string_view e1(getElement(version1, &i1));
        std::u16string_view e2(getElement(version2, &i2));
        if (e1.size() < e2.size()) {
            return -1;
        } else if (e1.size() > e2.size()) {
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
    std::u16string_view rRootStorageName, const bool bRecursive,
    const DocumentSignatureAlgorithm mode)
{
    const Sequence< OUString > aElements = rxStore->getElementNames();

    for ( const auto& rName : aElements )
    {
        if (rName == "[Content_Types].xml")
            // OOXML
            continue;

        // If the user enabled validating according to OOo 3.0
        // then mimetype and all content of META-INF must be excluded.
        if (mode != DocumentSignatureAlgorithm::OOo3_2
            && (rName == "META-INF" || rName == "mimetype"))
        {
            continue;
        }
        else
        {
            OUString sEncName = ::rtl::Uri::encode(
                rName, rtl_UriCharClassRelSegment,
                rtl_UriEncodeStrict, RTL_TEXTENCODING_UTF8);
            if (sEncName.isEmpty() && !rName.isEmpty())
                throw css::uno::RuntimeException(u"Failed to encode element name of XStorage"_ustr, nullptr);

            if ( rxStore->isStreamElement( rName ) )
            {
                //Exclude documentsignatures.xml!
                if (rName ==
                    DocumentSignatureHelper::GetDocumentContentSignatureDefaultStreamName())
                    continue;
                OUString aFullName( rRootStorageName + sEncName );
                rList.push_back(aFullName);
            }
            else if ( bRecursive && rxStore->isStorageElement( rName ) )
            {
                Reference < css::embed::XStorage > xSubStore = rxStore->openStorageElement( rName, css::embed::ElementModes::READ );
                OUString aFullRootName( rRootStorageName + sEncName + "/"  );
                ImplFillElementList(rList, xSubStore, aFullRootName, bRecursive, mode);
            }
        }
    }
}


bool DocumentSignatureHelper::isODFPre_1_2(std::u16string_view sVersion)
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
    std::u16string_view sODFVersion, const SignatureInformation & sigInfo)
{
    OSL_ASSERT(!sODFVersion.empty());
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
    OUString aSep(  u"/"_ustr  );

    switch ( eMode )
    {
        case DocumentSignatureMode::Content:
        {
            if (mode == DocumentSignatureAlgorithm::OOo2) //that is, ODF 1.0, 1.1
            {
                // 1) Main content
                ImplFillElementList(aElements, rxStore, std::u16string_view(), false, mode);

                // 2) Pictures...
                OUString aSubStorageName( u"Pictures"_ustr );
                try
                {
                    Reference < css::embed::XStorage > xSubStore = rxStore->openStorageElement( aSubStorageName, css::embed::ElementModes::READ );
                    ImplFillElementList(
                        aElements, xSubStore, Concat2View(aSubStorageName+aSep), true, mode);
                }
                catch(css::io::IOException& )
                {
                    ; // Doesn't have to exist...
                }
                // 3) OLE...
                aSubStorageName = "ObjectReplacements";
                try
                {
                    Reference < css::embed::XStorage > xSubStore = rxStore->openStorageElement( aSubStorageName, css::embed::ElementModes::READ );
                    ImplFillElementList(
                        aElements, xSubStore, Concat2View(aSubStorageName+aSep), true, mode);
                    xSubStore.clear();

                    // Object folders...
                    const Sequence< OUString > aElementNames = rxStore->getElementNames();
                    for ( const auto& rName : aElementNames )
                    {
                        if ( ( rName.match( "Object " ) ) && rxStore->isStorageElement( rName ) )
                        {
                            Reference < css::embed::XStorage > xTmpSubStore = rxStore->openStorageElement( rName, css::embed::ElementModes::READ );
                            ImplFillElementList(
                                aElements, xTmpSubStore, Concat2View(rName+aSep), true, mode);
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
                ImplFillElementList(aElements, rxStore, std::u16string_view(), true, mode);
            }
        }
        break;
        case DocumentSignatureMode::Macros:
        {
            // 1) Macros
            OUString aSubStorageName( u"Basic"_ustr );
            try
            {
                Reference < css::embed::XStorage > xSubStore = rxStore->openStorageElement( aSubStorageName, css::embed::ElementModes::READ );
                ImplFillElementList(
                    aElements, xSubStore, Concat2View(aSubStorageName+aSep), true, mode);
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
                ImplFillElementList(
                    aElements, xSubStore, Concat2View(aSubStorageName+aSep), true, mode);
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
                ImplFillElementList(
                    aElements, xSubStore, Concat2View(aSubStorageName+aSep), true, mode);
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
            ImplFillElementList(aElements, rxStore, std::u16string_view(), true, mode);
        }
        break;
    }

    return aElements;
}

void DocumentSignatureHelper::AppendContentTypes(const uno::Reference<embed::XStorage>& xStorage, std::vector<OUString>& rElements)
{
    if (!xStorage.is() || !xStorage->hasByName(u"[Content_Types].xml"_ustr))
        // ODF
        return;

    uno::Reference<io::XInputStream> xRelStream(xStorage->openStreamElement(u"[Content_Types].xml"_ustr, embed::ElementModes::READ), uno::UNO_QUERY);
    uno::Sequence< uno::Sequence<beans::StringPair> > aContentTypeInfo = comphelper::OFOPXMLHelper::ReadContentTypeSequence(xRelStream, comphelper::getProcessComponentContext());
    if (aContentTypeInfo.getLength() < 2)
    {
        SAL_WARN("xmlsecurity.helper", "no defaults or overrides in aContentTypeInfo");
        return;
    }
    const uno::Sequence<beans::StringPair>& rDefaults = aContentTypeInfo[0];
    const uno::Sequence<beans::StringPair>& rOverrides = aContentTypeInfo[1];

    for (OUString& rElement : rElements)
    {
        auto it = std::find_if(rOverrides.begin(), rOverrides.end(), [&](const beans::StringPair& rPair)
        {
            return rPair.First == Concat2View("/" + rElement);
        });

        if (it != rOverrides.end())
        {
            rElement = "/" + rElement + "?ContentType=" + it->Second;
            continue;
        }

        it = std::find_if(rDefaults.begin(), rDefaults.end(), [&](const beans::StringPair& rPair)
        {
            return rElement.endsWith(Concat2View("." + rPair.First));
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

    if (!rxStore.is())
        return aHelper;

    if (rxStore->hasByName(u"META-INF"_ustr))
    {
        try
        {
            aHelper.xSignatureStorage = rxStore->openStorageElement( u"META-INF"_ustr, nSubStorageOpenMode );
            if ( aHelper.xSignatureStorage.is() )
            {
                OUString aSIGStreamName;
                if ( eDocSigMode == DocumentSignatureMode::Content )
                    aSIGStreamName = DocumentSignatureHelper::GetDocumentContentSignatureDefaultStreamName();
                else if ( eDocSigMode == DocumentSignatureMode::Macros )
                    aSIGStreamName = DocumentSignatureHelper::GetScriptingContentSignatureDefaultStreamName();
                else
                    aSIGStreamName = DocumentSignatureHelper::GetPackageSignatureDefaultStreamName();

#ifdef SAL_LOG_INFO
                aHelper.xSignatureStream
                    = aHelper.xSignatureStorage->openStreamElement(aSIGStreamName, nOpenMode);
                SAL_INFO("xmlsecurity.helper",
                         "DocumentSignatureHelper::OpenSignatureStream: stream name is '"
                             << aSIGStreamName << "'");
                if (aHelper.xSignatureStream.is())
                {
                    uno::Reference<io::XInputStream> xInputStream(aHelper.xSignatureStream,
                                                                  uno::UNO_QUERY);
                    sal_Int64 nSize = 0;
                    uno::Reference<beans::XPropertySet> xPropertySet(xInputStream, uno::UNO_QUERY);
                    xPropertySet->getPropertyValue(u"Size"_ustr) >>= nSize;
                    if (nSize >= 0 || nSize < SAL_MAX_INT32)
                    {
                        uno::Sequence<sal_Int8> aData;
                        xInputStream->readBytes(aData, nSize);
                        SAL_INFO("xmlsecurity.helper",
                                 "DocumentSignatureHelper::OpenSignatureStream: stream content is '"
                                     << OString(reinterpret_cast<const char*>(aData.getArray()),
                                                aData.getLength())
                                     << "'");
                    }
                }
                aHelper.xSignatureStream.clear();
#endif

                aHelper.xSignatureStream = aHelper.xSignatureStorage->openStreamElement( aSIGStreamName, nOpenMode );
            }
        }
        catch(css::io::IOException& )
        {
            // Doesn't have to exist...
            SAL_WARN_IF( nOpenMode != css::embed::ElementModes::READ, "xmlsecurity.helper", "Error creating signature stream..." );
        }
    }
    else if(rxStore->hasByName(u"[Content_Types].xml"_ustr))
    {
        try
        {
            if (rxStore->hasByName(u"_xmlsignatures"_ustr) && (nOpenMode & embed::ElementModes::TRUNCATE))
                // Truncate, then all signatures will be written -> remove previous ones.
                rxStore->removeElement(u"_xmlsignatures"_ustr);

            aHelper.xSignatureStorage = rxStore->openStorageElement(u"_xmlsignatures"_ustr, nSubStorageOpenMode);
            aHelper.nStorageFormat = embed::StorageFormats::OFOPXML;
        }
        catch (const io::IOException&)
        {
            TOOLS_WARN_EXCEPTION_IF(nOpenMode != css::embed::ElementModes::READ, "xmlsecurity.helper", "DocumentSignatureHelper::OpenSignatureStream:");
        }
    }

    return aHelper;
}

/** Check whether the current file can be signed with GPG (only ODF >= 1.2 can currently) */
bool DocumentSignatureHelper::CanSignWithGPG(
    const Reference < css::embed::XStorage >& rxStore,
    std::u16string_view sOdfVersion)
{
    if (!rxStore.is())
        return false;

    if (rxStore->hasByName(u"META-INF"_ustr)) // ODF
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
    std::u16string_view rUri, std::u16string_view rPath)
{
    //split up the uri and path into segments. Both are separated by '/'
    std::vector<OUString> vUriSegments;
    for (sal_Int32 nIndex = 0; nIndex >= 0; )
        vUriSegments.push_back(OUString(o3tl::getToken(rUri, 0, '/', nIndex )));

    std::vector<OUString> vPathSegments;
    for (sal_Int32 nIndex = 0; nIndex >= 0; )
        vPathSegments.push_back(OUString(o3tl::getToken(rPath, 0, '/', nIndex )));

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
    return u"documentsignatures.xml"_ustr;
}

OUString DocumentSignatureHelper::GetScriptingContentSignatureDefaultStreamName()
{
    return u"macrosignatures.xml"_ustr;
}

OUString DocumentSignatureHelper::GetPackageSignatureDefaultStreamName()
{
    return u"packagesignatures.xml"_ustr;
}

void DocumentSignatureHelper::writeDigestMethod(
    const uno::Reference<xml::sax::XDocumentHandler>& xDocumentHandler)
{
    rtl::Reference<comphelper::AttributeList> pAttributeList(new comphelper::AttributeList());
    pAttributeList->AddAttribute(u"Algorithm"_ustr, ALGO_XMLDSIGSHA256);
    xDocumentHandler->startElement(u"DigestMethod"_ustr, uno::Reference<xml::sax::XAttributeList>(pAttributeList));
    xDocumentHandler->endElement(u"DigestMethod"_ustr);
}

static void WriteXadesCert(
    uno::Reference<xml::sax::XDocumentHandler> const& xDocumentHandler,
    SignatureInformation::X509CertInfo const& rCertInfo)
{
    xDocumentHandler->startElement(u"xd:Cert"_ustr, uno::Reference<xml::sax::XAttributeList>(new comphelper::AttributeList()));
    xDocumentHandler->startElement(u"xd:CertDigest"_ustr, uno::Reference<xml::sax::XAttributeList>(new comphelper::AttributeList()));
    DocumentSignatureHelper::writeDigestMethod(xDocumentHandler);
    xDocumentHandler->startElement(u"DigestValue"_ustr, uno::Reference<xml::sax::XAttributeList>(new comphelper::AttributeList()));
    assert(!rCertInfo.CertDigest.isEmpty());
    xDocumentHandler->characters(rCertInfo.CertDigest);
    xDocumentHandler->endElement(u"DigestValue"_ustr);
    xDocumentHandler->endElement(u"xd:CertDigest"_ustr);
    xDocumentHandler->startElement(u"xd:IssuerSerial"_ustr, uno::Reference<xml::sax::XAttributeList>(new comphelper::AttributeList()));
    xDocumentHandler->startElement(u"X509IssuerName"_ustr, uno::Reference<xml::sax::XAttributeList>(new comphelper::AttributeList()));
    xDocumentHandler->characters(rCertInfo.X509IssuerName);
    xDocumentHandler->endElement(u"X509IssuerName"_ustr);
    xDocumentHandler->startElement(u"X509SerialNumber"_ustr, uno::Reference<xml::sax::XAttributeList>(new comphelper::AttributeList()));
    xDocumentHandler->characters(rCertInfo.X509SerialNumber);
    xDocumentHandler->endElement(u"X509SerialNumber"_ustr);
    xDocumentHandler->endElement(u"xd:IssuerSerial"_ustr);
    xDocumentHandler->endElement(u"xd:Cert"_ustr);
}

void DocumentSignatureHelper::writeSignedProperties(
    const uno::Reference<xml::sax::XDocumentHandler>& xDocumentHandler,
    const SignatureInformation& signatureInfo,
    const OUString& sDate, const bool bWriteSignatureLineData)
{
    {
        rtl::Reference<comphelper::AttributeList> pAttributeList(new comphelper::AttributeList());
        pAttributeList->AddAttribute(u"Id"_ustr, "idSignedProperties_" + signatureInfo.ouSignatureId);
        xDocumentHandler->startElement(u"xd:SignedProperties"_ustr, uno::Reference<xml::sax::XAttributeList>(pAttributeList));
    }

    xDocumentHandler->startElement(u"xd:SignedSignatureProperties"_ustr, uno::Reference<xml::sax::XAttributeList>(new comphelper::AttributeList()));
    xDocumentHandler->startElement(u"xd:SigningTime"_ustr, uno::Reference<xml::sax::XAttributeList>(new comphelper::AttributeList()));
    xDocumentHandler->characters(sDate);
    xDocumentHandler->endElement(u"xd:SigningTime"_ustr);
    xDocumentHandler->startElement(u"xd:SigningCertificate"_ustr, uno::Reference<xml::sax::XAttributeList>(new comphelper::AttributeList()));
    assert(signatureInfo.GetSigningCertificate() || !signatureInfo.ouGpgKeyID.isEmpty());
    if (signatureInfo.GetSigningCertificate())
    {
        // how should this deal with multiple X509Data elements?
        // for now, let's write all of the certificates ...
        for (auto const& rData : signatureInfo.X509Datas)
        {
            for (auto const& it : rData)
            {
                WriteXadesCert(xDocumentHandler, it);
            }
        }
    }
    else
    {
        // for PGP, write empty mandatory X509IssuerName, X509SerialNumber
        SignatureInformation::X509CertInfo temp;
        temp.CertDigest = signatureInfo.ouGpgKeyID;
        WriteXadesCert(xDocumentHandler, temp);
    }
    xDocumentHandler->endElement(u"xd:SigningCertificate"_ustr);
    xDocumentHandler->startElement(u"xd:SignaturePolicyIdentifier"_ustr, uno::Reference<xml::sax::XAttributeList>(new comphelper::AttributeList()));
    xDocumentHandler->startElement(u"xd:SignaturePolicyImplied"_ustr, uno::Reference<xml::sax::XAttributeList>(new comphelper::AttributeList()));
    xDocumentHandler->endElement(u"xd:SignaturePolicyImplied"_ustr);
    xDocumentHandler->endElement(u"xd:SignaturePolicyIdentifier"_ustr);

    if (bWriteSignatureLineData && !signatureInfo.ouSignatureLineId.isEmpty()
        && signatureInfo.aValidSignatureImage.is() && signatureInfo.aInvalidSignatureImage.is())
    {
        rtl::Reference<comphelper::AttributeList> pAttributeList(new comphelper::AttributeList());
        pAttributeList->AddAttribute(
            u"xmlns:loext"_ustr, u"urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0"_ustr);
        xDocumentHandler->startElement(
            u"loext:SignatureLine"_ustr,
            Reference<XAttributeList>(pAttributeList));

        {
            // Write SignatureLineId element
            xDocumentHandler->startElement(
                u"loext:SignatureLineId"_ustr,
                Reference<XAttributeList>(new comphelper::AttributeList()));
            xDocumentHandler->characters(signatureInfo.ouSignatureLineId);
            xDocumentHandler->endElement(u"loext:SignatureLineId"_ustr);
        }

        {
            // Write SignatureLineValidImage element
            xDocumentHandler->startElement(
                u"loext:SignatureLineValidImage"_ustr,
                Reference<XAttributeList>(new comphelper::AttributeList()));

            OUString aGraphicInBase64;
            Graphic aGraphic(signatureInfo.aValidSignatureImage);
            if (!XOutBitmap::GraphicToBase64(aGraphic, aGraphicInBase64, false))
                SAL_WARN("xmlsecurity.helper", "could not convert graphic to base64");

            xDocumentHandler->characters(aGraphicInBase64);
            xDocumentHandler->endElement(u"loext:SignatureLineValidImage"_ustr);
        }

        {
            // Write SignatureLineInvalidImage element
            xDocumentHandler->startElement(
                u"loext:SignatureLineInvalidImage"_ustr,
                Reference<XAttributeList>(new comphelper::AttributeList()));
            OUString aGraphicInBase64;
            Graphic aGraphic(signatureInfo.aInvalidSignatureImage);
            if (!XOutBitmap::GraphicToBase64(aGraphic, aGraphicInBase64, false))
                SAL_WARN("xmlsecurity.helper", "could not convert graphic to base64");
            xDocumentHandler->characters(aGraphicInBase64);
            xDocumentHandler->endElement(u"loext:SignatureLineInvalidImage"_ustr);
        }

        xDocumentHandler->endElement(u"loext:SignatureLine"_ustr);
    }

    xDocumentHandler->endElement(u"xd:SignedSignatureProperties"_ustr);

    xDocumentHandler->endElement(u"xd:SignedProperties"_ustr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
