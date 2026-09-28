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

#include <sal/types.h>

#include <cppuhelper/implbase.hxx>

#include <cpo/uno/Reference.h>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XCharacterData.hpp>

#include <node.hxx>

namespace DOM
{
    typedef ::cppu::ImplInheritanceHelper< CNode, css::xml::dom::XCharacterData >
        CCharacterData_Base;

    class CCharacterData
        : public CCharacterData_Base
    {

    protected:
        CCharacterData(CDocument const& rDocument, ::osl::Mutex const& rMutex,
                css::xml::dom::NodeType const& reNodeType, xmlNodePtr const& rpNode);

        void dispatchEvent_Impl(::osl::ClearableMutexGuard& guard,
                OUString const& prevValue, OUString const& newValue);

    public:
        /**
        Append the string to the end of the character data of the node.
        */
        virtual void appendData(const OUString& arg) override;

        /**
        Remove a range of 16-bit units from the node.
        */
        virtual void deleteData(sal_Int32 offset, sal_Int32 count) override;

        /**
        Return the character data of the node that implements this interface.
        */
        virtual OUString getData() override;

        /**
        The number of 16-bit units that are available through data and the
        substringData method below.
        */
        virtual sal_Int32 getLength() override;

        /**
        Insert a string at the specified 16-bit unit offset.
        */
        virtual void insertData(sal_Int32 offset, const OUString& arg) override;

        /**
        Replace the characters starting at the specified 16-bit unit offset
        with the specified string.
        */
        virtual void replaceData(sal_Int32 offset, sal_Int32 count, const OUString& arg) override;

        /**
        Set the character data of the node that implements this interface.
        */
        virtual void setData(const OUString& data) override;

        /**
        Extracts a range of data from the node.
        */
        virtual OUString subStringData(sal_Int32 offset, sal_Int32 count) override;

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
        virtual OUString getLocalName() override
        {
            return CNode::getLocalName();
        }
        virtual OUString getNamespaceURI() override
        {
            return CNode::getNamespaceURI();
        }
        virtual cpo::uno::Reference< css::xml::dom::XNode > getNextSibling() override
        {
            return CNode::getNextSibling();
        }
        virtual OUString getNodeName() override
        {
            return CNode::getNodeName();
        }
        virtual css::xml::dom::NodeType getNodeType() override
        {
            return CNode::getNodeType();
        }
        virtual OUString getNodeValue() override
        {
            return getData();
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
            return setData(nodeValue);
        }
        virtual void setPrefix(const OUString& prefix) override
        {
            return CNode::setPrefix(prefix);
        }


    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
