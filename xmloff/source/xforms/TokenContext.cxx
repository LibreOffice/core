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

#include "TokenContext.hxx"
#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmloff/xmlerror.hxx"

#include <tools/debug.hxx>

using rtl::OUString;
using com::sun::star::uno::Reference;
using com::sun::star::xml::sax::XAttributeList;


struct SvXMLTokenMapEntry aEmptyMap[1] =
{
    XML_TOKEN_MAP_END
};


TokenContext::TokenContext( SvXMLImport& rImport,
                            sal_uInt16 nPrefix,
                            const OUString& rLocalName,
                            const SvXMLTokenMapEntry* pAttributes,
                            const SvXMLTokenMapEntry* pChildren )
    : SvXMLImportContext( rImport, nPrefix, rLocalName ),
      mpAttributes( pAttributes ),
      mpChildren( pChildren )
{
}

TokenContext::~TokenContext()
{
}

void TokenContext::StartElement(
    const Reference<XAttributeList>& xAttributeList )
{
    // iterate over attributes
    // - if in map: call HandleAttribute
    // - xmlns:... : ignore
    // - other: warning
    DBG_ASSERT( mpAttributes != NULL, "no token map for attributes" );
    SvXMLTokenMap aMap( mpAttributes );

    sal_Int16 nCount = xAttributeList->getLength();
    for( sal_Int16 i = 0; i < nCount; i++ )
    {
        // get key/local-name pair from namespace map
        OUString sLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
            GetKeyByAttrName( xAttributeList->getNameByIndex(i), &sLocalName );

        // get token from token map
        sal_uInt16 nToken = aMap.Get( nPrefix, sLocalName );

        // and the value...
        const OUString& rValue = xAttributeList->getValueByIndex(i);

        if( nToken != XML_TOK_UNKNOWN )
        {
            HandleAttribute( nToken, rValue );
        }
        else if( nPrefix != XML_NAMESPACE_XMLNS )
        {
            // error handling, for all attribute that are not
            // namespace declarations
            GetImport().SetError( XMLERROR_UNKNOWN_ATTRIBUTE,
                                  sLocalName, rValue);
        }
    }
}

SvXMLImportContext* TokenContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList>& xAttrList )
{
    // call HandleChild for elements in token map. Ignore other content.

    SvXMLImportContext* pContext = NULL;

    DBG_ASSERT( mpChildren != NULL, "no token map for child elements" );
    SvXMLTokenMap aMap( mpChildren );
    sal_uInt16 nToken = aMap.Get( nPrefix, rLocalName );
    if( nToken != XML_TOK_UNKNOWN )
    {
        // call handle child, and pass down arguments
        pContext = HandleChild( nToken, nPrefix, rLocalName, xAttrList );
    }

    // error handling: create default context and generate warning
    if( pContext == NULL )
    {
        GetImport().SetError( XMLERROR_UNKNOWN_ELEMENT, rLocalName );
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }
    return pContext;
}

static bool lcl_IsWhiteSpace( sal_Unicode c )
{
    return c == sal_Unicode(  ' ' )
        || c == sal_Unicode( 0x09 )
        || c == sal_Unicode( 0x0A )
        || c == sal_Unicode( 0x0D );
}

void TokenContext::Characters( const ::rtl::OUString& rCharacters )
{
    // get iterators for string data
    const sal_Unicode* pBegin = rCharacters.getStr();
    const sal_Unicode* pEnd = &( pBegin[ rCharacters.getLength() ] );

    // raise error if non-whitespace character is found
    if( ::std::find_if( pBegin, pEnd, ::std::not1(::std::ptr_fun(lcl_IsWhiteSpace)) ) != pEnd )
        GetImport().SetError( XMLERROR_UNKNOWN_CHARACTERS, rCharacters );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
