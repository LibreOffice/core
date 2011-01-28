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
#include <ManifestExport.hxx>
#include <ManifestDefines.hxx>
#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HXX
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif
#include <rtl/ustrbuf.hxx>
#ifndef _BASE64_CODEC_HXX_
#include <Base64Codec.hxx>
#endif
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HXX
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_BEANS_PROPERTYVALUE_HPP
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#include <comphelper/documentconstants.hxx>
#include <comphelper/attributelist.hxx>

using namespace rtl;
using namespace com::sun::star::beans;
using namespace com::sun::star::uno;
using namespace com::sun::star::xml::sax;

ManifestExport::ManifestExport(Reference < XDocumentHandler > xHandler,  const Sequence < Sequence < PropertyValue > > &rManList )
{
    const OUString sFileEntryElement    ( RTL_CONSTASCII_USTRINGPARAM ( ELEMENT_FILE_ENTRY ) );
    const OUString sManifestElement     ( RTL_CONSTASCII_USTRINGPARAM ( ELEMENT_MANIFEST ) );
    const OUString sEncryptionDataElement( RTL_CONSTASCII_USTRINGPARAM ( ELEMENT_ENCRYPTION_DATA ) );
    const OUString sAlgorithmElement    ( RTL_CONSTASCII_USTRINGPARAM ( ELEMENT_ALGORITHM ) );
    const OUString sStartKeyGenerationElement ( RTL_CONSTASCII_USTRINGPARAM ( ELEMENT_START_KEY_GENERATION ) );
    const OUString sKeyDerivationElement( RTL_CONSTASCII_USTRINGPARAM ( ELEMENT_KEY_DERIVATION ) );

    const OUString sCdataAttribute      ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_CDATA ) );
    const OUString sMediaTypeAttribute  ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_MEDIA_TYPE ) );
    const OUString sVersionAttribute    ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_VERSION ) );
    const OUString sFullPathAttribute   ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_FULL_PATH ) );
    const OUString sSizeAttribute       ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_SIZE ) );
    const OUString sKeySizeAttribute    ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_KEY_SIZE ) );
    const OUString sSaltAttribute       ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_SALT ) );
    const OUString sInitialisationVectorAttribute ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_INITIALISATION_VECTOR ) );
    const OUString sIterationCountAttribute ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_ITERATION_COUNT ) );
    const OUString sAlgorithmNameAttribute  ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_ALGORITHM_NAME ) );
    const OUString sStartKeyGenerationNameAttribute ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_START_KEY_GENERATION_NAME ) );
    const OUString sKeyDerivationNameAttribute  ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_KEY_DERIVATION_NAME ) );
    const OUString sChecksumTypeAttribute   ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_CHECKSUM_TYPE ) );
    const OUString sChecksumAttribute   ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_CHECKSUM) );

    const OUString sFullPathProperty    ( RTL_CONSTASCII_USTRINGPARAM ( "FullPath" ) );
    const OUString sVersionProperty     ( RTL_CONSTASCII_USTRINGPARAM ( "Version" ) );
    const OUString sMediaTypeProperty   ( RTL_CONSTASCII_USTRINGPARAM ( "MediaType" ) );
    const OUString sIterationCountProperty  ( RTL_CONSTASCII_USTRINGPARAM ( "IterationCount" ) );
    const OUString sSaltProperty        ( RTL_CONSTASCII_USTRINGPARAM ( "Salt" ) );
    const OUString sInitialisationVectorProperty( RTL_CONSTASCII_USTRINGPARAM ( "InitialisationVector" ) );
    const OUString sSizeProperty        ( RTL_CONSTASCII_USTRINGPARAM ( "Size" ) );
    const OUString sDigestProperty      ( RTL_CONSTASCII_USTRINGPARAM ( "Digest" ) );

    const OUString sWhiteSpace          ( RTL_CONSTASCII_USTRINGPARAM ( " " ) );
    const OUString sBlowfish            ( RTL_CONSTASCII_USTRINGPARAM ( "Blowfish CFB" ) );
    const OUString sPBKDF2              ( RTL_CONSTASCII_USTRINGPARAM ( "PBKDF2" ) );
    const OUString sChecksumType        ( RTL_CONSTASCII_USTRINGPARAM ( CHECKSUM_TYPE ) );
    const OUString sStartKeySize        ( RTL_CONSTASCII_USTRINGPARAM ( START_KEY_SIZE ) );
    const OUString sDerivedKeySize      ( RTL_CONSTASCII_USTRINGPARAM ( DERIVED_KEY_SIZE ) );
    const OUString sSHA1                ( RTL_CONSTASCII_USTRINGPARAM ( ALGORITHM_SHA1 ) );

    ::comphelper::AttributeList * pRootAttrList = new ::comphelper::AttributeList;
    const Sequence < PropertyValue > *pSequence = rManList.getConstArray();
    const sal_uInt32 nManLength = rManList.getLength();

    // find the mediatype of the document if any
    OUString aDocMediaType;
    OUString aDocVersion;
    for (sal_uInt32 nInd = 0; nInd < nManLength ; nInd++ )
    {
        OUString aMediaType;
        OUString aPath;
        OUString aVersion;

        const PropertyValue *pValue = pSequence[nInd].getConstArray();
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

        if ( aPath.equals( OUString( RTL_CONSTASCII_USTRINGPARAM( "/" ) ) ) )
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
        if ( aDocMediaType.equals( OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_TEXT_ASCII ) ) )
          || aDocMediaType.equals( OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_TEXT_WEB_ASCII ) ) )
          || aDocMediaType.equals( OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_TEXT_GLOBAL_ASCII ) ) )
          || aDocMediaType.equals( OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_DRAWING_ASCII ) ) )
          || aDocMediaType.equals( OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_ASCII ) ) )
          || aDocMediaType.equals( OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET_ASCII ) ) )
          || aDocMediaType.equals( OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_CHART_ASCII ) ) )
          || aDocMediaType.equals( OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_DATABASE_ASCII ) ) )
          || aDocMediaType.equals( OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_FORMULA_ASCII ) ) )

          || aDocMediaType.equals( OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_TEXT_TEMPLATE_ASCII ) ) )
          || aDocMediaType.equals( OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_DRAWING_TEMPLATE_ASCII ) ) )
          || aDocMediaType.equals( OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_TEMPLATE_ASCII ) ) )
          || aDocMediaType.equals( OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET_TEMPLATE_ASCII ) ) )
          || aDocMediaType.equals( OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_CHART_TEMPLATE_ASCII ) ) )
          || aDocMediaType.equals( OUString( RTL_CONSTASCII_USTRINGPARAM( MIMETYPE_OASIS_OPENDOCUMENT_FORMULA_TEMPLATE_ASCII ) ) ) )

        {
            // oasis format
            pRootAttrList->AddAttribute ( OUString( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_XMLNS ) ),
                                        sCdataAttribute,
                                        OUString( RTL_CONSTASCII_USTRINGPARAM ( MANIFEST_OASIS_NAMESPACE ) ) );
            bAcceptNonemptyVersion = sal_True;
            if ( aDocVersion.compareTo( ODFVER_012_TEXT ) >= 0 )
            {
                // this is ODF12 generation, let encrypted streams contain start-key-generation entry
                bStoreStartKeyGeneration = sal_True;

                // starting from ODF12 the version should be also in manifest:manifest element
                pRootAttrList->AddAttribute ( OUString( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_VERSION ) ),
                                            sCdataAttribute,
                                            aDocVersion );
            }
        }
        else
        {
            // even if it is no SO6 format the namespace must be specified
            // thus SO6 format is used as default one
            pRootAttrList->AddAttribute ( OUString( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_XMLNS ) ),
                                        sCdataAttribute,
                                        OUString( RTL_CONSTASCII_USTRINGPARAM ( MANIFEST_NAMESPACE ) ) );

            bProvideDTD = sal_True;
        }
    }

    Reference < XAttributeList > xRootAttrList (pRootAttrList);

    xHandler->startDocument();
    Reference < XExtendedDocumentHandler > xExtHandler ( xHandler, UNO_QUERY );
    if ( xExtHandler.is() && bProvideDTD )
    {
        OUString aDocType ( RTL_CONSTASCII_USTRINGPARAM ( MANIFEST_DOCTYPE ) );
        xExtHandler->unknown ( aDocType );
        xHandler->ignorableWhitespace ( sWhiteSpace );
    }
    xHandler->startElement( sManifestElement, xRootAttrList );

    for (sal_uInt32 i = 0 ; i < nManLength ; i++)
    {
        ::comphelper::AttributeList *pAttrList = new ::comphelper::AttributeList;
        const PropertyValue *pValue = pSequence[i].getConstArray();
        OUString aString;
        const PropertyValue *pVector = NULL, *pSalt = NULL, *pIterationCount = NULL, *pDigest = NULL;
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
                OUStringBuffer aBuffer;
                aBuffer.append ( nSize );
                pAttrList->AddAttribute ( sSizeAttribute, sCdataAttribute, aBuffer.makeStringAndClear() );
            }
            else if (pValue->Name.equals (sInitialisationVectorProperty) )
                pVector = pValue;
            else if (pValue->Name.equals (sSaltProperty) )
                pSalt = pValue;
            else if (pValue->Name.equals (sIterationCountProperty) )
                pIterationCount = pValue;
            else if (pValue->Name.equals ( sDigestProperty ) )
                pDigest = pValue;
        }
        xHandler->ignorableWhitespace ( sWhiteSpace );
        Reference < XAttributeList > xAttrList ( pAttrList );
        xHandler->startElement( sFileEntryElement , xAttrList);
        if ( pVector && pSalt && pIterationCount )
        {
            // ==== Encryption Data
            ::comphelper::AttributeList * pNewAttrList = new ::comphelper::AttributeList;
            Reference < XAttributeList > xNewAttrList (pNewAttrList);
            OUStringBuffer aBuffer;
            Sequence < sal_uInt8 > aSequence;

            xHandler->ignorableWhitespace ( sWhiteSpace );
            if ( pDigest )
            {
                pNewAttrList->AddAttribute ( sChecksumTypeAttribute, sCdataAttribute, sChecksumType );
                pDigest->Value >>= aSequence;
                Base64Codec::encodeBase64 ( aBuffer, aSequence );
                pNewAttrList->AddAttribute ( sChecksumAttribute, sCdataAttribute, aBuffer.makeStringAndClear() );
            }
            xHandler->startElement( sEncryptionDataElement , xNewAttrList);

            // ==== Algorithm
            pNewAttrList = new ::comphelper::AttributeList;
            xNewAttrList = pNewAttrList;

            pNewAttrList->AddAttribute ( sAlgorithmNameAttribute, sCdataAttribute, sBlowfish );

            pVector->Value >>= aSequence;
            Base64Codec::encodeBase64 ( aBuffer, aSequence );
            pNewAttrList->AddAttribute ( sInitialisationVectorAttribute, sCdataAttribute, aBuffer.makeStringAndClear() );

            xHandler->ignorableWhitespace ( sWhiteSpace );
            xHandler->startElement( sAlgorithmElement , xNewAttrList);
            xHandler->ignorableWhitespace ( sWhiteSpace );
            xHandler->endElement( sAlgorithmElement );

            // ==== Key Derivation
            pNewAttrList = new ::comphelper::AttributeList;
            xNewAttrList = pNewAttrList;

            pNewAttrList->AddAttribute ( sKeyDerivationNameAttribute, sCdataAttribute, sPBKDF2 );

            if ( bStoreStartKeyGeneration )
                pNewAttrList->AddAttribute ( sKeySizeAttribute, sCdataAttribute, sDerivedKeySize );

            sal_Int32 nCount = 0;
            pIterationCount->Value >>= nCount;
            aBuffer.append (nCount);
            pNewAttrList->AddAttribute ( sIterationCountAttribute, sCdataAttribute, aBuffer.makeStringAndClear() );

            pSalt->Value >>= aSequence;
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

                // currently SHA1 is used to generate 20-bytes start key
                pNewAttrList->AddAttribute ( sStartKeyGenerationNameAttribute, sCdataAttribute, sSHA1 );
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
