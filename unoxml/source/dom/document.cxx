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

#include <com/sun/star/uno/Sequence.h>

#include "document.hxx"
#include "attr.hxx"
#include "element.hxx"
#include "cdatasection.hxx"
#include "documentfragment.hxx"
#include "text.hxx"
#include "cdatasection.hxx"
#include "comment.hxx"
#include "processinginstruction.hxx"
#include "entityreference.hxx"
#include "documenttype.hxx"
#include "elementlist.hxx"
#include "domimplementation.hxx"

#include "../events/event.hxx"
#include "../events/mutationevent.hxx"
#include "../events/uievent.hxx"
#include "../events/mouseevent.hxx"

#include <string.h>

#include <com/sun/star/xml/sax/FastToken.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>

namespace DOM
{
    CDocument::~CDocument()
    {
        xmlFreeDoc(m_aDocPtr);
    }

    CDocument::CDocument(xmlDocPtr aDocPtr)
        : CDocument_Base(
                NodeType_DOCUMENT_NODE, reinterpret_cast<xmlNodePtr>(aDocPtr))
        , m_aDocPtr(aDocPtr)
        , m_streamListeners()
    {
    }

    void SAL_CALL CDocument::saxify(
            const Reference< XDocumentHandler >& i_xHandler) {
        i_xHandler->startDocument();
        for (xmlNodePtr pChild = m_aNodePtr->children;
                        pChild != 0; pChild = pChild->next) {
            ::rtl::Reference<CNode> const pNode = CNode::getCNode(pChild);
            OSL_ENSURE(pNode != 0, "CNode::get returned 0");
            pNode->saxify(i_xHandler);
        }
        i_xHandler->endDocument();
    }

    void SAL_CALL CDocument::fastSaxify( Context& rContext ) {
        rContext.mxDocHandler->startDocument();
        for (xmlNodePtr pChild = m_aNodePtr->children;
                        pChild != 0; pChild = pChild->next) {
            ::rtl::Reference<CNode> const pNode = CNode::getCNode(pChild);
            OSL_ENSURE(pNode != 0, "CNode::get returned 0");
            pNode->fastSaxify(rContext);
        }
        rContext.mxDocHandler->endDocument();
    }

    void SAL_CALL CDocument::addListener(const Reference< XStreamListener >& aListener )
        throw (RuntimeException)
    {
        m_streamListeners.insert(aListener);
    }

    void SAL_CALL CDocument::removeListener(const Reference< XStreamListener >& aListener )
        throw (RuntimeException)
    {
        m_streamListeners.erase(aListener);
    }

    // IO context functions for libxml2 interaction
    typedef struct {
        Reference< XOutputStream > stream;
        bool allowClose;
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

    // clsoe callback
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
        throw (RuntimeException)
    {
        if (! m_rOutputStream.is()) return;

        // notify listners about start
        listenerlist_t::const_iterator iter1 = m_streamListeners.begin();
        while (iter1 != m_streamListeners.end()) {
            Reference< XStreamListener > aListener = *iter1;
            aListener->started();
            iter1++;
        }

        // setup libxml IO and write data to output stream
        IOContext ioctx = {m_rOutputStream, false};
        xmlOutputBufferPtr pOut = xmlOutputBufferCreateIO(
            writeCallback, closeCallback, &ioctx, NULL);
        xmlSaveFileTo(pOut, m_aNodePtr->doc, NULL);

        // call listeners
        listenerlist_t::const_iterator iter2 = m_streamListeners.begin();
        while (iter2 != m_streamListeners.end()) {
            Reference< XStreamListener > aListener = *iter2;
            aListener->closed();
            iter2++;
        }

    }

    void SAL_CALL CDocument::terminate()
        throw (RuntimeException)
    {
        // not supported
    }

    void SAL_CALL CDocument::setOutputStream( const Reference< XOutputStream >& aStream )
        throw (RuntimeException)
    {
        m_rOutputStream = aStream;
    }

    Reference< XOutputStream > SAL_CALL  CDocument::getOutputStream() throw (RuntimeException)
    {
        return m_rOutputStream;
    }

    // Creates an Attr of the given name.
    Reference< XAttr > SAL_CALL CDocument::createAttribute(const OUString& name)
        throw (RuntimeException, DOMException)
    {
        OString o1 = OUStringToOString(name, RTL_TEXTENCODING_UTF8);
        xmlChar *xName = (xmlChar*)o1.getStr();
        xmlAttrPtr const pAttr = xmlNewDocProp(m_aDocPtr, xName, NULL);
        Reference< XAttr > const xRet(
            static_cast< XNode* >(CNode::getCNode(
                    reinterpret_cast<xmlNodePtr>(pAttr)).get()),
            UNO_QUERY_THROW);
        return xRet;
    };

    // Creates an attribute of the given qualified name and namespace URI.
    Reference< XAttr > SAL_CALL CDocument::createAttributeNS(
            const OUString& ns, const OUString& qname)
        throw (RuntimeException, DOMException)
    {

        // libxml does not allow a NS definition to be attached to an
        // attribute node - which is a good thing, since namespaces are
        // only defined as parts of element nodes
        // thus, we create a temporary element node which carries the ns definition
        // and is removed/merged as soon as the attribute gets append to it's
        // actual parent
        sal_Int32 i = qname.indexOf(':');
        OString oPrefix, oName, oUri;
        xmlChar *xPrefix, *xName, *xUri;
        if (i != -1)
        {
            oPrefix = OUStringToOString(qname.copy(0, i), RTL_TEXTENCODING_UTF8);
            xPrefix = (xmlChar*)oPrefix.getStr();
            oName = OUStringToOString(qname.copy(i+1, qname.getLength()-i-1), RTL_TEXTENCODING_UTF8);
        }
        else
        {
            xPrefix = (xmlChar*)"";
            oName = OUStringToOString(qname, RTL_TEXTENCODING_UTF8);
        }
        xName = (xmlChar*)oName.getStr();
        oUri = OUStringToOString(ns, RTL_TEXTENCODING_UTF8);
        xUri = (xmlChar*)oUri.getStr();

        // create the carrier node
        xmlNodePtr pNode = xmlNewDocNode(m_aDocPtr, NULL, (xmlChar*)"__private", NULL);
        xmlNsPtr pNs = xmlNewNs(pNode, xUri, xPrefix);
        xmlAttrPtr pAttr = xmlNewNsProp(pNode, pNs, xName, NULL);
        Reference< XAttr > const xRet(
            static_cast< XNode* >(CNode::getCNode(
                    reinterpret_cast<xmlNodePtr>(pAttr)).get()),
            UNO_QUERY_THROW);
        return xRet;
    };

    // Creates a CDATASection node whose value is the specified string.
    Reference< XCDATASection > SAL_CALL CDocument::createCDATASection(const OUString& data)
        throw (RuntimeException)
    {
        xmlChar *xData = (xmlChar*)OUStringToOString(data, RTL_TEXTENCODING_UTF8).getStr();
        xmlNodePtr pText = xmlNewCDataBlock(m_aDocPtr, xData, strlen((char*)xData));
        Reference< XCDATASection > const xRet(
            static_cast< XNode* >(CNode::getCNode(pText).get()),
            UNO_QUERY_THROW);
        return xRet;
    }

    // Creates a Comment node given the specified string.
    Reference< XComment > SAL_CALL CDocument::createComment(const OUString& data)
        throw (RuntimeException)
    {
        OString o1 = OUStringToOString(data, RTL_TEXTENCODING_UTF8);
        xmlChar *xData = (xmlChar*)o1.getStr();
        xmlNodePtr pComment = xmlNewDocComment(m_aDocPtr, xData);
        Reference< XComment > const xRet(
            static_cast< XNode* >(CNode::getCNode(pComment).get()),
            UNO_QUERY_THROW);
        return xRet;
    }

    //Creates an empty DocumentFragment object.
    Reference< XDocumentFragment > SAL_CALL CDocument::createDocumentFragment()
        throw (RuntimeException)
    {
        xmlNodePtr pFrag = xmlNewDocFragment(m_aDocPtr);
        Reference< XDocumentFragment > const xRet(
            static_cast< XNode* >(CNode::getCNode(pFrag).get()),
            UNO_QUERY_THROW);
        return xRet;
    }

    // Creates an element of the type specified.
    Reference< XElement > SAL_CALL CDocument::createElement(const OUString& tagName)
        throw (RuntimeException, DOMException)
    {
        OString o1 = OUStringToOString(tagName, RTL_TEXTENCODING_UTF8);
        xmlChar *xName = (xmlChar*)o1.getStr();
        xmlNodePtr const pNode = xmlNewDocNode(m_aDocPtr, NULL, xName, NULL);
        Reference< XElement > const xRet(
            static_cast< XNode* >(CNode::getCNode(pNode).get()),
            UNO_QUERY_THROW);
        return xRet;
    }

    // Creates an element of the given qualified name and namespace URI.
    Reference< XElement > SAL_CALL CDocument::createElementNS(
            const OUString& ns, const OUString& qname)
        throw (RuntimeException, DOMException)
    {
        sal_Int32 i = qname.indexOf(':');
        if (ns.getLength() == 0) throw RuntimeException();
        xmlChar *xPrefix;
        xmlChar *xName;
        OString o1, o2, o3;
        if ( i != -1) {
            o1 = OUStringToOString(qname.copy(0, i), RTL_TEXTENCODING_UTF8);
            xPrefix = (xmlChar*)o1.getStr();
            o2 = OUStringToOString(qname.copy(i+1, qname.getLength()-i-1), RTL_TEXTENCODING_UTF8);
            xName = (xmlChar*)o2.getStr();
        } else {
            // default prefix
            xPrefix = (xmlChar*)"";
            o2 = OUStringToOString(qname, RTL_TEXTENCODING_UTF8);
            xName = (xmlChar*)o2.getStr();
        }
        o3 = OUStringToOString(ns, RTL_TEXTENCODING_UTF8);
        xmlChar *xUri = (xmlChar*)o3.getStr();

        // xmlNsPtr aNsPtr = xmlNewReconciledNs?
        // xmlNsPtr aNsPtr = xmlNewGlobalNs?
        xmlNodePtr const pNode = xmlNewDocNode(m_aDocPtr, NULL, xName, NULL);
        xmlNsPtr const pNs = xmlNewNs(pNode, xUri, xPrefix);
        xmlSetNs(pNode, pNs);
        Reference< XElement > const xRet(
            static_cast< XNode* >(CNode::getCNode(pNode).get()),
            UNO_QUERY_THROW);
        return xRet;
    }

    //Creates an EntityReference object.
    Reference< XEntityReference > SAL_CALL CDocument::createEntityReference(const OUString& name)
        throw (RuntimeException, DOMException)
    {
        OString o1 = OUStringToOString(name, RTL_TEXTENCODING_UTF8);
        xmlChar *xName = (xmlChar*)o1.getStr();
        xmlNodePtr const pNode = xmlNewReference(m_aDocPtr, xName);
        Reference< XEntityReference > const xRet(
            static_cast< XNode* >(CNode::getCNode(pNode).get()),
            UNO_QUERY_THROW);
        return xRet;
    }

    // Creates a ProcessingInstruction node given the specified name and
    // data strings.
    Reference< XProcessingInstruction > SAL_CALL CDocument::createProcessingInstruction(
            const OUString& target, const OUString& data)
        throw (RuntimeException, DOMException)
    {
        OString o1 = OUStringToOString(target, RTL_TEXTENCODING_UTF8);
        xmlChar *xTarget = (xmlChar*)o1.getStr();
        OString o2 = OUStringToOString(data, RTL_TEXTENCODING_UTF8);
        xmlChar *xData = (xmlChar*)o2.getStr();
        xmlNodePtr const pNode = xmlNewPI(xTarget, xData);
        pNode->doc = m_aDocPtr;
        Reference< XProcessingInstruction > const xRet(
            static_cast< XNode* >(CNode::getCNode(pNode).get()),
            UNO_QUERY_THROW);
        return xRet;
    }

    // Creates a Text node given the specified string.
    Reference< XText > SAL_CALL CDocument::createTextNode(const OUString& data)
        throw (RuntimeException)
    {
        OString o1 = OUStringToOString(data, RTL_TEXTENCODING_UTF8);
        xmlChar *xData = (xmlChar*)o1.getStr();
        xmlNodePtr const pNode = xmlNewDocText(m_aDocPtr, xData);
        Reference< XText > const xRet(
            static_cast< XNode* >(CNode::getCNode(pNode).get()),
            UNO_QUERY_THROW);
        return xRet;
    }

    // The Document Type Declaration (see DocumentType) associated with this
    // document.
    Reference< XDocumentType > SAL_CALL CDocument::getDoctype()
        throw (RuntimeException)
    {
        // find the doc type
        xmlNodePtr cur = m_aDocPtr->children;
        while (cur != NULL)
        {
            if (cur->type == XML_DOCUMENT_TYPE_NODE || cur->type == XML_DTD_NODE)
                break;
        }
        Reference< XDocumentType > const xRet(
            static_cast< XNode* >(CNode::getCNode(cur).get()),
            UNO_QUERY_THROW);
        return xRet;
    }

    /// get the pointer to the root element node of the document
    static xmlNodePtr SAL_CALL _getDocumentRootPtr(xmlDocPtr i_pDocument) {
        // find the document element
        xmlNodePtr cur = i_pDocument->children;
        while (cur != NULL)
        {
            if (cur->type == XML_ELEMENT_NODE)
                break;
            cur = cur->next;
        }
        return cur;
    }

    // This is a convenience attribute that allows direct access to the child
    // node that is the root element of the document.
    Reference< XElement > SAL_CALL CDocument::getDocumentElement()
        throw (RuntimeException)
    {
        xmlNodePtr const pNode = _getDocumentRootPtr(m_aDocPtr);
        Reference< XElement > const xRet(
            static_cast< XNode* >(CNode::getCNode(pNode).get()),
            UNO_QUERY_THROW);
        return xRet;
    }

    static xmlNodePtr _search_element_by_id(const xmlNodePtr cur, const xmlChar* id)
    {

        if (cur == NULL)
            return NULL;
        // look in current node
        if (cur->type == XML_ELEMENT_NODE)
        {
            xmlAttrPtr a = cur->properties;
            while (a != NULL)
            {
                if (a->atype == XML_ATTRIBUTE_ID) {
                    if (strcmp((char*)a->children->content, (char*)id) == 0)
                        return cur;
                }
                a = a->next;
            }
        }
        // look in children
        xmlNodePtr result = _search_element_by_id(cur->children, id);
        if (result != NULL)
            return result;
        result = _search_element_by_id(cur->next, id);
            return result;
    }

    // Returns the Element whose ID is given by elementId.
    Reference< XElement > SAL_CALL CDocument::getElementById(const OUString& elementId)
        throw (RuntimeException)
    {
        // search the tree for an element with the given ID
        OString o1 = OUStringToOString(elementId, RTL_TEXTENCODING_UTF8);
        xmlChar *xId = (xmlChar*)o1.getStr();
        xmlNodePtr pStart = CNode::getNodePtr(getDocumentElement().get());
        xmlNodePtr const pNode = _search_element_by_id(pStart, xId);
        Reference< XElement > const xRet(
            static_cast< XNode* >(CNode::getCNode(pNode).get()),
            UNO_QUERY_THROW);
        return xRet;
    }


    Reference< XNodeList > SAL_CALL CDocument::getElementsByTagName(const OUString& tagname)
            throw (RuntimeException)
    {
        // build a list
        return Reference< XNodeList >(
            new CElementList(static_cast< CElement* >(
            this->getDocumentElement().get()), tagname));
    }

    Reference< XNodeList > SAL_CALL CDocument::getElementsByTagNameNS(
            const OUString& namespaceURI, const OUString& localName)
        throw (RuntimeException)
    {
        return Reference< XNodeList >(
            new CElementList(static_cast< CElement* >(
            this->getDocumentElement().get()), namespaceURI, localName));
    }

    Reference< XDOMImplementation > SAL_CALL CDocument::getImplementation()
        throw (RuntimeException)
    {
        // XXX
        return Reference< XDOMImplementation >(CDOMImplementation::get());
    }

    // helper function to recall import for siblings
    static Reference< XNode > _import_siblings (
        const Reference< XNode > aNode, const Reference< XNode> parent, CDocument* pTarget)
    {
        Reference< XNode > sibling = aNode;
        Reference< XNode > tmp;
        Reference< XNode > firstImported;
        while (sibling.is())
        {
            tmp = pTarget->importNode(sibling, sal_True);
            parent->appendChild(tmp);
            if (!firstImported.is())
                firstImported = tmp;
            sibling = sibling->getNextSibling();
        }
        return firstImported;
    }

    Reference< XNode > SAL_CALL CDocument::importNode(
            const Reference< XNode >& importedNode, sal_Bool deep)
        throw (RuntimeException, DOMException)
    {
        // this node could be from another memory model
        // only use uno interfaces to access is!!!

        {
            // already in doc?
            Reference< XDocument > const xDocument(
                static_cast< XNode* >(CNode::getCNode(
                    reinterpret_cast<xmlNodePtr>(m_aDocPtr)).get()),
                UNO_QUERY_THROW);
            if (importedNode->getOwnerDocument() == xDocument) {
                return importedNode;
            }
        }

        Reference< XNode > aNode;
        NodeType aNodeType = importedNode->getNodeType();
        switch (aNodeType)
        {
        case NodeType_ATTRIBUTE_NODE:
        {
            Reference< XAttr > attr(importedNode, UNO_QUERY);
            Reference< XAttr > newAttr = createAttribute(attr->getName());
            newAttr->setValue(attr->getValue());
            aNode.set(newAttr, UNO_QUERY);
            break;
        }
        case NodeType_CDATA_SECTION_NODE:
        {
            Reference< XCDATASection > cdata(importedNode, UNO_QUERY);
            Reference< XCDATASection > newCdata = createCDATASection(cdata->getData());
            aNode.set(newCdata, UNO_QUERY);
            break;
        }
        case NodeType_COMMENT_NODE:
        {
            Reference< XComment > comment(importedNode, UNO_QUERY);
            Reference< XComment > newComment = createComment(comment->getData());
            aNode.set(newComment, UNO_QUERY);
            break;
        }
        case NodeType_DOCUMENT_FRAGMENT_NODE:
        {
            Reference< XDocumentFragment > frag(importedNode, UNO_QUERY);
            Reference< XDocumentFragment > newFrag = createDocumentFragment();
            aNode.set(newFrag, UNO_QUERY);
            break;
        }
        case NodeType_ELEMENT_NODE:
        {
            Reference< XElement > element(importedNode, UNO_QUERY);
            OUString aNsUri = importedNode->getNamespaceURI();
            OUString aNsPrefix = importedNode->getPrefix();
            OUString aQName = element->getTagName();
            Reference< XElement > newElement;
            if (aNsUri.getLength() > 0)
            {

                if (aNsPrefix.getLength() > 0)
                    aQName = aNsPrefix + OUString::createFromAscii(":") + aQName;
                newElement = createElementNS(aNsUri, aQName);
            }
            else
                newElement = createElement(aQName);

            // get attributes
            if (element->hasAttributes())
            {
                Reference< XNamedNodeMap > attribs = element->getAttributes();
                Reference< XAttr > curAttr;
                for (sal_Int32 i = 0; i < attribs->getLength(); i++)
                {
                    curAttr = Reference< XAttr >(attribs->item(i), UNO_QUERY);
                    OUString aAttrUri = curAttr->getNamespaceURI();
                    OUString aAttrPrefix = curAttr->getPrefix();
                    OUString aAttrName = curAttr->getName();
                    if (aAttrUri.getLength() > 0)
                    {
                        if (aAttrPrefix.getLength() > 0)
                            aAttrName = aAttrPrefix + OUString::createFromAscii(":") + aAttrName;
                        newElement->setAttributeNS(aAttrUri, aAttrName, curAttr->getValue());
                    }
                    else
                        newElement->setAttribute(aAttrName, curAttr->getValue());
                }
            }
            aNode.set(newElement, UNO_QUERY);
            break;
        }
        case NodeType_ENTITY_REFERENCE_NODE:
        {
            Reference< XEntityReference > ref(importedNode, UNO_QUERY);
            Reference< XEntityReference > newRef(createEntityReference(ref->getNodeName()));
            aNode.set(newRef, UNO_QUERY);
            break;
        }
        case NodeType_PROCESSING_INSTRUCTION_NODE:
        {
            Reference< XProcessingInstruction > pi(importedNode, UNO_QUERY);
            Reference< XProcessingInstruction > newPi(
                createProcessingInstruction(pi->getTarget(), pi->getData()));
            aNode.set(newPi, UNO_QUERY);
            break;
        }
        case NodeType_TEXT_NODE:
        {
            Reference< XText > text(importedNode, UNO_QUERY);
            Reference< XText > newText(createTextNode(text->getData()));
            aNode.set(newText, UNO_QUERY);
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
            Reference< XNode > child = importedNode->getFirstChild();
            if (child.is())
            {
                _import_siblings(child, aNode, this);
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
        if (aNode.is())
        {
            Reference< XDocumentEvent > docevent(getOwnerDocument(), UNO_QUERY);
            Reference< XMutationEvent > event(docevent->createEvent(
                OUString::createFromAscii("DOMNodeInsertedIntoDocument")), UNO_QUERY);
            event->initMutationEvent(OUString::createFromAscii("DOMNodeInsertedIntoDocument")
                , sal_True, sal_False, Reference< XNode >(),
                OUString(), OUString(), OUString(), (AttrChangeType)0 );
            dispatchEvent(Reference< XEvent >(event, UNO_QUERY));
        }

        return aNode;
    }
    OUString SAL_CALL CDocument::getNodeName()throw (RuntimeException)
    {
        return OUString::createFromAscii("#document");
    }
    OUString SAL_CALL CDocument::getNodeValue() throw (RuntimeException)
    {
        return OUString();
    }

    Reference< XEvent > SAL_CALL CDocument::createEvent(const OUString& aType) throw (RuntimeException)
    {
        events::CEvent *pEvent = 0;
        if (
            aType.compareToAscii("DOMSubtreeModified")          == 0||
            aType.compareToAscii("DOMNodeInserted")             == 0||
            aType.compareToAscii("DOMNodeRemoved")              == 0||
            aType.compareToAscii("DOMNodeRemovedFromDocument")  == 0||
            aType.compareToAscii("DOMNodeInsertedIntoDocument") == 0||
            aType.compareToAscii("DOMAttrModified")             == 0||
            aType.compareToAscii("DOMCharacterDataModified")    == 0)
        {
            pEvent = new events::CMutationEvent;

        } else if (
            aType.compareToAscii("DOMFocusIn")  == 0||
            aType.compareToAscii("DOMFocusOut") == 0||
            aType.compareToAscii("DOMActivate") == 0)
        {
            pEvent = new events::CUIEvent;
        } else if (
            aType.compareToAscii("click")     == 0||
            aType.compareToAscii("mousedown") == 0||
            aType.compareToAscii("mouseup")   == 0||
            aType.compareToAscii("mouseover") == 0||
            aType.compareToAscii("mousemove") == 0||
            aType.compareToAscii("mouseout")  == 0 )
        {
            pEvent = new events::CMouseEvent;
        }
        else // generic event
        {
            pEvent = new events::CEvent;
        }
        return Reference< XEvent >(pEvent);
    }

    // ::com::sun::star::xml::sax::XSAXSerializable
    void SAL_CALL CDocument::serialize(
            const Reference< XDocumentHandler >& i_xHandler,
            const Sequence< beans::StringPair >& i_rNamespaces)
        throw (RuntimeException, SAXException)
    {
        // add new namespaces to root node
        xmlNodePtr pRoot = _getDocumentRootPtr(m_aDocPtr);
        if (0 != pRoot) {
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
            _nscleanup(pRoot->children, pRoot);
        }
        saxify(i_xHandler);
    }

    // ::com::sun::star::xml::sax::XFastSAXSerializable
    void SAL_CALL CDocument::fastSerialize( const Reference< XFastDocumentHandler >& i_xHandler,
                                            const Reference< XFastTokenHandler >& i_xTokenHandler,
                                            const Sequence< beans::StringPair >& i_rNamespaces,
                                            const Sequence< beans::Pair< rtl::OUString, sal_Int32 > >& i_rRegisterNamespaces )
        throw (SAXException, RuntimeException)
    {
        // add new namespaces to root node
        xmlNodePtr pRoot = _getDocumentRootPtr(m_aDocPtr);
        if (0 != pRoot) {
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
            _nscleanup(pRoot->children, pRoot);
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
