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

#pragma once

#include <libxml/tree.h>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XNodeList.hpp>
#include <com/sun/star/xml/dom/XNamedNodeMap.hpp>
#include <com/sun/star/xml/dom/NodeType.hpp>

#include <cppuhelper/implbase.hxx>
#include <node.hxx>

namespace DOM
{
    typedef ::cppu::ImplInheritanceHelper<CNode, css::xml::dom::XElement > CElement_Base;

    class CElement
        : public CElement_Base
    {
    private:
        friend class CDocument;

        css::uno::Reference< css::xml::dom::XAttr > setAttributeNode_Impl_Lock(
                css::uno::Reference< css::xml::dom::XAttr > const& xNewAttr, bool const bNS);

    protected:
        CElement(CDocument const& rDocument, ::osl::Mutex const& rMutex,
                xmlNodePtr const pNode);

    public:

        virtual void saxify(const css::uno::Reference< css::xml::sax::XDocumentHandler >& i_xHandler) override;

        virtual void fastSaxify( Context& i_rContext ) override;

        virtual bool IsChildTypeAllowed(css::xml::dom::NodeType const nodeType) override;

        /**
        Retrieves an attribute value by name.
        */
        virtual OUString  SAL_CALL getAttribute(const OUString& name) override;

        /**
        Retrieves an attribute node by name.
        */
        virtual css::uno::Reference< css::xml::dom::XAttr > SAL_CALL getAttributeNode(const OUString& name) override;

        /**
        Retrieves an Attr node by local name and namespace URI.
        */
        virtual css::uno::Reference< css::xml::dom::XAttr > SAL_CALL getAttributeNodeNS(const OUString& namespaceURI, const OUString& localName) override;

        /**
        Retrieves an attribute value by local name and namespace URI.
        */
        virtual OUString SAL_CALL getAttributeNS(const OUString& namespaceURI, const OUString& localName) override;

        /**
        Returns a NodeList of all descendant Elements with a given tag name,
        in the order in which they are
        encountered in a preorder traversal of this Element tree.
        */
        virtual css::uno::Reference< css::xml::dom::XNodeList > SAL_CALL getElementsByTagName(const OUString& name) override;

        /**
        Returns a NodeList of all the descendant Elements with a given local
        name and namespace URI in the order in which they are encountered in
        a preorder traversal of this Element tree.
        */
        virtual css::uno::Reference< css::xml::dom::XNodeList > SAL_CALL getElementsByTagNameNS(const OUString& namespaceURI,
                const OUString& localName) override;

        /**
        The name of the element.
        */
        virtual OUString SAL_CALL getTagName() override;

        /**
        Returns true when an attribute with a given name is specified on this
        element or has a default value, false otherwise.
        */
        virtual sal_Bool SAL_CALL hasAttribute(const OUString& name) override;

        /**
        Returns true when an attribute with a given local name and namespace
        URI is specified on this element or has a default value, false otherwise.
        */
        virtual sal_Bool SAL_CALL hasAttributeNS(const OUString& namespaceURI, const OUString& localName) override;

        /**
        Removes an attribute by name.
        */
        virtual void SAL_CALL removeAttribute(const OUString& name) override;

        /**
        Removes the specified attribute node.
        */
        virtual css::uno::Reference< css::xml::dom::XAttr > SAL_CALL removeAttributeNode(const css::uno::Reference< css::xml::dom::XAttr >& oldAttr) override;

        /**
        Removes an attribute by local name and namespace URI.
        */
        virtual void SAL_CALL removeAttributeNS(const OUString& namespaceURI, const OUString& localName) override;

        /**
        Adds a new attribute.
        */
        virtual void SAL_CALL setAttribute(const OUString& name, const OUString& value) override;

        /**
        Adds a new attribute node.
        */
        virtual css::uno::Reference< css::xml::dom::XAttr > SAL_CALL setAttributeNode(const css::uno::Reference< css::xml::dom::XAttr >& newAttr) override;

        /**
        Adds a new attribute.
        */
        virtual css::uno::Reference< css::xml::dom::XAttr > SAL_CALL setAttributeNodeNS(const css::uno::Reference< css::xml::dom::XAttr >& newAttr) override;

        /**
        Adds a new attribute.
        */
        virtual void SAL_CALL setAttributeNS(
                const OUString& namespaceURI, const OUString& qualifiedName, const OUString& value) override;

        // overrides for XNode base
        virtual OUString SAL_CALL getNodeName() override;
        virtual OUString SAL_CALL getNodeValue() override;
        virtual css::uno::Reference< css::xml::dom::XNamedNodeMap > SAL_CALL getAttributes() override;
        virtual OUString SAL_CALL getLocalName() override;

        // resolve uno inheritance problems...
        // --- delegation for XNode base.
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL appendChild(const css::uno::Reference< css::xml::dom::XNode >& newChild) override
        {
            return CNode::appendChild(newChild);
        }
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL cloneNode(sal_Bool deep) override
        {
            return CNode::cloneNode(deep);
        }
        virtual css::uno::Reference< css::xml::dom::XNodeList > SAL_CALL getChildNodes() override
        {
            return CNode::getChildNodes();
        }
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL getFirstChild() override
        {
            return CNode::getFirstChild();
        }
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL getLastChild() override
        {
            return CNode::getLastChild();
        }
        virtual OUString SAL_CALL getNamespaceURI() override
        {
            return CNode::getNamespaceURI();
        }
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL getNextSibling() override
        {
            return CNode::getNextSibling();
        }
        virtual css::xml::dom::NodeType SAL_CALL getNodeType() override
        {
            return CNode::getNodeType();
        }
        virtual css::uno::Reference< css::xml::dom::XDocument > SAL_CALL getOwnerDocument() override
        {
            return CNode::getOwnerDocument();
        }
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL getParentNode() override
        {
            return CNode::getParentNode();
        }
        virtual OUString SAL_CALL getPrefix() override
        {
            return CNode::getPrefix();
        }
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL getPreviousSibling() override
        {
            return CNode::getPreviousSibling();
        }
        virtual sal_Bool SAL_CALL hasAttributes() override
        {
            return CNode::hasAttributes();
        }
        virtual sal_Bool SAL_CALL hasChildNodes() override
        {
            return CNode::hasChildNodes();
        }
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL insertBefore(
                const css::uno::Reference< css::xml::dom::XNode >& newChild, const css::uno::Reference< css::xml::dom::XNode >& refChild) override
        {
            return CNode::insertBefore(newChild, refChild);
        }
        virtual sal_Bool SAL_CALL isSupported(const OUString& feature, const OUString& ver) override
        {
            return CNode::isSupported(feature, ver);
        }
        virtual void SAL_CALL normalize() override
        {
            CNode::normalize();
        }
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL removeChild(const css::uno::Reference< css::xml::dom::XNode >& oldChild) override
        {
            return CNode::removeChild(oldChild);
        }
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL replaceChild(
                const css::uno::Reference< css::xml::dom::XNode >& newChild, const css::uno::Reference< css::xml::dom::XNode >& oldChild) override
        {
            return CNode::replaceChild(newChild, oldChild);
        }
        virtual void SAL_CALL setNodeValue(const OUString& nodeValue) override
        {
            return CNode::setNodeValue(nodeValue);
        }
        virtual void SAL_CALL setPrefix(const OUString& prefix) override
        {
            return CNode::setPrefix(prefix);
        }

    };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
