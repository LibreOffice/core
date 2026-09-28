/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <memory>
#include <unordered_map>

#include <libxml/tree.h>

#include <sal/types.h>

#include <cppuhelper/implbase.hxx>
#include <unotools/weakref.hxx>

#include <cpo/uno/Reference.h>
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
#include <o3tl/sorted_vector.hxx>

#include <node.hxx>

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
        typedef o3tl::sorted_vector< cpo::uno::Reference< css::io::XStreamListener > > listenerlist_t;
        listenerlist_t m_streamListeners;
        cpo::uno::Reference< css::io::XOutputStream > m_rOutputStream;

        typedef std::unordered_map< xmlNodePtr,
                    ::std::pair< unotools::WeakReference<CNode>, CNode* > > nodemap_t;
        nodemap_t m_NodeMap;

        ::std::unique_ptr<events::CEventDispatcher> const m_pEventDispatcher;

        explicit CDocument(xmlDocPtr const pDocPtr);


    public:
        /// factory: only way to create instance!
        static ::rtl::Reference<CDocument>
            CreateCDocument(xmlDocPtr const pDoc);

        virtual ~CDocument() override;

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

        virtual void saxify(const cpo::uno::Reference< css::xml::sax::XDocumentHandler >& i_xHandler) override;

        virtual void fastSaxify( Context& rContext ) override;

        virtual bool IsChildTypeAllowed(css::xml::dom::NodeType const nodeType,
                css::xml::dom::NodeType const* pReplacedNodeType) override;

        /**
        Creates an Attr of the given name.
        */
        virtual cpo::uno::Reference< css::xml::dom::XAttr > createAttribute(const OUString& name) override;

        /**
        Creates an attribute of the given qualified name and namespace URI.
        */
        virtual cpo::uno::Reference< css::xml::dom::XAttr > createAttributeNS(const OUString& namespaceURI, const OUString& qualifiedName) override;

        /**
        Creates a CDATASection node whose value is the specified string.
        */
        virtual cpo::uno::Reference< css::xml::dom::XCDATASection > createCDATASection(const OUString& data) override;

        /**
        Creates a Comment node given the specified string.
        */
        virtual cpo::uno::Reference< css::xml::dom::XComment > createComment(const OUString& data) override;

        /**
        Creates an empty DocumentFragment object.
        */
        virtual cpo::uno::Reference< css::xml::dom::XDocumentFragment > createDocumentFragment() override;

        /**
        Creates an element of the type specified.
        */
        virtual cpo::uno::Reference< css::xml::dom::XElement > createElement(const OUString& tagName) override;

        /**
        Creates an element of the given qualified name and namespace URI.
        */
        virtual cpo::uno::Reference< css::xml::dom::XElement > createElementNS(const OUString& namespaceURI, const OUString& qualifiedName) override;

        /**
        Creates an EntityReference object.
        */
        virtual cpo::uno::Reference< css::xml::dom::XEntityReference > createEntityReference(const OUString& name) override;

        /**
        Creates a ProcessingInstruction node given the specified name and
        data strings.
        */
        virtual cpo::uno::Reference< css::xml::dom::XProcessingInstruction > createProcessingInstruction(
                const OUString& target, const OUString& data) override;

        /**
        Creates a Text node given the specified string.
        */
        virtual cpo::uno::Reference< css::xml::dom::XText > createTextNode(const OUString& data) override;

        /**
        The Document Type Declaration (see DocumentType) associated with this
        document.
        */
        virtual cpo::uno::Reference< css::xml::dom::XDocumentType > getDoctype() override;

        /**
        This is a convenience attribute that allows direct access to the child
        node that is the root element of the document.
        */
        virtual cpo::uno::Reference< css::xml::dom::XElement > getDocumentElement() override;

        /**
        Returns the Element whose ID is given by elementId.
        */
        virtual cpo::uno::Reference< css::xml::dom::XElement > getElementById(const OUString& elementId) override;

        /**
        Returns a NodeList of all the Elements with a given tag name in the
        order in which they are encountered in a preorder traversal of the
        Document tree.
        */
        virtual cpo::uno::Reference< css::xml::dom::XNodeList > getElementsByTagName(const OUString& tagname) override;

        /**
        Returns a NodeList of all the Elements with a given local name and
        namespace URI in the order in which they are encountered in a preorder
        traversal of the Document tree.
        */
        virtual cpo::uno::Reference< css::xml::dom::XNodeList > getElementsByTagNameNS(const OUString& namespaceURI, const OUString& localName) override;

        /**
        The DOMImplementation object that handles this document.
        */
        virtual cpo::uno::Reference< css::xml::dom::XDOMImplementation > getImplementation() override;

        /**
        Imports a node from another document to this document.
        */
        virtual cpo::uno::Reference< css::xml::dom::XNode > importNode(const cpo::uno::Reference< css::xml::dom::XNode >& importedNode, bool deep) override;

        // XDocumentEvent
        virtual cpo::uno::Reference< css::xml::dom::events::XEvent > createEvent(const OUString& eventType) override;

        // XActiveDataControl,
        // see https://api.libreoffice.org/docs/common/ref/com/sun/star/io/XActiveDataControl.html
        virtual void addListener(const cpo::uno::Reference< css::io::XStreamListener >& aListener ) override;
        virtual void removeListener(const cpo::uno::Reference< css::io::XStreamListener >& aListener ) override;
        virtual void start() override;
        virtual void terminate() override;

        // XActiveDataSource
        // see https://api.libreoffice.org/docs/common/ref/com/sun/star/io/XActiveDataSource.html
        virtual void setOutputStream(  const cpo::uno::Reference< css::io::XOutputStream >& aStream ) override;
        virtual cpo::uno::Reference< css::io::XOutputStream > getOutputStream() override;

        // ---- resolve uno inheritance problems...
        // overrides for XNode base
        virtual OUString getNodeName() override;
        virtual OUString getNodeValue() override;
        virtual cpo::uno::Reference< css::xml::dom::XNode > cloneNode(bool deep) override;
        // --- delegation for XNode base.
        virtual cpo::uno::Reference< css::xml::dom::XNode > appendChild(const cpo::uno::Reference< css::xml::dom::XNode >& newChild) override
        {
            return CNode::appendChild(newChild);
        }
        virtual cpo::uno::Reference< css::xml::dom::XNamedNodeMap > getAttributes() override
        {
            return CNode::getAttributes();
        }
        virtual cpo::uno::Reference< css::xml::dom::XNodeList > getChildNodes() override
        {
            return CNode::getChildNodes();
        }
        virtual cpo::uno::Reference< css::xml::dom::XNode > getFirstChild() override
        {
            return CNode::getFirstChild();
        }
        virtual cpo::uno::Reference< css::xml::dom::XNode > getLastChild() override
        {
            return CNode::getLastChild();
        }
        virtual OUString getLocalName() override
        {
            return CNode::getLocalName();
        }
        virtual OUString getNamespaceURI() override
        {
            return CNode::getNamespaceURI();
        }
        virtual cpo::uno::Reference< css::xml::dom::XNode > getNextSibling() override
        {
            return CNode::getNextSibling();
        }
        virtual css::xml::dom::NodeType getNodeType() override
        {
            return CNode::getNodeType();
        }
        virtual cpo::uno::Reference< css::xml::dom::XDocument > getOwnerDocument() override
        {
            return CNode::getOwnerDocument();
        }
        virtual cpo::uno::Reference< css::xml::dom::XNode > getParentNode() override
        {
            return CNode::getParentNode();
        }
        virtual OUString getPrefix() override
        {
           return CNode::getPrefix();
        }
        virtual cpo::uno::Reference< css::xml::dom::XNode > getPreviousSibling() override
        {
            return CNode::getPreviousSibling();
        }
        virtual bool hasAttributes() override
        {
            return CNode::hasAttributes();
        }
        virtual bool hasChildNodes() override
        {
            return CNode::hasChildNodes();
        }
        virtual cpo::uno::Reference< css::xml::dom::XNode > insertBefore(
                const cpo::uno::Reference< css::xml::dom::XNode >& newChild, const cpo::uno::Reference< css::xml::dom::XNode >& refChild) override
        {
            return CNode::insertBefore(newChild, refChild);
        }
        virtual bool isSupported(const OUString& feature, const OUString& ver) override
        {
            return CNode::isSupported(feature, ver);
        }
        virtual void normalize() override
        {
            CNode::normalize();
        }
        virtual cpo::uno::Reference< css::xml::dom::XNode > removeChild(const cpo::uno::Reference< css::xml::dom::XNode >& oldChild) override
        {
            return CNode::removeChild(oldChild);
        }
        virtual cpo::uno::Reference< css::xml::dom::XNode > replaceChild(
                const cpo::uno::Reference< css::xml::dom::XNode >& newChild, const cpo::uno::Reference< css::xml::dom::XNode >& oldChild) override
        {
            return CNode::replaceChild(newChild, oldChild);
        }
        virtual void setNodeValue(const OUString& nodeValue) override
        {
            return CNode::setNodeValue(nodeValue);
        }
        virtual void setPrefix(const OUString& prefix) override
        {
            return CNode::setPrefix(prefix);
        }

        // css::xml::sax::XSAXSerializable
        virtual void serialize(
            const cpo::uno::Reference< css::xml::sax::XDocumentHandler >& i_xHandler,
            const cpo::uno::Sequence< css::beans::StringPair >& i_rNamespaces) override;

        // css::xml::sax::XFastSAXSerializable
        virtual void fastSerialize( const cpo::uno::Reference< css::xml::sax::XFastDocumentHandler >& handler,
                                             const cpo::uno::Reference< css::xml::sax::XFastTokenHandler >& tokenHandler,
                                             const cpo::uno::Sequence< css::beans::StringPair >& i_rNamespaces,
                                             const cpo::uno::Sequence< css::beans::Pair< OUString, sal_Int32 > >& namespaces ) override;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
