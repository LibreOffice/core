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

#ifndef INCLUDED_UNOXML_SOURCE_DOM_DOCUMENT_HXX
#define INCLUDED_UNOXML_SOURCE_DOM_DOCUMENT_HXX

#include <set>
#include <memory>

#include <libxml/tree.h>

#include <sal/types.h>

#include <cppuhelper/implbase.hxx>

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

namespace DOM
{
    namespace events {
        class CEventDispatcher;
    }

    class CElement;

    typedef ::cppu::ImplInheritanceHelper<
            CNode, css::xml::dom::XDocument, css::xml::dom::events::XDocumentEvent,
            css::io::XActiveDataControl, css::io::XActiveDataSource,
            css::xml::sax::XSAXSerializable, css::xml::sax::XFastSAXSerializable>
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
        typedef std::set< css::uno::Reference< css::io::XStreamListener > > listenerlist_t;
        listenerlist_t m_streamListeners;
        css::uno::Reference< css::io::XOutputStream > m_rOutputStream;

        typedef std::map< const xmlNodePtr,
                    ::std::pair< css::uno::WeakReference<css::xml::dom::XNode>, CNode* > > nodemap_t;
        nodemap_t m_NodeMap;

        ::std::unique_ptr<events::CEventDispatcher> const m_pEventDispatcher;

        explicit CDocument(xmlDocPtr const pDocPtr);


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

        virtual CDocument & GetOwnerDocument() override;

        virtual void saxify(const css::uno::Reference< css::xml::sax::XDocumentHandler >& i_xHandler) override;

        virtual void fastSaxify( Context& rContext ) override;

        virtual bool IsChildTypeAllowed(css::xml::dom::NodeType const nodeType) override;

        /**
        Creates an Attr of the given name.
        */
        virtual css::uno::Reference< css::xml::dom::XAttr > SAL_CALL createAttribute(const OUString& name)
            throw (css::uno::RuntimeException, css::xml::dom::DOMException, std::exception) override;

        /**
        Creates an attribute of the given qualified name and namespace URI.
        */
        virtual css::uno::Reference< css::xml::dom::XAttr > SAL_CALL createAttributeNS(const OUString& namespaceURI, const OUString& qualifiedName)
            throw (css::uno::RuntimeException, css::xml::dom::DOMException, std::exception) override;

        /**
        Creates a CDATASection node whose value is the specified string.
        */
        virtual css::uno::Reference< css::xml::dom::XCDATASection > SAL_CALL createCDATASection(const OUString& data)
            throw (css::uno::RuntimeException, std::exception) override;

        /**
        Creates a Comment node given the specified string.
        */
        virtual css::uno::Reference< css::xml::dom::XComment > SAL_CALL createComment(const OUString& data)
            throw (css::uno::RuntimeException, std::exception) override;

        /**
        Creates an empty DocumentFragment object.
        */
        virtual css::uno::Reference< css::xml::dom::XDocumentFragment > SAL_CALL createDocumentFragment()
            throw (css::uno::RuntimeException, std::exception) override;

        /**
        Creates an element of the type specified.
        */
        virtual css::uno::Reference< css::xml::dom::XElement > SAL_CALL createElement(const OUString& tagName)
            throw (css::uno::RuntimeException, css::xml::dom::DOMException, std::exception) override;

        /**
        Creates an element of the given qualified name and namespace URI.
        */
        virtual css::uno::Reference< css::xml::dom::XElement > SAL_CALL createElementNS(const OUString& namespaceURI, const OUString& qualifiedName)
            throw (css::uno::RuntimeException, css::xml::dom::DOMException, std::exception) override;

        /**
        Creates an EntityReference object.
        */
        virtual css::uno::Reference< css::xml::dom::XEntityReference > SAL_CALL createEntityReference(const OUString& name)
            throw (css::uno::RuntimeException, css::xml::dom::DOMException, std::exception) override;

        /**
        Creates a ProcessingInstruction node given the specified name and
        data strings.
        */
        virtual css::uno::Reference< css::xml::dom::XProcessingInstruction > SAL_CALL createProcessingInstruction(
                const OUString& target, const OUString& data)
            throw (css::uno::RuntimeException, css::xml::dom::DOMException, std::exception) override;

        /**
        Creates a Text node given the specified string.
        */
        virtual css::uno::Reference< css::xml::dom::XText > SAL_CALL createTextNode(const OUString& data)
            throw (css::uno::RuntimeException, std::exception) override;

        /**
        The Document Type Declaration (see DocumentType) associated with this
        document.
        */
        virtual css::uno::Reference< css::xml::dom::XDocumentType > SAL_CALL getDoctype()
            throw (css::uno::RuntimeException, std::exception) override;

        /**
        This is a convenience attribute that allows direct access to the child
        node that is the root element of the document.
        */
        virtual css::uno::Reference< css::xml::dom::XElement > SAL_CALL getDocumentElement()
            throw (css::uno::RuntimeException, std::exception) override;

        /**
        Returns the Element whose ID is given by elementId.
        */
        virtual css::uno::Reference< css::xml::dom::XElement > SAL_CALL getElementById(const OUString& elementId)
            throw (css::uno::RuntimeException, std::exception) override;

        /**
        Returns a NodeList of all the Elements with a given tag name in the
        order in which they are encountered in a preorder traversal of the
        Document tree.
        */
        virtual css::uno::Reference< css::xml::dom::XNodeList > SAL_CALL getElementsByTagName(const OUString& tagname)
            throw (css::uno::RuntimeException, std::exception) override;

        /**
        Returns a NodeList of all the Elements with a given local name and
        namespace URI in the order in which they are encountered in a preorder
        traversal of the Document tree.
        */
        virtual css::uno::Reference< css::xml::dom::XNodeList > SAL_CALL getElementsByTagNameNS(const OUString& namespaceURI, const OUString& localName)
            throw (css::uno::RuntimeException, std::exception) override;

        /**
        The DOMImplementation object that handles this document.
        */
        virtual css::uno::Reference< css::xml::dom::XDOMImplementation > SAL_CALL getImplementation()
            throw (css::uno::RuntimeException, std::exception) override;

        /**
        Imports a node from another document to this document.
        */
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL importNode(const css::uno::Reference< css::xml::dom::XNode >& importedNode, sal_Bool deep)
            throw (css::uno::RuntimeException, css::xml::dom::DOMException, std::exception) override;

        // XDocumentEvent
        virtual css::uno::Reference< css::xml::dom::events::XEvent > SAL_CALL createEvent(const OUString& eventType) throw (css::uno::RuntimeException, std::exception) override;

        // XActiveDataControl,
        // see http://api.libreoffice.org/docs/common/ref/com/sun/star/io/XActiveDataControl.html
        virtual void SAL_CALL addListener(const css::uno::Reference< css::io::XStreamListener >& aListener )  throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeListener(const css::uno::Reference< css::io::XStreamListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL start() throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL terminate() throw (css::uno::RuntimeException, std::exception) override;

        // XActiveDataSource
        // see http://api.libreoffice.org/docs/common/ref/com/sun/star/io/XActiveDataSource.html
        virtual void SAL_CALL setOutputStream(  const css::uno::Reference< css::io::XOutputStream >& aStream ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::io::XOutputStream > SAL_CALL getOutputStream() throw (css::uno::RuntimeException, std::exception) override;

        // ---- resolve uno inheritance problems...
        // overrides for XNode base
        virtual OUString SAL_CALL getNodeName()
            throw (css::uno::RuntimeException, std::exception) override;
        virtual OUString SAL_CALL getNodeValue()
            throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL cloneNode(sal_Bool deep)
            throw (css::uno::RuntimeException, std::exception) override;
        // --- delegation for XNode base.
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL appendChild(const css::uno::Reference< css::xml::dom::XNode >& newChild)
            throw (css::uno::RuntimeException, css::xml::dom::DOMException, std::exception) override
        {
            return CNode::appendChild(newChild);
        }
        virtual css::uno::Reference< css::xml::dom::XNamedNodeMap > SAL_CALL getAttributes()
            throw (css::uno::RuntimeException, std::exception) override
        {
            return CNode::getAttributes();
        }
        virtual css::uno::Reference< css::xml::dom::XNodeList > SAL_CALL getChildNodes()
            throw (css::uno::RuntimeException, std::exception) override
        {
            return CNode::getChildNodes();
        }
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL getFirstChild()
            throw (css::uno::RuntimeException, std::exception) override
        {
            return CNode::getFirstChild();
        }
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL getLastChild()
            throw (css::uno::RuntimeException, std::exception) override
        {
            return CNode::getLastChild();
        }
        virtual OUString SAL_CALL getLocalName()
            throw (css::uno::RuntimeException, std::exception) override
        {
            return CNode::getLocalName();
        }
        virtual OUString SAL_CALL getNamespaceURI()
            throw (css::uno::RuntimeException, std::exception) override
        {
            return CNode::getNamespaceURI();
        }
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL getNextSibling()
            throw (css::uno::RuntimeException, std::exception) override
        {
            return CNode::getNextSibling();
        }
        virtual css::xml::dom::NodeType SAL_CALL getNodeType()
            throw (css::uno::RuntimeException, std::exception) override
        {
            return CNode::getNodeType();
        }
        virtual css::uno::Reference< css::xml::dom::XDocument > SAL_CALL getOwnerDocument()
            throw (css::uno::RuntimeException, std::exception) override
        {
            return CNode::getOwnerDocument();
        }
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL getParentNode()
            throw (css::uno::RuntimeException, std::exception) override
        {
            return CNode::getParentNode();
        }
        virtual OUString SAL_CALL getPrefix()
            throw (css::uno::RuntimeException, std::exception) override
        {
           return CNode::getPrefix();
        }
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL getPreviousSibling()
            throw (css::uno::RuntimeException, std::exception) override
        {
            return CNode::getPreviousSibling();
        }
        virtual sal_Bool SAL_CALL hasAttributes()
            throw (css::uno::RuntimeException, std::exception) override
        {
            return CNode::hasAttributes();
        }
        virtual sal_Bool SAL_CALL hasChildNodes()
            throw (css::uno::RuntimeException, std::exception) override
        {
            return CNode::hasChildNodes();
        }
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL insertBefore(
                const css::uno::Reference< css::xml::dom::XNode >& newChild, const css::uno::Reference< css::xml::dom::XNode >& refChild)
            throw (css::uno::RuntimeException, css::xml::dom::DOMException, std::exception) override
        {
            return CNode::insertBefore(newChild, refChild);
        }
        virtual sal_Bool SAL_CALL isSupported(const OUString& feature, const OUString& ver)
            throw (css::uno::RuntimeException, std::exception) override
        {
            return CNode::isSupported(feature, ver);
        }
        virtual void SAL_CALL normalize()
            throw (css::uno::RuntimeException, std::exception) override
        {
            CNode::normalize();
        }
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL removeChild(const css::uno::Reference< css::xml::dom::XNode >& oldChild)
            throw (css::uno::RuntimeException, css::xml::dom::DOMException, std::exception) override
        {
            return CNode::removeChild(oldChild);
        }
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL replaceChild(
                const css::uno::Reference< css::xml::dom::XNode >& newChild, const css::uno::Reference< css::xml::dom::XNode >& oldChild)
            throw (css::uno::RuntimeException, css::xml::dom::DOMException, std::exception) override
        {
            return CNode::replaceChild(newChild, oldChild);
        }
        virtual void SAL_CALL setNodeValue(const OUString& nodeValue)
            throw (css::uno::RuntimeException, css::xml::dom::DOMException, std::exception) override
        {
            return CNode::setNodeValue(nodeValue);
        }
        virtual void SAL_CALL setPrefix(const OUString& prefix)
            throw (css::uno::RuntimeException, css::xml::dom::DOMException, std::exception) override
        {
            return CNode::setPrefix(prefix);
        }

        // css::xml::sax::XSAXSerializable
        virtual void SAL_CALL serialize(
            const css::uno::Reference< css::xml::sax::XDocumentHandler >& i_xHandler,
            const css::uno::Sequence< css::beans::StringPair >& i_rNamespaces)
            throw (css::uno::RuntimeException, css::xml::sax::SAXException, std::exception) override;

        // css::xml::sax::XFastSAXSerializable
        virtual void SAL_CALL fastSerialize( const css::uno::Reference< css::xml::sax::XFastDocumentHandler >& handler,
                                             const css::uno::Reference< css::xml::sax::XFastTokenHandler >& tokenHandler,
                                             const css::uno::Sequence< css::beans::StringPair >& i_rNamespaces,
                                             const css::uno::Sequence< css::beans::Pair< OUString, sal_Int32 > >& namespaces )
            throw (css::xml::sax::SAXException, css::uno::RuntimeException, std::exception) override;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
