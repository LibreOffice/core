/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: element.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 14:48:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _ELEMENT_HXX
#define _ELEMENT_HXX
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XNodeList.hpp>
#include <com/sun/star/xml/dom/XNamedNodeMap.hpp>
#include <com/sun/star/xml/dom/NodeType.hpp>
#include <libxml/tree.h>
#include "node.hxx"

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::xml::dom;

namespace DOM
{
    class CElement : public cppu::ImplInheritanceHelper1<CNode, XElement >
    {
        friend class CNode;
    private:
        Reference< XAttr > _setAttributeNode(const Reference< XAttr >& newAttr, sal_Bool bNS)
            throw (RuntimeException);

    protected:
        CElement(const xmlNodePtr aNodePtr);

    public:

        virtual void SAL_CALL saxify(
            const Reference< XDocumentHandler >& i_xHandler);

        /**
        Retrieves an attribute value by name.
        */
        virtual OUString  SAL_CALL getAttribute(const OUString& name)
            throw (RuntimeException);

        /**
        Retrieves an attribute node by name.
        */
        virtual Reference< XAttr > SAL_CALL getAttributeNode(const OUString& name)
            throw (RuntimeException);

        /**
        Retrieves an Attr node by local name and namespace URI.
        */
        virtual Reference< XAttr > SAL_CALL getAttributeNodeNS(const OUString& namespaceURI, const OUString& localName)
            throw (RuntimeException);

        /**
        Retrieves an attribute value by local name and namespace URI.
        */
        virtual OUString SAL_CALL getAttributeNS(const OUString& namespaceURI, const OUString& localName)
            throw (RuntimeException);

        /**
        Returns a NodeList of all descendant Elements with a given tag name,
        in the order in which they are
        encountered in a preorder traversal of this Element tree.
        */
        virtual Reference< XNodeList > SAL_CALL getElementsByTagName(const OUString& name)
            throw (RuntimeException);

        /**
        Returns a NodeList of all the descendant Elements with a given local
        name and namespace URI in the order in which they are encountered in
        a preorder traversal of this Element tree.
        */
        virtual Reference< XNodeList > SAL_CALL getElementsByTagNameNS(const OUString& namespaceURI,
                const OUString& localName)
            throw (RuntimeException);

        /**
        The name of the element.
        */
        virtual OUString SAL_CALL getTagName()
            throw (RuntimeException);

        /**
        Returns true when an attribute with a given name is specified on this
        element or has a default value, false otherwise.
        */
        virtual sal_Bool SAL_CALL hasAttribute(const OUString& name)
            throw (RuntimeException);

        /**
        Returns true when an attribute with a given local name and namespace
        URI is specified on this element or has a default value, false otherwise.
        */
        virtual sal_Bool SAL_CALL hasAttributeNS(const OUString& namespaceURI, const OUString& localName)
            throw (RuntimeException);

        /**
        Removes an attribute by name.
        */
        virtual void SAL_CALL removeAttribute(const OUString& name)
            throw (RuntimeException, DOMException);

        /**
        Removes the specified attribute node.
        */
        virtual Reference< XAttr > SAL_CALL removeAttributeNode(const Reference< XAttr >& oldAttr)
            throw (RuntimeException, DOMException);

        /**
        Removes an attribute by local name and namespace URI.
        */
        virtual void SAL_CALL removeAttributeNS(const OUString& namespaceURI, const OUString& localName)
            throw (RuntimeException, DOMException);

        /**
        Adds a new attribute.
        */
        virtual void SAL_CALL setAttribute(const OUString& name, const OUString& value)
            throw (RuntimeException, DOMException);

        /**
        Adds a new attribute node.
        */
        virtual Reference< XAttr > SAL_CALL setAttributeNode(const Reference< XAttr >& newAttr)
            throw (RuntimeException, DOMException);

        /**
        Adds a new attribute.
        */
        virtual Reference< XAttr > SAL_CALL setAttributeNodeNS(const Reference< XAttr >& newAttr)
            throw (RuntimeException, DOMException);

        /**
        Adds a new attribute.
        */
        virtual void SAL_CALL setAttributeNS(
                const OUString& namespaceURI, const OUString& qualifiedName, const OUString& value)
            throw (RuntimeException, DOMException);

        /**
        sets the element name
        */
        virtual void SAL_CALL setElementName(const OUString& elementName)
            throw (RuntimeException, DOMException);

        // overrides for XNode base
        virtual OUString SAL_CALL getNodeName()
            throw (RuntimeException);
        virtual OUString SAL_CALL getNodeValue()
            throw (RuntimeException);
        virtual Reference< XNamedNodeMap > SAL_CALL getAttributes()
            throw (RuntimeException);
        virtual OUString SAL_CALL getLocalName()
            throw (RuntimeException);

        // resolve uno inheritance problems...
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

    };

}

#endif
