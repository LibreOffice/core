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

#include "DomExport.hxx"

#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlexp.hxx>
#include "xmlerror.hxx"

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/xml/dom/XAttr.hpp>
#include <com/sun/star/xml/dom/XDocumentBuilder.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XElement.hpp>
#include <com/sun/star/xml/dom/XEntity.hpp>
#include <com/sun/star/xml/dom/XNotation.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/xml/dom/NodeType.hpp>
#include <com/sun/star/xml/dom/XNamedNodeMap.hpp>

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/debug.hxx>

#include <unotools/processfactory.hxx>

#include <vector>


using com::sun::star::lang::XMultiServiceFactory;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::UNO_QUERY_THROW;
using std::vector;

using namespace com::sun::star::xml::dom;

using rtl::OUString;
using rtl::OUStringBuffer;


class DomVisitor
{
public:
    DomVisitor() {}
    virtual ~DomVisitor() {}
    virtual void element( const Reference<XElement>& ) {}
    virtual void character( const Reference<XCharacterData>& ) {}
    virtual void attribute( const Reference<XAttr>& ) {}
    virtual void cdata( const Reference<XCDATASection>& ) {}
    virtual void comment( const Reference<XComment>& ) {}
    virtual void documentFragment( const Reference<XDocumentFragment>& ) {}
    virtual void document( const Reference<XDocument>& ) {}
    virtual void documentType( const Reference<XDocumentType>& ) {}
    virtual void entity( const Reference<XEntity>& ) {}
    virtual void entityReference( const Reference<XEntityReference>& ) {}
    virtual void notation( const Reference<XNotation>& ) {}
    virtual void processingInstruction( const Reference<XProcessingInstruction>& ) {}
    virtual void endElement( const Reference<XElement>& ) {}
};

void visit( DomVisitor&, const Reference<XDocument>& );
void visit( DomVisitor&, const Reference<XNode>& );



void visitNode( DomVisitor& rVisitor, const Reference<XNode>& xNode )
{
    switch( xNode->getNodeType() )
    {
    case NodeType_ATTRIBUTE_NODE:
        rVisitor.attribute( Reference<XAttr>( xNode, UNO_QUERY_THROW ) );
        break;
    case NodeType_CDATA_SECTION_NODE:
        rVisitor.cdata( Reference<XCDATASection>( xNode, UNO_QUERY_THROW ) );
        break;
    case NodeType_COMMENT_NODE:
        rVisitor.comment( Reference<XComment>( xNode, UNO_QUERY_THROW ) );
        break;
    case NodeType_DOCUMENT_FRAGMENT_NODE:
        rVisitor.documentFragment( Reference<XDocumentFragment>( xNode, UNO_QUERY_THROW ) );
        break;
    case NodeType_DOCUMENT_NODE:
        rVisitor.document( Reference<XDocument>( xNode, UNO_QUERY_THROW ) );
        break;
    case NodeType_DOCUMENT_TYPE_NODE:
        rVisitor.documentType( Reference<XDocumentType>( xNode, UNO_QUERY_THROW ) );
        break;
    case NodeType_ELEMENT_NODE:
        rVisitor.element( Reference<XElement>( xNode, UNO_QUERY_THROW ) );
        break;
    case NodeType_ENTITY_NODE:
        rVisitor.entity( Reference<XEntity>( xNode, UNO_QUERY_THROW ) );
        break;
    case NodeType_ENTITY_REFERENCE_NODE:
        rVisitor.entityReference( Reference<XEntityReference>( xNode, UNO_QUERY_THROW ) );
        break;
    case NodeType_NOTATION_NODE:
        rVisitor.notation( Reference<XNotation>( xNode, UNO_QUERY_THROW ) );
        break;
    case NodeType_PROCESSING_INSTRUCTION_NODE:
        rVisitor.processingInstruction( Reference<XProcessingInstruction>( xNode, UNO_QUERY_THROW ) );
        break;
    case NodeType_TEXT_NODE:
        rVisitor.character( Reference<XCharacterData>( xNode, UNO_QUERY_THROW ) );
        break;
    default:
        OSL_FAIL( "unknown DOM node type" );
        break;
    }
}

void visit( DomVisitor& rVisitor, const Reference<XDocument>& xDocument )
{
    visit( rVisitor, Reference<XNode>( xDocument, UNO_QUERY_THROW ) );
}

void visit( DomVisitor& rVisitor, const Reference<XNode>& xNode )
{
    visitNode( rVisitor, xNode );
    for( Reference<XNode> xChild = xNode->getFirstChild();
         xChild.is();
         xChild = xChild->getNextSibling() )
    {
        visit( rVisitor, xChild );
    }
    if( xNode->getNodeType() == NodeType_ELEMENT_NODE )
        rVisitor.endElement( Reference<XElement>( xNode, UNO_QUERY_THROW ) );
}



class DomExport: public DomVisitor
{
    SvXMLExport& mrExport;
    vector<SvXMLNamespaceMap> maNamespaces;

    void pushNamespace();
    void popNamespace();
    void addNamespace( const OUString& sPrefix, const OUString& sURI );
    OUString qualifiedName( const OUString& sPrefix, const OUString& sURI,
                            const OUString& sLocalName );
    OUString qualifiedName( const Reference<XNode>&  );
    OUString qualifiedName( const Reference<XElement>&  );
    OUString qualifiedName( const Reference<XAttr>&  );
    void addAttribute( const Reference<XAttr>& );

public:

    DomExport( SvXMLExport& rExport );
    virtual ~DomExport();

    virtual void element( const Reference<XElement>& );
    virtual void endElement( const Reference<XElement>& );
    virtual void character( const Reference<XCharacterData>& );
};

DomExport::DomExport( SvXMLExport& rExport ) :
    mrExport( rExport )
{
    maNamespaces.push_back( rExport.GetNamespaceMap() );
}

DomExport::~DomExport()
{
    DBG_ASSERT( maNamespaces.size() == 1, "namespace missing" );
    maNamespaces.clear();
}

void DomExport::pushNamespace()
{
    maNamespaces.push_back( maNamespaces.back() );
}

void DomExport::popNamespace()
{
    maNamespaces.pop_back();
}

void DomExport::addNamespace( const OUString& sPrefix, const OUString& sURI )
{
    SvXMLNamespaceMap& rMap = maNamespaces.back();
    sal_uInt16 nKey = rMap.GetKeyByPrefix( sPrefix );

    // we need to register the namespace, if either the prefix isn't known or
    // is used for a different namespace
    if( nKey == XML_NAMESPACE_UNKNOWN  ||
        rMap.GetNameByKey( nKey ) != sURI )
    {
        // add prefix to map, and add declaration
        rMap.Add( sPrefix, sURI );
        mrExport.AddAttribute(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "xmlns:" ) ) + sPrefix,
            sURI );
    }
}

OUString DomExport::qualifiedName( const OUString& sPrefix,
                                   const OUString& sURI,
                                   const OUString& sLocalName )
{
    OUStringBuffer sBuffer;
    if( ( sPrefix.getLength() > 0 ) && ( sURI.getLength() > 0 ) )
    {
        addNamespace( sPrefix, sURI );
        sBuffer.append( sPrefix );
        sBuffer.append( sal_Unicode( ':' ) );
    }
    sBuffer.append( sLocalName );
    return sBuffer.makeStringAndClear();
}

OUString DomExport::qualifiedName( const Reference<XNode>& xNode )
{
    return qualifiedName( xNode->getPrefix(), xNode->getNamespaceURI(),
                          xNode->getNodeName() );
}

OUString DomExport::qualifiedName( const Reference<XElement>& xElement )
{
    return qualifiedName( xElement->getPrefix(), xElement->getNamespaceURI(),
                          xElement->getNodeName() );
}

OUString DomExport::qualifiedName( const Reference<XAttr>& xAttr )
{
    return qualifiedName( xAttr->getPrefix(), xAttr->getNamespaceURI(),
                          xAttr->getNodeName() );
}

void DomExport::addAttribute( const Reference<XAttr>& xAttribute )
{
    mrExport.AddAttribute( qualifiedName( xAttribute ),
                           xAttribute->getNodeValue() );
}

void DomExport::element( const Reference<XElement>& xElement )
{
    pushNamespace();

    // write attributes
    Reference<XNamedNodeMap> xAttributes = xElement->getAttributes();
    sal_Int32 nLength = xAttributes.is() ? xAttributes->getLength() : 0;
    for( sal_Int32 n = 0; n < nLength; n++ )
    {
        addAttribute( Reference<XAttr>( xAttributes->item( n ), UNO_QUERY_THROW ) );
    }

    // write name
    mrExport.StartElement( qualifiedName( xElement ), sal_False );
}

void DomExport::endElement( const Reference<XElement>& xElement )
{
    mrExport.EndElement( qualifiedName( xElement ), sal_False );
    popNamespace();
}

void DomExport::character( const Reference<XCharacterData>& xChars )
{
    mrExport.Characters( xChars->getNodeValue() );
}


void exportDom( SvXMLExport& rExport, const Reference<XDocument>& xDocument )
{
    DomExport aDomExport( rExport );
    visit( aDomExport, xDocument );
}

void exportDom( SvXMLExport& rExport, const Reference<XNode>& xNode )
{
    DomExport aDomExport( rExport );
    visit( aDomExport, xNode );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
