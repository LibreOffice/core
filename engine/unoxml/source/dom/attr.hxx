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

#include <memory>
#include <optional>

#include <libxml/tree.h>

#include <cppuhelper/implbase.hxx>

#include <cpo/uno/Reference.h>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XAttr.hpp>

#include <node.hxx>

namespace DOM
{
    typedef ::std::pair< OString, OString > stringpair_t;

    typedef ::cppu::ImplInheritanceHelper< CNode, css::xml::dom::XAttr > CAttr_Base;

    class CAttr
        : public CAttr_Base
    {
        friend class CDocument;

        xmlAttrPtr m_aAttrPtr;
        ::std::optional< stringpair_t > m_oNamespace;

        CAttr(CDocument const& rDocument, ::osl::Mutex const& rMutex,
                xmlAttrPtr const pAttr);

    public:
        /// return the libxml namespace corresponding to m_pNamespace on pNode
        xmlNsPtr GetNamespace(xmlNodePtr const pNode);

        virtual bool IsChildTypeAllowed(css::xml::dom::NodeType const nodeType,
                        css::xml::dom::NodeType const*) override;

        /**
        Returns the name of this attribute.
        */
        virtual OUString getName() override;

        /**
        The Element node this attribute is attached to or null if this
        attribute is not in use.
        */
        virtual cpo::uno::Reference< css::xml::dom::XElement > getOwnerElement() override;

        /**
        If this attribute was explicitly given a value in the original
        document, this is true; otherwise, it is false.
        */
        virtual bool getSpecified() override;

        /**
        On retrieval, the value of the attribute is returned as a string.
        */
        virtual OUString getValue() override;

        /**
        Sets the value of the attribute from a string.
        */

        virtual void setValue(const OUString& value) override;

        // resolve uno inheritance problems...
        // overrides for XNode base
        virtual OUString getNodeName() override;
        virtual OUString getNodeValue() override;
        virtual OUString getLocalName() override;

    // --- delegation for XNode base.
    virtual cpo::uno::Reference< css::xml::dom::XNode > appendChild(const cpo::uno::Reference< css::xml::dom::XNode >& newChild) override
    {
        return CNode::appendChild(newChild);
    }
    virtual cpo::uno::Reference< css::xml::dom::XNode > cloneNode(bool deep) override
    {
        return CNode::cloneNode(deep);
    }
    virtual cpo::uno::Reference< css::xml::dom::XNamedNodeMap > getAttributes() override
    {
        return CNode::getAttributes();
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
    virtual OUString getNamespaceURI() override;
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
    virtual OUString getPrefix() override;
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
        return setValue(nodeValue);
    }
    virtual void setPrefix(const OUString& prefix) override;

    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
