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

// MARKER( update_precomp.py ): autogen include statement, do not remove
#include <ManifestImport.hxx>
#include <ManifestDefines.hxx>
#include <sax/tools/converter.hxx>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/xml/crypto/DigestID.hpp>
#include <com/sun/star/xml/crypto/CipherID.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star;
using namespace std;

ManifestImport::ManifestImport( vector < Sequence < PropertyValue > > & rNewManVector )
: bIgnoreEncryptData    ( sal_False )
, nDerivedKeySize( 0 )
, rManVector ( rNewManVector )

, sFileEntryElement     ( ELEMENT_FILE_ENTRY )
, sManifestElement      ( ELEMENT_MANIFEST )
, sEncryptionDataElement( ELEMENT_ENCRYPTION_DATA )
, sAlgorithmElement ( ELEMENT_ALGORITHM )
, sStartKeyAlgElement   ( ELEMENT_START_KEY_GENERATION )
, sKeyDerivationElement( ELEMENT_KEY_DERIVATION )

, sCdataAttribute               ( ATTRIBUTE_CDATA )
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

, sWhiteSpace                   ( " " )

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

ManifestImport::~ManifestImport ( void )
{
}

void SAL_CALL ManifestImport::startDocument(  )
        throw( xml::sax::SAXException, uno::RuntimeException )
{
}

void SAL_CALL ManifestImport::endDocument(  )
        throw( xml::sax::SAXException, uno::RuntimeException )
{
}

void ManifestImport::doFileEntry(StringHashMap &rConvertedAttribs)
        throw( uno::RuntimeException )
{
    aSequence.resize(PKG_SIZE_ENCR_MNFST);

    aSequence[PKG_MNFST_FULLPATH].Name = sFullPathProperty;
    aSequence[PKG_MNFST_FULLPATH].Value <<= rConvertedAttribs[sFullPathAttribute];
    aSequence[PKG_MNFST_MEDIATYPE].Name = sMediaTypeProperty;
    aSequence[PKG_MNFST_MEDIATYPE].Value <<= rConvertedAttribs[sMediaTypeAttribute];

    OUString sVersion = rConvertedAttribs[sVersionAttribute];
    if ( sVersion.getLength() )
    {
        aSequence[PKG_MNFST_VERSION].Name = sVersionProperty;
        aSequence[PKG_MNFST_VERSION].Value <<= sVersion;
    }

    OUString sSize = rConvertedAttribs[sSizeAttribute];
    if ( sSize.getLength() )
    {
        sal_Int64 nSize = sSize.toInt64();
        aSequence[PKG_MNFST_UCOMPSIZE].Name = sSizeProperty;
        aSequence[PKG_MNFST_UCOMPSIZE].Value <<= nSize;
    }
}

void ManifestImport::doEncryptionData(StringHashMap &rConvertedAttribs)
        throw( uno::RuntimeException )
{
    // If this element exists, then this stream is encrypted and we need
    // to import the initialisation vector, salt and iteration count used
    nDerivedKeySize = 0;
    OUString aString = rConvertedAttribs[sChecksumTypeAttribute];
    if ( !bIgnoreEncryptData )
    {
        if ( aString.equals( sSHA1_1k_Name ) || aString.equals( sSHA1_1k_URL ) )
        {
            aSequence[PKG_MNFST_DIGESTALG].Name = sDigestAlgProperty;
            aSequence[PKG_MNFST_DIGESTALG].Value <<= xml::crypto::DigestID::SHA1_1K;
        }
        else if ( aString.equals( sSHA256_1k_URL ) )
        {
            aSequence[PKG_MNFST_DIGESTALG].Name = sDigestAlgProperty;
            aSequence[PKG_MNFST_DIGESTALG].Value <<= xml::crypto::DigestID::SHA256_1K;
        }
        else
            bIgnoreEncryptData = sal_True;

        if ( !bIgnoreEncryptData )
        {
            aString = rConvertedAttribs[sChecksumAttribute];
            uno::Sequence < sal_Int8 > aDecodeBuffer;
            ::sax::Converter::decodeBase64(aDecodeBuffer, aString);
            aSequence[PKG_MNFST_DIGEST].Name = sDigestProperty;
            aSequence[PKG_MNFST_DIGEST].Value <<= aDecodeBuffer;
        }
    }
}

void ManifestImport::doAlgorithm(StringHashMap &rConvertedAttribs)
        throw( uno::RuntimeException )
{
    if ( !bIgnoreEncryptData )
    {
        OUString aString = rConvertedAttribs[sAlgorithmNameAttribute];
        if ( aString.equals( sBlowfish_Name ) || aString.equals( sBlowfish_URL ) )
        {
            aSequence[PKG_MNFST_ENCALG].Name = sEncryptionAlgProperty;
            aSequence[PKG_MNFST_ENCALG].Value <<= xml::crypto::CipherID::BLOWFISH_CFB_8;
        }
        else if ( aString.equals( sAES256_URL ) )
        {
            aSequence[PKG_MNFST_ENCALG].Name = sEncryptionAlgProperty;
            aSequence[PKG_MNFST_ENCALG].Value <<= xml::crypto::CipherID::AES_CBC_W3C_PADDING;
            OSL_ENSURE( !nDerivedKeySize || nDerivedKeySize == 32, "Unexpected derived key length!" );
            nDerivedKeySize = 32;
        }
        else if ( aString.equals( sAES192_URL ) )
        {
            aSequence[PKG_MNFST_ENCALG].Name = sEncryptionAlgProperty;
            aSequence[PKG_MNFST_ENCALG].Value <<= xml::crypto::CipherID::AES_CBC_W3C_PADDING;
            OSL_ENSURE( !nDerivedKeySize || nDerivedKeySize == 24, "Unexpected derived key length!" );
            nDerivedKeySize = 24;
        }
        else if ( aString.equals( sAES128_URL ) )
        {
            aSequence[PKG_MNFST_ENCALG].Name = sEncryptionAlgProperty;
            aSequence[PKG_MNFST_ENCALG].Value <<= xml::crypto::CipherID::AES_CBC_W3C_PADDING;
            OSL_ENSURE( !nDerivedKeySize || nDerivedKeySize == 16, "Unexpected derived key length!" );
            nDerivedKeySize = 16;
        }
        else
            bIgnoreEncryptData = sal_True;

        if ( !bIgnoreEncryptData )
        {
            aString = rConvertedAttribs[sInitialisationVectorAttribute];
            uno::Sequence < sal_Int8 > aDecodeBuffer;
            ::sax::Converter::decodeBase64(aDecodeBuffer, aString);
            aSequence[PKG_MNFST_INIVECTOR].Name = sInitialisationVectorProperty;
            aSequence[PKG_MNFST_INIVECTOR].Value <<= aDecodeBuffer;
        }
    }
}

void ManifestImport::doKeyDerivation(StringHashMap &rConvertedAttribs)
        throw( uno::RuntimeException )
{
    if ( !bIgnoreEncryptData )
    {
        OUString aString = rConvertedAttribs[sKeyDerivationNameAttribute];
        if ( aString.equals( sPBKDF2_Name ) || aString.equals( sPBKDF2_URL ) )
        {
            aString = rConvertedAttribs[sSaltAttribute];
            uno::Sequence < sal_Int8 > aDecodeBuffer;
            ::sax::Converter::decodeBase64(aDecodeBuffer, aString);
            aSequence[PKG_MNFST_SALT].Name = sSaltProperty;
            aSequence[PKG_MNFST_SALT].Value <<= aDecodeBuffer;

            aString = rConvertedAttribs[sIterationCountAttribute];
            aSequence[PKG_MNFST_ITERATION].Name = sIterationCountProperty;
            aSequence[PKG_MNFST_ITERATION].Value <<= aString.toInt32();

            aString = rConvertedAttribs[sKeySizeAttribute];
            if ( aString.getLength() )
            {
                sal_Int32 nKey = aString.toInt32();
                OSL_ENSURE( !nDerivedKeySize || nKey == nDerivedKeySize , "Provided derived key length differs from the expected one!" );
                nDerivedKeySize = nKey;
            }
            else if ( !nDerivedKeySize )
                nDerivedKeySize = 16;
            else if ( nDerivedKeySize != 16 )
                OSL_ENSURE( sal_False, "Default derived key length differs from the expected one!" );

            aSequence[PKG_MNFST_DERKEYSIZE].Name = sDerivedKeySizeProperty;
            aSequence[PKG_MNFST_DERKEYSIZE].Value <<= nDerivedKeySize;
        }
        else
            bIgnoreEncryptData = sal_True;
    }
}

void ManifestImport::doStartKeyAlg(StringHashMap &rConvertedAttribs)
        throw( uno::RuntimeException )
{
    OUString aString = rConvertedAttribs[sStartKeyAlgNameAttribute];
    if ( aString.equals( sSHA256_URL ) )
    {
        aSequence[PKG_MNFST_STARTALG].Name = sStartKeyAlgProperty;
        aSequence[PKG_MNFST_STARTALG].Value <<= xml::crypto::DigestID::SHA256;
    }
    else if ( aString.equals( sSHA1_Name ) || aString.equals( sSHA1_URL ) )
    {
        aSequence[PKG_MNFST_STARTALG].Name = sStartKeyAlgProperty;
        aSequence[PKG_MNFST_STARTALG].Value <<= xml::crypto::DigestID::SHA1;
    }
    else
        bIgnoreEncryptData = sal_True;
}

void SAL_CALL ManifestImport::startElement( const OUString& aName, const uno::Reference< xml::sax::XAttributeList >& xAttribs )
        throw( xml::sax::SAXException, uno::RuntimeException )
{
    StringHashMap aConvertedAttribs;
    OUString aConvertedName = PushNameAndNamespaces( aName, xAttribs, aConvertedAttribs );

    size_t nLevel = aStack.size();

    assert(nLevel >= 1);

    switch (nLevel)
    {
        case 1:
        {
            if (!aConvertedName.equalsAscii(ELEMENT_MANIFEST)) //manifest:manifest
                aStack.back().m_bValid = false;
            break;
        }
        case 2:
        {
            if (aConvertedName == sFileEntryElement) //manifest:file-entry
                doFileEntry(aConvertedAttribs);
            else
                aStack.back().m_bValid = false;
            break;
        }
        case 3:
        {
            ManifestStack::reverse_iterator aIter = aStack.rbegin();
            ++aIter;

            if (!aIter->m_bValid)
                aStack.back().m_bValid = false;
            else if (aConvertedName.equals(sEncryptionDataElement))   //manifest:encryption-data
                doEncryptionData(aConvertedAttribs);
            else
                aStack.back().m_bValid = false;
            break;
        }
        case 4:
        {
            ManifestStack::reverse_iterator aIter = aStack.rbegin();
            ++aIter;

            if (!aIter->m_bValid)
                aStack.back().m_bValid = false;
            else if (aConvertedName.equals(sAlgorithmElement))   //manifest:algorithm,
                doAlgorithm(aConvertedAttribs);
            else if (aConvertedName.equals(sKeyDerivationElement)) //manifest:key-derivation,
                doKeyDerivation(aConvertedAttribs);
            else if (aConvertedName.equals(sStartKeyAlgElement))   //manifest:start-key-generation
                doStartKeyAlg(aConvertedAttribs);
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
    bool isEmpty(const com::sun::star::beans::PropertyValue &rProp)
    {
        return rProp.Name.isEmpty();
    }
}

void SAL_CALL ManifestImport::endElement( const OUString& aName )
    throw( xml::sax::SAXException, uno::RuntimeException )
{
    OUString aConvertedName = ConvertName( aName );
    if ( !aStack.empty() && aStack.rbegin()->m_aConvertedName.equals( aConvertedName ) )
    {
        if ( aConvertedName.equals( sFileEntryElement ) && aStack.back().m_bValid )
        {
            com::sun::star::beans::PropertyValue aEmpty;
            aSequence.erase(std::remove_if(aSequence.begin(), aSequence.end(),
                isEmpty), aSequence.end());

            bIgnoreEncryptData = sal_False;
            rManVector.push_back ( aSequence.getAsConstList() );

            aSequence.clear();
        }

        aStack.pop_back();
    }
}

void SAL_CALL ManifestImport::characters( const OUString& /*aChars*/ )
        throw( xml::sax::SAXException, uno::RuntimeException )
{
}

void SAL_CALL ManifestImport::ignorableWhitespace( const OUString& /*aWhitespaces*/ )
        throw( xml::sax::SAXException, uno::RuntimeException )
{
}

void SAL_CALL ManifestImport::processingInstruction( const OUString& /*aTarget*/, const OUString& /*aData*/ )
        throw( xml::sax::SAXException, uno::RuntimeException )
{
}

void SAL_CALL ManifestImport::setDocumentLocator( const uno::Reference< xml::sax::XLocator >& /*xLocator*/ )
        throw( xml::sax::SAXException, uno::RuntimeException )
{
}

OUString ManifestImport::PushNameAndNamespaces( const OUString& aName, const uno::Reference< xml::sax::XAttributeList >& xAttribs, StringHashMap& o_aConvertedAttribs )
{
    StringHashMap aNamespaces;
    ::std::vector< ::std::pair< OUString, OUString > > aAttribsStrs;

    if ( xAttribs.is() )
    {
        sal_Int16 nAttrCount = xAttribs.is() ? xAttribs->getLength() : 0;
        aAttribsStrs.reserve( nAttrCount );

        for( sal_Int16 nInd = 0; nInd < nAttrCount; nInd++ )
        {
            OUString aAttrName = xAttribs->getNameByIndex( nInd );
            OUString aAttrValue = xAttribs->getValueByIndex( nInd );
            if ( aAttrName.getLength() >= 5
              && aAttrName.startsWith("xmlns")
              && ( aAttrName.getLength() == 5 || aAttrName.getStr()[5] == ( sal_Unicode )':' ) )
            {
                // this is a namespace declaration
                OUString aNsName( ( aAttrName.getLength() == 5 ) ? OUString() : aAttrName.copy( 6 ) );
                aNamespaces[aNsName] = aAttrValue;
            }
            else
            {
                // this is no namespace declaration
                aAttribsStrs.push_back( pair< OUString, OUString >( aAttrName, aAttrValue ) );
            }
        }
    }

    OUString aConvertedName = ConvertNameWithNamespace( aName, aNamespaces );
    if ( !aConvertedName.getLength() )
        aConvertedName = ConvertName( aName );

    aStack.push_back( ManifestScopeEntry( aConvertedName, aNamespaces ) );

    for ( sal_uInt16 nInd = 0; nInd < aAttribsStrs.size(); nInd++ )
    {
        // convert the attribute names on filling
        o_aConvertedAttribs[ConvertName( aAttribsStrs[nInd].first )] = aAttribsStrs[nInd].second;
    }

    return aConvertedName;
}

OUString ManifestImport::ConvertNameWithNamespace( const OUString& aName, const StringHashMap& aNamespaces )
{
    OUString aNsAlias;
    OUString aPureName = aName;

    sal_Int32 nInd = aName.indexOf( ( sal_Unicode )':' );
    if ( nInd != -1 && nInd < aName.getLength() )
    {
        aNsAlias = aName.copy( 0, nInd );
        aPureName = aName.copy( nInd + 1 );
    }

    OUString aResult;

    StringHashMap::const_iterator aIter = aNamespaces.find( aNsAlias );
    if ( aIter != aNamespaces.end()
      && ( aIter->second == MANIFEST_NAMESPACE || aIter->second == MANIFEST_OASIS_NAMESPACE ) )
    {
        // no check for manifest.xml consistency currently since the old versions have supported inconsistent documents as well
        aResult = MANIFEST_NSPREFIX;
        aResult += aPureName;
    }

    return aResult;
}

OUString ManifestImport::ConvertName( const OUString& aName )
{
    OUString aConvertedName;
    for ( ManifestStack::reverse_iterator aIter = aStack.rbegin(); !aConvertedName.getLength() && aIter != aStack.rend(); ++aIter )
    {
        if ( !aIter->m_aNamespaces.empty() )
            aConvertedName = ConvertNameWithNamespace( aName, aIter->m_aNamespaces );
    }

    if ( !aConvertedName.getLength() )
        aConvertedName = aName;

    return aConvertedName;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
