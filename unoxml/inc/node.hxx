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

#ifndef INCLUDED_UNOXML_INC_NODE_HXX
#define INCLUDED_UNOXML_INC_NODE_HXX

#include <libxml/tree.h>

#include <sal/types.h>
#include <rtl/ref.hxx>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>

#include <cppuhelper/implbase.hxx>

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

#include <unordered_map>

namespace DOM
{
    struct Context
    {
        Context( const css::uno::Reference< css::xml::sax::XFastDocumentHandler >& i_xHandler,
                 const css::uno::Reference< css::xml::sax::XFastTokenHandler >& i_xTokenHandler ) :
            maNamespaces( 1, std::vector<Namespace>() ),
            maNamespaceMap(101),
            mxAttribList(new sax_fastparser::FastAttributeList(i_xTokenHandler)),
            mxCurrentHandler(i_xHandler),
            mxDocHandler(i_xHandler),
            mxTokenHandler(i_xTokenHandler)
        {}

        struct Namespace
        {
            OString     maPrefix;
            sal_Int32   mnToken;

            const OString& getPrefix() const { return maPrefix; }
        };

        typedef std::vector< std::vector<Namespace> > NamespaceVectorType;
        typedef std::unordered_map< OUString, sal_Int32 > NamespaceMapType;

        /// outer vector: xml context; inner vector: current NS
        NamespaceVectorType                 maNamespaces;
        NamespaceMapType                    maNamespaceMap;
        ::rtl::Reference<sax_fastparser::FastAttributeList> mxAttribList;
        css::uno::Reference<css::xml::sax::XFastContextHandler>      mxCurrentHandler;
        css::uno::Reference<css::xml::sax::XFastDocumentHandler>     mxDocHandler;
        css::uno::Reference<css::xml::sax::XFastTokenHandler>        mxTokenHandler;
    };

    void pushContext(Context& io_rContext);
    void popContext(Context& io_rContext);

    sal_Int32 getTokenWithPrefix( const Context& rContext, const sal_Char* xPrefix, const sal_Char* xName );
    sal_Int32 getToken( const Context& rContext, const sal_Char* xName );

    /// add namespaces on this node to context
    void addNamespaces(Context& io_rContext, xmlNodePtr pNode);

    class CDocument;

    class CNode : public cppu::WeakImplHelper< css::xml::dom::XNode, css::lang::XUnoTunnel, css::xml::dom::events::XEventTarget >
    {
        friend class CDocument;
        friend class CElement;
        friend class CAttributesMap;

    private:
        bool m_bUnlinked; /// node has been removed from document

    protected:
        css::xml::dom::NodeType const m_aNodeType;
        /// libxml node; NB: not const, because invalidate may reset it to 0!
        xmlNodePtr m_aNodePtr;

        ::rtl::Reference< CDocument > const m_xDocument;
        ::osl::Mutex & m_rMutex;

        // for initialization by classes derived through ImplInheritanceHelper
        CNode(CDocument const& rDocument, ::osl::Mutex const& rMutex,
                css::xml::dom::NodeType const& reNodeType, xmlNodePtr const& rpNode);
        void invalidate();

        void dispatchSubtreeModified();

    public:

        virtual ~CNode() override;

        static CNode * GetImplementation(css::uno::Reference<
                css::uno::XInterface> const& xNode);

        xmlNodePtr GetNodePtr() { return m_aNodePtr; }

        virtual CDocument & GetOwnerDocument();

        // recursively create SAX events
        virtual void saxify(const css::uno::Reference< css::xml::sax::XDocumentHandler >& i_xHandler);

        // recursively create SAX events
        virtual void fastSaxify( Context& io_rContext );

        // constrains child relationship between nodes based on type
        virtual bool IsChildTypeAllowed(css::xml::dom::NodeType const nodeType);

        // ---- DOM interfaces

        /**
        Adds the node newChild to the end of the list of children of this node.
        */
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL
            appendChild(css::uno::Reference< css::xml::dom::XNode > const& xNewChild) override;

        /**
        Returns a duplicate of this node, i.e., serves as a generic copy
        constructor for nodes.
        */
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL cloneNode(sal_Bool deep) override;

        /**
        A NamedNodeMap containing the attributes of this node
        (if it is an Element) or null otherwise.
        */
        virtual css::uno::Reference< css::xml::dom::XNamedNodeMap > SAL_CALL getAttributes() override;

        /**
        A NodeList that contains all children of this node.
        */
        virtual css::uno::Reference< css::xml::dom::XNodeList > SAL_CALL getChildNodes() override;

        /**
        The first child of this node.
        */
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL getFirstChild() override;

        /**
        The last child of this node.
        */
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL getLastChild() override;

        /**
        Returns the local part of the qualified name of this node.
        */
        virtual OUString SAL_CALL getLocalName() override;

        /**
        The namespace URI of this node, or null if it is unspecified.
        */
        virtual OUString SAL_CALL getNamespaceURI() override;

        /**
        The node immediately following this node.
        */
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL getNextSibling() override;

        /**
        The name of this node, depending on its type; see the table above.
        -- virtual implemented by actual node types
        */
        virtual OUString SAL_CALL getNodeName() override;

        /**
        A code representing the type of the underlying object, as defined above.
        */
        virtual css::xml::dom::NodeType SAL_CALL getNodeType() override;

        /**
        The value of this node, depending on its type; see the table above.
        -- virtual implemented by actual node types
        */
        virtual OUString SAL_CALL getNodeValue() override;

        /**
        The Document object associated with this node.
        */
        virtual css::uno::Reference< css::xml::dom::XDocument > SAL_CALL getOwnerDocument() override;

        /**
        The parent of this node.
        */
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL getParentNode() override;

        /**
        The namespace prefix of this node, or null if it is unspecified.
        */
        virtual OUString SAL_CALL getPrefix() override;

        /**
        The node immediately preceding this node.
        */
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL getPreviousSibling() override;

        /**
        Returns whether this node (if it is an element) has any attributes.
        */
        virtual sal_Bool SAL_CALL hasAttributes() override;

        /**
        Returns whether this node has any children.
        */
        virtual sal_Bool SAL_CALL hasChildNodes() override;

        /**
        Inserts the node newChild before the existing child node refChild.
        */
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL insertBefore(
                const css::uno::Reference< css::xml::dom::XNode >& newChild, const css::uno::Reference< css::xml::dom::XNode >& refChild) override;

        /**
        Tests whether the DOM implementation implements a specific feature and
        that feature is supported by this node.
        */
        virtual sal_Bool SAL_CALL isSupported(const OUString& feature, const OUString& ver) override;

        /**
        Puts all Text nodes in the full depth of the sub-tree underneath this
        Node, including attribute nodes, into a "normal" form where only structure
        (e.g., elements, comments, processing instructions, CDATA sections, and
        entity references) separates Text nodes, i.e., there are neither adjacent
        Text nodes nor empty Text nodes.
        */
        virtual void SAL_CALL normalize() override;

        /**
        Removes the child node indicated by oldChild from the list of children,
        and returns it.
        */
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL removeChild(const css::uno::Reference< css::xml::dom::XNode >& oldChild) override;

        /**
        Replaces the child node oldChild with newChild in the list of children,
        and returns the oldChild node.
        */
        virtual css::uno::Reference< css::xml::dom::XNode > SAL_CALL replaceChild(
                const css::uno::Reference< css::xml::dom::XNode >& newChild, const css::uno::Reference< css::xml::dom::XNode >& oldChild) override;

        /**
        The value of this node, depending on its type; see the table above.
        */
        virtual void SAL_CALL setNodeValue(const OUString& nodeValue) override;

        /**
        The namespace prefix of this node, or null if it is unspecified.
        */
        virtual void SAL_CALL setPrefix(const OUString& prefix) override;


        // --- XEventTarget
        virtual void SAL_CALL addEventListener(const OUString& eventType,
            const css::uno::Reference< css::xml::dom::events::XEventListener >& listener,
            sal_Bool useCapture) override;

        virtual void SAL_CALL removeEventListener(const OUString& eventType,
            const css::uno::Reference< css::xml::dom::events::XEventListener >& listener,
            sal_Bool useCapture) override;

        virtual sal_Bool SAL_CALL dispatchEvent(const css::uno::Reference< css::xml::dom::events::XEvent >& evt) override;

        // --- XUnoTunnel
        virtual ::sal_Int64 SAL_CALL
            getSomething(css::uno::Sequence< ::sal_Int8 > const& rId) override;
    };

    /// eliminate redundant namespace declarations
    void nscleanup(const xmlNodePtr aNode, const xmlNodePtr aParent);
}

#endif // INCLUDED_UNOXML_INC_NODE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
