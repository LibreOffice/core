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
#include <com/sun/star/xml/dom/XText.hpp>

#include "characterdata.hxx"

namespace DOM
{
    typedef ::cppu::ImplInheritanceHelper< CCharacterData, css::xml::dom::XText > CText_Base;

    class CText
        : public CText_Base
    {
    private:
        friend class CDocument;

    protected:
        CText(CDocument const& rDocument, ::osl::Mutex const& rMutex,
                css::xml::dom::NodeType const& reNodeType, xmlNodePtr const& rpNode);
        CText(CDocument const& rDocument, ::osl::Mutex const& rMutex,
                xmlNodePtr const pNode);

    public:

        virtual void saxify(const cpo::uno::Reference< css::xml::sax::XDocumentHandler >& i_xHandler) override;

        virtual void fastSaxify( Context& io_rContext ) override;

         // Breaks this node into two nodes at the specified offset, keeping
         // both in the tree as siblings.
         virtual cpo::uno::Reference< css::xml::dom::XText > splitText(sal_Int32 offset) override;


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

        // --- resolve uno inheritance problems...
        // --- delegation for XNode base.
        virtual cpo::uno::Reference< css::xml::dom::XNode > appendChild(const cpo::uno::Reference< css::xml::dom::XNode >& newChild) override
        {
            return CCharacterData::appendChild(newChild);
        }
        virtual cpo::uno::Reference< css::xml::dom::XNode > cloneNode(bool deep) override
        {
            return CCharacterData::cloneNode(deep);
        }
        virtual cpo::uno::Reference< css::xml::dom::XNamedNodeMap > getAttributes() override
        {
            return CCharacterData::getAttributes();
        }
        virtual cpo::uno::Reference< css::xml::dom::XNodeList > getChildNodes() override
        {
            return CCharacterData::getChildNodes();
        }
        virtual cpo::uno::Reference< css::xml::dom::XNode > getFirstChild() override
        {
            return CCharacterData::getFirstChild();
        }
        virtual cpo::uno::Reference< css::xml::dom::XNode > getLastChild() override
        {
            return CCharacterData::getLastChild();
        }
        virtual OUString getLocalName() override
        {
            return CCharacterData::getLocalName();
        }
        virtual OUString getNamespaceURI() override
        {
            return CCharacterData::getNamespaceURI();
        }
        virtual cpo::uno::Reference< css::xml::dom::XNode > getNextSibling() override
        {
            return CCharacterData::getNextSibling();
        }
        virtual css::xml::dom::NodeType getNodeType() override
        {
            return CCharacterData::getNodeType();
        }
        virtual OUString getNodeValue() override
        {
            return CCharacterData::getNodeValue();
        }
        virtual cpo::uno::Reference< css::xml::dom::XDocument > getOwnerDocument() override
        {
            return CCharacterData::getOwnerDocument();
        }
        virtual cpo::uno::Reference< css::xml::dom::XNode > getParentNode() override
        {
            return CCharacterData::getParentNode();
        }
        virtual OUString getPrefix() override
        {
            return CCharacterData::getPrefix();
        }
        virtual cpo::uno::Reference< css::xml::dom::XNode > getPreviousSibling() override
        {
            return CCharacterData::getPreviousSibling();
        }
        virtual bool hasAttributes() override
        {
            return CCharacterData::hasAttributes();
        }
        virtual bool hasChildNodes() override
        {
            return CCharacterData::hasChildNodes();
        }
        virtual cpo::uno::Reference< css::xml::dom::XNode > insertBefore(
                const cpo::uno::Reference< css::xml::dom::XNode >& newChild, const cpo::uno::Reference< css::xml::dom::XNode >& refChild) override
        {
            return CCharacterData::insertBefore(newChild, refChild);
        }
        virtual bool isSupported(const OUString& feature, const OUString& ver) override
        {
            return CCharacterData::isSupported(feature, ver);
        }
        virtual void normalize() override
        {
            CCharacterData::normalize();
        }
        virtual cpo::uno::Reference< css::xml::dom::XNode > removeChild(const cpo::uno::Reference< css::xml::dom::XNode >& oldChild) override
        {
            return CCharacterData::removeChild(oldChild);
        }
        virtual cpo::uno::Reference< css::xml::dom::XNode > replaceChild(
                const cpo::uno::Reference< css::xml::dom::XNode >& newChild, const cpo::uno::Reference< css::xml::dom::XNode >& oldChild) override
        {
            return CCharacterData::replaceChild(newChild, oldChild);
        }
        virtual void setNodeValue(const OUString& nodeValue) override
        {
            return CCharacterData::setNodeValue(nodeValue);
        }
        virtual void setPrefix(const OUString& prefix) override
        {
            return CCharacterData::setPrefix(prefix);
        }

    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
