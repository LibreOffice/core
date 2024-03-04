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

#include <sal/config.h>

#include "TokenContext.hxx"
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlerror.hxx>

#include <algorithm>

using com::sun::star::uno::Reference;

TokenContext::TokenContext( SvXMLImport& rImport )
    : SvXMLImportContext( rImport )
{
}

void TokenContext::startFastElement(
    sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    // iterate over attributes
    // - if in map: call HandleAttribute
    // - xmlns:... : ignore
    // - other: warning

    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
        HandleAttribute( aIter );
}

css::uno::Reference< css::xml::sax::XFastContextHandler > TokenContext::createFastChildContext(
    sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    // call handle child, and pass down arguments
    SvXMLImportContext* pContext = HandleChild( nElement, xAttrList );
    // error handling: create default context and generate warning
    if( pContext == nullptr )
    {
        GetImport().SetError( XMLERROR_UNKNOWN_ELEMENT, SvXMLImport::getNameFromToken( nElement ) );
    }
    return pContext;
}

 css::uno::Reference< css::xml::sax::XFastContextHandler > TokenContext::createUnknownChildContext(
     const OUString& Namespace, const OUString& Name, const css::uno::Reference< css::xml::sax::XFastAttributeList >& )
{
    GetImport().SetError( XMLERROR_UNKNOWN_ELEMENT, Namespace + " " + Name );
    return nullptr;
}

static bool lcl_IsWhiteSpace( sal_Unicode c )
{
    return c == ' '
        || c == u'\x0009'
        || c == u'\x000A'
        || c == u'\x000D';
}

void TokenContext::characters( const OUString& rCharacters )
{
    // get iterators for string data
    const sal_Unicode* pBegin = rCharacters.getStr();
    const sal_Unicode* pEnd = &( pBegin[ rCharacters.getLength() ] );

    // raise error if non-whitespace character is found
    if( !::std::all_of( pBegin, pEnd, lcl_IsWhiteSpace ) )
        GetImport().SetError( XMLERROR_UNKNOWN_CHARACTERS, rCharacters );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
