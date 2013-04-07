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

#ifndef DOM_DOCUMENT_HXX
#define DOM_DOCUMENT_HXX

#include <set>
#include <memory>

#include <libxml/tree.h>

#include <sal/types.h>

#include <cppuhelper/implbase6.hxx>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XAttr.hpp>
#include <com/sun/star/xml/dom/XElement.hpp>
#include <com/sun/star/xml/dom/XDOMImplementation.hpp>
#include <com/sun/star/xml/dom/events/XDocumentEvent.hpp>
#include <com/sun/star/xml/dom/events/XEvent.hpp>
#include <com/sun/star/xml/sax/XSAXSerializable.hpp>
#include <com/sun/star/xml/sax/XFastSAXSerializable.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XFastDocumentHandler.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XActiveDataControl.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XStreamListener.hpp>

#include "node.hxx"


using namespace std;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::xml::sax;
using namespace com::sun::star::io;
using namespace com::sun::star::xml::dom;
using namespace com::sun::star::xml::dom::events;

namespace DOM
{
    namespace events {
        class CEventDispatcher;
    }

    class CElement;

    typedef ::cppu::ImplInheritanceHelper6<
            CNode, XDocument, XDocumentEvent,
            XActiveDataControl, XActiveDataSource,
            XSAXSerializable, XFastSAXSerializable>
        CDocument_Base;

    class CDocument
        : public CDocument_Base
    {

    private:
        /// this Mutex is used for synchronization of all UNO wrapper
        /// objects that belong to this document
        ::osl::Mutex m_Mutex;
        /// the libxml document: freed in destructor
        /// => all UNO wrapper objects must keep the CDocument alive
        xmlDocPtr const m_aDocPtr;

        // datacontrol/source state
        typedef set< Reference< XStreamListener > > listenerlist_t;
        listenerlist_t m_streamListeners;
        Reference< XOutputStream > m_rOutputStream;

        typedef std::map< const xmlNodePtr,
                    ::std::pair< WeakReference<XNode>, CNode* > > nodemap_t;
        nodemap_t m_NodeMap;

        ::std::auto_ptr<events::CEventDispatcher> const m_pEventDispatcher;

        CDocument(xmlDocPtr const pDocPtr);


    public:
        /// factory: only way to create instance!
        static ::rtl::Reference<CDocument>
            CreateCDocument(xmlDocPtr const pDoc);

        virtual ~CDocument();

        // needed by CXPathAPI
        ::osl::Mutex & GetMutex() { return m_Mutex; }

        events::CEventDispatcher & GetEventDispatcher();
        ::rtl::Reference< CElement > GetDocumentElement();

        /// get UNO wrapper instance for a libxml node
        ::rtl::Reference<CNode> GetCNode(
                xmlNodePtr const pNode, bool const bCreate = true);
        /// remove a UNO wrapper instance
        void RemoveCNode(xmlNodePtr const pNode, CNode const*const pCNode);

        virtual CDocument & GetOwnerDocument();

        virtual void saxify(const Reference< XDocumentHandler >& i_xHandler);

        virtual void fastSaxify( Context& rContext );

        virtual bool IsChildTypeAllowed(NodeType const nodeType);

        /**
        Creates an Attr of the given name.
        */
        virtual Reference< XAttr > SAL_CALL createAttribute(const OUString& name)
            throw (RuntimeException, DOMException);

        /**
        Creates an attribute of the given qualified name and namespace URI.
        */
        virtual Reference< XAttr > SAL_CALL createAttributeNS(const OUString& namespaceURI, const OUString& qualifiedName)
            throw (RuntimeException, DOMException);

        /**
        Creates a CDATASection node whose value is the specified string.
        */
        virtual Reference< XCDATASection > SAL_CALL createCDATASection(const OUString& data)
            throw (RuntimeException);

        /**
        Creates a Comment node given the specified string.
        */
        virtual Reference< XComment > SAL_CALL createComment(const OUString& data)
            throw (RuntimeException);

        /**
        Creates an empty DocumentFragment object.
        */
        virtual Reference< XDocumentFragment > SAL_CALL createDocumentFragment()
            throw (RuntimeException);

        /**
        Creates an element of the type specified.
        */
        virtual Reference< XElement > SAL_CALL createElement(const OUString& tagName)
            throw (RuntimeException, DOMException);

        /**
        Creates an element of the given qualified name and namespace URI.
        */
        virtual Reference< XElement > SAL_CALL createElementNS(const OUString& namespaceURI, const OUString& qualifiedName)
            throw (RuntimeException, DOMException);

        /**
        Creates an EntityReference object.
        */
        virtual Reference< XEntityReference > SAL_CALL createEntityReference(const OUString& name)
            throw (RuntimeException, DOMException);

        /**
        Creates a ProcessingInstruction node given the specified name and
        data strings.
        */
        virtual Reference< XProcessingInstruction > SAL_CALL createProcessingInstruction(
                const OUString& target, const OUString& data)
            throw (RuntimeException, DOMException);

        /**
        Creates a Text node given the specified string.
        */
        virtual Reference< XText > SAL_CALL createTextNode(const OUString& data)
            throw (RuntimeException);

        /**
        The Document Type Declaration (see DocumentType) associated with this
        document.
        */
        virtual Reference< XDocumentType > SAL_CALL getDoctype()
            throw (RuntimeException);

        /**
        This is a convenience attribute that allows direct access to the child
        node that is the root element of the document.
        */
        virtual Reference< XElement > SAL_CALL getDocumentElement()
            throw (RuntimeException);

        /**
        Returns the Element whose ID is given by elementId.
        */
        virtual Reference< XElement > SAL_CALL getElementById(const OUString& elementId)
            throw (RuntimeException);

        /**
        Returns a NodeList of all the Elements with a given tag name in the
        order in which they are encountered in a preorder traversal of the
        Document tree.
        */
        virtual Reference< XNodeList > SAL_CALL getElementsByTagName(const OUString& tagname)
            throw (RuntimeException);

        /**
        Returns a NodeList of all the Elements with a given local name and
        namespace URI in the order in which they are encountered in a preorder
        traversal of the Document tree.
        */
        virtual Reference< XNodeList > SAL_CALL getElementsByTagNameNS(const OUString& namespaceURI, const OUString& localName)
            throw (RuntimeException);

        /**
        The DOMImplementation object that handles this document.
        */
        virtual Reference< XDOMImplementation > SAL_CALL getImplementation()
            throw (RuntimeException);

        /**
        Imports a node from another document to this document.
        */
        virtual Reference< XNode > SAL_CALL importNode(const Reference< XNode >& importedNode, sal_Bool deep)
            throw (RuntimeException, DOMException);

        // XDocumentEvent
        virtual Reference< XEvent > SAL_CALL createEvent(const OUString& eventType) throw (RuntimeException);

        // XActiveDataControl,
        // see http://api.libreoffice.org/docs/common/ref/com/sun/star/io/XActiveDataControl.html
        virtual void SAL_CALL addListener(const Reference< XStreamListener >& aListener )  throw (RuntimeException);
        virtual void SAL_CALL removeListener(const Reference< XStreamListener >& aListener ) throw (RuntimeException);
        virtual void SAL_CALL start() throw (RuntimeException);
        virtual void SAL_CALL terminate() throw (RuntimeException);

        // XActiveDataSource
        // see http://api.libreoffice.org/docs/common/ref/com/sun/star/io/XActiveDataSource.html
        virtual void SAL_CALL setOutputStream(  const Reference< XOutputStream >& aStream ) throw (RuntimeException);
        virtual Reference< XOutputStream > SAL_CALL getOutputStream() throw (RuntimeException);

        // ---- resolve uno inheritance problems...
        // overrides for XNode base
        virtual OUString SAL_CALL getNodeName()
            throw (RuntimeException);
        virtual OUString SAL_CALL getNodeValue()
            throw (RuntimeException);
        virtual Reference< XNode > SAL_CALL cloneNode(sal_Bool deep)
            throw (RuntimeException);
        // --- delegation for XNde base.
        virtual Reference< XNode > SAL_CALL appendChild(const Reference< XNode >& newChild)
            throw (RuntimeException, DOMException)
        {
            return CNode::appendChild(newChild);
        }
        virtual Reference< XNamedNodeMap > SAL_CALL getAttributes()
            throw (RuntimeException)
        {
            return CNode::getAttributes();
        }
        virtual Reference< XNodeList > SAL_CALL getChildNodes()
            throw (RuntimeException)
        {
            return CNode::getChildNodes();
        }
        virtual Reference< XNode > SAL_CALL getFirstChild()
            throw (RuntimeException)
        {
            return CNode::getFirstChild();
        }
        virtual Reference< XNode > SAL_CALL getLastChild()
            throw (RuntimeException)
        {
            return CNode::getLastChild();
        }
        virtual OUString SAL_CALL getLocalName()
            throw (RuntimeException)
        {
            return CNode::getLocalName();
        }
        virtual OUString SAL_CALL getNamespaceURI()
            throw (RuntimeException)
        {
            return CNode::getNamespaceURI();
        }
        virtual Reference< XNode > SAL_CALL getNextSibling()
            throw (RuntimeException)
        {
            return CNode::getNextSibling();
        }
        virtual NodeType SAL_CALL getNodeType()
            throw (RuntimeException)
        {
            return CNode::getNodeType();
        }
        virtual Reference< XDocument > SAL_CALL getOwnerDocument()
            throw (RuntimeException)
        {
            return CNode::getOwnerDocument();
        }
        virtual Reference< XNode > SAL_CALL getParentNode()
            throw (RuntimeException)
        {
            return CNode::getParentNode();
        }
        virtual OUString SAL_CALL getPrefix()
            throw (RuntimeException)
        {
           return CNode::getPrefix();
        }
        virtual Reference< XNode > SAL_CALL getPreviousSibling()
            throw (RuntimeException)
        {
            return CNode::getPreviousSibling();
        }
        virtual sal_Bool SAL_CALL hasAttributes()
            throw (RuntimeException)
        {
            return CNode::hasAttributes();
        }
        virtual sal_Bool SAL_CALL hasChildNodes()
            throw (RuntimeException)
        {
            return CNode::hasChildNodes();
        }
        virtual Reference< XNode > SAL_CALL insertBefore(
                const Reference< XNode >& newChild, const Reference< XNode >& refChild)
            throw (RuntimeException, DOMException)
        {
            return CNode::insertBefore(newChild, refChild);
        }
        virtual sal_Bool SAL_CALL isSupported(const OUString& feature, const OUString& ver)
            throw (RuntimeException)
        {
            return CNode::isSupported(feature, ver);
        }
        virtual void SAL_CALL normalize()
            throw (RuntimeException)
        {
            CNode::normalize();
        }
        virtual Reference< XNode > SAL_CALL removeChild(const Reference< XNode >& oldChild)
            throw (RuntimeException, DOMException)
        {
            return CNode::removeChild(oldChild);
        }
        virtual Reference< XNode > SAL_CALL replaceChild(
                const Reference< XNode >& newChild, const Reference< XNode >& oldChild)
            throw (RuntimeException, DOMException)
        {
            return CNode::replaceChild(newChild, oldChild);
        }
        virtual void SAL_CALL setNodeValue(const OUString& nodeValue)
            throw (RuntimeException, DOMException)
        {
            return CNode::setNodeValue(nodeValue);
        }
        virtual void SAL_CALL setPrefix(const OUString& prefix)
            throw (RuntimeException, DOMException)
        {
            return CNode::setPrefix(prefix);
        }

        // ::com::sun::star::xml::sax::XSAXSerializable
        virtual void SAL_CALL serialize(
            const Reference< XDocumentHandler >& i_xHandler,
            const Sequence< beans::StringPair >& i_rNamespaces)
            throw (RuntimeException, SAXException);

        // ::com::sun::star::xml::sax::XFastSAXSerializable
        virtual void SAL_CALL fastSerialize( const Reference< XFastDocumentHandler >& handler,
                                             const Reference< XFastTokenHandler >& tokenHandler,
                                             const Sequence< beans::StringPair >& i_rNamespaces,
                                             const Sequence< beans::Pair< OUString, sal_Int32 > >& namespaces )
            throw (SAXException, RuntimeException);
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
