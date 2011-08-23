/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "oox/core/filterdetect.hxx"
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/xml/sax/XFastParser.hpp>
#include <rtl/digest.h>
#include <openssl/evp.h>
#include <comphelper/docpasswordhelper.hxx>
#include <comphelper/mediadescriptor.hxx>
#include "oox/helper/attributelist.hxx"
#include "oox/helper/binaryinputstream.hxx"
#include "oox/helper/binaryoutputstream.hxx"
#include "oox/helper/zipstorage.hxx"
#include "oox/core/fasttokenhandler.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/ole/olestorage.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::UNO_SET_THROW;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::beans::NamedValue;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::io::XInputStream;
using ::com::sun::star::io::XOutputStream;
using ::com::sun::star::io::XStream;
using ::com::sun::star::xml::sax::InputSource;
using ::com::sun::star::xml::sax::SAXException;
using ::com::sun::star::xml::sax::XFastAttributeList;
using ::com::sun::star::xml::sax::XFastContextHandler;
using ::com::sun::star::xml::sax::XFastParser;
using ::com::sun::star::xml::sax::XLocator;
using ::comphelper::MediaDescriptor;
using ::comphelper::SequenceAsHashMap;

namespace oox {
namespace core {

// ============================================================================

FilterDetectDocHandler::FilterDetectDocHandler( OUString& rFilterName ) :
    mrFilterName( rFilterName )
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
        case NMSP_PACKAGE_RELATIONSHIPS|XML_Relationships:
        break;
        case NMSP_PACKAGE_RELATIONSHIPS|XML_Relationship:
            if( !maContextStack.empty() && (maContextStack.back() == (NMSP_PACKAGE_RELATIONSHIPS|XML_Relationships)) )
                parseRelationship( aAttribs );
        break;

        // cases for [Content_Types].xml
        case NMSP_CONTENT_TYPES|XML_Types:
        break;
        case NMSP_CONTENT_TYPES|XML_Default:
            if( !maContextStack.empty() && (maContextStack.back() == (NMSP_CONTENT_TYPES|XML_Types)) )
                parseContentTypesDefault( aAttribs );
        break;
        case NMSP_CONTENT_TYPES|XML_Override:
            if( !maContextStack.empty() && (maContextStack.back() == (NMSP_CONTENT_TYPES|XML_Types)) )
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
    if( aType.equalsAscii( "http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" ) )
        maTargetPath = OUString( sal_Unicode( '/' ) ) + rAttribs.getString( XML_Target, OUString() );
}

OUString FilterDetectDocHandler::getFilterNameFromContentType( const OUString& rContentType ) const
{
    if( rContentType.equalsAscii( "application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml" ) ||
        rContentType.equalsAscii( "application/vnd.ms-word.document.macroEnabled.main+xml" ) )
        return CREATE_OUSTRING( "writer_MS_Word_2007" );

    if( rContentType.equalsAscii( "application/vnd.openxmlformats-officedocument.wordprocessingml.template.main+xml" ) ||
        rContentType.equalsAscii( "application/vnd.ms-word.template.macroEnabledTemplate.main+xml" ) )
        return CREATE_OUSTRING( "writer_MS_Word_2007_Template" );

    if( rContentType.equalsAscii( "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml" ) ||
        rContentType.equalsAscii( "application/vnd.ms-excel.sheet.macroEnabled.main+xml" ) )
        return CREATE_OUSTRING( "MS Excel 2007 XML" );

    if( rContentType.equalsAscii( "application/vnd.openxmlformats-officedocument.spreadsheetml.template.main+xml" ) ||
        rContentType.equalsAscii( "application/vnd.ms-excel.template.macroEnabled.main+xml" ) )
        return CREATE_OUSTRING( "MS Excel 2007 XML Template" );

    if( rContentType.equalsAscii( "application/vnd.ms-excel.sheet.binary.macroEnabled.main" ) )
        return CREATE_OUSTRING( "MS Excel 2007 Binary" );

    if( rContentType.equalsAscii( "application/vnd.openxmlformats-officedocument.presentationml.presentation.main+xml" ) ||
        rContentType.equalsAscii( "application/vnd.ms-powerpoint.presentation.macroEnabled.main+xml" ) )
        return CREATE_OUSTRING( "MS PowerPoint 2007 XML" );

    if( rContentType.equalsAscii( "application/vnd.openxmlformats-officedocument.presentationml.slideshow.main+xml" ) ||
        rContentType.equalsAscii( "application/vnd.ms-powerpoint.slideshow.macroEnabled.main+xml" ) )
        return CREATE_OUSTRING( "MS PowerPoint 2007 XML AutoPlay" );

    if( rContentType.equalsAscii( "application/vnd.openxmlformats-officedocument.presentationml.template.main+xml" ) ||
        rContentType.equalsAscii( "application/vnd.ms-powerpoint.template.macroEnabled.main+xml" ) )
        return CREATE_OUSTRING( "MS PowerPoint 2007 XML Template" );

    return OUString();
}

void FilterDetectDocHandler::parseContentTypesDefault( const AttributeList& rAttribs )
{
    // only if no overridden part name found
    if( mrFilterName.getLength() == 0 )
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
    aServiceNames[ 0 ] = CREATE_OUSTRING( "com.sun.star.frame.ExtendedTypeDetection" );
    return aServiceNames;
}

/* Helper for XServiceInfo */
OUString FilterDetect_getImplementationName()
{
    return CREATE_OUSTRING( "com.sun.star.comp.oox.FormatDetector" );
}

/* Helper for registry */
Reference< XInterface > SAL_CALL FilterDetect_createInstance( const Reference< XMultiServiceFactory >& xServiceManager ) throw( Exception )
{
    return Reference< XInterface >( *new FilterDetect( xServiceManager ) );
}

// ----------------------------------------------------------------------------

FilterDetect::FilterDetect( const Reference< XMultiServiceFactory >& rxFactory ) :
    mxFactory( rxFactory )
{
    OSL_ENSURE( mxFactory.is(), "FilterDetect::FilterDetect - no service factory" );
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

const sal_uInt32 ENCRYPT_ALGO_AES128        = 0x0000660E;
const sal_uInt32 ENCRYPT_ALGO_AES192        = 0x0000660F;
const sal_uInt32 ENCRYPT_ALGO_AES256        = 0x00006610;
const sal_uInt32 ENCRYPT_ALGO_RC4           = 0x00006801;

const sal_uInt32 ENCRYPT_HASH_SHA1          = 0x00008004;

// ----------------------------------------------------------------------------

bool lclIsZipPackage( const Reference< XMultiServiceFactory >& rxFactory, const Reference< XInputStream >& rxInStrm )
{
    ZipStorage aZipStorage( rxFactory, rxInStrm );
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

bool lclReadEncryptionInfo( PackageEncryptionInfo& rEncrInfo, BinaryInputStream& rStrm )
{
    rStrm.skip( 4 );
    rStrm >> rEncrInfo.mnFlags;
    if( getFlag( rEncrInfo.mnFlags, ENCRYPTINFO_EXTERNAL ) )
        return false;

    sal_uInt32 nHeaderSize, nRepeatedFlags;
    rStrm >> nHeaderSize >> nRepeatedFlags;
    if( (nHeaderSize < 20) || (nRepeatedFlags != rEncrInfo.mnFlags) )
        return false;

    rStrm.skip( 4 );
    rStrm >> rEncrInfo.mnAlgorithmId >> rEncrInfo.mnAlgorithmIdHash >> rEncrInfo.mnKeySize;
    rStrm.skip( nHeaderSize - 20 );
    rStrm >> rEncrInfo.mnSaltSize;
    if( rEncrInfo.mnSaltSize != 16 )
        return false;

    rStrm.readMemory( rEncrInfo.mpnSalt, 16 );
    rStrm.readMemory( rEncrInfo.mpnEncrVerifier, 16 );
    rStrm >> rEncrInfo.mnVerifierHashSize;
    rStrm.readMemory( rEncrInfo.mpnEncrVerifierHash, 32 );
    return !rStrm.isEof();
}

// ----------------------------------------------------------------------------

void lclDeriveKey( const sal_uInt8* pnHash, sal_uInt32 nHashLen, sal_uInt8* pnKeyDerived, sal_uInt32 nRequiredKeyLen )
{
    sal_uInt8 pnBuffer[ 64 ];
    memset( pnBuffer, 0x36, sizeof( pnBuffer ) );
    for( sal_uInt32 i = 0; i < nHashLen; ++i )
        pnBuffer[ i ] ^= pnHash[ i ];

    rtlDigest aDigest = rtl_digest_create( rtl_Digest_AlgorithmSHA1 );
    rtlDigestError aError = rtl_digest_update( aDigest, pnBuffer, sizeof( pnBuffer ) );
    sal_uInt8 pnX1[ RTL_DIGEST_LENGTH_SHA1 ];
    aError = rtl_digest_get( aDigest, pnX1, RTL_DIGEST_LENGTH_SHA1 );
    rtl_digest_destroy( aDigest );

    memset( pnBuffer, 0x5C, sizeof( pnBuffer ) );
    for( sal_uInt32 i = 0; i < nHashLen; ++i )
        pnBuffer[ i ] ^= pnHash[ i ];

    aDigest = rtl_digest_create( rtl_Digest_AlgorithmSHA1 );
    aError = rtl_digest_update( aDigest, pnBuffer, sizeof( pnBuffer ) );
    sal_uInt8 pnX2[ RTL_DIGEST_LENGTH_SHA1 ];
    aError = rtl_digest_get( aDigest, pnX2, RTL_DIGEST_LENGTH_SHA1 );
    rtl_digest_destroy( aDigest );

    if( nRequiredKeyLen > RTL_DIGEST_LENGTH_SHA1 )
    {
        memcpy( pnKeyDerived + RTL_DIGEST_LENGTH_SHA1, pnX2, nRequiredKeyLen - RTL_DIGEST_LENGTH_SHA1 );
        nRequiredKeyLen = RTL_DIGEST_LENGTH_SHA1;
    }
    memcpy( pnKeyDerived, pnX1, nRequiredKeyLen );
}

// ----------------------------------------------------------------------------

bool lclGenerateEncryptionKey( const PackageEncryptionInfo& rEncrInfo, const OUString& rPassword, sal_uInt8* pnKey, sal_uInt32 nRequiredKeyLen )
{
    size_t nBufferSize = rEncrInfo.mnSaltSize + 2 * rPassword.getLength();
    sal_uInt8* pnBuffer = new sal_uInt8[ nBufferSize ];
    memcpy( pnBuffer, rEncrInfo.mpnSalt, rEncrInfo.mnSaltSize );

    sal_uInt8* pnPasswordLoc = pnBuffer + rEncrInfo.mnSaltSize;
    const sal_Unicode* pStr = rPassword.getStr();
    for( sal_Int32 i = 0, nLen = rPassword.getLength(); i < nLen; ++i, ++pStr, pnPasswordLoc += 2 )
        ByteOrderConverter::writeLittleEndian( pnPasswordLoc, static_cast< sal_uInt16 >( *pStr ) );

    rtlDigest aDigest = rtl_digest_create( rtl_Digest_AlgorithmSHA1 );
    rtlDigestError aError = rtl_digest_update( aDigest, pnBuffer, nBufferSize );
    delete[] pnBuffer;

    size_t nHashSize = RTL_DIGEST_LENGTH_SHA1 + 4;
    sal_uInt8* pnHash = new sal_uInt8[ nHashSize ];
    aError = rtl_digest_get( aDigest, pnHash + 4, RTL_DIGEST_LENGTH_SHA1 );
    rtl_digest_destroy( aDigest );

    for( sal_uInt32 i = 0; i < 50000; ++i )
    {
        ByteOrderConverter::writeLittleEndian( pnHash, i );
        aDigest = rtl_digest_create( rtl_Digest_AlgorithmSHA1 );
        aError = rtl_digest_update( aDigest, pnHash, nHashSize );
        aError = rtl_digest_get( aDigest, pnHash + 4, RTL_DIGEST_LENGTH_SHA1 );
        rtl_digest_destroy( aDigest );
    }

    memmove( pnHash, pnHash + 4, RTL_DIGEST_LENGTH_SHA1 );
    memset( pnHash + RTL_DIGEST_LENGTH_SHA1, 0, 4 );
    aDigest = rtl_digest_create( rtl_Digest_AlgorithmSHA1 );
    aError = rtl_digest_update( aDigest, pnHash, nHashSize );
    aError = rtl_digest_get( aDigest, pnHash, RTL_DIGEST_LENGTH_SHA1 );
    rtl_digest_destroy( aDigest );

    lclDeriveKey( pnHash, RTL_DIGEST_LENGTH_SHA1, pnKey, nRequiredKeyLen );
    delete[] pnHash;

    // check password
    EVP_CIPHER_CTX aes_ctx;
    EVP_CIPHER_CTX_init( &aes_ctx );
    EVP_DecryptInit_ex( &aes_ctx, EVP_aes_128_ecb(), 0, pnKey, 0 );
    EVP_CIPHER_CTX_set_padding( &aes_ctx, 0 );
    int nOutLen = 0;
    sal_uInt8 pnVerifier[ 16 ] = { 0 };
    /*int*/ EVP_DecryptUpdate( &aes_ctx, pnVerifier, &nOutLen, rEncrInfo.mpnEncrVerifier, sizeof( rEncrInfo.mpnEncrVerifier ) );
    EVP_CIPHER_CTX_cleanup( &aes_ctx );

    EVP_CIPHER_CTX_init( &aes_ctx );
    EVP_DecryptInit_ex( &aes_ctx, EVP_aes_128_ecb(), 0, pnKey, 0 );
    EVP_CIPHER_CTX_set_padding( &aes_ctx, 0 );
    sal_uInt8 pnVerifierHash[ 32 ] = { 0 };
    /*int*/ EVP_DecryptUpdate( &aes_ctx, pnVerifierHash, &nOutLen, rEncrInfo.mpnEncrVerifierHash, sizeof( rEncrInfo.mpnEncrVerifierHash ) );
    EVP_CIPHER_CTX_cleanup( &aes_ctx );

    aDigest = rtl_digest_create( rtl_Digest_AlgorithmSHA1 );
    aError = rtl_digest_update( aDigest, pnVerifier, sizeof( pnVerifier ) );
    sal_uInt8 pnSha1Hash[ RTL_DIGEST_LENGTH_SHA1 ];
    aError = rtl_digest_get( aDigest, pnSha1Hash, RTL_DIGEST_LENGTH_SHA1 );
    rtl_digest_destroy( aDigest );

    return memcmp( pnSha1Hash, pnVerifierHash, RTL_DIGEST_LENGTH_SHA1 ) == 0;
}

// the password verifier ------------------------------------------------------

class PasswordVerifier : public ::comphelper::IDocPasswordVerifier
{
public:
    explicit            PasswordVerifier( const PackageEncryptionInfo& rEncryptInfo );

    virtual ::comphelper::DocPasswordVerifierResult
                        verifyPassword( const OUString& rPassword );

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

::comphelper::DocPasswordVerifierResult PasswordVerifier::verifyPassword( const OUString& rPassword )
{
    // verifies the password and writes the related decryption key into maKey
    return lclGenerateEncryptionKey( mrEncryptInfo, rPassword, &maKey.front(), maKey.size() ) ?
        ::comphelper::DocPasswordVerifierResult_OK : ::comphelper::DocPasswordVerifierResult_WRONG_PASSWORD;
}

} // namespace

// ----------------------------------------------------------------------------

Reference< XInputStream > FilterDetect::extractUnencryptedPackage( MediaDescriptor& rMediaDesc ) const
{
    if( mxFactory.is() )
    {
        // try the plain input stream
        Reference< XInputStream > xInStrm( rMediaDesc[ MediaDescriptor::PROP_INPUTSTREAM() ], UNO_QUERY );
        if( !xInStrm.is() || lclIsZipPackage( mxFactory, xInStrm ) )
            return xInStrm;

        // check if a temporary file is passed in the 'ComponentData' property
        Reference< XStream > xDecrypted( rMediaDesc.getComponentDataEntry( CREATE_OUSTRING( "DecryptedPackage" ) ), UNO_QUERY );
        if( xDecrypted.is() )
        {
            Reference< XInputStream > xDecrInStrm = xDecrypted->getInputStream();
            if( lclIsZipPackage( mxFactory, xDecrInStrm ) )
                return xDecrInStrm;
        }

        // try to decrypt an encrypted OLE package
        ::oox::ole::OleStorage aOleStorage( mxFactory, xInStrm, false );
        if( aOleStorage.isStorage() ) try
        {
            // open the required input streams in the encrypted package
            Reference< XInputStream > xEncryptionInfo( aOleStorage.openInputStream( CREATE_OUSTRING( "EncryptionInfo" ) ), UNO_SET_THROW );
            Reference< XInputStream > xEncryptedPackage( aOleStorage.openInputStream( CREATE_OUSTRING( "EncryptedPackage" ) ), UNO_SET_THROW );

            // read the encryption info stream
            PackageEncryptionInfo aEncryptInfo;
            BinaryXInputStream aInfoStrm( xEncryptionInfo, true );
            bool bValidInfo = lclReadEncryptionInfo( aEncryptInfo, aInfoStrm );

            // check flags and agorithm IDs, requiered are AES128 and SHA-1
            bool bImplemented = bValidInfo &&
                getFlag( aEncryptInfo.mnFlags, ENCRYPTINFO_CRYPTOAPI ) &&
                getFlag( aEncryptInfo.mnFlags, ENCRYPTINFO_AES ) &&
                // algorithm ID 0 defaults to AES128 too, if ENCRYPTINFO_AES flag is set
                ((aEncryptInfo.mnAlgorithmId == 0) || (aEncryptInfo.mnAlgorithmId == ENCRYPT_ALGO_AES128)) &&
                // hash algorithm ID 0 defaults to SHA-1 too
                ((aEncryptInfo.mnAlgorithmIdHash == 0) || (aEncryptInfo.mnAlgorithmIdHash == ENCRYPT_HASH_SHA1)) &&
                (aEncryptInfo.mnVerifierHashSize == 20);

            if( bImplemented )
            {
                /*  "VelvetSweatshop" is the built-in default encryption
                    password used by MS Excel for the "workbook protection"
                    feature with password. Try this first before prompting the
                    user for a password. */
                ::std::vector< OUString > aDefaultPasswords;
                aDefaultPasswords.push_back( CREATE_OUSTRING( "VelvetSweatshop" ) );

                /*  Use the comphelper password helper to request a password.
                    This helper returns either with the correct password
                    (according to the verifier), or with an empty string if
                    user has cancelled the password input dialog. */
                PasswordVerifier aVerifier( aEncryptInfo );
                OUString aPassword = ::comphelper::DocPasswordHelper::requestAndVerifyDocPassword(
                    aVerifier, rMediaDesc, ::comphelper::DocPasswordRequestType_MS, &aDefaultPasswords );

                if( aPassword.getLength() == 0 )
                {
                    rMediaDesc[ MediaDescriptor::PROP_ABORTED() ] <<= true;
                }
                else
                {
                    // create temporary file for unencrypted package
                    Reference< XStream > xTempFile( mxFactory->createInstance( CREATE_OUSTRING( "com.sun.star.io.TempFile" ) ), UNO_QUERY_THROW );
                    Reference< XOutputStream > xDecryptedPackage( xTempFile->getOutputStream(), UNO_SET_THROW );
                    BinaryXOutputStream aDecryptedPackage( xDecryptedPackage, true );
                    BinaryXInputStream aEncryptedPackage( xEncryptedPackage, true );

                    EVP_CIPHER_CTX aes_ctx;
                    EVP_CIPHER_CTX_init( &aes_ctx );
                    EVP_DecryptInit_ex( &aes_ctx, EVP_aes_128_ecb(), 0, aVerifier.getKey(), 0 );
                    EVP_CIPHER_CTX_set_padding( &aes_ctx, 0 );

                    sal_uInt8 pnInBuffer[ 1024 ];
                    sal_uInt8 pnOutBuffer[ 1024 ];
                    sal_Int32 nInLen;
                    int nOutLen;
                    aEncryptedPackage.skip( 8 ); // decrypted size
                    while( (nInLen = aEncryptedPackage.readMemory( pnInBuffer, sizeof( pnInBuffer ) )) > 0 )
                    {
                        EVP_DecryptUpdate( &aes_ctx, pnOutBuffer, &nOutLen, pnInBuffer, nInLen );
                        aDecryptedPackage.writeMemory( pnOutBuffer, nOutLen );
                    }
                    EVP_DecryptFinal_ex( &aes_ctx, pnOutBuffer, &nOutLen );
                    aDecryptedPackage.writeMemory( pnOutBuffer, nOutLen );

                    EVP_CIPHER_CTX_cleanup( &aes_ctx );
                    xDecryptedPackage->flush();
                    aDecryptedPackage.seekToStart();

                    // store temp file in media descriptor to keep it alive
                    rMediaDesc.setComponentDataEntry( CREATE_OUSTRING( "DecryptedPackage" ), Any( xTempFile ) );

                    Reference< XInputStream > xDecrInStrm = xTempFile->getInputStream();
                    if( lclIsZipPackage( mxFactory, xDecrInStrm ) )
                        return xDecrInStrm;
                }
            }
        }
        catch( Exception& )
        {
        }
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
    if( !bAborted && mxFactory.is() ) try
    {
        aMediaDesc.addInputStream();

        /*  Get the unencrypted input stream. This may include creation of a
            temporary file that contains the decrypted package. This temporary
            file will be stored in the 'ComponentData' property of the media
            descriptor. */
        Reference< XInputStream > xInStrm( extractUnencryptedPackage( aMediaDesc ), UNO_SET_THROW );

        // try to detect the file type, must be a ZIP package
        ZipStorage aZipStorage( mxFactory, xInStrm );
        if( aZipStorage.isStorage() )
        {
            Reference< XFastParser > xParser( mxFactory->createInstance(
                CREATE_OUSTRING( "com.sun.star.xml.sax.FastParser" ) ), UNO_QUERY_THROW );

            xParser->setFastDocumentHandler( new FilterDetectDocHandler( aFilterName ) );
            xParser->setTokenHandler( new FastTokenHandler );

            xParser->registerNamespace( CREATE_OUSTRING( "http://schemas.openxmlformats.org/package/2006/relationships" ), NMSP_PACKAGE_RELATIONSHIPS );
            xParser->registerNamespace( CREATE_OUSTRING( "http://schemas.openxmlformats.org/officeDocument/2006/relationships" ), NMSP_RELATIONSHIPS );
            xParser->registerNamespace( CREATE_OUSTRING( "http://schemas.openxmlformats.org/package/2006/content-types" ), NMSP_CONTENT_TYPES );

            // Parse _rels/.rels to get the target path.
            InputSource aParserInput;
            aParserInput.sSystemId = CREATE_OUSTRING( "_rels/.rels" );
            aParserInput.aInputStream = aZipStorage.openInputStream( aParserInput.sSystemId );
            xParser->parseStream( aParserInput );

            // Parse [Content_Types].xml to determine the content type of the part at the target path.
            aParserInput.sSystemId = CREATE_OUSTRING( "[Content_Types].xml" );
            aParserInput.aInputStream = aZipStorage.openInputStream( aParserInput.sSystemId );
            xParser->parseStream( aParserInput );
        }
    }
    catch( Exception& )
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
