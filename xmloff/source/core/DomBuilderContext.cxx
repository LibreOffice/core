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


#include <DomBuilderContext.hxx>

#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlerror.hxx>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/xml/dom/DocumentBuilder.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XElement.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/xml/dom/NodeType.hpp>

#include <rtl/ustring.hxx>
#include <sal/log.hxx>

#include <comphelper/processfactory.hxx>


using com::sun::star::uno::XComponentContext;
using com::sun::star::uno::Reference;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::UNO_QUERY_THROW;
using com::sun::star::xml::dom::DocumentBuilder;
using com::sun::star::xml::dom::XDocument;
using com::sun::star::xml::dom::XDocumentBuilder;
using com::sun::star::xml::dom::XNode;
using com::sun::star::xml::dom::XElement;
using com::sun::star::xml::sax::XAttributeList;
using com::sun::star::xml::dom::NodeType_ELEMENT_NODE;


// helper functions; implemented below
static Reference<XNode> lcl_createDomInstance();
static Reference<XNode> lcl_createElement( SvXMLImport& rImport,
                                    sal_uInt16 nPrefix,
                                    const OUString& rLocalName,
                                    const Reference<XNode>& xParent);


DomBuilderContext::DomBuilderContext( SvXMLImport& rImport,
                                      sal_uInt16 nPrefix,
                                      const OUString& rLocalName ) :
    SvXMLImportContext( rImport, nPrefix, rLocalName ),
    mxNode( lcl_createElement( rImport, nPrefix, rLocalName,
                               lcl_createDomInstance() ) )
{
    SAL_WARN_IF( !mxNode.is(), "xmloff", "empty XNode not allowed" );
    SAL_WARN_IF( !Reference<XElement>( mxNode, UNO_QUERY ).is(), "xmloff", "need element" );
    SAL_WARN_IF( mxNode->getNodeType() != NodeType_ELEMENT_NODE, "xmloff", "need element" );
}

DomBuilderContext::DomBuilderContext( SvXMLImport& rImport,
                                      sal_uInt16 nPrefix,
                                      const OUString& rLocalName,
                                      Reference<XNode> const & xParent ) :
    SvXMLImportContext( rImport, nPrefix, rLocalName ),
    mxNode( lcl_createElement( rImport, nPrefix, rLocalName, xParent ) )
{
    SAL_WARN_IF( !mxNode.is(), "xmloff", "empty XNode not allowed" );
    SAL_WARN_IF( !Reference<XElement>( mxNode, UNO_QUERY ).is(), "xmloff", "need element" );
    SAL_WARN_IF( mxNode->getNodeType() != NodeType_ELEMENT_NODE, "xmloff", "need element" );
}

DomBuilderContext::~DomBuilderContext()
{
}

Reference<XDocument> DomBuilderContext::getTree()
{
    SAL_WARN_IF( !mxNode.is(), "xmloff", "empty XNode not allowed" );
    return mxNode->getOwnerDocument();
}

SvXMLImportContextRef DomBuilderContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList>& )
{
    // create DomBuilder for subtree
    return new DomBuilderContext( GetImport(), nPrefix, rLocalName, mxNode );
}


void DomBuilderContext::StartElement(
    const Reference<XAttributeList>& xAttrList )
{
    SAL_WARN_IF( !mxNode.is(), "xmloff", "empty XNode not allowed" );
    SAL_WARN_IF( !mxNode->getOwnerDocument().is(), "xmloff", "XNode must have XDocument" );

    // add attribute nodes to new node
    sal_Int16 nAttributeCount = xAttrList->getLength();
    for( sal_Int16 i = 0; i < nAttributeCount; i++ )
    {
        // get name & value for attribute
        const OUString& rName = xAttrList->getNameByIndex( i );
        const OUString& rValue = xAttrList->getValueByIndex( i );

        // namespace handling: determine namespace & namespace key
        OUString sNamespace;
        sal_uInt16 nNamespaceKey =
            GetImport().GetNamespaceMap().GetKeyByAttrName(
                rName, nullptr, nullptr, &sNamespace);

        // create attribute node and set value
        Reference<XElement> xElement( mxNode, UNO_QUERY_THROW );
        switch( nNamespaceKey )
        {
        case XML_NAMESPACE_NONE:
            // no namespace: create a non-namespaced attribute
            xElement->setAttribute( rName, rValue );
            break;
        case XML_NAMESPACE_XMLNS:
            // namespace declaration: ignore, since the DOM tree handles these
            // declarations implicitly
            break;
        case XML_NAMESPACE_UNKNOWN:
            // unknown namespace: illegal input. Raise Warning.
            {
                Sequence<OUString> aSeq(2);
                aSeq[0] = rName;
                aSeq[1] = rValue;
                GetImport().SetError(
                    XMLERROR_FLAG_WARNING | XMLERROR_NAMESPACE_TROUBLE, aSeq );
            }
            break;
        default:
            // a real and proper namespace: create namespaced attribute
            xElement->setAttributeNS( sNamespace, rName, rValue );
            break;
        }
    }
}

void DomBuilderContext::EndElement()
{
    // nothing to be done!
}

void DomBuilderContext::Characters( const OUString& rCharacters )
{
    SAL_WARN_IF( !mxNode.is(), "xmloff", "empty XNode not allowed" );

    // TODO: I assume adjacent text nodes should be joined, to preserve
    // processing model? (I.e., if the SAX parser breaks a string into 2
    // Characters(..) calls, the DOM model would still see only one child.)

    // create text node and append to parent
    Reference<XNode> xNew(
        mxNode->getOwnerDocument()->createTextNode( rCharacters ),
        UNO_QUERY_THROW );
    mxNode->appendChild( xNew );
}


// helper function implementations


static Reference<XNode> lcl_createDomInstance()
{
    Reference<XComponentContext> xContext = comphelper::getProcessComponentContext();
    SAL_WARN_IF( !xContext.is(), "xmloff", "can't get service factory" );

    Reference<XDocumentBuilder> xBuilder( DocumentBuilder::create(xContext) );

    return Reference<XNode>( xBuilder->newDocument(), UNO_QUERY_THROW );
}

static Reference<XNode> lcl_createElement( SvXMLImport& rImport,
                                    sal_uInt16 nPrefix,
                                    const OUString& rLocalName,
                                    const Reference<XNode>& xParent)
{
    SAL_WARN_IF( !xParent.is(), "xmloff", "need parent node" );

    Reference<XDocument> xDocument = xParent->getOwnerDocument();
    SAL_WARN_IF( !xDocument.is(), "xmloff", "no XDocument found!" );

    // TODO: come up with proper way of handling namespaces; re-creating the
    // namespace from the key is NOT a good idea, and will not work for
    // multiple prefixes for the same namespace. Fortunately, those are rare.

    Reference<XElement> xElement;
    switch( nPrefix )
    {
    case XML_NAMESPACE_NONE:
        // no namespace: use local name
        xElement = xDocument->createElement( rLocalName );
        break;
    case XML_NAMESPACE_XMLNS:
    case XML_NAMESPACE_UNKNOWN:
        // both cases are illegal; raise warning (and use only local name)
        xElement = xDocument->createElement( rLocalName );
        {
            Sequence<OUString> aSeq { rLocalName };
            rImport.SetError(
                XMLERROR_FLAG_WARNING | XMLERROR_NAMESPACE_TROUBLE, aSeq );
        }
        break;
    default:
        // We are only given the prefix and the local name; thus we have to ask
        // the namespace map to create a qualified name for us. Technically,
        // this is a bug, since this will fail for multiple prefixes used for
        // the same namespace.
        xElement = xDocument->createElementNS(
            rImport.GetNamespaceMap().GetNameByKey( nPrefix ),
            rImport.GetNamespaceMap().GetQNameByKey( nPrefix, rLocalName ) );
        break;
    }
    SAL_WARN_IF( !xElement.is(), "xmloff", "can't create element" );

    // add new element to parent and return
    xParent->appendChild( xElement );
    return xElement;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
