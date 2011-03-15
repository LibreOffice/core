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

#include <attr.hxx>

#include <string.h>

#include <boost/shared_ptr.hpp>

#include <com/sun/star/xml/dom/DOMException.hdl>
#include <com/sun/star/xml/dom/events/XMutationEvent.hpp>

#include <document.hxx>


namespace DOM
{
    CAttr::CAttr(CDocument const& rDocument, ::osl::Mutex const& rMutex,
            xmlAttrPtr const pAttr)
        : CAttr_Base(rDocument, rMutex,
                NodeType_ATTRIBUTE_NODE, reinterpret_cast<xmlNodePtr>(pAttr))
        , m_aAttrPtr(pAttr)
    {
    }

    xmlNsPtr CAttr::GetNamespace(xmlNodePtr const pNode)
    {
        if (!m_pNamespace.get()) {
            return 0;
        }
        xmlChar const*const pUri(reinterpret_cast<xmlChar const*>(
                m_pNamespace->first.getStr()));
        xmlChar const*const pPrefix(reinterpret_cast<xmlChar const*>(
                m_pNamespace->second.getStr()));
        xmlNsPtr pNs = xmlSearchNs(pNode->doc, pNode, pPrefix);
        if (pNs && (0 != xmlStrcmp(pNs->href, pUri))) {
            return pNs;
        }
        pNs = xmlNewNs(pNode, pUri, pPrefix);
        if (pNs) {
            return pNs;
        }
        pNs = xmlSearchNsByHref(pNode->doc, pNode, pUri);
        // if (!pNs) hmm... now what? throw?
        if (!pNs) { OSL_TRACE("CAtttr: cannot create namespace"); }
        return pNs;
    }

    bool CAttr::IsChildTypeAllowed(NodeType const nodeType)
    {
        switch (nodeType) {
            case NodeType_TEXT_NODE:
            case NodeType_ENTITY_REFERENCE_NODE:
                return true;
            default:
                return false;
        }
    }

    OUString SAL_CALL CAttr::getNodeName()
        throw (RuntimeException)
    {
        return getName();
    }
    OUString SAL_CALL CAttr::getNodeValue()
        throw (RuntimeException)
    {
        return getValue();
    }
    OUString SAL_CALL CAttr::getLocalName()
        throw (RuntimeException)
    {
        return getName();
    }


    /**
    Returns the name of this attribute.
    */
    OUString SAL_CALL CAttr::getName() throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        if ((0 == m_aNodePtr) || (0 == m_aAttrPtr)) {
            return ::rtl::OUString();
        }
        OUString const aName((char*)m_aAttrPtr->name,
                strlen((char*)m_aAttrPtr->name), RTL_TEXTENCODING_UTF8);
        return aName;
    }

    /**
    The Element node this attribute is attached to or null if this
    attribute is not in use.
    */
    Reference< XElement > SAL_CALL CAttr::getOwnerElement()
        throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        if ((0 == m_aNodePtr) || (0 == m_aAttrPtr)) {
            return 0;
        }
        if (0 == m_aAttrPtr->parent) {
            return 0;
        }
        Reference< XElement > const xRet(
            static_cast< XNode* >(GetOwnerDocument().GetCNode(
                    m_aAttrPtr->parent).get()),
            UNO_QUERY_THROW);
        return xRet;
    }

    /**
    If this attribute was explicitly given a value in the original
    document, this is true; otherwise, it is false.
    */
    sal_Bool SAL_CALL CAttr::getSpecified()
        throw (RuntimeException)
    {
        // FIXME if this DOM implemenatation supported DTDs it would need
        // to check that this attribute is not default or something
        return sal_True;
    }

    /**
    On retrieval, the value of the attribute is returned as a string.
    */
    OUString SAL_CALL CAttr::getValue()
        throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        if ((0 == m_aNodePtr) || (0 == m_aAttrPtr)) {
            return ::rtl::OUString();
        }
        if (0 == m_aAttrPtr->children) {
            return ::rtl::OUString();
        }
        char const*const pContent((m_aAttrPtr->children)
            ? reinterpret_cast<char const*>(m_aAttrPtr->children->content)
            : "");
        OUString const ret(pContent, strlen(pContent), RTL_TEXTENCODING_UTF8);
        return ret;
    }

    /**
    Sets the value of the attribute from a string.
    */
    void SAL_CALL CAttr::setValue(const OUString& value)
        throw (RuntimeException, DOMException)
    {
        ::osl::ClearableMutexGuard guard(m_rMutex);

        if ((0 == m_aNodePtr) || (0 == m_aAttrPtr)) {
            return;
        }

        // remember old value (for mutation event)
        OUString sOldValue = getValue();

        OString o1 = OUStringToOString(value, RTL_TEXTENCODING_UTF8);
        xmlChar* xValue = (xmlChar*)o1.getStr();
        // xmlChar* xName = OUStringToOString(m_aAttrPtr->name, RTL_TEXTENCODING_UTF8).getStr();
        // this does not work if the attribute was created anew
        // xmlNodePtr pNode = m_aAttrPtr->parent;
        // xmlSetProp(pNode, m_aAttrPtr->name, xValue);
        ::boost::shared_ptr<xmlChar const> const buffer(
                xmlEncodeEntitiesReentrant(m_aAttrPtr->doc, xValue), xmlFree);
        xmlFreeNodeList(m_aAttrPtr->children);
        m_aAttrPtr->children =
            xmlStringGetNodeList(m_aAttrPtr->doc, buffer.get());
        xmlNodePtr tmp = m_aAttrPtr->children;
        while (tmp != NULL) {
            tmp->parent = (xmlNodePtr) m_aNodePtr;
            tmp->doc = m_aAttrPtr->doc;
            if (tmp->next == NULL)
                m_aNodePtr->last = tmp;
            tmp = tmp->next;
        }

        // dispatch DOM events to signal change in attribute value
        // dispatch DomAttrModified + DOMSubtreeModified
        OUString sEventName( RTL_CONSTASCII_USTRINGPARAM("DOMAttrModified") );
        Reference< XDocumentEvent > docevent(getOwnerDocument(), UNO_QUERY);
        Reference< XMutationEvent > event(docevent->createEvent(sEventName),UNO_QUERY);
        event->initMutationEvent(
                sEventName, sal_True, sal_False,
                Reference<XNode>( static_cast<XAttr*>( this ) ),
                sOldValue, value, getName(), AttrChangeType_MODIFICATION );

        guard.clear(); // release mutex before calling event handlers

        dispatchEvent(Reference< XEvent >(event, UNO_QUERY));
        dispatchSubtreeModified();
    }

    void SAL_CALL CAttr::setPrefix(const OUString& prefix)
        throw (RuntimeException, DOMException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        if (!m_aNodePtr) { return; }

        if (m_pNamespace.get()) {
            OSL_ASSERT(!m_aNodePtr->parent);
            m_pNamespace->second =
                OUStringToOString(prefix, RTL_TEXTENCODING_UTF8);
        } else {
            CNode::setPrefix(prefix);
        }
    }

    OUString SAL_CALL CAttr::getPrefix()
        throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        if (!m_aNodePtr) { return ::rtl::OUString(); }

        if (m_pNamespace.get()) {
            OSL_ASSERT(!m_aNodePtr->parent);
            OUString const ret(::rtl::OStringToOUString(
                        m_pNamespace->second, RTL_TEXTENCODING_UTF8));
            return ret;
        } else {
            return CNode::getPrefix();
        }
    }

    OUString SAL_CALL CAttr::getNamespaceURI()
        throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        if (!m_aNodePtr) { return ::rtl::OUString(); }

        if (m_pNamespace.get()) {
            OSL_ASSERT(!m_aNodePtr->parent);
            OUString const ret(::rtl::OStringToOUString(
                        m_pNamespace->first, RTL_TEXTENCODING_UTF8));
            return ret;
        } else {
            return CNode::getNamespaceURI();
        }
    }
}
