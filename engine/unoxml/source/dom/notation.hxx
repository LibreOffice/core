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
#include <com/sun/star/xml/dom/XNotation.hpp>

#include <cppuhelper/implbase.hxx>
#include <node.hxx>

namespace DOM
{
    typedef cppu::ImplInheritanceHelper< CNode, css::xml::dom::XNotation > CNotation_Base;

    class CNotation
        : public CNotation_Base
    {
    private:
        friend class CDocument;

        CNotation(CDocument const& rDocument, ::osl::Mutex const& rMutex,
                xmlNotationPtr const pNotation);

        /**
        The public identifier of this notation.
        */
        virtual OUString SAL_CALL getPublicId() override;

        /**
        The system identifier of this notation.
        */
        virtual OUString SAL_CALL getSystemId() override;

        // ---- resolve uno inheritance problems...
        // overrides for XNode base
        virtual OUString SAL_CALL getNodeName() override;
        virtual OUString SAL_CALL getNodeValue() override;
    // --- delegation for XNode base.
    virtual cpo::uno::Reference< css::xml::dom::XNode > SAL_CALL appendChild(const cpo::uno::Reference< css::xml::dom::XNode >& newChild) override
    {
        return CNode::appendChild(newChild);
    }
    virtual cpo::uno::Reference< css::xml::dom::XNode > SAL_CALL cloneNode(bool deep) override
    {
        return CNode::cloneNode(deep);
    }
    virtual cpo::uno::Reference< css::xml::dom::XNamedNodeMap > SAL_CALL getAttributes() override
    {
        return CNode::getAttributes();
    }
    virtual cpo::uno::Reference< css::xml::dom::XNodeList > SAL_CALL getChildNodes() override
    {
        return CNode::getChildNodes();
    }
    virtual cpo::uno::Reference< css::xml::dom::XNode > SAL_CALL getFirstChild() override
    {
        return CNode::getFirstChild();
    }
    virtual cpo::uno::Reference< css::xml::dom::XNode > SAL_CALL getLastChild() override
    {
        return CNode::getLastChild();
    }
    virtual OUString SAL_CALL getLocalName() override
    {
        return CNode::getLocalName();
    }
    virtual OUString SAL_CALL getNamespaceURI() override
    {
        return CNode::getNamespaceURI();
    }
    virtual cpo::uno::Reference< css::xml::dom::XNode > SAL_CALL getNextSibling() override
    {
        return CNode::getNextSibling();
    }
    virtual css::xml::dom::NodeType SAL_CALL getNodeType() override
    {
        return CNode::getNodeType();
    }
    virtual cpo::uno::Reference< css::xml::dom::XDocument > SAL_CALL getOwnerDocument() override
    {
        return CNode::getOwnerDocument();
    }
    virtual cpo::uno::Reference< css::xml::dom::XNode > SAL_CALL getParentNode() override
    {
        return CNode::getParentNode();
    }
    virtual OUString SAL_CALL getPrefix() override
    {
        return CNode::getPrefix();
    }
    virtual cpo::uno::Reference< css::xml::dom::XNode > SAL_CALL getPreviousSibling() override
    {
        return CNode::getPreviousSibling();
    }
    virtual bool SAL_CALL hasAttributes() override
    {
        return CNode::hasAttributes();
    }
    virtual bool SAL_CALL hasChildNodes() override
    {
        return CNode::hasChildNodes();
    }
    virtual cpo::uno::Reference< css::xml::dom::XNode > SAL_CALL insertBefore(
            const cpo::uno::Reference< css::xml::dom::XNode >& newChild, const cpo::uno::Reference< css::xml::dom::XNode >& refChild) override
    {
        return CNode::insertBefore(newChild, refChild);
    }
    virtual bool SAL_CALL isSupported(const OUString& feature, const OUString& ver) override
    {
        return CNode::isSupported(feature, ver);
    }
    virtual void SAL_CALL normalize() override
    {
        CNode::normalize();
    }
    virtual cpo::uno::Reference< css::xml::dom::XNode > SAL_CALL removeChild(const cpo::uno::Reference< css::xml::dom::XNode >& oldChild) override
    {
        return CNode::removeChild(oldChild);
    }
    virtual cpo::uno::Reference< css::xml::dom::XNode > SAL_CALL replaceChild(
            const cpo::uno::Reference< css::xml::dom::XNode >& newChild, const cpo::uno::Reference< css::xml::dom::XNode >& oldChild) override
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
