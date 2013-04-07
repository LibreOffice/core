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

#ifndef DOM_ELEMENT_HXX
#define DOM_ELEMENT_HXX

#include <libxml/tree.h>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XNodeList.hpp>
#include <com/sun/star/xml/dom/XNamedNodeMap.hpp>
#include <com/sun/star/xml/dom/NodeType.hpp>

#include <node.hxx>


using namespace com::sun::star::uno;
using namespace com::sun::star::xml::dom;

namespace DOM
{
    typedef ::cppu::ImplInheritanceHelper1<CNode, XElement > CElement_Base;

    class CElement
        : public CElement_Base
    {
    private:
        friend class CDocument;

        Reference< XAttr > setAttributeNode_Impl_Lock(
                Reference< XAttr > const& xNewAttr, bool const bNS);

    protected:
        CElement(CDocument const& rDocument, ::osl::Mutex const& rMutex,
                xmlNodePtr const pNode);

    public:

        virtual void saxify(const Reference< XDocumentHandler >& i_xHandler);

        virtual void fastSaxify( Context& i_rContext );

        virtual bool IsChildTypeAllowed(NodeType const nodeType);

        /**
        Retrieves an attribute value by name.
        */
        virtual OUString  SAL_CALL getAttribute(const OUString& name)
            throw (RuntimeException);

        /**
        Retrieves an attribute node by name.
        */
        virtual Reference< XAttr > SAL_CALL getAttributeNode(const OUString& name)
            throw (RuntimeException);

        /**
        Retrieves an Attr node by local name and namespace URI.
        */
        virtual Reference< XAttr > SAL_CALL getAttributeNodeNS(const OUString& namespaceURI, const OUString& localName)
            throw (RuntimeException);

        /**
        Retrieves an attribute value by local name and namespace URI.
        */
        virtual OUString SAL_CALL getAttributeNS(const OUString& namespaceURI, const OUString& localName)
            throw (RuntimeException);

        /**
        Returns a NodeList of all descendant Elements with a given tag name,
        in the order in which they are
        encountered in a preorder traversal of this Element tree.
        */
        virtual Reference< XNodeList > SAL_CALL getElementsByTagName(const OUString& name)
            throw (RuntimeException);

        /**
        Returns a NodeList of all the descendant Elements with a given local
        name and namespace URI in the order in which they are encountered in
        a preorder traversal of this Element tree.
        */
        virtual Reference< XNodeList > SAL_CALL getElementsByTagNameNS(const OUString& namespaceURI,
                const OUString& localName)
            throw (RuntimeException);

        /**
        The name of the element.
        */
        virtual OUString SAL_CALL getTagName()
            throw (RuntimeException);

        /**
        Returns true when an attribute with a given name is specified on this
        element or has a default value, false otherwise.
        */
        virtual sal_Bool SAL_CALL hasAttribute(const OUString& name)
            throw (RuntimeException);

        /**
        Returns true when an attribute with a given local name and namespace
        URI is specified on this element or has a default value, false otherwise.
        */
        virtual sal_Bool SAL_CALL hasAttributeNS(const OUString& namespaceURI, const OUString& localName)
            throw (RuntimeException);

        /**
        Removes an attribute by name.
        */
        virtual void SAL_CALL removeAttribute(const OUString& name)
            throw (RuntimeException, DOMException);

        /**
        Removes the specified attribute node.
        */
        virtual Reference< XAttr > SAL_CALL removeAttributeNode(const Reference< XAttr >& oldAttr)
            throw (RuntimeException, DOMException);

        /**
        Removes an attribute by local name and namespace URI.
        */
        virtual void SAL_CALL removeAttributeNS(const OUString& namespaceURI, const OUString& localName)
            throw (RuntimeException, DOMException);

        /**
        Adds a new attribute.
        */
        virtual void SAL_CALL setAttribute(const OUString& name, const OUString& value)
            throw (RuntimeException, DOMException);

        /**
        Adds a new attribute node.
        */
        virtual Reference< XAttr > SAL_CALL setAttributeNode(const Reference< XAttr >& newAttr)
            throw (RuntimeException, DOMException);

        /**
        Adds a new attribute.
        */
        virtual Reference< XAttr > SAL_CALL setAttributeNodeNS(const Reference< XAttr >& newAttr)
            throw (RuntimeException, DOMException);

        /**
        Adds a new attribute.
        */
        virtual void SAL_CALL setAttributeNS(
                const OUString& namespaceURI, const OUString& qualifiedName, const OUString& value)
            throw (RuntimeException, DOMException);

        /**
        sets the element name
        */
        virtual void SAL_CALL setElementName(const OUString& elementName)
            throw (RuntimeException, DOMException);

        // overrides for XNode base
        virtual OUString SAL_CALL getNodeName()
            throw (RuntimeException);
        virtual OUString SAL_CALL getNodeValue()
            throw (RuntimeException);
        virtual Reference< XNamedNodeMap > SAL_CALL getAttributes()
            throw (RuntimeException);
        virtual OUString SAL_CALL getLocalName()
            throw (RuntimeException);

        // resolve uno inheritance problems...
        // --- delegation for XNde base.
        virtual Reference< XNode > SAL_CALL appendChild(const Reference< XNode >& newChild)
            throw (RuntimeException, DOMException)
        {
            return CNode::appendChild(newChild);
        }
        virtual Reference< XNode > SAL_CALL cloneNode(sal_Bool deep)
            throw (RuntimeException)
        {
            return CNode::cloneNode(deep);
        }
        virtual Reference< XNodeList > SAL_CALL getChildNodes()
            throw (RuntimeException)
        {
            return CNode::getChildNodes();
        }
        virtual Reference< XNode > SAL_CALL getFirstChild()
            throw (RuntimeException)
        {
            return CNode::getFirstChild();
        }
        virtual Reference< XNode > SAL_CALL getLastChild()
            throw (RuntimeException)
        {
            return CNode::getLastChild();
        }
        virtual OUString SAL_CALL getNamespaceURI()
            throw (RuntimeException)
        {
            return CNode::getNamespaceURI();
        }
        virtual Reference< XNode > SAL_CALL getNextSibling()
            throw (RuntimeException)
        {
            return CNode::getNextSibling();
        }
        virtual NodeType SAL_CALL getNodeType()
            throw (RuntimeException)
        {
            return CNode::getNodeType();
        }
        virtual Reference< XDocument > SAL_CALL getOwnerDocument()
            throw (RuntimeException)
        {
            return CNode::getOwnerDocument();
        }
        virtual Reference< XNode > SAL_CALL getParentNode()
            throw (RuntimeException)
        {
            return CNode::getParentNode();
        }
        virtual OUString SAL_CALL getPrefix()
            throw (RuntimeException)
        {
            return CNode::getPrefix();
        }
        virtual Reference< XNode > SAL_CALL getPreviousSibling()
            throw (RuntimeException)
        {
            return CNode::getPreviousSibling();
        }
        virtual sal_Bool SAL_CALL hasAttributes()
            throw (RuntimeException)
        {
            return CNode::hasAttributes();
        }
        virtual sal_Bool SAL_CALL hasChildNodes()
            throw (RuntimeException)
        {
            return CNode::hasChildNodes();
        }
        virtual Reference< XNode > SAL_CALL insertBefore(
                const Reference< XNode >& newChild, const Reference< XNode >& refChild)
            throw (RuntimeException, DOMException)
        {
            return CNode::insertBefore(newChild, refChild);
        }
        virtual sal_Bool SAL_CALL isSupported(const OUString& feature, const OUString& ver)
            throw (RuntimeException)
        {
            return CNode::isSupported(feature, ver);
        }
        virtual void SAL_CALL normalize()
            throw (RuntimeException)
        {
            CNode::normalize();
        }
        virtual Reference< XNode > SAL_CALL removeChild(const Reference< XNode >& oldChild)
            throw (RuntimeException, DOMException)
        {
            return CNode::removeChild(oldChild);
        }
        virtual Reference< XNode > SAL_CALL replaceChild(
                const Reference< XNode >& newChild, const Reference< XNode >& oldChild)
            throw (RuntimeException, DOMException)
        {
            return CNode::replaceChild(newChild, oldChild);
        }
        virtual void SAL_CALL setNodeValue(const OUString& nodeValue)
            throw (RuntimeException, DOMException)
        {
            return CNode::setNodeValue(nodeValue);
        }
        virtual void SAL_CALL setPrefix(const OUString& prefix)
            throw (RuntimeException, DOMException)
        {
            return CNode::setPrefix(prefix);
        }

    };

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
