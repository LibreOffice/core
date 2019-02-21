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

#include <node.hxx>

#include <stdio.h>
#include <string.h>

#include <libxml/xmlstring.h>

#include <algorithm>

#include <rtl/uuid.h>
#include <rtl/instance.hxx>
#include <osl/mutex.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>

#include <com/sun/star/xml/sax/FastToken.hpp>

#include <comphelper/servicehelper.hxx>

#include "document.hxx"
#include "attr.hxx"
#include "childlist.hxx"

#include <eventdispatcher.hxx>
#include <mutationevent.hxx>

using namespace css;
using namespace css::uno;
using namespace css::xml::dom;
using namespace css::xml::dom::events;
using namespace css::xml::sax;

namespace
{
    class theCNodeUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theCNodeUnoTunnelId > {};
}

namespace DOM
{
    void pushContext(Context& io_rContext)
    {
        // Explicitly use a temp. variable.
        // Windows/VC++ seems to mess up if .back() is directly passed as
        // parameter. i.e. Don't use push_back( .back() );
        Context::NamespaceVectorType::value_type aVal = io_rContext.maNamespaces.back();
        io_rContext.maNamespaces.push_back( aVal );
    }

    void popContext(Context& io_rContext)
    {
        io_rContext.maNamespaces.pop_back();
    }

    void addNamespaces(Context& io_rContext, xmlNodePtr pNode)
    {
        // add node's namespaces to current context
        for (xmlNsPtr pNs = pNode->nsDef; pNs != nullptr; pNs = pNs->next) {
            const xmlChar *pPrefix = pNs->prefix;
            // prefix can be NULL when xmlns attribute is empty (xmlns="")
            OString prefix(reinterpret_cast<const sal_Char*>(pPrefix),
                           pPrefix ? strlen(reinterpret_cast<const char*>(pPrefix)) : 0);
            const xmlChar *pHref = pNs->href;
            OUString val(reinterpret_cast<const sal_Char*>(pHref),
                strlen(reinterpret_cast<const char*>(pHref)),
                RTL_TEXTENCODING_UTF8);

            Context::NamespaceMapType::iterator aIter=
                io_rContext.maNamespaceMap.find(val);
            if( aIter != io_rContext.maNamespaceMap.end() )
            {
                Context::Namespace aNS;
                aNS.maPrefix = prefix;
                aNS.mnToken = aIter->second;

                io_rContext.maNamespaces.back().push_back(aNS);

                SAL_INFO("unoxml", "Added with token " << aIter->second);
            }
        }
    }

    sal_Int32 getToken( const Context& rContext, const sal_Char* pToken )
    {
        const Sequence<sal_Int8> aSeq( reinterpret_cast<sal_Int8 const *>(pToken), strlen( pToken ) );
        return rContext.mxTokenHandler->getTokenFromUTF8( aSeq );
    }

    sal_Int32 getTokenWithPrefix( const Context& rContext, const sal_Char* pPrefix, const sal_Char* pName )
    {
        sal_Int32 nNamespaceToken = FastToken::DONTKNOW;
        OString prefix(pPrefix,
                       strlen(reinterpret_cast<const char*>(pPrefix)));

        SAL_INFO("unoxml", "getTokenWithPrefix(): prefix " << pPrefix << ", name " << pName);

        Context::NamespaceVectorType::value_type::const_iterator aIter;
        if( (aIter=std::find_if(rContext.maNamespaces.back().begin(),
                                rContext.maNamespaces.back().end(),
                                [&prefix](const Context::Namespace &aNamespace){ return aNamespace.getPrefix() == prefix; } )) !=
                                            rContext.maNamespaces.back().end() )
        {
            nNamespaceToken = aIter->mnToken;
            sal_Int32 nNameToken = getToken( rContext, pName );
            if( nNameToken == FastToken::DONTKNOW )
                nNamespaceToken = FastToken::DONTKNOW;
            else
                nNamespaceToken |= nNameToken;
        }

        return nNamespaceToken;
    }


    CNode::CNode(CDocument const& rDocument, ::osl::Mutex const& rMutex,
                NodeType const& reNodeType, xmlNodePtr const& rpNode)
        :   m_bUnlinked(false)
        ,   m_aNodeType(reNodeType)
        ,   m_aNodePtr(rpNode)
        // keep containing document alive
        // (but not if this is a document; that would create a leak!)
        ,   m_xDocument( (m_aNodePtr->type != XML_DOCUMENT_NODE)
                ? &const_cast<CDocument&>(rDocument) : nullptr )
        ,   m_rMutex(const_cast< ::osl::Mutex & >(rMutex))
    {
        OSL_ASSERT(m_aNodePtr);
    }

    void CNode::invalidate()
    {
        //remove from list if this wrapper goes away
        if (m_aNodePtr != nullptr && m_xDocument.is()) {
            m_xDocument->RemoveCNode(m_aNodePtr, this);
        }
        // #i113663#: unlinked nodes will not be freed by xmlFreeDoc
        if (m_bUnlinked) {
            xmlFreeNode(m_aNodePtr);
        }
        m_aNodePtr = nullptr;
    }

    CNode::~CNode()
    {
        // if this is the document itself, the mutex is already freed!
        if (NodeType_DOCUMENT_NODE == m_aNodeType) {
            invalidate();
        } else {
            ::osl::MutexGuard const g(m_rMutex);
            invalidate(); // other nodes are still alive so must lock mutex
        }
    }

    CNode *
    CNode::GetImplementation(uno::Reference<uno::XInterface> const& xNode)
    {
        uno::Reference<lang::XUnoTunnel> const xUnoTunnel(xNode, UNO_QUERY);
        if (!xUnoTunnel.is()) { return nullptr; }
        CNode *const pCNode( reinterpret_cast< CNode* >(
                        ::sal::static_int_cast< sal_IntPtr >(
                            xUnoTunnel->getSomething(theCNodeUnoTunnelId::get().getSeq()))));
        return pCNode;
    }

    CDocument & CNode::GetOwnerDocument()
    {
        OSL_ASSERT(m_xDocument.is());
        return *m_xDocument; // needs overriding in CDocument!
    }


    static void lcl_nsexchange(
            xmlNodePtr const aNode, xmlNsPtr const oldNs, xmlNsPtr const newNs)
    {
        // recursively exchange any references to oldNs with references to newNs
        xmlNodePtr cur = aNode;
        while (cur != nullptr)
        {
            if (cur->ns == oldNs)
                cur->ns = newNs;
            if (cur->type == XML_ELEMENT_NODE)
            {
                xmlAttrPtr curAttr = cur->properties;
                while(curAttr != nullptr)
                {
                    if (curAttr->ns == oldNs)
                        curAttr->ns = newNs;
                    curAttr = curAttr->next;
                }
                lcl_nsexchange(cur->children, oldNs, newNs);
            }
            cur = cur->next;
        }
    }

    /*static*/ void nscleanup(const xmlNodePtr aNode, const xmlNodePtr aParent)
    {
        xmlNodePtr cur = aNode;

        //handle attributes
        if (cur != nullptr && cur->type == XML_ELEMENT_NODE)
        {
            xmlAttrPtr curAttr = cur->properties;
            while(curAttr != nullptr)
            {
                if (curAttr->ns != nullptr)
                {
                    xmlNsPtr ns = xmlSearchNs(cur->doc, aParent, curAttr->ns->prefix);
                    if (ns != nullptr)
                        curAttr->ns = ns;
                }
                curAttr = curAttr->next;
            }
        }

        while (cur != nullptr)
        {
            nscleanup(cur->children, cur);
            if (cur->ns != nullptr)
            {
                xmlNsPtr ns = xmlSearchNs(cur->doc, aParent, cur->ns->prefix);
                if (ns != nullptr && ns != cur->ns && strcmp(reinterpret_cast<char const *>(ns->href), reinterpret_cast<char const *>(cur->ns->href))==0)
                {
                    xmlNsPtr curDef = cur->nsDef;
                    xmlNsPtr *refp = &(cur->nsDef); // insert point
                    while (curDef != nullptr)
                    {
                        ns = xmlSearchNs(cur->doc, aParent, curDef->prefix);
                        if (ns != nullptr && ns != curDef && strcmp(reinterpret_cast<char const *>(ns->href), reinterpret_cast<char const *>(curDef->href))==0)
                        {
                            // reconnect ns pointers in sub-tree to newly found ns before
                            // removing redundant nsdecl to prevent dangling pointers.
                            lcl_nsexchange(cur, curDef, ns);
                            *refp = curDef->next;
                            xmlFreeNs(curDef);
                            curDef = *refp;
                        } else {
                            refp = &(curDef->next);
                            curDef = curDef->next;
                        }
                    }
                }
            }
            cur = cur->next;
        }
    }

    void CNode::saxify(const Reference< XDocumentHandler >& i_xHandler)
    {
        if (!i_xHandler.is()) throw RuntimeException();
        // default: do nothing
    }

    void CNode::fastSaxify(Context& io_rContext)
    {
        if (!io_rContext.mxDocHandler.is()) throw RuntimeException();
        // default: do nothing
    }

    bool CNode::IsChildTypeAllowed(NodeType const /*nodeType*/)
    {
        // default: no children allowed
        return false;
    }

    /**
    Adds the node newChild to the end of the list of children of this node.
    */
    Reference< XNode > SAL_CALL CNode::appendChild(
            Reference< XNode > const& xNewChild)
    {
        ::osl::ClearableMutexGuard guard(m_rMutex);

        if (nullptr == m_aNodePtr) { return nullptr; }

        CNode *const pNewChild(CNode::GetImplementation(xNewChild));
        if (!pNewChild) { throw RuntimeException(); }
        xmlNodePtr const cur = pNewChild->GetNodePtr();
        if (!cur) { throw RuntimeException(); }

        // error checks:
        // from other document
        if (cur->doc != m_aNodePtr->doc) {
            DOMException e;
            e.Code = DOMExceptionType_WRONG_DOCUMENT_ERR;
            throw e;
        }
        // same node
        if (cur == m_aNodePtr) {
            DOMException e;
            e.Code = DOMExceptionType_HIERARCHY_REQUEST_ERR;
            throw e;
        }
        if (cur->parent != nullptr) {
            DOMException e;
            e.Code = DOMExceptionType_HIERARCHY_REQUEST_ERR;
            throw e;
        }
        if (!IsChildTypeAllowed(pNewChild->m_aNodeType)) {
            DOMException e;
            e.Code = DOMExceptionType_HIERARCHY_REQUEST_ERR;
            throw e;
        }

        // check whether this is an attribute node; it needs special handling
        xmlNodePtr res = nullptr;
        if (cur->type == XML_ATTRIBUTE_NODE)
        {
            xmlChar const*const pChildren((cur->children)
                    ? cur->children->content
                    : reinterpret_cast<xmlChar const*>(""));
            CAttr *const pCAttr(dynamic_cast<CAttr *>(pNewChild));
            if (!pCAttr) { throw RuntimeException(); }
            xmlNsPtr const pNs( pCAttr->GetNamespace(m_aNodePtr) );
            if (pNs) {
                res = reinterpret_cast<xmlNodePtr>(
                        xmlNewNsProp(m_aNodePtr, pNs, cur->name, pChildren));
            } else {
                res = reinterpret_cast<xmlNodePtr>(
                        xmlNewProp(m_aNodePtr, cur->name, pChildren));
            }
        }
        else
        {
            res = xmlAddChild(m_aNodePtr, cur);

            // libxml can do optimization when appending nodes.
            // if res != cur, something was optimized and the newchild-wrapper
            // should be updated
            if (res && (cur != res)) {
                pNewChild->invalidate(); // cur has been freed
            }
        }

        if (!res) { return nullptr; }

        // use custom ns cleanup instead of
        // xmlReconciliateNs(m_aNodePtr->doc, m_aNodePtr);
        // because that will not remove unneeded ns decls
        nscleanup(res, m_aNodePtr);

        ::rtl::Reference<CNode> const pNode = GetOwnerDocument().GetCNode(res);

        if (!pNode.is()) { return nullptr; }

        // dispatch DOMNodeInserted event, target is the new node
        // this node is the related node
        // does bubble
        pNode->m_bUnlinked = false; // will be deleted by xmlFreeDoc
        Reference< XDocumentEvent > docevent(getOwnerDocument(), UNO_QUERY);
        Reference< XMutationEvent > event(docevent->createEvent(
            "DOMNodeInserted"), UNO_QUERY);
        event->initMutationEvent("DOMNodeInserted", true, false, this,
            OUString(), OUString(), OUString(), AttrChangeType(0) );

        // the following dispatch functions use only UNO interfaces
        // and call event listeners, so release mutex to prevent deadlocks.
        guard.clear();

        dispatchEvent(event);
        // dispatch subtree modified for this node
        dispatchSubtreeModified();

        return pNode.get();
    }

    /**
    Returns a duplicate of this node, i.e., serves as a generic copy
    constructor for nodes.
    */
    Reference< XNode > SAL_CALL CNode::cloneNode(sal_Bool bDeep)
    {
        ::osl::MutexGuard const g(m_rMutex);

        if (nullptr == m_aNodePtr) {
            return nullptr;
        }
        ::rtl::Reference<CNode> const pNode = GetOwnerDocument().GetCNode(
            xmlCopyNode(m_aNodePtr, bDeep ? 1 : 0));
        if (!pNode.is()) { return nullptr; }
        pNode->m_bUnlinked = true; // not linked yet
        return pNode.get();
    }

    /**
    A NamedNodeMap containing the attributes of this node (if it is an Element)
    or null otherwise.
    */
    Reference< XNamedNodeMap > SAL_CALL CNode::getAttributes()
    {
        // return empty reference; only element node may override this impl
        return Reference< XNamedNodeMap>();
    }

    /**
    A NodeList that contains all children of this node.
    */
    Reference< XNodeList > SAL_CALL CNode::getChildNodes()
    {
        ::osl::MutexGuard const g(m_rMutex);

        if (nullptr == m_aNodePtr) {
            return nullptr;
        }
        Reference< XNodeList > const xNodeList(new CChildList(this, m_rMutex));
        return xNodeList;
    }

    /**
    The first child of this node.
    */
    Reference< XNode > SAL_CALL CNode::getFirstChild()
    {
        ::osl::MutexGuard const g(m_rMutex);

        if (nullptr == m_aNodePtr) {
            return nullptr;
        }
        Reference< XNode > const xNode(
                GetOwnerDocument().GetCNode(m_aNodePtr->children).get());
        return xNode;
    }

    /**
    The last child of this node.
    */
    Reference< XNode > SAL_CALL CNode::getLastChild()
    {
        ::osl::MutexGuard const g(m_rMutex);

        if (nullptr == m_aNodePtr) {
            return nullptr;
        }
        Reference< XNode > const xNode(
            GetOwnerDocument().GetCNode(xmlGetLastChild(m_aNodePtr)).get());
        return xNode;
    }

    /**
    Returns the local part of the qualified name of this node.
    */
    OUString SAL_CALL CNode::getLocalName()
    {
        // see CElement/CAttr
        return OUString();
    }


    /**
    The namespace URI of this node, or null if it is unspecified.
    */
    OUString SAL_CALL CNode::getNamespaceURI()
    {
        ::osl::MutexGuard const g(m_rMutex);

        OUString aURI;
        if (m_aNodePtr != nullptr &&
            (m_aNodePtr->type == XML_ELEMENT_NODE || m_aNodePtr->type == XML_ATTRIBUTE_NODE) &&
            m_aNodePtr->ns != nullptr)
        {
            const xmlChar* pHref = m_aNodePtr->ns->href;
            aURI = OUString(reinterpret_cast<char const *>(pHref), strlen(reinterpret_cast<char const *>(pHref)), RTL_TEXTENCODING_UTF8);
        }
        return aURI;
    }

    /**
    The node immediately following this node.
    */
    Reference< XNode > SAL_CALL CNode::getNextSibling()
    {
        ::osl::MutexGuard const g(m_rMutex);

        if (nullptr == m_aNodePtr) {
            return nullptr;
        }
        Reference< XNode > const xNode(
                GetOwnerDocument().GetCNode(m_aNodePtr->next).get());
        return xNode;
    }

    /**
    The name of this node, depending on its type; see the table above.
    */
    OUString SAL_CALL CNode::getNodeName()
    {
        /*
        Interface        nodeName               nodeValue                       attributes
        --------------------------------------------------------------------------------------
        Attr             name of attribute      value of attribute              null
        CDATASection     "#cdata-section"       content of the CDATA Section    null
        Comment          "#comment"             content of the comment          null
        Document         "#document"            null                            null
        DocumentFragment "#document-fragment"   null                            null
        DocumentType     document type name     null                            null
        Element          tag name               null                            NamedNodeMap
        Entity           entity name            null                            null
        EntityReference  name of entity         null                            null
                         referenced
        Notation         notation name          null                            null
        Processing\      target                 entire content excluding        null
        Instruction                             the target
        Text             "#text"                content of the text node        null
        */
        return OUString();
    }

    /**
    A code representing the type of the underlying object, as defined above.
    */
    NodeType SAL_CALL CNode::getNodeType()
    {
        ::osl::MutexGuard const g(m_rMutex);

        return m_aNodeType;
    }

    /**
    The value of this node, depending on its type; see the table above.
    */
    OUString SAL_CALL CNode::getNodeValue()
    {
        return OUString();
    }

    /**
    The Document object associated with this node.
    */
    Reference< XDocument > SAL_CALL CNode::getOwnerDocument()
    {
        ::osl::MutexGuard const g(m_rMutex);

        if (nullptr == m_aNodePtr) {
            return nullptr;
        }
        Reference< XDocument > const xDoc(& GetOwnerDocument());
        return xDoc;
    }

    /**
    The parent of this node.
    */
    Reference< XNode > SAL_CALL CNode::getParentNode()
    {
        ::osl::MutexGuard const g(m_rMutex);

        if (nullptr == m_aNodePtr) {
            return nullptr;
        }
        Reference< XNode > const xNode(
                GetOwnerDocument().GetCNode(m_aNodePtr->parent).get());
        return xNode;
    }

    /**
    The namespace prefix of this node, or null if it is unspecified.
    */
    OUString SAL_CALL CNode::getPrefix()
    {
        ::osl::MutexGuard const g(m_rMutex);

        OUString aPrefix;
        if (m_aNodePtr != nullptr &&
            (m_aNodePtr->type == XML_ELEMENT_NODE || m_aNodePtr->type == XML_ATTRIBUTE_NODE) &&
            m_aNodePtr->ns != nullptr)
        {
            const xmlChar* pPrefix = m_aNodePtr->ns->prefix;
            if( pPrefix != nullptr )
                aPrefix = OUString(reinterpret_cast<char const *>(pPrefix), strlen(reinterpret_cast<char const *>(pPrefix)), RTL_TEXTENCODING_UTF8);
        }
        return aPrefix;

    }

    /**
    The node immediately preceding this node.
    */
    Reference< XNode > SAL_CALL CNode::getPreviousSibling()
    {
        ::osl::MutexGuard const g(m_rMutex);

        if (nullptr == m_aNodePtr) {
            return nullptr;
        }
        Reference< XNode > const xNode(
                GetOwnerDocument().GetCNode(m_aNodePtr->prev).get());
        return xNode;
    }

    /**
    Returns whether this node (if it is an element) has any attributes.
    */
    sal_Bool SAL_CALL CNode::hasAttributes()
    {
        ::osl::MutexGuard const g(m_rMutex);

        return (m_aNodePtr != nullptr && m_aNodePtr->properties != nullptr);
    }

    /**
    Returns whether this node has any children.
    */
    sal_Bool SAL_CALL CNode::hasChildNodes()
    {
        ::osl::MutexGuard const g(m_rMutex);

        return (m_aNodePtr != nullptr && m_aNodePtr->children != nullptr);
    }

    /**
    Inserts the node newChild before the existing child node refChild.
    */
    Reference< XNode > SAL_CALL CNode::insertBefore(
            const Reference< XNode >& newChild, const Reference< XNode >& refChild)
    {
        if (!newChild.is() || !refChild.is()) { throw RuntimeException(); }

        if (newChild->getOwnerDocument() != getOwnerDocument()) {
            DOMException e;
            e.Code = DOMExceptionType_WRONG_DOCUMENT_ERR;
            throw e;
        }
        if (refChild->getParentNode() != Reference< XNode >(this)) {
            DOMException e;
            e.Code = DOMExceptionType_HIERARCHY_REQUEST_ERR;
            throw e;
        }

        ::osl::ClearableMutexGuard guard(m_rMutex);

        CNode *const pNewNode(CNode::GetImplementation(newChild));
        CNode *const pRefNode(CNode::GetImplementation(refChild));
        if (!pNewNode || !pRefNode) { throw RuntimeException(); }
        xmlNodePtr const pNewChild(pNewNode->GetNodePtr());
        xmlNodePtr const pRefChild(pRefNode->GetNodePtr());
        if (!pNewChild || !pRefChild) { throw RuntimeException(); }

        if (pNewChild == m_aNodePtr) {
            DOMException e;
            e.Code = DOMExceptionType_HIERARCHY_REQUEST_ERR;
            throw e;
        }
        // already has parent
        if (pNewChild->parent != nullptr)
        {
            DOMException e;
            e.Code = DOMExceptionType_HIERARCHY_REQUEST_ERR;
            throw e;
        }
        if (!IsChildTypeAllowed(pNewNode->m_aNodeType)) {
            DOMException e;
            e.Code = DOMExceptionType_HIERARCHY_REQUEST_ERR;
            throw e;
        }

        // attributes are unordered anyway, so just do appendChild
        if (XML_ATTRIBUTE_NODE == pNewChild->type) {
            guard.clear();
            return appendChild(newChild);
        }

        xmlNodePtr cur = m_aNodePtr->children;

        //search child before which to insert
        while (cur != nullptr)
        {
            if (cur == pRefChild) {
                // insert before
                pNewChild->next = cur;
                pNewChild->prev = cur->prev;
                cur->prev = pNewChild;
                if (pNewChild->prev != nullptr) {
                    pNewChild->prev->next = pNewChild;
                }
                pNewChild->parent = cur->parent;
                if (pNewChild->parent->children == cur) {
                    pNewChild->parent->children = pNewChild;
                }
                // do not update parent->last here!
                pNewNode->m_bUnlinked = false; // will be deleted by xmlFreeDoc
                break;
            }
            cur = cur->next;
        }
        return refChild;
    }

    /**
    Tests whether the DOM implementation implements a specific feature and
    that feature is supported by this node.
    */
  sal_Bool SAL_CALL CNode::isSupported(const OUString& /*feature*/, const OUString& /*ver*/)
    {
        OSL_ENSURE(false, "CNode::isSupported: not implemented (#i113683#)");
        return false;
    }

    /**
    Puts all Text nodes in the full depth of the sub-tree underneath this
    Node, including attribute nodes, into a "normal" form where only structure
    (e.g., elements, comments, processing instructions, CDATA sections, and
    entity references) separates Text nodes, i.e., there are neither adjacent
    Text nodes nor empty Text nodes.
    */
    void SAL_CALL CNode::normalize()
    {
        //XXX combine adjacent text nodes and remove empty ones
        OSL_ENSURE(false, "CNode::normalize: not implemented (#i113683#)");
    }

    /**
    Removes the child node indicated by oldChild from the list of children,
    and returns it.
    */
    Reference< XNode > SAL_CALL
    CNode::removeChild(const Reference< XNode >& xOldChild)
    {
        if (!xOldChild.is()) {
            throw RuntimeException();
        }

        if (xOldChild->getOwnerDocument() != getOwnerDocument()) {
            DOMException e;
            e.Code = DOMExceptionType_WRONG_DOCUMENT_ERR;
            throw e;
        }
        if (xOldChild->getParentNode() != Reference< XNode >(this)) {
            DOMException e;
            e.Code = DOMExceptionType_HIERARCHY_REQUEST_ERR;
            throw e;
        }

        ::osl::ClearableMutexGuard guard(m_rMutex);

        if (!m_aNodePtr) { throw RuntimeException(); }

        Reference<XNode> xReturn( xOldChild );

        ::rtl::Reference<CNode> const pOld(CNode::GetImplementation(xOldChild));
        if (!pOld.is()) { throw RuntimeException(); }
        xmlNodePtr const old = pOld->GetNodePtr();
        if (!old) { throw RuntimeException(); }

        if( old->type == XML_ATTRIBUTE_NODE )
        {
            xmlAttrPtr pAttr = reinterpret_cast<xmlAttrPtr>(old);
            xmlRemoveProp( pAttr );
            pOld->invalidate(); // freed by xmlRemoveProp
            xReturn.clear();
        }
        else
        {
            xmlUnlinkNode(old);
            pOld->m_bUnlinked = true;
        }

        /*DOMNodeRemoved
         * Fired when a node is being removed from its parent node.
         * This event is dispatched before the node is removed from the tree.
         * The target of this event is the node being removed.
         *   Bubbles: Yes
         *   Cancelable: No
         *   Context Info: relatedNode holds the parent node
         */
        Reference< XDocumentEvent > docevent(getOwnerDocument(), UNO_QUERY);
        Reference< XMutationEvent > event(docevent->createEvent(
            "DOMNodeRemoved"), UNO_QUERY);
        event->initMutationEvent("DOMNodeRemoved",
            true,
            false,
            this,
            OUString(), OUString(), OUString(), AttrChangeType(0) );

        // the following dispatch functions use only UNO interfaces
        // and call event listeners, so release mutex to prevent deadlocks.
        guard.clear();

        dispatchEvent(event);
        // subtree modified for this node
        dispatchSubtreeModified();

        return xReturn;
    }

    /**
    Replaces the child node oldChild with newChild in the list of children,
    and returns the oldChild node.
    */
    Reference< XNode > SAL_CALL CNode::replaceChild(
            Reference< XNode > const& xNewChild,
            Reference< XNode > const& xOldChild)
    {
        if (!xOldChild.is() || !xNewChild.is()) {
            throw RuntimeException();
        }

        if (xNewChild->getOwnerDocument() != getOwnerDocument()) {
            DOMException e;
            e.Code = DOMExceptionType_WRONG_DOCUMENT_ERR;
            throw e;
        }
        if (xOldChild->getParentNode() != Reference< XNode >(this)) {
            DOMException e;
            e.Code = DOMExceptionType_HIERARCHY_REQUEST_ERR;
            throw e;
        }

        ::osl::ClearableMutexGuard guard(m_rMutex);

        ::rtl::Reference<CNode> const pOldNode(
                CNode::GetImplementation(xOldChild));
        ::rtl::Reference<CNode> const pNewNode(
                CNode::GetImplementation(xNewChild));
        if (!pOldNode.is() || !pNewNode.is()) { throw RuntimeException(); }
        xmlNodePtr const pOld = pOldNode->GetNodePtr();
        xmlNodePtr const pNew = pNewNode->GetNodePtr();
        if (!pOld || !pNew) { throw RuntimeException(); }

        if (pNew == m_aNodePtr) {
            DOMException e;
            e.Code = DOMExceptionType_HIERARCHY_REQUEST_ERR;
            throw e;
        }
        // already has parent
        if (pNew->parent != nullptr) {
            DOMException e;
            e.Code = DOMExceptionType_HIERARCHY_REQUEST_ERR;
            throw e;
        }
        if (!IsChildTypeAllowed(pNewNode->m_aNodeType)) {
            DOMException e;
            e.Code = DOMExceptionType_HIERARCHY_REQUEST_ERR;
            throw e;
        }

        if( pOld->type == XML_ATTRIBUTE_NODE )
        {
            // can only replace attribute with attribute
            if ( pOld->type != pNew->type )
            {
                DOMException e;
                e.Code = DOMExceptionType_HIERARCHY_REQUEST_ERR;
                throw e;
            }

            xmlAttrPtr pAttr = reinterpret_cast<xmlAttrPtr>(pOld);
            xmlRemoveProp( pAttr );
            pOldNode->invalidate(); // freed by xmlRemoveProp
            appendChild(xNewChild);
        }
        else
        {

        xmlNodePtr cur = m_aNodePtr->children;
        //find old node in child list
        while (cur != nullptr)
        {
            if(cur == pOld)
            {
                // exchange nodes
                pNew->prev = pOld->prev;
                if (pNew->prev != nullptr)
                    pNew->prev->next = pNew;
                pNew->next = pOld->next;
                if (pNew->next != nullptr)
                    pNew->next->prev = pNew;
                pNew->parent = pOld->parent;
                assert(pNew->parent && "coverity[var_deref_op] pNew->parent cannot be NULL here");
                if(pNew->parent->children == pOld)
                    pNew->parent->children = pNew;
                if(pNew->parent->last == pOld)
                    pNew->parent->last = pNew;
                pOld->next = nullptr;
                pOld->prev = nullptr;
                pOld->parent = nullptr;
                pOldNode->m_bUnlinked = true;
                pNewNode->m_bUnlinked = false; // will be deleted by xmlFreeDoc
            }
            cur = cur->next;
        }
        }

        guard.clear(); // release for calling event handlers
        dispatchSubtreeModified();

        return xOldChild;
    }

    void CNode::dispatchSubtreeModified()
    {
        // only uses UNO interfaces => needs no mutex

        // dispatch DOMSubtreeModified
        // target is _this_ node
        Reference< XDocumentEvent > docevent(getOwnerDocument(), UNO_QUERY);
        Reference< XMutationEvent > event(docevent->createEvent(
            "DOMSubtreeModified"), UNO_QUERY);
        event->initMutationEvent(
            "DOMSubtreeModified", true,
            false, Reference< XNode >(),
            OUString(), OUString(), OUString(), AttrChangeType(0) );
        dispatchEvent(event);
    }

    /**
    The value of this node, depending on its type; see the table above.
    */
    void SAL_CALL CNode::setNodeValue(const OUString& /*nodeValue*/)
    {
        // use specific node implementation
        // if we end up down here, something went wrong
        DOMException e;
        e.Code = DOMExceptionType_NO_MODIFICATION_ALLOWED_ERR;
        throw e;
    }

    /**
    The namespace prefix of this node, or null if it is unspecified.
    */
    void SAL_CALL CNode::setPrefix(const OUString& prefix)
    {
        ::osl::MutexGuard const g(m_rMutex);

        if ((nullptr == m_aNodePtr) ||
            ((m_aNodePtr->type != XML_ELEMENT_NODE) &&
             (m_aNodePtr->type != XML_ATTRIBUTE_NODE)))
        {
            DOMException e;
            e.Code = DOMExceptionType_NO_MODIFICATION_ALLOWED_ERR;
            throw e;
        }
        OString o1 = OUStringToOString(prefix, RTL_TEXTENCODING_UTF8);
        xmlChar const *pBuf = reinterpret_cast<xmlChar const *>(o1.getStr());
        if (m_aNodePtr != nullptr && m_aNodePtr->ns != nullptr)
        {
            xmlFree(const_cast<xmlChar *>(m_aNodePtr->ns->prefix));
            m_aNodePtr->ns->prefix = xmlStrdup(pBuf);
        }

    }

        // --- XEventTarget
    void SAL_CALL CNode::addEventListener(const OUString& eventType,
        const Reference< css::xml::dom::events::XEventListener >& listener,
        sal_Bool useCapture)
    {
        ::osl::MutexGuard const g(m_rMutex);

        CDocument & rDocument(GetOwnerDocument());
        events::CEventDispatcher & rDispatcher(rDocument.GetEventDispatcher());
        rDispatcher.addListener(m_aNodePtr, eventType, listener, useCapture);
    }

    void SAL_CALL CNode::removeEventListener(const OUString& eventType,
        const Reference< css::xml::dom::events::XEventListener >& listener,
        sal_Bool useCapture)
    {
        ::osl::MutexGuard const g(m_rMutex);

        CDocument & rDocument(GetOwnerDocument());
        events::CEventDispatcher & rDispatcher(rDocument.GetEventDispatcher());
        rDispatcher.removeListener(m_aNodePtr, eventType, listener, useCapture);
    }

    sal_Bool SAL_CALL CNode::dispatchEvent(const Reference< XEvent >& evt)
    {
        CDocument * pDocument;
        events::CEventDispatcher * pDispatcher;
        xmlNodePtr pNode;
        {
            ::osl::MutexGuard const g(m_rMutex);

            pDocument = & GetOwnerDocument();
            pDispatcher = & pDocument->GetEventDispatcher();
            pNode = m_aNodePtr;
        }
        // this calls event listeners, do not call with locked mutex
        pDispatcher->dispatchEvent(*pDocument, m_rMutex, pNode, this, evt);
        return true;
    }

    ::sal_Int64 SAL_CALL
    CNode::getSomething(Sequence< ::sal_Int8 > const& rId)
    {
        if ((rId.getLength() == 16) &&
            (0 == memcmp(theCNodeUnoTunnelId::get().getSeq().getConstArray(),
                                    rId.getConstArray(), 16)))
        {
            return ::sal::static_int_cast< sal_Int64 >(
                    reinterpret_cast< sal_IntPtr >(this) );
        }
        return 0;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
