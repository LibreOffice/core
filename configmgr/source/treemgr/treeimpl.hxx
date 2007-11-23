/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: treeimpl.hxx,v $
 *
 *  $Revision: 1.25 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:47:49 $
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

#ifndef CONFIGMGR_CONFIGNODEIMPL_HXX_
#define CONFIGMGR_CONFIGNODEIMPL_HXX_

#ifndef CONFIGMGR_CHANGE_HXX
#include "change.hxx"
#endif
#ifndef CONFIGMGR_CONFIGNODEBEHAVIOR_HXX_
#include "nodeimpl.hxx"
#endif
#ifndef CONFIGMGR_CONFIGPATH_HXX_
#include "configpath.hxx"
#endif
#ifndef CONFIGMGR_CONFIGTEMPLATE_HXX_
#include "template.hxx"
#endif
#ifndef CONFIGMGR_TREEACCESSOR_HXX
#include "treeaccessor.hxx"
#endif

#ifndef CONFIGMGR_UTILITY_HXX_
#include "utility.hxx"
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

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
    class INode;
    class ISubtree;

    class Change;
    class SubtreeChange;

    namespace view { class ViewStrategy; class ViewTreeAccess; }
    namespace node { struct Attributes; }
    namespace configuration
    {
//-----------------------------------------------------------------------------
        typedef com::sun::star::uno::Any UnoAny;

//-----------------------------------------------------------------------------
        class TreeImpl;
        class TemplateProvider;
//-----------------------------------------------------------------------------
        typedef unsigned int NodeOffset;
        typedef unsigned int TreeDepth;

        // WARNING: a similar item is in noderef.hxx
        const TreeDepth c_TreeDepthAll = ~0u;

//-----------------------------------------------------------------------------
        inline
        TreeDepth& incDepth(TreeDepth& rDepth)
        {
            if (rDepth != c_TreeDepthAll) ++rDepth;
            return rDepth;
        }

        inline
        TreeDepth& decDepth(TreeDepth& rDepth)
        {
            OSL_ENSURE(rDepth != 0,"Cannot decrement zero depth");
            if (rDepth != c_TreeDepthAll && rDepth != 0) --rDepth;
            return rDepth;
        }

        inline
        TreeDepth childDepth(TreeDepth nDepth)
        { return decDepth(nDepth); }

        inline
        TreeDepth parentDepth(TreeDepth nDepth)
        { return incDepth(nDepth); }

        inline
        TreeDepth remainingDepth(TreeDepth nOuterDepth, TreeDepth nRelativeDepth)
        {
            OSL_ENSURE(nRelativeDepth != c_TreeDepthAll,"RelativeDepth can't be infinite");
            OSL_ENSURE(nRelativeDepth <= nOuterDepth,"ERROR: RelativeDepth is larger than enclosing depth");

            TreeDepth nInnerDepth = (nOuterDepth == c_TreeDepthAll) ? nOuterDepth :
                                    (nRelativeDepth < nOuterDepth)  ? nOuterDepth-nRelativeDepth :
                                                                      0;
            return nInnerDepth;
        }
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
            Name                     m_aName_; // cached for better performance
            NodeOffset               m_nParent;
        public:
            NodeData(rtl::Reference<NodeImpl> const& aSpecificNodeImpl, Name const& aName, NodeOffset nParent);

            void rebuild(rtl::Reference<view::ViewStrategy> const& _xNewStrategy, data::NodeAccess const & _aNewData);
        // COMMON: information
            Name                getName()       const { return m_aName_; }
            NodeOffset          getParent()     const { return m_nParent; }

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
            data::NodeAccess getOriginalNodeAccess() const
                { return data::NodeAccess(m_pSpecificNode->getOriginalNodeAddress()); }

        private:
            NodeImpl&       implGetNodeImpl() const;
            SetNodeImpl&    implGetSetImpl()   const;
            GroupNodeImpl&  implGetGroupImpl() const ;
            ValueElementNodeImpl& implGetValueImpl() const ;
        };
//-----------------------------------------------------------------------------
        class RootTreeImpl; // for 'dynamic-casting'
        class ElementTreeImpl; // for 'dynamic-casting'

        typedef rtl::Reference<ElementTreeImpl> ElementTreeHolder; // see also setnodeimpl.hxx
        typedef std::vector< ElementTreeHolder > ElementList; // see also setnodeimpl.hxx

        /** is the Implementation class for class <type>Tree</type>.
            <p> Holds a list of <type>Node</type> which it allows to access by
                <type>NodeOffset</type> (which is basically a one-based index).
            </p>
            <p> Also provides for navigation to the context this tree is located in
            </p>
        */
        class TreeImpl : public configmgr::SimpleReferenceObject
        {
            friend class view::ViewStrategy;
            friend class TreeSetNodeImpl;
         //   friend class DeferredSetNodeImpl;
        public:
            /// the type of the internal list of <type>Node</type>
            typedef std::vector<NodeData> NodeList;

        protected:
        //  Construction
            /// creates a TreeImpl for a detached, virgin tree
            explicit
            TreeImpl( );

            /// creates a TreeImpl with a parent tree
            TreeImpl(TreeImpl& rParentTree, NodeOffset nParentNode);

            /// fills this TreeImpl starting from _aRootNode, using the given factory and the tree's template provider
            void build(rtl::Reference<view::ViewStrategy> const& _xStrategy, data::NodeAccess const& _aRootNode, TreeDepth nDepth, TemplateProvider const& aTemplateProvider);

            void rebuild(rtl::Reference<view::ViewStrategy> const& _xNewStrategy, data::NodeAccess const & _aNewData);

        public:
            /// destroys a TreeImpl
            virtual ~TreeImpl();

            // realeses the data this refers to
            virtual void disposeData();

        // Context Access
            /// gets the path to the root node of this tree
            AbsolutePath    getRootPath() const;
            /// gets the tree of parent node of this tree
            TreeImpl*       getContextTree()        { return m_pParentTree; }
            /// gets the tree of parent node of this tree
            TreeImpl const *getContextTree() const  { return m_pParentTree; }
            /// gets the offset of parent node of this tree within its tree
            NodeOffset      getContextNode() const  { return m_nParentNode; }

        // Node Collection information
            /// checks whether <var>nNode</var> is a valid node offset in this tree
            bool isValidNode(NodeOffset nNode)  const;

            /// gets the depth that is available in this tree (due to the original request)
            TreeDepth getAvailableDepth()       const   { return m_nDepth; }

            /// gets the depth that is available in this tree within the given node
            TreeDepth getRemainingDepth(NodeOffset nNode) const
            { return remainingDepth(getAvailableDepth(),depthTo(nNode)); }

        // Node Collection navigation
            /** gets the simple <type>Name</type> of the node <var>nNode</var>
                <p>PRE: <code>isValidNode(nNode)</code>
                </p>
            */
            Name            getSimpleNodeName(NodeOffset nNode) const;

            /** gets the simple <type>Name</type> of the root node (i.e. of the tree as a whole)
            */
            virtual Name            getSimpleRootName() const;

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
            TreeDepth       depthTo(NodeOffset nNode) const;

            /// append the local path (relative to root) to a node to a collection of names
            void    prependLocalPathTo(NodeOffset nNode, Path::Rep& rNames);

            // check whether defaults are available
            bool    hasDefaults(NodeOffset _nNode) const;
        public:
            /// gets the <type>NodeOffset</type> of the root node in this tree
            NodeOffset      root_() const { return m_nRoot; }

            /** gets the <type>NodeOffset</type> of the parent node <var>nNode</var> in this tree
                or 0 (zero) if it is the root node
                <p>PRE: <code>isValidNode(nNode)</code>
                </p>
            */
            NodeOffset      parent_(NodeOffset nNode) const;

        // Node iteration and access
            /** gets the <type>NodeOffset</type> of the first child node
                of node <var>nParent</var> in this tree (in list order)
                or 0 (zero) if it has no children in this tree
                <p>PRE: <code>isValidNode(nParent)</code>
                </p>
            */
            NodeOffset      firstChild_ (NodeOffset nParent) const;

            /** gets the <type>NodeOffset</type> of the next node
                after <var>nNode</var> in this tree (in list order)
                that has the same parent node,
                or 0 (zero) if there is no such node
                <p>PRE: <code>isValidNode(nNode)</code>
                </p>
            */
            NodeOffset      nextSibling_(NodeOffset nNode) const;

            /** gets the <type>NodeOffset</type> of the first child node
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
            NodeOffset      findNextChild_(NodeOffset nParent, NodeOffset nStartAfter) const;

            /** gets the <type>NodeOffset</type> of the first (and only) child node
                of node <var>nParent</var> in this tree (in list order)
                where the name of the node is <var>aName</var>,
                or 0 (zero) if there is no such node
                <p>PRE: <code>isValidNode(nParent)</code>
                </p>
            */
            NodeOffset      findChild_(NodeOffset nParent, Name const& aName) const;

        // Node Collection access
            /// get the number of nodes in this tree
            NodeOffset      nodeCount() const;

            /// get the <type>NodeData</type> for node <var>nNode</var> in this tree
            NodeData*       nodeData(NodeOffset nNode);
            /// get the <type>NodeData</type> for node <var>nNode</var> in this tree
            NodeData const* nodeData(NodeOffset nNode) const;
            /// get the <type>NodeData</type> for node <var>nNode</var> in this tree
            NodeImpl&       nodeImpl(NodeOffset nNode)       { return nodeData(nNode)->nodeImpl(); }
            /// get the <type>NodeData</type> for node <var>nNode</var> in this tree
            NodeImpl const& nodeImpl(NodeOffset nNode) const { return nodeData(nNode)->nodeImpl(); }

            NodeOffset nodeOffset(NodeData const & rNodeData) const;
        // dynamic_cast replacement
            RootTreeImpl        * asRootTree();
            RootTreeImpl const  * asRootTree() const;

            ElementTreeImpl     * asElementTree();
            ElementTreeImpl const* asElementTree() const;

        // Behavior
            rtl::Reference< view::ViewStrategy > getViewBehavior() const;
        protected:
            // immediate commit
/*          // implementation of  commit protocol
            void commitDirect();

            void implCommitDirectFrom(NodeOffset nNode);
*/
            void implRebuild(NodeOffset nNode, data::NodeAccess const & _aNewData);

        protected:
            /// set a new parent context for this tree
            void setContext(TreeImpl* pParentTree, NodeOffset nParentNode);
            /// set no-parent context for this tree
            void clearContext();

            inline // is protected and should be used only in the implementation
            Name    implGetOriginalName(NodeOffset nNode) const;

        private:
            virtual RootTreeImpl const* doCastToRootTree() const = 0;
            virtual ElementTreeImpl const* doCastToElementTree() const = 0;

            /// get the full name of the root of this tree
            virtual Path::Component doGetRootName() const = 0;

            /// prepend the absolute path to the root of this tree (no context use)
            virtual void doFinishRootPath(Path::Rep& rPath) const = 0;

            rtl::Reference<view::ViewStrategy> m_xStrategy;
            NodeList    m_aNodes;
            TreeImpl*   m_pParentTree;
            NodeOffset  m_nParentNode;
            TreeDepth   m_nDepth;

            enum { m_nRoot = 1 }; /// base of <type>NodeOffset</type>s used in this class

            /// prepend the absolute path to the root of this tree (using context if present)
            void implPrependRootPath(Path::Rep& rPath) const;

            friend class TreeImplBuilder;
        };
//-----------------------------------------------------------------------------

        class ElementTreeImpl : public TreeImpl
        {
        public:

            /// creates a TreeImpl for a detached, virgin instance of <var>aTemplate</var> (always will be direct)
            ElementTreeImpl( data::TreeSegment const& _aElementData, TemplateHolder aTemplate, TemplateProvider const& aTemplateProvider  );

            /** creates a TreeImpl with a parent tree, that (supposedly)
                is an instance of <var>aTemplateInfo</var>
            */
            ElementTreeImpl(rtl::Reference<view::ViewStrategy> const& _xStrategy,
                            TreeImpl& rParentTree, NodeOffset nParentNode,
                            data::TreeAccessor const& _aDataTree, TreeDepth nDepth,
                            TemplateHolder aTemplateInfo,
                            TemplateProvider const& aTemplateProvider );

            /** creates a TreeImpl with no parent node, that (supposedly)
                is an instance of <var>aTemplateInfo</var>
            */
            ElementTreeImpl(rtl::Reference<view::ViewStrategy> const& _xStrategy,
                            data::TreeAccessor const& _aDataTree, TreeDepth nDepth,
                            TemplateHolder aTemplateInfo,
                            TemplateProvider const& aTemplateProvider );

            ~ElementTreeImpl();

        // realeses the data this refers to
            virtual void disposeData();

        // rebuilding
            using TreeImpl::rebuild;
            void rebuild(rtl::Reference<view::ViewStrategy> const& _xNewStrategy, data::TreeAccessor const & _aNewData);

        // data access
            data::TreeAccessor  getOriginalTreeAccess() const { return data::TreeAccessor(m_aDataAddress); }

        // Tree information
            virtual Name            getSimpleRootName() const;
            /// checks whether this is an instance of a known template
            bool isTemplateInstance() const { return !!m_aInstanceInfo.is(); }
            /// checks whether this is an instance of the given template
            bool isInstanceOf(TemplateHolder const& aTemplateInfo) const
            { return m_aInstanceInfo == aTemplateInfo && aTemplateInfo.is(); }
            /// retrieves the template that this is an instance of
            TemplateHolder getTemplate() const { return m_aInstanceInfo; }
            /// makes a complete name from a simple name and template information
            Path::Component makeExtendedName(Name const& aSimpleName) const;

        // node control operation
            /// check if this is a free-floating tree
            bool isFree() const { return m_aOwnData.is(); }
            /// transfer ownership to the given set
            void attachTo(data::SetNodeAccess const & _aUpdatableSetNode, Name const& aElementName);
            /// tranfer ownership from the given set
            void detachFrom(data::SetNodeAccess const & _aUpdatableSetNode, Name const& aElementName);

            /// take ownership of the given tree (which must not already be the one in use)
            void takeTreeAndRebuild(data::TreeSegment const& _aElementData);
            /// take ownership of the given tree (which must already be the one in use)
            void takeTreeBack(data::TreeSegment const& _aElementData);

            /// release ownership
            data::TreeSegment getOwnedTree() const;
            /// release ownership
            data::TreeSegment releaseOwnedTree();

        // context operation
            /// set a new root name
            void renameTree(Name const& aNewName);
            /// set a new parent context for this tree
            void moveTree(TreeImpl* pParentTree, NodeOffset nParentNode);
            /// set no-parent context for this tree
            void detachTree();

        private:
            static bool isUpdatableSegment(TreeImpl& _rTree);

            virtual RootTreeImpl const* doCastToRootTree() const;
            virtual ElementTreeImpl const* doCastToElementTree() const;

            virtual Path::Component doGetRootName() const;

            virtual void doFinishRootPath(Path::Rep& rPath) const;
        private:
            TemplateHolder  const   m_aInstanceInfo;
            Name                    m_aElementName;
            data::TreeAddress       m_aDataAddress;
            data::TreeSegment       m_aOwnData;
        };
//-----------------------------------------------------------------------------

        inline
        NodeImpl&   NodeData::implGetNodeImpl()   const
        {
            OSL_ASSERT(m_pSpecificNode != 0);
            return *m_pSpecificNode;
        }
//-----------------------------------------------------------------------------
        inline
        bool TreeImpl::isValidNode(NodeOffset nNode) const
        {
            return m_nRoot <= nNode && nNode < nodeCount() +m_nRoot;
        }
        //---------------------------------------------------------------------
        inline
        NodeOffset TreeImpl::nodeCount() const
        {
            return m_aNodes.size();
        }
        //---------------------------------------------------------------------
        inline
        NodeData* TreeImpl::nodeData(NodeOffset nNode)
        {
            if (nNode == 0) return NULL;
            OSL_ASSERT(isValidNode(nNode));
            return &m_aNodes[nNode-root_()];
        }
        //---------------------------------------------------------------------
        inline
        NodeData const* TreeImpl::nodeData(NodeOffset nNode) const
        {
            if (nNode == 0) return NULL;
            OSL_ASSERT(isValidNode(nNode));
            return &m_aNodes[nNode-root_()];
        }
        //---------------------------------------------------------------------
        inline
        NodeOffset TreeImpl::nodeOffset(NodeData const & rNode) const
        {
            NodeOffset nOffset = root_() + (&rNode - &m_aNodes[0]);
            OSL_ASSERT(isValidNode(nOffset));
            return nOffset;
        }

//-----------------------------------------------------------------------------
// helper for other impl classes
//-----------------------------------------------------------------------------
    class Tree;
    class TreeRef;
    class NodeRef;
    class ValueRef;
    class AnyNodeRef;
    class NodeID;

    class ValueMemberNode;
//-----------------------------------------------------------------------------
    /// is a hack to avoid too many friend declarations in public headers
    class TreeImplHelper
    {
    public:
        static
        NodeRef makeNode(NodeOffset nOffset, TreeDepth nDepth);

        static
        NodeRef makeNode(TreeImpl& rTree, NodeOffset nOffset);

        static
        NodeRef makeNode(NodeID const& aNodeID);

        static
        ValueRef makeValue(Name const& aName, NodeOffset nParentOffset);

        static
        AnyNodeRef makeAnyNode(NodeOffset nOffset, TreeDepth nDepth);

        static
        AnyNodeRef makeAnyNode(Name const& aName, NodeOffset nParentOffset);

        static
        TreeImpl* impl(Tree const& aTree);

        static
        TreeImpl* impl(TreeRef const& aTree);

        static
        NodeData* nodeData(NodeRef const& aNode);

        static
        NodeOffset offset(NodeRef const& aNode);

        static
        NodeOffset parent_offset(ValueRef const& aNode);

        static
        Name value_name(ValueRef const& aNode);

        static
        ValueMemberNode member_node(Tree const & _aTree,ValueRef const& aValue);

        static
        TreeImpl* tree(NodeID const& aNodeID);

        static
        NodeOffset offset(NodeID const& aNodeID);

    };
//-----------------------------------------------------------------------------
    class ElementTree;
//-----------------------------------------------------------------------------

    struct ElementHelper
    {
        static
        UnoType getUnoType(ElementTree const& aElement);
    };
//-----------------------------------------------------------------------------
    }
//-----------------------------------------------------------------------------
}

#endif // CONFIGMGR_CONFIGNODEIMPL_HXX_
