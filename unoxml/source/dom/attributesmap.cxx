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

#include <attributesmap.hxx>

#include <string.h>

#include <element.hxx>
#include <document.hxx>

using namespace css::uno;
using namespace css::xml::dom;

namespace DOM
{
    CAttributesMap::CAttributesMap(::rtl::Reference<CElement> const& pElement,
                ::osl::Mutex & rMutex)
        : m_pElement(pElement)
        , m_rMutex(rMutex)
    {
    }

    /**
    The number of nodes in this map.
    */
    sal_Int32 SAL_CALL CAttributesMap::getLength() throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard const g(m_rMutex);

        sal_Int32 count = 0;
        xmlNodePtr pNode = m_pElement->GetNodePtr();
        if (pNode != NULL)
        {
            xmlAttrPtr cur = pNode->properties;
            while (cur != NULL)
            {
                count++;
                cur = cur->next;
            }
        }
        return count;
    }

    /**
    Retrieves a node specified by local name
    */
    Reference< XNode > SAL_CALL
    CAttributesMap::getNamedItem(OUString const& name) throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard const g(m_rMutex);

        Reference< XNode > aNode;
        xmlNodePtr pNode = m_pElement->GetNodePtr();
        if (pNode != NULL)
        {
            OString o1 = OUStringToOString(name, RTL_TEXTENCODING_UTF8);
            xmlChar const * xName = reinterpret_cast<xmlChar const *>(o1.getStr());
            xmlAttrPtr cur = pNode->properties;
            while (cur != NULL)
            {
                if( strcmp(reinterpret_cast<char const *>(xName), reinterpret_cast<char const *>(cur->name)) == 0)
                {
                    aNode.set( m_pElement->GetOwnerDocument().GetCNode(
                                   reinterpret_cast<xmlNodePtr>(cur)).get() );
                    break;
                }
                cur = cur->next;
            }
        }
        return aNode;
    }

    /**
    Retrieves a node specified by local name and namespace URI.
    */
    Reference< XNode > SAL_CALL
    CAttributesMap::getNamedItemNS(
            OUString const& namespaceURI, OUString const& localName)
    throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard const g(m_rMutex);

        Reference< XNode > aNode;
        xmlNodePtr pNode = m_pElement->GetNodePtr();
        if (pNode != NULL)
        {
            OString o1 = OUStringToOString(localName, RTL_TEXTENCODING_UTF8);
            xmlChar const * xName = reinterpret_cast<xmlChar const *>(o1.getStr());
            OString o2 = OUStringToOString(namespaceURI, RTL_TEXTENCODING_UTF8);
            xmlChar const*const xNs =
                reinterpret_cast<xmlChar const*>(o2.getStr());
            xmlNsPtr const pNs = xmlSearchNsByHref(pNode->doc, pNode, xNs);
            xmlAttrPtr cur = pNode->properties;
            while (cur != NULL && pNs != NULL)
            {
                if( strcmp(reinterpret_cast<char const *>(xName), reinterpret_cast<char const *>(cur->name)) == 0 &&
                    cur->ns == pNs)
                {
                    aNode.set( m_pElement->GetOwnerDocument().GetCNode(
                                  reinterpret_cast<xmlNodePtr>(cur)).get() );
                    break;
                }
                cur = cur->next;
            }
        }
        return aNode;
    }

    /**
    Returns the indexth item in the map.
    */
    Reference< XNode > SAL_CALL
    CAttributesMap::item(sal_Int32 index) throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard const g(m_rMutex);

        Reference< XNode > aNode;
        xmlNodePtr pNode = m_pElement->GetNodePtr();
        if (pNode != NULL)
        {
            xmlAttrPtr cur = pNode->properties;
            sal_Int32 count = 0;
            while (cur != NULL)
            {
                if (count == index)
                {
                    aNode.set( m_pElement->GetOwnerDocument().GetCNode(
                                reinterpret_cast<xmlNodePtr>(cur)).get() );
                    break;
                }
                count++;
                cur = cur->next;
            }
        }
        return aNode;
    }

    /**
    Removes a node specified by name.
    */
    Reference< XNode > SAL_CALL
    CAttributesMap::removeNamedItem(OUString const& name)
    throw (DOMException, RuntimeException, std::exception)
    {
        // no MutexGuard needed: m_pElement is const
        Reference< XAttr > const xAttr(m_pElement->getAttributeNode(name));
        if (!xAttr.is()) {
            throw DOMException(
                "CAttributesMap::removeNamedItem: no such attribute",
                static_cast<OWeakObject*>(this),
                DOMExceptionType_NOT_FOUND_ERR);
        }
        Reference< XNode > const xRet(
            m_pElement->removeAttributeNode(xAttr), UNO_QUERY);
        return xRet;
    }

    /**
    // Removes a node specified by local name and namespace URI.
    */
    Reference< XNode > SAL_CALL
    CAttributesMap::removeNamedItemNS(
            OUString const& namespaceURI, OUString const& localName)
    throw (DOMException, RuntimeException, std::exception)
    {
        // no MutexGuard needed: m_pElement is const
        Reference< XAttr > const xAttr(
            m_pElement->getAttributeNodeNS(namespaceURI, localName));
        if (!xAttr.is()) {
            throw DOMException(
                "CAttributesMap::removeNamedItemNS: no such attribute",
                static_cast<OWeakObject*>(this),
                DOMExceptionType_NOT_FOUND_ERR);
        }
        Reference< XNode > const xRet(
            m_pElement->removeAttributeNode(xAttr), UNO_QUERY);
        return xRet;
    }

    /**
    // Adds a node using its nodeName attribute.
    */
    Reference< XNode > SAL_CALL
    CAttributesMap::setNamedItem(Reference< XNode > const& xNode)
    throw (DOMException, RuntimeException, std::exception)
    {
        Reference< XAttr > const xAttr(xNode, UNO_QUERY);
        if (!xNode.is()) {
            throw DOMException(
                "CAttributesMap::setNamedItem: XAttr argument expected",
                static_cast<OWeakObject*>(this),
                DOMExceptionType_HIERARCHY_REQUEST_ERR);
        }
        // no MutexGuard needed: m_pElement is const
        Reference< XNode > const xRet(
            m_pElement->setAttributeNode(xAttr), UNO_QUERY);
        return xRet;
    }

    /**
    Adds a node using its namespaceURI and localName.
    */
    Reference< XNode > SAL_CALL
    CAttributesMap::setNamedItemNS(Reference< XNode > const& xNode)
    throw (DOMException, RuntimeException, std::exception)
    {
        Reference< XAttr > const xAttr(xNode, UNO_QUERY);
        if (!xNode.is()) {
            throw DOMException(
                "CAttributesMap::setNamedItemNS: XAttr argument expected",
                static_cast<OWeakObject*>(this),
                DOMExceptionType_HIERARCHY_REQUEST_ERR);
        }
        // no MutexGuard needed: m_pElement is const
        Reference< XNode > const xRet(
            m_pElement->setAttributeNodeNS(xAttr), UNO_QUERY);
        return xRet;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
