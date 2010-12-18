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
#include "precompiled_sw.hxx"
#include <SwXMLBlockImport.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <SwXMLBlockListContext.hxx>
#include <SwXMLTextBlocks.hxx>
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::xmloff::token;
using ::rtl::OUString;

sal_Char __READONLY_DATA sXML_np__block_list[] = "_block-list";
sal_Char __READONLY_DATA sXML_np__office[] = "_ooffice";
sal_Char __READONLY_DATA sXML_np__text[] = "_otext";

// #110680#
SwXMLBlockListImport::SwXMLBlockListImport(
    const uno::Reference< lang::XMultiServiceFactory > xServiceFactory,
    SwXMLTextBlocks &rBlocks )
:   SvXMLImport( xServiceFactory, 0 ),
    rBlockList (rBlocks)
{
    GetNamespaceMap().Add( OUString ( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__block_list ) ),
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

// #110680#
SwXMLTextBlockImport::SwXMLTextBlockImport(
    const uno::Reference< lang::XMultiServiceFactory > xServiceFactory,
    SwXMLTextBlocks &rBlocks,
    String & rNewText,
    sal_Bool bNewTextOnly )
:   SvXMLImport(xServiceFactory, IMPORT_ALL ),
    rBlockList ( rBlocks ),
    bTextOnly ( bNewTextOnly ),
    m_rText ( rNewText )
{
    GetNamespaceMap().Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__office ) ),
                            GetXMLToken(XML_N_OFFICE_OOO),
                            XML_NAMESPACE_OFFICE );
    GetNamespaceMap().Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__text ) ),
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
