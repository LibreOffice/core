/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: node.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 14:49:24 $
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

#ifndef _NODE_HXX
#define _NODE_HXX

#include <map>
#include <sal/types.h>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase3.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XNodeList.hpp>
#include <com/sun/star/xml/dom/XNamedNodeMap.hpp>
#include <com/sun/star/xml/dom/NodeType.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/xml/dom/events/XEventTarget.hpp>
#include <com/sun/star/xml/dom/events/XDocumentEvent.hpp>
#include <com/sun/star/xml/dom/events/XEvent.hpp>
#include <com/sun/star/xml/dom/events/XMutationEvent.hpp>
#include <com/sun/star/xml/dom/events/XUIEvent.hpp>
#include <com/sun/star/xml/dom/events/XMouseEvent.hpp>
#include <com/sun/star/xml/dom/DOMException.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <libxml/tree.h>

using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::xml::sax;
using namespace com::sun::star::xml::dom;
using namespace com::sun::star::xml::dom::events;
using com::sun::star::lang::XUnoTunnel;

namespace DOM
{
    class CNode;
    typedef std::map< const xmlNodePtr, CNode* > nodemap_t;


    class CNode : public cppu::WeakImplHelper3< XNode, XUnoTunnel, XEventTarget >
    {
        friend class CDocument;
        friend class CElement;
        friend class CAttributesMap;
        friend class CChildList;
        friend class CElementList;
        friend class CEntitiesMap;
        friend class CNotationsMap;
    private:
        static nodemap_t theNodeMap;

    protected:
        NodeType m_aNodeType;
        xmlNodePtr m_aNodePtr;

        Reference< XDocument > m_rDocument;

        // for initialization by classes derived through ImplInheritanceHelper
        CNode();
        void init_node(const xmlNodePtr aNode);

        void dispatchSubtreeModified();

    public:

        virtual ~CNode();

        // get a representaion for a libxml node
        static CNode* get(const xmlNodePtr aNode, sal_Bool bCreate = sal_True);
        // remove a wrapper instance
        static void remove(const xmlNodePtr aNode);

        // get the libxml node implementation
        static xmlNodePtr getNodePtr(const Reference< XNode >& aNode);

        //static Sequence< sal_Int8 >

        // recursively create SAX events
        virtual void SAL_CALL saxify(
            const Reference< XDocumentHandler >& i_xHandler);

        // ---- DOM interfaces

        /**
        Adds the node newChild to the end of the list of children of this node.
        */
        virtual Reference< XNode > SAL_CALL appendChild(const Reference< XNode >& newChild)
            throw (RuntimeException, DOMException);

        /**
        Returns a duplicate of this node, i.e., serves as a generic copy
        constructor for nodes.
        */
        virtual Reference< XNode > SAL_CALL cloneNode(sal_Bool deep)
            throw (RuntimeException);

        /**
        A NamedNodeMap containing the attributes of this node (if it is an Element)
        or null otherwise.
        */
        virtual Reference< XNamedNodeMap > SAL_CALL getAttributes()
            throw (RuntimeException);

        /**
        A NodeList that contains all children of this node.
        */
        virtual Reference< XNodeList > SAL_CALL getChildNodes()
            throw (RuntimeException);

        /**
        The first child of this node.
        */
        virtual Reference< XNode > SAL_CALL getFirstChild()
            throw (RuntimeException);

        /**
        The last child of this node.
        */
        virtual Reference< XNode > SAL_CALL getLastChild()
            throw (RuntimeException);

        /**
        Returns the local part of the qualified name of this node.
        */
        virtual OUString SAL_CALL getLocalName()
            throw (RuntimeException);

        /**
        The namespace URI of this node, or null if it is unspecified.
        */
        virtual OUString SAL_CALL getNamespaceURI()
            throw (RuntimeException);

        /**
        The node immediately following this node.
        */
        virtual Reference< XNode > SAL_CALL getNextSibling()
            throw (RuntimeException);

        /**
        The name of this node, depending on its type; see the table above.
        -- virtual implemented by actual node types
        */
        virtual OUString SAL_CALL getNodeName()
            throw (RuntimeException);

        /**
        A code representing the type of the underlying object, as defined above.
        */
        virtual NodeType SAL_CALL getNodeType()
            throw (RuntimeException);

        /**
        The value of this node, depending on its type; see the table above.
        -- virtual implemented by actual node types
        */
        virtual OUString SAL_CALL getNodeValue()
            throw (RuntimeException);

        /**
        The Document object associated with this node.
        */
        virtual Reference< XDocument > SAL_CALL getOwnerDocument()
            throw (RuntimeException);

        /**
        The parent of this node.
        */
        virtual Reference< XNode > SAL_CALL getParentNode()
            throw (RuntimeException);

        /**
        The namespace prefix of this node, or null if it is unspecified.
        */
        virtual OUString SAL_CALL getPrefix()
            throw (RuntimeException);

        /**
        The node immediately preceding this node.
        */
        virtual Reference< XNode > SAL_CALL getPreviousSibling()
            throw (RuntimeException);

        /**
        Returns whether this node (if it is an element) has any attributes.
        */
        virtual sal_Bool SAL_CALL hasAttributes()
            throw (RuntimeException);

        /**
        Returns whether this node has any children.
        */
        virtual sal_Bool SAL_CALL hasChildNodes()
            throw (RuntimeException);

        /**
        Inserts the node newChild before the existing child node refChild.
        */
        virtual Reference< XNode > SAL_CALL insertBefore(
                const Reference< XNode >& newChild, const Reference< XNode >& refChild)
            throw (RuntimeException, DOMException);

        /**
        Tests whether the DOM implementation implements a specific feature and
        that feature is supported by this node.
        */
        virtual sal_Bool SAL_CALL isSupported(const OUString& feature, const OUString& ver)
            throw (RuntimeException);

        /**
        Puts all Text nodes in the full depth of the sub-tree underneath this
        Node, including attribute nodes, into a "normal" form where only structure
        (e.g., elements, comments, processing instructions, CDATA sections, and
        entity references) separates Text nodes, i.e., there are neither adjacent
        Text nodes nor empty Text nodes.
        */
        virtual void SAL_CALL normalize()
            throw (RuntimeException);

        /**
        Removes the child node indicated by oldChild from the list of children,
        and returns it.
        */
        virtual Reference< XNode > SAL_CALL removeChild(const Reference< XNode >& oldChild)
            throw (RuntimeException, DOMException);

        /**
        Replaces the child node oldChild with newChild in the list of children,
        and returns the oldChild node.
        */
        virtual Reference< XNode > SAL_CALL replaceChild(
                const Reference< XNode >& newChild, const Reference< XNode >& oldChild)
            throw (RuntimeException, DOMException);

        /**
        The value of this node, depending on its type; see the table above.
        */
        virtual void SAL_CALL setNodeValue(const OUString& nodeValue)
            throw (RuntimeException, DOMException);

        /**
        The namespace prefix of this node, or null if it is unspecified.
        */
        virtual void SAL_CALL setPrefix(const OUString& prefix)
            throw (RuntimeException, DOMException);


        // --- XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething(const Sequence< sal_Int8 >& id) throw (RuntimeException);

        // --- XEventTarget
        virtual void SAL_CALL addEventListener(const OUString& eventType,
            const Reference< XEventListener >& listener,
            sal_Bool useCapture)
            throw (RuntimeException);

        virtual void SAL_CALL removeEventListener(const OUString& eventType,
            const Reference< XEventListener >& listener,
            sal_Bool useCapture)
            throw (RuntimeException);

        virtual sal_Bool SAL_CALL dispatchEvent(const Reference< XEvent >& evt)
            throw(RuntimeException, EventException);

    };

    /// eliminate redundant namespace declarations
    void _nscleanup(const xmlNodePtr aNode, const xmlNodePtr aParent);
}

#endif
