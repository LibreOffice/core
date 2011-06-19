/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

bool lcl_IsWhiteSpace( sal_Unicode c )
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
