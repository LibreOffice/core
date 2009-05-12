/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ManifestImport.cxx,v $
 * $Revision: 1.12 $
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
#include <ManifestImport.hxx>
#include <ManifestDefines.hxx>
#ifndef _BASE64_CODEC_HXX_
#include <Base64Codec.hxx>
#endif
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star;
using namespace rtl;
using namespace std;

ManifestImport::ManifestImport( vector < Sequence < PropertyValue > > & rNewManVector )
: nNumProperty (0)
, bIgnoreEncryptData    ( sal_False )
, rManVector ( rNewManVector )

, sFileEntryElement     ( RTL_CONSTASCII_USTRINGPARAM ( ELEMENT_FILE_ENTRY ) )
, sManifestElement      ( RTL_CONSTASCII_USTRINGPARAM ( ELEMENT_MANIFEST ) )
, sEncryptionDataElement( RTL_CONSTASCII_USTRINGPARAM ( ELEMENT_ENCRYPTION_DATA ) )
, sAlgorithmElement ( RTL_CONSTASCII_USTRINGPARAM ( ELEMENT_ALGORITHM ) )
, sKeyDerivationElement( RTL_CONSTASCII_USTRINGPARAM ( ELEMENT_KEY_DERIVATION ) )

, sCdataAttribute               ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_CDATA ) )
, sMediaTypeAttribute           ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_MEDIA_TYPE ) )
, sVersionAttribute             ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_VERSION ) )
, sFullPathAttribute            ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_FULL_PATH ) )
, sSizeAttribute                ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_SIZE ) )
, sSaltAttribute                ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_SALT ) )
, sInitialisationVectorAttribute ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_INITIALISATION_VECTOR ) )
, sIterationCountAttribute      ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_ITERATION_COUNT ) )
, sAlgorithmNameAttribute       ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_ALGORITHM_NAME ) )
, sKeyDerivationNameAttribute   ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_KEY_DERIVATION_NAME ) )
, sChecksumAttribute            ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_CHECKSUM ) )
, sChecksumTypeAttribute        ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_CHECKSUM_TYPE ) )

, sFullPathProperty             ( RTL_CONSTASCII_USTRINGPARAM ( "FullPath" ) )
, sMediaTypeProperty            ( RTL_CONSTASCII_USTRINGPARAM ( "MediaType" ) )
, sVersionProperty              ( RTL_CONSTASCII_USTRINGPARAM ( "Version" ) )
, sIterationCountProperty       ( RTL_CONSTASCII_USTRINGPARAM ( "IterationCount" ) )
, sSaltProperty                 ( RTL_CONSTASCII_USTRINGPARAM ( "Salt" ) )
, sInitialisationVectorProperty ( RTL_CONSTASCII_USTRINGPARAM ( "InitialisationVector" ) )
, sSizeProperty                 ( RTL_CONSTASCII_USTRINGPARAM ( "Size" ) )
, sDigestProperty               ( RTL_CONSTASCII_USTRINGPARAM ( "Digest" ) )

, sWhiteSpace                   ( RTL_CONSTASCII_USTRINGPARAM ( " " ) )
, sBlowfish                     ( RTL_CONSTASCII_USTRINGPARAM ( "Blowfish CFB" ) )
, sPBKDF2                       ( RTL_CONSTASCII_USTRINGPARAM ( "PBKDF2" ) )
, sChecksumType                 ( RTL_CONSTASCII_USTRINGPARAM ( CHECKSUM_TYPE ) )
{
}
ManifestImport::~ManifestImport (void )
{
}
void SAL_CALL ManifestImport::startDocument(  )
        throw(xml::sax::SAXException, uno::RuntimeException)
{
}
void SAL_CALL ManifestImport::endDocument(  )
        throw(xml::sax::SAXException, uno::RuntimeException)
{
}
void SAL_CALL ManifestImport::startElement( const OUString& aName, const uno::Reference< xml::sax::XAttributeList >& xAttribs )
        throw(xml::sax::SAXException, uno::RuntimeException)
{
    if (aName == sFileEntryElement)
    {
        aStack.push( e_FileEntry );
        aSequence.realloc ( PKG_SIZE_ENCR_MNFST );

        // Put full-path property first for MBA
        aSequence[nNumProperty].Name = sFullPathProperty;
        aSequence[nNumProperty++].Value <<= xAttribs->getValueByName( sFullPathAttribute );
        aSequence[nNumProperty].Name = sMediaTypeProperty;
        aSequence[nNumProperty++].Value <<= xAttribs->getValueByName( sMediaTypeAttribute );

        OUString sVersion = xAttribs->getValueByName ( sVersionAttribute );
        if ( sVersion.getLength() )
        {
            aSequence[nNumProperty].Name = sVersionProperty;
            aSequence[nNumProperty++].Value <<= sVersion;
        }

        OUString sSize = xAttribs->getValueByName ( sSizeAttribute );
        if (sSize.getLength())
        {
            sal_Int32 nSize;
            nSize = sSize.toInt32();
            aSequence[nNumProperty].Name = sSizeProperty;
            aSequence[nNumProperty++].Value <<= nSize;
        }
    }
    else if (!aStack.empty())
    {
        if (aStack.top() == e_FileEntry && aName == sEncryptionDataElement)
        {
            // If this element exists, then this stream is encrypted and we need
            // to store the initialisation vector, salt and iteration count used
            aStack.push (e_EncryptionData );
            OUString aString = xAttribs->getValueByName ( sChecksumTypeAttribute );
            if (aString == sChecksumType && !bIgnoreEncryptData)
            {
                aString = xAttribs->getValueByName ( sChecksumAttribute );
                Sequence < sal_uInt8 > aDecodeBuffer;
                Base64Codec::decodeBase64 (aDecodeBuffer, aString);
                aSequence[nNumProperty].Name = sDigestProperty;
                aSequence[nNumProperty++].Value <<= aDecodeBuffer;
            }
        }
        else if (aStack.top() == e_EncryptionData && aName == sAlgorithmElement)
        {
            aStack.push (e_Algorithm);
            OUString aString = xAttribs->getValueByName ( sAlgorithmNameAttribute );
            if (aString == sBlowfish && !bIgnoreEncryptData)
            {
                aString = xAttribs->getValueByName ( sInitialisationVectorAttribute );
                Sequence < sal_uInt8 > aDecodeBuffer;
                Base64Codec::decodeBase64 (aDecodeBuffer, aString);
                aSequence[nNumProperty].Name = sInitialisationVectorProperty;
                aSequence[nNumProperty++].Value <<= aDecodeBuffer;
            }
            else
                // If we don't recognise the algorithm, then the key derivation info
                // is useless to us
                bIgnoreEncryptData = sal_True;
        }
        else if (aStack.top() == e_EncryptionData && aName == sKeyDerivationElement)
        {
            aStack.push (e_KeyDerivation);
            OUString aString = xAttribs->getValueByName ( sKeyDerivationNameAttribute );
            if ( aString == sPBKDF2 && !bIgnoreEncryptData )
            {
                aString = xAttribs->getValueByName ( sSaltAttribute );
                Sequence < sal_uInt8 > aDecodeBuffer;
                Base64Codec::decodeBase64 (aDecodeBuffer, aString);
                aSequence[nNumProperty].Name = sSaltProperty;
                aSequence[nNumProperty++].Value <<= aDecodeBuffer;

                aString = xAttribs->getValueByName ( sIterationCountAttribute );
                aSequence[nNumProperty].Name = sIterationCountProperty;
                aSequence[nNumProperty++].Value <<= aString.toInt32();
            }
            else
                // If we don't recognise the key derivation technique, then the
                // algorithm info is useless to us
                bIgnoreEncryptData = sal_True;
        }
    }
}
void SAL_CALL ManifestImport::endElement( const OUString& /*aName*/ )
    throw(xml::sax::SAXException, uno::RuntimeException)
{
    if ( !aStack.empty() )
    {
        if (aStack.top() == e_FileEntry)
        {
            aSequence.realloc ( nNumProperty );
            bIgnoreEncryptData = sal_False;
            rManVector.push_back ( aSequence );
            nNumProperty = 0;
        }
        aStack.pop();
    }
}
void SAL_CALL ManifestImport::characters( const OUString& /*aChars*/ )
        throw(xml::sax::SAXException, uno::RuntimeException)
{
}
void SAL_CALL ManifestImport::ignorableWhitespace( const OUString& /*aWhitespaces*/ )
        throw(xml::sax::SAXException, uno::RuntimeException)
{
}
void SAL_CALL ManifestImport::processingInstruction( const OUString& /*aTarget*/, const OUString& /*aData*/ )
        throw(xml::sax::SAXException, uno::RuntimeException)
{
}
void SAL_CALL ManifestImport::setDocumentLocator( const uno::Reference< xml::sax::XLocator >& /*xLocator*/ )
        throw(xml::sax::SAXException, uno::RuntimeException)
{
}

