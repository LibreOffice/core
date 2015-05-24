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


#include "DomExport.hxx"

#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlerror.hxx>

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


#include <vector>


using com::sun::star::lang::XMultiServiceFactory;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::UNO_QUERY_THROW;
using std::vector;

using namespace com::sun::star::xml::dom;



class DomVisitor
{
public:
    DomVisitor() {}
    virtual ~DomVisitor() {}
    virtual void element( const Reference<XElement>& ) {}
    virtual void character( const Reference<XCharacterData>& ) {}
    virtual void endElement( const Reference<XElement>& ) {}
};

void visit( DomVisitor&, const Reference<XDocument>& );
void visit( DomVisitor&, const Reference<XNode>& );



void visitNode( DomVisitor& rVisitor, const Reference<XNode>& xNode )
{
    switch( xNode->getNodeType() )
    {
    case NodeType_ATTRIBUTE_NODE:
        break;
    case NodeType_CDATA_SECTION_NODE:
        break;
    case NodeType_COMMENT_NODE:
        break;
    case NodeType_DOCUMENT_FRAGMENT_NODE:
        break;
    case NodeType_DOCUMENT_NODE:
        break;
    case NodeType_DOCUMENT_TYPE_NODE:
        break;
    case NodeType_ELEMENT_NODE:
        rVisitor.element( Reference<XElement>( xNode, UNO_QUERY_THROW ) );
        break;
    case NodeType_ENTITY_NODE:
        break;
    case NodeType_ENTITY_REFERENCE_NODE:
        break;
    case NodeType_NOTATION_NODE:
        break;
    case NodeType_PROCESSING_INSTRUCTION_NODE:
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
    OUString qualifiedName( const Reference<XElement>&  );
    OUString qualifiedName( const Reference<XAttr>&  );
    void addAttribute( const Reference<XAttr>& );

public:

    explicit DomExport( SvXMLExport& rExport );
    virtual ~DomExport();

    virtual void element( const Reference<XElement>& ) SAL_OVERRIDE;
    virtual void endElement( const Reference<XElement>& ) SAL_OVERRIDE;
    virtual void character( const Reference<XCharacterData>& ) SAL_OVERRIDE;
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
    SvXMLNamespaceMap const aMap(maNamespaces.back());
    maNamespaces.push_back(aMap);
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
        mrExport.AddAttribute( "xmlns:" + sPrefix, sURI );
    }
}

OUString DomExport::qualifiedName( const OUString& sPrefix,
                                   const OUString& sURI,
                                   const OUString& sLocalName )
{
    OUStringBuffer sBuffer;
    if( !sPrefix.isEmpty() && !sURI.isEmpty() )
    {
        addNamespace( sPrefix, sURI );
        sBuffer.append( sPrefix );
        sBuffer.append( ':' );
    }
    sBuffer.append( sLocalName );
    return sBuffer.makeStringAndClear();
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
    mrExport.StartElement( qualifiedName( xElement ), false );
}

void DomExport::endElement( const Reference<XElement>& xElement )
{
    mrExport.EndElement( qualifiedName( xElement ), false );
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
