/*************************************************************************
 *
 *  $RCSfile: noderef.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jb $ $Date: 2000-11-10 12:19:02 $
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

#ifndef CONFIGMGR_CONFIGNODE_HXX_
#define CONFIGMGR_CONFIGNODE_HXX_

#include "apitypes.hxx"
#include "configexcept.hxx"
#include <stl/vector>

namespace configmgr
{
    class INode;
    class ISynchronizedData;

    namespace configapi { class Factory; }
    namespace configuration
    {
    //-------------------------------------------------------------------------
        class Name;
        class AbsolutePath;
        class RelativePath;

        struct NodeInfo;
        struct Attributes;

        class NodeChange;
        class NodeChanges;
        class NodeChangesTree;
    //-------------------------------------------------------------------------

        namespace argument { struct NoValidate; }

        typedef com::sun::star::uno::Type       UnoType;
        typedef com::sun::star::uno::Any        UnoAny;
    //-------------------------------------------------------------------------

        struct NodeState
        {
            enum State { eDEFAULT, eREPLACING, eMODIFYING, eREMOVING, eUNCHANGED = -1 };
        };
    //-------------------------------------------------------------------------

        class NodeID;
        class NodeRef;
        class Tree;

        class Node;
        class TreeImpl;

        typedef unsigned int NodeOffset;
        typedef unsigned int TreeDepth;

    //-------------------------------------------------------------------------

        /// interface for a class that can be used to do some operation on a set of <type>NodeRef</type>s.
        struct NodeVisitor
        {
            /// returned from <method>handle</method> to indicate whether the operation is complete or should continue
            enum Result { DONE, CONTINUE };
            /// do the operation on <var>aNode</var>. needs to be implemented by concrete visitor classes
            virtual Result handle(NodeRef const& aNode) = 0;
        };
    //-------------------------------------------------------------------------

        /// represents a node position in some tree
        class NodeRef
        {
        public:
            /// constructs an empty (invalid) node
            NodeRef();

            /// copy a node (with reference semantics)
            NodeRef(NodeRef const& rOther);
            /// copy a node (with reference semantics)
            NodeRef& operator=(NodeRef const& rOther);

            void swap(NodeRef& rOther);

            ~NodeRef();

            /// checks, if this represents an existing node
            bool isValid() const;

            /// checks whether this node has any child nodes (of its own).
            bool hasChildren() const;

            /// checks whether this node owns a child node named <var>aName</var>.
            bool hasChild(Name const& aName) const;

            /** gets the owned child node named <var>aName</var> of this node.
                <p>Also sets <var>aTree<var/> to point to the tree containing that node.</p>
                <p>PRE: <code>hasChild(aName) == true</code></p>
                <p>If there is no such node, may return an empty node or raise an exception (?)</p>

                @throws InvalidName
                    if <var>aName</var> is not a valid child name for this node
            */
            NodeRef getChild(Name const& aName, Tree& aTree) const;

            /// return a <type>NodeInfo</type> showing the attributes of this
            NodeInfo        getInfo()   const;

            /// return a <type>NodeInfo</type> showing the attributes of this
            Attributes      getAttributes() const;

            /// return the local <type>Name</type> of this (or an empty name, if there isn't one)
            Name            getName() const;

            /// get the Uno <type scope='com::sun::star::uno'>Type</type> of the object represented by this node
            UnoType         getUnoType() const;

            /// dispatch this node to a Visitor
            NodeVisitor::Result accept(NodeVisitor& aVisitor) const { return aVisitor.handle(*this); }

        private:
            friend class Tree;
            friend class TreeImplHelper;
            NodeRef(Node*   pImpl, NodeOffset nPos, TreeDepth nDepth);
        private:
            Node*   m_pImpl;
            NodeOffset  m_nPos;
            TreeDepth   m_nDepth;
        };
    //-------------------------------------------------------------------------

        /** represents a hierarchy of config entries (identified by <type>NodeRef</type>s)

            <p>Examples for trees include</p>
            <ulist>
                <li>A module tree (for a specific set of parameters).</li>
                <li>An updating tree (for a part of the whole).</li>
                <li>A set element (updating or not), which could be detached.</li>
            <ulist>
        */
        class Tree
        {
        public:
            /// create a tree with a given implementation
            Tree(TreeImpl* pImpl);
            /// copy a tree (with reference semantics)
            Tree(Tree const& rOther);
            /// copy a tree (with reference semantics)
            Tree& operator=(Tree const& rOther);

            void swap(Tree& rOther);

            ~Tree();

            /// checks, if this represents an real tree
            bool isEmpty() const;

            /// checks whether the node <var>aNode</var> is a valid node in this tree.
            bool isValidNode(NodeRef const& aNode) const;

            /// checks whether the node <var>aNode</var> has any child nodes (in this tree).
            bool hasChildren(NodeRef const& aNode) const;

            /// checks whether the node <var>aNode</var> have a child node (in this tree) named <var>aName</var>.
            bool hasChild(NodeRef const& aNode, Name const& aName) const;

            /** gets the child node (in this tree) named <var>aName</var> of node <var>aNode</var>.
                <p>PRE: <code>hasChild(aNode,aName) == true</code></p>
                <P>If there is no such node, may return an empty node or raise an exception (?)</p>

                @throws InvalidName
                    if <var>aName</var> is not a valid child name for this node
            */
            NodeRef getChild(NodeRef const& aNode, Name const& aName) const;

        // Parent/NodeRef context handling
        public:
            /// return the parent <type>NodeRef</type> of <var>aNode</var> (or an empty node, if it is the tree root)
            NodeRef         getParent(NodeRef const& aNode) const;

            /// return the <type>Path</type> of <var>aNode</var>, relative to the tree root (or an empty path if it is empty)
            RelativePath    getLocalPath(NodeRef const& aNode) const;

            /// gets the root node of this tree
            NodeRef         getRootNode() const;

            /// checks whether <var>aNode</var> is the root node of this tree
            bool            isRootNode(NodeRef const& aNode) const;

        // default value handling
        public:
            /// ensure default values are available for nodes where they can be provided at all
            void        ensureDefaults() const;

            /// checks whether <var>aNode</var> assumes its default value
            bool        isNodeDefault(NodeRef const& aNode)     const; // only works for value nodes

            /** retrieves the default value for <var>aNode</var>, provided there is one and it
                is available.
                <p>call <method>Tree::ensureDefaults</method> first to achieve best results</p>
            */
            UnoAny      getNodeDefault(NodeRef const& aNode)        const; // only works for value nodes

        // Tree context handling
        public:
            /// gets the parent tree of this tree, if available
            Tree getContextTree() const;
            /// gets the parent node of this tree ('s root node), if available
            NodeRef getContextNode() const;
            /// gets the path of the (possibly fictitious) parent node of this tree
            AbsolutePath getContextPath() const;

        // Update handling
        public:
            /// checks whether there are pending changes on this tree
            bool hasChanges()  const;

            /// lists any pending changes on this tree
            bool collectChanges(NodeChanges& aChanges)  const;

            /// applies <var>aChange</var> to <var>aNode</var> within this tree
            void integrate(NodeChange& aChange, NodeRef const& aNode, bool bLocal)  const;

            /// applies <var>aChanges</var> to the children or descendants of <var>aNode</var> within this tree
            void integrate(NodeChanges& aChanges, NodeRef const& aNode, bool bLocal) const;

            /// applies <var>aChanges</var> to the descendants of <var>aNode</var> within this tree
            void integrate(NodeChangesTree& aChanges, NodeRef const& aNode) const;

        // Visitor handling
        public:
            /** lets <var>aVisitor</var> visit the child nodes of <var>aNode</var>
                <p>The order in which nodes are visited is repeatable (but currently unspecified)</p>
                <p> Visits nodes until NodeVisitor::DONE is returned, then returns NodeVisitor::DONE.<BR/>
                    If all visits return NodeVisitor::CONTINUE, returns NodeVisitor::CONTINUE.<BR/>
                    If no children are present, returns NodeVisitor::CONTINUE
                </p>
            */
            NodeVisitor::Result dispatchToChildren(NodeRef const& aNode, NodeVisitor& aVisitor) const;
        // More NodeRef handling
        public:
            NodeRef bind(NodeOffset nNode) const;
            NodeRef rebind(NodeRef const& aNode) const;

        // Comparison
        public:
            friend bool equalTree(Tree const& lhs, Tree const& rhs) { return lhs.m_pImpl == rhs.m_pImpl; }
        private:
            friend class TreeImplHelper;
            TreeImpl* m_pImpl;
        };
    //-------------------------------------------------------------------------

        class NodeID
        {
        public:
            NodeID(Tree const& rTree, NodeRef const& rNode);
            NodeID(TreeImpl* pImpl, NodeOffset nNode);

        // comparison
            // equality
            friend bool operator==(NodeID const& lhs, NodeID const& rhs)
            { return lhs.m_pTree == rhs.m_pTree && lhs.m_nNode == rhs.m_nNode; }
            // ordering
            friend bool operator < (NodeID const& lhs, NodeID const& rhs);
            // hashing
            size_t hashCode() const;
        private:
            friend class TreeImplHelper;
            TreeImpl*   m_pTree;
            NodeOffset  m_nNode;
        };
    //-------------------------------------------------------------------------

        /** make a <type>Name</type> out of <var>sName</var>.
            @throws InvalidName
                if <var>sName is not a valid name for a <var>aNode</var> within <var>aTree</var>

        */
        Name validateNodeName(OUString const& sName, Tree const& aTree, NodeRef const& aNode );

        /** reduce <var>aPath</var> to be a path relative to (<var>aTree<var/>,<var>aNode<var/>)

            @returns
                <var>aPath<var/> if it already is a relative path or the part of it relative to <var>aNode<var/>
                if it is an absolute path that to a descendant of <var>aNode<var/>
            @throws InvalidName
                if <var>aPath<var/> is an absolute path that is not to a descendant of <var>aNode<var/>
        */
        RelativePath reduceRelativePath(OUString const& aPath, Tree const& aTree, NodeRef const& aBaseNode);

        /* * reduce <var>aPath</var> to be a path relative to (<var>aTree<var/>,<var>aNode<var/>)

            @returns
                <var>aPath<var/> if it already is a relative path or the part of it relative to <var>aNode<var/>
                if it is an absolute path that to a descendant of <var>aNode<var/>

            @returns
                an empty path, if <var>aPath<var/> is an absolute path that is not to a descendant of <var>aNode<var/>
        RelativePath reduceRelativePath(OUString const& aPath, Tree const& aTree, NodeRef const& aBaseNode, argument::NoValidate);
        */

        /** checks whether there are any immediate children of <var>aNode</var> (which is in <var>aTree</var>)

            @return
                <TRUE/> if a child node exists
                <FALSE/> otherwise
        */
        bool hasChildNode(Tree const& aTree, NodeRef const& aNode);

        /** checks whether there is an immediate child of <var>aNode</var> (which is in <var>aTree</var>)
            specified by <var>aName</var>

            @return
                <TRUE/> if the child node exists
                <FALSE/> otherwise
        */
        bool hasChildNode(Tree const& aTree, NodeRef const& aNode, Name const& aName);

        /** tries to find the immediate child of <var>aNode</var> (which is in <var>aTree</var>)
            specified by <var>aName</var>
            <p> On return <var>aNode</var> is modified to refer to the node found and
                <var>aTree</var> will then refer to the tree that node is in.
            <p/>

            @return
                <TRUE/> if the child node exists
                (so <var>aNode</var> and <var>aTree</var> refer to the desired node and <var>aPath</var> is empty),
                <FALSE/> otherwise
        */
        bool findChildNode(Tree& aTree, NodeRef& aNode, Name const& aName);

        /** tries to find the descendant of <var>aNode</var> (which is in <var>aTree</var>) specified by <var>aPath</var>
            <p> This function follows the given path stepwise, until a requested node is missing in the tree.</p>
            <p> On return <var>aNode</var> is modified to refer to the last node found and
                <var>aTree</var> will then refer to the tree that node is in.
            <p/>
            <p> Also, <var>aPath</var> is modified to contain the unresolved part of the original path.
            </p>

            @return
                <TRUE/> if the path could be resolved completely
                (so <var>aNode</var> and <var>aTree</var> refer to the desired node,
                <var>aPath</var> is empty)<BR/>
                <FALSE/> otherwise
        */
        bool findDescendantNode(Tree& aTree, NodeRef& aNode, RelativePath& aPath);

        /// test whether the given node is a plain value
        bool isSimpleValue(Tree const& aTree, NodeRef const& aNode);

        /// test whether the given node is a inner group node
        bool isGroupNode(Tree const& aTree, NodeRef const& aNode);

        /// test whether the given node is a inner set node
        bool isSetNode(Tree const& aTree, NodeRef const& aNode);

        /** extract the value from a plain value
            <p> PRE: <code>isSimpleValue(aTree,aNode)</code>
            </p>
        */
        UnoAny getSimpleValue(Tree const& aTree, NodeRef const& aNode);

        ISynchronizedData* getRootLock(Tree const& aTree);

        typedef std::vector<NodeID>     NodeIDList;

        void getAllContainedNodes(Tree const& aTree, NodeIDList& aList);
        void getAllChildrenHelper(NodeID const& aNode, NodeIDList& aList);

    //-------------------------------------------------------------------------

        inline bool operator!=(NodeID const& lhs, NodeID const& rhs)
        { return !(lhs == rhs); }
        //---------------------------------------------------------------------

        inline bool operator>=(NodeID const& lhs, NodeID const& rhs)
        { return !(lhs < rhs); }
        //---------------------------------------------------------------------
        inline bool operator > (NodeID const& lhs, NodeID const& rhs)
        { return  (rhs < lhs); }
        inline bool operator<=(NodeID const& lhs, NodeID const& rhs)
        { return !(rhs < lhs); }
    //-------------------------------------------------------------------------
    }
}

#endif // CONFIGMGR_CONFIGNODE_HXX_
