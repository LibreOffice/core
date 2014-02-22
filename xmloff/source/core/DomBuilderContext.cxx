/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "DomBuilderContext.hxx"

#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlerror.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/xml/dom/XAttr.hpp>
#include <com/sun/star/xml/dom/DocumentBuilder.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XElement.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/xml/dom/NodeType.hpp>

#include <rtl/ustring.hxx>
#include <tools/debug.hxx>

#include <comphelper/processfactory.hxx>


using com::sun::star::lang::XMultiServiceFactory;
using com::sun::star::uno::XComponentContext;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::UNO_QUERY_THROW;
using com::sun::star::xml::dom::DocumentBuilder;
using com::sun::star::xml::dom::XAttr;
using com::sun::star::xml::dom::XDocument;
using com::sun::star::xml::dom::XDocumentBuilder;
using com::sun::star::xml::dom::XNode;
using com::sun::star::xml::dom::XElement;
using com::sun::star::xml::sax::XAttributeList;
using com::sun::star::xml::dom::NodeType_ELEMENT_NODE;



static Reference<XNode> lcl_createDomInstance();
static Reference<XNode> lcl_createElement( SvXMLImport& rImport,
                                    sal_uInt16 nPrefix,
                                    const OUString rLocalName,
                                    Reference<XNode> xParent);


DomBuilderContext::DomBuilderContext( SvXMLImport& rImport,
                                      sal_uInt16 nPrefix,
                                      const OUString& rLocalName ) :
    SvXMLImportContext( rImport, nPrefix, rLocalName ),
    mxNode( lcl_createElement( rImport, nPrefix, rLocalName,
                               lcl_createDomInstance() ) )
{
    DBG_ASSERT( mxNode.is(), "empty XNode not allowed" );
    DBG_ASSERT( Reference<XElement>( mxNode, UNO_QUERY ).is(), "need element" );
    DBG_ASSERT( mxNode->getNodeType() == NodeType_ELEMENT_NODE, "need element" );
}

DomBuilderContext::DomBuilderContext( SvXMLImport& rImport,
                                      sal_uInt16 nPrefix,
                                      const OUString& rLocalName,
                                      Reference<XNode>& xParent ) :
    SvXMLImportContext( rImport, nPrefix, rLocalName ),
    mxNode( lcl_createElement( rImport, nPrefix, rLocalName, xParent ) )
{
    DBG_ASSERT( mxNode.is(), "empty XNode not allowed" );
    DBG_ASSERT( Reference<XElement>( mxNode, UNO_QUERY ).is(), "need element" );
    DBG_ASSERT( mxNode->getNodeType() == NodeType_ELEMENT_NODE, "need element" );
}

DomBuilderContext::~DomBuilderContext()
{
}

Reference<XDocument> DomBuilderContext::getTree()
{
    DBG_ASSERT( mxNode.is(), "empty XNode not allowed" );
    return mxNode->getOwnerDocument();
}

SvXMLImportContext* DomBuilderContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList>& )
{
    
    return new DomBuilderContext( GetImport(), nPrefix, rLocalName, mxNode );
}


void DomBuilderContext::StartElement(
    const Reference<XAttributeList>& xAttrList )
{
    DBG_ASSERT( mxNode.is(), "empty XNode not allowed" );
    DBG_ASSERT( mxNode->getOwnerDocument().is(), "XNode must have XDocument" );

    
    sal_Int16 nAttributeCount = xAttrList->getLength();
    for( sal_Int16 i = 0; i < nAttributeCount; i++ )
    {
        
        const OUString& rName = xAttrList->getNameByIndex( i );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        
        OUString sNamespace;
        sal_uInt16 nNamespaceKey =
            GetImport().GetNamespaceMap()._GetKeyByAttrName(
                rName, NULL, NULL, &sNamespace );

        
        Reference<XElement> xElement( mxNode, UNO_QUERY_THROW );
        switch( nNamespaceKey )
        {
        case XML_NAMESPACE_NONE:
            
            xElement->setAttribute( rName, rValue );
            break;
        case XML_NAMESPACE_XMLNS:
            
            
            break;
        case XML_NAMESPACE_UNKNOWN:
            
            {
                Sequence<OUString> aSeq(2);
                aSeq[0] = rName;
                aSeq[1] = rValue;
                GetImport().SetError(
                    XMLERROR_FLAG_WARNING | XMLERROR_NAMESPACE_TROUBLE, aSeq );
            }
            break;
        default:
            
            xElement->setAttributeNS( sNamespace, rName, rValue );
            break;
        }
    }
}

void DomBuilderContext::EndElement()
{
    
}

void DomBuilderContext::Characters( const OUString& rCharacters )
{
    DBG_ASSERT( mxNode.is(), "empty XNode not allowed" );

    
    
    

    
    Reference<XNode> xNew(
        mxNode->getOwnerDocument()->createTextNode( rCharacters ),
        UNO_QUERY_THROW );
    mxNode->appendChild( xNew );
}


//

//

static Reference<XNode> lcl_createDomInstance()
{
    Reference<XComponentContext> xContext = comphelper::getProcessComponentContext();
    DBG_ASSERT( xContext.is(), "can't get service factory" );

    Reference<XDocumentBuilder> xBuilder( DocumentBuilder::create(xContext) );

    return Reference<XNode>( xBuilder->newDocument(), UNO_QUERY_THROW );
}

static Reference<XNode> lcl_createElement( SvXMLImport& rImport,
                                    sal_uInt16 nPrefix,
                                    const OUString rLocalName,
                                    Reference<XNode> xParent)
{
    DBG_ASSERT( xParent.is(), "need parent node" );

    Reference<XDocument> xDocument = xParent->getOwnerDocument();
    DBG_ASSERT( xDocument.is(), "no XDocument found!" );

    
    
    

    Reference<XElement> xElement;
    switch( nPrefix )
    {
    case XML_NAMESPACE_NONE:
        
        xElement = xDocument->createElement( rLocalName );
        break;
    case XML_NAMESPACE_XMLNS:
    case XML_NAMESPACE_UNKNOWN:
        
        xElement = xDocument->createElement( rLocalName );
        {
            Sequence<OUString> aSeq(1);
            aSeq[0] = rLocalName;
            rImport.SetError(
                XMLERROR_FLAG_WARNING | XMLERROR_NAMESPACE_TROUBLE, aSeq );
        }
        break;
    default:
        
        
        
        
        xElement = xDocument->createElementNS(
            rImport.GetNamespaceMap().GetNameByKey( nPrefix ),
            rImport.GetNamespaceMap().GetQNameByKey( nPrefix, rLocalName ) );
        break;
    }
    DBG_ASSERT( xElement.is(), "can't create element" );

    
    xParent->appendChild( xElement );
    return xElement;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
