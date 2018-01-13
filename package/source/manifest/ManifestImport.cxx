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
#include <comphelper/sequence.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star;
using namespace std;

ManifestImport::ManifestImport( vector < Sequence < PropertyValue > > & rNewManVector )
    : bIgnoreEncryptData    ( false )
    , bPgpEncryption ( false )
    , nDerivedKeySize( 0 )
    , rManVector ( rNewManVector )

    , sFileEntryElement     ( ELEMENT_FILE_ENTRY )
    , sEncryptionDataElement( ELEMENT_ENCRYPTION_DATA )
    , sAlgorithmElement ( ELEMENT_ALGORITHM )
    , sStartKeyAlgElement   ( ELEMENT_START_KEY_GENERATION )
    , sKeyDerivationElement( ELEMENT_KEY_DERIVATION )

    , sMediaTypeAttribute           ( ATTRIBUTE_MEDIA_TYPE )
    , sVersionAttribute             ( ATTRIBUTE_VERSION )
    , sFullPathAttribute            ( ATTRIBUTE_FULL_PATH )
    , sSizeAttribute                ( ATTRIBUTE_SIZE )
    , sSaltAttribute                ( ATTRIBUTE_SALT )
    , sInitialisationVectorAttribute ( ATTRIBUTE_INITIALISATION_VECTOR )
    , sIterationCountAttribute      ( ATTRIBUTE_ITERATION_COUNT )
    , sKeySizeAttribute             ( ATTRIBUTE_KEY_SIZE )
    , sAlgorithmNameAttribute       ( ATTRIBUTE_ALGORITHM_NAME )
    , sStartKeyAlgNameAttribute     ( ATTRIBUTE_START_KEY_GENERATION_NAME )
    , sKeyDerivationNameAttribute   ( ATTRIBUTE_KEY_DERIVATION_NAME )
    , sChecksumAttribute            ( ATTRIBUTE_CHECKSUM )
    , sChecksumTypeAttribute        ( ATTRIBUTE_CHECKSUM_TYPE )

    , sKeyInfoElement               ( ELEMENT_ENCRYPTED_KEYINFO )
    , sManifestKeyInfoElement       ( ELEMENT_MANIFEST_KEYINFO )
    , sEncryptedKeyElement          ( ELEMENT_ENCRYPTEDKEY )
    , sEncryptionMethodElement      ( ELEMENT_ENCRYPTIONMETHOD )
    , sPgpDataElement               ( ELEMENT_PGPDATA )
    , sPgpKeyIDElement              ( ELEMENT_PGPKEYID )
    , sPGPKeyPacketElement          ( ELEMENT_PGPKEYPACKET )
    , sAlgorithmAttribute           ( ATTRIBUTE_ALGORITHM )
    , sCipherDataElement            ( ELEMENT_CIPHERDATA )
    , sCipherValueElement           ( ELEMENT_CIPHERVALUE )

    , sFullPathProperty             ( "FullPath" )
    , sMediaTypeProperty            ( "MediaType" )
    , sVersionProperty              ( "Version" )
    , sIterationCountProperty       ( "IterationCount" )
    , sDerivedKeySizeProperty       ( "DerivedKeySize" )
    , sSaltProperty                 ( "Salt" )
    , sInitialisationVectorProperty ( "InitialisationVector" )
    , sSizeProperty                 ( "Size" )
    , sDigestProperty               ( "Digest" )
    , sEncryptionAlgProperty        ( "EncryptionAlgorithm" )
    , sStartKeyAlgProperty          ( "StartKeyAlgorithm" )
    , sDigestAlgProperty            ( "DigestAlgorithm" )

    , sSHA256_URL_ODF12             ( SHA256_URL_ODF12 )
    , sSHA256_URL                   ( SHA256_URL )
    , sSHA1_Name                    ( SHA1_NAME )
    , sSHA1_URL                     ( SHA1_URL )

    , sSHA256_1k_URL                ( SHA256_1K_URL )
    , sSHA1_1k_Name                 ( SHA1_1K_NAME )
    , sSHA1_1k_URL                  ( SHA1_1K_URL )

    , sBlowfish_Name                ( BLOWFISH_NAME )
    , sBlowfish_URL                 ( BLOWFISH_URL )
    , sAES128_URL                   ( AES128_URL )
    , sAES192_URL                   ( AES192_URL )
    , sAES256_URL                   ( AES256_URL )

    , sPBKDF2_Name                  ( PBKDF2_NAME )
    , sPBKDF2_URL                   ( PBKDF2_URL )
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

    aSequence[PKG_MNFST_FULLPATH].Name = sFullPathProperty;
    aSequence[PKG_MNFST_FULLPATH].Value <<= rConvertedAttribs[sFullPathAttribute];
    aSequence[PKG_MNFST_MEDIATYPE].Name = sMediaTypeProperty;
    aSequence[PKG_MNFST_MEDIATYPE].Value <<= rConvertedAttribs[sMediaTypeAttribute];

    OUString sVersion = rConvertedAttribs[sVersionAttribute];
    if ( sVersion.getLength() ) {
        aSequence[PKG_MNFST_VERSION].Name = sVersionProperty;
        aSequence[PKG_MNFST_VERSION].Value <<= sVersion;
    }

    OUString sSize = rConvertedAttribs[sSizeAttribute];
    if ( sSize.getLength() ) {
        sal_Int64 nSize = sSize.toInt64();
        aSequence[PKG_MNFST_UCOMPSIZE].Name = sSizeProperty;
        aSequence[PKG_MNFST_UCOMPSIZE].Value <<= nSize;
    }
}

void ManifestImport::doKeyInfoEntry(StringHashMap &)
{
}

void ManifestImport::doEncryptedKey(StringHashMap &)
{
    aKeyInfoSequence.clear();
    aKeyInfoSequence.resize(3);
}

void ManifestImport::doEncryptionMethod(StringHashMap &rConvertedAttribs)
{
    OUString aString = rConvertedAttribs[sAlgorithmAttribute];
    if ( aKeyInfoSequence.size() != 3
         || aString != "http://www.w3.org/2001/04/xmlenc#rsa-oaep-mgf1p" )
    {
        bIgnoreEncryptData = true;
    }
}

void ManifestImport::doEncryptedKeyInfo(StringHashMap &)
{
}

void ManifestImport::doEncryptedCipherData(StringHashMap &)
{
}

void ManifestImport::doEncryptedPgpData(StringHashMap &)
{
}

void ManifestImport::doEncryptedCipherValue()
{
    if ( aKeyInfoSequence.size() == 3 )
    {
        aKeyInfoSequence[2].Name = "CipherValue";
        uno::Sequence < sal_Int8 > aDecodeBuffer;
        ::sax::Converter::decodeBase64(aDecodeBuffer, aCurrentCharacters);
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
        ::sax::Converter::decodeBase64(aDecodeBuffer, aCurrentCharacters);
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
        ::sax::Converter::decodeBase64(aDecodeBuffer, aCurrentCharacters);
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
    OUString aString = rConvertedAttribs[sChecksumTypeAttribute];
    if ( !bIgnoreEncryptData ) {
        if ( aString == sSHA1_1k_Name || aString == sSHA1_1k_URL ) {
            aSequence[PKG_MNFST_DIGESTALG].Name = sDigestAlgProperty;
            aSequence[PKG_MNFST_DIGESTALG].Value <<= xml::crypto::DigestID::SHA1_1K;
        } else if ( aString == sSHA256_1k_URL ) {
            aSequence[PKG_MNFST_DIGESTALG].Name = sDigestAlgProperty;
            aSequence[PKG_MNFST_DIGESTALG].Value <<= xml::crypto::DigestID::SHA256_1K;
        } else
            bIgnoreEncryptData = true;

        if ( !bIgnoreEncryptData ) {
            aString = rConvertedAttribs[sChecksumAttribute];
            uno::Sequence < sal_Int8 > aDecodeBuffer;
            ::sax::Converter::decodeBase64(aDecodeBuffer, aString);
            aSequence[PKG_MNFST_DIGEST].Name = sDigestProperty;
            aSequence[PKG_MNFST_DIGEST].Value <<= aDecodeBuffer;
        }
    }
}

void ManifestImport::doAlgorithm(StringHashMap &rConvertedAttribs)
{
    if ( !bIgnoreEncryptData ) {
        OUString aString = rConvertedAttribs[sAlgorithmNameAttribute];
        if ( aString == sBlowfish_Name || aString == sBlowfish_URL ) {
            aSequence[PKG_MNFST_ENCALG].Name = sEncryptionAlgProperty;
            aSequence[PKG_MNFST_ENCALG].Value <<= xml::crypto::CipherID::BLOWFISH_CFB_8;
        } else if ( aString == sAES256_URL ) {
            aSequence[PKG_MNFST_ENCALG].Name = sEncryptionAlgProperty;
            aSequence[PKG_MNFST_ENCALG].Value <<= xml::crypto::CipherID::AES_CBC_W3C_PADDING;
            OSL_ENSURE( !nDerivedKeySize || nDerivedKeySize == 32, "Unexpected derived key length!" );
            nDerivedKeySize = 32;
        } else if ( aString == sAES192_URL ) {
            aSequence[PKG_MNFST_ENCALG].Name = sEncryptionAlgProperty;
            aSequence[PKG_MNFST_ENCALG].Value <<= xml::crypto::CipherID::AES_CBC_W3C_PADDING;
            OSL_ENSURE( !nDerivedKeySize || nDerivedKeySize == 24, "Unexpected derived key length!" );
            nDerivedKeySize = 24;
        } else if ( aString == sAES128_URL ) {
            aSequence[PKG_MNFST_ENCALG].Name = sEncryptionAlgProperty;
            aSequence[PKG_MNFST_ENCALG].Value <<= xml::crypto::CipherID::AES_CBC_W3C_PADDING;
            OSL_ENSURE( !nDerivedKeySize || nDerivedKeySize == 16, "Unexpected derived key length!" );
            nDerivedKeySize = 16;
        } else
            bIgnoreEncryptData = true;

        if ( !bIgnoreEncryptData ) {
            aString = rConvertedAttribs[sInitialisationVectorAttribute];
            uno::Sequence < sal_Int8 > aDecodeBuffer;
            ::sax::Converter::decodeBase64(aDecodeBuffer, aString);
            aSequence[PKG_MNFST_INIVECTOR].Name = sInitialisationVectorProperty;
            aSequence[PKG_MNFST_INIVECTOR].Value <<= aDecodeBuffer;
        }
    }
}

void ManifestImport::doKeyDerivation(StringHashMap &rConvertedAttribs)
{
    if ( !bIgnoreEncryptData ) {
        OUString aString = rConvertedAttribs[sKeyDerivationNameAttribute];
        if ( aString == sPBKDF2_Name || aString == sPBKDF2_URL ) {
            aString = rConvertedAttribs[sSaltAttribute];
            uno::Sequence < sal_Int8 > aDecodeBuffer;
            ::sax::Converter::decodeBase64(aDecodeBuffer, aString);
            aSequence[PKG_MNFST_SALT].Name = sSaltProperty;
            aSequence[PKG_MNFST_SALT].Value <<= aDecodeBuffer;

            aString = rConvertedAttribs[sIterationCountAttribute];
            aSequence[PKG_MNFST_ITERATION].Name = sIterationCountProperty;
            aSequence[PKG_MNFST_ITERATION].Value <<= aString.toInt32();

            aString = rConvertedAttribs[sKeySizeAttribute];
            if ( aString.getLength() ) {
                sal_Int32 nKey = aString.toInt32();
                OSL_ENSURE( !nDerivedKeySize || nKey == nDerivedKeySize , "Provided derived key length differs from the expected one!" );
                nDerivedKeySize = nKey;
            } else if ( !nDerivedKeySize )
                nDerivedKeySize = 16;
            else if ( nDerivedKeySize != 16 )
                OSL_ENSURE( false, "Default derived key length differs from the expected one!" );

            aSequence[PKG_MNFST_DERKEYSIZE].Name = sDerivedKeySizeProperty;
            aSequence[PKG_MNFST_DERKEYSIZE].Value <<= nDerivedKeySize;
        } else if ( bPgpEncryption ) {
            if ( aString != "PGP" )
                bIgnoreEncryptData = true;
        } else
            bIgnoreEncryptData = true;
    }
}

void ManifestImport::doStartKeyAlg(StringHashMap &rConvertedAttribs)
{
    OUString aString = rConvertedAttribs[sStartKeyAlgNameAttribute];
    if (aString == sSHA256_URL || aString == sSHA256_URL_ODF12) {
        aSequence[PKG_MNFST_STARTALG].Name = sStartKeyAlgProperty;
        aSequence[PKG_MNFST_STARTALG].Value <<= xml::crypto::DigestID::SHA256;
    } else if ( aString == sSHA1_Name || aString == sSHA1_URL ) {
        aSequence[PKG_MNFST_STARTALG].Name = sStartKeyAlgProperty;
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
        if (aConvertedName == sFileEntryElement) //manifest:file-entry
            doFileEntry(aConvertedAttribs);
        else if (aConvertedName == sManifestKeyInfoElement) //loext:KeyInfo
            doKeyInfoEntry(aConvertedAttribs);
        else
            aStack.back().m_bValid = false;
        break;
    }
    case 3: {
        ManifestStack::reverse_iterator aIter = aStack.rbegin();
        ++aIter;

        if (!aIter->m_bValid)
            aStack.back().m_bValid = false;
        else if (aConvertedName == sEncryptionDataElement)   //manifest:encryption-data
            doEncryptionData(aConvertedAttribs);
        else if (aConvertedName == sEncryptedKeyElement)   //loext:encrypted-key
            doEncryptedKey(aConvertedAttribs);
        else
            aStack.back().m_bValid = false;
        break;
    }
    case 4: {
        ManifestStack::reverse_iterator aIter = aStack.rbegin();
        ++aIter;

        if (!aIter->m_bValid)
            aStack.back().m_bValid = false;
        else if (aConvertedName == sAlgorithmElement)   //manifest:algorithm,
            doAlgorithm(aConvertedAttribs);
        else if (aConvertedName == sKeyDerivationElement) //manifest:key-derivation,
            doKeyDerivation(aConvertedAttribs);
        else if (aConvertedName == sStartKeyAlgElement)   //manifest:start-key-generation
            doStartKeyAlg(aConvertedAttribs);
        else if (aConvertedName == sEncryptionMethodElement)   //loext:encryption-method
            doEncryptionMethod(aConvertedAttribs);
        else if (aConvertedName == sKeyInfoElement)            //loext:KeyInfo
            doEncryptedKeyInfo(aConvertedAttribs);
        else if (aConvertedName == sCipherDataElement)            //loext:CipherData
            doEncryptedCipherData(aConvertedAttribs);
        else
            aStack.back().m_bValid = false;
        break;
    }
    case 5: {
        ManifestStack::reverse_iterator aIter = aStack.rbegin();
        ++aIter;

        if (!aIter->m_bValid)
            aStack.back().m_bValid = false;
        else if (aConvertedName == sPgpDataElement)   //loext:PGPData
            doEncryptedPgpData(aConvertedAttribs);
        else if (aConvertedName == sCipherValueElement) //loext:CipherValue
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
        else if (aConvertedName == sPgpKeyIDElement)   //loext:PGPKeyID
            // ciphervalue action happens on endElement
            aCurrentCharacters = "";
        else if (aConvertedName == sPGPKeyPacketElement) //loext:PGPKeyPacket
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
    if ( !aStack.empty() && aStack.rbegin()->m_aConvertedName == aConvertedName ) {
        if ( aConvertedName == sFileEntryElement && aStack.back().m_bValid ) {
            // root folder gets KeyInfo entry if any, for PGP encryption
            if (!bIgnoreEncryptData && !aKeys.empty() && aSequence[PKG_MNFST_FULLPATH].Value.get<OUString>() == "/" )
            {
                aSequence[PKG_SIZE_NOENCR_MNFST].Name = "KeyInfo";
                aSequence[PKG_SIZE_NOENCR_MNFST].Value <<= comphelper::containerToSequence(aKeys);
            }
            css::beans::PropertyValue aEmpty;
            aSequence.erase(std::remove_if(aSequence.begin(), aSequence.end(),
                                           isEmpty), aSequence.end());

            bIgnoreEncryptData = false;
            rManVector.push_back ( comphelper::containerToSequence(aSequence) );

            aSequence.clear();
        }
        else if ( aConvertedName == sEncryptedKeyElement && aStack.back().m_bValid ) {
            if ( !bIgnoreEncryptData )
            {
                aKeys.push_back( comphelper::containerToSequence(aKeyInfoSequence) );
                bPgpEncryption = true;
            }
            aKeyInfoSequence.clear();
        }

        // end element handling for elements with cdata
        switch (nLevel) {
            case 5: {
                if (aConvertedName == sCipherValueElement) //loext:CipherValue
                    doEncryptedCipherValue();
                else
                    aStack.back().m_bValid = false;
                break;
            }
            case 6: {
                if (aConvertedName == sPgpKeyIDElement)   //loext:PGPKeyID
                    doEncryptedKeyId();
                else if (aConvertedName == sPGPKeyPacketElement) //loext:PGPKeyPacket
                    doEncryptedKeyPacket();
                else
                    aStack.back().m_bValid = false;
                break;
            }
        }

        aStack.pop_back();
        return;
    }
}

void SAL_CALL ManifestImport::characters( const OUString& aChars )
{
    aCurrentCharacters += aChars;
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
        aResult = MANIFEST_NSPREFIX;
        aResult += aPureName;
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
