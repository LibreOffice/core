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

#ifndef DOM_ATTR_HXX
#define DOM_ATTR_HXX

#include <memory>

#include <libxml/tree.h>

#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XAttr.hpp>

#include <node.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::xml::dom;

namespace DOM
{
    typedef ::std::pair< OString, OString > stringpair_t;

    typedef ::cppu::ImplInheritanceHelper1< CNode, XAttr > CAttr_Base;

    class CAttr
        : public CAttr_Base
    {
    private:
        friend class CDocument;

    private:
        xmlAttrPtr m_aAttrPtr;
        ::std::auto_ptr< stringpair_t > m_pNamespace;

    protected:
        CAttr(CDocument const& rDocument, ::osl::Mutex const& rMutex,
                xmlAttrPtr const pAttr);

    public:
        /// return the libxml namespace corresponding to m_pNamespace on pNode
        xmlNsPtr GetNamespace(xmlNodePtr const pNode);

        virtual bool IsChildTypeAllowed(NodeType const nodeType);

        /**
        Returns the name of this attribute.
        */
        virtual OUString SAL_CALL getName() throw (RuntimeException);

        /**
        The Element node this attribute is attached to or null if this
        attribute is not in use.
        */
        virtual Reference< XElement > SAL_CALL getOwnerElement() throw (RuntimeException);

        /**
        If this attribute was explicitly given a value in the original
        document, this is true; otherwise, it is false.
        */
        virtual sal_Bool SAL_CALL getSpecified()throw (RuntimeException);

        /**
        On retrieval, the value of the attribute is returned as a string.
        */
        virtual OUString SAL_CALL getValue() throw (RuntimeException);

        /**
        Sets the value of the attribute from a string.
        */

        virtual void SAL_CALL setValue(const OUString& value) throw (RuntimeException, DOMException);

        // resolve uno inheritance problems...
        // overrides for XNode base
        virtual OUString SAL_CALL getNodeName()
            throw (RuntimeException);
        virtual OUString SAL_CALL getNodeValue()
            throw (RuntimeException);
        virtual OUString SAL_CALL getLocalName()
            throw (RuntimeException);

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
    virtual Reference< XNamedNodeMap > SAL_CALL getAttributes()
        throw (RuntimeException)
    {
        return CNode::getAttributes();
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
        throw (RuntimeException);
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
        throw (RuntimeException);
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
        return setValue(nodeValue);
    }
    virtual void SAL_CALL setPrefix(const OUString& prefix)
        throw (RuntimeException, DOMException);

    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
