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
#include <com/sun/star/xml/crypto/DigestID.hpp>
#include <com/sun/star/xml/crypto/CipherID.hpp>
#include <com/sun/star/xml/crypto/KDFID.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>

#include "ManifestDefines.hxx"
#include "ManifestExport.hxx"

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
    static constexpr OUStringLiteral sKeyInfo                     ( u"KeyInfo" );
    static constexpr OUStringLiteral sPgpKeyIDProperty            ( u"KeyId" );
    static constexpr OUStringLiteral sPgpKeyPacketProperty        ( u"KeyPacket" );
    static constexpr OUStringLiteral sCipherValueProperty         ( u"CipherValue" );
    static constexpr OUString sFullPathProperty     ( u"FullPath"_ustr );
    static constexpr OUString sVersionProperty  ( u"Version"_ustr );
    static constexpr OUString sMediaTypeProperty    ( u"MediaType"_ustr );
    static constexpr OUStringLiteral sIterationCountProperty   ( u"IterationCount" );
    static constexpr OUStringLiteral sDerivedKeySizeProperty  ( u"DerivedKeySize" );
    static constexpr OUStringLiteral sSaltProperty         ( u"Salt" );
    static constexpr OUStringLiteral sInitialisationVectorProperty( u"InitialisationVector" );
    static constexpr OUStringLiteral sSizeProperty         ( u"Size" );
    static constexpr OUStringLiteral sDigestProperty       ( u"Digest" );
    static constexpr OUStringLiteral sEncryptionAlgProperty    ( u"EncryptionAlgorithm" );
    static constexpr OUStringLiteral sStartKeyAlgProperty  ( u"StartKeyAlgorithm" );
    static constexpr OUStringLiteral sDigestAlgProperty    ( u"DigestAlgorithm" );

    static constexpr OUString sWhiteSpace           ( u" "_ustr );

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
    bool isWholesomeEncryption(false);
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
            assert(aDocMediaType.isEmpty());
            // unfortunately no aMediaType in some cases where non-documents
            // are stored as StorageFormats::PACKAGE instead of sensible
            // StorageFormats::ZIP, such as SvxXMLXTableExportComponent and
            // SwXMLTextBlocks, which results in an empty "mimetype" etc but
            // can't be easily fixed; try to exclude these cases by checking
            // for aVersion, but of course then forgetting to set both version
            // and type on an actual document can't be found :(
            assert(!aMediaType.isEmpty() || aVersion.isEmpty());
            aDocMediaType = aMediaType;
            aDocVersion = aVersion;
            pRootFolderPropSeq = &rSequence;
        }

        if (aPath == "encrypted-package")
        {
            isWholesomeEncryption = true;
            assert(aDocMediaType.isEmpty() || aDocMediaType == aMediaType);
        }
    }
    assert(pRootFolderPropSeq);

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
                                        MANIFEST_OASIS_NAMESPACE );
            bAcceptNonemptyVersion = true;
            if ( aDocVersion.compareTo( ODFVER_012_TEXT ) >= 0 )
            {
                // this is ODF12 or later generation, let encrypted
                // streams contain start-key-generation entry
                bStoreStartKeyGeneration = true;
                pRootAttrList->AddAttribute ( ATTRIBUTE_VERSION, aDocVersion );
                // plus gpg4libre extensions - loext NS for that
                pRootAttrList->AddAttribute ( ATTRIBUTE_XMLNS_LOEXT,
                                              MANIFEST_LOEXT_NAMESPACE );
            }
        }
        else
        {
            // even if it is no SO6 format the namespace must be specified
            // thus SO6 format is used as default one
            pRootAttrList->AddAttribute ( ATTRIBUTE_XMLNS,
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
    xHandler->startElement( ELEMENT_MANIFEST, pRootAttrList );

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
            // no start-key-generation needed, our session key has
            // max size already
            bStoreStartKeyGeneration = false;

            // yeah, so that goes directly below the manifest:manifest
            // element
            OUStringBuffer aBuffer;

            xHandler->ignorableWhitespace ( sWhiteSpace );

            // ==== manifest:keyinfo & children
            bool const isODF13(aDocVersion.compareTo(ODFVER_013_TEXT) >= 0);
            if (!isODF13)
            {
                xHandler->startElement(ELEMENT_MANIFEST_KEYINFO, nullptr);
            }
            xHandler->ignorableWhitespace ( sWhiteSpace );

            uno::Sequence< uno::Sequence < beans::NamedValue > > aKeyInfoSequence;
            *pKeyInfoProperty >>= aKeyInfoSequence;
            for (const uno::Sequence<beans::NamedValue>& rKeyInfoSequence : aKeyInfoSequence)
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
                    xHandler->startElement(isODF13 ? ELEMENT_ENCRYPTEDKEY13 : ELEMENT_ENCRYPTEDKEY, nullptr);
                    xHandler->ignorableWhitespace ( sWhiteSpace );

                    rtl::Reference<::comphelper::AttributeList> pNewAttrList = new ::comphelper::AttributeList;
                    // TODO: the algorithm should rather be configurable
                    pNewAttrList->AddAttribute(
                        isODF13 ? ATTRIBUTE_ALGORITHM13 : ATTRIBUTE_ALGORITHM,
                                                 u"http://www.w3.org/2001/04/xmlenc#rsa-oaep-mgf1p"_ustr );
                    xHandler->startElement(isODF13 ? ELEMENT_ENCRYPTIONMETHOD13 : ELEMENT_ENCRYPTIONMETHOD, pNewAttrList);
                    xHandler->endElement(isODF13 ? ELEMENT_ENCRYPTIONMETHOD13 : ELEMENT_ENCRYPTIONMETHOD);
                    xHandler->ignorableWhitespace ( sWhiteSpace );

                    // note: the mismatch here corresponds to ODF 1.3 cs01 schema
                    xHandler->startElement(isODF13 ? ELEMENT_MANIFEST13_KEYINFO : ELEMENT_MANIFEST_KEYINFO, nullptr);
                    xHandler->ignorableWhitespace ( sWhiteSpace );

                    xHandler->startElement(isODF13 ? ELEMENT_PGPDATA13 : ELEMENT_PGPDATA, nullptr);
                    xHandler->ignorableWhitespace ( sWhiteSpace );

                    xHandler->startElement(isODF13 ? ELEMENT_PGPKEYID13 : ELEMENT_PGPKEYID, nullptr);
                    ::comphelper::Base64::encode(aBuffer, aPgpKeyID);
                    xHandler->characters( aBuffer.makeStringAndClear() );
                    xHandler->endElement(isODF13 ? ELEMENT_PGPKEYID13 : ELEMENT_PGPKEYID);
                    xHandler->ignorableWhitespace ( sWhiteSpace );

                    // key packet is optional
                    if (aPgpKeyPacket.hasElements())
                    {
                        xHandler->startElement(isODF13 ? ELEMENT_PGPKEYPACKET13 : ELEMENT_PGPKEYPACKET, nullptr);
                        ::comphelper::Base64::encode(aBuffer, aPgpKeyPacket);
                        xHandler->characters( aBuffer.makeStringAndClear() );
                        xHandler->endElement(isODF13 ? ELEMENT_PGPKEYPACKET13 : ELEMENT_PGPKEYPACKET);
                        xHandler->ignorableWhitespace ( sWhiteSpace );
                    }

                    xHandler->endElement(isODF13 ? ELEMENT_PGPDATA13 : ELEMENT_PGPDATA);
                    xHandler->ignorableWhitespace ( sWhiteSpace );

                    xHandler->endElement(isODF13 ? ELEMENT_MANIFEST13_KEYINFO : ELEMENT_MANIFEST_KEYINFO);
                    xHandler->ignorableWhitespace ( sWhiteSpace );

                    xHandler->startElement(isODF13 ? ELEMENT_CIPHERDATA13 : ELEMENT_CIPHERDATA, nullptr);
                    xHandler->ignorableWhitespace ( sWhiteSpace );

                    xHandler->startElement(isODF13 ? ELEMENT_CIPHERVALUE13 : ELEMENT_CIPHERVALUE, nullptr);
                    ::comphelper::Base64::encode(aBuffer, aCipherValue);
                    xHandler->characters( aBuffer.makeStringAndClear() );
                    xHandler->endElement(isODF13 ? ELEMENT_CIPHERVALUE13 : ELEMENT_CIPHERVALUE);
                    xHandler->ignorableWhitespace ( sWhiteSpace );

                    xHandler->endElement(isODF13 ? ELEMENT_CIPHERDATA13 : ELEMENT_CIPHERDATA);
                    xHandler->ignorableWhitespace ( sWhiteSpace );

                    xHandler->endElement(isODF13 ? ELEMENT_ENCRYPTEDKEY13 : ELEMENT_ENCRYPTEDKEY);
                    xHandler->ignorableWhitespace ( sWhiteSpace );
                }
            }

            if (!isODF13)
            {
                xHandler->endElement(ELEMENT_MANIFEST_KEYINFO);
            }
            xHandler->ignorableWhitespace ( sWhiteSpace );
        }
    }

    // now write individual file entries
    for (const uno::Sequence<beans::PropertyValue>& rSequence : rManList)
    {
        if (&rSequence == pRootFolderPropSeq && isWholesomeEncryption)
        {
            continue; // no root document, but embedded package => omit
        }
        rtl::Reference<::comphelper::AttributeList> pAttrList = new ::comphelper::AttributeList;
        OUString fullPath;
        OUString aString;
        const uno::Any *pVector = nullptr, *pSalt = nullptr, *pIterationCount = nullptr, *pDigest = nullptr, *pDigestAlg = nullptr, *pEncryptAlg = nullptr, *pStartKeyAlg = nullptr, *pDerivedKeySize = nullptr;
        uno::Any const* pKDF = nullptr;
        uno::Any const* pArgon2Args = nullptr;
        for (const beans::PropertyValue& rValue : rSequence)
        {
            if (rValue.Name == sMediaTypeProperty )
            {
                rValue.Value >>= aString;
                pAttrList->AddAttribute ( ATTRIBUTE_MEDIA_TYPE, aString );
            }
            else if (rValue.Name == sVersionProperty )
            {
                rValue.Value >>= aString;
                // the version is stored only if it is not empty
                if ( bAcceptNonemptyVersion && !aString.isEmpty() )
                    pAttrList->AddAttribute ( ATTRIBUTE_VERSION, aString );
            }
            else if (rValue.Name == sFullPathProperty )
            {
                rValue.Value >>= fullPath;
                pAttrList->AddAttribute(ATTRIBUTE_FULL_PATH, fullPath);
            }
            else if (rValue.Name == sSizeProperty )
            {
                sal_Int64 nSize = 0;
                rValue.Value >>= nSize;
                pAttrList->AddAttribute ( ATTRIBUTE_SIZE, OUString::number( nSize ) );
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
            else if (rValue.Name == "KeyDerivationFunction") {
                pKDF = &rValue.Value;
            } else if (rValue.Name == "Argon2Args") {
                pArgon2Args = &rValue.Value;
            }
        }
        assert(!fullPath.isEmpty());
        if (isWholesomeEncryption)
        {   // there may be signatures in META-INF too
            assert(fullPath == "encrypted-package" || fullPath.startsWith("META-INF/"));
        }

        xHandler->ignorableWhitespace ( sWhiteSpace );
        xHandler->startElement( ELEMENT_FILE_ENTRY , pAttrList);
        if (pVector && pEncryptAlg && pDerivedKeySize && pKDF
            && ((pSalt && pStartKeyAlg && (pIterationCount || pArgon2Args))
                || pKeyInfoProperty))
        {
            // ==== Encryption Data
            rtl::Reference<::comphelper::AttributeList> pNewAttrList = new ::comphelper::AttributeList;
            OUStringBuffer aBuffer;
            uno::Sequence < sal_Int8 > aSequence;

            xHandler->ignorableWhitespace ( sWhiteSpace );

            // ==== Digest
            if (pDigest && pDigestAlg && pDigestAlg->hasValue())
            {
                OUString sChecksumType;
                sal_Int32 nDigestAlgID = 0;
                *pDigestAlg >>= nDigestAlgID;
                if ( nDigestAlgID == xml::crypto::DigestID::SHA256_1K )
                    sChecksumType = sSHA256_1k_URL;
                else if ( nDigestAlgID == xml::crypto::DigestID::SHA1_1K )
                    sChecksumType = sSHA1_1k_Name;
                else
                    throw uno::RuntimeException( THROW_WHERE "Unexpected digest algorithm is provided!" );

                pNewAttrList->AddAttribute(ATTRIBUTE_CHECKSUM_TYPE, sChecksumType);
                *pDigest >>= aSequence;
                ::comphelper::Base64::encode(aBuffer, aSequence);
                pNewAttrList->AddAttribute(ATTRIBUTE_CHECKSUM, aBuffer.makeStringAndClear());
            }

            xHandler->startElement( ELEMENT_ENCRYPTION_DATA , pNewAttrList);

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
            else if (nEncAlgID == xml::crypto::CipherID::AES_GCM_W3C)
            {
                assert(bStoreStartKeyGeneration || pKeyInfoProperty);
                SAL_WARN_IF(nDerivedKeySize != 32, "package.manifest", "Unexpected key size is provided!");
                if (nDerivedKeySize != 32)
                {
                    throw uno::RuntimeException(THROW_WHERE "Unexpected key size is provided!");
                }
                sEncAlgName = AESGCM256_URL;
            }
            else if ( nEncAlgID == xml::crypto::CipherID::BLOWFISH_CFB_8 )
            {
                sEncAlgName = sBlowfish_Name;
            }
            else
                throw uno::RuntimeException( THROW_WHERE "Unexpected encryption algorithm is provided!" );

            pNewAttrList->AddAttribute ( ATTRIBUTE_ALGORITHM_NAME, sEncAlgName );

            *pVector >>= aSequence;
            ::comphelper::Base64::encode(aBuffer, aSequence);
            pNewAttrList->AddAttribute ( ATTRIBUTE_INITIALISATION_VECTOR, aBuffer.makeStringAndClear() );

            xHandler->ignorableWhitespace ( sWhiteSpace );
            xHandler->startElement( ELEMENT_ALGORITHM , pNewAttrList);
            xHandler->ignorableWhitespace ( sWhiteSpace );
            xHandler->endElement( ELEMENT_ALGORITHM );

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
                    if (nEncAlgID == xml::crypto::CipherID::AES_GCM_W3C)
                    {   // new encryption is incompatible anyway, use W3C URL
                        sStartKeyAlg = SHA256_URL;
                    }
                    else // to interop with ODF <= 1.4 consumers use bad ODF URL
                    {
                        sStartKeyAlg = sSHA256_URL_ODF12;
                    }
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

                pNewAttrList->AddAttribute ( ATTRIBUTE_START_KEY_GENERATION_NAME, sStartKeyAlg );
                pNewAttrList->AddAttribute ( ATTRIBUTE_KEY_SIZE, sStartKeySize );

                xHandler->ignorableWhitespace ( sWhiteSpace );
                xHandler->startElement( ELEMENT_START_KEY_GENERATION , pNewAttrList);
                xHandler->ignorableWhitespace ( sWhiteSpace );
                xHandler->endElement( ELEMENT_START_KEY_GENERATION );
            }

            // ==== Key Derivation
            pNewAttrList = new ::comphelper::AttributeList;

            if (pKeyInfoProperty)
            {
                assert(pKDF->get<sal_Int32>() == xml::crypto::KDFID::PGP_RSA_OAEP_MGF1P);
                pNewAttrList->AddAttribute(ATTRIBUTE_KEY_DERIVATION_NAME,
                                           sPGP_Name);
            }
            else
            {
                if (pKDF->get<sal_Int32>() == xml::crypto::KDFID::Argon2id)
                {
                    pNewAttrList->AddAttribute(ATTRIBUTE_KEY_DERIVATION_NAME,
                                               ARGON2ID_URL_LO);

                    uno::Sequence<sal_Int32> args;
                    *pArgon2Args >>= args;
                    assert(args.getLength() == 3);
                    pNewAttrList->AddAttribute(ATTRIBUTE_ARGON2_T_LO, OUString::number(args[0]));
                    pNewAttrList->AddAttribute(ATTRIBUTE_ARGON2_M_LO, OUString::number(args[1]));
                    pNewAttrList->AddAttribute(ATTRIBUTE_ARGON2_P_LO, OUString::number(args[2]));
                }
                else
                {
                    assert(pKDF->get<sal_Int32>() == xml::crypto::KDFID::PBKDF2);
                    pNewAttrList->AddAttribute(ATTRIBUTE_KEY_DERIVATION_NAME,
                                               sPBKDF2_Name);

                    sal_Int32 nCount = 0;
                    *pIterationCount >>= nCount;
                    aBuffer.append(nCount);
                    pNewAttrList->AddAttribute(ATTRIBUTE_ITERATION_COUNT, aBuffer.makeStringAndClear());
                }

                *pSalt >>= aSequence;
                ::comphelper::Base64::encode(aBuffer, aSequence);
                pNewAttrList->AddAttribute ( ATTRIBUTE_SALT, aBuffer.makeStringAndClear() );
            }

            // ODF 1.3 specifies the default as 16 so have to write it for PGP
            if (bStoreStartKeyGeneration || pKeyInfoProperty)
            {
                aBuffer.append(nDerivedKeySize);
                pNewAttrList->AddAttribute(ATTRIBUTE_KEY_SIZE, aBuffer.makeStringAndClear());
            }

            xHandler->ignorableWhitespace(sWhiteSpace);
            xHandler->startElement(ELEMENT_KEY_DERIVATION, pNewAttrList);
            xHandler->ignorableWhitespace(sWhiteSpace);
            xHandler->endElement(ELEMENT_KEY_DERIVATION);

            xHandler->ignorableWhitespace ( sWhiteSpace );
            xHandler->endElement( ELEMENT_ENCRYPTION_DATA );
        }
        xHandler->ignorableWhitespace ( sWhiteSpace );
        xHandler->endElement( ELEMENT_FILE_ENTRY );
    }
    xHandler->ignorableWhitespace ( sWhiteSpace );
    xHandler->endElement( ELEMENT_MANIFEST );
    xHandler->endDocument();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
