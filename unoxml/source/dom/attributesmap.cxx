/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <attributesmap.hxx>

#include <string.h>

#include <element.hxx>
#include <document.hxx>


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
    sal_Int32 SAL_CALL CAttributesMap::getLength() throw (RuntimeException)
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
    CAttributesMap::getNamedItem(OUString const& name) throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        Reference< XNode > aNode;
        xmlNodePtr pNode = m_pElement->GetNodePtr();
        if (pNode != NULL)
        {
            OString o1 = OUStringToOString(name, RTL_TEXTENCODING_UTF8);
            xmlChar* xName = (xmlChar*)o1.getStr();
            xmlAttrPtr cur = pNode->properties;
            while (cur != NULL)
            {
                if( strcmp((char*)xName, (char*)cur->name) == 0)
                {
                    aNode = Reference< XNode >(
                            m_pElement->GetOwnerDocument().GetCNode(
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
    throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        Reference< XNode > aNode;
        xmlNodePtr pNode = m_pElement->GetNodePtr();
        if (pNode != NULL)
        {
            OString o1 = OUStringToOString(localName, RTL_TEXTENCODING_UTF8);
            xmlChar* xName = (xmlChar*)o1.getStr();
            OString o2 = OUStringToOString(namespaceURI, RTL_TEXTENCODING_UTF8);
            xmlChar const*const xNs =
                reinterpret_cast<xmlChar const*>(o2.getStr());
            xmlNsPtr const pNs = xmlSearchNsByHref(pNode->doc, pNode, xNs);
            xmlAttrPtr cur = pNode->properties;
            while (cur != NULL && pNs != NULL)
            {
                if( strcmp((char*)xName, (char*)cur->name) == 0 &&
                    cur->ns == pNs)
                {
                    aNode = Reference< XNode >(
                            m_pElement->GetOwnerDocument().GetCNode(
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
    CAttributesMap::item(sal_Int32 index) throw (RuntimeException)
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
                    aNode = Reference< XNode >(
                            m_pElement->GetOwnerDocument().GetCNode(
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
    throw (RuntimeException)
    {
        // no MutexGuard needed: m_pElement is const
        Reference< XAttr > const xAttr(m_pElement->getAttributeNode(name));
        if (!xAttr.is()) {
            throw DOMException(OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "CAttributesMap::removeNamedItem: no such attribute")),
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
    throw (RuntimeException)
    {
        // no MutexGuard needed: m_pElement is const
        Reference< XAttr > const xAttr(
            m_pElement->getAttributeNodeNS(namespaceURI, localName));
        if (!xAttr.is()) {
            throw DOMException(OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "CAttributesMap::removeNamedItemNS: no such attribute")),
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
    throw (RuntimeException)
    {
        Reference< XAttr > const xAttr(xNode, UNO_QUERY);
        if (!xNode.is()) {
            throw DOMException(OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "CAttributesMap::setNamedItem: XAttr argument expected")),
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
    throw (RuntimeException)
    {
        Reference< XAttr > const xAttr(xNode, UNO_QUERY);
        if (!xNode.is()) {
            throw DOMException(OUString(RTL_CONSTASCII_USTRINGPARAM(
                    "CAttributesMap::setNamedItemNS: XAttr argument expected")),
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
