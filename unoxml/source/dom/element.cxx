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

#include <element.hxx>

#include <string.h>

#include <com/sun/star/xml/sax/FastToken.hdl>

#include <comphelper/attributelist.hxx>

#include <node.hxx>
#include <attr.hxx>
#include <elementlist.hxx>
#include <attributesmap.hxx>

#include "../events/mutationevent.hxx"

#include <document.hxx>


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
        OUString type = OUString::createFromAscii("");
        // add namespace definitions to attributes
        for (xmlNsPtr pNs = m_aNodePtr->nsDef; pNs != 0; pNs = pNs->next) {
            const xmlChar *pPrefix = pNs->prefix;
            OUString prefix(reinterpret_cast<const sal_Char*>(pPrefix),
                strlen(reinterpret_cast<const char*>(pPrefix)),
                RTL_TEXTENCODING_UTF8);
            OUString name = (prefix.equalsAscii(""))
                ? OUString::createFromAscii("xmlns")
                : OUString::createFromAscii("xmlns:") + prefix;
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
            OUString name = (prefix.getLength() == 0)
                ? pNode->getLocalName()
                : prefix + OUString(static_cast<sal_Unicode>(':')) + pNode->getLocalName();
            OUString val  = pNode->getNodeValue();
            pAttrs->AddAttribute(name, type, val);
        }
        OUString prefix = getPrefix();
        OUString name = (prefix.getLength() == 0)
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

    /**
        Retrieves an attribute value by name.
        return empty string if attribute is not set
    */
    OUString SAL_CALL CElement::getAttribute(OUString const& name)
        throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        OUString aValue;
        // search properties
        if (m_aNodePtr != NULL)
        {
            OString o1 = OUStringToOString(name, RTL_TEXTENCODING_UTF8);
            xmlChar *xValue = xmlGetProp(m_aNodePtr, (xmlChar*)o1.getStr());
            if (xValue != NULL) {
                aValue = OUString((sal_Char*)xValue, strlen((char*)xValue), RTL_TEXTENCODING_UTF8);
            }
        }
        return aValue;
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
            return ::rtl::OUString();
        }
        OString o1 = OUStringToOString(localName, RTL_TEXTENCODING_UTF8);
        xmlChar const*const pName =
            reinterpret_cast<xmlChar const*>(o1.getStr());
        OString o2 = OUStringToOString(namespaceURI, RTL_TEXTENCODING_UTF8);
        xmlChar const*const pNS =
            reinterpret_cast<xmlChar const*>(o2.getStr());
        xmlChar *const pValue = xmlGetNsProp(m_aNodePtr, pName, pNS);
        if (0 == pValue) {
            return ::rtl::OUString();
        }
        OUString const ret(reinterpret_cast<sal_Char const*>(pValue),
                        strlen(reinterpret_cast<char const*>(pValue)),
                        RTL_TEXTENCODING_UTF8);
        xmlFree(pValue);
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

        OUString aName;
        if (m_aNodePtr != NULL)
        {
            aName = OUString((sal_Char*)m_aNodePtr->name, strlen((char*)m_aNodePtr->name), RTL_TEXTENCODING_UTF8);
        }
        return aName;
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

        xmlChar *xName = (xmlChar*)OUStringToOString(name, RTL_TEXTENCODING_UTF8).getStr();
        if (m_aNodePtr != NULL) {
            xmlUnsetProp(m_aNodePtr, xName);
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

        OString o1 = OUStringToOString(localName, RTL_TEXTENCODING_UTF8);
        xmlChar *xName = (xmlChar*)o1.getStr();
        OString o2 = OUStringToOString(namespaceURI, RTL_TEXTENCODING_UTF8);
        xmlChar *xURI = (xmlChar*)o2.getStr();
        if (m_aNodePtr != NULL) {
            // XXX
            xmlNsPtr pNs = xmlSearchNsByHref(m_aNodePtr->doc, m_aNodePtr, xURI);
            xmlUnsetNsProp(m_aNodePtr, pNs, xName);
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

        Reference< XAttr > aAttr;
        if(m_aNodePtr != NULL)
        {
            ::rtl::Reference<CNode> const pCNode(
                CNode::GetImplementation(Reference<XNode>(oldAttr.get())));
            if (!pCNode.is()) { throw RuntimeException(); }

            xmlNodePtr const pNode = pCNode->GetNodePtr();
            xmlAttrPtr const pAttr = (xmlAttrPtr) pNode;

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

            if (oldAttr->getNamespaceURI().getLength() > 0)
                aAttr = oldAttr->getOwnerDocument()->createAttributeNS(
                    oldAttr->getNamespaceURI(), oldAttr->getName());
            else
                aAttr = oldAttr->getOwnerDocument()->createAttribute(oldAttr->getName());
            aAttr->setValue(oldAttr->getValue());
            xmlRemoveProp(pAttr);
            pCNode->invalidate(); // freed by xmlRemoveProp
        }
        return aAttr;
    }

    /**
    Adds a new attribute node.
    */
    Reference< XAttr >
    CElement::setAttributeNode_Impl_Lock(
            Reference< XAttr > const& newAttr, bool const bNS)
        throw (RuntimeException)
    {
        // check whether the attrib belongs to this document
        Reference< XDocument > newDoc(newAttr->getOwnerDocument(), UNO_QUERY);
        Reference< XDocument > oldDoc(CNode::getOwnerDocument(), UNO_QUERY);
        if (newDoc != oldDoc) {
            throw RuntimeException();
        }

        ::osl::ClearableMutexGuard guard(m_rMutex);

        Reference< XAttr > aAttr;
        if (m_aNodePtr != NULL)
        {
            // get the implementation
            CNode *const pCNode = CNode::GetImplementation(newAttr);
            if (!pCNode) { throw RuntimeException(); }
            xmlAttrPtr const pAttr =
                reinterpret_cast<xmlAttrPtr>(pCNode->GetNodePtr());
            if (!pAttr) { throw RuntimeException(); }

            // check whether the attribute is not in use by another element
            xmlNsPtr pNs = NULL;
            if (pAttr->parent != NULL)
                if(strcmp((char*)pAttr->parent->name, "__private") == 0
                    && pNs && pAttr->ns != NULL)
                {
                    pNs = xmlSearchNs(m_aNodePtr->doc, m_aNodePtr, pAttr->ns->prefix);
                    if (pNs == NULL || strcmp((char*)pNs->href, (char*)pAttr->ns->href) !=0 )
                        pNs = xmlNewNs(m_aNodePtr, pAttr->ns->href, pAttr->ns->href);
                else
                    throw RuntimeException();
            }

            xmlAttrPtr res = NULL;

            if (bNS)
                res = xmlNewNsProp(m_aNodePtr, pNs, pAttr->name, pAttr->children->content);
            else
                res = xmlNewProp(m_aNodePtr, pAttr->name, pAttr->children->content);

            // free carrier node ...
            if(pAttr->parent != NULL && strcmp((char*)pAttr->parent->name, "__private")== 0)
                xmlFreeNode(pAttr->parent);

            // get the new attr node
            aAttr = Reference< XAttr >(
                static_cast< XNode* >(GetOwnerDocument().GetCNode(
                        reinterpret_cast<xmlNodePtr>(res)).get()),
                UNO_QUERY_THROW);
        }

        if (aAttr.is())
        {
            // attribute adition event
            // dispatch DOMAttrModified event
            Reference< XDocumentEvent > docevent(getOwnerDocument(), UNO_QUERY);
            Reference< XMutationEvent > event(docevent->createEvent(
                OUString::createFromAscii("DOMAttrModified")), UNO_QUERY);
            event->initMutationEvent(OUString::createFromAscii("DOMAttrModified"),
                sal_True, sal_False, Reference< XNode >(aAttr, UNO_QUERY),
                OUString(), aAttr->getValue(), aAttr->getName(), AttrChangeType_ADDITION);

            guard.clear(); // release mutex before calling event handlers

            dispatchEvent(Reference< XEvent >(event, UNO_QUERY));
            dispatchSubtreeModified();
        }
        return aAttr;
    }

    Reference< XAttr > CElement::setAttributeNode(const Reference< XAttr >& newAttr)
        throw (RuntimeException, DOMException)
    {
        return setAttributeNode_Impl_Lock(newAttr, false);
    }

    /**
    Adds a new attribute.
    */
    Reference< XAttr > CElement::setAttributeNodeNS(const Reference< XAttr >& newAttr)
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
        if (m_aNodePtr != NULL)
        {
            OUString oldValue;
            AttrChangeType aChangeType = AttrChangeType_MODIFICATION;
            xmlChar *xOld = xmlGetProp(m_aNodePtr, xName);
            if (xOld == NULL)
            {
                aChangeType = AttrChangeType_ADDITION;
                xmlNewProp(m_aNodePtr, xName, xValue);
            }
            else
            {
                oldValue = OUString((char*)xOld, strlen((char*)xOld), RTL_TEXTENCODING_UTF8);
                xmlSetProp(m_aNodePtr, xName, xValue);
            }

            // dispatch DOMAttrModified event

            Reference< XDocumentEvent > docevent(getOwnerDocument(), UNO_QUERY);
            Reference< XMutationEvent > event(docevent->createEvent(
                OUString::createFromAscii("DOMAttrModified")), UNO_QUERY);
            event->initMutationEvent(OUString::createFromAscii("DOMAttrModified"),
                sal_True, sal_False, Reference< XNode >(getAttributeNode(name), UNO_QUERY),
                oldValue, value, name, aChangeType);

            guard.clear(); // release mutex before calling event handlers
            dispatchEvent(Reference< XEvent >(event, UNO_QUERY));
            dispatchSubtreeModified();
        }
    }

    /**
    Adds a new attribute.
    */
    void SAL_CALL
    CElement::setAttributeNS(OUString const& namespaceURI,
            OUString const& qualifiedName, OUString const& value)
        throw (RuntimeException, DOMException)
    {
        if (namespaceURI.getLength() == 0) throw RuntimeException();

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
        if (m_aNodePtr != NULL)
        {
            //find the right namespace
            xmlNsPtr pNs = xmlSearchNs(m_aNodePtr->doc, m_aNodePtr, xPrefix);
            // if no namespace found, create a new one
            if (pNs == NULL)
                pNs = xmlNewNs(m_aNodePtr, xURI, xPrefix);

            if (strcmp((char*)pNs->href, (char*)xURI) == 0)
            {
                // found namespace matches

                OUString oldValue;
                AttrChangeType aChangeType = AttrChangeType_MODIFICATION;
                xmlChar *xOld = xmlGetNsProp(m_aNodePtr, xLName, pNs->href);
                if (xOld == NULL)
                {
                    aChangeType = AttrChangeType_ADDITION;
                    xmlNewNsProp(m_aNodePtr, pNs, xLName, xValue);
                }
                else
                {
                    oldValue = OUString((char *)xOld, strlen((char *)xOld), RTL_TEXTENCODING_UTF8);
                    xmlSetNsProp(m_aNodePtr, pNs, xLName, xValue);
                }
                // dispatch DOMAttrModified event
                Reference< XDocumentEvent > docevent(getOwnerDocument(), UNO_QUERY);
                Reference< XMutationEvent > event(docevent->createEvent(
                    OUString::createFromAscii("DOMAttrModified")), UNO_QUERY);
                event->initMutationEvent(OUString::createFromAscii("DOMAttrModified"), sal_True, sal_False,
                    Reference< XNode >(getAttributeNodeNS(namespaceURI, OUString((char*)xLName, strlen((char*)xLName), RTL_TEXTENCODING_UTF8)), UNO_QUERY),
                    oldValue, value, qualifiedName, aChangeType);

                guard.clear(); // release mutex before calling event handlers
                dispatchEvent(Reference< XEvent >(event, UNO_QUERY));
                dispatchSubtreeModified();

            } else {
                // ambigious ns prefix
                throw RuntimeException();
            }

        }
    }

    Reference< XNamedNodeMap > SAL_CALL
    CElement::getAttributes() throw (RuntimeException)
    {
        ::osl::MutexGuard const g(m_rMutex);

        if (!hasAttributes()) { return 0; }
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
        if ((aName.getLength() <= 0) ||
            (0 <= aName.indexOf(OUString::createFromAscii(":"))))
        {
            DOMException e;
            e.Code = DOMExceptionType_INVALID_CHARACTER_ERR;
            throw e;
        }

        ::osl::MutexGuard const g(m_rMutex);

        OString oName = OUStringToOString(aName, RTL_TEXTENCODING_UTF8);
        xmlChar *xName = (xmlChar*)oName.getStr();
        // xmlFree((void*)m_aNodePtr->name);
        m_aNodePtr->name = xmlStrdup(xName);
    }

}
