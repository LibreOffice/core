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

#ifndef INCLUDED_UNOXML_SOURCE_DOM_ATTR_HXX
#define INCLUDED_UNOXML_SOURCE_DOM_ATTR_HXX

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
    private:
        friend class CDocument;

    private:
        xmlAttrPtr m_aAttrPtr;
        ::std::unique_ptr< stringpair_t > m_pNamespace;

    protected:
        CAttr(CDocument const& rDocument, ::osl::Mutex const& rMutex,
                xmlAttrPtr const pAttr);

    public:
        /// return the libxml namespace corresponding to m_pNamespace on pNode
        xmlNsPtr GetNamespace(xmlNodePtr const pNode);

        virtual bool IsChildTypeAllowed(css::xml::dom::NodeType const nodeType) override;

        /**
        Returns the name of this attribute.
        */
        virtual OUString SAL_CALL getName() throw (css::uno::RuntimeException, std::exception) override;

        /**
        The Element node this attribute is attached to or null if this
        attribute is not in use.
        */
        virtual css::uno::Reference< css::xml::dom::XElement > SAL_CALL getOwnerElement() throw (css::uno::RuntimeException, std::exception) override;

        /**
        If this attribute was explicitly given a value in the original
        document, this is true; otherwise, it is false.
        */
        virtual sal_Bool SAL_CALL getSpecified()throw (css::uno::RuntimeException, std::exception) override;

        /**
        On retrieval, the value of the attribute is returned as a string.
        */
        virtual OUString SAL_CALL getValue() throw (css::uno::RuntimeException, std::exception) override;

        /**
        Sets the value of the attribute from a string.
        */

        virtual void SAL_CALL setValue(const OUString& value) throw (css::uno::RuntimeException, css::xml::dom::DOMException, std::exception) override;

        // resolve uno inheritance problems...
        // overrides for XNode base
        virtual OUString SAL_CALL getNodeName()
            throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getNodeValue()
            throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getLocalName()
            throw (css::uno::RuntimeException, std::exception) override;

    // --- delegation for XNode base.
    virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL appendChild(const css::uno::Reference< css::xml::dom::XNode >& newChild)
        throw (css::uno::RuntimeException, css::xml::dom::DOMException, std::exception) override
    {
        return CNode::appendChild(newChild);
    }
    virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL cloneNode(sal_Bool deep)
        throw (css::uno::RuntimeException, std::exception) override
    {
        return CNode::cloneNode(deep);
    }
    virtual css::uno::Reference< css::xml::dom::XNamedNodeMap > SAL_CALL getAttributes()
        throw (css::uno::RuntimeException, std::exception) override
    {
        return CNode::getAttributes();
    }
    virtual css::uno::Reference< css::xml::dom::XNodeList > SAL_CALL getChildNodes()
        throw (css::uno::RuntimeException, std::exception) override
    {
        return CNode::getChildNodes();
    }
    virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL getFirstChild()
        throw (css::uno::RuntimeException, std::exception) override
    {
        return CNode::getFirstChild();
    }
    virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL getLastChild()
        throw (css::uno::RuntimeException, std::exception) override
    {
        return CNode::getLastChild();
    }
    virtual OUString SAL_CALL getNamespaceURI()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL getNextSibling()
        throw (css::uno::RuntimeException, std::exception) override
    {
        return CNode::getNextSibling();
    }
    virtual css::xml::dom::NodeType SAL_CALL getNodeType()
        throw (css::uno::RuntimeException, std::exception) override
    {
        return CNode::getNodeType();
    }
    virtual css::uno::Reference< css::xml::dom::XDocument > SAL_CALL getOwnerDocument()
        throw (css::uno::RuntimeException, std::exception) override
    {
        return CNode::getOwnerDocument();
    }
    virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL getParentNode()
        throw (css::uno::RuntimeException, std::exception) override
    {
        return CNode::getParentNode();
    }
    virtual OUString SAL_CALL getPrefix()
        throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL getPreviousSibling()
        throw (css::uno::RuntimeException, std::exception) override
    {
        return CNode::getPreviousSibling();
    }
    virtual sal_Bool SAL_CALL hasAttributes()
        throw (css::uno::RuntimeException, std::exception) override
    {
        return CNode::hasAttributes();
    }
    virtual sal_Bool SAL_CALL hasChildNodes()
        throw (css::uno::RuntimeException, std::exception) override
    {
        return CNode::hasChildNodes();
    }
    virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL insertBefore(
            const css::uno::Reference< css::xml::dom::XNode >& newChild, const css::uno::Reference< css::xml::dom::XNode >& refChild)
        throw (css::uno::RuntimeException, css::xml::dom::DOMException, std::exception) override
    {
        return CNode::insertBefore(newChild, refChild);
    }
    virtual sal_Bool SAL_CALL isSupported(const OUString& feature, const OUString& ver)
        throw (css::uno::RuntimeException, std::exception) override
    {
        return CNode::isSupported(feature, ver);
    }
    virtual void SAL_CALL normalize()
        throw (css::uno::RuntimeException, std::exception) override
    {
        CNode::normalize();
    }
    virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL removeChild(const css::uno::Reference< css::xml::dom::XNode >& oldChild)
        throw (css::uno::RuntimeException, css::xml::dom::DOMException, std::exception) override
    {
        return CNode::removeChild(oldChild);
    }
    virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL replaceChild(
            const css::uno::Reference< css::xml::dom::XNode >& newChild, const css::uno::Reference< css::xml::dom::XNode >& oldChild)
        throw (css::uno::RuntimeException, css::xml::dom::DOMException, std::exception) override
    {
        return CNode::replaceChild(newChild, oldChild);
    }
    virtual void SAL_CALL setNodeValue(const OUString& nodeValue)
        throw (css::uno::RuntimeException, css::xml::dom::DOMException, std::exception) override
    {
        return setValue(nodeValue);
    }
    virtual void SAL_CALL setPrefix(const OUString& prefix)
        throw (css::uno::RuntimeException, css::xml::dom::DOMException, std::exception) override;

    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
