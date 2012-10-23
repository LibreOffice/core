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
#include <com/sun/star/uno/RuntimeException.hpp>

#include <ManifestDefines.hxx>
#include <ManifestExport.hxx>
#include <sax/tools/converter.hxx>

#include <rtl/ustrbuf.hxx>
#include <comphelper/documentconstants.hxx>
#include <comphelper/attributelist.hxx>

using namespace ::com::sun::star;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

ManifestExport::ManifestExport( uno::Reference< xml::sax::XDocumentHandler > xHandler,  const uno::Sequence< uno::Sequence < beans::PropertyValue > >& rManList )
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

    const OUString sFullPathProperty     ( "FullPath" );
    const OUString sVersionProperty  ( "Version" );
    const OUString sMediaTypeProperty    ( "MediaType" );
    const OUString sIterationCountProperty   ( "IterationCount" );
    const OUString  sDerivedKeySizeProperty  ( "DerivedKeySize" );
    const OUString sSaltProperty         ( "Salt" );
    const OUString sInitialisationVectorProperty( "InitialisationVector" );
    const OUString sSizeProperty         ( "Size" );
    const OUString sDigestProperty       ( "Digest" );
    const OUString sEncryptionAlgProperty    ( "EncryptionAlgorithm" );
    const OUString sStartKeyAlgProperty  ( "StartKeyAlgorithm" );
    const OUString sDigestAlgProperty    ( "DigestAlgorithm" );

    const OUString sWhiteSpace           ( " " );

    const OUString sSHA256_URL           ( SHA256_URL );
    const OUString  sSHA1_Name           ( SHA1_NAME );

    const OUString  sSHA1_1k_Name        ( SHA1_1K_NAME );
    const OUString  sSHA256_1k_URL       ( SHA256_1K_URL );

    const OUString  sBlowfish_Name       ( BLOWFISH_NAME );
    const OUString  sAES256_URL          ( AES256_URL );

    const OUString  sPBKDF2_Name         ( PBKDF2_NAME );

    ::comphelper::AttributeList * pRootAttrList = new ::comphelper::AttributeList;
    const uno::Sequence < beans::PropertyValue > *pSequence = rManList.getConstArray();
    const sal_uInt32 nManLength = rManList.getLength();

    // find the mediatype of the document if any
    ::rtl::OUString aDocMediaType;
    ::rtl::OUString aDocVersion;
    for (sal_uInt32 nInd = 0; nInd < nManLength ; nInd++ )
    {
        ::rtl::OUString aMediaType;
        ::rtl::OUString aPath;
        ::rtl::OUString aVersion;

        const beans::PropertyValue *pValue = pSequence[nInd].getConstArray();
        for (sal_uInt32 j = 0, nNum = pSequence[nInd].getLength(); j < nNum; j++, pValue++)
        {
            if (pValue->Name.equals (sMediaTypeProperty) )
            {
                pValue->Value >>= aMediaType;
            }
            else if (pValue->Name.equals (sFullPathProperty) )
            {
                pValue->Value >>= aPath;
            }
            else if (pValue->Name.equals (sVersionProperty) )
            {
                pValue->Value >>= aVersion;
            }

            if ( !aPath.isEmpty() && !aMediaType.isEmpty() && !aVersion.isEmpty() )
                break;
        }

        if ( aPath == "/" )
        {
            aDocMediaType = aMediaType;
            aDocVersion = aVersion;
            break;
        }
    }

    sal_Bool bProvideDTD = sal_False;
    sal_Bool bAcceptNonemptyVersion = sal_False;
    sal_Bool bStoreStartKeyGeneration = sal_False;
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
            bAcceptNonemptyVersion = sal_True;
            if ( aDocVersion.compareTo( ODFVER_012_TEXT ) >= 0 )
            {
                // this is ODF12 generation, let encrypted streams contain start-key-generation entry
                bStoreStartKeyGeneration = sal_True;
                pRootAttrList->AddAttribute ( sVersionAttribute, sCdataAttribute, aDocVersion );
            }
        }
        else
        {
            // even if it is no SO6 format the namespace must be specified
            // thus SO6 format is used as default one
            pRootAttrList->AddAttribute ( ATTRIBUTE_XMLNS,
                                        sCdataAttribute,
                                        MANIFEST_NAMESPACE );

            bProvideDTD = sal_True;
        }
    }

    uno::Reference < xml::sax::XAttributeList > xRootAttrList (pRootAttrList);

    xHandler->startDocument();
    uno::Reference < xml::sax::XExtendedDocumentHandler > xExtHandler ( xHandler, uno::UNO_QUERY );
    if ( xExtHandler.is() && bProvideDTD )
    {
        OUString aDocType ( MANIFEST_DOCTYPE );
        xExtHandler->unknown ( aDocType );
        xHandler->ignorableWhitespace ( sWhiteSpace );
    }
    xHandler->startElement( sManifestElement, xRootAttrList );

    for (sal_uInt32 i = 0 ; i < nManLength ; i++)
    {
        ::comphelper::AttributeList *pAttrList = new ::comphelper::AttributeList;
        const beans::PropertyValue *pValue = pSequence[i].getConstArray();
        ::rtl::OUString aString;
        const uno::Any *pVector = NULL, *pSalt = NULL, *pIterationCount = NULL, *pDigest = NULL, *pDigestAlg = NULL, *pEncryptAlg = NULL, *pStartKeyAlg = NULL, *pDerivedKeySize = NULL;
        for (sal_uInt32 j = 0, nNum = pSequence[i].getLength(); j < nNum; j++, pValue++)
        {
            if (pValue->Name.equals (sMediaTypeProperty) )
            {
                pValue->Value >>= aString;
                pAttrList->AddAttribute ( sMediaTypeAttribute, sCdataAttribute, aString );
            }
            else if (pValue->Name.equals (sVersionProperty) )
            {
                pValue->Value >>= aString;
                // the version is stored only if it is not empty
                if ( bAcceptNonemptyVersion && !aString.isEmpty() )
                    pAttrList->AddAttribute ( sVersionAttribute, sCdataAttribute, aString );
            }
            else if (pValue->Name.equals (sFullPathProperty) )
            {
                pValue->Value >>= aString;
                pAttrList->AddAttribute ( sFullPathAttribute, sCdataAttribute, aString );
            }
            else if (pValue->Name.equals (sSizeProperty) )
            {
                sal_Int32 nSize = 0;
                pValue->Value >>= nSize;
                ::rtl::OUStringBuffer aBuffer;
                aBuffer.append ( nSize );
                pAttrList->AddAttribute ( sSizeAttribute, sCdataAttribute, aBuffer.makeStringAndClear() );
            }
            else if (pValue->Name.equals (sInitialisationVectorProperty) )
                pVector = &pValue->Value;
            else if (pValue->Name.equals (sSaltProperty) )
                pSalt = &pValue->Value;
            else if (pValue->Name.equals (sIterationCountProperty) )
                pIterationCount = &pValue->Value;
            else if (pValue->Name.equals ( sDigestProperty ) )
                pDigest = &pValue->Value;
            else if (pValue->Name.equals ( sDigestAlgProperty ) )
                pDigestAlg = &pValue->Value;
            else if (pValue->Name.equals ( sEncryptionAlgProperty ) )
                pEncryptAlg = &pValue->Value;
            else if (pValue->Name.equals ( sStartKeyAlgProperty ) )
                pStartKeyAlg = &pValue->Value;
            else if (pValue->Name.equals ( sDerivedKeySizeProperty ) )
                pDerivedKeySize = &pValue->Value;
        }

        xHandler->ignorableWhitespace ( sWhiteSpace );
        uno::Reference < xml::sax::XAttributeList > xAttrList ( pAttrList );
        xHandler->startElement( sFileEntryElement , xAttrList);
        if ( pVector && pSalt && pIterationCount && pDigest && pDigestAlg && pEncryptAlg && pStartKeyAlg && pDerivedKeySize )
        {
            // ==== Encryption Data
            ::comphelper::AttributeList * pNewAttrList = new ::comphelper::AttributeList;
            uno::Reference < xml::sax::XAttributeList > xNewAttrList (pNewAttrList);
            ::rtl::OUStringBuffer aBuffer;
            uno::Sequence < sal_Int8 > aSequence;

            xHandler->ignorableWhitespace ( sWhiteSpace );

            // ==== Digest
            ::rtl::OUString sChecksumType;
            sal_Int32 nDigestAlgID = 0;
            *pDigestAlg >>= nDigestAlgID;
            if ( nDigestAlgID == xml::crypto::DigestID::SHA256_1K )
                sChecksumType = sSHA256_1k_URL;
            else if ( nDigestAlgID == xml::crypto::DigestID::SHA1_1K )
                sChecksumType = sSHA1_1k_Name;
            else
                throw uno::RuntimeException( OSL_LOG_PREFIX "Unexpected digest algorithm is provided!", uno::Reference< uno::XInterface >() );

            pNewAttrList->AddAttribute ( sChecksumTypeAttribute, sCdataAttribute, sChecksumType );
            *pDigest >>= aSequence;
            ::sax::Converter::encodeBase64(aBuffer, aSequence);
            pNewAttrList->AddAttribute ( sChecksumAttribute, sCdataAttribute, aBuffer.makeStringAndClear() );

            xHandler->startElement( sEncryptionDataElement , xNewAttrList);

            // ==== Algorithm
            pNewAttrList = new ::comphelper::AttributeList;
            xNewAttrList = pNewAttrList;

            sal_Int32 nEncAlgID = 0;
            sal_Int32 nDerivedKeySize = 0;
            *pEncryptAlg >>= nEncAlgID;
            *pDerivedKeySize >>= nDerivedKeySize;

            ::rtl::OUString sEncAlgName;
            if ( nEncAlgID == xml::crypto::CipherID::AES_CBC_W3C_PADDING )
            {
                OSL_ENSURE( nDerivedKeySize, "Unexpected key size is provided!" );
                if ( nDerivedKeySize != 32 )
                    throw uno::RuntimeException( OSL_LOG_PREFIX "Unexpected key size is provided!", uno::Reference< uno::XInterface >() );

                sEncAlgName = sAES256_URL;
            }
            else if ( nEncAlgID == xml::crypto::CipherID::BLOWFISH_CFB_8 )
            {
                sEncAlgName = sBlowfish_Name;
            }
            else
                throw uno::RuntimeException( OSL_LOG_PREFIX "Unexpected encryption algorithm is provided!", uno::Reference< uno::XInterface >() );

            pNewAttrList->AddAttribute ( sAlgorithmNameAttribute, sCdataAttribute, sEncAlgName );

            *pVector >>= aSequence;
            ::sax::Converter::encodeBase64(aBuffer, aSequence);
            pNewAttrList->AddAttribute ( sInitialisationVectorAttribute, sCdataAttribute, aBuffer.makeStringAndClear() );

            xHandler->ignorableWhitespace ( sWhiteSpace );
            xHandler->startElement( sAlgorithmElement , xNewAttrList);
            xHandler->ignorableWhitespace ( sWhiteSpace );
            xHandler->endElement( sAlgorithmElement );

            // ==== Key Derivation
            pNewAttrList = new ::comphelper::AttributeList;
            xNewAttrList = pNewAttrList;

            pNewAttrList->AddAttribute ( sKeyDerivationNameAttribute, sCdataAttribute, sPBKDF2_Name );

            if ( bStoreStartKeyGeneration )
            {
                aBuffer.append( nDerivedKeySize );
                pNewAttrList->AddAttribute ( sKeySizeAttribute, sCdataAttribute, aBuffer.makeStringAndClear() );
            }

            sal_Int32 nCount = 0;
            *pIterationCount >>= nCount;
            aBuffer.append (nCount);
            pNewAttrList->AddAttribute ( sIterationCountAttribute, sCdataAttribute, aBuffer.makeStringAndClear() );

            *pSalt >>= aSequence;
            ::sax::Converter::encodeBase64(aBuffer, aSequence);
            pNewAttrList->AddAttribute ( sSaltAttribute, sCdataAttribute, aBuffer.makeStringAndClear() );

            xHandler->ignorableWhitespace ( sWhiteSpace );
            xHandler->startElement( sKeyDerivationElement , xNewAttrList);
            xHandler->ignorableWhitespace ( sWhiteSpace );
            xHandler->endElement( sKeyDerivationElement );

            // we have to store start-key-generation element as the last one to workaround the parsing problem
            // in OOo3.1 and older versions
            if ( bStoreStartKeyGeneration )
            {
                // ==== Start Key Generation
                pNewAttrList = new ::comphelper::AttributeList;
                xNewAttrList = pNewAttrList;

                ::rtl::OUString sStartKeyAlg;
                ::rtl::OUString sStartKeySize;
                sal_Int32 nStartKeyAlgID = 0;
                *pStartKeyAlg >>= nStartKeyAlgID;
                if ( nStartKeyAlgID == xml::crypto::DigestID::SHA256 )
                {
                    sStartKeyAlg = sSHA256_URL;
                    aBuffer.append( (sal_Int32)32 );
                    sStartKeySize = aBuffer.makeStringAndClear();
                }
                else if ( nStartKeyAlgID == xml::crypto::DigestID::SHA1 )
                {
                    sStartKeyAlg = sSHA1_Name;
                    aBuffer.append( (sal_Int32)20 );
                    sStartKeySize = aBuffer.makeStringAndClear();
                }
                else
                    throw uno::RuntimeException( OSL_LOG_PREFIX "Unexpected start key algorithm is provided!", uno::Reference< uno::XInterface >() );

                pNewAttrList->AddAttribute ( sStartKeyGenerationNameAttribute, sCdataAttribute, sStartKeyAlg );
                pNewAttrList->AddAttribute ( sKeySizeAttribute, sCdataAttribute, sStartKeySize );

                xHandler->ignorableWhitespace ( sWhiteSpace );
                xHandler->startElement( sStartKeyGenerationElement , xNewAttrList);
                xHandler->ignorableWhitespace ( sWhiteSpace );
                xHandler->endElement( sStartKeyGenerationElement );
            }

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
