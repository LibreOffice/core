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
#include <PackageConstants.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/xml/crypto/DigestID.hpp>
#include <com/sun/star/xml/crypto/CipherID.hpp>
#include <com/sun/star/xml/crypto/KDFID.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <comphelper/base64.hxx>
#include <comphelper/sequence.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star;

constexpr OUStringLiteral gsFullPathProperty             ( u"FullPath" );
constexpr OUStringLiteral gsMediaTypeProperty            ( u"MediaType" );
constexpr OUStringLiteral gsVersionProperty              ( u"Version" );
constexpr OUStringLiteral gsIterationCountProperty       ( u"IterationCount" );
constexpr OUStringLiteral gsDerivedKeySizeProperty       ( u"DerivedKeySize" );
constexpr OUStringLiteral gsSaltProperty                 ( u"Salt" );
constexpr OUStringLiteral gsInitialisationVectorProperty ( u"InitialisationVector" );
constexpr OUStringLiteral gsSizeProperty                 ( u"Size" );
constexpr OUStringLiteral gsDigestProperty               ( u"Digest" );
constexpr OUString gsEncryptionAlgProperty        ( u"EncryptionAlgorithm"_ustr );
constexpr OUString gsStartKeyAlgProperty          ( u"StartKeyAlgorithm"_ustr );
constexpr OUString gsDigestAlgProperty            ( u"DigestAlgorithm"_ustr );

ManifestImport::ManifestImport( std::vector < Sequence < PropertyValue > > & rNewManVector )
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
    aSequence[PKG_MNFST_FULLPATH].Value <<= rConvertedAttribs[ATTRIBUTE_FULL_PATH];
    aSequence[PKG_MNFST_MEDIATYPE].Name = gsMediaTypeProperty;
    aSequence[PKG_MNFST_MEDIATYPE].Value <<= rConvertedAttribs[ATTRIBUTE_MEDIA_TYPE];

    OUString sVersion = rConvertedAttribs[ATTRIBUTE_VERSION];
    if ( sVersion.getLength() ) {
        aSequence[PKG_MNFST_VERSION].Name = gsVersionProperty;
        aSequence[PKG_MNFST_VERSION].Value <<= sVersion;
    }

    OUString sSize = rConvertedAttribs[ATTRIBUTE_SIZE];
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
        ::comphelper::Base64::decode(aDecodeBuffer, aCurrentCharacters);
        aKeyInfoSequence[2].Value <<= aDecodeBuffer;
        aCurrentCharacters.setLength(0); // consumed
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
        ::comphelper::Base64::decode(aDecodeBuffer, aCurrentCharacters);
        aKeyInfoSequence[0].Value <<= aDecodeBuffer;
        aCurrentCharacters.setLength(0); // consumed
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
        ::comphelper::Base64::decode(aDecodeBuffer, aCurrentCharacters);
        aKeyInfoSequence[1].Value <<= aDecodeBuffer;
        aCurrentCharacters.setLength(0); // consumed
    }
    else
        bIgnoreEncryptData = true;
}

void ManifestImport::doEncryptionData(StringHashMap &rConvertedAttribs)
{
    // If this element exists, then this stream is encrypted and we need
    // to import the initialisation vector, salt and iteration count used
    nDerivedKeySize = 0;
    OUString aString = rConvertedAttribs[ATTRIBUTE_CHECKSUM_TYPE];
    if ( bIgnoreEncryptData )
        return;

    if ( aString == SHA1_1K_NAME || aString == SHA1_1K_URL ) {
        aSequence[PKG_MNFST_DIGESTALG].Name = gsDigestAlgProperty;
        aSequence[PKG_MNFST_DIGESTALG].Value <<= xml::crypto::DigestID::SHA1_1K;
    } else if ( aString == SHA256_1K_URL ) {
        aSequence[PKG_MNFST_DIGESTALG].Name = gsDigestAlgProperty;
        aSequence[PKG_MNFST_DIGESTALG].Value <<= xml::crypto::DigestID::SHA256_1K;
    }
    // note: digest is now *optional* - expected not to be used with AEAD

    if (aSequence[PKG_MNFST_DIGESTALG].Value.hasValue()) {
        aString = rConvertedAttribs[ATTRIBUTE_CHECKSUM];
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

    OUString aString = rConvertedAttribs[ATTRIBUTE_ALGORITHM_NAME];
    if ( aString == BLOWFISH_NAME || aString == BLOWFISH_URL ) {
        aSequence[PKG_MNFST_ENCALG].Name = gsEncryptionAlgProperty;
        aSequence[PKG_MNFST_ENCALG].Value <<= xml::crypto::CipherID::BLOWFISH_CFB_8;
    } else if (aString == AESGCM256_URL) {
        aSequence[PKG_MNFST_ENCALG].Name = gsEncryptionAlgProperty;
        aSequence[PKG_MNFST_ENCALG].Value <<= xml::crypto::CipherID::AES_GCM_W3C;
        SAL_INFO_IF(nDerivedKeySize != 0 && nDerivedKeySize != 32, "package.manifest", "Unexpected derived key length!");
        nDerivedKeySize = 32;
    } else if (aString == AESGCM192_URL) {
        aSequence[PKG_MNFST_ENCALG].Name = gsEncryptionAlgProperty;
        aSequence[PKG_MNFST_ENCALG].Value <<= xml::crypto::CipherID::AES_GCM_W3C;
        SAL_INFO_IF(nDerivedKeySize != 0 && nDerivedKeySize != 24, "package.manifest", "Unexpected derived key length!");
        nDerivedKeySize = 24;
    } else if (aString == AESGCM128_URL) {
        aSequence[PKG_MNFST_ENCALG].Name = gsEncryptionAlgProperty;
        aSequence[PKG_MNFST_ENCALG].Value <<= xml::crypto::CipherID::AES_GCM_W3C;
        SAL_INFO_IF(nDerivedKeySize != 0 && nDerivedKeySize != 16, "package.manifest", "Unexpected derived key length!");
        nDerivedKeySize = 16;
    } else if ( aString == AES256_URL ) {
        aSequence[PKG_MNFST_ENCALG].Name = gsEncryptionAlgProperty;
        aSequence[PKG_MNFST_ENCALG].Value <<= xml::crypto::CipherID::AES_CBC_W3C_PADDING;
        OSL_ENSURE( !nDerivedKeySize || nDerivedKeySize == 32, "Unexpected derived key length!" );
        nDerivedKeySize = 32;
    } else if ( aString == AES192_URL ) {
        aSequence[PKG_MNFST_ENCALG].Name = gsEncryptionAlgProperty;
        aSequence[PKG_MNFST_ENCALG].Value <<= xml::crypto::CipherID::AES_CBC_W3C_PADDING;
        OSL_ENSURE( !nDerivedKeySize || nDerivedKeySize == 24, "Unexpected derived key length!" );
        nDerivedKeySize = 24;
    } else if ( aString == AES128_URL ) {
        aSequence[PKG_MNFST_ENCALG].Name = gsEncryptionAlgProperty;
        aSequence[PKG_MNFST_ENCALG].Value <<= xml::crypto::CipherID::AES_CBC_W3C_PADDING;
        OSL_ENSURE( !nDerivedKeySize || nDerivedKeySize == 16, "Unexpected derived key length!" );
        nDerivedKeySize = 16;
    } else
        bIgnoreEncryptData = true;

    if ( !bIgnoreEncryptData ) {
        aString = rConvertedAttribs[ATTRIBUTE_INITIALISATION_VECTOR];
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

    OUString aString = rConvertedAttribs[ATTRIBUTE_KEY_DERIVATION_NAME];
    if (aString == PBKDF2_NAME || aString == PBKDF2_URL
        || aString == ARGON2ID_URL || aString == ARGON2ID_URL_LO)
    {
        aSequence[PKG_MNFST_KDF].Name = "KeyDerivationFunction";
        if (aString == ARGON2ID_URL || aString == ARGON2ID_URL_LO)
        {
            aSequence[PKG_MNFST_KDF].Value <<= xml::crypto::KDFID::Argon2id;

            aString = rConvertedAttribs.find(ATTRIBUTE_ARGON2_T) != rConvertedAttribs.end()
                ? rConvertedAttribs[ATTRIBUTE_ARGON2_T]
                : rConvertedAttribs[ATTRIBUTE_ARGON2_T_LO];
            sal_Int32 const t(aString.toInt32());
            aString = rConvertedAttribs.find(ATTRIBUTE_ARGON2_M) != rConvertedAttribs.end()
                ? rConvertedAttribs[ATTRIBUTE_ARGON2_M]
                : rConvertedAttribs[ATTRIBUTE_ARGON2_M_LO];
            sal_Int32 const m(aString.toInt32());
            aString = rConvertedAttribs.find(ATTRIBUTE_ARGON2_P) != rConvertedAttribs.end()
                ? rConvertedAttribs[ATTRIBUTE_ARGON2_P]
                : rConvertedAttribs[ATTRIBUTE_ARGON2_P_LO];
            sal_Int32 const p(aString.toInt32());
            if (0 < t && 0 < m && 0 < p)
            {
                aSequence[PKG_MNFST_ARGON2ARGS].Name = "Argon2Args";
                aSequence[PKG_MNFST_ARGON2ARGS].Value <<= uno::Sequence{t,m,p};
            }
            else
            {
                SAL_INFO("package.manifest", "invalid argon2 arguments");
                bIgnoreEncryptData = true;
            }
        }
        else
        {
            aSequence[PKG_MNFST_KDF].Value <<= xml::crypto::KDFID::PBKDF2;

            aString = rConvertedAttribs[ATTRIBUTE_ITERATION_COUNT];
            aSequence[PKG_MNFST_ITERATION].Name = gsIterationCountProperty;
            aSequence[PKG_MNFST_ITERATION].Value <<= aString.toInt32();
        }

        aString = rConvertedAttribs[ATTRIBUTE_SALT];
        uno::Sequence < sal_Int8 > aDecodeBuffer;
        ::comphelper::Base64::decode(aDecodeBuffer, aString);
        aSequence[PKG_MNFST_SALT].Name = gsSaltProperty;
        aSequence[PKG_MNFST_SALT].Value <<= aDecodeBuffer;

        aString = rConvertedAttribs[ATTRIBUTE_KEY_SIZE];
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
        if (aString == "PGP") {
            aSequence[PKG_MNFST_KDF].Name = "KeyDerivationFunction";
            aSequence[PKG_MNFST_KDF].Value <<= xml::crypto::KDFID::PGP_RSA_OAEP_MGF1P;
        } else {
            bIgnoreEncryptData = true;
        }
    } else
        bIgnoreEncryptData = true;
}

void ManifestImport::doStartKeyAlg(StringHashMap &rConvertedAttribs)
{
    OUString aString = rConvertedAttribs[ATTRIBUTE_START_KEY_GENERATION_NAME];
    if (aString == SHA256_URL || aString == SHA256_URL_ODF12) {
        aSequence[PKG_MNFST_STARTALG].Name = gsStartKeyAlgProperty;
        aSequence[PKG_MNFST_STARTALG].Value <<= xml::crypto::DigestID::SHA256;
    } else if ( aString == SHA1_NAME || aString == SHA1_URL ) {
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
        m_PackageVersion = aConvertedAttribs[ATTRIBUTE_VERSION];
        if (aConvertedName != ELEMENT_MANIFEST) //manifest:manifest
            aStack.back().m_bValid = false;
        break;
    }
    case 2: {
        if (aConvertedName == ELEMENT_FILE_ENTRY) //manifest:file-entry
            doFileEntry(aConvertedAttribs);
        else if (aConvertedName == ELEMENT_MANIFEST_KEYINFO) //loext:keyinfo
            ;
        else if (aConvertedName == ELEMENT_ENCRYPTEDKEY13)   //manifest:encrypted-key
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
        else if (aConvertedName == ELEMENT_ENCRYPTION_DATA)   //manifest:encryption-data
            doEncryptionData(aConvertedAttribs);
        else if (aConvertedName == ELEMENT_ENCRYPTEDKEY)   //loext:encrypted-key
            doEncryptedKey(aConvertedAttribs);
        else if (aConvertedName == ELEMENT_ENCRYPTIONMETHOD13)   //manifest:encryption-method
            doEncryptionMethod(aConvertedAttribs, ATTRIBUTE_ALGORITHM13);
        else if (aConvertedName == ELEMENT_MANIFEST13_KEYINFO) //manifest:keyinfo
            ;
        else if (aConvertedName == ELEMENT_CIPHERDATA13)            //manifest:CipherData
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
        else if (aConvertedName == ELEMENT_ALGORITHM)   //manifest:algorithm,
            doAlgorithm(aConvertedAttribs);
        else if (aConvertedName == ELEMENT_KEY_DERIVATION) //manifest:key-derivation,
            doKeyDerivation(aConvertedAttribs);
        else if (aConvertedName == ELEMENT_START_KEY_GENERATION)   //manifest:start-key-generation
            doStartKeyAlg(aConvertedAttribs);
        else if (aConvertedName == ELEMENT_ENCRYPTIONMETHOD)   //loext:encryption-method
            doEncryptionMethod(aConvertedAttribs, ATTRIBUTE_ALGORITHM);
        else if (aConvertedName == ELEMENT_ENCRYPTED_KEYINFO)            //loext:KeyInfo
            ;
        else if (aConvertedName == ELEMENT_CIPHERDATA)            //loext:CipherData
            ;
        else if (aConvertedName == ELEMENT_PGPDATA13)   //manifest:PGPData
            ;
        else if (aConvertedName == ELEMENT_CIPHERVALUE13) //manifest:CipherValue
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
        else if (aConvertedName == ELEMENT_PGPDATA)   //loext:PGPData
            ;
        else if (aConvertedName == ELEMENT_CIPHERVALUE) //loext:CipherValue
            // ciphervalue action happens on endElement
            aCurrentCharacters = "";
        else if (aConvertedName == ELEMENT_PGPKEYID13)   //manifest:PGPKeyID
            // ciphervalue action happens on endElement
            aCurrentCharacters = "";
        else if (aConvertedName == ELEMENT_PGPKEYPACKET13) //manifest:PGPKeyPacket
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
        else if (aConvertedName == ELEMENT_PGPKEYID)   //loext:PGPKeyID
            // ciphervalue action happens on endElement
            aCurrentCharacters = "";
        else if (aConvertedName == ELEMENT_PGPKEYPACKET) //loext:PGPKeyPacket
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

    if ( aConvertedName == ELEMENT_FILE_ENTRY && aStack.back().m_bValid ) {
        // required for wholesome encryption: if there is no document and hence
        // no file-entry with a version attribute, send the package's version
        // with the first file-entry.
        // (note: the only case when a valid ODF document has no "/" entry with
        // a version is when it is ODF 1.0/1.1 and then it doesn't have the
        // package version either)
        if (rManVector.empty() && !m_PackageVersion.isEmpty()
            && !aSequence[PKG_MNFST_VERSION].Value.hasValue())
        {
            aSequence[PKG_MNFST_VERSION].Name = u"Version"_ustr;
            aSequence[PKG_MNFST_VERSION].Value <<= m_PackageVersion;
        }
        // the first entry gets KeyInfo element if any, for PGP encryption
        if (!bIgnoreEncryptData && !aKeys.empty() && rManVector.empty())
        {
            aSequence[PKG_MNFST_KEYINFO].Name = "KeyInfo";
            aSequence[PKG_MNFST_KEYINFO].Value <<= comphelper::containerToSequence(aKeys);
        }
        std::erase_if(aSequence, isEmpty);

        bIgnoreEncryptData = false;
        rManVector.push_back ( comphelper::containerToSequence(aSequence) );

        aSequence.clear();
    }
    else if ( (aConvertedName == ELEMENT_ENCRYPTEDKEY
               || aConvertedName == ELEMENT_ENCRYPTEDKEY13)
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
            if (aConvertedName == ELEMENT_CIPHERVALUE13) //manifest:CipherValue
                doEncryptedCipherValue();
            else
                aStack.back().m_bValid = false;
            break;
        }
        case 5: {
            if (aConvertedName == ELEMENT_CIPHERVALUE) //loext:CipherValue
                doEncryptedCipherValue();
            else if (aConvertedName == ELEMENT_PGPKEYID13)   //manifest:PGPKeyID
                doEncryptedKeyId();
            else if (aConvertedName == ELEMENT_PGPKEYPACKET13) //manifest:PGPKeyPacket
                doEncryptedKeyPacket();
            else
                aStack.back().m_bValid = false;
            break;
        }
        case 6: {
            if (aConvertedName == ELEMENT_PGPKEYID)   //loext:PGPKeyID
                doEncryptedKeyId();
            else if (aConvertedName == ELEMENT_PGPKEYPACKET) //loext:PGPKeyPacket
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

    aStack.emplace_back( aConvertedName, std::move(aNamespaces) );

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
