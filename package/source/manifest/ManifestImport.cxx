/*************************************************************************
 *
 *  $RCSfile: ManifestImport.cxx,v $
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

#ifndef _MANIFEST_IMPORT_HXX
#include <ManifestImport.hxx>
#endif
#ifndef _MANIFEST_DEFINES_HXX
#include <ManifestDefines.hxx>
#endif
#ifndef _BASE64_CODEC_HXX_
#include <Base64Codec.hxx>
#endif
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star;
using namespace rtl;
using namespace std;

ManifestImport::ManifestImport( vector < Sequence < PropertyValue > > & rNewManVector )
: rManVector ( rNewManVector )
, nNumProperty (0)
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
    static const OUString sEncryptionDataElement   ( RTL_CONSTASCII_USTRINGPARAM ( ELEMENT_ENCRYPTION_DATA ) );
    static const OUString sInitialisationVectorElement   ( RTL_CONSTASCII_USTRINGPARAM ( ELEMENT_INITIALISATION_VECTOR ) );
    static const OUString sAlgorithmAttribute   ( RTL_CONSTASCII_USTRINGPARAM ( ELEMENT_INITIALISATION_VECTOR ) );
    static const OUString sSaltAttribute   ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_SALT ) );
    static const OUString sIterationCountAttribute   ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_ITERATION_COUNT ) );
    static const OUString sManifestElement    ( RTL_CONSTASCII_USTRINGPARAM ( ELEMENT_MANIFEST ) );
    static const OUString sFileEntryElement   ( RTL_CONSTASCII_USTRINGPARAM ( ELEMENT_FILE_ENTRY ) );
    static const OUString sCdataAttribute     ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_CDATA ) );
    static const OUString sMediaTypeAttribute ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_MEDIA_TYPE ) );
    static const OUString sFullPathAttribute  ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_FULL_PATH ) );
    static const OUString sMediaTypeProperty  ( RTL_CONSTASCII_USTRINGPARAM ( "MediaType" ) );
    static const OUString sFullPathProperty   ( RTL_CONSTASCII_USTRINGPARAM ( "FullPath" ) );
    static const OUString sSaltProperty   ( RTL_CONSTASCII_USTRINGPARAM ( "Salt" ) );
    static const OUString sAlgorithmProperty   ( RTL_CONSTASCII_USTRINGPARAM ( "Algorithm" ) );
    static const OUString sIterationCountProperty   ( RTL_CONSTASCII_USTRINGPARAM ( "IterationCount" ) );

    if (aName == sFileEntryElement)
    {
        aStack.push( e_FileEntry );
        aSequence.realloc ( 2 );
        aSequence[0].Name = sMediaTypeProperty;
        aSequence[0].Value <<= xAttribs->getValueByName( sMediaTypeAttribute );
        aSequence[1].Name = sFullPathProperty;
        aSequence[1].Value <<= xAttribs->getValueByName( sFullPathAttribute );
        nNumProperty=2;
    }
    else if (!aStack.empty())
    {
        if (aStack.top() == e_FileEntry && aName == sEncryptionDataElement)
        {
            aSequence.realloc (nNumProperty+3);
            aStack.push (e_EncryptionData );
            aSequence[nNumProperty].Name = sAlgorithmProperty;
            aSequence[nNumProperty].Value <<= xAttribs->getValueByName( sAlgorithmAttribute );
            aSequence[nNumProperty+1].Name = sSaltProperty;
            OUString aString;
            aString = xAttribs->getValueByName ( sSaltAttribute );
            Sequence < sal_Int8 > aDecodeBuffer;
            Base64Codec::decodeBase64 (aDecodeBuffer, aString);
            aSequence[nNumProperty+1].Value <<= aDecodeBuffer;
            aSequence[nNumProperty+2].Name = sIterationCountProperty;
            aSequence[nNumProperty+2].Value <<= xAttribs->getValueByName( sIterationCountAttribute );
            nNumProperty+=3;
        }
        else if (aStack.top() == e_EncryptionData && aName == sInitialisationVectorElement)
        {
            aStack.push (e_InitialisationVector);
            aSequence.realloc (++nNumProperty);
        }
    }
}
void SAL_CALL ManifestImport::endElement( const OUString& aName )
    throw(xml::sax::SAXException, uno::RuntimeException)
{
    if ( !aStack.empty() )
    {
        if (aStack.top() == e_FileEntry)
            rManVector.push_back ( aSequence );
        aStack.pop();
    }
}
void SAL_CALL ManifestImport::characters( const OUString& aChars )
        throw(xml::sax::SAXException, uno::RuntimeException)
{
    if (!aStack.empty() && aStack.top() == e_InitialisationVector)
    {
        const OUString sAlgorithmProperty   ( RTL_CONSTASCII_USTRINGPARAM ( "Algorithm" ) );
        Sequence < sal_Int8 > aDecodeBuffer;
        Base64Codec::decodeBase64 (aDecodeBuffer, aChars);
        aSequence[nNumProperty-1].Name = sAlgorithmProperty;
        aSequence[nNumProperty-1].Value <<= aDecodeBuffer;
    }
}
void SAL_CALL ManifestImport::ignorableWhitespace( const OUString& aWhitespaces )
        throw(xml::sax::SAXException, uno::RuntimeException)
{
}
void SAL_CALL ManifestImport::processingInstruction( const OUString& aTarget, const OUString& aData )
        throw(xml::sax::SAXException, uno::RuntimeException)
{
}
void SAL_CALL ManifestImport::setDocumentLocator( const uno::Reference< xml::sax::XLocator >& xLocator )
        throw(xml::sax::SAXException, uno::RuntimeException)
{
}

