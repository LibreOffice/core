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

#include <com/sun/star/uno/Sequence.h>

#include "document.hxx"
#include "attr.hxx"
#include "element.hxx"
#include "cdatasection.hxx"
#include "documentfragment.hxx"
#include "text.hxx"
#include "comment.hxx"
#include "processinginstruction.hxx"
#include "entityreference.hxx"
#include "documenttype.hxx"
#include "elementlist.hxx"
#include "domimplementation.hxx"
#include "entity.hxx"
#include "notation.hxx"

#include <event.hxx>
#include <mutationevent.hxx>
#include <uievent.hxx>
#include <mouseevent.hxx>
#include <eventdispatcher.hxx>

#include <string.h>

#include <osl/diagnose.h>

#include <com/sun/star/xml/sax/FastToken.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>

using namespace css;
using namespace css::io;
using namespace css::uno;
using namespace css::xml::dom;
using namespace css::xml::dom::events;
using namespace css::xml::sax;

namespace DOM
{
    static xmlNodePtr lcl_getDocumentType(xmlDocPtr const i_pDocument)
    {
        // find the doc type
        xmlNodePtr cur = i_pDocument->children;
        while (cur != nullptr)
        {
            if ((cur->type == XML_DOCUMENT_TYPE_NODE) ||
                (cur->type == XML_DTD_NODE)) {
                    return cur;
            }
        }
        return nullptr;
    }

    /// get the pointer to the root element node of the document
    static xmlNodePtr lcl_getDocumentRootPtr(xmlDocPtr const i_pDocument)
    {
        // find the document element
        xmlNodePtr cur = i_pDocument->children;
        while (cur != nullptr)
        {
            if (cur->type == XML_ELEMENT_NODE)
                break;
            cur = cur->next;
        }
        return cur;
    }

    CDocument::CDocument(xmlDocPtr const pDoc)
        : CDocument_Base(*this, m_Mutex,
                NodeType_DOCUMENT_NODE, reinterpret_cast<xmlNodePtr>(pDoc))
        , m_aDocPtr(pDoc)
        , m_streamListeners()
        , m_pEventDispatcher(new events::CEventDispatcher)
    {
    }

    ::rtl::Reference<CDocument> CDocument::CreateCDocument(xmlDocPtr const pDoc)
    {
        ::rtl::Reference<CDocument> const xDoc(new CDocument(pDoc));
        // add the doc itself to its nodemap!
        xDoc->m_NodeMap.emplace(
                reinterpret_cast<xmlNodePtr>(pDoc),
                ::std::make_pair(
                    WeakReference<XNode>(static_cast<XDocument*>(xDoc.get())),
                    xDoc.get()));
        return xDoc;
    }

    CDocument::~CDocument()
    {
        ::osl::MutexGuard const g(m_Mutex);
#ifdef DBG_UTIL
        // node map must be empty now, otherwise CDocument must not die!
        for (const auto& rEntry : m_NodeMap)
        {
            Reference<XNode> const xNode(rEntry.second.first);
            OSL_ENSURE(!xNode.is(),
            "CDocument::~CDocument(): ERROR: live node in document node map!");
        }
#endif
        xmlFreeDoc(m_aDocPtr);
    }


    events::CEventDispatcher & CDocument::GetEventDispatcher()
    {
        return *m_pEventDispatcher;
    }

    ::rtl::Reference< CElement > CDocument::GetDocumentElement()
    {
        xmlNodePtr const pNode = lcl_getDocumentRootPtr(m_aDocPtr);
        ::rtl::Reference< CElement > const xRet(
            dynamic_cast<CElement*>(GetCNode(pNode).get()));
        return xRet;
    }

    void
    CDocument::RemoveCNode(xmlNodePtr const pNode, CNode const*const pCNode)
    {
        nodemap_t::iterator const i = m_NodeMap.find(pNode);
        if (i != m_NodeMap.end()) {
            // #i113681# consider this scenario:
            // T1 calls ~CNode
            // T2 calls getCNode:    lookup will find i->second->first invalid
            //                       so a new CNode is created and inserted
            // T1 calls removeCNode: i->second->second now points to a
            //                       different CNode instance!

            // check that the CNode is the right one
            CNode *const pCurrent = i->second.second;
            if (pCurrent == pCNode) {
                m_NodeMap.erase(i);
            }
        }
    }

    /** NB: this is the CNode factory.
        it is the only place where CNodes may be instantiated.
        all CNodes must be registered at the m_NodeMap.
     */
    ::rtl::Reference<CNode>
    CDocument::GetCNode(xmlNodePtr const pNode, bool const bCreate)
    {
        if (nullptr == pNode) {
            return nullptr;
        }
        //check whether there is already an instance for this node
        nodemap_t::const_iterator const i = m_NodeMap.find(pNode);
        if (i != m_NodeMap.end()) {
            // #i113681# check that the CNode is still alive
            uno::Reference<XNode> const xNode(i->second.first);
            if (xNode.is())
            {
                ::rtl::Reference<CNode> ret(i->second.second);
                OSL_ASSERT(ret.is());
                return ret;
            }
        }

        if (!bCreate) { return nullptr; }

        // there is not yet an instance wrapping this node,
        // create it and store it in the map

        ::rtl::Reference<CNode> pCNode;
        switch (pNode->type)
        {
            case XML_ELEMENT_NODE:
                // m_aNodeType = NodeType::ELEMENT_NODE;
                pCNode = new CElement(*this, m_Mutex, pNode);
            break;
            case XML_TEXT_NODE:
                // m_aNodeType = NodeType::TEXT_NODE;
                pCNode = new CText(*this, m_Mutex, pNode);
            break;
            case XML_CDATA_SECTION_NODE:
                // m_aNodeType = NodeType::CDATA_SECTION_NODE;
                pCNode = new CCDATASection(*this, m_Mutex, pNode);
            break;
            case XML_ENTITY_REF_NODE:
                // m_aNodeType = NodeType::ENTITY_REFERENCE_NODE;
                pCNode = new CEntityReference(*this, m_Mutex, pNode);
            break;
            case XML_ENTITY_NODE:
                // m_aNodeType = NodeType::ENTITY_NODE;
                pCNode = new CEntity(*this, m_Mutex, reinterpret_cast<xmlEntityPtr>(pNode));
            break;
            case XML_PI_NODE:
                // m_aNodeType = NodeType::PROCESSING_INSTRUCTION_NODE;
                pCNode = new CProcessingInstruction(*this, m_Mutex, pNode);
            break;
            case XML_COMMENT_NODE:
                // m_aNodeType = NodeType::COMMENT_NODE;
                pCNode = new CComment(*this, m_Mutex, pNode);
            break;
            case XML_DOCUMENT_NODE:
                // m_aNodeType = NodeType::DOCUMENT_NODE;
                OSL_ENSURE(false, "CDocument::GetCNode is not supposed to"
                        " create a CDocument!!!");
                pCNode = new CDocument(reinterpret_cast<xmlDocPtr>(pNode));
            break;
            case XML_DOCUMENT_TYPE_NODE:
            case XML_DTD_NODE:
                // m_aNodeType = NodeType::DOCUMENT_TYPE_NODE;
                pCNode = new CDocumentType(*this, m_Mutex, reinterpret_cast<xmlDtdPtr>(pNode));
            break;
            case XML_DOCUMENT_FRAG_NODE:
                // m_aNodeType = NodeType::DOCUMENT_FRAGMENT_NODE;
                pCNode = new CDocumentFragment(*this, m_Mutex, pNode);
            break;
            case XML_NOTATION_NODE:
                // m_aNodeType = NodeType::NOTATION_NODE;
                pCNode = new CNotation(*this, m_Mutex, reinterpret_cast<xmlNotationPtr>(pNode));
            break;
            case XML_ATTRIBUTE_NODE:
                // m_aNodeType = NodeType::ATTRIBUTE_NODE;
                pCNode = new CAttr(*this, m_Mutex, reinterpret_cast<xmlAttrPtr>(pNode));
            break;
            // unsupported node types
            case XML_HTML_DOCUMENT_NODE:
            case XML_ELEMENT_DECL:
            case XML_ATTRIBUTE_DECL:
            case XML_ENTITY_DECL:
            case XML_NAMESPACE_DECL:
            default:
            break;
        }

        if (pCNode != nullptr) {
            bool const bInserted = m_NodeMap.emplace(
                        pNode,
                        ::std::make_pair(WeakReference<XNode>(pCNode.get()), pCNode.get())
                ).second;
            OSL_ASSERT(bInserted);
            if (!bInserted) {
                // if insertion failed, delete new instance and return null
                return nullptr;
            }
        }

        OSL_ENSURE(pCNode.is(), "no node produced during CDocument::GetCNode!");
        return pCNode;
    }


    CDocument & CDocument::GetOwnerDocument()
    {
        return *this;
    }

    void CDocument::saxify(const Reference< XDocumentHandler >& i_xHandler)
    {
        i_xHandler->startDocument();
        for (xmlNodePtr pChild = m_aNodePtr->children;
                        pChild != nullptr; pChild = pChild->next) {
            ::rtl::Reference<CNode> const pNode = GetCNode(pChild);
            OSL_ENSURE(pNode != nullptr, "CNode::get returned 0");
            pNode->saxify(i_xHandler);
        }
        i_xHandler->endDocument();
    }

    void CDocument::fastSaxify( Context& rContext )
    {
        rContext.mxDocHandler->startDocument();
        for (xmlNodePtr pChild = m_aNodePtr->children;
                        pChild != nullptr; pChild = pChild->next) {
            ::rtl::Reference<CNode> const pNode = GetCNode(pChild);
            OSL_ENSURE(pNode != nullptr, "CNode::get returned 0");
            pNode->fastSaxify(rContext);
        }
        rContext.mxDocHandler->endDocument();
    }

    bool CDocument::IsChildTypeAllowed(NodeType const nodeType)
    {
        switch (nodeType) {
            case NodeType_PROCESSING_INSTRUCTION_NODE:
            case NodeType_COMMENT_NODE:
                return true;
            case NodeType_ELEMENT_NODE:
                 // there may be only one!
                return nullptr == lcl_getDocumentRootPtr(m_aDocPtr);
            case NodeType_DOCUMENT_TYPE_NODE:
                 // there may be only one!
                return nullptr == lcl_getDocumentType(m_aDocPtr);
            default:
                return false;
        }
    }


    void SAL_CALL CDocument::addListener(const Reference< XStreamListener >& aListener )
    {
        ::osl::MutexGuard const g(m_Mutex);

        m_streamListeners.insert(aListener);
    }

    void SAL_CALL CDocument::removeListener(const Reference< XStreamListener >& aListener )
    {
        ::osl::MutexGuard const g(m_Mutex);

        m_streamListeners.erase(aListener);
    }

    // IO context functions for libxml2 interaction
    typedef struct {
        Reference< XOutputStream > stream;
        bool const allowClose;
    } IOContext;

    extern "C" {
    // write callback
    // int xmlOutputWriteCallback (void * context, const char * buffer, int len)
    static int writeCallback(void *context, const char* buffer, int len){
        // create a sequence and write it to the stream
        IOContext *pContext = static_cast<IOContext*>(context);
        Sequence<sal_Int8> bs(reinterpret_cast<const sal_Int8*>(buffer), len);
        pContext->stream->writeBytes(bs);
        return len;
    }

    // close callback
    //int xmlOutputCloseCallback (void * context)
    static int closeCallback(void *context)
    {
        IOContext *pContext = static_cast<IOContext*>(context);
        if (pContext->allowClose) {
            pContext->stream->closeOutput();
        }
        return 0;
    }
    } // extern "C"

    void SAL_CALL CDocument::start()
    {
        listenerlist_t streamListeners;
        {
            ::osl::MutexGuard const g(m_Mutex);

            if (! m_rOutputStream.is()) { throw RuntimeException(); }
            streamListeners = m_streamListeners;
        }

        // notify listeners about start
        for (const Reference< XStreamListener >& aListener : streamListeners) {
            aListener->started();
        }

        {
            ::osl::MutexGuard const g(m_Mutex);

            // check again! could have been reset...
            if (! m_rOutputStream.is()) { throw RuntimeException(); }

            // setup libxml IO and write data to output stream
            IOContext ioctx = {m_rOutputStream, false};
            xmlOutputBufferPtr pOut = xmlOutputBufferCreateIO(
                writeCallback, closeCallback, &ioctx, nullptr);
            xmlSaveFileTo(pOut, m_aNodePtr->doc, nullptr);
        }

        // call listeners
        for (const Reference< XStreamListener >& aListener : streamListeners) {
            aListener->closed();
        }
    }

    void SAL_CALL CDocument::terminate()
    {
        // not supported
    }

    void SAL_CALL CDocument::setOutputStream( const Reference< XOutputStream >& aStream )
    {
        ::osl::MutexGuard const g(m_Mutex);

        m_rOutputStream = aStream;
    }

    Reference< XOutputStream > SAL_CALL  CDocument::getOutputStream()
    {
        ::osl::MutexGuard const g(m_Mutex);

        return m_rOutputStream;
    }

    // Creates an Attr of the given name.
    Reference< XAttr > SAL_CALL CDocument::createAttribute(const OUString& name)
    {
        ::osl::MutexGuard const g(m_Mutex);

        OString o1 = OUStringToOString(name, RTL_TEXTENCODING_UTF8);
        xmlChar const *pName = reinterpret_cast<xmlChar const *>(o1.getStr());
        xmlAttrPtr const pAttr = xmlNewDocProp(m_aDocPtr, pName, nullptr);
        ::rtl::Reference< CAttr > const pCAttr(
            dynamic_cast< CAttr* >(GetCNode(
                    reinterpret_cast<xmlNodePtr>(pAttr)).get()));
        if (!pCAttr.is()) { throw RuntimeException(); }
        pCAttr->m_bUnlinked = true;
        return pCAttr.get();
    };

    // Creates an attribute of the given qualified name and namespace URI.
    Reference< XAttr > SAL_CALL CDocument::createAttributeNS(
            const OUString& ns, const OUString& qname)
    {
        ::osl::MutexGuard const g(m_Mutex);

        // libxml does not allow a NS definition to be attached to an
        // attribute node - which is a good thing, since namespaces are
        // only defined as parts of element nodes
        // thus the namespace data is stored in CAttr::m_pNamespace
        sal_Int32 i = qname.indexOf(':');
        OString oPrefix, oName, oUri;
        if (i != -1)
        {
            oPrefix = OUStringToOString(qname.copy(0, i), RTL_TEXTENCODING_UTF8);
            oName = OUStringToOString(qname.copy(i+1), RTL_TEXTENCODING_UTF8);
        }
        else
        {
            oName = OUStringToOString(qname, RTL_TEXTENCODING_UTF8);
        }
        oUri = OUStringToOString(ns, RTL_TEXTENCODING_UTF8);
        xmlAttrPtr const pAttr = xmlNewDocProp(m_aDocPtr,
                reinterpret_cast<xmlChar const*>(oName.getStr()), nullptr);
        ::rtl::Reference< CAttr > const pCAttr(
            dynamic_cast< CAttr* >(GetCNode(
                    reinterpret_cast<xmlNodePtr>(pAttr)).get()));
        if (!pCAttr.is()) { throw RuntimeException(); }
        // store the namespace data!
        pCAttr->m_pNamespace.reset( new stringpair_t(oUri, oPrefix) );
        pCAttr->m_bUnlinked = true;

        return pCAttr.get();
    };

    // Creates a CDATASection node whose value is the specified string.
    Reference< XCDATASection > SAL_CALL CDocument::createCDATASection(const OUString& data)
    {
        ::osl::MutexGuard const g(m_Mutex);

        OString const oData(
                OUStringToOString(data, RTL_TEXTENCODING_UTF8));
        xmlChar const*const pData =
            reinterpret_cast<xmlChar const*>(oData.getStr());
        xmlNodePtr const pText =
            xmlNewCDataBlock(m_aDocPtr, pData, oData.getLength());
        Reference< XCDATASection > const xRet(
            static_cast< XNode* >(GetCNode(pText).get()),
            UNO_QUERY_THROW);
        return xRet;
    }

    // Creates a Comment node given the specified string.
    Reference< XComment > SAL_CALL CDocument::createComment(const OUString& data)
    {
        ::osl::MutexGuard const g(m_Mutex);

        OString o1 = OUStringToOString(data, RTL_TEXTENCODING_UTF8);
        xmlChar const *pData = reinterpret_cast<xmlChar const *>(o1.getStr());
        xmlNodePtr pComment = xmlNewDocComment(m_aDocPtr, pData);
        Reference< XComment > const xRet(
            static_cast< XNode* >(GetCNode(pComment).get()),
            UNO_QUERY_THROW);
        return xRet;
    }

    //Creates an empty DocumentFragment object.
    Reference< XDocumentFragment > SAL_CALL CDocument::createDocumentFragment()
    {
        ::osl::MutexGuard const g(m_Mutex);

        xmlNodePtr pFrag = xmlNewDocFragment(m_aDocPtr);
        Reference< XDocumentFragment > const xRet(
            static_cast< XNode* >(GetCNode(pFrag).get()),
            UNO_QUERY_THROW);
        return xRet;
    }

    // Creates an element of the type specified.
    Reference< XElement > SAL_CALL CDocument::createElement(const OUString& tagName)
    {
        ::osl::MutexGuard const g(m_Mutex);

        OString o1 = OUStringToOString(tagName, RTL_TEXTENCODING_UTF8);
        xmlChar const *pName = reinterpret_cast<xmlChar const *>(o1.getStr());
        xmlNodePtr const pNode = xmlNewDocNode(m_aDocPtr, nullptr, pName, nullptr);
        Reference< XElement > const xRet(
            static_cast< XNode* >(GetCNode(pNode).get()),
            UNO_QUERY_THROW);
        return xRet;
    }

    // Creates an element of the given qualified name and namespace URI.
    Reference< XElement > SAL_CALL CDocument::createElementNS(
            const OUString& ns, const OUString& qname)
    {
        ::osl::MutexGuard const g(m_Mutex);

        sal_Int32 i = qname.indexOf(':');
        if (ns.isEmpty()) throw RuntimeException();
        xmlChar const *pPrefix;
        xmlChar const *pName;
        OString o1, o2, o3;
        if ( i != -1) {
            o1 = OUStringToOString(qname.copy(0, i), RTL_TEXTENCODING_UTF8);
            pPrefix = reinterpret_cast<xmlChar const *>(o1.getStr());
            o2 = OUStringToOString(qname.copy(i+1), RTL_TEXTENCODING_UTF8);
            pName = reinterpret_cast<xmlChar const *>(o2.getStr());
        } else {
            // default prefix
            pPrefix = reinterpret_cast<xmlChar const *>("");
            o2 = OUStringToOString(qname, RTL_TEXTENCODING_UTF8);
            pName = reinterpret_cast<xmlChar const *>(o2.getStr());
        }
        o3 = OUStringToOString(ns, RTL_TEXTENCODING_UTF8);
        xmlChar const *pUri = reinterpret_cast<xmlChar const *>(o3.getStr());

        // xmlNsPtr aNsPtr = xmlNewReconciledNs?
        // xmlNsPtr aNsPtr = xmlNewGlobalNs?
        xmlNodePtr const pNode = xmlNewDocNode(m_aDocPtr, nullptr, pName, nullptr);
        xmlNsPtr const pNs = xmlNewNs(pNode, pUri, pPrefix);
        xmlSetNs(pNode, pNs);
        Reference< XElement > const xRet(
            static_cast< XNode* >(GetCNode(pNode).get()),
            UNO_QUERY_THROW);
        return xRet;
    }

    //Creates an EntityReference object.
    Reference< XEntityReference > SAL_CALL CDocument::createEntityReference(const OUString& name)
    {
        ::osl::MutexGuard const g(m_Mutex);

        OString o1 = OUStringToOString(name, RTL_TEXTENCODING_UTF8);
        xmlChar const *pName = reinterpret_cast<xmlChar const *>(o1.getStr());
        xmlNodePtr const pNode = xmlNewReference(m_aDocPtr, pName);
        Reference< XEntityReference > const xRet(
            static_cast< XNode* >(GetCNode(pNode).get()),
            UNO_QUERY_THROW);
        return xRet;
    }

    // Creates a ProcessingInstruction node given the specified name and
    // data strings.
    Reference< XProcessingInstruction > SAL_CALL CDocument::createProcessingInstruction(
            const OUString& target, const OUString& data)
    {
        ::osl::MutexGuard const g(m_Mutex);

        OString o1 = OUStringToOString(target, RTL_TEXTENCODING_UTF8);
        xmlChar const *pTarget = reinterpret_cast<xmlChar const *>(o1.getStr());
        OString o2 = OUStringToOString(data, RTL_TEXTENCODING_UTF8);
        xmlChar const *pData = reinterpret_cast<xmlChar const *>(o2.getStr());
        xmlNodePtr const pNode = xmlNewDocPI(m_aDocPtr, pTarget, pData);
        pNode->doc = m_aDocPtr;
        Reference< XProcessingInstruction > const xRet(
            static_cast< XNode* >(GetCNode(pNode).get()),
            UNO_QUERY_THROW);
        return xRet;
    }

    // Creates a Text node given the specified string.
    Reference< XText > SAL_CALL CDocument::createTextNode(const OUString& data)
    {
        ::osl::MutexGuard const g(m_Mutex);

        OString o1 = OUStringToOString(data, RTL_TEXTENCODING_UTF8);
        xmlChar const *pData = reinterpret_cast<xmlChar const *>(o1.getStr());
        xmlNodePtr const pNode = xmlNewDocText(m_aDocPtr, pData);
        Reference< XText > const xRet(
            static_cast< XNode* >(GetCNode(pNode).get()),
            UNO_QUERY_THROW);
        return xRet;
    }

    // The Document Type Declaration (see DocumentType) associated with this
    // document.
    Reference< XDocumentType > SAL_CALL CDocument::getDoctype()
    {
        ::osl::MutexGuard const g(m_Mutex);

        xmlNodePtr const pDocType(lcl_getDocumentType(m_aDocPtr));
        Reference< XDocumentType > const xRet(
            static_cast< XNode* >(GetCNode(pDocType).get()),
            UNO_QUERY);
        return xRet;
    }

    // This is a convenience attribute that allows direct access to the child
    // node that is the root element of the document.
    Reference< XElement > SAL_CALL CDocument::getDocumentElement()
    {
        ::osl::MutexGuard const g(m_Mutex);

        xmlNodePtr const pNode = lcl_getDocumentRootPtr(m_aDocPtr);
        if (!pNode) { return nullptr; }
        Reference< XElement > const xRet(
            static_cast< XNode* >(GetCNode(pNode).get()),
            UNO_QUERY);
        return xRet;
    }

    static xmlNodePtr
    lcl_search_element_by_id(const xmlNodePtr cur, const xmlChar* id)
    {
        if (cur == nullptr)
            return nullptr;
        // look in current node
        if (cur->type == XML_ELEMENT_NODE)
        {
            xmlAttrPtr a = cur->properties;
            while (a != nullptr)
            {
                if (a->atype == XML_ATTRIBUTE_ID) {
                    if (strcmp(reinterpret_cast<char*>(a->children->content), reinterpret_cast<char const *>(id)) == 0)
                        return cur;
                }
                a = a->next;
            }
        }
        // look in children
        xmlNodePtr result = lcl_search_element_by_id(cur->children, id);
        if (result != nullptr)
            return result;
        result = lcl_search_element_by_id(cur->next, id);
        return result;
    }

    // Returns the Element whose ID is given by elementId.
    Reference< XElement > SAL_CALL
    CDocument::getElementById(const OUString& elementId)
    {
        ::osl::MutexGuard const g(m_Mutex);

        // search the tree for an element with the given ID
        OString o1 = OUStringToOString(elementId, RTL_TEXTENCODING_UTF8);
        xmlChar const *pId = reinterpret_cast<xmlChar const *>(o1.getStr());
        xmlNodePtr const pStart = lcl_getDocumentRootPtr(m_aDocPtr);
        if (!pStart) { return nullptr; }
        xmlNodePtr const pNode = lcl_search_element_by_id(pStart, pId);
        Reference< XElement > const xRet(
            static_cast< XNode* >(GetCNode(pNode).get()),
            UNO_QUERY);
        return xRet;
    }


    Reference< XNodeList > SAL_CALL
    CDocument::getElementsByTagName(OUString const& rTagname)
    {
        ::osl::MutexGuard const g(m_Mutex);

        Reference< XNodeList > const xRet(
            new CElementList(GetDocumentElement(), m_Mutex, rTagname));
        return xRet;
    }

    Reference< XNodeList > SAL_CALL CDocument::getElementsByTagNameNS(
            OUString const& rNamespaceURI, OUString const& rLocalName)
    {
        ::osl::MutexGuard const g(m_Mutex);

        Reference< XNodeList > const xRet(
            new CElementList(GetDocumentElement(), m_Mutex,
                rLocalName, &rNamespaceURI));
        return xRet;
    }

    Reference< XDOMImplementation > SAL_CALL CDocument::getImplementation()
    {
        // does not need mutex currently
        return Reference< XDOMImplementation >(CDOMImplementation::get());
    }

    // helper function to recursively import siblings
    static void lcl_ImportSiblings(
        Reference< XDocument > const& xTargetDocument,
        Reference< XNode > const& xTargetParent,
        Reference< XNode > const& xChild)
    {
        Reference< XNode > xSibling = xChild;
        while (xSibling.is())
        {
            Reference< XNode > const xTmp(
                    xTargetDocument->importNode(xSibling, true));
            xTargetParent->appendChild(xTmp);
            xSibling = xSibling->getNextSibling();
        }
    }

    static Reference< XNode >
    lcl_ImportNode( Reference< XDocument > const& xDocument,
            Reference< XNode > const& xImportedNode, bool deep)
    {
        Reference< XNode > xNode;
        NodeType aNodeType = xImportedNode->getNodeType();
        switch (aNodeType)
        {
        case NodeType_ATTRIBUTE_NODE:
        {
            Reference< XAttr > const xAttr(xImportedNode, UNO_QUERY_THROW);
            Reference< XAttr > const xNew =
                xDocument->createAttribute(xAttr->getName());
            xNew->setValue(xAttr->getValue());
            xNode.set(xNew, UNO_QUERY);
            break;
        }
        case NodeType_CDATA_SECTION_NODE:
        {
            Reference< XCDATASection > const xCData(xImportedNode,
                    UNO_QUERY_THROW);
            Reference< XCDATASection > const xNewCData =
                xDocument->createCDATASection(xCData->getData());
            xNode.set(xNewCData, UNO_QUERY);
            break;
        }
        case NodeType_COMMENT_NODE:
        {
            Reference< XComment > const xComment(xImportedNode,
                    UNO_QUERY_THROW);
            Reference< XComment > const xNewComment =
                xDocument->createComment(xComment->getData());
            xNode.set(xNewComment, UNO_QUERY);
            break;
        }
        case NodeType_DOCUMENT_FRAGMENT_NODE:
        {
            Reference< XDocumentFragment > const xFrag(xImportedNode,
                    UNO_QUERY_THROW);
            Reference< XDocumentFragment > const xNewFrag =
                xDocument->createDocumentFragment();
            xNode.set(xNewFrag, UNO_QUERY);
            break;
        }
        case NodeType_ELEMENT_NODE:
        {
            Reference< XElement > const xElement(xImportedNode,
                    UNO_QUERY_THROW);
            OUString const aNsUri = xImportedNode->getNamespaceURI();
            OUString const aNsPrefix = xImportedNode->getPrefix();
            OUString aQName = xElement->getTagName();
            Reference< XElement > xNewElement;
            if (!aNsUri.isEmpty())
            {
                if (!aNsPrefix.isEmpty()) {
                    aQName = aNsPrefix + ":" + aQName;
                }
                xNewElement = xDocument->createElementNS(aNsUri, aQName);
            } else {
                xNewElement = xDocument->createElement(aQName);
            }

            // get attributes
            if (xElement->hasAttributes())
            {
                Reference< XNamedNodeMap > attribs = xElement->getAttributes();
                for (sal_Int32 i = 0; i < attribs->getLength(); i++)
                {
                    Reference< XAttr > const curAttr(attribs->item(i),
                            UNO_QUERY_THROW);
                    OUString const aAttrUri = curAttr->getNamespaceURI();
                    OUString const aAttrPrefix = curAttr->getPrefix();
                    OUString aAttrName = curAttr->getName();
                    OUString const sValue = curAttr->getValue();
                    if (!aAttrUri.isEmpty())
                    {
                        if (!aAttrPrefix.isEmpty()) {
                            aAttrName = aAttrPrefix + ":" + aAttrName;
                        }
                        xNewElement->setAttributeNS(
                                aAttrUri, aAttrName, sValue);
                    } else {
                        xNewElement->setAttribute(aAttrName, sValue);
                    }
                }
            }
            xNode.set(xNewElement, UNO_QUERY);
            break;
        }
        case NodeType_ENTITY_REFERENCE_NODE:
        {
            Reference< XEntityReference > const xRef(xImportedNode,
                    UNO_QUERY_THROW);
            Reference< XEntityReference > const xNewRef(
                xDocument->createEntityReference(xRef->getNodeName()));
            xNode.set(xNewRef, UNO_QUERY);
            break;
        }
        case NodeType_PROCESSING_INSTRUCTION_NODE:
        {
            Reference< XProcessingInstruction > const xPi(xImportedNode,
                    UNO_QUERY_THROW);
            Reference< XProcessingInstruction > const xNewPi(
                xDocument->createProcessingInstruction(
                    xPi->getTarget(), xPi->getData()));
            xNode.set(xNewPi, UNO_QUERY);
            break;
        }
        case NodeType_TEXT_NODE:
        {
            Reference< XText > const xText(xImportedNode, UNO_QUERY_THROW);
            Reference< XText > const xNewText(
                xDocument->createTextNode(xText->getData()));
            xNode.set(xNewText, UNO_QUERY);
            break;
        }
        case NodeType_ENTITY_NODE:
        case NodeType_DOCUMENT_NODE:
        case NodeType_DOCUMENT_TYPE_NODE:
        case NodeType_NOTATION_NODE:
        default:
            // can't be imported
            throw RuntimeException();

        }
        if (deep)
        {
            // get children and import them
            Reference< XNode > const xChild = xImportedNode->getFirstChild();
            if (xChild.is())
            {
                lcl_ImportSiblings(xDocument, xNode, xChild);
            }
        }

        /* DOMNodeInsertedIntoDocument
         * Fired when a node is being inserted into a document,
         * either through direct insertion of the Node or insertion of a
         * subtree in which it is contained. This event is dispatched after
         * the insertion has taken place. The target of this event is the node
         * being inserted. If the Node is being directly inserted the DOMNodeInserted
         * event will fire before the DOMNodeInsertedIntoDocument event.
         *   Bubbles: No
         *   Cancelable: No
         *   Context Info: None
         */
        if (xNode.is())
        {
            Reference< XDocumentEvent > const xDocevent(xDocument, UNO_QUERY);
            Reference< XMutationEvent > const event(xDocevent->createEvent(
                "DOMNodeInsertedIntoDocument"), UNO_QUERY_THROW);
            event->initMutationEvent(
                "DOMNodeInsertedIntoDocument", true, false, Reference< XNode >(),
                OUString(), OUString(), OUString(), AttrChangeType(0) );
            Reference< XEventTarget > const xDocET(xDocument, UNO_QUERY);
            xDocET->dispatchEvent(event);
        }

        return xNode;
    }

    Reference< XNode > SAL_CALL CDocument::importNode(
            Reference< XNode > const& xImportedNode, sal_Bool deep)
    {
        if (!xImportedNode.is()) { throw RuntimeException(); }

        // NB: this whole operation inherently accesses 2 distinct documents.
        // The imported node could even be from a different DOM implementation,
        // so this implementation cannot make any assumptions about the
        // locking strategy of the imported node.
        // So the import takes no lock on this document;
        // it only calls UNO methods on this document that temporarily
        // lock the document, and UNO methods on the imported node that
        // may temporarily lock the other document.
        // As a consequence, the import is not atomic with regard to
        // concurrent modifications of either document, but it should not
        // deadlock.
        // To ensure that no members are accessed, the implementation is in
        // static non-member functions.

        Reference< XDocument > const xDocument(this);
        // already in doc?
        if (xImportedNode->getOwnerDocument() == xDocument) {
            return xImportedNode;
        }

        Reference< XNode > const xNode(
            lcl_ImportNode(xDocument, xImportedNode, deep) );
        return xNode;
    }

    OUString SAL_CALL CDocument::getNodeName()
    {
        // does not need mutex currently
        return OUString("#document");
    }

    OUString SAL_CALL CDocument::getNodeValue()
    {
        // does not need mutex currently
        return OUString();
    }

    Reference< XNode > SAL_CALL CDocument::cloneNode(sal_Bool bDeep)
    {
        ::osl::MutexGuard const g(m_rMutex);

        OSL_ASSERT(nullptr != m_aNodePtr);
        if (nullptr == m_aNodePtr) {
            return nullptr;
        }
        xmlDocPtr const pClone(xmlCopyDoc(m_aDocPtr, bDeep ? 1 : 0));
        if (nullptr == pClone) { return nullptr; }
        Reference< XNode > const xRet(
            static_cast<CNode*>(CDocument::CreateCDocument(pClone).get()));
        return xRet;
    }

    Reference< XEvent > SAL_CALL CDocument::createEvent(const OUString& aType)
    {
        // does not need mutex currently
        events::CEvent *pEvent = nullptr;
        if ( aType == "DOMSubtreeModified" || aType == "DOMNodeInserted" || aType == "DOMNodeRemoved"
          || aType == "DOMNodeRemovedFromDocument" || aType == "DOMNodeInsertedIntoDocument" || aType == "DOMAttrModified"
          || aType == "DOMCharacterDataModified")
        {
            pEvent = new events::CMutationEvent;

        } else if ( aType == "DOMFocusIn" || aType == "DOMFocusOut" || aType == "DOMActivate")
        {
            pEvent = new events::CUIEvent;
        } else if ( aType == "click"     || aType == "mousedown" || aType == "mouseup"
                 || aType == "mouseover" || aType == "mousemove" || aType == "mouseout" )
        {
            pEvent = new events::CMouseEvent;
        }
        else // generic event
        {
            pEvent = new events::CEvent;
        }
        return Reference< XEvent >(pEvent);
    }

    // css::xml::sax::XSAXSerializable
    void SAL_CALL CDocument::serialize(
            const Reference< XDocumentHandler >& i_xHandler,
            const Sequence< beans::StringPair >& i_rNamespaces)
    {
        ::osl::MutexGuard const g(m_Mutex);

        // add new namespaces to root node
        xmlNodePtr const pRoot = lcl_getDocumentRootPtr(m_aDocPtr);
        if (nullptr != pRoot) {
            const beans::StringPair * pSeq = i_rNamespaces.getConstArray();
            for (const beans::StringPair *pNsDef = pSeq;
                 pNsDef < pSeq + i_rNamespaces.getLength(); ++pNsDef) {
                OString prefix = OUStringToOString(pNsDef->First,
                                    RTL_TEXTENCODING_UTF8);
                OString href   = OUStringToOString(pNsDef->Second,
                                    RTL_TEXTENCODING_UTF8);
                // this will only add the ns if it does not exist already
                xmlNewNs(pRoot, reinterpret_cast<const xmlChar*>(href.getStr()),
                         reinterpret_cast<const xmlChar*>(prefix.getStr()));
            }
            // eliminate duplicate namespace declarations
            nscleanup(pRoot->children, pRoot);
        }
        saxify(i_xHandler);
    }

    // css::xml::sax::XFastSAXSerializable
    void SAL_CALL CDocument::fastSerialize( const Reference< XFastDocumentHandler >& i_xHandler,
                                            const Reference< XFastTokenHandler >& i_xTokenHandler,
                                            const Sequence< beans::StringPair >& i_rNamespaces,
                                            const Sequence< beans::Pair< OUString, sal_Int32 > >& i_rRegisterNamespaces )
    {
        ::osl::MutexGuard const g(m_Mutex);

        // add new namespaces to root node
        xmlNodePtr const pRoot = lcl_getDocumentRootPtr(m_aDocPtr);
        if (nullptr != pRoot) {
            const beans::StringPair * pSeq = i_rNamespaces.getConstArray();
            for (const beans::StringPair *pNsDef = pSeq;
                 pNsDef < pSeq + i_rNamespaces.getLength(); ++pNsDef) {
                OString prefix = OUStringToOString(pNsDef->First,
                                    RTL_TEXTENCODING_UTF8);
                OString href   = OUStringToOString(pNsDef->Second,
                                    RTL_TEXTENCODING_UTF8);
                // this will only add the ns if it does not exist already
                xmlNewNs(pRoot, reinterpret_cast<const xmlChar*>(href.getStr()),
                         reinterpret_cast<const xmlChar*>(prefix.getStr()));
            }
            // eliminate duplicate namespace declarations
            nscleanup(pRoot->children, pRoot);
        }

        Context aContext(i_xHandler,
                         i_xTokenHandler);

        // register namespace ids
        const beans::Pair<OUString,sal_Int32>* pSeq = i_rRegisterNamespaces.getConstArray();
        for (const beans::Pair<OUString,sal_Int32>* pNs = pSeq;
             pNs < pSeq + i_rRegisterNamespaces.getLength(); ++pNs)
        {
            OSL_ENSURE(pNs->Second >= FastToken::NAMESPACE,
                       "CDocument::fastSerialize(): invalid NS token id");
            aContext.maNamespaceMap[ pNs->First ] = pNs->Second;
        }

        fastSaxify(aContext);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
