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

#include <element.hxx>

#include <string.h>

#include <boost/shared_ptr.hpp>

#include <rtl/ustrbuf.hxx>

#include <com/sun/star/xml/sax/FastToken.hpp>

#include <comphelper/attributelist.hxx>

#include <node.hxx>
#include <attr.hxx>
#include <elementlist.hxx>
#include <attributesmap.hxx>
#include <document.hxx>

#include "../events/mutationevent.hxx"


namespace DOM
{

    CElement::CElement(CDocument const& rDocument, ::osl::Mutex const& rMutex,
            xmlNodePtr const pNode)
        : CElement_Base(rDocument, rMutex, NodeType_ELEMENT_NODE, pNode)
    {
    }

    void CElement::saxify(const Reference< XDocumentHandler >& i_xHandler)
    {
        if (!i_xHandler.is()) throw RuntimeException();
        comphelper::AttributeList *pAttrs =
            new comphelper::AttributeList();
        OUString type = "";
        // add namespace definitions to attributes
        for (xmlNsPtr pNs = m_aNodePtr->nsDef; pNs != 0; pNs = pNs->next) {
            const xmlChar *pPrefix = pNs->prefix ? pNs->prefix : (const xmlChar*)"";
            OUString prefix(reinterpret_cast<const sal_Char*>(pPrefix),
                strlen(reinterpret_cast<const char*>(pPrefix)),
                RTL_TEXTENCODING_UTF8);
            OUString name = (prefix.isEmpty())
                ? OUString( "xmlns" ) : OUString( "xmlns:" ) + prefix;
            const xmlChar *pHref = pNs->href;
            OUString val(reinterpret_cast<const sal_Char*>(pHref),
                strlen(reinterpret_cast<const char*>(pHref)),
                RTL_TEXTENCODING_UTF8);
            pAttrs->AddAttribute(name, type, val);
        }
        // add attributes
        for (xmlAttrPtr pAttr = m_aNodePtr->properties;
                        pAttr != 0; pAttr = pAttr->next) {
            ::rtl::Reference<CNode> const pNode = GetOwnerDocument().GetCNode(
                    reinterpret_cast<xmlNodePtr>(pAttr));
            OSL_ENSURE(pNode != 0, "CNode::get returned 0");
            OUString prefix = pNode->getPrefix();
            OUString name = (prefix.isEmpty())
                ? pNode->getLocalName()
                : prefix + OUString(static_cast<sal_Unicode>(':')) + pNode->getLocalName();
            OUString val  = pNode->getNodeValue();
            pAttrs->AddAttribute(name, type, val);
        }
        OUString prefix = getPrefix();
        OUString name = (prefix.isEmpty())
            ? getLocalName()
            : prefix + OUString(static_cast<sal_Unicode>(':')) + getLocalName();
        Reference< XAttributeList > xAttrList(pAttrs);
        i_xHandler->startElement(name, xAttrList);
        // recurse
        for (xmlNodePtr pChild = m_aNodePtr->children;
                        pChild != 0; pChild = pChild->next) {
            ::rtl::Reference<CNode> const pNode(
                    GetOwnerDocument().GetCNode(pChild));
            OSL_ENSURE(pNode != 0, "CNode::get returned 0");
            pNode->saxify(i_xHandler);
        }
        i_xHandler->endElement(name);
    }

    void CElement::fastSaxify( Context& i_rContext )
    {
        if (!i_rContext.mxDocHandler.is()) throw RuntimeException();
        pushContext(i_rContext);
        addNamespaces(i_rContext,m_aNodePtr);

        // add attributes
        i_rContext.mxAttribList->clear();
        for (xmlAttrPtr pAttr = m_aNodePtr->properties;
                        pAttr != 0; pAttr = pAttr->next) {
            ::rtl::Reference<CNode> const pNode = GetOwnerDocument().GetCNode(
                    reinterpret_cast<xmlNodePtr>(pAttr));
            OSL_ENSURE(pNode != 0, "CNode::get returned 0");

            const xmlChar* xName = pAttr->name;
            sal_Int32 nAttributeToken=FastToken::DONTKNOW;

            if( pAttr->ns && strlen((char*)pAttr->ns->prefix) )
                nAttributeToken = getTokenWithPrefix( i_rContext,
                                                      (sal_Char*)pAttr->ns->prefix,
                                                      (sal_Char*)xName );
            else
                nAttributeToken = getToken( i_rContext, (sal_Char*)xName );

            if( nAttributeToken != FastToken::DONTKNOW )
                i_rContext.mxAttribList->add( nAttributeToken,
                                              OUStringToOString(pNode->getNodeValue(),
                                                                RTL_TEXTENCODING_UTF8));
        }

        const xmlChar* xPrefix = m_aNodePtr->ns ? m_aNodePtr->ns->prefix : (const xmlChar*)"";
        const xmlChar* xName = m_aNodePtr->name;
        sal_Int32 nElementToken=FastToken::DONTKNOW;
        if( strlen((char*)xPrefix) )
            nElementToken = getTokenWithPrefix( i_rContext, (sal_Char*)xPrefix, (sal_Char*)xName );
        else
            nElementToken = getToken( i_rContext, (sal_Char*)xName );

        Reference<XFastContextHandler> xParentHandler(i_rContext.mxCurrentHandler);
        try
        {
            Reference< XFastAttributeList > xAttr( i_rContext.mxAttribList.get() );
            if( nElementToken == FastToken::DONTKNOW )
            {
                const OUString aNamespace;
                const OUString aElementName( (sal_Char*)xPrefix,
                                             strlen((char*)xPrefix),
                                             RTL_TEXTENCODING_UTF8 );

                if( xParentHandler.is() )
                    i_rContext.mxCurrentHandler = xParentHandler->createUnknownChildContext( aNamespace, aElementName, xAttr );
                else
                    i_rContext.mxCurrentHandler = i_rContext.mxDocHandler->createUnknownChildContext( aNamespace, aElementName, xAttr );

                if( i_rContext.mxCurrentHandler.is() )
                    i_rContext.mxCurrentHandler->startUnknownElement( aNamespace, aElementName, xAttr );
            }
            else
            {
                if( xParentHandler.is() )
                    i_rContext.mxCurrentHandler = xParentHandler->createFastChildContext( nElementToken, xAttr );
                else
                    i_rContext.mxCurrentHandler = i_rContext.mxDocHandler->createFastChildContext( nElementToken, xAttr );

                if( i_rContext.mxCurrentHandler.is() )
                    i_rContext.mxCurrentHandler->startFastElement( nElementToken, xAttr );
            }
        }
        catch( Exception& )
        {}

        // recurse
        for (xmlNodePtr pChild = m_aNodePtr->children;
                        pChild != 0; pChild = pChild->next) {
            ::rtl::Reference<CNode> const pNode(
                    GetOwnerDocument().GetCNode(pChild));
            OSL_ENSURE(pNode != 0, "CNode::get returned 0");
            pNode->fastSaxify(i_rContext);
        }

        if( i_rContext.mxCurrentHandler.is() ) try
        {
            if( nElementToken != FastToken::DONTKNOW )
                i_rContext.mxCurrentHandler->endFastElement( nElementToken );
            else
            {
                const OUString aNamespace;
                const OUString aElementName( (sal_Char*)xPrefix,
                                             strlen((char*)xPrefix),
                                             RTL_TEXTENCODING_UTF8 );

                i_rContext.mxCurrentHandler->endUnknownElement( aNamespace, aElementName );
            }
        }
        catch( Exception& )
        {}

        // restore after children have been processed
        i_rContext.mxCurrentHandler = xParentHandler;
        popContext(i_rContext);
    }

    bool CElement::IsChildTypeAllowed(NodeType const nodeType)
    {
        switch (nodeType) {
            case NodeType_ELEMENT_NODE:
            case NodeType_TEXT_NODE:
            case NodeType_COMMENT_NODE:
            case NodeType_PROCESSING_INSTRUCTION_NODE:
            case NodeType_CDATA_SECTION_NODE:
            case NodeType_ENTITY_REFERENCE_NODE:
                return true;
            case NodeType_ATTRIBUTE_NODE:
                /* this is not relly allowed by the DOM spec, but this
                   implementation has evidently supported it (by special case
                   handling, so the attribute does not actually become a child)
                   so allow it for backward compatiblity */
                return true;
            default:
                return false;
        }
    }


    /**
        Retrieves an attribute value by name.
        return empty string if attribute is not set
    */
    OUString SAL_CALL CElement::getAttribute(OUString const& name)
        throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        if (0 == m_aNodePtr) {
            return OUString();
        }
        // search properties
        OString o1 = OUStringToOString(name, RTL_TEXTENCODING_UTF8);
        ::boost::shared_ptr<xmlChar const> const pValue(
            xmlGetProp(m_aNodePtr, (xmlChar*)o1.getStr()), xmlFree);
        OUString const ret( (pValue)
            ?   OUString(reinterpret_cast<sal_Char const*>(pValue.get()),
                        strlen(reinterpret_cast<char const*>(pValue.get())),
                        RTL_TEXTENCODING_UTF8)
            :   OUString() );
        return ret;
    }

    /**
    Retrieves an attribute node by name.
    */
    Reference< XAttr > SAL_CALL CElement::getAttributeNode(OUString const& name)
        throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        if (0 == m_aNodePtr) {
            return 0;
        }
        OString o1 = OUStringToOString(name, RTL_TEXTENCODING_UTF8);
        xmlChar const*const pName =
            reinterpret_cast<xmlChar const*>(o1.getStr());
        xmlAttrPtr const pAttr = xmlHasProp(m_aNodePtr, pName);
        if (0 == pAttr) {
            return 0;
        }
        Reference< XAttr > const xRet(
            static_cast< XNode* >(GetOwnerDocument().GetCNode(
                    reinterpret_cast<xmlNodePtr>(pAttr)).get()),
            UNO_QUERY_THROW);
        return xRet;
    }

    /**
    Retrieves an Attr node by local name and namespace URI.
    */
    Reference< XAttr > SAL_CALL CElement::getAttributeNodeNS(
            const OUString& namespaceURI, const OUString& localName)
        throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        if (0 == m_aNodePtr) {
            return 0;
        }
        OString o1 = OUStringToOString(localName, RTL_TEXTENCODING_UTF8);
        xmlChar const*const pName =
            reinterpret_cast<xmlChar const*>(o1.getStr());
        OString o2 = OUStringToOString(namespaceURI, RTL_TEXTENCODING_UTF8);
        xmlChar const*const pNS =
            reinterpret_cast<xmlChar const*>(o2.getStr());
        xmlAttrPtr const pAttr = xmlHasNsProp(m_aNodePtr, pName, pNS);
        if (0 == pAttr) {
            return 0;
        }
        Reference< XAttr > const xRet(
            static_cast< XNode* >(GetOwnerDocument().GetCNode(
                    reinterpret_cast<xmlNodePtr>(pAttr)).get()),
            UNO_QUERY_THROW);
        return xRet;
    }

    /**
    Retrieves an attribute value by local name and namespace URI.
    return empty string if attribute is not set
    */
    OUString SAL_CALL
    CElement::getAttributeNS(
            OUString const& namespaceURI, OUString const& localName)
        throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        if (0 == m_aNodePtr) {
            return OUString();
        }
        OString o1 = OUStringToOString(localName, RTL_TEXTENCODING_UTF8);
        xmlChar const*const pName =
            reinterpret_cast<xmlChar const*>(o1.getStr());
        OString o2 = OUStringToOString(namespaceURI, RTL_TEXTENCODING_UTF8);
        xmlChar const*const pNS =
            reinterpret_cast<xmlChar const*>(o2.getStr());
        ::boost::shared_ptr<xmlChar const> const pValue(
                xmlGetNsProp(m_aNodePtr, pName, pNS), xmlFree);
        if (0 == pValue) {
            return OUString();
        }
        OUString const ret(reinterpret_cast<sal_Char const*>(pValue.get()),
                        strlen(reinterpret_cast<char const*>(pValue.get())),
                        RTL_TEXTENCODING_UTF8);
        return ret;
    }

    /**
    Returns a NodeList of all descendant Elements with a given tag name,
    in the order in which they are
    encountered in a preorder traversal of this Element tree.
    */
    Reference< XNodeList > SAL_CALL
    CElement::getElementsByTagName(OUString const& rLocalName)
        throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        Reference< XNodeList > const xList(
                new CElementList(this, m_rMutex, rLocalName));
        return xList;
    }

    /**
    Returns a NodeList of all the descendant Elements with a given local
    name and namespace URI in the order in which they are encountered in
    a preorder traversal of this Element tree.
    */
    Reference< XNodeList > SAL_CALL
    CElement::getElementsByTagNameNS(
            OUString const& rNamespaceURI, OUString const& rLocalName)
        throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        Reference< XNodeList > const xList(
            new CElementList(this, m_rMutex, rLocalName, &rNamespaceURI));
        return xList;
    }

    /**
    The name of the element.
    */
    OUString SAL_CALL CElement::getTagName()
        throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        if (0 == m_aNodePtr) {
            return OUString();
        }
        OUString const ret((sal_Char*)m_aNodePtr->name,
                strlen((char*)m_aNodePtr->name), RTL_TEXTENCODING_UTF8);
        return ret;
    }


    /**
    Returns true when an attribute with a given name is specified on this
    element or has a default value, false otherwise.
    */
    sal_Bool SAL_CALL CElement::hasAttribute(OUString const& name)
        throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        OString o1 = OUStringToOString(name, RTL_TEXTENCODING_UTF8);
        xmlChar *xName = (xmlChar*)o1.getStr();
        return (m_aNodePtr != NULL && xmlHasProp(m_aNodePtr, xName) != NULL);
    }

    /**
    Returns true when an attribute with a given local name and namespace
    URI is specified on this element or has a default value, false otherwise.
    */
    sal_Bool SAL_CALL CElement::hasAttributeNS(
            OUString const& namespaceURI, OUString const& localName)
        throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        OString o1 = OUStringToOString(localName, RTL_TEXTENCODING_UTF8);
        xmlChar *xName = (xmlChar*)o1.getStr();
        OString o2 = OUStringToOString(namespaceURI, RTL_TEXTENCODING_UTF8);
        xmlChar *xNs = (xmlChar*)o2.getStr();
        return (m_aNodePtr != NULL && xmlHasNsProp(m_aNodePtr, xName, xNs) != NULL);
    }

    /**
    Removes an attribute by name.
    */
    void SAL_CALL CElement::removeAttribute(OUString const& name)
        throw (RuntimeException, DOMException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        if (0 == m_aNodePtr) {
            return;
        }
        OString o1 = OUStringToOString(name, RTL_TEXTENCODING_UTF8);
        xmlChar const*const pName =
            reinterpret_cast<xmlChar const*>(o1.getStr());
        xmlAttrPtr const pAttr = xmlHasProp(m_aNodePtr, pName);
        if (0 == xmlUnsetProp(m_aNodePtr, pName)) {
            ::rtl::Reference<CNode> const pCNode(GetOwnerDocument().GetCNode(
                    reinterpret_cast<xmlNodePtr>(pAttr), false));
            if (pCNode.is()) {
                pCNode->invalidate(); // freed by xmlUnsetProp
            }
        }
    }

    /**
    Removes an attribute by local name and namespace URI.
    */
    void SAL_CALL CElement::removeAttributeNS(
            OUString const& namespaceURI, OUString const& localName)
        throw (RuntimeException, DOMException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        if (0 == m_aNodePtr) {
            return;
        }
        OString o1 = OUStringToOString(localName, RTL_TEXTENCODING_UTF8);
        xmlChar const*const pName =
            reinterpret_cast<xmlChar const*>(o1.getStr());
        OString o2 = OUStringToOString(namespaceURI, RTL_TEXTENCODING_UTF8);
        xmlChar const*const pURI =
            reinterpret_cast<xmlChar const*>(o2.getStr());
        xmlNsPtr const pNs =
            xmlSearchNsByHref(m_aNodePtr->doc, m_aNodePtr, pURI);
        xmlAttrPtr const pAttr = xmlHasNsProp(m_aNodePtr, pName, pURI);
        if (0 == xmlUnsetNsProp(m_aNodePtr, pNs, pName)) {
            ::rtl::Reference<CNode> const pCNode(GetOwnerDocument().GetCNode(
                    reinterpret_cast<xmlNodePtr>(pAttr), false));
            if (pCNode.is()) {
                pCNode->invalidate(); // freed by xmlUnsetNsProp
            }
        }
    }

    /**
    Removes the specified attribute node.
    */
    Reference< XAttr > SAL_CALL
    CElement::removeAttributeNode(Reference< XAttr > const& oldAttr)
        throw (RuntimeException, DOMException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        if (0 == m_aNodePtr) {
            return 0;
        }

        ::rtl::Reference<CNode> const pCNode(
            CNode::GetImplementation(Reference<XNode>(oldAttr.get())));
        if (!pCNode.is()) { throw RuntimeException(); }

        xmlNodePtr const pNode = pCNode->GetNodePtr();
        xmlAttrPtr const pAttr = (xmlAttrPtr) pNode;
        if (!pAttr) { throw RuntimeException(); }

        if (pAttr->parent != m_aNodePtr)
        {
            DOMException e;
            e.Code = DOMExceptionType_HIERARCHY_REQUEST_ERR;
            throw e;
        }
        if (pAttr->doc != m_aNodePtr->doc)
        {
            DOMException e;
            e.Code = DOMExceptionType_WRONG_DOCUMENT_ERR;
            throw e;
        }

        Reference< XAttr > aAttr;
        if (!oldAttr->getNamespaceURI().isEmpty()) {
            OUStringBuffer qname(oldAttr->getPrefix());
            if (!qname.isEmpty()) {
                qname.append(sal_Unicode(':'));
            }
            qname.append(oldAttr->getName());
            aAttr = GetOwnerDocument().createAttributeNS(
                oldAttr->getNamespaceURI(), qname.makeStringAndClear());
        } else {
            aAttr = GetOwnerDocument().createAttribute(oldAttr->getName());
        }
        aAttr->setValue(oldAttr->getValue());
        xmlRemoveProp(pAttr);
        pCNode->invalidate(); // freed by xmlRemoveProp

        return aAttr;
    }

    /**
    Adds a new attribute node.
    */
    Reference< XAttr >
    CElement::setAttributeNode_Impl_Lock(
            Reference< XAttr > const& xNewAttr, bool const bNS)
    {
        if (xNewAttr->getOwnerDocument() != getOwnerDocument()) {
            DOMException e;
            e.Code = DOMExceptionType_WRONG_DOCUMENT_ERR;
            throw e;
        }

        ::osl::ClearableMutexGuard guard(m_rMutex);

        if (0 == m_aNodePtr) {
            throw RuntimeException();
        }

        // get the implementation
        CAttr *const pCAttr = dynamic_cast<CAttr*>(
                CNode::GetImplementation(xNewAttr));
        if (!pCAttr) { throw RuntimeException(); }
        xmlAttrPtr const pAttr =
            reinterpret_cast<xmlAttrPtr>(pCAttr->GetNodePtr());
        if (!pAttr) { throw RuntimeException(); }

        // check whether the attribute is not in use by another element
        if (pAttr->parent) {
            DOMException e;
            e.Code = DOMExceptionType_INUSE_ATTRIBUTE_ERR;
            throw e;
        }

        xmlAttrPtr res = NULL;
        xmlChar const*const pContent(
                (pAttr->children) ? pAttr->children->content : 0);

        if (bNS) {
            xmlNsPtr const pNs( pCAttr->GetNamespace(m_aNodePtr) );
            res = xmlNewNsProp(m_aNodePtr, pNs, pAttr->name, pContent);
        } else {
            res = xmlNewProp(m_aNodePtr, pAttr->name, pContent);
        }

        // get the new attr node
        Reference< XAttr > const xAttr(
            static_cast< XNode* >(GetOwnerDocument().GetCNode(
                    reinterpret_cast<xmlNodePtr>(res)).get()),
            UNO_QUERY_THROW);

        // attribute addition event
        // dispatch DOMAttrModified event
        Reference< XDocumentEvent > docevent(getOwnerDocument(), UNO_QUERY);
        Reference< XMutationEvent > event(docevent->createEvent(
            "DOMAttrModified"), UNO_QUERY);
        event->initMutationEvent("DOMAttrModified",
            sal_True, sal_False, xAttr,
            OUString(), xAttr->getValue(), xAttr->getName(),
            AttrChangeType_ADDITION);

        guard.clear(); // release mutex before calling event handlers

        dispatchEvent(event);
        dispatchSubtreeModified();

        return xAttr;
    }

    Reference< XAttr >
    CElement::setAttributeNode(const Reference< XAttr >& newAttr)
        throw (RuntimeException, DOMException)
    {
        return setAttributeNode_Impl_Lock(newAttr, false);
    }

    /**
    Adds a new attribute.
    */
    Reference< XAttr >
    CElement::setAttributeNodeNS(const Reference< XAttr >& newAttr)
        throw (RuntimeException, DOMException)
    {
        return setAttributeNode_Impl_Lock(newAttr, true);
    }

    /**
    Adds a new attribute.
    */
    void SAL_CALL
    CElement::setAttribute(OUString const& name, OUString const& value)
        throw (RuntimeException, DOMException)
    {
        ::osl::ClearableMutexGuard guard(m_rMutex);

        OString o1 = OUStringToOString(name, RTL_TEXTENCODING_UTF8);
        xmlChar *xName = (xmlChar*)o1.getStr();
        OString o2 = OUStringToOString(value, RTL_TEXTENCODING_UTF8);
        xmlChar *xValue = (xmlChar*)o2.getStr();

        if (0 == m_aNodePtr) {
            throw RuntimeException();
        }
        OUString oldValue;
        AttrChangeType aChangeType = AttrChangeType_MODIFICATION;
        ::boost::shared_ptr<xmlChar const> const pOld(
            xmlGetProp(m_aNodePtr, xName), xmlFree);
        if (pOld == 0) {
            aChangeType = AttrChangeType_ADDITION;
            xmlNewProp(m_aNodePtr, xName, xValue);
        } else {
            oldValue = OUString(reinterpret_cast<sal_Char const*>(pOld.get()),
                        strlen(reinterpret_cast<char const*>(pOld.get())),
                        RTL_TEXTENCODING_UTF8);
            xmlSetProp(m_aNodePtr, xName, xValue);
        }

        // dispatch DOMAttrModified event
        Reference< XDocumentEvent > docevent(getOwnerDocument(), UNO_QUERY);
        Reference< XMutationEvent > event(docevent->createEvent(
            "DOMAttrModified"), UNO_QUERY);
        event->initMutationEvent("DOMAttrModified",
            sal_True, sal_False,
            Reference< XNode >(getAttributeNode(name), UNO_QUERY),
            oldValue, value, name, aChangeType);

        guard.clear(); // release mutex before calling event handlers
        dispatchEvent(event);
        dispatchSubtreeModified();
    }

    /**
    Adds a new attribute.
    */
    void SAL_CALL
    CElement::setAttributeNS(OUString const& namespaceURI,
            OUString const& qualifiedName, OUString const& value)
        throw (RuntimeException, DOMException)
    {
        if (namespaceURI.isEmpty()) throw RuntimeException();

        ::osl::ClearableMutexGuard guard(m_rMutex);

        OString o1, o2, o3, o4, o5;
        xmlChar *xPrefix = NULL;
        xmlChar *xLName = NULL;
        o1 = OUStringToOString(qualifiedName, RTL_TEXTENCODING_UTF8);
        xmlChar *xQName = (xmlChar*)o1.getStr();
        sal_Int32 idx = qualifiedName.indexOf(':');
        if (idx != -1)
        {
            o2 = OUStringToOString(
                qualifiedName.copy(0,idx),
                RTL_TEXTENCODING_UTF8);
            xPrefix = (xmlChar*)o2.getStr();
            o3 = OUStringToOString(
                qualifiedName.copy(idx+1),
                RTL_TEXTENCODING_UTF8);
            xLName = (xmlChar*)o3.getStr();
        }  else {
            xPrefix = (xmlChar*)"";
            xLName = xQName;
        }
        o4 = OUStringToOString(namespaceURI, RTL_TEXTENCODING_UTF8);
        o5 = OUStringToOString(value, RTL_TEXTENCODING_UTF8);
        xmlChar *xURI= (xmlChar*)o4.getStr();
        xmlChar *xValue = (xmlChar*)o5.getStr();

        if (0 == m_aNodePtr) {
            throw RuntimeException();
        }

        //find the right namespace
        xmlNsPtr pNs = xmlSearchNs(m_aNodePtr->doc, m_aNodePtr, xPrefix);
        // if no namespace found, create a new one
        if (pNs == NULL) {
            pNs = xmlNewNs(m_aNodePtr, xURI, xPrefix);
        }

        if (strcmp((char*)pNs->href, (char*)xURI) != 0) {
            // ambiguous ns prefix
            throw RuntimeException();
        }

        // found namespace matches

        OUString oldValue;
        AttrChangeType aChangeType = AttrChangeType_MODIFICATION;
        ::boost::shared_ptr<xmlChar const> const pOld(
                xmlGetNsProp(m_aNodePtr, xLName, pNs->href), xmlFree);
        if (pOld == 0) {
            aChangeType = AttrChangeType_ADDITION;
            xmlNewNsProp(m_aNodePtr, pNs, xLName, xValue);
        } else {
            oldValue = OUString(reinterpret_cast<sal_Char const*>(pOld.get()),
                        strlen(reinterpret_cast<char const*>(pOld.get())),
                        RTL_TEXTENCODING_UTF8);
            xmlSetNsProp(m_aNodePtr, pNs, xLName, xValue);
        }
        // dispatch DOMAttrModified event
        Reference< XDocumentEvent > docevent(getOwnerDocument(), UNO_QUERY);
        Reference< XMutationEvent > event(docevent->createEvent(
            "DOMAttrModified"), UNO_QUERY);
        event->initMutationEvent(
            "DOMAttrModified", sal_True, sal_False,
            Reference< XNode >(getAttributeNodeNS(namespaceURI, OUString((char*)xLName, strlen((char*)xLName), RTL_TEXTENCODING_UTF8)), UNO_QUERY),
            oldValue, value, qualifiedName, aChangeType);

        guard.clear(); // release mutex before calling event handlers
        dispatchEvent(event);
        dispatchSubtreeModified();
    }

    Reference< XNamedNodeMap > SAL_CALL
    CElement::getAttributes() throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        Reference< XNamedNodeMap > const xMap(
                new CAttributesMap(this, m_rMutex));
        return xMap;
    }

    OUString SAL_CALL CElement::getNodeName()throw (RuntimeException)
    {
        return getLocalName();
    }

    OUString SAL_CALL CElement::getLocalName()throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        OUString aName;
        if (m_aNodePtr != NULL)
        {
            const xmlChar* xName = m_aNodePtr->name;
            aName = OUString((const sal_Char*)xName, strlen((const char*)xName), RTL_TEXTENCODING_UTF8);
        }
        return aName;
    }

    OUString SAL_CALL CElement::getNodeValue() throw (RuntimeException)
    {
        return OUString();
    }

    void SAL_CALL CElement::setElementName(const OUString& aName)
        throw (RuntimeException, DOMException)
    {
        if (aName.isEmpty() || (0 <= aName.indexOf(':')))
        {
            DOMException e;
            e.Code = DOMExceptionType_INVALID_CHARACTER_ERR;
            throw e;
        }

        ::osl::MutexGuard const g(m_rMutex);

        if (0 == m_aNodePtr) {
            throw RuntimeException();
        }
        OString oName = OUStringToOString(aName, RTL_TEXTENCODING_UTF8);
        xmlChar *xName = (xmlChar*)oName.getStr();
        xmlNodeSetName(m_aNodePtr, xName);
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
