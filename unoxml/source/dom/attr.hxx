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

#include <memory>

#include <libxml/tree.h>

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/uno/Reference.h>
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
        ::std::unique_ptr< stringpair_t > m_pNamespace;

        CAttr(CDocument const& rDocument, ::osl::Mutex const& rMutex,
                xmlAttrPtr const pAttr);

    public:
        /// return the libxml namespace corresponding to m_pNamespace on pNode
        xmlNsPtr GetNamespace(xmlNodePtr const pNode);

        virtual bool IsChildTypeAllowed(css::xml::dom::NodeType const nodeType) override;

        /**
        Returns the name of this attribute.
        */
        virtual OUString SAL_CALL getName() override;

        /**
        The Element node this attribute is attached to or null if this
        attribute is not in use.
        */
        virtual css::uno::Reference< css::xml::dom::XElement > SAL_CALL getOwnerElement() override;

        /**
        If this attribute was explicitly given a value in the original
        document, this is true; otherwise, it is false.
        */
        virtual sal_Bool SAL_CALL getSpecified() override;

        /**
        On retrieval, the value of the attribute is returned as a string.
        */
        virtual OUString SAL_CALL getValue() override;

        /**
        Sets the value of the attribute from a string.
        */

        virtual void SAL_CALL setValue(const OUString& value) override;

        // resolve uno inheritance problems...
        // overrides for XNode base
        virtual OUString SAL_CALL getNodeName() override;
        virtual OUString SAL_CALL getNodeValue() override;
        virtual OUString SAL_CALL getLocalName() override;

    // --- delegation for XNode base.
    virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL appendChild(const css::uno::Reference< css::xml::dom::XNode >& newChild) override
    {
        return CNode::appendChild(newChild);
    }
    virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL cloneNode(sal_Bool deep) override
    {
        return CNode::cloneNode(deep);
    }
    virtual css::uno::Reference< css::xml::dom::XNamedNodeMap > SAL_CALL getAttributes() override
    {
        return CNode::getAttributes();
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
    virtual OUString SAL_CALL getNamespaceURI() override;
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
    virtual OUString SAL_CALL getPrefix() override;
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
        return setValue(nodeValue);
    }
    virtual void SAL_CALL setPrefix(const OUString& prefix) override;

    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
