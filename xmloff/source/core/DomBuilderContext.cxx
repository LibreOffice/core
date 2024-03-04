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
using com::sun::star::xml::dom::NodeType_ELEMENT_NODE;


// helper functions; implemented below
static Reference<XNode> lcl_createDomInstance();
static Reference<XNode> lcl_createElement( SvXMLImport& rImport,
                                    sal_Int32 nElement,
                                    const Reference<XNode>& xParent);
static Reference<XNode> lcl_createElement(
                                    const OUString & rNamespace, const OUString & rName,
                                    const Reference<XNode>& xParent);

DomBuilderContext::DomBuilderContext( SvXMLImport& rImport,
                                      sal_Int32 nElement ) :
    SvXMLImportContext( rImport ),
    mxNode( lcl_createElement( rImport, nElement,
                               lcl_createDomInstance() ) )
{
    SAL_WARN_IF( !mxNode.is(), "xmloff", "empty XNode not allowed" );
    SAL_WARN_IF( !Reference<XElement>( mxNode, UNO_QUERY ).is(), "xmloff", "need element" );
    SAL_WARN_IF( mxNode->getNodeType() != NodeType_ELEMENT_NODE, "xmloff", "need element" );
}

DomBuilderContext::DomBuilderContext( SvXMLImport& rImport,
                                      const OUString & rNamespace, const OUString & rName ) :
    SvXMLImportContext( rImport ),
    mxNode( lcl_createElement( rNamespace, rName,
                               lcl_createDomInstance() ) )
{
    SAL_WARN_IF( !mxNode.is(), "xmloff", "empty XNode not allowed" );
    SAL_WARN_IF( !Reference<XElement>( mxNode, UNO_QUERY ).is(), "xmloff", "need element" );
    SAL_WARN_IF( mxNode->getNodeType() != NodeType_ELEMENT_NODE, "xmloff", "need element" );
}

DomBuilderContext::DomBuilderContext( SvXMLImport& rImport,
                                      sal_Int32 nElement,
                                      Reference<XNode> const & xParent ) :
    SvXMLImportContext( rImport ),
    mxNode( lcl_createElement( rImport, nElement, xParent ) )
{
    SAL_WARN_IF( !mxNode.is(), "xmloff", "empty XNode not allowed" );
    SAL_WARN_IF( !Reference<XElement>( mxNode, UNO_QUERY ).is(), "xmloff", "need element" );
    SAL_WARN_IF( mxNode->getNodeType() != NodeType_ELEMENT_NODE, "xmloff", "need element" );
}

DomBuilderContext::DomBuilderContext( SvXMLImport& rImport,
                                      const OUString & rNamespace, const OUString & rName,
                                      Reference<XNode> const & xParent ) :
    SvXMLImportContext( rImport ),
    mxNode( lcl_createElement( rNamespace, rName, xParent ) )
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

css::uno::Reference< css::xml::sax::XFastContextHandler > DomBuilderContext::createFastChildContext(
    sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >&  )
{
    // create DomBuilder for subtree
    return new DomBuilderContext( GetImport(), nElement, mxNode );
}

css::uno::Reference< css::xml::sax::XFastContextHandler > DomBuilderContext::createUnknownChildContext(
    const OUString & rNamespace, const OUString &rName, const css::uno::Reference< css::xml::sax::XFastAttributeList >&  )
{
    // create DomBuilder for subtree
    return new DomBuilderContext( GetImport(), rNamespace, rName, mxNode );
}

void SAL_CALL DomBuilderContext::startFastElement(
    sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    SAL_WARN_IF( !mxNode.is(), "xmloff", "empty XNode not allowed" );
    SAL_WARN_IF( !mxNode->getOwnerDocument().is(), "xmloff", "XNode must have XDocument" );

    HandleAttributes(xAttrList);
}

void SAL_CALL DomBuilderContext::startUnknownElement(
    const OUString & /*rNamespace*/, const OUString & /*rName*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    SAL_WARN_IF( !mxNode.is(), "xmloff", "empty XNode not allowed" );
    SAL_WARN_IF( !mxNode->getOwnerDocument().is(), "xmloff", "XNode must have XDocument" );
    HandleAttributes(xAttrList);
}

void DomBuilderContext::HandleAttributes(
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    // add attribute nodes to new node
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        sal_Int32 nAttrToken = aIter.getToken();
        // get name & value for attribute
        sal_uInt16 nNamespace = (nAttrToken >> NMSP_SHIFT) - 1;
        const OUString& rPrefix = SvXMLImport::getNamespacePrefixFromToken(nAttrToken, &GetImport().GetNamespaceMap());
        const OUString& rLocalName = SvXMLImport::getNameFromToken( nAttrToken );
        OUString aValue = aIter.toString();

        // create attribute node and set value
        Reference<XElement> xElement( mxNode, UNO_QUERY_THROW );
        switch( nNamespace )
        {
        case XML_NAMESPACE_NONE:
            // no namespace: create a non-namespaced attribute
            xElement->setAttribute( rLocalName, aValue );
            break;
        case XML_NAMESPACE_XMLNS:
            // namespace declaration: ignore, since the DOM tree handles these
            // declarations implicitly
            break;
        case XML_NAMESPACE_UNKNOWN:
            // unknown namespace: illegal input. Raise Warning.
            {
                GetImport().SetError(
                    XMLERROR_FLAG_WARNING | XMLERROR_NAMESPACE_TROUBLE, { rLocalName, aValue } );
            }
            break;
        default:
            {
                // a real and proper namespace: create namespaced attribute
                OUString namespaceURI = SvXMLImport::getNamespaceURIFromToken(aIter.getToken());
                OUString qualifiedName = rPrefix.isEmpty() ? rLocalName : rPrefix + SvXMLImport::aNamespaceSeparator + rLocalName;
                xElement->setAttributeNS( namespaceURI, qualifiedName, aValue );
            }
            break;
        }
    }
    const css::uno::Sequence< css::xml::Attribute > unknownAttribs = xAttrList->getUnknownAttributes();
    for ( const auto& rUnknownAttrib : unknownAttribs )
    {
        // create attribute node and set value
        Reference<XElement> xElement( mxNode, UNO_QUERY_THROW );

        if (!rUnknownAttrib.NamespaceURL.isEmpty())
        {
            // unknown namespace: illegal input. Raise Warning.
            GetImport().SetError(
                XMLERROR_FLAG_WARNING | XMLERROR_NAMESPACE_TROUBLE, { rUnknownAttrib.Name, rUnknownAttrib.Value } );
        }
        else
        {
             // no namespace: create a non-namespaced attribute
            xElement->setAttribute( rUnknownAttrib.Name, rUnknownAttrib.Value );
        }
    }
}

void DomBuilderContext::characters( const OUString& rCharacters )
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
                                    sal_Int32 nElement,
                                    const Reference<XNode>& xParent)
{
    SAL_WARN_IF( !xParent.is(), "xmloff", "need parent node" );

    Reference<XDocument> xDocument = xParent->getOwnerDocument();
    SAL_WARN_IF( !xDocument.is(), "xmloff", "no XDocument found!" );

    // TODO: come up with proper way of handling namespaces; re-creating the
    // namespace from the key is NOT a good idea, and will not work for
    // multiple prefixes for the same namespace. Fortunately, those are rare.

    Reference<XElement> xElement;
    sal_uInt16 nNamespace = (nElement >> NMSP_SHIFT) - 1;
    const OUString& rPrefix = SvXMLImport::getNamespacePrefixFromToken(nElement, &rImport.GetNamespaceMap());
    const OUString& rLocalName = SvXMLImport::getNameFromToken( nElement );
    switch( nNamespace )
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
        OUString namespaceURI = SvXMLImport::getNamespaceURIFromToken(nElement);
        OUString qualifiedName = rPrefix.isEmpty() ? rLocalName : rPrefix + SvXMLImport::aNamespaceSeparator + rLocalName;
        xElement = xDocument->createElementNS(namespaceURI, qualifiedName);
        break;
    }
    SAL_WARN_IF( !xElement.is(), "xmloff", "can't create element" );

    // add new element to parent and return
    xParent->appendChild( xElement );
    return xElement;
}

static Reference<XNode> lcl_createElement(
                                    const OUString & rNamespace, const OUString & rName,
                                    const Reference<XNode>& xParent)
{
    SAL_WARN_IF( !xParent.is(), "xmloff", "need parent node" );

    Reference<XDocument> xDocument = xParent->getOwnerDocument();
    SAL_WARN_IF( !xDocument.is(), "xmloff", "no XDocument found!" );

    // TODO: come up with proper way of handling namespaces; re-creating the
    // namespace from the key is NOT a good idea, and will not work for
    // multiple prefixes for the same namespace. Fortunately, those are rare.

    Reference<XElement> xElement;
    if (rNamespace.isEmpty())
    {
        // no namespace: use local name
        xElement = xDocument->createElement( rName );
    }
    else
    {
        xElement = xDocument->createElementNS(rNamespace, rName);
    }

    // add new element to parent and return
    xParent->appendChild( xElement );
    return xElement;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
