/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
