/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile$
 * $Revision$
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

#ifndef CONFIGMGR_CONFIGNODEIMPL_HXX_
#define CONFIGMGR_CONFIGNODEIMPL_HXX_

#include "change.hxx"
#include "configpath.hxx"
#include "template.hxx"
#include "utility.hxx"
#include <rtl/ref.hxx>
#include <salhelper/simplereferenceobject.hxx>
#include <osl/diagnose.h>

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif
#ifndef INCLUDED_MAP
#include <map>
#define INCLUDED_MAP
#endif
#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif

namespace configmgr
{
    namespace node { struct Attributes; }
    namespace sharable { union Node; }
    namespace view { class ViewStrategy; }
    namespace configuration
    {
//-----------------------------------------------------------------------------
        class AnyNodeRef;
        class ElementTree;
        class GroupNodeImpl;
        class NodeChange;
        class NodeChanges;
        class NodeImpl;
        class NodeRef;
        class SetNodeImpl;
        class TemplateProvider;
        class Tree;
        class ValueElementNodeImpl;
        class ValueMemberNode;
        class ValueRef;
//-----------------------------------------------------------------------------
        // WARNING: a similar item is in noderef.hxx
        const unsigned int c_TreeDepthAll = ~0u;

//-----------------------------------------------------------------------------
        inline
        unsigned int& incDepth(unsigned int& rDepth)
        {
            if (rDepth != c_TreeDepthAll) ++rDepth;
            return rDepth;
        }

        inline
        unsigned int& decDepth(unsigned int& rDepth)
        {
            OSL_ENSURE(rDepth != 0,"Cannot decrement zero depth");
            if (rDepth != c_TreeDepthAll && rDepth != 0) --rDepth;
            return rDepth;
        }

        inline
        unsigned int childDepth(unsigned int nDepth)
        { return decDepth(nDepth); }

        inline
        unsigned int parentDepth(unsigned int nDepth)
        { return incDepth(nDepth); }

        inline
        unsigned int remainingDepth(unsigned int nOuterDepth, unsigned int nRelativeDepth)
        {
            OSL_ENSURE(nRelativeDepth != c_TreeDepthAll,"RelativeDepth can't be infinite");
            OSL_ENSURE(nRelativeDepth <= nOuterDepth,"ERROR: RelativeDepth is larger than enclosing depth");

            unsigned int nInnerDepth = (nOuterDepth == c_TreeDepthAll) ? nOuterDepth :
                                    (nRelativeDepth < nOuterDepth)  ? nOuterDepth-nRelativeDepth :
                                                                      0;
            return nInnerDepth;
        }
    //-------------------------------------------------------------------------

        /// interface for a class that can be used to do some operation on a set of <type>NodeRef</type>s and <type>ValueRef</type>s.
        struct NodeVisitor
        {
            /// returned from <method>handle</method> to indicate whether the operation is complete or should continue
            enum Result { DONE, CONTINUE };
            /// do the operation on <var>aNode</var>. needs to be implemented by concrete visitor classes
            virtual Result handle(rtl::Reference< Tree > const& aTree, NodeRef const& aNode) = 0;
            /// do the operation on <var>aValue</var>. needs to be implemented by concrete visitor classes
            virtual Result handle(rtl::Reference< Tree > const& aTree, ValueRef const& aValue) = 0;
        protected:
            virtual ~NodeVisitor() {}
        };
//-----------------------------------------------------------------------------

// class Node Impl
//-----------------------------------------------------------------------------

        /** is the Implementation class for class <type>Node</type>, held inside a <type>Tree</type>.
            <p> Implements some functionality common to all node types.
            </p>
            <p> Otherwise it provides (not really typesafe) access to a
                <type scope='configmgr::configuration::NodeType>NodeImpl</type> which implements
                functionality for a node of a given type
                (as given by a <type scope='configmgr::configuration::NodeType>Enum</type> value).
            </p>
        */
        class NodeData
        {
            rtl::Reference<NodeImpl> m_pSpecificNode;
            rtl::OUString m_aName_; // cached for better performance
            unsigned int                 m_nParent;
        public:
            NodeData(rtl::Reference<NodeImpl> const& aSpecificNodeImpl, rtl::OUString const& aName, unsigned int nParent);

            void rebuild(rtl::Reference<view::ViewStrategy> const& _xNewStrategy, sharable::Node * _aNewData);
        // COMMON: information
            rtl::OUString getName()       const { return m_aName_; }
            unsigned int            getParent()     const { return m_nParent; }

        // change management
        public:
        // BASIC NODE: access to common attributes
            NodeImpl &          nodeImpl()          { return implGetNodeImpl(); }
            NodeImpl const &    nodeImpl() const    { return implGetNodeImpl(); }

        // SET: access to child elements
            bool                isSetNode()     const;
            SetNodeImpl&        setImpl()             { return implGetSetImpl(); }
            SetNodeImpl const&  setImpl()       const { return implGetSetImpl(); }

        // VALUES: access to data
            bool                        isValueElementNode()    const;
            ValueElementNodeImpl&       valueElementImpl()            { return implGetValueImpl(); }
            ValueElementNodeImpl const& valueElementImpl()      const { return implGetValueImpl(); }

        // GROUP: access to children
            bool                isGroupNode()   const;
            GroupNodeImpl&      groupImpl()           { return implGetGroupImpl(); }
            GroupNodeImpl const&groupImpl()     const { return implGetGroupImpl(); }

        // access helper
        public:
            sharable::Node * getOriginalNodeAccess() const;

        private:
            NodeImpl&       implGetNodeImpl() const;
            SetNodeImpl&    implGetSetImpl()   const;
            GroupNodeImpl&  implGetGroupImpl() const ;
            ValueElementNodeImpl& implGetValueImpl() const ;
        };
//-----------------------------------------------------------------------------
        /** represents a hierarchy of config entries (identified by <type>NodeRef</type>s and <type>ValueRef</type>s)

            <p>Examples for trees include</p>
            <ulist>
                <li>A module tree (for a specific set of parameters).</li>
                <li>An updating tree (for a part of the whole).</li>
                <li>A set element (updating or not), which could be detached.</li>
            <ulist>
            <p> Holds a list of <type>Node</type> which it allows to access by
                <type>unsigned int</type> (which is basically a one-based index).
            </p>
            <p> Also provides for navigation to the context this tree is located in
            </p>
        */
        class Tree : public salhelper::SimpleReferenceObject
        {
            friend class view::ViewStrategy;
        protected:
        //  Construction
            /// creates a Tree for a detached, virgin tree
            Tree( );

            /// creates a Tree with a parent tree
            Tree(Tree& rParentTree, unsigned int nParentNode);

            virtual ~Tree() = 0;

            /// fills this Tree starting from _aRootNode, using the given factory and the tree's template provider
            void build(rtl::Reference<view::ViewStrategy> const& _xStrategy, sharable::Node * _aRootNode, unsigned int nDepth, TemplateProvider const& aTemplateProvider);

            void rebuild(rtl::Reference<view::ViewStrategy> const& _xNewStrategy, sharable::Node * _aNewData);

        public:
            // realeses the data this refers to
            virtual void disposeData();

        // Context Access
            /// gets the path to the root node of this tree
            AbsolutePath    getRootPath() const;
            /// gets the tree of parent node of this tree
            Tree*       getContextTree()        { return m_pParentTree; }
            /// gets the tree of parent node of this tree
            Tree const *getContextTree() const  { return m_pParentTree; }
            /// gets the offset of parent node of this tree within its tree
            unsigned int        getContextNode() const  { return m_nParentNode; }

        // Node Collection information
            /// checks whether <var>nNode</var> is a valid node offset in this tree
            bool isValidNode(unsigned int nNode)    const;

            bool isRootNode(NodeRef const & node) const;

            NodeRef getRootNode() const;

            NodeRef getContextNodeRef() const;

            bool isValidValueNode(ValueRef const & value);

            bool isValidAnyNode(AnyNodeRef const & node);

            /// checks whether the node has any element nodes (of its own)
            bool hasElements(NodeRef const & node);

            bool hasElement(NodeRef const & node, rtl::OUString const & name);

            bool hasElement(NodeRef const & node, Path::Component const & name);

            /** gets the element with the given name of the given node
                <p>PRE: <code>hasElement(node, name)</code></p>
                <p>If there is no such element, may return an empty node or
                raise an exception (?)</p>

                @throws InvalidName
                    if name is not a valid child name for this node
            */
            rtl::Reference< ElementTree > getElement(
                NodeRef const & node, rtl::OUString const & name);

            /** gets the element with the given name of the given node, if it is
                available
                <p>PRE: <code>hasElement(node, name)</code></p>
                <p>If there is no such element, may return an empty node or
                raise an exception (?)</p>
                <p>Caution: May miss existing children unless hasChild/getChild
                has been called before.</p>

                @throws InvalidName
                    if name is not a valid child name for this node
            */
            rtl::Reference< ElementTree > getAvailableElement(
                NodeRef const & node, rtl::OUString const & name);

            /// checks whether the node has any child nodes (in this tree)
            bool hasChildren(NodeRef const & node);

            bool hasChildValue(
                NodeRef const & node, rtl::OUString const & name);

            bool hasChildNode(NodeRef const & node, rtl::OUString const & name);

            bool hasChild(NodeRef const & node, rtl::OUString const & name);

            /** gets the child value (in this tree) with the given name of the
                given node
                <p>PRE: <code>hasChildValue(node, name)</code></p>
                <P>If there is no such node, may return an empty node or raise
                an exception (?)</p>

                @throws InvalidName
                    if <var>aName</var> is not a valid child name for this node
            */
            ValueRef getChildValue(
                NodeRef const & node, rtl::OUString const & name);

            NodeRef getChildNode(
                NodeRef const & node, rtl::OUString const & name);

            AnyNodeRef getAnyChild(
                NodeRef const& node, rtl::OUString const & name);

            node::Attributes getAttributes(NodeRef const & node);

            node::Attributes getAttributes(AnyNodeRef const & node);

            node::Attributes getAttributes(ValueRef const & value);

            com::sun::star::uno::Type getUnoType(ValueRef const & value);

            /// return the parent of the given node (or an empty node, if it is
            /// the tree root)
            NodeRef getParent(NodeRef const & node);

            /// return the parent of the given value (or an empty node, if it is
            /// the tree root)
            NodeRef getParent(ValueRef const & value);

            AbsolutePath getAbsolutePath(NodeRef const & node);

            /// retrieves the current value for the given node, provided there
            /// is one and it is available (only works for value nodes)
            com::sun::star::uno::Any getNodeValue(ValueRef const & value);

            /// checks whether the given node has a default value (only works
            /// for value nodes)
            bool hasNodeDefault(ValueRef const & value);

            /// checks whether the given node assumes its default value (only
            /// works for value nodes)
            bool isNodeDefault(ValueRef const & value);

            /// checks whether the given node has a default state
            bool hasNodeDefault(NodeRef const & node);

            /// checks whether the given node assumes its default state
            bool isNodeDefault(NodeRef const & node);

            /// checks whether the given node has a default state
            bool hasNodeDefault(AnyNodeRef const & node);

            /// checks whether the given node assumes its default state
            bool isNodeDefault(AnyNodeRef const & node);

            /// checks whether the default values are available for the children
            /// of the given node (if applicable)
            bool areValueDefaultsAvailable(NodeRef const & node);

            /// retrieves the default value for the given node, provided there
            /// is one and it is available (only works for value nodes)
            com::sun::star::uno::Any getNodeDefaultValue(
                ValueRef const & value);

            bool hasChanges();

            /// lists any pending changes on this tree
            bool collectChanges(NodeChanges & changes);

            void integrate(
                NodeChange & change, NodeRef const & node, bool local);

            void integrate(
                NodeChanges & changes, NodeRef const & node, bool local);

            NodeVisitor::Result visit(
                NodeRef const & node, NodeVisitor & visitor)
            { return visitor.handle(this, node); }

            NodeVisitor::Result visit(
                ValueRef const & value, NodeVisitor & visitor)
            { return visitor.handle(this, value); }

            /** lets the given visitor visit the child nodes of the given node

                The order in which nodes are visited is repeatable (but
                currently unspecified).  Visits nodes until NodeVisitor::DONE is
                returned, then returns NodeVisitor::DONE.  If all visits return
                NodeVisitor::CONTINUE, returns NodeVisitor::CONTINUE.  If no
                children are present, returns NodeVisitor::CONTINUE.
            */
            NodeVisitor::Result dispatchToChildren(
                NodeRef const & node, NodeVisitor & visitor);

            NodeRef getNode(unsigned int offset) const;

            rtl::Reference< Template > extractElementInfo(NodeRef const & node);

            /// gets the depth that is available in this tree (due to the original request)
            unsigned int getAvailableDepth()        const   { return m_nDepth; }

            /// gets the depth that is available in this tree within the given node
            unsigned int getRemainingDepth(unsigned int nNode) const
            { return remainingDepth(getAvailableDepth(),depthTo(nNode)); }

        // Node Collection navigation
            /** gets the simple name of the node <var>nNode</var>
                <p>PRE: <code>isValidNode(nNode)</code>
                </p>
            */
            rtl::OUString getSimpleNodeName(unsigned int nNode) const;

            /** gets the simple name of the root node (i.e. of the tree as a whole)
            */
            virtual rtl::OUString getSimpleRootName() const;

            /** gets the full name of the root node
            */
            Path::Component getExtendedRootName() const;

            /** gets the number of hierarchy levels from the root node to node <var>nNode</var>
                in this tree
                <p>In particular <code>depthTo(N) == 0</code> if <code>N == root()</code>
                </p>
                <p>PRE: <code>isValidNode(nNode)</code>
                </p>
            */
            unsigned int        depthTo(unsigned int nNode) const;

            /// append the local path (relative to root) to a node to a collection of names
            void    prependLocalPathTo(unsigned int nNode, Path::Rep& rNames);

            // check whether defaults are available
            bool    hasDefaults(unsigned int _nNode) const;
        public:
            enum { ROOT = 1 }; /// base of <type>unsigned int</type>s used in this class

            /** gets the <type>unsigned int</type> of the parent node <var>nNode</var> in this tree
                or 0 (zero) if it is the root node
                <p>PRE: <code>isValidNode(nNode)</code>
                </p>
            */
            unsigned int        parent_(unsigned int nNode) const;

        // Node iteration and access
            /** gets the <type>unsigned int</type> of the first child node
                of node <var>nParent</var> in this tree (in list order)
                or 0 (zero) if it has no children in this tree
                <p>PRE: <code>isValidNode(nParent)</code>
                </p>
            */
            unsigned int        firstChild_ (unsigned int nParent) const;

            /** gets the <type>unsigned int</type> of the first child node
                of node <var>nParent</var> that is after
                node <var>nNode</var> in this tree (in list order)
                or 0 (zero) if there is no such node
                <p>if <code>nStartAfter == 0</code> searching starts at the beginning
                </p>
                <p>PRE: <code>isValidNode(nParent)</code>
                </p>
                <p>PRE: <code>isValidNode(nStartAfter) || nStartAfter == 0</code>
                </p>
            */
            unsigned int        findNextChild_(unsigned int nParent, unsigned int nStartAfter) const;

            /** gets the <type>unsigned int</type> of the first (and only) child node
                of node <var>nParent</var> in this tree (in list order)
                where the name of the node is <var>aName</var>,
                or 0 (zero) if there is no such node
                <p>PRE: <code>isValidNode(nParent)</code>
                </p>
            */
            unsigned int        findChild_(unsigned int nParent, rtl::OUString const& aName) const;

        // Node Collection access
            /// get the number of nodes in this tree
            unsigned int        nodeCount() const;

            /// get the <type>NodeData</type> for node <var>nNode</var> in this tree
            NodeData*       nodeData(unsigned int nNode);
            /// get the <type>NodeData</type> for node <var>nNode</var> in this tree
            NodeData const* nodeData(unsigned int nNode) const;
            /// get the <type>NodeData</type> for node <var>nNode</var> in this tree
            NodeImpl&       nodeImpl(unsigned int nNode)       { return nodeData(nNode)->nodeImpl(); }
            /// get the <type>NodeData</type> for node <var>nNode</var> in this tree
            NodeImpl const& nodeImpl(unsigned int nNode) const { return nodeData(nNode)->nodeImpl(); }

            unsigned int nodeOffset(NodeData const & rNodeData) const;

        // Behavior
            rtl::Reference< view::ViewStrategy > getViewBehavior() const;
        protected:
            // immediate commit
/*          // implementation of  commit protocol
            void commitDirect();

            void implCommitDirectFrom(unsigned int nNode);
*/
            void implRebuild(unsigned int nNode, sharable::Node * _aNewData);

        protected:
            /// set a new parent context for this tree
            void setContext(Tree* pParentTree, unsigned int nParentNode);
            /// set no-parent context for this tree
            void clearContext();

            inline // is protected and should be used only in the implementation
            rtl::OUString implGetOriginalName(unsigned int nNode) const;

        private:
            /// get the full name of the root of this tree
            virtual Path::Component doGetRootName() const = 0;

            /// prepend the absolute path to the root of this tree (no context use)
            virtual void doFinishRootPath(Path::Rep& rPath) const = 0;

            ValueMemberNode getMemberNode(ValueRef const & value);

            rtl::Reference<view::ViewStrategy> m_xStrategy;
            std::vector<NodeData>   m_aNodes;
            Tree*   m_pParentTree;
            unsigned int    m_nParentNode;
            unsigned int    m_nDepth;

            /// prepend the absolute path to the root of this tree (using context if present)
            void implPrependRootPath(Path::Rep& rPath) const;

            friend class TreeImplBuilder;
        };

        /// checks, if tree represents a real tree
        bool isEmpty(Tree * tree);
//-----------------------------------------------------------------------------

        class ElementTree : public Tree
        {
        public:

            /// creates a Tree for a detached, virgin instance of <var>aTemplate</var> (always will be direct)
            ElementTree(rtl::Reference< data::TreeSegment > const& _aElementData, rtl::Reference<Template> aTemplate, TemplateProvider const& aTemplateProvider  );

            /** creates a Tree with a parent tree, that (supposedly)
                is an instance of <var>aTemplateInfo</var>
            */
            ElementTree(rtl::Reference<view::ViewStrategy> const& _xStrategy,
                            Tree& rParentTree, unsigned int nParentNode,
                            sharable::TreeFragment * dataTree, unsigned int nDepth,
                            rtl::Reference<Template> aTemplateInfo,
                            TemplateProvider const& aTemplateProvider );

            /** creates a Tree with no parent node, that (supposedly)
                is an instance of <var>aTemplateInfo</var>
            */
            ElementTree(rtl::Reference<view::ViewStrategy> const& _xStrategy,
                            sharable::TreeFragment * dataTree, unsigned int nDepth,
                            rtl::Reference<Template> aTemplateInfo,
                            TemplateProvider const& aTemplateProvider );

            ~ElementTree();

        // realeses the data this refers to
            virtual void disposeData();

        // rebuilding
            using Tree::rebuild;
            void rebuild(rtl::Reference<view::ViewStrategy> const& _xNewStrategy, sharable::TreeFragment * newData);

        // data access
            sharable::TreeFragment * getOriginalTreeAccess() const { return m_aDataAddress; }

        // Tree information
            virtual rtl::OUString getSimpleRootName() const;
            /// checks whether this is an instance of a known template
            bool isTemplateInstance() const { return !!m_aInstanceInfo.is(); }
            /// checks whether this is an instance of the given template
            bool isInstanceOf(rtl::Reference<Template> const& aTemplateInfo) const
            { return m_aInstanceInfo == aTemplateInfo && aTemplateInfo.is(); }
            /// retrieves the template that this is an instance of
            rtl::Reference<Template> getTemplate() const { return m_aInstanceInfo; }
            /// makes a complete name from a simple name and template information
            Path::Component makeExtendedName(rtl::OUString const& aSimpleName) const;

        // node control operation
            /// check if this is a free-floating tree
            bool isFree() const { return m_aOwnData.is(); }
            /// transfer ownership to the given set
            void attachTo(sharable::SetNode * updatableSetNode, rtl::OUString const& aElementName);
            /// tranfer ownership from the given set
            void detachFrom(sharable::SetNode * updatableSetNode, rtl::OUString const& aElementName);

            /// take ownership of the given tree (which must not already be the one in use)
            void takeTreeAndRebuild(rtl::Reference< data::TreeSegment > const& _aElementData);
            /// take ownership of the given tree (which must already be the one in use)
            void takeTreeBack(rtl::Reference< data::TreeSegment > const& _aElementData);

            /// release ownership
            rtl::Reference< data::TreeSegment > releaseOwnedTree();

        // context operation
            /// set a new root name
            void renameTree(rtl::OUString const& aNewName);
            /// set a new parent context for this tree
            void moveTree(Tree* pParentTree, unsigned int nParentNode);
            /// set no-parent context for this tree
            void detachTree();

        private:
            static bool isUpdatableSegment(Tree& _rTree);

            virtual Path::Component doGetRootName() const;

            virtual void doFinishRootPath(Path::Rep& rPath) const;
        private:
            rtl::Reference<Template>    const   m_aInstanceInfo;
            rtl::OUString m_aElementName;
            sharable::TreeFragment *       m_aDataAddress;
            rtl::Reference< data::TreeSegment > m_aOwnData;
        };
//-----------------------------------------------------------------------------
        inline
        bool Tree::isValidNode(unsigned int nNode) const
        {
            return ROOT <= nNode && nNode < nodeCount() + ROOT;
        }
        //---------------------------------------------------------------------
        inline
        unsigned int Tree::nodeCount() const
        {
            return m_aNodes.size();
        }
        //---------------------------------------------------------------------
        inline
        NodeData* Tree::nodeData(unsigned int nNode)
        {
            if (nNode == 0) return NULL;
            OSL_ASSERT(isValidNode(nNode));
            return &m_aNodes[nNode - ROOT];
        }
        //---------------------------------------------------------------------
        inline
        NodeData const* Tree::nodeData(unsigned int nNode) const
        {
            if (nNode == 0) return NULL;
            OSL_ASSERT(isValidNode(nNode));
            return &m_aNodes[nNode - ROOT];
        }
        //---------------------------------------------------------------------
        inline
        unsigned int Tree::nodeOffset(NodeData const & rNode) const
        {
            unsigned int nOffset = ROOT + (&rNode - &m_aNodes[0]);
            OSL_ASSERT(isValidNode(nOffset));
            return nOffset;
        }

//-----------------------------------------------------------------------------
// helper for other impl classes
//-----------------------------------------------------------------------------
#if OSL_DEBUG_LEVEL > 0
    struct ElementHelper
    {
        static
        com::sun::star::uno::Type getUnoType(rtl::Reference< ElementTree > const& aElement);
    };
#endif
//-----------------------------------------------------------------------------
    }
//-----------------------------------------------------------------------------
}

#endif // CONFIGMGR_CONFIGNODEIMPL_HXX_
