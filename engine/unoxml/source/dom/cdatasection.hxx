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

#include <cpo/uno/Reference.h>
#include <com/sun/star/xml/dom/XCDATASection.hpp>

#include <cppuhelper/implbase.hxx>
#include "text.hxx"

namespace DOM
{
    typedef ::cppu::ImplInheritanceHelper< CText, css::xml::dom::XCDATASection >
        CCDATASection_Base;

    class CCDATASection
        : public CCDATASection_Base
    {
        friend class CDocument;

        CCDATASection(CDocument const& rDocument, ::osl::Mutex const& rMutex,
                xmlNodePtr const pNode);

    public:

        virtual void saxify(const cpo::uno::Reference< css::xml::sax::XDocumentHandler >& i_xHandler) override;

        virtual cpo::uno::Reference< css::xml::dom::XText > splitText(sal_Int32 offset) override
        {
            return CText::splitText(offset);
        }

         // --- delegations for XCharacterData
        virtual void appendData(const OUString& arg) override
        {
            CCharacterData::appendData(arg);
        }
        virtual void deleteData(sal_Int32 offset, sal_Int32 count) override
        {
            CCharacterData::deleteData(offset, count);
        }
        virtual OUString getData() override
        {
            return CCharacterData::getData();
        }
        virtual sal_Int32 getLength() override
        {
            return CCharacterData::getLength();
        }
        virtual void insertData(sal_Int32 offset, const OUString& arg) override
        {
            CCharacterData::insertData(offset, arg);
        }
        virtual void replaceData(sal_Int32 offset, sal_Int32 count, const OUString& arg) override
        {
            CCharacterData::replaceData(offset, count, arg);
        }
        virtual void setData(const OUString& data) override
        {
            CCharacterData::setData(data);
        }
        virtual OUString subStringData(sal_Int32 offset, sal_Int32 count) override
        {
            return CCharacterData::subStringData(offset, count);
        }


         // --- overrides for XNode base
        virtual OUString getNodeName() override;
        virtual OUString getNodeValue() override;

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
        return CText::setNodeValue(nodeValue);
    }
    virtual void setPrefix(const OUString& prefix) override
    {
        return CNode::setPrefix(prefix);
    }

    };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
