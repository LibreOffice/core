/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _DOCUMENT_HXX
#define _DOCUMENT_HXX

#include <list>
#include <set>
#include <sal/types.h>
#include <cppuhelper/implbase6.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Exception.hpp>
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

#include <libxml/tree.h>

using namespace std;
using ::rtl::OUString;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::xml::sax;
using namespace com::sun::star::io;
using namespace com::sun::star::xml::dom;
using namespace com::sun::star::xml::dom::events;

namespace DOM
{

    class CDocument : public cppu::ImplInheritanceHelper6<
        CNode, XDocument, XDocumentEvent,
        XActiveDataControl, XActiveDataSource, XSAXSerializable, XFastSAXSerializable>
    {
        friend class CNode;
        typedef set< Reference< XStreamListener > > listenerlist_t;
    private:

        xmlDocPtr m_aDocPtr;

        // datacontrol/source state
        listenerlist_t m_streamListeners;
        Reference< XOutputStream > m_rOutputStream;

    protected:
        CDocument(xmlDocPtr aDocPtr);

    public:

        virtual ~CDocument();

        virtual void SAL_CALL saxify(
            const Reference< XDocumentHandler >& i_xHandler);

        virtual void SAL_CALL fastSaxify( Context& rContext );

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
        // see http://api.openoffice.org/docs/common/ref/com/sun/star/io/XActiveDataControl.html
        virtual void SAL_CALL addListener(const Reference< XStreamListener >& aListener )  throw (RuntimeException);
        virtual void SAL_CALL removeListener(const Reference< XStreamListener >& aListener ) throw (RuntimeException);
        virtual void SAL_CALL start() throw (RuntimeException);
        virtual void SAL_CALL terminate() throw (RuntimeException);

        // XActiveDataSource
        // see http://api.openoffice.org/docs/common/ref/com/sun/star/io/XActiveDataSource.html
        virtual void SAL_CALL setOutputStream( 	const Reference< XOutputStream >& aStream ) throw (RuntimeException);
        virtual Reference< XOutputStream > SAL_CALL getOutputStream() throw (RuntimeException);

        // ---- resolve uno inheritance problems...
        // overrides for XNode base
        virtual OUString SAL_CALL getNodeName()
            throw (RuntimeException);
        virtual OUString SAL_CALL getNodeValue()
            throw (RuntimeException);
        // --- delegation for XNde base.
        virtual Reference< XNode > SAL_CALL appendChild(const Reference< XNode >& newChild)
            throw (RuntimeException, DOMException)
        {
            return CNode::appendChild(newChild);
        }
        virtual Reference< XNode > SAL_CALL cloneNode(sal_Bool deep)
            throw (RuntimeException)
        {
            return CNode::cloneNode(deep);
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
            throw (DOMException)
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
                                             const Sequence< beans::Pair< rtl::OUString, sal_Int32 > >& namespaces ) 
            throw (SAXException, RuntimeException);
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
