/*************************************************************************
 *
 *  $RCSfile: ManifestExport.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mtg $ $Date: 2001-04-27 14:56:05 $
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
using namespace std;
using namespace com::sun::star::beans;
using namespace com::sun::star::uno;
using namespace com::sun::star::xml::sax;

ManifestExport::ManifestExport(Reference < XDocumentHandler > xHandler,  const Sequence < Sequence < PropertyValue > > &rManList)
{
    const OUString sFileEntryElement    ( RTL_CONSTASCII_USTRINGPARAM ( ELEMENT_FILE_ENTRY ) );
    const OUString sManifestElement     ( RTL_CONSTASCII_USTRINGPARAM ( ELEMENT_MANIFEST ) );
    const OUString sInitialisationVectorElement    ( RTL_CONSTASCII_USTRINGPARAM ( ELEMENT_INITIALISATION_VECTOR ) );
    const OUString sEncryptionDataElement    ( RTL_CONSTASCII_USTRINGPARAM ( ELEMENT_ENCRYPTION_DATA ) );
    const OUString sCdataAttribute      ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_CDATA ) );
    const OUString sMediaTypeAttribute  ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_MEDIA_TYPE ) );
    const OUString sFullPathAttribute   ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_FULL_PATH ) );
    const OUString sAlgorithmAttribute  ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_ALGORITHM ) );
    const OUString sSaltAttribute       ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_SALT ) );
    const OUString sIterationCountAttribute ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_ITERATION_COUNT ) );

    const OUString sFullPath            ( RTL_CONSTASCII_USTRINGPARAM ( "FullPath" ) );
    const OUString sMediaType           ( RTL_CONSTASCII_USTRINGPARAM ( "MediaType" ) );
    const OUString sBlowfish            ( RTL_CONSTASCII_USTRINGPARAM ( "Blowfish" ) );
    const OUString sIterationCount      ( RTL_CONSTASCII_USTRINGPARAM ( "IterationCount" ) );
    const OUString sSalt                ( RTL_CONSTASCII_USTRINGPARAM ( "Salt" ) );
    const OUString sInitialisationVector( RTL_CONSTASCII_USTRINGPARAM ( "InitialisationVector" ) );

    AttributeList * pRootAttrList = new AttributeList;
    pRootAttrList->AddAttribute ( OUString( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_XMLNS ) ),
                                  sCdataAttribute,
                                  OUString( RTL_CONSTASCII_USTRINGPARAM ( MANIFEST_NAMESPACE ) ) );

    Reference < XAttributeList > xRootAttrList (pRootAttrList);
    xHandler->startDocument();
    xHandler->startElement( sManifestElement, xRootAttrList );
    const Sequence < PropertyValue > *pSequence = rManList.getConstArray();

    for (sal_uInt32 i = 0, nEnd = rManList.getLength() ; i < nEnd ; i++)
    {
        AttributeList *pAttrList = new AttributeList;
        const PropertyValue *pValue = pSequence->getConstArray();
        OUString aString;
        const PropertyValue *pVector = NULL, *pSalt = NULL, *pIterationCount = NULL;
        for (sal_uInt32 j = 0, nNum = pSequence->getLength(); j < nNum; j++, pValue++)
        {
            if (pValue->Name.equals (sMediaType) )
            {
                pValue->Value >>= aString;
                pAttrList->AddAttribute ( sMediaTypeAttribute, sCdataAttribute, aString );
            }
            else if (pValue->Name.equals (sFullPath) )
            {
                pValue->Value >>= aString;
                pAttrList->AddAttribute ( sFullPathAttribute, sCdataAttribute, aString );
            }
            else if (pValue->Name.equals (sInitialisationVector) )
                pVector = pValue;
            else if (pValue->Name.equals (sSalt) )
                pSalt = pValue;
            else if (pValue->Name.equals (sIterationCount) )
                pIterationCount = pValue;
        }
        Reference < XAttributeList > xAttrList = pAttrList;
        xHandler->startElement( sFileEntryElement , xAttrList);
        if ( pVector )
        {
            AttributeList * pAttrList = new AttributeList;
            pAttrList->AddAttribute ( sAlgorithmAttribute, sCdataAttribute, sBlowfish );
            if ( pIterationCount )
            {
                sal_Int64 nValue;
                pIterationCount->Value >>= nValue;
                OUStringBuffer aBuffer;
                aBuffer.append (nValue);
                pAttrList->AddAttribute ( sIterationCountAttribute, sCdataAttribute, aBuffer.makeStringAndClear() );
            }
            if ( pSalt )
            {
                OUStringBuffer aBuffer;
                Sequence < sal_Int8 > aSequence;
                pSalt->Value >>= aSequence;
                Base64Codec::encodeBase64 ( aBuffer, aSequence );
                pAttrList->AddAttribute ( sSaltAttribute, sCdataAttribute, aBuffer.makeStringAndClear() );
            }
            Reference < XAttributeList > xAttrList (pAttrList);
            xHandler->startElement( sEncryptionDataElement , xAttrList);
            if ( pVector )
            {
                AttributeList * pAttrList = new AttributeList;
                Reference < XAttributeList > xAttrList (pAttrList);
                OUStringBuffer aBuffer;
                Sequence < sal_Int8 > aSequence;
                pVector->Value >>= aSequence;
                Base64Codec::encodeBase64 ( aBuffer, aSequence );
                xHandler->startElement ( sInitialisationVectorElement, xAttrList);
                xHandler->characters ( aBuffer.makeStringAndClear() );
                xHandler->endElement ( sInitialisationVectorElement );
            }
            xHandler->endElement( sEncryptionDataElement );
        }
        xHandler->endElement( sFileEntryElement );
        pSequence++;
    }
    xHandler->endElement( sManifestElement );
    xHandler->endDocument();
}
