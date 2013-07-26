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

#include <SwXMLBlockImport.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <SwXMLBlockListContext.hxx>
#include <SwXMLTextBlocks.hxx>
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::xmloff::token;

sal_Char const sXML_np__block_list[] = "_block-list";
sal_Char const sXML_np__office[] = "_ooffice";
sal_Char const sXML_np__text[] = "_otext";

SwXMLBlockListImport::SwXMLBlockListImport(
    const uno::Reference< uno::XComponentContext > xContext,
    SwXMLTextBlocks &rBlocks )
:   SvXMLImport( xContext, 0 ),
    rBlockList (rBlocks)
{
    GetNamespaceMap().Add( OUString ( sXML_np__block_list ),
                           GetXMLToken ( XML_N_BLOCK_LIST ),
                           XML_NAMESPACE_BLOCKLIST );
}

SwXMLBlockListImport::~SwXMLBlockListImport ( void )
    throw ()
{
}

SvXMLImportContext *SwXMLBlockListImport::CreateContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    if ( XML_NAMESPACE_BLOCKLIST == nPrefix &&
         IsXMLToken ( rLocalName, XML_BLOCK_LIST ) )
        pContext = new SwXMLBlockListContext( *this, nPrefix, rLocalName,
                                              xAttrList );
    else
        pContext = SvXMLImport::CreateContext( nPrefix, rLocalName, xAttrList );
    return pContext;
}

SwXMLTextBlockImport::SwXMLTextBlockImport(
    const uno::Reference< uno::XComponentContext > xContext,
    SwXMLTextBlocks &rBlocks,
    OUString & rNewText,
    sal_Bool bNewTextOnly )
:   SvXMLImport(xContext, IMPORT_ALL ),
    rBlockList ( rBlocks ),
    bTextOnly ( bNewTextOnly ),
    m_rText ( rNewText )
{
    GetNamespaceMap().Add( OUString( sXML_np__office ),
                            GetXMLToken(XML_N_OFFICE_OOO),
                            XML_NAMESPACE_OFFICE );
    GetNamespaceMap().Add( OUString( sXML_np__text ),
                            GetXMLToken(XML_N_TEXT_OOO),
                            XML_NAMESPACE_TEXT );
}

SwXMLTextBlockImport::~SwXMLTextBlockImport ( void )
    throw()
{
}

SvXMLImportContext *SwXMLTextBlockImport::CreateContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    if( XML_NAMESPACE_OFFICE == nPrefix &&
        IsXMLToken ( rLocalName, bTextOnly ? XML_DOCUMENT : XML_DOCUMENT_CONTENT ) )
        pContext = new SwXMLTextBlockDocumentContext( *this, nPrefix, rLocalName, xAttrList );
    else
        pContext = SvXMLImport::CreateContext( nPrefix, rLocalName, xAttrList );
    return pContext;
}
void SAL_CALL SwXMLTextBlockImport::endDocument(void)
        throw( xml::sax::SAXException, uno::RuntimeException )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
