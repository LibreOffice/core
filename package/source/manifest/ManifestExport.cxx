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

#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/xml/crypto/DigestID.hpp>
#include <com/sun/star/xml/crypto/CipherID.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>

#include "ManifestDefines.hxx"
#include "ManifestExport.hxx"
#include <sax/tools/converter.hxx>

#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/ref.hxx>
#include <sal/log.hxx>
#include <comphelper/base64.hxx>
#include <comphelper/documentconstants.hxx>
#include <comphelper/attributelist.hxx>

using namespace ::com::sun::star;

#if OSL_DEBUG_LEVEL > 0
#define THROW_WHERE SAL_WHERE
#else
#define THROW_WHERE ""
#endif

ManifestExport::ManifestExport( uno::Reference< xml::sax::XDocumentHandler > const & xHandler,  const uno::Sequence< uno::Sequence < beans::PropertyValue > >& rManList )
{
    const OUString sFileEntryElement     ( ELEMENT_FILE_ENTRY );
    const OUString sManifestElement      ( ELEMENT_MANIFEST );
    const OUString sEncryptionDataElement( ELEMENT_ENCRYPTION_DATA );
    const OUString sAlgorithmElement     ( ELEMENT_ALGORITHM );
    const OUString sStartKeyGenerationElement ( ELEMENT_START_KEY_GENERATION );
    const OUString sKeyDerivationElement ( ELEMENT_KEY_DERIVATION );

    const OUString sCdataAttribute       ( ATTRIBUTE_CDATA );
    const OUString sMediaTypeAttribute   ( ATTRIBUTE_MEDIA_TYPE );
    const OUString sVersionAttribute     ( ATTRIBUTE_VERSION );
    const OUString sFullPathAttribute    ( ATTRIBUTE_FULL_PATH );
    const OUString sSizeAttribute        ( ATTRIBUTE_SIZE );
    const OUString sKeySizeAttribute     ( ATTRIBUTE_KEY_SIZE );
    const OUString sSaltAttribute        ( ATTRIBUTE_SALT );
    const OUString sInitialisationVectorAttribute ( ATTRIBUTE_INITIALISATION_VECTOR );
    const OUString sIterationCountAttribute  ( ATTRIBUTE_ITERATION_COUNT );
    const OUString sAlgorithmNameAttribute   ( ATTRIBUTE_ALGORITHM_NAME );
    const OUString sStartKeyGenerationNameAttribute ( ATTRIBUTE_START_KEY_GENERATION_NAME );
    const OUString sKeyDerivationNameAttribute   ( ATTRIBUTE_KEY_DERIVATION_NAME );
    const OUString sChecksumTypeAttribute    ( ATTRIBUTE_CHECKSUM_TYPE );
    const OUString sChecksumAttribute    ( ATTRIBUTE_CHECKSUM);

    const OUString sKeyInfoElement              ( ELEMENT_ENCRYPTED_KEYINFO );
    const OUString sManifestKeyInfoElement      ( ELEMENT_MANIFEST_KEYINFO );
    const OUString sEncryptedKeyElement         ( ELEMENT_ENCRYPTEDKEY );
    const OUString sEncryptionMethodElement     ( ELEMENT_ENCRYPTIONMETHOD );
    const OUString sPgpDataElement              ( ELEMENT_PGPDATA );
    const OUString sPgpKeyIDElement             ( ELEMENT_PGPKEYID );
    const OUString sPGPKeyPacketElement         ( ELEMENT_PGPKEYPACKET );
    const OUString sAlgorithmAttribute          ( ATTRIBUTE_ALGORITHM );
    const OUString sCipherDataElement           ( ELEMENT_CIPHERDATA );
    const OUString sCipherValueElement          ( ELEMENT_CIPHERVALUE );
    const OUString sManifestKeyInfoElement13    ( ELEMENT_MANIFEST13_KEYINFO );
    const OUString sEncryptedKeyElement13       ( ELEMENT_ENCRYPTEDKEY13 );
    const OUString sEncryptionMethodElement13   ( ELEMENT_ENCRYPTIONMETHOD13 );
    const OUString sPgpDataElement13            ( ELEMENT_PGPDATA13 );
    const OUString sPgpKeyIDElement13           ( ELEMENT_PGPKEYID13 );
    const OUString sPGPKeyPacketElement13       ( ELEMENT_PGPKEYPACKET13 );
    const OUString sAlgorithmAttribute13        ( ATTRIBUTE_ALGORITHM13 );
    const OUString sCipherDataElement13         ( ELEMENT_CIPHERDATA13 );
    const OUString sCipherValueElement13        ( ELEMENT_CIPHERVALUE13 );
    const OUString sKeyInfo                     ( "KeyInfo" );
    const OUString sPgpKeyIDProperty            ( "KeyId" );
    const OUString sPgpKeyPacketProperty        ( "KeyPacket" );
    const OUString sCipherValueProperty         ( "CipherValue" );
    const OUString sFullPathProperty     ( "FullPath" );
    const OUString sVersionProperty  ( "Version" );
    const OUString sMediaTypeProperty    ( "MediaType" );
    const OUString sIterationCountProperty   ( "IterationCount" );
    const OUString sDerivedKeySizeProperty  ( "DerivedKeySize" );
    const OUString sSaltProperty         ( "Salt" );
    const OUString sInitialisationVectorProperty( "InitialisationVector" );
    const OUString sSizeProperty         ( "Size" );
    const OUString sDigestProperty       ( "Digest" );
    const OUString sEncryptionAlgProperty    ( "EncryptionAlgorithm" );
    const OUString sStartKeyAlgProperty  ( "StartKeyAlgorithm" );
    const OUString sDigestAlgProperty    ( "DigestAlgorithm" );

    const OUString sWhiteSpace           ( " " );

    const OUString sSHA256_URL_ODF12     ( SHA256_URL_ODF12 );
    const OUString  sSHA1_Name           ( SHA1_NAME );

    const OUString  sSHA1_1k_Name        ( SHA1_1K_NAME );
    const OUString  sSHA256_1k_URL       ( SHA256_1K_URL );

    const OUString  sBlowfish_Name       ( BLOWFISH_NAME );
    const OUString  sAES256_URL          ( AES256_URL );

    const OUString  sPBKDF2_Name         ( PBKDF2_NAME );
    const OUString  sPGP_Name            ( PGP_NAME );

    rtl::Reference<::comphelper::AttributeList> pRootAttrList = new ::comphelper::AttributeList;

    // find the mediatype of the document if any
    OUString aDocMediaType;
    OUString aDocVersion;
    const uno::Sequence<beans::PropertyValue>* pRootFolderPropSeq = nullptr;
    for (const uno::Sequence < beans::PropertyValue >& rSequence : rManList)
    {
        OUString aMediaType;
        OUString aPath;
        OUString aVersion;

        for (const beans::PropertyValue& rValue : rSequence)
        {
            if (rValue.Name == sMediaTypeProperty )
            {
                rValue.Value >>= aMediaType;
            }
            else if (rValue.Name == sFullPathProperty )
            {
                rValue.Value >>= aPath;
            }
            else if (rValue.Name == sVersionProperty )
            {
                rValue.Value >>= aVersion;
            }

            if ( !aPath.isEmpty() && !aMediaType.isEmpty() && !aVersion.isEmpty() )
                break;
        }

        if ( aPath == "/" )
        {
            aDocMediaType = aMediaType;
            aDocVersion = aVersion;
            pRootFolderPropSeq = &rSequence;
            break;
        }
    }

    bool bProvideDTD = false;
    bool bAcceptNonemptyVersion = false;
    bool bStoreStartKeyGeneration = false;
    if ( !aDocMediaType.isEmpty() )
    {
        if ( aDocMediaType == MIMETYPE_OASIS_OPENDOCUMENT_TEXT_ASCII
          || aDocMediaType == MIMETYPE_OASIS_OPENDOCUMENT_TEXT_WEB_ASCII
          || aDocMediaType == MIMETYPE_OASIS_OPENDOCUMENT_TEXT_GLOBAL_ASCII
          || aDocMediaType == MIMETYPE_OASIS_OPENDOCUMENT_DRAWING_ASCII
          || aDocMediaType == MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_ASCII
          || aDocMediaType == MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET_ASCII
          || aDocMediaType == MIMETYPE_OASIS_OPENDOCUMENT_CHART_ASCII
          || aDocMediaType == MIMETYPE_OASIS_OPENDOCUMENT_DATABASE_ASCII
          || aDocMediaType == MIMETYPE_OASIS_OPENDOCUMENT_FORMULA_ASCII
          || aDocMediaType == MIMETYPE_OASIS_OPENDOCUMENT_TEXT_TEMPLATE_ASCII
          || aDocMediaType == MIMETYPE_OASIS_OPENDOCUMENT_TEXT_GLOBAL_TEMPLATE_ASCII
          || aDocMediaType == MIMETYPE_OASIS_OPENDOCUMENT_DRAWING_TEMPLATE_ASCII
          || aDocMediaType == MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_TEMPLATE_ASCII
          || aDocMediaType == MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET_TEMPLATE_ASCII
          || aDocMediaType == MIMETYPE_OASIS_OPENDOCUMENT_CHART_TEMPLATE_ASCII
          || aDocMediaType == MIMETYPE_OASIS_OPENDOCUMENT_FORMULA_TEMPLATE_ASCII )

        {
            // oasis format
            pRootAttrList->AddAttribute ( ATTRIBUTE_XMLNS,
                                        sCdataAttribute,
                                        MANIFEST_OASIS_NAMESPACE );
            bAcceptNonemptyVersion = true;
            if ( aDocVersion.compareTo( u"" ODFVER_012_TEXT ) >= 0 )
            {
                // this is ODF12 or later generation, let encrypted
                // streams contain start-key-generation entry
                bStoreStartKeyGeneration = true;
                pRootAttrList->AddAttribute ( sVersionAttribute, sCdataAttribute, aDocVersion );
                // plus gpg4libre extensions - loext NS for that
                pRootAttrList->AddAttribute ( ATTRIBUTE_XMLNS_LOEXT,
                                              sCdataAttribute,
                                              MANIFEST_LOEXT_NAMESPACE );
            }
        }
        else
        {
            // even if it is no SO6 format the namespace must be specified
            // thus SO6 format is used as default one
            pRootAttrList->AddAttribute ( ATTRIBUTE_XMLNS,
                                        sCdataAttribute,
                                        MANIFEST_NAMESPACE );

            bProvideDTD = true;
        }
    }

    xHandler->startDocument();
    uno::Reference < xml::sax::XExtendedDocumentHandler > xExtHandler ( xHandler, uno::UNO_QUERY );
    if ( xExtHandler.is() && bProvideDTD )
    {
        xExtHandler->unknown ( MANIFEST_DOCTYPE );
        xHandler->ignorableWhitespace ( sWhiteSpace );
    }
    xHandler->startElement( sManifestElement, pRootAttrList );

    const uno::Any *pKeyInfoProperty = nullptr;
    if ( pRootFolderPropSeq )
    {
        // do we have package-wide encryption info?
        for (const beans::PropertyValue& rValue : *pRootFolderPropSeq)
        {
            if (rValue.Name == sKeyInfo )
                pKeyInfoProperty = &rValue.Value;
        }

        if ( pKeyInfoProperty )
        {
            // yeah, so that goes directly below the manifest:manifest
            // element
            OUStringBuffer aBuffer;

            xHandler->ignorableWhitespace ( sWhiteSpace );

            // ==== manifest:keyinfo & children
            bool const isODF13(aDocVersion.compareTo(u"" ODFVER_013_TEXT) >= 0);
            if (!isODF13)
            {
                xHandler->startElement(sManifestKeyInfoElement, nullptr);
            }
            xHandler->ignorableWhitespace ( sWhiteSpace );

            uno::Sequence< uno::Sequence < beans::NamedValue > > aKeyInfoSequence;
            *pKeyInfoProperty >>= aKeyInfoSequence;
            for (const uno::Sequence<beans::NamedValue>& rKeyInfoSequence : std::as_const(aKeyInfoSequence))
            {
                uno::Sequence < sal_Int8 > aPgpKeyID;
                uno::Sequence < sal_Int8 > aPgpKeyPacket;
                uno::Sequence < sal_Int8 > aCipherValue;
                for (const beans::NamedValue& rNValue : rKeyInfoSequence)
                {
                    if (rNValue.Name == sPgpKeyIDProperty )
                        rNValue.Value >>= aPgpKeyID;
                    else if (rNValue.Name == sPgpKeyPacketProperty )
                        rNValue.Value >>= aPgpKeyPacket;
                    else if (rNValue.Name == sCipherValueProperty )
                        rNValue.Value >>= aCipherValue;
                }

                if (aPgpKeyID.hasElements() && aCipherValue.hasElements() )
                {
                    // ==== manifest:encrypted-key & children - one for each recipient
                    xHandler->startElement(isODF13 ? sEncryptedKeyElement13 : sEncryptedKeyElement, nullptr);
                    xHandler->ignorableWhitespace ( sWhiteSpace );

                    rtl::Reference<::comphelper::AttributeList> pNewAttrList = new ::comphelper::AttributeList;
                    // TODO: the algorithm should rather be configurable
                    pNewAttrList->AddAttribute(
                        isODF13 ? sAlgorithmAttribute13 : sAlgorithmAttribute,
                        sCdataAttribute,
                                                 "http://www.w3.org/2001/04/xmlenc#rsa-oaep-mgf1p" );
                    xHandler->startElement(isODF13 ? sEncryptionMethodElement13 : sEncryptionMethodElement, pNewAttrList);
                    xHandler->endElement(isODF13 ? sEncryptionMethodElement13 :  sEncryptionMethodElement);
                    xHandler->ignorableWhitespace ( sWhiteSpace );

                    // note: the mismatch here corresponds to ODF 1.3 cs01 schema
                    xHandler->startElement(isODF13 ? sManifestKeyInfoElement13 : sKeyInfoElement, nullptr);
                    xHandler->ignorableWhitespace ( sWhiteSpace );

                    xHandler->startElement(isODF13 ? sPgpDataElement13 : sPgpDataElement, nullptr);
                    xHandler->ignorableWhitespace ( sWhiteSpace );

                    xHandler->startElement(isODF13 ? sPgpKeyIDElement13 : sPgpKeyIDElement, nullptr);
                    ::comphelper::Base64::encode(aBuffer, aPgpKeyID);
                    xHandler->characters( aBuffer.makeStringAndClear() );
                    xHandler->endElement(isODF13 ? sPgpKeyIDElement13 : sPgpKeyIDElement);
                    xHandler->ignorableWhitespace ( sWhiteSpace );

                    // key packet is optional
                    if (aPgpKeyPacket.hasElements())
                    {
                        xHandler->startElement(isODF13 ? sPGPKeyPacketElement13 : sPGPKeyPacketElement, nullptr);
                        ::comphelper::Base64::encode(aBuffer, aPgpKeyPacket);
                        xHandler->characters( aBuffer.makeStringAndClear() );
                        xHandler->endElement(isODF13 ? sPGPKeyPacketElement13 :  sPGPKeyPacketElement);
                        xHandler->ignorableWhitespace ( sWhiteSpace );
                    }

                    xHandler->endElement(isODF13 ? sPgpDataElement13 : sPgpDataElement);
                    xHandler->ignorableWhitespace ( sWhiteSpace );

                    xHandler->endElement(isODF13 ? sManifestKeyInfoElement13 : sKeyInfoElement);
                    xHandler->ignorableWhitespace ( sWhiteSpace );

                    xHandler->startElement(isODF13 ? sCipherDataElement13 : sCipherDataElement, nullptr);
                    xHandler->ignorableWhitespace ( sWhiteSpace );

                    xHandler->startElement(isODF13 ? sCipherValueElement13 : sCipherValueElement, nullptr);
                    ::comphelper::Base64::encode(aBuffer, aCipherValue);
                    xHandler->characters( aBuffer.makeStringAndClear() );
                    xHandler->endElement(isODF13 ? sCipherValueElement13 : sCipherValueElement);
                    xHandler->ignorableWhitespace ( sWhiteSpace );

                    xHandler->endElement(isODF13 ? sCipherDataElement13 : sCipherDataElement);
                    xHandler->ignorableWhitespace ( sWhiteSpace );

                    xHandler->endElement(isODF13 ? sEncryptedKeyElement13 : sEncryptedKeyElement);
                    xHandler->ignorableWhitespace ( sWhiteSpace );
                }
            }

            if (!isODF13)
            {
                xHandler->endElement(sManifestKeyInfoElement);
            }
            xHandler->ignorableWhitespace ( sWhiteSpace );
        }
    }

    // now write individual file entries
    for (const uno::Sequence<beans::PropertyValue>& rSequence : rManList)
    {
        rtl::Reference<::comphelper::AttributeList> pAttrList = new ::comphelper::AttributeList;
        OUString aString;
        const uno::Any *pVector = nullptr, *pSalt = nullptr, *pIterationCount = nullptr, *pDigest = nullptr, *pDigestAlg = nullptr, *pEncryptAlg = nullptr, *pStartKeyAlg = nullptr, *pDerivedKeySize = nullptr;
        for (const beans::PropertyValue& rValue : rSequence)
        {
            if (rValue.Name == sMediaTypeProperty )
            {
                rValue.Value >>= aString;
                pAttrList->AddAttribute ( sMediaTypeAttribute, sCdataAttribute, aString );
            }
            else if (rValue.Name == sVersionProperty )
            {
                rValue.Value >>= aString;
                // the version is stored only if it is not empty
                if ( bAcceptNonemptyVersion && !aString.isEmpty() )
                    pAttrList->AddAttribute ( sVersionAttribute, sCdataAttribute, aString );
            }
            else if (rValue.Name == sFullPathProperty )
            {
                rValue.Value >>= aString;
                pAttrList->AddAttribute ( sFullPathAttribute, sCdataAttribute, aString );
            }
            else if (rValue.Name == sSizeProperty )
            {
                sal_Int64 nSize = 0;
                rValue.Value >>= nSize;
                pAttrList->AddAttribute ( sSizeAttribute, sCdataAttribute, OUString::number( nSize ) );
            }
            else if (rValue.Name == sInitialisationVectorProperty )
                pVector = &rValue.Value;
            else if (rValue.Name == sSaltProperty )
                pSalt = &rValue.Value;
            else if (rValue.Name == sIterationCountProperty )
                pIterationCount = &rValue.Value;
            else if (rValue.Name == sDigestProperty )
                pDigest = &rValue.Value;
            else if (rValue.Name == sDigestAlgProperty )
                pDigestAlg = &rValue.Value;
            else if (rValue.Name == sEncryptionAlgProperty )
                pEncryptAlg = &rValue.Value;
            else if (rValue.Name == sStartKeyAlgProperty )
                pStartKeyAlg = &rValue.Value;
            else if (rValue.Name == sDerivedKeySizeProperty )
                pDerivedKeySize = &rValue.Value;
        }

        xHandler->ignorableWhitespace ( sWhiteSpace );
        xHandler->startElement( sFileEntryElement , pAttrList);
        if ( pVector && pSalt && pIterationCount && pDigest && pDigestAlg && pEncryptAlg && pStartKeyAlg && pDerivedKeySize )
        {
            // ==== Encryption Data
            rtl::Reference<::comphelper::AttributeList> pNewAttrList = new ::comphelper::AttributeList;
            OUStringBuffer aBuffer;
            uno::Sequence < sal_Int8 > aSequence;

            xHandler->ignorableWhitespace ( sWhiteSpace );

            // ==== Digest
            OUString sChecksumType;
            sal_Int32 nDigestAlgID = 0;
            *pDigestAlg >>= nDigestAlgID;
            if ( nDigestAlgID == xml::crypto::DigestID::SHA256_1K )
                sChecksumType = sSHA256_1k_URL;
            else if ( nDigestAlgID == xml::crypto::DigestID::SHA1_1K )
                sChecksumType = sSHA1_1k_Name;
            else
                throw uno::RuntimeException( THROW_WHERE "Unexpected digest algorithm is provided!" );

            pNewAttrList->AddAttribute ( sChecksumTypeAttribute, sCdataAttribute, sChecksumType );
            *pDigest >>= aSequence;
            ::comphelper::Base64::encode(aBuffer, aSequence);
            pNewAttrList->AddAttribute ( sChecksumAttribute, sCdataAttribute, aBuffer.makeStringAndClear() );

            xHandler->startElement( sEncryptionDataElement , pNewAttrList);

            // ==== Algorithm
            pNewAttrList = new ::comphelper::AttributeList;

            sal_Int32 nEncAlgID = 0;
            sal_Int32 nDerivedKeySize = 0;
            *pEncryptAlg >>= nEncAlgID;
            *pDerivedKeySize >>= nDerivedKeySize;

            OUString sEncAlgName;
            if ( nEncAlgID == xml::crypto::CipherID::AES_CBC_W3C_PADDING )
            {
                OSL_ENSURE( nDerivedKeySize, "Unexpected key size is provided!" );
                if ( nDerivedKeySize != 32 )
                    throw uno::RuntimeException( THROW_WHERE "Unexpected key size is provided!" );

                sEncAlgName = sAES256_URL;
            }
            else if ( nEncAlgID == xml::crypto::CipherID::BLOWFISH_CFB_8 )
            {
                sEncAlgName = sBlowfish_Name;
            }
            else
                throw uno::RuntimeException( THROW_WHERE "Unexpected encryption algorithm is provided!" );

            pNewAttrList->AddAttribute ( sAlgorithmNameAttribute, sCdataAttribute, sEncAlgName );

            *pVector >>= aSequence;
            ::comphelper::Base64::encode(aBuffer, aSequence);
            pNewAttrList->AddAttribute ( sInitialisationVectorAttribute, sCdataAttribute, aBuffer.makeStringAndClear() );

            xHandler->ignorableWhitespace ( sWhiteSpace );
            xHandler->startElement( sAlgorithmElement , pNewAttrList);
            xHandler->ignorableWhitespace ( sWhiteSpace );
            xHandler->endElement( sAlgorithmElement );

            if ( bStoreStartKeyGeneration )
            {
                // ==== Start Key Generation
                pNewAttrList = new ::comphelper::AttributeList;

                OUString sStartKeyAlg;
                OUString sStartKeySize;
                sal_Int32 nStartKeyAlgID = 0;
                *pStartKeyAlg >>= nStartKeyAlgID;
                if ( nStartKeyAlgID == xml::crypto::DigestID::SHA256 )
                {
                    sStartKeyAlg = sSHA256_URL_ODF12; // TODO use SHA256_URL
                    aBuffer.append( sal_Int32(32) );
                    sStartKeySize = aBuffer.makeStringAndClear();
                }
                else if ( nStartKeyAlgID == xml::crypto::DigestID::SHA1 )
                {
                    sStartKeyAlg = sSHA1_Name;
                    aBuffer.append( sal_Int32(20) );
                    sStartKeySize = aBuffer.makeStringAndClear();
                }
                else
                    throw uno::RuntimeException( THROW_WHERE "Unexpected start key algorithm is provided!" );

                pNewAttrList->AddAttribute ( sStartKeyGenerationNameAttribute, sCdataAttribute, sStartKeyAlg );
                pNewAttrList->AddAttribute ( sKeySizeAttribute, sCdataAttribute, sStartKeySize );

                xHandler->ignorableWhitespace ( sWhiteSpace );
                xHandler->startElement( sStartKeyGenerationElement , pNewAttrList);
                xHandler->ignorableWhitespace ( sWhiteSpace );
                xHandler->endElement( sStartKeyGenerationElement );
            }

            // ==== Key Derivation
            pNewAttrList = new ::comphelper::AttributeList;

            if (pKeyInfoProperty)
            {
                pNewAttrList->AddAttribute(sKeyDerivationNameAttribute,
                                           sCdataAttribute,
                                           sPGP_Name);
                // no start-key-generation needed, our session key has
                // max size already
                bStoreStartKeyGeneration = false;
            }
            else
            {
                pNewAttrList->AddAttribute(sKeyDerivationNameAttribute,
                                           sCdataAttribute,
                                           sPBKDF2_Name);

                if (bStoreStartKeyGeneration)
                {
                    aBuffer.append(nDerivedKeySize);
                    pNewAttrList->AddAttribute(sKeySizeAttribute, sCdataAttribute, aBuffer.makeStringAndClear());
                }

                sal_Int32 nCount = 0;
                *pIterationCount >>= nCount;
                aBuffer.append(nCount);
                pNewAttrList->AddAttribute(sIterationCountAttribute, sCdataAttribute, aBuffer.makeStringAndClear());

                *pSalt >>= aSequence;
                ::comphelper::Base64::encode(aBuffer, aSequence);
                pNewAttrList->AddAttribute(sSaltAttribute, sCdataAttribute, aBuffer.makeStringAndClear());
            }

            xHandler->ignorableWhitespace(sWhiteSpace);
            xHandler->startElement(sKeyDerivationElement, pNewAttrList);
            xHandler->ignorableWhitespace(sWhiteSpace);
            xHandler->endElement(sKeyDerivationElement);

            xHandler->ignorableWhitespace ( sWhiteSpace );
            xHandler->endElement( sEncryptionDataElement );
        }
        xHandler->ignorableWhitespace ( sWhiteSpace );
        xHandler->endElement( sFileEntryElement );
    }
    xHandler->ignorableWhitespace ( sWhiteSpace );
    xHandler->endElement( sManifestElement );
    xHandler->endDocument();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
