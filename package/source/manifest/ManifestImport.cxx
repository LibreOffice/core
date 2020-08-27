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

#include "ManifestImport.hxx"
#include "ManifestDefines.hxx"
#include <sax/tools/converter.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/xml/crypto/DigestID.hpp>
#include <com/sun/star/xml/crypto/CipherID.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <comphelper/base64.hxx>
#include <comphelper/sequence.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star;
using namespace std;


const OUStringLiteral gsFileEntryElement     ( u"" ELEMENT_FILE_ENTRY );
const OUStringLiteral gsEncryptionDataElement( u"" ELEMENT_ENCRYPTION_DATA );
const OUStringLiteral gsAlgorithmElement ( u"" ELEMENT_ALGORITHM );
const OUStringLiteral gsStartKeyAlgElement   ( u"" ELEMENT_START_KEY_GENERATION );
const OUStringLiteral gsKeyDerivationElement( u"" ELEMENT_KEY_DERIVATION );

const OUStringLiteral gsMediaTypeAttribute           ( u"" ATTRIBUTE_MEDIA_TYPE );
const OUStringLiteral gsVersionAttribute             ( u"" ATTRIBUTE_VERSION );
const OUStringLiteral gsFullPathAttribute            ( u"" ATTRIBUTE_FULL_PATH );
const OUStringLiteral gsSizeAttribute                ( u"" ATTRIBUTE_SIZE );
const OUStringLiteral gsSaltAttribute                ( u"" ATTRIBUTE_SALT );
const OUStringLiteral gsInitialisationVectorAttribute ( u"" ATTRIBUTE_INITIALISATION_VECTOR );
const OUStringLiteral gsIterationCountAttribute      ( u"" ATTRIBUTE_ITERATION_COUNT );
const OUStringLiteral gsKeySizeAttribute             ( u"" ATTRIBUTE_KEY_SIZE );
const OUStringLiteral gsAlgorithmNameAttribute       ( u"" ATTRIBUTE_ALGORITHM_NAME );
const OUStringLiteral gsStartKeyAlgNameAttribute     ( u"" ATTRIBUTE_START_KEY_GENERATION_NAME );
const OUStringLiteral gsKeyDerivationNameAttribute   ( u"" ATTRIBUTE_KEY_DERIVATION_NAME );
const OUStringLiteral gsChecksumAttribute            ( u"" ATTRIBUTE_CHECKSUM );
const OUStringLiteral gsChecksumTypeAttribute        ( u"" ATTRIBUTE_CHECKSUM_TYPE );

const OUStringLiteral gsKeyInfoElement               ( u"" ELEMENT_ENCRYPTED_KEYINFO );
const OUStringLiteral gsManifestKeyInfoElement       ( u"" ELEMENT_MANIFEST_KEYINFO );
const OUStringLiteral gsEncryptedKeyElement          ( u"" ELEMENT_ENCRYPTEDKEY );
const OUStringLiteral gsEncryptionMethodElement      ( u"" ELEMENT_ENCRYPTIONMETHOD );
const OUStringLiteral gsPgpDataElement               ( u"" ELEMENT_PGPDATA );
const OUStringLiteral gsPgpKeyIDElement              ( u"" ELEMENT_PGPKEYID );
const OUStringLiteral gsPGPKeyPacketElement          ( u"" ELEMENT_PGPKEYPACKET );
const OUStringLiteral gsAlgorithmAttribute           ( u"" ATTRIBUTE_ALGORITHM );
const OUStringLiteral gsCipherDataElement            ( u"" ELEMENT_CIPHERDATA );
const OUStringLiteral gsCipherValueElement           ( u"" ELEMENT_CIPHERVALUE );

const OUStringLiteral gsManifestKeyInfoElement13       ( u"" ELEMENT_MANIFEST13_KEYINFO );
const OUStringLiteral gsEncryptedKeyElement13          ( u"" ELEMENT_ENCRYPTEDKEY13 );
const OUStringLiteral gsEncryptionMethodElement13      ( u"" ELEMENT_ENCRYPTIONMETHOD13 );
const OUStringLiteral gsPgpDataElement13               ( u"" ELEMENT_PGPDATA13 );
const OUStringLiteral gsPgpKeyIDElement13              ( u"" ELEMENT_PGPKEYID13 );
const OUStringLiteral gsPGPKeyPacketElement13          ( u"" ELEMENT_PGPKEYPACKET13 );
const OUStringLiteral gsAlgorithmAttribute13           ( u"" ATTRIBUTE_ALGORITHM13 );
const OUStringLiteral gsCipherDataElement13            ( u"" ELEMENT_CIPHERDATA13 );
const OUStringLiteral gsCipherValueElement13           ( u"" ELEMENT_CIPHERVALUE13 );

const OUStringLiteral gsFullPathProperty             ( u"FullPath" );
const OUStringLiteral gsMediaTypeProperty            ( u"MediaType" );
const OUStringLiteral gsVersionProperty              ( u"Version" );
const OUStringLiteral gsIterationCountProperty       ( u"IterationCount" );
const OUStringLiteral gsDerivedKeySizeProperty       ( u"DerivedKeySize" );
const OUStringLiteral gsSaltProperty                 ( u"Salt" );
const OUStringLiteral gsInitialisationVectorProperty ( u"InitialisationVector" );
const OUStringLiteral gsSizeProperty                 ( u"Size" );
const OUStringLiteral gsDigestProperty               ( u"Digest" );
const OUStringLiteral gsEncryptionAlgProperty        ( u"EncryptionAlgorithm" );
const OUStringLiteral gsStartKeyAlgProperty          ( u"StartKeyAlgorithm" );
const OUStringLiteral gsDigestAlgProperty            ( u"DigestAlgorithm" );

const OUStringLiteral gsSHA256_URL_ODF12             ( u"" SHA256_URL_ODF12 );
const OUStringLiteral gsSHA256_URL                   ( u"" SHA256_URL );
const OUStringLiteral gsSHA1_Name                    ( u"" SHA1_NAME );
const OUStringLiteral gsSHA1_URL                     ( u"" SHA1_URL );

const OUStringLiteral gsSHA256_1k_URL                ( u"" SHA256_1K_URL );
const OUStringLiteral gsSHA1_1k_Name                 ( u"" SHA1_1K_NAME );
const OUStringLiteral gsSHA1_1k_URL                  ( u"" SHA1_1K_URL );

const OUStringLiteral gsBlowfish_Name                ( u"" BLOWFISH_NAME );
const OUStringLiteral gsBlowfish_URL                 ( u"" BLOWFISH_URL );
const OUStringLiteral gsAES128_URL                   ( u"" AES128_URL );
const OUStringLiteral gsAES192_URL                   ( u"" AES192_URL );
const OUStringLiteral gsAES256_URL                   ( u"" AES256_URL );

const OUStringLiteral gsPBKDF2_Name                  ( u"" PBKDF2_NAME );
const OUStringLiteral gsPBKDF2_URL                   ( u"" PBKDF2_URL );

ManifestImport::ManifestImport( vector < Sequence < PropertyValue > > & rNewManVector )
    : bIgnoreEncryptData    ( false )
    , bPgpEncryption ( false )
    , nDerivedKeySize( 0 )
    , rManVector ( rNewManVector )
{
    aStack.reserve( 10 );
}

ManifestImport::~ManifestImport()
{
}

void SAL_CALL ManifestImport::startDocument(  )
{
}

void SAL_CALL ManifestImport::endDocument(  )
{
}

void ManifestImport::doFileEntry(StringHashMap &rConvertedAttribs)
{
    aSequence.resize(PKG_SIZE_ENCR_MNFST);

    aSequence[PKG_MNFST_FULLPATH].Name = gsFullPathProperty;
    aSequence[PKG_MNFST_FULLPATH].Value <<= rConvertedAttribs[gsFullPathAttribute];
    aSequence[PKG_MNFST_MEDIATYPE].Name = gsMediaTypeProperty;
    aSequence[PKG_MNFST_MEDIATYPE].Value <<= rConvertedAttribs[gsMediaTypeAttribute];

    OUString sVersion = rConvertedAttribs[gsVersionAttribute];
    if ( sVersion.getLength() ) {
        aSequence[PKG_MNFST_VERSION].Name = gsVersionProperty;
        aSequence[PKG_MNFST_VERSION].Value <<= sVersion;
    }

    OUString sSize = rConvertedAttribs[gsSizeAttribute];
    if ( sSize.getLength() ) {
        sal_Int64 nSize = sSize.toInt64();
        aSequence[PKG_MNFST_UCOMPSIZE].Name = gsSizeProperty;
        aSequence[PKG_MNFST_UCOMPSIZE].Value <<= nSize;
    }
}

void ManifestImport::doEncryptedKey(StringHashMap &)
{
    aKeyInfoSequence.clear();
    aKeyInfoSequence.resize(3);
}

void ManifestImport::doEncryptionMethod(StringHashMap &rConvertedAttribs,
                                        const OUString& rAlgoAttrName)
{
    OUString aString = rConvertedAttribs[rAlgoAttrName];
    if ( aKeyInfoSequence.size() != 3
         || aString != "http://www.w3.org/2001/04/xmlenc#rsa-oaep-mgf1p" )
    {
        bIgnoreEncryptData = true;
    }
}

void ManifestImport::doEncryptedCipherValue()
{
    if ( aKeyInfoSequence.size() == 3 )
    {
        aKeyInfoSequence[2].Name = "CipherValue";
        uno::Sequence < sal_Int8 > aDecodeBuffer;
        ::comphelper::Base64::decode(aDecodeBuffer, aCurrentCharacters.toString());
        aKeyInfoSequence[2].Value <<= aDecodeBuffer;
        aCurrentCharacters = ""; // consumed
    }
    else
        bIgnoreEncryptData = true;
}

void ManifestImport::doEncryptedKeyId()
{
    if ( aKeyInfoSequence.size() == 3 )
    {
        aKeyInfoSequence[0].Name = "KeyId";
        uno::Sequence < sal_Int8 > aDecodeBuffer;
        ::comphelper::Base64::decode(aDecodeBuffer, aCurrentCharacters.toString());
        aKeyInfoSequence[0].Value <<= aDecodeBuffer;
        aCurrentCharacters = ""; // consumed
    }
    else
        bIgnoreEncryptData = true;
}

void ManifestImport::doEncryptedKeyPacket()
{
    if ( aKeyInfoSequence.size() == 3 )
    {
        aKeyInfoSequence[1].Name = "KeyPacket";
        uno::Sequence < sal_Int8 > aDecodeBuffer;
        ::comphelper::Base64::decode(aDecodeBuffer, aCurrentCharacters.toString());
        aKeyInfoSequence[1].Value <<= aDecodeBuffer;
        aCurrentCharacters = ""; // consumed
    }
    else
        bIgnoreEncryptData = true;
}

void ManifestImport::doEncryptionData(StringHashMap &rConvertedAttribs)
{
    // If this element exists, then this stream is encrypted and we need
    // to import the initialisation vector, salt and iteration count used
    nDerivedKeySize = 0;
    OUString aString = rConvertedAttribs[gsChecksumTypeAttribute];
    if ( bIgnoreEncryptData )
        return;

    if ( aString == gsSHA1_1k_Name || aString == gsSHA1_1k_URL ) {
        aSequence[PKG_MNFST_DIGESTALG].Name = gsDigestAlgProperty;
        aSequence[PKG_MNFST_DIGESTALG].Value <<= xml::crypto::DigestID::SHA1_1K;
    } else if ( aString == gsSHA256_1k_URL ) {
        aSequence[PKG_MNFST_DIGESTALG].Name = gsDigestAlgProperty;
        aSequence[PKG_MNFST_DIGESTALG].Value <<= xml::crypto::DigestID::SHA256_1K;
    } else
        bIgnoreEncryptData = true;

    if ( !bIgnoreEncryptData ) {
        aString = rConvertedAttribs[gsChecksumAttribute];
        uno::Sequence < sal_Int8 > aDecodeBuffer;
        ::comphelper::Base64::decode(aDecodeBuffer, aString);
        aSequence[PKG_MNFST_DIGEST].Name = gsDigestProperty;
        aSequence[PKG_MNFST_DIGEST].Value <<= aDecodeBuffer;
    }
}

void ManifestImport::doAlgorithm(StringHashMap &rConvertedAttribs)
{
    if ( bIgnoreEncryptData )
        return;

    OUString aString = rConvertedAttribs[gsAlgorithmNameAttribute];
    if ( aString == gsBlowfish_Name || aString == gsBlowfish_URL ) {
        aSequence[PKG_MNFST_ENCALG].Name = gsEncryptionAlgProperty;
        aSequence[PKG_MNFST_ENCALG].Value <<= xml::crypto::CipherID::BLOWFISH_CFB_8;
    } else if ( aString == gsAES256_URL ) {
        aSequence[PKG_MNFST_ENCALG].Name = gsEncryptionAlgProperty;
        aSequence[PKG_MNFST_ENCALG].Value <<= xml::crypto::CipherID::AES_CBC_W3C_PADDING;
        OSL_ENSURE( !nDerivedKeySize || nDerivedKeySize == 32, "Unexpected derived key length!" );
        nDerivedKeySize = 32;
    } else if ( aString == gsAES192_URL ) {
        aSequence[PKG_MNFST_ENCALG].Name = gsEncryptionAlgProperty;
        aSequence[PKG_MNFST_ENCALG].Value <<= xml::crypto::CipherID::AES_CBC_W3C_PADDING;
        OSL_ENSURE( !nDerivedKeySize || nDerivedKeySize == 24, "Unexpected derived key length!" );
        nDerivedKeySize = 24;
    } else if ( aString == gsAES128_URL ) {
        aSequence[PKG_MNFST_ENCALG].Name = gsEncryptionAlgProperty;
        aSequence[PKG_MNFST_ENCALG].Value <<= xml::crypto::CipherID::AES_CBC_W3C_PADDING;
        OSL_ENSURE( !nDerivedKeySize || nDerivedKeySize == 16, "Unexpected derived key length!" );
        nDerivedKeySize = 16;
    } else
        bIgnoreEncryptData = true;

    if ( !bIgnoreEncryptData ) {
        aString = rConvertedAttribs[gsInitialisationVectorAttribute];
        uno::Sequence < sal_Int8 > aDecodeBuffer;
        ::comphelper::Base64::decode(aDecodeBuffer, aString);
        aSequence[PKG_MNFST_INIVECTOR].Name = gsInitialisationVectorProperty;
        aSequence[PKG_MNFST_INIVECTOR].Value <<= aDecodeBuffer;
    }
}

void ManifestImport::doKeyDerivation(StringHashMap &rConvertedAttribs)
{
    if ( bIgnoreEncryptData )
        return;

    OUString aString = rConvertedAttribs[gsKeyDerivationNameAttribute];
    if ( aString == gsPBKDF2_Name || aString == gsPBKDF2_URL ) {
        aString = rConvertedAttribs[gsSaltAttribute];
        uno::Sequence < sal_Int8 > aDecodeBuffer;
        ::comphelper::Base64::decode(aDecodeBuffer, aString);
        aSequence[PKG_MNFST_SALT].Name = gsSaltProperty;
        aSequence[PKG_MNFST_SALT].Value <<= aDecodeBuffer;

        aString = rConvertedAttribs[gsIterationCountAttribute];
        aSequence[PKG_MNFST_ITERATION].Name = gsIterationCountProperty;
        aSequence[PKG_MNFST_ITERATION].Value <<= aString.toInt32();

        aString = rConvertedAttribs[gsKeySizeAttribute];
        if ( aString.getLength() ) {
            sal_Int32 nKey = aString.toInt32();
            OSL_ENSURE( !nDerivedKeySize || nKey == nDerivedKeySize , "Provided derived key length differs from the expected one!" );
            nDerivedKeySize = nKey;
        } else if ( !nDerivedKeySize )
            nDerivedKeySize = 16;
        else if ( nDerivedKeySize != 16 )
            OSL_ENSURE( false, "Default derived key length differs from the expected one!" );

        aSequence[PKG_MNFST_DERKEYSIZE].Name = gsDerivedKeySizeProperty;
        aSequence[PKG_MNFST_DERKEYSIZE].Value <<= nDerivedKeySize;
    } else if ( bPgpEncryption ) {
        if ( aString != "PGP" )
            bIgnoreEncryptData = true;
    } else
        bIgnoreEncryptData = true;
}

void ManifestImport::doStartKeyAlg(StringHashMap &rConvertedAttribs)
{
    OUString aString = rConvertedAttribs[gsStartKeyAlgNameAttribute];
    if (aString == gsSHA256_URL || aString == gsSHA256_URL_ODF12) {
        aSequence[PKG_MNFST_STARTALG].Name = gsStartKeyAlgProperty;
        aSequence[PKG_MNFST_STARTALG].Value <<= xml::crypto::DigestID::SHA256;
    } else if ( aString == gsSHA1_Name || aString == gsSHA1_URL ) {
        aSequence[PKG_MNFST_STARTALG].Name = gsStartKeyAlgProperty;
        aSequence[PKG_MNFST_STARTALG].Value <<= xml::crypto::DigestID::SHA1;
    } else
        bIgnoreEncryptData = true;
}

void SAL_CALL ManifestImport::startElement( const OUString& aName, const uno::Reference< xml::sax::XAttributeList >& xAttribs )
{
    StringHashMap aConvertedAttribs;
    OUString aConvertedName = PushNameAndNamespaces( aName, xAttribs, aConvertedAttribs );

    size_t nLevel = aStack.size();

    assert(nLevel >= 1);

    switch (nLevel) {
    case 1: {
        if (aConvertedName != ELEMENT_MANIFEST) //manifest:manifest
            aStack.back().m_bValid = false;
        break;
    }
    case 2: {
        if (aConvertedName == gsFileEntryElement) //manifest:file-entry
            doFileEntry(aConvertedAttribs);
        else if (aConvertedName == gsManifestKeyInfoElement) //loext:keyinfo
            ;
        else if (aConvertedName == gsEncryptedKeyElement13)   //manifest:encrypted-key
            doEncryptedKey(aConvertedAttribs);
        else
            aStack.back().m_bValid = false;
        break;
    }
    case 3: {
        ManifestStack::reverse_iterator aIter = aStack.rbegin();
        ++aIter;

        if (!aIter->m_bValid)
            aStack.back().m_bValid = false;
        else if (aConvertedName == gsEncryptionDataElement)   //manifest:encryption-data
            doEncryptionData(aConvertedAttribs);
        else if (aConvertedName == gsEncryptedKeyElement)   //loext:encrypted-key
            doEncryptedKey(aConvertedAttribs);
        else if (aConvertedName == gsEncryptionMethodElement13)   //manifest:encryption-method
            doEncryptionMethod(aConvertedAttribs, gsAlgorithmAttribute13);
        else if (aConvertedName == gsManifestKeyInfoElement13) //manifest:keyinfo
            ;
        else if (aConvertedName == gsCipherDataElement13)            //manifest:CipherData
            ;
        else
            aStack.back().m_bValid = false;
        break;
    }
    case 4: {
        ManifestStack::reverse_iterator aIter = aStack.rbegin();
        ++aIter;

        if (!aIter->m_bValid)
            aStack.back().m_bValid = false;
        else if (aConvertedName == gsAlgorithmElement)   //manifest:algorithm,
            doAlgorithm(aConvertedAttribs);
        else if (aConvertedName == gsKeyDerivationElement) //manifest:key-derivation,
            doKeyDerivation(aConvertedAttribs);
        else if (aConvertedName == gsStartKeyAlgElement)   //manifest:start-key-generation
            doStartKeyAlg(aConvertedAttribs);
        else if (aConvertedName == gsEncryptionMethodElement)   //loext:encryption-method
            doEncryptionMethod(aConvertedAttribs, gsAlgorithmAttribute);
        else if (aConvertedName == gsKeyInfoElement)            //loext:KeyInfo
            ;
        else if (aConvertedName == gsCipherDataElement)            //loext:CipherData
            ;
        else if (aConvertedName == gsPgpDataElement13)   //manifest:PGPData
            ;
        else if (aConvertedName == gsCipherValueElement13) //manifest:CipherValue
            // ciphervalue action happens on endElement
            aCurrentCharacters = "";
        else
            aStack.back().m_bValid = false;
        break;
    }
    case 5: {
        ManifestStack::reverse_iterator aIter = aStack.rbegin();
        ++aIter;

        if (!aIter->m_bValid)
            aStack.back().m_bValid = false;
        else if (aConvertedName == gsPgpDataElement)   //loext:PGPData
            ;
        else if (aConvertedName == gsCipherValueElement) //loext:CipherValue
            // ciphervalue action happens on endElement
            aCurrentCharacters = "";
        else if (aConvertedName == gsPgpKeyIDElement13)   //manifest:PGPKeyID
            // ciphervalue action happens on endElement
            aCurrentCharacters = "";
        else if (aConvertedName == gsPGPKeyPacketElement13) //manifest:PGPKeyPacket
            // ciphervalue action happens on endElement
            aCurrentCharacters = "";
        else
            aStack.back().m_bValid = false;
        break;
    }
    case 6: {
        ManifestStack::reverse_iterator aIter = aStack.rbegin();
        ++aIter;

        if (!aIter->m_bValid)
            aStack.back().m_bValid = false;
        else if (aConvertedName == gsPgpKeyIDElement)   //loext:PGPKeyID
            // ciphervalue action happens on endElement
            aCurrentCharacters = "";
        else if (aConvertedName == gsPGPKeyPacketElement) //loext:PGPKeyPacket
            // ciphervalue action happens on endElement
            aCurrentCharacters = "";
        else
            aStack.back().m_bValid = false;
        break;
    }
    default:
        aStack.back().m_bValid = false;
        break;
    }
}

namespace
{
bool isEmpty(const css::beans::PropertyValue &rProp)
{
    return rProp.Name.isEmpty();
}
}

void SAL_CALL ManifestImport::endElement( const OUString& aName )
{
    size_t nLevel = aStack.size();

    assert(nLevel >= 1);

    OUString aConvertedName = ConvertName( aName );
    if ( aStack.empty() || aStack.rbegin()->m_aConvertedName != aConvertedName )
        return;

    if ( aConvertedName == gsFileEntryElement && aStack.back().m_bValid ) {
        // root folder gets KeyInfo entry if any, for PGP encryption
        if (!bIgnoreEncryptData && !aKeys.empty() && aSequence[PKG_MNFST_FULLPATH].Value.get<OUString>() == "/" )
        {
            aSequence[PKG_SIZE_NOENCR_MNFST].Name = "KeyInfo";
            aSequence[PKG_SIZE_NOENCR_MNFST].Value <<= comphelper::containerToSequence(aKeys);
        }
        aSequence.erase(std::remove_if(aSequence.begin(), aSequence.end(),
                                       isEmpty), aSequence.end());

        bIgnoreEncryptData = false;
        rManVector.push_back ( comphelper::containerToSequence(aSequence) );

        aSequence.clear();
    }
    else if ( (aConvertedName == gsEncryptedKeyElement
               || aConvertedName == gsEncryptedKeyElement13)
              && aStack.back().m_bValid ) {
        if ( !bIgnoreEncryptData )
        {
            aKeys.push_back( comphelper::containerToSequence(aKeyInfoSequence) );
            bPgpEncryption = true;
        }
        aKeyInfoSequence.clear();
    }

    // end element handling for elements with cdata
    switch (nLevel) {
        case 4: {
            if (aConvertedName == gsCipherValueElement13) //manifest:CipherValue
                doEncryptedCipherValue();
            else
                aStack.back().m_bValid = false;
            break;
        }
        case 5: {
            if (aConvertedName == gsCipherValueElement) //loext:CipherValue
                doEncryptedCipherValue();
            else if (aConvertedName == gsPgpKeyIDElement13)   //manifest:PGPKeyID
                doEncryptedKeyId();
            else if (aConvertedName == gsPGPKeyPacketElement13) //manifest:PGPKeyPacket
                doEncryptedKeyPacket();
            else
                aStack.back().m_bValid = false;
            break;
        }
        case 6: {
            if (aConvertedName == gsPgpKeyIDElement)   //loext:PGPKeyID
                doEncryptedKeyId();
            else if (aConvertedName == gsPGPKeyPacketElement) //loext:PGPKeyPacket
                doEncryptedKeyPacket();
            else
                aStack.back().m_bValid = false;
            break;
        }
    }

    aStack.pop_back();
}

void SAL_CALL ManifestImport::characters( const OUString& aChars )
{
    aCurrentCharacters.append(aChars);
}

void SAL_CALL ManifestImport::ignorableWhitespace( const OUString& /*aWhitespaces*/ )
{
}

void SAL_CALL ManifestImport::processingInstruction( const OUString& /*aTarget*/, const OUString& /*aData*/ )
{
}

void SAL_CALL ManifestImport::setDocumentLocator( const uno::Reference< xml::sax::XLocator >& /*xLocator*/ )
{
}

OUString ManifestImport::PushNameAndNamespaces( const OUString& aName, const uno::Reference< xml::sax::XAttributeList >& xAttribs, StringHashMap& o_aConvertedAttribs )
{
    StringHashMap aNamespaces;
    ::std::vector< ::std::pair< OUString, OUString > > aAttribsStrs;

    if ( xAttribs.is() ) {
        sal_Int16 nAttrCount = xAttribs.is() ? xAttribs->getLength() : 0;
        aAttribsStrs.reserve( nAttrCount );

        for( sal_Int16 nInd = 0; nInd < nAttrCount; nInd++ ) {
            OUString aAttrName = xAttribs->getNameByIndex( nInd );
            OUString aAttrValue = xAttribs->getValueByIndex( nInd );
            if ( aAttrName.getLength() >= 5
                 && aAttrName.startsWith("xmlns")
                 && ( aAttrName.getLength() == 5 || aAttrName[5] == ':' ) ) {
                // this is a namespace declaration
                OUString aNsName( ( aAttrName.getLength() == 5 ) ? OUString() : aAttrName.copy( 6 ) );
                aNamespaces[aNsName] = aAttrValue;
            } else {
                // this is no namespace declaration
                aAttribsStrs.emplace_back( aAttrName, aAttrValue );
            }
        }
    }

    OUString aConvertedName = ConvertNameWithNamespace( aName, aNamespaces );
    if ( !aConvertedName.getLength() )
        aConvertedName = ConvertName( aName );

    aStack.emplace_back( aConvertedName, aNamespaces );

    for (const std::pair<OUString,OUString> & rAttribsStr : aAttribsStrs) {
        // convert the attribute names on filling
        o_aConvertedAttribs[ConvertName( rAttribsStr.first )] = rAttribsStr.second;
    }

    return aConvertedName;
}

OUString ManifestImport::ConvertNameWithNamespace( const OUString& aName, const StringHashMap& aNamespaces )
{
    OUString aNsAlias;
    OUString aPureName = aName;

    sal_Int32 nInd = aName.indexOf( ':' );
    if ( nInd != -1 && nInd < aName.getLength() ) {
        aNsAlias = aName.copy( 0, nInd );
        aPureName = aName.copy( nInd + 1 );
    }

    OUString aResult;

    StringHashMap::const_iterator aIter = aNamespaces.find( aNsAlias );
    if ( aIter != aNamespaces.end()
         && ( aIter->second == MANIFEST_NAMESPACE || aIter->second == MANIFEST_OASIS_NAMESPACE ) ) {
        // no check for manifest.xml consistency currently since the old versions have supported inconsistent documents as well
        aResult = MANIFEST_NSPREFIX + aPureName;
    }

    return aResult;
}

OUString ManifestImport::ConvertName( const OUString& aName )
{
    OUString aConvertedName;
    for ( ManifestStack::reverse_iterator aIter = aStack.rbegin(); !aConvertedName.getLength() && aIter != aStack.rend(); ++aIter ) {
        if ( !aIter->m_aNamespaces.empty() )
            aConvertedName = ConvertNameWithNamespace( aName, aIter->m_aNamespaces );
    }

    if ( !aConvertedName.getLength() )
        aConvertedName = aName;

    return aConvertedName;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
