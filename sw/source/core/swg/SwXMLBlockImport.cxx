/*************************************************************************
 *
 *  $RCSfile: SwXMLBlockImport.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 09:05:01 $
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
#ifndef _SW_XMLBLOCKIMPORT_HXX
#include <SwXMLBlockImport.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
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
using namespace ::rtl;

sal_Char __READONLY_DATA sXML_np__block_list[] = "_block-list";
sal_Char __READONLY_DATA sXML_np__office[] = "_ooffice";
sal_Char __READONLY_DATA sXML_np__text[] = "_otext";

// #110680#
SwXMLBlockListImport::SwXMLBlockListImport(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
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
        const Reference< xml::sax::XAttributeList > & xAttrList )
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
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
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
        const Reference< xml::sax::XAttributeList > & xAttrList )
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
        throw( ::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException )
{
}
