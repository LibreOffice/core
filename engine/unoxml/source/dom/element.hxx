/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <cpo/uno/Reference.h>
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

        cpo::uno::Reference< css::xml::dom::XAttr > setAttributeNode_Impl_Lock(
                cpo::uno::Reference< css::xml::dom::XAttr > const& xNewAttr, bool const bNS);

    protected:
        CElement(CDocument const& rDocument, ::osl::Mutex const& rMutex,
                xmlNodePtr const pNode);

    public:

        virtual void saxify(const cpo::uno::Reference< css::xml::sax::XDocumentHandler >& i_xHandler) override;

        virtual void fastSaxify( Context& i_rContext ) override;

        virtual bool IsChildTypeAllowed(css::xml::dom::NodeType const nodeType,
                        css::xml::dom::NodeType const*) override;

        /**
        Retrieves an attribute value by name.
        */
        virtual OUString  getAttribute(const OUString& name) override;

        /**
        Retrieves an attribute node by name.
        */
        virtual cpo::uno::Reference< css::xml::dom::XAttr > getAttributeNode(const OUString& name) override;

        /**
        Retrieves an Attr node by local name and namespace URI.
        */
        virtual cpo::uno::Reference< css::xml::dom::XAttr > getAttributeNodeNS(const OUString& namespaceURI, const OUString& localName) override;

        /**
        Retrieves an attribute value by local name and namespace URI.
        */
        virtual OUString getAttributeNS(const OUString& namespaceURI, const OUString& localName) override;

        /**
        Returns a NodeList of all descendant Elements with a given tag name,
        in the order in which they are
        encountered in a preorder traversal of this Element tree.
        */
        virtual cpo::uno::Reference< css::xml::dom::XNodeList > getElementsByTagName(const OUString& name) override;

        /**
        Returns a NodeList of all the descendant Elements with a given local
        name and namespace URI in the order in which they are encountered in
        a preorder traversal of this Element tree.
        */
        virtual cpo::uno::Reference< css::xml::dom::XNodeList > getElementsByTagNameNS(const OUString& namespaceURI,
                const OUString& localName) override;

        /**
        The name of the element.
        */
        virtual OUString getTagName() override;

        /**
        Returns true when an attribute with a given name is specified on this
        element or has a default value, false otherwise.
        */
        virtual bool hasAttribute(const OUString& name) override;

        /**
        Returns true when an attribute with a given local name and namespace
        URI is specified on this element or has a default value, false otherwise.
        */
        virtual bool hasAttributeNS(const OUString& namespaceURI, const OUString& localName) override;

        /**
        Removes an attribute by name.
        */
        virtual void removeAttribute(const OUString& name) override;

        /**
        Removes the specified attribute node.
        */
        virtual cpo::uno::Reference< css::xml::dom::XAttr > removeAttributeNode(const cpo::uno::Reference< css::xml::dom::XAttr >& oldAttr) override;

        /**
        Removes an attribute by local name and namespace URI.
        */
        virtual void removeAttributeNS(const OUString& namespaceURI, const OUString& localName) override;

        /**
        Adds a new attribute.
        */
        virtual void setAttribute(const OUString& name, const OUString& value) override;

        /**
        Adds a new attribute node.
        */
        virtual cpo::uno::Reference< css::xml::dom::XAttr > setAttributeNode(const cpo::uno::Reference< css::xml::dom::XAttr >& newAttr) override;

        /**
        Adds a new attribute.
        */
        virtual cpo::uno::Reference< css::xml::dom::XAttr > setAttributeNodeNS(const cpo::uno::Reference< css::xml::dom::XAttr >& newAttr) override;

        /**
        Adds a new attribute.
        */
        virtual void setAttributeNS(
                const OUString& namespaceURI, const OUString& qualifiedName, const OUString& value) override;

        // overrides for XNode base
        virtual OUString getNodeName() override;
        virtual OUString getNodeValue() override;
        virtual cpo::uno::Reference< css::xml::dom::XNamedNodeMap > getAttributes() override;
        virtual OUString getLocalName() override;

        // resolve uno inheritance problems...
        // --- delegation for XNode base.
        virtual cpo::uno::Reference< css::xml::dom::XNode > appendChild(const cpo::uno::Reference< css::xml::dom::XNode >& newChild) override
        {
            return CNode::appendChild(newChild);
        }
        virtual cpo::uno::Reference< css::xml::dom::XNode > cloneNode(bool deep) override
        {
            return CNode::cloneNode(deep);
        }
        virtual cpo::uno::Reference< css::xml::dom::XNodeList > getChildNodes() override
        {
            return CNode::getChildNodes();
        }
        virtual cpo::uno::Reference< css::xml::dom::XNode > getFirstChild() override
        {
            return CNode::getFirstChild();
        }
        virtual cpo::uno::Reference< css::xml::dom::XNode > getLastChild() override
        {
            return CNode::getLastChild();
        }
        virtual OUString getNamespaceURI() override
        {
            return CNode::getNamespaceURI();
        }
        virtual cpo::uno::Reference< css::xml::dom::XNode > getNextSibling() override
        {
            return CNode::getNextSibling();
        }
        virtual css::xml::dom::NodeType getNodeType() override
        {
            return CNode::getNodeType();
        }
        virtual cpo::uno::Reference< css::xml::dom::XDocument > getOwnerDocument() override
        {
            return CNode::getOwnerDocument();
        }
        virtual cpo::uno::Reference< css::xml::dom::XNode > getParentNode() override
        {
            return CNode::getParentNode();
        }
        virtual OUString getPrefix() override
        {
            return CNode::getPrefix();
        }
        virtual cpo::uno::Reference< css::xml::dom::XNode > getPreviousSibling() override
        {
            return CNode::getPreviousSibling();
        }
        virtual bool hasAttributes() override
        {
            return CNode::hasAttributes();
        }
        virtual bool hasChildNodes() override
        {
            return CNode::hasChildNodes();
        }
        virtual cpo::uno::Reference< css::xml::dom::XNode > insertBefore(
                const cpo::uno::Reference< css::xml::dom::XNode >& newChild, const cpo::uno::Reference< css::xml::dom::XNode >& refChild) override
        {
            return CNode::insertBefore(newChild, refChild);
        }
        virtual bool isSupported(const OUString& feature, const OUString& ver) override
        {
            return CNode::isSupported(feature, ver);
        }
        virtual void normalize() override
        {
            CNode::normalize();
        }
        virtual cpo::uno::Reference< css::xml::dom::XNode > removeChild(const cpo::uno::Reference< css::xml::dom::XNode >& oldChild) override
        {
            return CNode::removeChild(oldChild);
        }
        virtual cpo::uno::Reference< css::xml::dom::XNode > replaceChild(
                const cpo::uno::Reference< css::xml::dom::XNode >& newChild, const cpo::uno::Reference< css::xml::dom::XNode >& oldChild) override
        {
            return CNode::replaceChild(newChild, oldChild);
        }
        virtual void setNodeValue(const OUString& nodeValue) override
        {
            return CNode::setNodeValue(nodeValue);
        }
        virtual void setPrefix(const OUString& prefix) override
        {
            return CNode::setPrefix(prefix);
        }

    };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
