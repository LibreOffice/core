/*************************************************************************
 *
 *  $RCSfile: treeimpl.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2000-11-07 14:35:59 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef CONFIGMGR_CONFIGNODEIMPL_HXX_
#define CONFIGMGR_CONFIGNODEIMPL_HXX_

#include "apitypes.hxx"
#include "synchronize.hxx"
#include "nodeimpl.hxx"
#include "nodefactory.hxx"
#include "configpath.hxx"
#include "template.hxx"

#include <vos/ref.hxx>
#include <vos/refernce.hxx>
#include <stl/vector>
#include <stl/map>
#include <stl/memory>
#include <osl/diagnose.h>

namespace configmgr
{
    class INode;
    class ISubtree;
    class ITemplateProvider;
    class NodeChanges;

    namespace configuration
    {
//-----------------------------------------------------------------------------
        typedef com::sun::star::uno::Any UnoAny;

        typedef unsigned int NodeOffset;
        typedef unsigned int TreeDepth;

        struct NodeInfo;
        struct Attributes;

        class Node;
        class NodeRef;
        class Tree;
        class TreeImpl;
        struct NodeFactory;
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
        class Node
        {
            NodeImplHolder      m_pSpecificNode;
            NodeOffset          m_nParent;
        public:
            Node(NodeImplHolder const& aSpecificNodeImpl, NodeOffset nParent);

        // COMMON: information
            NodeOffset          parent()        const { return m_nParent; }
            NodeType::Enum      getNodeType()   const { return m_pSpecificNode->getType(); }

            Name                name()          const;
            Attributes          attributes()    const;
            NodeInfo            info()          const;

        // change management
            bool hasChanges()                   const { return m_pSpecificNode->hasChanges(); }
            void collectChanges(NodeChanges& rChanges)  const { m_pSpecificNode->collectChanges(rChanges); }
            void markChanged()                        { m_pSpecificNode->markChanged(); }
            void commitChanges()                      { m_pSpecificNode->commitChanges(); }
            void makeIndirect(bool bIndirect)         { NodeImpl::makeIndirect(m_pSpecificNode,bIndirect); }

        /// renames a node without concern for context consistency ! Only works for nodes without parent
            void    renameNode(Name const& aName);

        // COMMON: handler dispatch
            void dispatch(INodeHandler& rHandler) { m_pSpecificNode->dispatch(rHandler); }

        // SET: access to child elements
            bool                isSetNode()     const { return NodeType::isSet(getNodeType()); }
            SetNodeImpl&        setImpl()             { return implGetSetImpl(); }
            SetNodeImpl const&  setImpl()       const { return implGetSetImpl(); }

        // VALUES: access to data
            bool                isValueNode()   const { return NodeType::isValue(getNodeType()); }
            ValueNodeImpl&      valueImpl()           { return implGetValueImpl(); }
            ValueNodeImpl const&valueImpl()     const { return implGetValueImpl(); }

        // GROUP: access to children
            bool                isGroupNode()   const { return NodeType::isGroup(getNodeType()); }
            GroupNodeImpl&      groupImpl()           { return implGetGroupImpl(); }
            GroupNodeImpl const&groupImpl()     const { return implGetGroupImpl(); }

        private:
            SetNodeImpl&   implGetSetImpl()   const;
            GroupNodeImpl& implGetGroupImpl() const ;
            ValueNodeImpl& implGetValueImpl() const ;

        };
//-----------------------------------------------------------------------------
        class RootTreeImpl; // for 'dynamic-casting'
        class ElementTreeImpl; // for 'dynamic-casting'

        /** is the Implementation class for class <type>Tree</type>.
            <p> Holds a list of <type>Node</type> which it allows to access by
                <type>NodeOffset</type> (which is basically a one-based index).
            </p>
            <p> Also provides for navigation to the context this tree is located in
            </p>
        */
        class TreeImpl : public vos::OReference , public ISynchronizedData
        {
        public:
            /// the type of the internal list of <type>Node</type>
            typedef std::vector<Node> NodeList;

        protected:
        //  Construction
            /// creates a TreeImpl for a detached, virgin tree
            TreeImpl( );

            /// creates a TreeImpl without a parent tree
            TreeImpl(   AbsolutePath const& aContextPath,
                        NodeOffset nRoot = 1);

            /// creates a TreeImpl with a parent tree
            TreeImpl(TreeImpl& rParentTree, NodeOffset nParentNode );

            /// fills this TreeImpl starting from rNode, using the given factory
            void build(NodeFactory& rFactory, INode& rNode, TreeDepth nDepth);

        public:
            /// destroys a TreeImpl
            virtual ~TreeImpl();

        // Context Access
            /// gets the path to the parent node of this tree
            AbsolutePath    getContextPath() const  { return m_aContextPath; }
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

        // Node Collection navigation
            /// gets the <type>NodeOffset</type> of the root node in this tree
            NodeOffset      root() const { return m_nRoot; }

            /** gets the <type>NodeOffset</type> of the parent node <var>nNode</var> in this tree
                or 0 (zero) if it is the root node
                <p>PRE: <code>isValidNode(nNode)</code>
                </p>
            */
            NodeOffset      parent(NodeOffset nNode) const;

            /** gets the <type>Name</type> of the node <var>nNode</var>
                <p>PRE: <code>isValidNode(nNode)</code>
                </p>
            */
            Name            name(NodeOffset nNode) const;

            /** gets the number of hierarchy levels from the root node to node <var>nNode</var>
                in this tree
                <p>In particular <code>depthTo(N) == 0</code> if <code>N == root()</code>
                </p>
                <p>PRE: <code>isValidNode(nNode)</code>
                </p>
            */
            TreeDepth       depthTo(NodeOffset nNode) const;

            /// append the local path to a node to a collection of names
            void    appendPathTo(NodeOffset nNode, Path::Components& rNames);

        // Change management
            bool    hasChanges() const;
            void    collectChanges(NodeChanges& rChanges) const;
            void    markChanged(NodeOffset nNode);
            void    commitChanges();
            void    makeIndirect(bool bIndirect);

        // Node iteration and access
            /** gets the <type>NodeOffset</type> of the first child node
                of node <var>nParent</var> in this tree (in list order)
                or 0 (zero) if it has no children in this tree
                <p>PRE: <code>isValidNode(nParent)</code>
                </p>
            */
            NodeOffset      firstChild (NodeOffset nParent) const;

            /** gets the <type>NodeOffset</type> of the next node
                after <var>nNode</var> in this tree (in list order)
                that has the same parent node,
                or 0 (zero) if there is no such node
                <p>PRE: <code>isValidNode(nNode)</code>
                </p>
            */
            NodeOffset      nextSibling(NodeOffset nNode) const;

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
            NodeOffset      findNextChild(NodeOffset nParent, NodeOffset nStartAfter) const;

            /** gets the <type>NodeOffset</type> of the first (and only) child node
                of node <var>nParent</var> in this tree (in list order)
                where the name of the node is <var>aName</var>,
                or 0 (zero) if there is no such node
                <p>PRE: <code>isValidNode(nParent)</code>
                </p>
            */
            NodeOffset      findChild(NodeOffset nParent, Name const& aName) const;

        // Node Collection access
            /// get the number of nodes in this tree
            NodeOffset      nodeCount() const;

            /// get the <type>Node</type> for node <var>nNode</var> in this tree
            Node*       node(NodeOffset nNode);
            /// get the <type>Node</type> for node <var>nNode</var> in this tree
            Node const* node(NodeOffset nNode) const;

        // dynamic_cast replacement
            RootTreeImpl        * asRootTree();
            RootTreeImpl const  * asRootTree() const;

            ElementTreeImpl     * asElementTree();
            ElementTreeImpl const* asElementTree() const;
            virtual ISynchronizedData* getRootLock() = 0;
        private:
            virtual RootTreeImpl const* doCastToRootTree() const = 0;
            virtual ElementTreeImpl const* doCastToElementTree() const = 0;
        protected:
            /// set a new parent context for this tree
            void setContext(TreeImpl* pParentTree, NodeOffset nParentNode);
            /// set no-parent context for this tree
            void clearContext();

        private:
            void implCollectChangesFrom(NodeOffset nNode, NodeChanges& rChanges) const;
            void implCommitChangesFrom(NodeOffset nNode);

            NodeList        m_aNodes;
            AbsolutePath    m_aContextPath;
            TreeImpl*   m_pParentTree;
            NodeOffset  m_nParentNode;
            NodeOffset  const m_nRoot; /// base of <type>NodeOffset</type>s used in this class
            TreeDepth   m_nDepth;

            void initPath();

            friend class TreeImplBuilder;
        };
//-----------------------------------------------------------------------------

        class ElementTreeImpl : public TreeImpl
        {
        public:

            /// creates a TreeImpl for a detached, virgin instance of <var>aTemplate</var>
            ElementTreeImpl( TemplateHolder aTemplate, ITemplateProvider* pProvider );

            /// creates a TreeImpl for a detached, virgin instance of <var>aTemplate</var>
            ElementTreeImpl( TemplateHolder aTemplate, std::auto_ptr<INode>& pNewNode );

            /** creates a TreeImpl with a parent tree, that (supposedly)
                is an instance of <var>aTemplateInfo</var>
            */
            ElementTreeImpl(NodeFactory& rNodeFactory,
                            TreeImpl& rParentTree, NodeOffset nParentNode,
                            INode& rCacheNode, TreeDepth nDepth,
                            TemplateHolder aTemplateInfo );

            /** creates a TreeImpl with no parent node, that (supposedly)
                is an instance of <var>aTemplateInfo</var>
            */
            ElementTreeImpl(NodeFactory& rNodeFactory,
                            INode& rCacheNode, TreeDepth nDepth,
                            TemplateHolder aTemplateInfo );

            ~ElementTreeImpl();

        // Tree information
            /// checks whether this is an instance of a known template
            bool isTemplateInstance() const { return !!m_aInstanceInfo.isValid(); }
            /// checks whether this is an instance of the given template
            bool isInstanceOf(TemplateHolder const& aTemplateInfo) const { return m_aInstanceInfo == aTemplateInfo && aTemplateInfo.isValid(); }
            /// retrieves the template that this is an instance of
            TemplateHolder getTemplate() const { return m_aInstanceInfo; }

        // node control operation
            /// check if this is a free-floating tree
            bool isFree() const { return m_pOwnedNode != 0; }
            /// transfer ownership to the given set
            void attachTo(ISubtree& rOwningSet, Name const& aElementName);
            /// tranfer ownership from the given set
            void detachFrom(ISubtree& rOwningSet, Name const& aElementName);

        // context operation
            /// set a new root name
            void renameTree(Name const& aNewName);
            /// set a new parent context for this tree
            void moveTree(TreeImpl* pParentTree, NodeOffset nParentNode);
            /// set no-parent context for this tree
            void detachTree();

        // ISynchronizedData
            void acquireReadAccess() const;
            void releaseReadAccess() const;
            void acquireWriteAccess();
            void releaseWriteAccess();
            ISynchronizedData* getRootLock();
        private:
            virtual RootTreeImpl const* doCastToRootTree() const;
            virtual ElementTreeImpl const* doCastToElementTree() const;
        private:
            TemplateHolder  const m_aInstanceInfo;
            INode* m_pOwnedNode;
            ISynchronizedData* m_pLockImpl;
        };
//-----------------------------------------------------------------------------
        inline
        SetNodeImpl&   Node::implGetSetImpl()   const
        {
            OSL_ASSERT(isSetNode());
            return AsSetNode(m_pSpecificNode.getBody());
        }
        //---------------------------------------------------------------------
        inline
        GroupNodeImpl& Node::implGetGroupImpl() const
        {
            OSL_ASSERT(isGroupNode());
            return AsGroupNode(m_pSpecificNode.getBody());
        }
        //---------------------------------------------------------------------
        inline
        ValueNodeImpl& Node::implGetValueImpl() const
        {
            OSL_ASSERT(isValueNode());
            return AsValueNode(m_pSpecificNode.getBody());
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
        Node* TreeImpl::node(NodeOffset nNode)
        {
            OSL_ASSERT(isValidNode(nNode));
            return &m_aNodes[nNode-root()];
        }
        //---------------------------------------------------------------------
        inline
        Node const* TreeImpl::node(NodeOffset nNode) const
        {
            OSL_ASSERT(isValidNode(nNode));
            return &m_aNodes[nNode-root()];
        }
//-----------------------------------------------------------------------------

        typedef vos::ORef<TreeImpl> TreeHolder;

//-----------------------------------------------------------------------------
// helper for other impl classes
//-----------------------------------------------------------------------------
    /// is a hack to avoid too many friend declarations in public headers
    class TreeImplHelper
    {
    public:
        static
        NodeRef makeNode(Node* pNode, NodeOffset nOffset, TreeDepth nDepth);

        static
        bool isSet(NodeRef const& aNode);

        static
        bool isGroup(NodeRef const& aNode);

        static
        bool isValue(NodeRef const& aNode);

        static
        TreeImpl* impl(Tree const& aTree);

        static
        Node* node(NodeRef const& aNode);

        static
        NodeOffset offset(NodeRef const& aNode);

    };
//-----------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_CONFIGNODEIMPL_HXX_
