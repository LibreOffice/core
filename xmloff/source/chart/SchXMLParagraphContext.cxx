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


#include "SchXMLImport.hxx"
#include "SchXMLParagraphContext.hxx"

#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/nmspmap.hxx>

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
        bool bHaveXmlId( false );

        for( sal_Int16 i = 0; i < nAttrCount; i++ )
        {
            OUString sAttrName = xAttrList->getNameByIndex( i );
            OUString aLocalName;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
