/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SwXMLBlockImport.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 12:23:05 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#ifndef _SW_XMLBLOCKIMPORT_HXX
#include <SwXMLBlockImport.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _SW_XMLBLOCKLIST_CONTEXT_HXX
#include <SwXMLBlockListContext.hxx>
#endif
#ifndef _SW_XMLTEXTBLOCKS_HXX
#include <SwXMLTextBlocks.hxx>
#endif
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
