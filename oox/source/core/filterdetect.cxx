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

#include "oox/core/filterdetect.hxx"

#include <config_oox.h>
#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <comphelper/docpasswordhelper.hxx>
#include <comphelper/mediadescriptor.hxx>
#if USE_TLS_OPENSSL
#include <openssl/evp.h>
#endif // USE_TLS_OPENSSL
#if USE_TLS_NSS
#include <nss.h>
#include <pk11pub.h>
#endif // USE_TLS_NSS
#include <rtl/digest.h>
#include "oox/core/fastparser.hxx"
#include "oox/helper/attributelist.hxx"
#include "oox/helper/binaryinputstream.hxx"
#include "oox/helper/binaryoutputstream.hxx"
#include "oox/helper/zipstorage.hxx"
#include "oox/ole/olestorage.hxx"
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/dom/DocumentBuilder.hpp>
#include <com/sun/star/xml/dom/NodeType.hpp>

namespace oox {
namespace core {

// ============================================================================

using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::dom;
using namespace ::com::sun::star::xml::sax;

using ::comphelper::MediaDescriptor;
using ::comphelper::SequenceAsHashMap;

// ============================================================================

FilterDetectDocHandler::FilterDetectDocHandler( const  Reference< XComponentContext >& rxContext, OUString& rFilterName ) :
    mrFilterName( rFilterName ), mxContext( rxContext )
{
    maContextStack.reserve( 2 );
}

FilterDetectDocHandler::~FilterDetectDocHandler()
{
}

void SAL_CALL FilterDetectDocHandler::startDocument()
    throw (SAXException, RuntimeException)
{
}

void SAL_CALL FilterDetectDocHandler::endDocument()
    throw (SAXException, RuntimeException)
{
}

void SAL_CALL FilterDetectDocHandler::setDocumentLocator( const Reference<XLocator>& /*xLocator*/ )
    throw (SAXException, RuntimeException)
{
}

void SAL_CALL FilterDetectDocHandler::startFastElement(
        sal_Int32 nElement, const Reference< XFastAttributeList >& rAttribs )
    throw (SAXException,RuntimeException)
{
    AttributeList aAttribs( rAttribs );
    switch ( nElement )
    {
        // cases for _rels/.rels
        case PR_TOKEN( Relationships ):
        break;
        case PR_TOKEN( Relationship ):
            if( !maContextStack.empty() && (maContextStack.back() == PR_TOKEN( Relationships )) )
                parseRelationship( aAttribs );
        break;

        // cases for [Content_Types].xml
        case PC_TOKEN( Types ):
        break;
        case PC_TOKEN( Default ):
            if( !maContextStack.empty() && (maContextStack.back() == PC_TOKEN( Types )) )
                parseContentTypesDefault( aAttribs );
        break;
        case PC_TOKEN( Override ):
            if( !maContextStack.empty() && (maContextStack.back() == PC_TOKEN( Types )) )
                parseContentTypesOverride( aAttribs );
        break;
    }
    maContextStack.push_back( nElement );
}

void SAL_CALL FilterDetectDocHandler::startUnknownElement(
    const OUString& /*Namespace*/, const OUString& /*Name*/, const Reference<XFastAttributeList>& /*Attribs*/ )
    throw (SAXException, RuntimeException)
{
}

void SAL_CALL FilterDetectDocHandler::endFastElement( sal_Int32 /*nElement*/ )
    throw (SAXException, RuntimeException)
{
    maContextStack.pop_back();
}

void SAL_CALL FilterDetectDocHandler::endUnknownElement(
    const OUString& /*Namespace*/, const OUString& /*Name*/ ) throw (SAXException, RuntimeException)
{
}

Reference<XFastContextHandler> SAL_CALL FilterDetectDocHandler::createFastChildContext(
    sal_Int32 /*Element*/, const Reference<XFastAttributeList>& /*Attribs*/ )
    throw (SAXException, RuntimeException)
{
    return this;
}

Reference<XFastContextHandler> SAL_CALL FilterDetectDocHandler::createUnknownChildContext(
    const OUString& /*Namespace*/, const OUString& /*Name*/, const Reference<XFastAttributeList>& /*Attribs*/)
    throw (SAXException, RuntimeException)
{
    return this;
}

void SAL_CALL FilterDetectDocHandler::characters( const OUString& /*aChars*/ )
    throw (SAXException, RuntimeException)
{
}

void SAL_CALL FilterDetectDocHandler::ignorableWhitespace( const OUString& /*aWhitespaces*/ )
    throw (SAXException, RuntimeException)
{
}

void SAL_CALL FilterDetectDocHandler::processingInstruction(
    const OUString& /*aTarget*/, const OUString& /*aData*/ )
    throw (SAXException, RuntimeException)
{
}

void FilterDetectDocHandler::parseRelationship( const AttributeList& rAttribs )
{
    OUString aType = rAttribs.getString( XML_Type, OUString() );
    if ( aType == "http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" )
    {
        Reference< com::sun::star::uri::XUriReferenceFactory > xFac =  com::sun::star::uri::UriReferenceFactory::create( mxContext );
        try
        {
             // use '/' to representent the root of the zip package ( and provide a 'file' scheme to
             // keep the XUriReference implementation happy )
             Reference< com::sun::star::uri::XUriReference > xBase = xFac->parse( OUString("file:///") );

             Reference< com::sun::star::uri::XUriReference > xPart = xFac->parse(  rAttribs.getString( XML_Target, OUString() ) );
             Reference< com::sun::star::uri::XUriReference > xAbs = xFac->makeAbsolute(  xBase, xPart, sal_True, com::sun::star::uri::RelativeUriExcessParentSegments_RETAIN );

             if ( xAbs.is() )
                 maTargetPath = xAbs->getPath();
        }
        catch( const Exception& )
        {
        }
    }
}

OUString FilterDetectDocHandler::getFilterNameFromContentType( const OUString& rContentType ) const
{
    if( rContentType.equalsAscii("application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml" ) ||
        rContentType.equalsAscii("application/vnd.ms-word.document.macroEnabled.main+xml" ) )
        return OUString( "writer_MS_Word_2007" );

    if( rContentType.equalsAscii("application/vnd.openxmlformats-officedocument.wordprocessingml.template.main+xml") ||
        rContentType.equalsAscii("application/vnd.ms-word.template.macroEnabledTemplate.main+xml") )
        return OUString( "writer_MS_Word_2007_Template" );

    if( rContentType.equalsAscii("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml") ||
        rContentType.equalsAscii("application/vnd.ms-excel.sheet.macroEnabled.main+xml") )
        return OUString( "MS Excel 2007 XML" );

    if( rContentType.equalsAscii("application/vnd.openxmlformats-officedocument.spreadsheetml.template.main+xml") ||
        rContentType.equalsAscii("application/vnd.ms-excel.template.macroEnabled.main+xml") )
        return OUString( "MS Excel 2007 XML Template" );

    if ( rContentType == "application/vnd.ms-excel.sheet.binary.macroEnabled.main" )
        return OUString( "MS Excel 2007 Binary" );

    if( rContentType.equalsAscii("application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml") ||
        rContentType.equalsAscii("application/vnd.ms-powerpoint.presentation.macroEnabled.main+xml") )
        return OUString( "MS PowerPoint 2007 XML" );

    if( rContentType.equalsAscii("application/vnd.openxmlformats-officedocument.presentationml.slideshow.main+xml") ||
        rContentType.equalsAscii("application/vnd.ms-powerpoint.slideshow.macroEnabled.main+xml") )
        return OUString( "MS PowerPoint 2007 XML AutoPlay" );

    if( rContentType.equalsAscii("application/vnd.openxmlformats-officedocument.presentationml.template.main+xml") ||
        rContentType.equalsAscii("application/vnd.ms-powerpoint.template.macroEnabled.main+xml") )
        return OUString( "MS PowerPoint 2007 XML Template" );

    return OUString();
}

void FilterDetectDocHandler::parseContentTypesDefault( const AttributeList& rAttribs )
{
    // only if no overridden part name found
    if( mrFilterName.isEmpty() )
    {
        // check if target path ends with extension
        OUString aExtension = rAttribs.getString( XML_Extension, OUString() );
        sal_Int32 nExtPos = maTargetPath.getLength() - aExtension.getLength();
        if( (nExtPos > 0) && (maTargetPath[ nExtPos - 1 ] == '.') && maTargetPath.match( aExtension, nExtPos ) )
            mrFilterName = getFilterNameFromContentType( rAttribs.getString( XML_ContentType, OUString() ) );
    }
}

void FilterDetectDocHandler::parseContentTypesOverride( const AttributeList& rAttribs )
{
    if( rAttribs.getString( XML_PartName, OUString() ).equals( maTargetPath ) )
        mrFilterName = getFilterNameFromContentType( rAttribs.getString( XML_ContentType, OUString() ) );
}

// ============================================================================

/* Helper for XServiceInfo */
Sequence< OUString > FilterDetect_getSupportedServiceNames()
{
    Sequence< OUString > aServiceNames( 1 );
    aServiceNames[ 0 ] = "com.sun.star.frame.ExtendedTypeDetection";
    return aServiceNames;
}

/* Helper for XServiceInfo */
OUString FilterDetect_getImplementationName()
{
    return OUString( "com.sun.star.comp.oox.FormatDetector" );
}

/* Helper for registry */
Reference< XInterface > SAL_CALL FilterDetect_createInstance( const Reference< XComponentContext >& rxContext ) throw( Exception )
{
    return static_cast< ::cppu::OWeakObject* >( new FilterDetect( rxContext ) );
}

// ----------------------------------------------------------------------------

FilterDetect::FilterDetect( const Reference< XComponentContext >& rxContext ) throw( RuntimeException ) :
    mxContext( rxContext, UNO_SET_THROW )
{
}

FilterDetect::~FilterDetect()
{
}

/* =========================================================================== */
/*  Kudos to Caolan McNamara who provided the core decryption implementations. */
/* =========================================================================== */

namespace {

const sal_uInt32 ENCRYPTINFO_CRYPTOAPI      = 0x00000004;
const sal_uInt32 ENCRYPTINFO_DOCPROPS       = 0x00000008;
const sal_uInt32 ENCRYPTINFO_EXTERNAL       = 0x00000010;
const sal_uInt32 ENCRYPTINFO_AES            = 0x00000020;
const sal_uInt32 ENCRYPTINFO_AGILE          = 0x00000040;

const sal_uInt32 ENCRYPT_ALGO_AES128        = 0x0000660E;
const sal_uInt32 ENCRYPT_ALGO_AES192        = 0x0000660F;
const sal_uInt32 ENCRYPT_ALGO_AES256        = 0x00006610;
const sal_uInt32 ENCRYPT_ALGO_RC4           = 0x00006801;

const sal_uInt32 ENCRYPT_HASH_SHA1          = 0x00008004;

// ----------------------------------------------------------------------------

bool lclIsZipPackage( const Reference< XComponentContext >& rxContext, const Reference< XInputStream >& rxInStrm )
{
    ZipStorage aZipStorage( rxContext, rxInStrm );
    return aZipStorage.isStorage();
}

// ----------------------------------------------------------------------------

struct PackageEncryptionInfo
{
    sal_uInt8           mpnSalt[ 16 ];
    sal_uInt8           mpnEncrVerifier[ 16 ];
    sal_uInt8           mpnEncrVerifierHash[ 32 ];
    sal_uInt32          mnFlags;
    sal_uInt32          mnAlgorithmId;
    sal_uInt32          mnAlgorithmIdHash;
    sal_uInt32          mnKeySize;
    sal_uInt32          mnSaltSize;
    sal_uInt32          mnVerifierHashSize;
};

bool lclReadEncryptionInfo( const  Reference< XComponentContext >& rxContext,
    PackageEncryptionInfo& rEncrInfo, Reference< XInputStream >& rxStrm )
{
    BinaryXInputStream aInfoStrm( rxStrm, true );
    aInfoStrm.skip( 4 );
    aInfoStrm >> rEncrInfo.mnFlags;
    if( getFlag( rEncrInfo.mnFlags, ENCRYPTINFO_EXTERNAL ) )
        return false;

    if( getFlag( rEncrInfo.mnFlags, ENCRYPTINFO_AGILE) )
    {
        //in this mode the info follows in xml format

        fprintf(stderr, "AGILE\n");

        Reference<XDocumentBuilder> xDomBuilder(
            DocumentBuilder::create(rxContext));

        fprintf(stderr, "dom\n");

        Reference<XDocument> xDom(
            xDomBuilder->parse(rxStrm),
            UNO_QUERY_THROW );

        fprintf(stderr, "loaded\n");

        Reference<XElement> xDocElem( xDom->getDocumentElement(),
            UNO_QUERY_THROW );

        fprintf(stderr, "has root\n");

        Reference< XNodeList > keyDataList = xDocElem->getElementsByTagNameNS(
            "http://schemas.microsoft.com/office/2006/encryption", "keyData");

        fprintf(stderr, "nodelist is %d\n", keyDataList.is() && keyDataList->getLength() > 0);

        if (!keyDataList.is() || keyDataList->getLength() < 1)
            return false;

        Reference< XNode > keyData = keyDataList->item(0);

        Reference< XNamedNodeMap > keyDataAttributes = keyData->getAttributes();

        if (!keyDataAttributes.is())
            return false;

        Reference< XNode > xAttrib;

        xAttrib = keyDataAttributes->getNamedItem("saltSize");
        if (!xAttrib.is())
            return false;
        rEncrInfo.mnSaltSize = xAttrib->getNodeValue().toInt32();

        xAttrib = keyDataAttributes->getNamedItem("blockSize");
        if (!xAttrib.is())
            return false;
        int kdBlockSize = xAttrib->getNodeValue().toInt32();

        xAttrib = keyDataAttributes->getNamedItem("keyBits");
        if (!xAttrib.is())
            return false;
        int kdKeyBits = xAttrib->getNodeValue().toInt32();

        xAttrib = keyDataAttributes->getNamedItem("hashSize");
        if (!xAttrib.is())
            return false;
        int kdHashSize = xAttrib->getNodeValue().toInt32();

        fprintf(stderr, "salt block bits hash is %d %d %d %d\n",
            rEncrInfo.mnSaltSize, kdBlockSize, kdKeyBits, kdHashSize);


        OUString kdCipherAlgorithm;
        xAttrib = keyDataAttributes->getNamedItem("cipherAlgorithm");
        if (xAttrib.is())
            kdCipherAlgorithm = xAttrib->getNodeValue();
        if (kdCipherAlgorithm.isEmpty())
            kdCipherAlgorithm = "AES";

        fprintf(stderr, "alg is %s\n", OUStringToOString(kdCipherAlgorithm, RTL_TEXTENCODING_UTF8).getStr());

        OUString kdCipherChaining;
        xAttrib = keyDataAttributes->getNamedItem("cipherChaining");
        if (xAttrib.is())
            kdCipherChaining = xAttrib->getNodeValue();
        if (kdCipherChaining.isEmpty())
            kdCipherChaining = "ChainingModeCBC";

        fprintf(stderr, "chaining is %s\n", OUStringToOString(kdCipherChaining, RTL_TEXTENCODING_UTF8).getStr());

        OUString kdHashAlgorithm;
        xAttrib = keyDataAttributes->getNamedItem("hashAlgorithm");
        if (xAttrib.is())
            kdHashAlgorithm = xAttrib->getNodeValue();
        if (kdHashAlgorithm.isEmpty())
            kdHashAlgorithm = "SHA1";

        fprintf(stderr, "hash is %s\n", OUStringToOString(kdHashAlgorithm, RTL_TEXTENCODING_UTF8).getStr());

        OUString kdSaltValue;
        xAttrib = keyDataAttributes->getNamedItem("saltValue");
        if (xAttrib.is())
            kdSaltValue = xAttrib->getNodeValue();

        fprintf(stderr, "salt is %s\n", OUStringToOString(kdSaltValue, RTL_TEXTENCODING_UTF8).getStr());

        Reference< XNodeList > dataIntegrityList = xDocElem->getElementsByTagNameNS(
            "http://schemas.microsoft.com/office/2006/encryption", "dataIntegrity");

        fprintf(stderr, "dataIntegrity is %d\n", dataIntegrityList.is() && dataIntegrityList->getLength() > 0);

        if (!dataIntegrityList.is() || dataIntegrityList->getLength() < 1)
            return false;

        Reference< XNode > dataIntegrity = dataIntegrityList->item(0);

        Reference< XNamedNodeMap > dataIntegrityAttributes = dataIntegrity->getAttributes();

        if (!dataIntegrityAttributes.is())
            return false;

        OUString encryptedHmacKey;
        xAttrib = keyDataAttributes->getNamedItem("encryptedHmacKey");
        if (xAttrib.is())
            encryptedHmacKey = xAttrib->getNodeValue();

        fprintf(stderr, "hmackey is %s\n", OUStringToOString(encryptedHmacKey, RTL_TEXTENCODING_UTF8).getStr());

        OUString encryptedHmacValue;
        xAttrib = keyDataAttributes->getNamedItem("encryptedHmacValue");
        if (xAttrib.is())
            encryptedHmacValue = xAttrib->getNodeValue();

        fprintf(stderr, "hmacvalue is %s\n", OUStringToOString(encryptedHmacValue, RTL_TEXTENCODING_UTF8).getStr());

        OUString encryptedDataIntegritySaltValue;
        xAttrib = keyDataAttributes->getNamedItem("encryptedSaltValue");
        if (xAttrib.is())
            encryptedDataIntegritySaltValue = xAttrib->getNodeValue();

        fprintf(stderr, "saltvalue is %s\n", OUStringToOString(encryptedDataIntegritySaltValue, RTL_TEXTENCODING_UTF8).getStr());

        OUString encryptedDataIntegrityHashValue;
        xAttrib = keyDataAttributes->getNamedItem("encryptedHashValue");
        if (xAttrib.is())
            encryptedDataIntegrityHashValue = xAttrib->getNodeValue();

        fprintf(stderr, "hashvalue is %s\n", OUStringToOString(encryptedDataIntegrityHashValue, RTL_TEXTENCODING_UTF8).getStr());


#if 0
                encryptedDataIntegritySaltValue = encryptedHmacKey;
                encryptedDataIntegrityHashValue = encryptedHmacValue;
#endif

        Reference< XNodeList > passwordKeyEncryptorList = xDocElem->getElementsByTagNameNS(
            "http://schemas.microsoft.com/office/2006/encryption", "keyEncryptors");

        fprintf(stderr, "keyEncryptors is %d\n", passwordKeyEncryptorList.is() && passwordKeyEncryptorList->getLength() > 0);

        if (!passwordKeyEncryptorList.is() || passwordKeyEncryptorList->getLength() < 1)
            return false;

        Reference< XElement > passwordKeyEncryptor(passwordKeyEncryptorList->item(0), UNO_QUERY_THROW);

        Reference< XNodeList > keList = passwordKeyEncryptor->getElementsByTagNameNS(
            "http://schemas.microsoft.com/office/2006/encryption", "keyEncryptor");

        fprintf(stderr, "keyEncryptor is %d\n", keList.is() && keList->getLength() > 0);

        if (!keList.is() || keList->getLength() < 1)
            return false;

        Reference< XNode > ke = keList->item(0);

        fprintf(stderr, "ke is %d\n", ke.is());

        if (!ke.is())
            return false;

        ke = ke->getFirstChild();

        fprintf(stderr, "ke now %d\n", ke.is());

        if (!ke.is())
            return false;

        Reference< XNamedNodeMap > keAttributes = ke->getAttributes();

        if (!keAttributes.is())
            return false;

        xAttrib = keAttributes->getNamedItem("spinCount");
        int spinCount = xAttrib.is() ? xAttrib->getNodeValue().toInt32() : 0;
        fprintf(stderr, "spincount is %d\n", spinCount);

        xAttrib = keAttributes->getNamedItem("saltSize");
        int pkeSaltSize = xAttrib.is() ? xAttrib->getNodeValue().toInt32() : 0;
        fprintf(stderr, "saltsize is %d\n", pkeSaltSize);

        xAttrib = keAttributes->getNamedItem("blockSize");
        int pkeBlockSize = xAttrib.is() ? xAttrib->getNodeValue().toInt32() : 0;
        fprintf(stderr, "blockSize is %d\n", pkeBlockSize);

        xAttrib = keAttributes->getNamedItem("keyBits");
        int pkeKeyBits = xAttrib.is() ? xAttrib->getNodeValue().toInt32() : 0;
        fprintf(stderr, "keyBits is %d\n", pkeKeyBits);

        xAttrib = keAttributes->getNamedItem("hashSize");
        int pkeHashSize = xAttrib.is() ? xAttrib->getNodeValue().toInt32() : 0;
        fprintf(stderr, "hashSize is %d\n", pkeHashSize);

        OUString pkeCipherAlgorithm;
        xAttrib = keAttributes->getNamedItem("cipherAlgorithm");
        if (xAttrib.is())
            pkeCipherAlgorithm = xAttrib->getNodeValue();
        if (pkeCipherAlgorithm.isEmpty())
            pkeCipherAlgorithm = "AES";

        fprintf(stderr, "alg is %s\n", OUStringToOString(pkeCipherAlgorithm, RTL_TEXTENCODING_UTF8).getStr());

        OUString pkeCipherChaining;
        xAttrib = keAttributes->getNamedItem("cipherChaining");
        if (xAttrib.is())
            pkeCipherChaining = xAttrib->getNodeValue();
        if (pkeCipherChaining.isEmpty())
            pkeCipherChaining = "ChainingModeCBC";

        fprintf(stderr, "chaining is %s\n", OUStringToOString(pkeCipherChaining, RTL_TEXTENCODING_UTF8).getStr());

        OUString pkeHashAlgorithm;
        xAttrib = keAttributes->getNamedItem("hashAlgorithm");
        if (xAttrib.is())
            pkeHashAlgorithm = xAttrib->getNodeValue();
        if (pkeHashAlgorithm.isEmpty())
            pkeHashAlgorithm = "SHA1";

        fprintf(stderr, "hash is %s\n", OUStringToOString(pkeHashAlgorithm, RTL_TEXTENCODING_UTF8).getStr());


        OUString pkeSaltValue;
        xAttrib = keAttributes->getNamedItem("saltValue");
        if (xAttrib.is())
            pkeSaltValue = xAttrib->getNodeValue();

        fprintf(stderr, "salt is %s\n", OUStringToOString(pkeSaltValue, RTL_TEXTENCODING_UTF8).getStr());

        OUString encryptedVerifierHashInput;
        xAttrib = keAttributes->getNamedItem("saltValue");
        if (xAttrib.is())
            encryptedVerifierHashInput = xAttrib->getNodeValue();

        fprintf(stderr, "hashinput is %s\n", OUStringToOString(encryptedVerifierHashInput, RTL_TEXTENCODING_UTF8).getStr());

        OUString encryptedVerifierHashValue;
        xAttrib = keAttributes->getNamedItem("encryptedVerifierHashValue");
        if (xAttrib.is())
            encryptedVerifierHashValue = xAttrib->getNodeValue();

        fprintf(stderr, "hashvalue is %s\n", OUStringToOString(encryptedVerifierHashValue, RTL_TEXTENCODING_UTF8).getStr());

        OUString pkeEncryptedKeyValue;
        xAttrib = keAttributes->getNamedItem("encryptedVerifierHashValue");
        if (xAttrib.is())
            pkeEncryptedKeyValue = xAttrib->getNodeValue();

        fprintf(stderr, "keyval is %s\n", OUStringToOString(pkeEncryptedKeyValue, RTL_TEXTENCODING_UTF8).getStr());

        return true;
    }
    else
    {
        sal_uInt32 nHeaderSize, nRepeatedFlags;
        aInfoStrm >> nHeaderSize >> nRepeatedFlags;
        if( (nHeaderSize < 20) || (nRepeatedFlags != rEncrInfo.mnFlags) )
            return false;

        aInfoStrm.skip( 4 );
        aInfoStrm >> rEncrInfo.mnAlgorithmId >> rEncrInfo.mnAlgorithmIdHash >> rEncrInfo.mnKeySize;
        aInfoStrm.skip( nHeaderSize - 20 );
        aInfoStrm >> rEncrInfo.mnSaltSize;
        if( rEncrInfo.mnSaltSize != 16 )
            return false;

        aInfoStrm.readMemory( rEncrInfo.mpnSalt, 16 );
        aInfoStrm.readMemory( rEncrInfo.mpnEncrVerifier, 16 );
        aInfoStrm >> rEncrInfo.mnVerifierHashSize;
        aInfoStrm.readMemory( rEncrInfo.mpnEncrVerifierHash, 32 );
    }
    return !aInfoStrm.isEof();
}

// ----------------------------------------------------------------------------

void lclDeriveKey( const sal_uInt8* pnHash, sal_uInt32 nHashLen, sal_uInt8* pnKeyDerived, sal_uInt32 nRequiredKeyLen )
{
    sal_uInt8 pnBuffer[ 64 ];
    memset( pnBuffer, 0x36, sizeof( pnBuffer ) );
    for( sal_uInt32 i = 0; i < nHashLen; ++i )
        pnBuffer[ i ] ^= pnHash[ i ];

    rtlDigest aDigest = rtl_digest_create( rtl_Digest_AlgorithmSHA1 );
    rtl_digest_update( aDigest, pnBuffer, sizeof( pnBuffer ) );
    sal_uInt8 pnX1[ RTL_DIGEST_LENGTH_SHA1 ];
    rtl_digest_get( aDigest, pnX1, RTL_DIGEST_LENGTH_SHA1 );
    rtl_digest_destroy( aDigest );

    memset( pnBuffer, 0x5C, sizeof( pnBuffer ) );
    for( sal_uInt32 i = 0; i < nHashLen; ++i )
        pnBuffer[ i ] ^= pnHash[ i ];

    aDigest = rtl_digest_create( rtl_Digest_AlgorithmSHA1 );
    rtl_digest_update( aDigest, pnBuffer, sizeof( pnBuffer ) );
    sal_uInt8 pnX2[ RTL_DIGEST_LENGTH_SHA1 ];
    rtl_digest_get( aDigest, pnX2, RTL_DIGEST_LENGTH_SHA1 );
    rtl_digest_destroy( aDigest );

    if( nRequiredKeyLen > RTL_DIGEST_LENGTH_SHA1 )
    {
        memcpy( pnKeyDerived + RTL_DIGEST_LENGTH_SHA1, pnX2, nRequiredKeyLen - RTL_DIGEST_LENGTH_SHA1 );
        nRequiredKeyLen = RTL_DIGEST_LENGTH_SHA1;
    }
    memcpy( pnKeyDerived, pnX1, nRequiredKeyLen );
}

// ----------------------------------------------------------------------------

bool lclCheckEncryptionData( const sal_uInt8* pnKey, sal_uInt32 nKeySize, const sal_uInt8* pnVerifier, sal_uInt32 nVerifierSize, const sal_uInt8* pnVerifierHash, sal_uInt32 nVerifierHashSize )
{
    bool bResult = false;

    // the only currently supported algorithm needs key size 128
    if ( nKeySize == 16 && nVerifierSize == 16 && nVerifierHashSize == 32 )
    {
        // check password
#if USE_TLS_OPENSSL
        EVP_CIPHER_CTX aes_ctx;
        EVP_CIPHER_CTX_init( &aes_ctx );
        EVP_DecryptInit_ex( &aes_ctx, EVP_aes_128_ecb(), 0, pnKey, 0 );
        EVP_CIPHER_CTX_set_padding( &aes_ctx, 0 );
        int nOutLen = 0;
        sal_uInt8 pnTmpVerifier[ 16 ];
        (void) memset( pnTmpVerifier, 0, sizeof(pnTmpVerifier) );

        /*int*/ EVP_DecryptUpdate( &aes_ctx, pnTmpVerifier, &nOutLen, pnVerifier, nVerifierSize );
        EVP_CIPHER_CTX_cleanup( &aes_ctx );

        EVP_CIPHER_CTX_init( &aes_ctx );
        EVP_DecryptInit_ex( &aes_ctx, EVP_aes_128_ecb(), 0, pnKey, 0 );
        EVP_CIPHER_CTX_set_padding( &aes_ctx, 0 );
        sal_uInt8 pnTmpVerifierHash[ 32 ];
        (void) memset( pnTmpVerifierHash, 0, sizeof(pnTmpVerifierHash) );

        /*int*/ EVP_DecryptUpdate( &aes_ctx, pnTmpVerifierHash, &nOutLen, pnVerifierHash, nVerifierHashSize );
        EVP_CIPHER_CTX_cleanup( &aes_ctx );
#endif // USE_TLS_OPENSSL

#if USE_TLS_NSS
        PK11SlotInfo *aSlot( PK11_GetBestSlot( CKM_AES_ECB, NULL ) );
        sal_uInt8 *key( new sal_uInt8[ nKeySize ] );
        (void) memcpy( key, pnKey, nKeySize * sizeof(sal_uInt8) );

        SECItem keyItem;
        keyItem.type = siBuffer;
        keyItem.data = key;
        keyItem.len  = nKeySize;

        PK11SymKey *symKey( PK11_ImportSymKey( aSlot, CKM_AES_ECB, PK11_OriginUnwrap, CKA_ENCRYPT, &keyItem, NULL ) );
        SECItem *secParam( PK11_ParamFromIV( CKM_AES_ECB, NULL ) );
        PK11Context *encContext( PK11_CreateContextBySymKey( CKM_AES_ECB, CKA_DECRYPT, symKey, secParam ) );

        int nOutLen(0);
        sal_uInt8 pnTmpVerifier[ 16 ];
        (void) memset( pnTmpVerifier, 0, sizeof(pnTmpVerifier) );

        PK11_CipherOp( encContext, pnTmpVerifier, &nOutLen, sizeof(pnTmpVerifier), const_cast<sal_uInt8*>(pnVerifier), nVerifierSize );

        sal_uInt8 pnTmpVerifierHash[ 32 ];
        (void) memset( pnTmpVerifierHash, 0, sizeof(pnTmpVerifierHash) );
        PK11_CipherOp( encContext, pnTmpVerifierHash, &nOutLen, sizeof(pnTmpVerifierHash), const_cast<sal_uInt8*>(pnVerifierHash), nVerifierHashSize );

        PK11_DestroyContext( encContext, PR_TRUE );
        PK11_FreeSymKey( symKey );
        SECITEM_FreeItem( secParam, PR_TRUE );
        delete[] key;
#endif // USE_TLS_NSS

        rtlDigest aDigest = rtl_digest_create( rtl_Digest_AlgorithmSHA1 );
        rtl_digest_update( aDigest, pnTmpVerifier, sizeof( pnTmpVerifier ) );
        sal_uInt8 pnSha1Hash[ RTL_DIGEST_LENGTH_SHA1 ];
        rtl_digest_get( aDigest, pnSha1Hash, RTL_DIGEST_LENGTH_SHA1 );
        rtl_digest_destroy( aDigest );

        bResult = ( memcmp( pnSha1Hash, pnTmpVerifierHash, RTL_DIGEST_LENGTH_SHA1 ) == 0 );
    }

    return bResult;
}

// ----------------------------------------------------------------------------

Sequence< NamedValue > lclGenerateEncryptionKey( const PackageEncryptionInfo& rEncrInfo, const OUString& rPassword, sal_uInt8* pnKey, sal_uInt32 nRequiredKeyLen )
{
    size_t nBufferSize = rEncrInfo.mnSaltSize + 2 * rPassword.getLength();
    sal_uInt8* pnBuffer = new sal_uInt8[ nBufferSize ];
    memcpy( pnBuffer, rEncrInfo.mpnSalt, rEncrInfo.mnSaltSize );

    sal_uInt8* pnPasswordLoc = pnBuffer + rEncrInfo.mnSaltSize;
    const sal_Unicode* pStr = rPassword.getStr();
    for( sal_Int32 i = 0, nLen = rPassword.getLength(); i < nLen; ++i, ++pStr, pnPasswordLoc += 2 )
        ByteOrderConverter::writeLittleEndian( pnPasswordLoc, static_cast< sal_uInt16 >( *pStr ) );

    rtlDigest aDigest = rtl_digest_create( rtl_Digest_AlgorithmSHA1 );
    rtl_digest_update( aDigest, pnBuffer, nBufferSize );
    delete[] pnBuffer;

    size_t nHashSize = RTL_DIGEST_LENGTH_SHA1 + 4;
    sal_uInt8* pnHash = new sal_uInt8[ nHashSize ];
    rtl_digest_get( aDigest, pnHash + 4, RTL_DIGEST_LENGTH_SHA1 );
    rtl_digest_destroy( aDigest );

    for( sal_uInt32 i = 0; i < 50000; ++i )
    {
        ByteOrderConverter::writeLittleEndian( pnHash, i );
        aDigest = rtl_digest_create( rtl_Digest_AlgorithmSHA1 );
        rtl_digest_update( aDigest, pnHash, nHashSize );
        rtl_digest_get( aDigest, pnHash + 4, RTL_DIGEST_LENGTH_SHA1 );
        rtl_digest_destroy( aDigest );
    }

    memmove( pnHash, pnHash + 4, RTL_DIGEST_LENGTH_SHA1 );
    memset( pnHash + RTL_DIGEST_LENGTH_SHA1, 0, 4 );
    aDigest = rtl_digest_create( rtl_Digest_AlgorithmSHA1 );
    rtl_digest_update( aDigest, pnHash, nHashSize );
    rtl_digest_get( aDigest, pnHash, RTL_DIGEST_LENGTH_SHA1 );
    rtl_digest_destroy( aDigest );

    lclDeriveKey( pnHash, RTL_DIGEST_LENGTH_SHA1, pnKey, nRequiredKeyLen );
    delete[] pnHash;

    Sequence< NamedValue > aResult;
    if( lclCheckEncryptionData( pnKey, nRequiredKeyLen, rEncrInfo.mpnEncrVerifier, sizeof( rEncrInfo.mpnEncrVerifier ), rEncrInfo.mpnEncrVerifierHash, sizeof( rEncrInfo.mpnEncrVerifierHash ) ) )
    {
        SequenceAsHashMap aEncryptionData;
        aEncryptionData[ "AES128EncryptionKey" ] <<= Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( pnKey ), nRequiredKeyLen );
        aEncryptionData[ "AES128EncryptionSalt" ] <<= Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( rEncrInfo.mpnSalt ), rEncrInfo.mnSaltSize );
        aEncryptionData[ "AES128EncryptionVerifier" ] <<= Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( rEncrInfo.mpnEncrVerifier ), sizeof( rEncrInfo.mpnEncrVerifier ) );
        aEncryptionData[ "AES128EncryptionVerifierHash" ] <<= Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( rEncrInfo.mpnEncrVerifierHash ), sizeof( rEncrInfo.mpnEncrVerifierHash ) );
        aResult = aEncryptionData.getAsConstNamedValueList();
    }

    return aResult;
}

// the password verifier ------------------------------------------------------

class PasswordVerifier : public ::comphelper::IDocPasswordVerifier
{
public:
    explicit            PasswordVerifier( const PackageEncryptionInfo& rEncryptInfo );

    virtual ::comphelper::DocPasswordVerifierResult
                        verifyPassword( const OUString& rPassword, Sequence< NamedValue >& o_rEncryptionData );
    virtual ::comphelper::DocPasswordVerifierResult
                        verifyEncryptionData( const Sequence< NamedValue >& rEncryptionData );

    inline const sal_uInt8* getKey() const { return &maKey.front(); }

private:
    const PackageEncryptionInfo& mrEncryptInfo;
    ::std::vector< sal_uInt8 > maKey;
};

PasswordVerifier::PasswordVerifier( const PackageEncryptionInfo& rEncryptInfo ) :
    mrEncryptInfo( rEncryptInfo ),
    maKey( static_cast< size_t >( rEncryptInfo.mnKeySize / 8 ), 0 )
{
}

::comphelper::DocPasswordVerifierResult PasswordVerifier::verifyPassword( const OUString& rPassword, Sequence< NamedValue >& o_rEncryptionData )
{
    // verifies the password and writes the related decryption key into maKey
    o_rEncryptionData = lclGenerateEncryptionKey( mrEncryptInfo, rPassword, &maKey.front(), maKey.size() );
    return o_rEncryptionData.hasElements() ? ::comphelper::DocPasswordVerifierResult_OK : ::comphelper::DocPasswordVerifierResult_WRONG_PASSWORD;
}

::comphelper::DocPasswordVerifierResult PasswordVerifier::verifyEncryptionData( const Sequence< NamedValue >& rEncryptionData )
{
    SequenceAsHashMap aHashData( rEncryptionData );
    Sequence< sal_Int8 > aKey = aHashData.getUnpackedValueOrDefault( "AES128EncryptionKey", Sequence< sal_Int8 >() );
    Sequence< sal_Int8 > aVerifier = aHashData.getUnpackedValueOrDefault( "AES128EncryptionVerifier", Sequence< sal_Int8 >() );
    Sequence< sal_Int8 > aVerifierHash = aHashData.getUnpackedValueOrDefault( "AES128EncryptionVerifierHash", Sequence< sal_Int8 >() );

    bool bResult = lclCheckEncryptionData(
        reinterpret_cast< const sal_uInt8* >( aKey.getConstArray() ), aKey.getLength(),
        reinterpret_cast< const sal_uInt8* >( aVerifier.getConstArray() ), aVerifier.getLength(),
        reinterpret_cast< const sal_uInt8* >( aVerifierHash.getConstArray() ), aVerifierHash.getLength() );

    return bResult ? ::comphelper::DocPasswordVerifierResult_OK : ::comphelper::DocPasswordVerifierResult_WRONG_PASSWORD;
}

} // namespace

// ----------------------------------------------------------------------------

Reference< XInputStream > FilterDetect::extractUnencryptedPackage( MediaDescriptor& rMediaDesc ) const
{
    // try the plain input stream
    Reference< XInputStream > xInStrm( rMediaDesc[ MediaDescriptor::PROP_INPUTSTREAM() ], UNO_QUERY );
    if( !xInStrm.is() || lclIsZipPackage( mxContext, xInStrm ) )
        return xInStrm;

    // check if a temporary file is passed in the 'ComponentData' property
    Reference< XStream > xDecrypted( rMediaDesc.getComponentDataEntry( "DecryptedPackage" ), UNO_QUERY );
    if( xDecrypted.is() )
    {
        Reference< XInputStream > xDecrInStrm = xDecrypted->getInputStream();
        if( lclIsZipPackage( mxContext, xDecrInStrm ) )
            return xDecrInStrm;
    }

    // try to decrypt an encrypted OLE package
    ::oox::ole::OleStorage aOleStorage( mxContext, xInStrm, false );
    if( aOleStorage.isStorage() ) try
    {
        // open the required input streams in the encrypted package
        Reference< XInputStream > xEncryptionInfo( aOleStorage.openInputStream( "EncryptionInfo" ), UNO_SET_THROW );
        Reference< XInputStream > xEncryptedPackage( aOleStorage.openInputStream( "EncryptedPackage" ), UNO_SET_THROW );

        fprintf(stderr, "EncryptedPackage %d %d\n", xEncryptionInfo.is(), xEncryptedPackage.is());

        // read the encryption info stream
        PackageEncryptionInfo aEncryptInfo;
        bool bValidInfo = lclReadEncryptionInfo( mxContext, aEncryptInfo, xEncryptionInfo );

        // check flags and algorithm IDs, required are AES128 and SHA-1
        bool bImplemented = bValidInfo &&
            getFlag( aEncryptInfo.mnFlags, ENCRYPTINFO_CRYPTOAPI ) &&
            getFlag( aEncryptInfo.mnFlags, ENCRYPTINFO_AES ) &&
            // algorithm ID 0 defaults to AES128 too, if ENCRYPTINFO_AES flag is set
            ((aEncryptInfo.mnAlgorithmId == 0) || (aEncryptInfo.mnAlgorithmId == ENCRYPT_ALGO_AES128)) &&
            // hash algorithm ID 0 defaults to SHA-1 too
            ((aEncryptInfo.mnAlgorithmIdHash == 0) || (aEncryptInfo.mnAlgorithmIdHash == ENCRYPT_HASH_SHA1)) &&
            (aEncryptInfo.mnVerifierHashSize == 20);

        fprintf(stderr, "bImplemented is %d\n", bImplemented);


        if( bImplemented )
        {
#if USE_TLS_NSS
            // Initialize NSS, database functions are not needed
            NSS_NoDB_Init( NULL );
#endif // USE_TLS_NSS

            /*  "VelvetSweatshop" is the built-in default encryption
                password used by MS Excel for the "workbook protection"
                feature with password. Try this first before prompting the
                user for a password. */
            ::std::vector< OUString > aDefaultPasswords;
            aDefaultPasswords.push_back( "VelvetSweatshop" );

            /*  Use the comphelper password helper to request a password.
                This helper returns either with the correct password
                (according to the verifier), or with an empty string if
                user has cancelled the password input dialog. */
            PasswordVerifier aVerifier( aEncryptInfo );
            Sequence< NamedValue > aEncryptionData = ::comphelper::DocPasswordHelper::requestAndVerifyDocPassword(
                aVerifier, rMediaDesc, ::comphelper::DocPasswordRequestType_MS, &aDefaultPasswords );

            if( aEncryptionData.getLength() == 0 )
            {
                rMediaDesc[ MediaDescriptor::PROP_ABORTED() ] <<= true;
            }
            else
            {
                // create temporary file for unencrypted package
                Reference< XStream > xTempFile( TempFile::create(mxContext), UNO_QUERY_THROW );
                Reference< XOutputStream > xDecryptedPackage( xTempFile->getOutputStream(), UNO_SET_THROW );
                BinaryXOutputStream aDecryptedPackage( xDecryptedPackage, true );
                BinaryXInputStream aEncryptedPackage( xEncryptedPackage, true );

#if USE_TLS_OPENSSL
                EVP_CIPHER_CTX aes_ctx;
                EVP_CIPHER_CTX_init( &aes_ctx );
                EVP_DecryptInit_ex( &aes_ctx, EVP_aes_128_ecb(), 0, aVerifier.getKey(), 0 );
                EVP_CIPHER_CTX_set_padding( &aes_ctx, 0 );
#endif // USE_TLS_OPENSSL

#if USE_TLS_NSS
                // Retrieve the valid key so we can get its size later
                SequenceAsHashMap aHashData( aEncryptionData );
                Sequence<sal_Int8> validKey( aHashData.getUnpackedValueOrDefault( OUString("AES128EncryptionKey"), Sequence<sal_Int8>() ) );

                PK11SlotInfo *aSlot( PK11_GetBestSlot( CKM_AES_ECB, NULL ) );
                sal_uInt8 *key = new sal_uInt8[ validKey.getLength() ];
                (void) memcpy( key, aVerifier.getKey(), validKey.getLength() );

                SECItem keyItem;
                keyItem.type = siBuffer;
                keyItem.data = key;
                keyItem.len  = validKey.getLength();

                PK11SymKey *symKey( PK11_ImportSymKey( aSlot, CKM_AES_ECB, PK11_OriginUnwrap, CKA_ENCRYPT, &keyItem, NULL ) );
                SECItem *secParam( PK11_ParamFromIV( CKM_AES_ECB, NULL ) );
                PK11Context *encContext( PK11_CreateContextBySymKey( CKM_AES_ECB, CKA_DECRYPT, symKey, secParam ) );
#endif // USE_TLS_NSS

                sal_uInt8 pnInBuffer[ 1024 ];
                sal_uInt8 pnOutBuffer[ 1024 ];
                sal_Int32 nInLen;
                int nOutLen;
                aEncryptedPackage.skip( 8 ); // decrypted size
                while( (nInLen = aEncryptedPackage.readMemory( pnInBuffer, sizeof( pnInBuffer ) )) > 0 )
                {
#if USE_TLS_OPENSSL
                    EVP_DecryptUpdate( &aes_ctx, pnOutBuffer, &nOutLen, pnInBuffer, nInLen );
#endif // USE_TLS_OPENSSL

#if USE_TLS_NSS
                    PK11_CipherOp( encContext, pnOutBuffer, &nOutLen, sizeof(pnOutBuffer), pnInBuffer, nInLen );
#endif // USE_TLS_NSS
                    aDecryptedPackage.writeMemory( pnOutBuffer, nOutLen );
                }
#if USE_TLS_OPENSSL
                EVP_DecryptFinal_ex( &aes_ctx, pnOutBuffer, &nOutLen );
#endif // USE_TLS_OPENSSL

#if USE_TLS_NSS
                uint final;
                PK11_DigestFinal( encContext, pnOutBuffer, &final, nInLen - nOutLen );
                nOutLen = final;
#endif // USE_TLS_NSS
                aDecryptedPackage.writeMemory( pnOutBuffer, nOutLen );

#if USE_TLS_OPENSSL
                EVP_CIPHER_CTX_cleanup( &aes_ctx );
#endif // USE_TLS_OPENSSL

#if USE_TLS_NSS
                PK11_DestroyContext( encContext, PR_TRUE );
                PK11_FreeSymKey( symKey );
                SECITEM_FreeItem( secParam, PR_TRUE );
                delete[] key;
#endif // USE_TLS_NSS
                xDecryptedPackage->flush();
                aDecryptedPackage.seekToStart();

                // store temp file in media descriptor to keep it alive
                rMediaDesc.setComponentDataEntry( "DecryptedPackage", Any( xTempFile ) );

                Reference< XInputStream > xDecrInStrm = xTempFile->getInputStream();
                if( lclIsZipPackage( mxContext, xDecrInStrm ) )
                    return xDecrInStrm;
            }
        }
    }
    catch( const Exception& )
    {
    }

    return Reference< XInputStream >();
}

// com.sun.star.lang.XServiceInfo interface -----------------------------------

OUString SAL_CALL FilterDetect::getImplementationName() throw( RuntimeException )
{
    return FilterDetect_getImplementationName();
}

sal_Bool SAL_CALL FilterDetect::supportsService( const OUString& rServiceName ) throw( RuntimeException )
{
    const Sequence< OUString > aServices = FilterDetect_getSupportedServiceNames();
    const OUString* pArray = aServices.getConstArray();
    const OUString* pArrayEnd = pArray + aServices.getLength();
    return ::std::find( pArray, pArrayEnd, rServiceName ) != pArrayEnd;
}

Sequence< OUString > SAL_CALL FilterDetect::getSupportedServiceNames() throw( RuntimeException )
{
    return FilterDetect_getSupportedServiceNames();
}

// com.sun.star.document.XExtendedFilterDetection interface -------------------

OUString SAL_CALL FilterDetect::detect( Sequence< PropertyValue >& rMediaDescSeq ) throw( RuntimeException )
{
    OUString aFilterName;
    MediaDescriptor aMediaDesc( rMediaDescSeq );

    /*  Check that the user has not choosen to abort detection, e.g. by hitting
        'Cancel' in the password input dialog. This may happen because this
        filter detection is used by different filters. */
    bool bAborted = aMediaDesc.getUnpackedValueOrDefault( MediaDescriptor::PROP_ABORTED(), false );
    if( !bAborted ) try
    {
        aMediaDesc.addInputStream();

        /*  Get the unencrypted input stream. This may include creation of a
            temporary file that contains the decrypted package. This temporary
            file will be stored in the 'ComponentData' property of the media
            descriptor. */
        Reference< XInputStream > xInStrm( extractUnencryptedPackage( aMediaDesc ), UNO_SET_THROW );

        // stream must be a ZIP package
        ZipStorage aZipStorage( mxContext, xInStrm );
        if( aZipStorage.isStorage() )
        {
            // create the fast parser, register the XML namespaces, set document handler
            FastParser aParser( mxContext );
            aParser.registerNamespace( NMSP_packageRel );
            aParser.registerNamespace( NMSP_officeRel );
            aParser.registerNamespace( NMSP_packageContentTypes );
            aParser.setDocumentHandler( new FilterDetectDocHandler( mxContext, aFilterName ) );

            /*  Parse '_rels/.rels' to get the target path and '[Content_Types].xml'
                to determine the content type of the part at the target path. */
            aParser.parseStream( aZipStorage, "_rels/.rels" );
            aParser.parseStream( aZipStorage, "[Content_Types].xml" );
        }
    }
    catch( const Exception& )
    {
    }

    // write back changed media descriptor members
    aMediaDesc >> rMediaDescSeq;
    return aFilterName;
}

// ============================================================================

} // namespace core
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
