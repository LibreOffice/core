/*************************************************************************
 *
 *  $RCSfile: ManifestExport.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: mtg $ $Date: 2001-09-05 19:21:21 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Martin Gallwey (gallwey@sun.com)
 *
 *
 ************************************************************************/

#ifndef _MANIFEST_EXPORT_HXX
#include <ManifestExport.hxx>
#endif
#ifndef _ATTRIBUTE_LIST_HXX
#include <AttributeList.hxx>
#endif
#ifndef _MANIFEST_DEFINES_HXX
#include <ManifestDefines.hxx>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HXX
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _BASE64_CODEC_HXX_
#include <Base64Codec.hxx>
#endif

using namespace rtl;
using namespace com::sun::star::beans;
using namespace com::sun::star::uno;
using namespace com::sun::star::xml::sax;

ManifestExport::ManifestExport(Reference < XDocumentHandler > xHandler,  const Sequence < Sequence < PropertyValue > > &rManList)
{
    const OUString sFileEntryElement    ( RTL_CONSTASCII_USTRINGPARAM ( ELEMENT_FILE_ENTRY ) );
    const OUString sManifestElement     ( RTL_CONSTASCII_USTRINGPARAM ( ELEMENT_MANIFEST ) );
    const OUString sEncryptionDataElement( RTL_CONSTASCII_USTRINGPARAM ( ELEMENT_ENCRYPTION_DATA ) );
    const OUString sAlgorithmElement    ( RTL_CONSTASCII_USTRINGPARAM ( ELEMENT_ALGORITHM ) );
    const OUString sKeyDerivationElement( RTL_CONSTASCII_USTRINGPARAM ( ELEMENT_KEY_DERIVATION ) );

    const OUString sCdataAttribute      ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_CDATA ) );
    const OUString sMediaTypeAttribute  ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_MEDIA_TYPE ) );
    const OUString sFullPathAttribute   ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_FULL_PATH ) );
    const OUString sSizeAttribute       ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_SIZE ) );
    const OUString sSaltAttribute       ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_SALT ) );
    const OUString sInitialisationVectorAttribute ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_INITIALISATION_VECTOR ) );
    const OUString sIterationCountAttribute ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_ITERATION_COUNT ) );
    const OUString sAlgorithmNameAttribute  ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_ALGORITHM_NAME ) );
    const OUString sKeyDerivationNameAttribute  ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_KEY_DERIVATION_NAME ) );
    const OUString sChecksumTypeAttribute   ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_CHECKSUM_TYPE ) );
    const OUString sChecksumAttribute   ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_CHECKSUM) );

    const OUString sFullPathProperty    ( RTL_CONSTASCII_USTRINGPARAM ( "FullPath" ) );
    const OUString sMediaTypeProperty   ( RTL_CONSTASCII_USTRINGPARAM ( "MediaType" ) );
    const OUString sIterationCountProperty  ( RTL_CONSTASCII_USTRINGPARAM ( "IterationCount" ) );
    const OUString sSaltProperty        ( RTL_CONSTASCII_USTRINGPARAM ( "Salt" ) );
    const OUString sInitialisationVectorProperty( RTL_CONSTASCII_USTRINGPARAM ( "InitialisationVector" ) );
    const OUString sSizeProperty        ( RTL_CONSTASCII_USTRINGPARAM ( "Size" ) );
    const OUString sDigestProperty      ( RTL_CONSTASCII_USTRINGPARAM ( "Digest" ) );

    const OUString sWhiteSpace          ( RTL_CONSTASCII_USTRINGPARAM ( " " ) );
    const OUString sBlowfish            ( RTL_CONSTASCII_USTRINGPARAM ( "Blowfish CFB" ) );
    const OUString sPBKDF2              ( RTL_CONSTASCII_USTRINGPARAM ( "PBKDF2" ) );
    const OUString sMD5                 ( RTL_CONSTASCII_USTRINGPARAM ( "MD5" ) );

    AttributeList * pRootAttrList = new AttributeList;
    pRootAttrList->AddAttribute ( OUString( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_XMLNS ) ),
                                  sCdataAttribute,
                                  OUString( RTL_CONSTASCII_USTRINGPARAM ( MANIFEST_NAMESPACE ) ) );

    Reference < XAttributeList > xRootAttrList (pRootAttrList);

    xHandler->startDocument();
    Reference < XExtendedDocumentHandler > xExtHandler ( xHandler, UNO_QUERY );
    if (xExtHandler.is())
    {
        OUString aDocType ( RTL_CONSTASCII_USTRINGPARAM ( MANIFEST_DOCTYPE ) );
        xExtHandler->unknown ( aDocType );
    }
    xHandler->ignorableWhitespace ( sWhiteSpace );
    xHandler->startElement( sManifestElement, xRootAttrList );

    const Sequence < PropertyValue > *pSequence = rManList.getConstArray();

    for (sal_uInt32 i = 0, nEnd = rManList.getLength() ; i < nEnd ; i++)
    {
        AttributeList *pAttrList = new AttributeList;
        const PropertyValue *pValue = pSequence->getConstArray();
        OUString aString;
        const PropertyValue *pVector = NULL, *pSalt = NULL, *pIterationCount = NULL, *pDigest = NULL;
        for (sal_uInt32 j = 0, nNum = pSequence->getLength(); j < nNum; j++, pValue++)
        {
            if (pValue->Name.equals (sMediaTypeProperty) )
            {
                pValue->Value >>= aString;
                pAttrList->AddAttribute ( sMediaTypeAttribute, sCdataAttribute, aString );
            }
            else if (pValue->Name.equals (sFullPathProperty) )
            {
                pValue->Value >>= aString;
                pAttrList->AddAttribute ( sFullPathAttribute, sCdataAttribute, aString );
            }
            else if (pValue->Name.equals (sSizeProperty) )
            {
                sal_Int32 nSize;
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
            AttributeList * pAttrList = new AttributeList;
            Reference < XAttributeList > xAttrList (pAttrList);
            OUStringBuffer aBuffer;
            Sequence < sal_uInt8 > aSequence;

            xHandler->ignorableWhitespace ( sWhiteSpace );
            if ( pDigest )
            {
                pAttrList->AddAttribute ( sChecksumTypeAttribute, sCdataAttribute, sMD5 );
                pDigest->Value >>= aSequence;
                Base64Codec::encodeBase64 ( aBuffer, aSequence );
                pAttrList->AddAttribute ( sChecksumAttribute, sCdataAttribute, aBuffer.makeStringAndClear() );
            }
            xHandler->startElement( sEncryptionDataElement , xAttrList);

            pAttrList = new AttributeList;
            xAttrList = pAttrList;

            pAttrList->AddAttribute ( sAlgorithmNameAttribute, sCdataAttribute, sBlowfish );

            pVector->Value >>= aSequence;
            Base64Codec::encodeBase64 ( aBuffer, aSequence );
            pAttrList->AddAttribute ( sInitialisationVectorAttribute, sCdataAttribute, aBuffer.makeStringAndClear() );

            xHandler->ignorableWhitespace ( sWhiteSpace );
            xHandler->startElement( sAlgorithmElement , xAttrList);
            xHandler->ignorableWhitespace ( sWhiteSpace );
            xHandler->endElement( sAlgorithmElement );

            pAttrList = new AttributeList;
            xAttrList = pAttrList;

            pAttrList->AddAttribute ( sKeyDerivationNameAttribute, sCdataAttribute, sPBKDF2 );

            sal_Int32 nCount;
            pIterationCount->Value >>= nCount;
            aBuffer.append (nCount);
            pAttrList->AddAttribute ( sIterationCountAttribute, sCdataAttribute, aBuffer.makeStringAndClear() );

            pSalt->Value >>= aSequence;
            Base64Codec::encodeBase64 ( aBuffer, aSequence );
            pAttrList->AddAttribute ( sSaltAttribute, sCdataAttribute, aBuffer.makeStringAndClear() );

            xHandler->ignorableWhitespace ( sWhiteSpace );
            xHandler->startElement( sKeyDerivationElement , xAttrList);
            xHandler->ignorableWhitespace ( sWhiteSpace );
            xHandler->endElement( sKeyDerivationElement );
            xHandler->ignorableWhitespace ( sWhiteSpace );
            xHandler->endElement( sEncryptionDataElement );
        }
        xHandler->ignorableWhitespace ( sWhiteSpace );
        xHandler->endElement( sFileEntryElement );
        pSequence++;
    }
    xHandler->ignorableWhitespace ( sWhiteSpace );
    xHandler->endElement( sManifestElement );
    xHandler->endDocument();
}
