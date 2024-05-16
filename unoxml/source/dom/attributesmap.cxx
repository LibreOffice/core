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

#include "attributesmap.hxx"

#include <string.h>

#include <com/sun/star/xml/dom/DOMException.hpp>
#include <utility>

#include "element.hxx"
#include "document.hxx"

using namespace css::uno;
using namespace css::xml::dom;

namespace DOM
{
    CAttributesMap::CAttributesMap(::rtl::Reference<CElement> pElement,
                ::osl::Mutex & rMutex)
        : m_pElement(std::move(pElement))
        , m_rMutex(rMutex)
    {
    }

    /**
    The number of nodes in this map.
    */
    sal_Int32 SAL_CALL CAttributesMap::getLength()
    {
        ::osl::MutexGuard const g(m_rMutex);

        sal_Int32 count = 0;
        xmlNodePtr pNode = m_pElement->GetNodePtr();
        if (pNode != nullptr)
        {
            xmlAttrPtr cur = pNode->properties;
            while (cur != nullptr)
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
    CAttributesMap::getNamedItem(OUString const& name)
    {
        ::osl::MutexGuard const g(m_rMutex);

        Reference< XNode > aNode;
        xmlNodePtr pNode = m_pElement->GetNodePtr();
        if (pNode != nullptr)
        {
            OString o1 = OUStringToOString(name, RTL_TEXTENCODING_UTF8);
            xmlChar const * pName = reinterpret_cast<xmlChar const *>(o1.getStr());
            xmlAttrPtr cur = pNode->properties;
            while (cur != nullptr)
            {
                if( strcmp(reinterpret_cast<char const *>(pName), reinterpret_cast<char const *>(cur->name)) == 0)
                {
                    aNode = m_pElement->GetOwnerDocument().GetCNode(
                                   reinterpret_cast<xmlNodePtr>(cur));
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
    {
        ::osl::MutexGuard const g(m_rMutex);

        Reference< XNode > aNode;
        xmlNodePtr pNode = m_pElement->GetNodePtr();
        if (pNode != nullptr)
        {
            OString o1 = OUStringToOString(localName, RTL_TEXTENCODING_UTF8);
            xmlChar const * pName = reinterpret_cast<xmlChar const *>(o1.getStr());
            OString o2 = OUStringToOString(namespaceURI, RTL_TEXTENCODING_UTF8);
            xmlChar const* pSearchNs =
                reinterpret_cast<xmlChar const*>(o2.getStr());
            xmlNsPtr const pNs = xmlSearchNsByHref(pNode->doc, pNode, pSearchNs);
            xmlAttrPtr cur = pNode->properties;
            while (cur != nullptr && pNs != nullptr)
            {
                if( strcmp(reinterpret_cast<char const *>(pName), reinterpret_cast<char const *>(cur->name)) == 0 &&
                    cur->ns == pNs)
                {
                    aNode = m_pElement->GetOwnerDocument().GetCNode(
                                  reinterpret_cast<xmlNodePtr>(cur));
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
    CAttributesMap::item(sal_Int32 index)
    {
        ::osl::MutexGuard const g(m_rMutex);

        Reference< XNode > aNode;
        xmlNodePtr pNode = m_pElement->GetNodePtr();
        if (pNode != nullptr)
        {
            xmlAttrPtr cur = pNode->properties;
            sal_Int32 count = 0;
            while (cur != nullptr)
            {
                if (count == index)
                {
                    aNode = m_pElement->GetOwnerDocument().GetCNode(
                                reinterpret_cast<xmlNodePtr>(cur));
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
    {
        // no MutexGuard needed: m_pElement is const
        Reference< XAttr > const xAttr(m_pElement->getAttributeNode(name));
        if (!xAttr.is()) {
            throw DOMException(
                u"CAttributesMap::removeNamedItem: no such attribute"_ustr,
                getXWeak(),
                DOMExceptionType_NOT_FOUND_ERR);
        }
        return m_pElement->removeAttributeNode(xAttr);
    }

    /**
    // Removes a node specified by local name and namespace URI.
    */
    Reference< XNode > SAL_CALL
    CAttributesMap::removeNamedItemNS(
            OUString const& namespaceURI, OUString const& localName)
    {
        // no MutexGuard needed: m_pElement is const
        Reference< XAttr > const xAttr(
            m_pElement->getAttributeNodeNS(namespaceURI, localName));
        if (!xAttr.is()) {
            throw DOMException(
                u"CAttributesMap::removeNamedItemNS: no such attribute"_ustr,
                getXWeak(),
                DOMExceptionType_NOT_FOUND_ERR);
        }
        return m_pElement->removeAttributeNode(xAttr);
    }

    /**
    // Adds a node using its nodeName attribute.
    */
    Reference< XNode > SAL_CALL
    CAttributesMap::setNamedItem(Reference< XNode > const& xNode)
    {
        Reference< XAttr > const xAttr(xNode, UNO_QUERY);
        if (!xNode.is()) {
            throw DOMException(
                u"CAttributesMap::setNamedItem: XAttr argument expected"_ustr,
                getXWeak(),
                DOMExceptionType_HIERARCHY_REQUEST_ERR);
        }
        // no MutexGuard needed: m_pElement is const
        return m_pElement->setAttributeNode(xAttr);
    }

    /**
    Adds a node using its namespaceURI and localName.
    */
    Reference< XNode > SAL_CALL
    CAttributesMap::setNamedItemNS(Reference< XNode > const& xNode)
    {
        Reference< XAttr > const xAttr(xNode, UNO_QUERY);
        if (!xNode.is()) {
            throw DOMException(
                u"CAttributesMap::setNamedItemNS: XAttr argument expected"_ustr,
                getXWeak(),
                DOMExceptionType_HIERARCHY_REQUEST_ERR);
        }
        // no MutexGuard needed: m_pElement is const
        return m_pElement->setAttributeNodeNS(xAttr);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
