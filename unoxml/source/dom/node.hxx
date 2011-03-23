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

#ifndef DOM_NODE_HXX
#define DOM_NODE_HXX

#include <hash_map>

#include <libxml/tree.h>

#include <sal/types.h>
#include <rtl/ref.hxx>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>

#include <cppuhelper/implbase3.hxx>

#include <sax/fastattribs.hxx>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/xml/dom/XNode.hpp>
#include <com/sun/star/xml/dom/XNodeList.hpp>
#include <com/sun/star/xml/dom/XNamedNodeMap.hpp>
#include <com/sun/star/xml/dom/NodeType.hpp>
#include <com/sun/star/xml/dom/events/XEventTarget.hpp>
#include <com/sun/star/xml/dom/events/XEvent.hpp>
#include <com/sun/star/xml/dom/DOMException.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XFastDocumentHandler.hpp>

#include <boost/unordered_map.hpp>

using ::rtl::OUString;
using ::rtl::OString;
using namespace sax_fastparser;
using namespace com::sun::star::uno;
using namespace com::sun::star::xml::sax;
using namespace com::sun::star::xml::dom;
using namespace com::sun::star::xml::dom::events;
using com::sun::star::lang::XUnoTunnel;


namespace DOM
{
    struct Context
    {
        Context( const Reference< XFastDocumentHandler >& i_xHandler,
                 const Reference< XFastTokenHandler >& i_xTokenHandler ) :
            maNamespaces( 1, std::vector<Namespace>() ),
            maNamespaceMap(101),
            mxAttribList(new FastAttributeList(i_xTokenHandler)),
            mxCurrentHandler(i_xHandler, UNO_QUERY_THROW),
            mxDocHandler(i_xHandler),
            mxTokenHandler(i_xTokenHandler)
        {}

        struct Namespace
        {
            OString     maPrefix;
            sal_Int32   mnToken;
            OUString    maNamespaceURL;

            const OString& getPrefix() const { return maPrefix; }
        };

        typedef std::vector< std::vector<Namespace> > NamespaceVectorType;
        typedef boost::unordered_map< OUString,
                               sal_Int32,
                               rtl::OUStringHash > NamespaceMapType;

        /// outer vector: xml context; inner vector: current NS
        NamespaceVectorType                 maNamespaces;
        NamespaceMapType                    maNamespaceMap;
        ::rtl::Reference<FastAttributeList> mxAttribList;
        Reference<XFastContextHandler>      mxCurrentHandler;
        Reference<XFastDocumentHandler>     mxDocHandler;
        Reference<XFastTokenHandler>        mxTokenHandler;
    };

    void pushContext(Context& io_rContext);
    void popContext(Context& io_rContext);

    sal_Int32 getTokenWithPrefix( const Context& rContext, const sal_Char* xPrefix, const sal_Char* xName );
    sal_Int32 getToken( const Context& rContext, const sal_Char* xName );

    /// add namespaces on this node to context
    void addNamespaces(Context& io_rContext, xmlNodePtr pNode);

    class CDocument;

    class CNode : public cppu::WeakImplHelper3< XNode, XUnoTunnel, XEventTarget >
    {
        friend class CDocument;
        friend class CElement;
        friend class CAttributesMap;

    private:
        bool m_bUnlinked; /// node has been removed from document

    protected:
        NodeType const m_aNodeType;
        /// libxml node; NB: not const, because invalidate may reset it to 0!
        xmlNodePtr m_aNodePtr;

        ::rtl::Reference< CDocument > const m_xDocument;
        ::osl::Mutex & m_rMutex;

        // for initialization by classes derived through ImplInheritanceHelper
        CNode(CDocument const& rDocument, ::osl::Mutex const& rMutex,
                NodeType const& reNodeType, xmlNodePtr const& rpNode);
        void invalidate();

        void dispatchSubtreeModified();

    public:

        virtual ~CNode();

        static CNode * GetImplementation(::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XInterface> const& xNode);

        xmlNodePtr GetNodePtr() { return m_aNodePtr; }

        virtual CDocument & GetOwnerDocument();

        // recursively create SAX events
        virtual void saxify(const Reference< XDocumentHandler >& i_xHandler);

        // recursively create SAX events
        virtual void fastSaxify( Context& io_rContext );

        // constrains child relationship between nodes based on type
        virtual bool IsChildTypeAllowed(NodeType const nodeType);

        // ---- DOM interfaces

        /**
        Adds the node newChild to the end of the list of children of this node.
        */
        virtual Reference< XNode > SAL_CALL
            appendChild(Reference< XNode > const& xNewChild)
            throw (RuntimeException, DOMException);

        /**
        Returns a duplicate of this node, i.e., serves as a generic copy
        constructor for nodes.
        */
        virtual Reference< XNode > SAL_CALL cloneNode(sal_Bool deep)
            throw (RuntimeException);

        /**
        A NamedNodeMap containing the attributes of this node
        (if it is an Element) or null otherwise.
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

        // --- XUnoTunnel
        virtual ::sal_Int64 SAL_CALL
            getSomething(Sequence< ::sal_Int8 > const& rId)
            throw (RuntimeException);
    };

    /// eliminate redundant namespace declarations
    void nscleanup(const xmlNodePtr aNode, const xmlNodePtr aParent);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
