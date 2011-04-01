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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_package.hxx"

#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/xml/crypto/DigestID.hpp>
#include <com/sun/star/xml/crypto/CipherID.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>

#include <ManifestDefines.hxx>
#include <ManifestExport.hxx>
#include <Base64Codec.hxx>

#include <rtl/ustrbuf.hxx>
#include <comphelper/documentconstants.hxx>
#include <comphelper/attributelist.hxx>

using namespace ::com::sun::star;

ManifestExport::ManifestExport( uno::Reference< xml::sax::XDocumentHandler > xHandler,  const uno::Sequence< uno::Sequence < beans::PropertyValue > >& rManList )
{
    const ::rtl::OUString sFileEntryElement     ( RTL_CONSTASCII_USTRINGPARAM ( ELEMENT_FILE_ENTRY ) );
    const ::rtl::OUString sManifestElement      ( RTL_CONSTASCII_USTRINGPARAM ( ELEMENT_MANIFEST ) );
    const ::rtl::OUString sEncryptionDataElement( RTL_CONSTASCII_USTRINGPARAM ( ELEMENT_ENCRYPTION_DATA ) );
    const ::rtl::OUString sAlgorithmElement     ( RTL_CONSTASCII_USTRINGPARAM ( ELEMENT_ALGORITHM ) );
    const ::rtl::OUString sStartKeyGenerationElement ( RTL_CONSTASCII_USTRINGPARAM ( ELEMENT_START_KEY_GENERATION ) );
    const ::rtl::OUString sKeyDerivationElement ( RTL_CONSTASCII_USTRINGPARAM ( ELEMENT_KEY_DERIVATION ) );

    const ::rtl::OUString sCdataAttribute       ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_CDATA ) );
    const ::rtl::OUString sMediaTypeAttribute   ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_MEDIA_TYPE ) );
    const ::rtl::OUString sVersionAttribute     ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_VERSION ) );
    const ::rtl::OUString sFullPathAttribute    ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_FULL_PATH ) );
    const ::rtl::OUString sSizeAttribute        ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_SIZE ) );
    const ::rtl::OUString sKeySizeAttribute     ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_KEY_SIZE ) );
    const ::rtl::OUString sSaltAttribute        ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_SALT ) );
    const ::rtl::OUString sInitialisationVectorAttribute ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_INITIALISATION_VECTOR ) );
    const ::rtl::OUString sIterationCountAttribute  ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_ITERATION_COUNT ) );
    const ::rtl::OUString sAlgorithmNameAttribute   ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_ALGORITHM_NAME ) );
    const ::rtl::OUString sStartKeyGenerationNameAttribute ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_START_KEY_GENERATION_NAME ) );
    const ::rtl::OUString sKeyDerivationNameAttribute   ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_KEY_DERIVATION_NAME ) );
    const ::rtl::OUString sChecksumTypeAttribute    ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_CHECKSUM_TYPE ) );
    const ::rtl::OUString sChecksumAttribute    ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_CHECKSUM) );

    const ::rtl::OUString sFullPathProperty     ( RTL_CONSTASCII_USTRINGPARAM ( "FullPath" ) );
    const ::rtl::OUString sVersionProperty  ( RTL_CONSTASCII_USTRINGPARAM ( "Version" ) );
    const ::rtl::OUString sMediaTypeProperty    ( RTL_CONSTASCII_USTRINGPARAM ( "MediaType" ) );
    const ::rtl::OUString sIterationCountProperty   ( RTL_CONSTASCII_USTRINGPARAM ( "IterationCount" ) );
    const ::rtl::OUString  sDerivedKeySizeProperty  ( RTL_CONSTASCII_USTRINGPARAM ( "DerivedKeySize" ) );
    const ::rtl::OUString sSaltProperty         ( RTL_CONSTASCII_USTRINGPARAM ( "Salt" ) );
    const ::rtl::OUString sInitialisationVectorProperty( RTL_CONSTASCII_USTRINGPARAM ( "InitialisationVector" ) );
    const ::rtl::OUString sSizeProperty         ( RTL_CONSTASCII_USTRINGPARAM ( "Size" ) );
    const ::rtl::OUString sDigestProperty       ( RTL_CONSTASCII_USTRINGPARAM ( "Digest" ) );
    const ::rtl::OUString sEncryptionAlgProperty    ( RTL_CONSTASCII_USTRINGPARAM ( "EncryptionAlgorithm" ) );
    const ::rtl::OUString sStartKeyAlgProperty  ( RTL_CONSTASCII_USTRINGPARAM ( "StartKeyAlgorithm" ) );
    const ::rtl::OUString sDigestAlgProperty    ( RTL_CONSTASCII_USTRINGPARAM ( "DigestAlgorithm" ) );

    const ::rtl::OUString sWhiteSpace           ( RTL_CONSTASCII_USTRINGPARAM ( " " ) );

    const ::rtl::OUString sSHA256_URL           ( RTL_CONSTASCII_USTRINGPARAM ( SHA256_URL ) );
    const ::rtl::OUString  sSHA1_Name           ( RTL_CONSTASCII_USTRINGPARAM ( SHA1_NAME ) );

    const ::rtl::OUString  sSHA1_1k_Name        ( RTL_CONSTASCII_USTRINGPARAM ( SHA1_1K_NAME ) );
    const ::rtl::OUString  sSHA256_1k_URL       ( RTL_CONSTASCII_USTRINGPARAM ( SHA256_1K_URL ) );

    const ::rtl::OUString  sBlowfish_Name       ( RTL_CONSTASCII_USTRINGPARAM ( BLOWFISH_NAME ) );
    const ::rtl::OUString  sAES256_URL          ( RTL_CONSTASCII_USTRINGPARAM ( AES256_URL ) );

    const ::rtl::OUString  sPBKDF2_Name         ( RTL_CONSTASCII_USTRINGPARAM ( PBKDF2_NAME ) );

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

            if ( aPath.getLength() && aMediaType.getLength() && aVersion.getLength() )
                break;
        }

        if ( aPath.equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/" ) ) ) )
        {
            aDocMediaType = aMediaType;
            aDocVersion = aVersion;
            break;
        }
    }

    sal_Bool bProvideDTD = sal_False;
    sal_Bool bAcceptNonemptyVersion = sal_False;
    sal_Bool bStoreStartKeyGeneration = sal_False;
    if ( aDocMediaType.getLength() )
    {
        if ( aDocMediaType.equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_TEXT_ASCII ) ) )
          || aDocMediaType.equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_TEXT_WEB_ASCII ) ) )
          || aDocMediaType.equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_TEXT_GLOBAL_ASCII ) ) )
          || aDocMediaType.equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_DRAWING_ASCII ) ) )
          || aDocMediaType.equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_ASCII ) ) )
          || aDocMediaType.equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET_ASCII ) ) )
          || aDocMediaType.equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_CHART_ASCII ) ) )
          || aDocMediaType.equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_DATABASE_ASCII ) ) )
          || aDocMediaType.equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_FORMULA_ASCII ) ) )

          || aDocMediaType.equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_TEXT_TEMPLATE_ASCII ) ) )
          || aDocMediaType.equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_DRAWING_TEMPLATE_ASCII ) ) )
          || aDocMediaType.equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_TEMPLATE_ASCII ) ) )
          || aDocMediaType.equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET_TEMPLATE_ASCII ) ) )
          || aDocMediaType.equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_CHART_TEMPLATE_ASCII ) ) )
          || aDocMediaType.equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_FORMULA_TEMPLATE_ASCII ) ) ) )

        {
            // oasis format
            pRootAttrList->AddAttribute ( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_XMLNS ) ),
                                        sCdataAttribute,
                                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( MANIFEST_OASIS_NAMESPACE ) ) );
            bAcceptNonemptyVersion = sal_True;
            if ( aDocVersion.compareTo( ODFVER_012_TEXT ) >= 0 )
            {
                // this is ODF12 generation, let encrypted streams contain start-key-generation entry
                bStoreStartKeyGeneration = sal_True;

                // starting from ODF12 the version should be also in manifest:manifest element
                pRootAttrList->AddAttribute ( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_VERSION ) ),
                                            sCdataAttribute,
                                            aDocVersion );
            }
        }
        else
        {
            // even if it is no SO6 format the namespace must be specified
            // thus SO6 format is used as default one
            pRootAttrList->AddAttribute ( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_XMLNS ) ),
                                        sCdataAttribute,
                                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( MANIFEST_NAMESPACE ) ) );

            bProvideDTD = sal_True;
        }
    }

    uno::Reference < xml::sax::XAttributeList > xRootAttrList (pRootAttrList);

    xHandler->startDocument();
    uno::Reference < xml::sax::XExtendedDocumentHandler > xExtHandler ( xHandler, uno::UNO_QUERY );
    if ( xExtHandler.is() && bProvideDTD )
    {
        ::rtl::OUString aDocType ( RTL_CONSTASCII_USTRINGPARAM ( MANIFEST_DOCTYPE ) );
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
                if ( bAcceptNonemptyVersion && aString.getLength() )
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
                throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Unexpected digest algorithm is provided!" ) ), uno::Reference< uno::XInterface >() );

            pNewAttrList->AddAttribute ( sChecksumTypeAttribute, sCdataAttribute, sChecksumType );
            *pDigest >>= aSequence;
            Base64Codec::encodeBase64( aBuffer, aSequence );
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
                    throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Unexpected key size is provided!" ) ), uno::Reference< uno::XInterface >() );

                sEncAlgName = sAES256_URL;
            }
            else if ( nEncAlgID == xml::crypto::CipherID::BLOWFISH_CFB_8 )
            {
                sEncAlgName = sBlowfish_Name;
            }
            else
                throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Unexpecte encryption algorithm is provided!" ) ), uno::Reference< uno::XInterface >() );

            pNewAttrList->AddAttribute ( sAlgorithmNameAttribute, sCdataAttribute, sEncAlgName );

            *pVector >>= aSequence;
            Base64Codec::encodeBase64 ( aBuffer, aSequence );
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
            Base64Codec::encodeBase64 ( aBuffer, aSequence );
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
                    throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX "Unexpected start key algorithm is provided!" ) ), uno::Reference< uno::XInterface >() );

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
