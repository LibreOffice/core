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
        xmlNodePtr pNode = m_pElement->m_aNodePtr;
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
        xmlNodePtr pNode = m_pElement->m_aNodePtr;
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
        xmlNodePtr pNode = m_pElement->m_aNodePtr;
        if (pNode != NULL)
        {
            OString o1 = OUStringToOString(localName, RTL_TEXTENCODING_UTF8);
            xmlChar* xName = (xmlChar*)o1.getStr();
            OString o2 = OUStringToOString(namespaceURI, RTL_TEXTENCODING_UTF8);
            xmlChar* xNs = (xmlChar*)o1.getStr();
            xmlNsPtr pNs = xmlSearchNs(pNode->doc, pNode, xNs);
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
        xmlNodePtr pNode = m_pElement->m_aNodePtr;
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
        ::osl::MutexGuard const g(m_rMutex);

        xmlNodePtr const pNode = m_pElement->m_aNodePtr;
        if (pNode != NULL)
        {
            OString o1 = OUStringToOString(name, RTL_TEXTENCODING_UTF8);
            xmlChar* xName = (xmlChar*)o1.getStr();
            xmlAttrPtr cur = pNode->properties;
            while (cur != NULL)
            {
                if (strcmp((char*)xName, (char*)cur->name) == 0) {
                    ::rtl::Reference<CNode> const pCNode =
                            m_pElement->GetOwnerDocument().GetCNode(
                                reinterpret_cast<xmlNodePtr>(cur)).get();
                    // this seems to be legal...
                    xmlUnlinkNode(reinterpret_cast<xmlNodePtr>(cur));
                    pCNode->m_bUnlinked = true;
                    return Reference< XNode >(pCNode.get());
                }
                cur = cur->next;
            }
        }
        return 0;
    }

    /**
    // Removes a node specified by local name and namespace URI.
    */
    Reference< XNode > SAL_CALL
    CAttributesMap::removeNamedItemNS(
            OUString const& namespaceURI, OUString const& localName)
    throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        xmlNodePtr const pNode = m_pElement->m_aNodePtr;
        if (pNode != NULL)
        {
            OString o1 = OUStringToOString(localName, RTL_TEXTENCODING_UTF8);
            xmlChar* xName = (xmlChar*)o1.getStr();
            OString o2 = OUStringToOString(namespaceURI, RTL_TEXTENCODING_UTF8);
            xmlChar* xNs = (xmlChar*)o1.getStr();
            xmlNsPtr pNs = xmlSearchNs(pNode->doc, pNode, xNs);
            xmlAttrPtr cur = pNode->properties;
            while (cur != NULL && pNs != NULL)
            {
                if (strcmp((char*)xName, (char*)cur->name) == 0 &&
                    cur->ns == pNs)
                {
                    ::rtl::Reference<CNode> const pCNode =
                            m_pElement->GetOwnerDocument().GetCNode(
                                reinterpret_cast<xmlNodePtr>(cur)).get();
                    // this seems to be legal...
                    xmlUnlinkNode(reinterpret_cast<xmlNodePtr>(cur));
                    pCNode->m_bUnlinked = true;
                    return Reference< XNode >(pCNode.get());
                }
                cur = cur->next;
            }
        }
        return 0;
    }

    /**
    // Adds a node using its nodeName attribute.
    */
    Reference< XNode > SAL_CALL
    CAttributesMap::setNamedItem(Reference< XNode > const& arg)
    throw (RuntimeException)
    {
      return arg;
      // return Reference< XNode >();
    }

    /**
    Adds a node using its namespaceURI and localName.
    */
    Reference< XNode > SAL_CALL
    CAttributesMap::setNamedItemNS(Reference< XNode > const& arg)
    throw (RuntimeException)
    {
        return arg;
    // return Reference< XNode >();
    }
}
