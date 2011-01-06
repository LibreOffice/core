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
#include "precompiled_xmloff.hxx"

#include "SchXMLImport.hxx"
#include "SchXMLParagraphContext.hxx"

#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/nmspmap.hxx>

using ::rtl::OUString;
using namespace com::sun::star;
using namespace ::xmloff::token;

SchXMLParagraphContext::SchXMLParagraphContext( SvXMLImport& rImport,
                                                const OUString& rLocalName,
                                                OUString& rText,
                                                OUString * pOutId /* = 0 */ ) :
        SvXMLImportContext( rImport, XML_NAMESPACE_TEXT, rLocalName ),
        mrText( rText ),
        mpId( pOutId )
{
}

SchXMLParagraphContext::~SchXMLParagraphContext()
{}

void SchXMLParagraphContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    // remember the id. It is used for storing the original cell range string in
    // a local table (cached data)
    if( mpId )
    {
        sal_Int16 nAttrCount = xAttrList.is()? xAttrList->getLength(): 0;
        rtl::OUString aValue;
        bool bHaveXmlId( false );

        for( sal_Int16 i = 0; i < nAttrCount; i++ )
        {
            rtl::OUString sAttrName = xAttrList->getNameByIndex( i );
            rtl::OUString aLocalName;
            sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

            if (IsXMLToken(aLocalName, XML_ID))
            {
                if (nPrefix == XML_NAMESPACE_XML)
                {
                    (*mpId) = xAttrList->getValueByIndex( i );
                    bHaveXmlId = true;
                }
                if (nPrefix == XML_NAMESPACE_TEXT)
                {   // text:id shall be ignored if xml:id exists
                    if (!bHaveXmlId)
                    {
                        (*mpId) = xAttrList->getValueByIndex( i );
                    }
                }
            }
        }
    }
}

void SchXMLParagraphContext::EndElement()
{
    mrText = maBuffer.makeStringAndClear();
}

SvXMLImportContext* SchXMLParagraphContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& )
{
    if( nPrefix == XML_NAMESPACE_TEXT )
    {
        if( rLocalName.equals( ::xmloff::token::GetXMLToken( ::xmloff::token::XML_TAB_STOP )))
        {
            maBuffer.append( sal_Unicode( 0x0009 ));  // tabulator
        }
        else if( rLocalName.equals( ::xmloff::token::GetXMLToken( ::xmloff::token::XML_LINE_BREAK )))
        {
            maBuffer.append( sal_Unicode( 0x000A ));  // linefeed
        }
    }

    return new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
}

void SchXMLParagraphContext::Characters( const OUString& rChars )
{
    maBuffer.append( rChars );
}
